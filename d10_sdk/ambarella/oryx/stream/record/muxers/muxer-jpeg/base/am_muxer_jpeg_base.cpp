/*******************************************************************************
 * am_muxer_jpeg_base.cpp
 *
 * History:
 *   2015-10-8 - [ccjing] created file
 *
 * Copyright (c) 2016 Ambarella, Inc.
 *
 * This file and its contents ("Software") are protected by intellectual
 * property rights including, without limitation, U.S. and/or foreign
 * copyrights. This Software is also the confidential and proprietary
 * information of Ambarella, Inc. and its licensors. You may not use, reproduce,
 * disclose, distribute, modify, or otherwise prepare derivative works of this
 * Software or any portion thereof except pursuant to a signed license agreement
 * or nondisclosure agreement with Ambarella, Inc. or its authorized affiliates.
 * In the absence of such an agreement, you agree to promptly notify and return
 * this Software to Ambarella, Inc.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
 * MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL AMBARELLA, INC. OR ITS AFFILIATES BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; COMPUTER FAILURE OR MALFUNCTION; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/
#include "am_base_include.h"
#include "am_amf_types.h"
#include "am_amf_interface.h"
#include "am_amf_packet.h"
#include "am_log.h"
#include "am_define.h"
#include "am_mutex.h"
#include "am_thread.h"

#include "am_file_sink_if.h"
#include "am_muxer_codec_info.h"
#include "am_muxer_jpeg_file_writer.h"
#include "am_muxer_jpeg_exif.h"
#include "am_file.h"
#include "am_api_helper.h"
#include "am_api_image.h"
#include "am_muxer_jpeg_base.h"

#include <time.h>
#include <math.h>
#include <unistd.h>
#include <sys/statfs.h>
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <signal.h>
#include <sys/time.h>
#include "mw_struct.h"

AMExifParameter AMMuxerJpegBase::m_exif_param;
static const uint8_t exif_header[] = {
  0xff, 0xd8, 0xff, 0xe1
};

AMMuxerJpegBase::AMMuxerJpegBase() :
    m_stop_recording_pts(0),
    m_thread(nullptr),
    m_muxer_jpeg_config(nullptr),
    m_exif_config(nullptr),
    m_config(nullptr),
    m_packet_queue(nullptr),
    m_config_file(nullptr),
    m_file_writer(nullptr),
    m_jpeg_exif(nullptr),
    m_state(AM_MUXER_CODEC_INIT),
    m_stream_id(0),
    m_recording_file_num(0),
    m_recording_duration(0),
    m_file_counter(0),
    m_exif_image_width(0),
    m_exif_image_height(0),
    m_file_writing(true),
    m_update_exif_once(false),
    m_run(false)
{
}

AM_STATE AMMuxerJpegBase::init(const char* config_file)
{
  AM_STATE ret = AM_STATE_OK;
  do {
    if (AM_UNLIKELY(config_file == nullptr)) {
      ERROR("Config_file is NULL, should input valid config_file");
      ret = AM_STATE_ERROR;
      break;
    }
    m_config_file = amstrdup(config_file);
    if (AM_UNLIKELY((m_packet_queue = new packet_queue()) == nullptr)) {
      ERROR("Failed to create packet_queue in AMMuxerJpegBase.");
      ret = AM_STATE_NO_MEMORY;
      break;
    }
    if (AM_UNLIKELY((m_config = new AMMuxerJpegConfig()) == nullptr)) {
      ERROR("Failed to create jpeg config in AMMuxerJpegBase.");
      ret = AM_STATE_NO_MEMORY;
      break;
    }
    m_muxer_jpeg_config = m_config->get_config(std::string(m_config_file));
    if (AM_UNLIKELY(!m_muxer_jpeg_config)) {
      ERROR("Failed to get config in AMMuxerJpegBase");
      ret = AM_STATE_ERROR;
      break;
    }
    m_exif_config_file = std::string(ORYX_MUXER_CONF_DIR) + "/jpeg-exif.acs";
    m_exif_config = m_config->get_exif_config(m_exif_config_file);
    if (AM_UNLIKELY(!m_exif_config)) {
      ERROR("Failed to get exif_config in AMMuxerJpegBase");
      ret = AM_STATE_ERROR;
      break;
    }
    m_file_writing = m_muxer_jpeg_config->auto_file_writing;
    m_recording_file_num = m_muxer_jpeg_config->recording_file_num;
    m_recording_duration = m_muxer_jpeg_config->recording_duration;

    /* set timer */
    if (m_exif_config->enable) {
      signal(SIGALRM, timer_action);
      if(AM_UNLIKELY(AM_STATE_OK != timer_set(m_exif_config->update_time))) {
        ERROR("Failed to set timer!");
        ret = AM_STATE_ERROR;
        break;
      }
    }
  } while (0);
  return ret;
}

AMMuxerJpegBase::~AMMuxerJpegBase()
{
  stop();
  delete m_config;
  delete m_packet_queue;
  delete[] m_config_file;
  INFO("AMJpegMuxer object destroyed.");
}

AM_STATE AMMuxerJpegBase::start()
{
  AUTO_MEM_LOCK(m_interface_lock);
  AM_STATE ret = AM_STATE_OK;
  do {
    bool need_break = false;
    switch (get_state()) {
      case AM_MUXER_CODEC_RUNNING: {
        NOTICE("The jpeg muxer is already running.");
        need_break = true;
      }break;
      case AM_MUXER_CODEC_ERROR: {
        NOTICE("Jpeg muxer state is error! Need to be re-created!");
        need_break = true;
      }break;
      default:
        break;
    }
    if (AM_UNLIKELY(need_break)) {
      break;
    }
    AM_DESTROY(m_thread);
    if (AM_LIKELY(!m_thread)) {
      m_thread = AMThread::create(m_muxer_name.c_str(), thread_entry, this);
      if (AM_UNLIKELY(!m_thread)) {
        ERROR("Failed to create thread.");
        ret = AM_STATE_ERROR;
        break;
      }
    }
    while ((get_state() == AM_MUXER_CODEC_STOPPED)
        || (get_state() == AM_MUXER_CODEC_INIT)) {
      usleep(5000);
    }
    if(get_state() == AM_MUXER_CODEC_RUNNING) {
      NOTICE("Start %s success.", m_muxer_name.c_str());
    } else {
      ERROR("Failed to start %s.", m_muxer_name.c_str());
      ret = AM_STATE_ERROR;
      break;
    }
  } while (0);
  return ret;
}

AM_MUXER_CODEC_STATE AMMuxerJpegBase::create_resource()
{
  char file_name[strlen(m_muxer_jpeg_config->file_location.c_str())
        + strlen(m_muxer_jpeg_config->file_name_prefix.c_str()) + 128];
  memset(file_name, 0, sizeof(file_name));
  INFO("Begin to create resource in %s.", m_muxer_name.c_str());
  AM_MUXER_CODEC_STATE ret = AM_MUXER_CODEC_RUNNING;
  do {
    if (AM_UNLIKELY(generate_file_name(file_name) != AM_STATE_OK)) {
      ERROR("%s generate file name error, exit main loop.",
            m_muxer_name.c_str());
      ret = AM_MUXER_CODEC_ERROR;
      break;
    }
    AM_DESTROY(m_file_writer);
    if (AM_UNLIKELY((m_file_writer = AMJpegFileWriter::create (
        m_muxer_jpeg_config)) == NULL)) {
      ERROR("Failed to create m_file_writer in %s!", m_muxer_name.c_str());
      ret = AM_MUXER_CODEC_ERROR;
      break;
    }
    if (AM_UNLIKELY(m_file_writer->set_media_sink(file_name) != AM_STATE_OK)) {
      ERROR("Failed to set media sink for m_file_writer in %s!",
            m_muxer_name.c_str());
      ret = AM_MUXER_CODEC_ERROR;
      break;
    }
    if (AM_UNLIKELY((m_jpeg_exif = AMMuxerJpegExif::create()) == nullptr)) {
      ERROR("Failed to create m_jpeg_exif in %s!", m_muxer_name.c_str());
      ret = AM_MUXER_CODEC_ERROR;
      break;
    }

  } while (0);
  AUTO_MEM_LOCK(m_state_lock);
  m_state = ret;
  return ret;
}

AM_STATE AMMuxerJpegBase::stop()
{
  AUTO_MEM_LOCK(m_interface_lock);
  AM_STATE ret = AM_STATE_OK;
  m_run = false;
  AM_DESTROY(m_thread);
  NOTICE("Stop %s success.", m_muxer_name.c_str());
  return ret;
}

bool AMMuxerJpegBase::start_file_writing()
{
  bool ret = true;
  INFO("Begin to start file writing in %s.", m_muxer_name.c_str());
  char file_name[strlen(m_muxer_jpeg_config->file_location.c_str())
        + strlen(m_muxer_jpeg_config->file_name_prefix.c_str()) + 128];
  do{
    if(m_file_writing) {
      NOTICE("File writing is already startted in %s.", m_muxer_name.c_str());
      break;
    }
    if (m_file_writer) {
      if (AM_UNLIKELY(generate_file_name(file_name) != AM_STATE_OK)) {
        ERROR("Generate file name error in %s.", m_muxer_name.c_str());
        ret = false;
        break;
      }
      if (AM_UNLIKELY(m_file_writer->set_media_sink(file_name) != AM_STATE_OK)) {
        ERROR("Failed to set file name to m_file_writer in %s!",
              m_muxer_name.c_str());
        ret = false;
        break;
      }
    }
    m_file_writing = true;
    m_file_counter = 0;
    m_stop_recording_pts = 0;
    INFO("Start file writing success in %s.", m_muxer_name.c_str());
  }while(0);
  return ret;
}

bool AMMuxerJpegBase::stop_file_writing()
{
  bool ret = true;
  INFO("Begin to stop file writing in %s.", m_muxer_name.c_str());
  do{
    if(!m_file_writing) {
      NOTICE("File writing is already stopped in %s", m_muxer_name.c_str());
      break;
    }
    m_file_writing = false;
    m_file_counter = 0;
    m_stop_recording_pts = 0;
    INFO("Stop file writing success in %s.", m_muxer_name.c_str());
  }while(0);
  return ret;
}

bool AMMuxerJpegBase::set_recording_file_num(uint32_t file_num)
{
  NOTICE("Set recording file num %u success in %s", file_num,
         m_muxer_name.c_str());
  m_recording_file_num = file_num;
  return true;
}

bool AMMuxerJpegBase::set_recording_duration(int32_t duration)
{
  NOTICE("Set recording duration %u seconds in %s", duration,
         m_muxer_name.c_str());
  m_recording_duration = duration;
  return true;
}

bool AMMuxerJpegBase::is_running()
{
  return m_run.load();
}

void AMMuxerJpegBase::release_resource()
{
  while (!(m_packet_queue->empty())) {
    m_packet_queue->front()->release();
    m_packet_queue->pop_front();
  }
  AM_DESTROY(m_file_writer);
  INFO("Release resource success in %s.", m_muxer_name.c_str());
}

uint8_t AMMuxerJpegBase::get_muxer_codec_stream_id()
{
  return (uint8_t)((0x01) << (m_stream_id));
}

uint32_t AMMuxerJpegBase::get_muxer_id()
{
  return m_muxer_jpeg_config->muxer_id;
}

AM_MUXER_CODEC_STATE AMMuxerJpegBase::get_state()
{
  AUTO_MEM_LOCK(m_state_lock);
  return m_state;
}

AM_STATE AMMuxerJpegBase::set_config(AMMuxerCodecConfig* config)
{
  return AM_STATE_ERROR;
}

AM_STATE AMMuxerJpegBase::get_config(AMMuxerCodecConfig* config)
{
  return AM_STATE_ERROR;
}

void AMMuxerJpegBase::thread_entry(void* p)
{
  ((AMMuxerJpegBase*) p)->main_loop();
}

bool AMMuxerJpegBase::get_current_time_string(char *time_str, int32_t len,
                                              const char *format)
{
  time_t current = time(NULL);
  if (AM_UNLIKELY(strftime(time_str, len, format, localtime(&current))
                  == 0)) {
    ERROR("Date string format error!");
    time_str[0] = '\0';
    return false;
  }
  return true;
}

bool AMMuxerJpegBase::get_proper_file_location(std::string& file_location)
{
  bool ret = true;
  uint64_t max_free_space = 0;
  uint64_t free_space = 0;
  std::ifstream file;
  std::string read_line;
  std::string storage_str = "/storage";
  std::string sdcard_str = "/sdcard";
  std::string config_path;
  string_list location_list;
  location_list.clear();
  size_t find_str_position = 0;
  INFO("Begin to get proper file location in %s.", m_muxer_name.c_str());
  do {
    if (m_muxer_jpeg_config->file_location_auto_parse) {
      std::string file_location_suffix;
      std::string::size_type pos = 0;
      if ((pos = m_muxer_jpeg_config->file_location.find(storage_str, pos))
          != std::string::npos) {
        pos += storage_str.size() + 1;
        config_path = storage_str + "/";
      } else {
        pos = 1;
        config_path = "/";
      }
      bool parse_config_path = false;
      for (uint32_t i = pos;i < m_muxer_jpeg_config->file_location.size(); ++ i) {
        if (m_muxer_jpeg_config->file_location.substr(i, 1) != "/") {
          config_path += m_muxer_jpeg_config->file_location.substr(i, 1);
        } else {
          INFO("Get config path is %s", config_path.c_str());
          parse_config_path = true;
          break;
        }
      }
      if (!parse_config_path) {
        WARN("The file location in config file is not on sdcard or storage, "
            "use file location specified in config file.");
        file_location = m_muxer_jpeg_config->file_location;
        break;
      }
      pos = m_muxer_jpeg_config->file_location.find('/', pos);
      file_location_suffix = m_muxer_jpeg_config->file_location.substr(pos);
      file.open("/proc/self/mounts");
      INFO("mount information :");
      while (getline(file, read_line)) {
        std::string temp_location;
        temp_location.clear();
        INFO("%s", read_line.c_str());
        if ((find_str_position = read_line.find(storage_str))
            != std::string::npos) {
          for (uint32_t i = find_str_position;; ++ i) {
            if (read_line.substr(i, 1) != " ") {
              temp_location += read_line.substr(i, 1);
            } else {
              location_list.push_back(temp_location);
              INFO("Find a storage str : %s in %s", temp_location.c_str(),
                   m_muxer_name.c_str());
              break;
            }
          }
        } else if ((find_str_position = read_line.find(sdcard_str))
            != std::string::npos) {
          for (uint32_t i = find_str_position;; ++ i) {
            if (read_line.substr(i, 1) != " ") {
              temp_location += read_line.substr(i, 1);
            } else {
              location_list.push_back(temp_location);
              INFO("Find a sdcard str : %s in %s", temp_location.c_str(),
                   m_muxer_name.c_str());
              break;
            }
          }
        }
      }
      if (!location_list.empty()) {
        string_list::iterator it = location_list.begin();
        for (; it != location_list.end(); ++ it) {
          if ((*it) == config_path) {
            NOTICE("File location specified by config file is on %s, great."
                    , config_path.c_str());
            file_location = config_path + file_location_suffix;
            break;
          }
        }
        if (it == location_list.end()) {
          WARN("File location specified by config file is not on sdcard "
              "or usb strorage, %s will auto parse a proper file location",
              m_muxer_name.c_str());
          string_list::iterator max_free_space_it = location_list.begin();
          for (string_list::iterator i = location_list.begin();
              i != location_list.end(); ++ i) {
            struct statfs disk_statfs;
            if (statfs((*i).c_str(), &disk_statfs) < 0) {
              PERROR("File location statfs");
              ret = false;
              break;
            } else {
              free_space = ((uint64_t) disk_statfs.f_bsize
                  * (uint64_t) disk_statfs.f_bfree) / (uint64_t) (1024 * 1024);
              if (free_space > max_free_space) {
                max_free_space = free_space;
                max_free_space_it = i;
              }
            }
          }
          if (!ret) {
            break;
          }
          struct statfs disk_statfs;
          if (statfs(config_path.c_str(), &disk_statfs) < 0) {
            NOTICE("The file location in config file is not exist in %s.",
                   m_muxer_name.c_str());
            file_location = (*max_free_space_it) + file_location_suffix;
          } else {
            free_space = ((uint64_t) disk_statfs.f_bsize
                * (uint64_t) disk_statfs.f_bfree) / (uint64_t) (1024 * 1024);
            if (free_space > max_free_space) {
              NOTICE("The free space of file location in config file is larger"
                  "than sdcard free space in %s, use file location in "
                  "config file.", m_muxer_name.c_str());
              file_location = config_path + file_location_suffix;
            } else {
              file_location = (*max_free_space_it) + file_location_suffix;
              NOTICE("The free space of file location in config file is smaller "
                  "than sdcard free space, set file location on %s.",
                  (*max_free_space_it).c_str());
            }
          }
        }
      } else {
        NOTICE("Do not find storage or sdcard string in mount information in"
            " %s.", m_muxer_name.c_str());
        if (!AMFile::exists(m_muxer_jpeg_config->file_location.c_str())) {
          if (!AMFile::create_path(m_muxer_jpeg_config->file_location.c_str())) {
            ERROR("Failed to create file path: %s in %s!",
                  m_muxer_jpeg_config->file_location.c_str(),
                  m_muxer_name.c_str());
            ret = false;
            break;
          }
        }
        struct statfs disk_statfs;
        if (statfs(m_muxer_jpeg_config->file_location.c_str(), &disk_statfs)
            < 0) {
          PERROR("File location in config file statfs");
          ret = false;
          break;
        } else {
          free_space = ((uint64_t) disk_statfs.f_bsize
              * (uint64_t) disk_statfs.f_bfree) / (uint64_t) (1024 * 1024);
          if (free_space >= 20) {
            file_location = m_muxer_jpeg_config->file_location;
            NOTICE("Free space is larger than 20M, use it in %s.",
                   m_muxer_name.c_str());
            break;
          } else {
            ERROR("Free space is smaller than 20M, please"
                "set file location on sdcard or usb storage in %s.",
                m_muxer_name.c_str());
            ret = false;
            break;
          }
        }
      }
    } else { //file_location_auto_parse is false
      file_location = m_muxer_jpeg_config->file_location;
      break;
    }
    if (!ret) {
      break;
    }
    NOTICE("Get proper file location: %s success in %s.",
           file_location.c_str(), m_muxer_name.c_str());
  } while (0);
  if (file.is_open()) {
    file.close();
  }
  return ret;
}

void AMMuxerJpegBase::check_storage_free_space()
{
  uint64_t free_space = 0;
  struct statfs disk_statfs;
  if(statfs(m_file_location.c_str(), &disk_statfs) < 0) {
    PERROR("Statfs");
    ERROR("%s statfs error in %s", m_file_location.c_str(),
          m_muxer_name.c_str());
  } else {
    free_space = ((uint64_t)disk_statfs.f_bsize *
        (uint64_t)disk_statfs.f_bfree) / (uint64_t)(1024 * 1024);
    DEBUG("Free space is %llu M in %s", free_space, m_muxer_name.c_str());
    if(AM_UNLIKELY(free_space <=
                   m_muxer_jpeg_config->smallest_free_space)) {
      ERROR("The free space is smaller than %d M in %s, "
          "will stop writing data to jpeg file",
          m_muxer_jpeg_config->smallest_free_space, m_muxer_name.c_str());
      m_file_writing = false;
    }
  }
}

AM_STATE AMMuxerJpegBase::timer_set(int time)
{
  AM_STATE state = AM_STATE_OK;
  itimerval tick;
  memset(&tick, 0, sizeof(tick));
  tick.it_value.tv_sec = time;
  tick.it_value.tv_usec = 0;
  tick.it_interval.tv_sec = time;
  tick.it_interval.tv_usec = 0;
  if(AM_UNLIKELY(setitimer(ITIMER_REAL, &tick, nullptr) < 0)) {
    ERROR("Set timer failed!\n");
    state = AM_STATE_ERROR;
  }
  return state;
}

void AMMuxerJpegBase::timer_action(int signo)
{
  if (AM_UNLIKELY(AM_STATE_OK != update_exif_parameters())) {
    ERROR("Failed to update exif parameters!");
  }
}

AM_STATE AMMuxerJpegBase::update_exif_parameters()
{
  AM_STATE state = AM_STATE_OK;
  static AMAPIHelperPtr g_api_helper = nullptr;
  do {
    g_api_helper = AMAPIHelper::get_instance();
    if (!g_api_helper) {
      ERROR("unable to get AMAPIHelper instance\n");
      state = AM_STATE_ERROR;
      break;
    }
    am_service_result_t service_result = {0};
    g_api_helper->method_call(AM_IPC_MW_CMD_IMAGE_AE_SETTING_GET,
                              nullptr,
                              0,
                              &service_result,
                              sizeof(service_result));
    if (AM_UNLIKELY(service_result.ret != 0)) {
      ERROR("Failed to get AE setting!\n");
      state = AM_STATE_ERROR;
      break;
    }
    am_ae_config_s *ae_setting_get = (am_ae_config_s *)service_result.data;
    /* iso */
    float iso_tmp =  (float)ae_setting_get->sensor_gain / 6.0;
    m_exif_param.iso = powf(2, iso_tmp) * 100;

    /* exposure time */
    m_exif_param.exposure_time_den = ae_setting_get->sensor_shutter;

    /* metering mode */
    switch (ae_setting_get->ae_metering_mode) {
      case MW_AE_SPOT_METERING: {
        m_exif_param.metering_mode = AM_METERING_SPOT;
      }break;
      case MW_AE_CENTER_METERING: {
        m_exif_param.metering_mode = AM_METERING_CENTER_WEIGHTED_AVERAGE;
      }break;
      case MW_AE_AVERAGE_METERING: {
        m_exif_param.metering_mode = AM_METERING_AVERAGE;
      }break;
      case MW_AE_CUSTOM_METERING: {
        m_exif_param.metering_mode = AM_METERING_PARTIAL;
      }break;
      default : {
        m_exif_param.metering_mode = AM_METERING_UNKNOWN;
      }break;
    }
    g_api_helper->method_call(AM_IPC_MW_CMD_IMAGE_AWB_SETTING_GET,
                              nullptr,
                              0,
                              &service_result,
                              sizeof(service_result));
    if (AM_UNLIKELY(service_result.ret != 0)) {
      ERROR("Failed to get AE setting!\n");
      state = AM_STATE_ERROR;
      break;
    }
    am_awb_config_s *awb_setting_get = (am_awb_config_s *)service_result.data;
    /* white balance */
    switch (awb_setting_get->wb_mode) {
      case MW_WB_AUTO: {
        m_exif_param.white_balance = AM_AUTO_WHITE_BALANCE;
      }break;
      case MW_WB_SUNNY: {  // 6500K
        m_exif_param.white_balance = AM_MANUAL_WHITE_BALANCE;
        m_exif_param.light_source = AM_LIGHT_DAYLIGHT;
      }break;
      case MW_WB_FLASH: {
        m_exif_param.white_balance = AM_MANUAL_WHITE_BALANCE;
        m_exif_param.light_source = AM_LIGHT_FLASH;
      }break;
      case MW_WB_CLOUDY: {  // 7500K
        m_exif_param.white_balance = AM_MANUAL_WHITE_BALANCE;
        m_exif_param.light_source = AM_LIGHT_CLOUDY_WEATHER;
      }break;
      case MW_WB_INCANDESCENT:  // 2800K
      case MW_WB_D4000:
      case MW_WB_D5000:
      case MW_WB_FLUORESCENT:
      case MW_WB_FLUORESCENT_H:
      case MW_WB_UNDERWATER:
      case MW_WB_CUSTOM:        // custom
      case MW_WB_MODE_NUMBER:
      default : {
        m_exif_param.white_balance = AM_MANUAL_WHITE_BALANCE;
      }break;
    }
  } while(0);
  g_api_helper = nullptr;

  return state;
}

AM_STATE AMMuxerJpegBase::create_exif_data_packet(uint8_t *&data, uint32_t &len,
                                                  AMPacket *packet)
{
  AM_STATE state = AM_STATE_OK;
  ExifEntry *entry;
  uint8_t *exif_data;
  uint32_t exif_data_len;
  do {
    if (!m_exif_config->enable) {
      data = packet->get_data_ptr();
      len = packet->get_data_size();
      break;
    }
    if (AM_UNLIKELY(!m_update_exif_once)) {
      if (AM_UNLIKELY(AM_STATE_OK != update_exif_parameters())) {
        ERROR("Failed to update exif parameters!");
        state = AM_STATE_ERROR;
        break;
      } else {
        m_update_exif_once = true;
      }
    }
    /* PIXEL_X_DIMENSION */
    entry = m_jpeg_exif->init_tag(EXIF_IFD_EXIF, EXIF_TAG_PIXEL_X_DIMENSION);
    exif_set_short(entry->data, FILE_BYTE_ORDER, m_exif_image_width);

    /* PIXEL_Y_DIMENSION */
    entry = m_jpeg_exif->init_tag(EXIF_IFD_EXIF, EXIF_TAG_PIXEL_Y_DIMENSION);
    exif_set_short(entry->data, FILE_BYTE_ORDER, m_exif_image_height);

    /* ExposureTime */
    ExifRational et_rational = {1, m_exif_param.exposure_time_den};
    entry = m_jpeg_exif->init_tag(EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_TIME);
    exif_set_rational(entry->data, FILE_BYTE_ORDER, et_rational);

    //ISO
    entry = m_jpeg_exif->init_tag(EXIF_IFD_EXIF, EXIF_TAG_ISO_SPEED_RATINGS);
    exif_set_short(entry->data, FILE_BYTE_ORDER, m_exif_param.iso);

    /* ApertureValue */
    ExifRational ap_rational = {m_exif_config->aperture_value.num,
                                m_exif_config->aperture_value.denom};
    entry = m_jpeg_exif->init_tag(EXIF_IFD_EXIF, EXIF_TAG_APERTURE_VALUE);
    exif_set_rational(entry->data, FILE_BYTE_ORDER, ap_rational);

    /* F-Number */
    float aperture = (float)m_exif_config->aperture_value.num /
                     (float)m_exif_config->aperture_value.denom;
    aperture = powf(1.4142, aperture);
    ExifRational f_rational = {(unsigned)(aperture * 1000), 1000};
    entry = m_jpeg_exif->init_tag(EXIF_IFD_EXIF, EXIF_TAG_FNUMBER);
    exif_set_rational(entry->data, FILE_BYTE_ORDER, f_rational);

    /* FocalLength */
    ExifRational fl_rational = {m_exif_config->focal_length.num,
                                m_exif_config->focal_length.denom};
    entry = m_jpeg_exif->init_tag(EXIF_IFD_EXIF, EXIF_TAG_FOCAL_LENGTH);
    exif_set_rational(entry->data, FILE_BYTE_ORDER, fl_rational);

    // DateTime & DateTimeOriginal
    char time_string[32] = {0};
    if (AM_UNLIKELY(!(get_current_time_string(time_string,
                                              sizeof(time_string),
                                              EXIF_TIME_FORMAT)))) {
      ERROR("Get current time string error in %s.", m_muxer_name.c_str());
      state = AM_STATE_ERROR;
      break;
    }
    if (AM_UNLIKELY(nullptr == (entry = m_jpeg_exif->is_exif_content_has_entry(
                                EXIF_IFD_0, EXIF_TAG_DATE_TIME)))) {
      entry = m_jpeg_exif->create_tag(EXIF_IFD_0,
                                      EXIF_TAG_DATE_TIME,
                                      sizeof(time_string));
    }
    entry->format = EXIF_FORMAT_ASCII;
    memcpy(entry->data, time_string, sizeof(time_string));
    if (AM_UNLIKELY(nullptr == (entry = m_jpeg_exif->is_exif_content_has_entry(
                    EXIF_IFD_EXIF, EXIF_TAG_DATE_TIME_ORIGINAL)))) {
      entry = m_jpeg_exif->create_tag(EXIF_IFD_EXIF,
                                      EXIF_TAG_DATE_TIME_ORIGINAL,
                                      sizeof(time_string));
    }
    entry->format = EXIF_FORMAT_ASCII;
    memcpy(entry->data, time_string, sizeof(time_string));

    /* MAKE */
    if (AM_UNLIKELY(nullptr == (entry = m_jpeg_exif->is_exif_content_has_entry(
                                EXIF_IFD_0, EXIF_TAG_MAKE)))) {
      entry = m_jpeg_exif->create_tag(EXIF_IFD_0, EXIF_TAG_MAKE,
                                      m_exif_config->make.size() + 1);
    }
    entry->format = EXIF_FORMAT_ASCII;
    memcpy(entry->data, m_exif_config->make.c_str(),
            m_exif_config->make.size() + 1);

    /* MODEL */
    if (AM_UNLIKELY(nullptr == (entry = m_jpeg_exif->is_exif_content_has_entry(
                                EXIF_IFD_0, EXIF_TAG_MODEL)))) {
      entry = m_jpeg_exif->create_tag(EXIF_IFD_0, EXIF_TAG_MODEL,
                                      m_exif_config->model.size() + 1);
    }
    entry->format = EXIF_FORMAT_ASCII;
    memcpy(entry->data, m_exif_config->model.c_str(),
           m_exif_config->model.size() + 1);

    /* IMAGE_DESCRIPTION */
    if (AM_UNLIKELY(nullptr == (entry = m_jpeg_exif->is_exif_content_has_entry(
                                EXIF_IFD_0, EXIF_TAG_IMAGE_DESCRIPTION)))) {
      entry = m_jpeg_exif->create_tag(EXIF_IFD_0, EXIF_TAG_IMAGE_DESCRIPTION,
                                 m_exif_config->image_description.size() + 1);
    }
    entry->format = EXIF_FORMAT_ASCII;
    memcpy(entry->data, m_exif_config->image_description.c_str(),
           m_exif_config->image_description.size() + 1);

    /* ARTIST */
    if (AM_UNLIKELY(nullptr == (entry = m_jpeg_exif->is_exif_content_has_entry(
                                EXIF_IFD_0, EXIF_TAG_ARTIST)))) {
      entry = m_jpeg_exif->create_tag(EXIF_IFD_0, EXIF_TAG_ARTIST,
                                      m_exif_config->artist.size() + 1);
    }
    entry->format = EXIF_FORMAT_ASCII;
    memcpy(entry->data, m_exif_config->artist.c_str(),
           m_exif_config->artist.size() + 1);

    /* METERING_MODE */
    entry = m_jpeg_exif->init_tag(EXIF_IFD_EXIF, EXIF_TAG_METERING_MODE);
    exif_set_short(entry->data, FILE_BYTE_ORDER, m_exif_param.metering_mode);

    /* LIGHT_SOURCE */
    entry = m_jpeg_exif->init_tag(EXIF_IFD_EXIF, EXIF_TAG_LIGHT_SOURCE);
    exif_set_short(entry->data, FILE_BYTE_ORDER, m_exif_param.light_source);

    /* EXIF_TAG_WHITE_BALANCE */
    entry = m_jpeg_exif->init_tag(EXIF_IFD_EXIF, EXIF_TAG_WHITE_BALANCE);
    exif_set_short(entry->data, FILE_BYTE_ORDER, m_exif_param.white_balance);

#if 0
    //GPS
    if (AM_UNLIKELY(nullptr == (entry = m_jpeg_exif->is_exif_content_has_entry(
                             EXIF_IFD_GPS, (ExifTag)EXIF_TAG_GPS_LATITUDE)))) {
      entry = m_jpeg_exif->create_tag(EXIF_IFD_GPS,
                                      (ExifTag)EXIF_TAG_GPS_LATITUDE, 24);
    }
    // Set the field's format and number of components
    entry->format = EXIF_FORMAT_RATIONAL;
    entry->components = 3;
    // Degrees
    float lat = 52.977900;
    ExifRational gps_rational = {(unsigned)(lat * 1000000.0), 1000000 };
    exif_set_rational(entry->data, EXIF_BYTE_ORDER_INTEL, gps_rational);
    exif_set_rational(entry->data+8, EXIF_BYTE_ORDER_INTEL, gps_rational);
    exif_set_rational(entry->data+16, EXIF_BYTE_ORDER_INTEL, gps_rational);
#endif

    exif_data_save_data(m_jpeg_exif->get_exif_data(), &exif_data,
                        &exif_data_len);
    uint8_t exif_len[2] = {uint8_t((exif_data_len + 2) >> 8),
                           uint8_t((exif_data_len + 2) & 0xff)};

    len = sizeof(exif_header) + exif_data_len + packet->get_data_size();
    data = new uint8_t[len];
    if (AM_UNLIKELY(data == nullptr)) {
      ERROR("Memory create error!");
      state = AM_STATE_ERROR;
      break;
    }

    /* Write EXIF header */
    memcpy(data, exif_header, sizeof(exif_header));
    /* Write EXIF block length in big-endian order */
    memcpy(data + sizeof(exif_header), exif_len, 2);
    /* Write EXIF data block */
    memcpy(data + sizeof(exif_header) + 2, exif_data, exif_data_len);
    /* Write JPEG image data, skipping the non-EXIF header */
    memcpy(data + sizeof(exif_header) + 2 + exif_data_len,
           (packet->get_data_ptr() + 2), packet->get_data_size() - 2);

    free(exif_data);

  }while(0);

  return state;
}

AM_STATE AMMuxerJpegBase::on_data_packet(AMPacket* packet)
{
  AM_STATE ret = AM_STATE_OK;
  do{
    if(packet->get_attr() == AMPacket::AM_PAYLOAD_ATTR_VIDEO) {
      if((m_recording_duration > 0) && (m_stop_recording_pts == 0)) {
        m_stop_recording_pts = packet->get_pts() +
            m_recording_duration * 90000;
      }
      if ((m_recording_duration > 0) && (packet->get_pts() > m_stop_recording_pts)) {
        NOTICE("Packet pts reach the value of stop recording pts, stop file writing");
        m_file_writing = false;
        m_stop_recording_pts = 0;
        m_file_counter = 0;
        break;
      }
      uint8_t *exif_data = nullptr;
      uint32_t data_len = 0;
      if(AM_UNLIKELY(create_exif_data_packet(exif_data, data_len, packet)
                     != AM_STATE_OK)) {
        ERROR("Failed to create exif data packet!");
        ret = AM_STATE_ERROR;
        break;
      }

      if(AM_UNLIKELY(m_file_writer->write_data(exif_data, data_len)
                     != AM_STATE_OK)) {
        ERROR("Failed to write data to jpeg file.");
        ret = AM_STATE_ERROR;
        break;
      } else {
        if (m_exif_config->enable) {
          delete [] exif_data;
        }
      }
    } else {
      NOTICE("Jpeg muxer just support video stream.");
      ret = AM_STATE_ERROR;
      break;
    }
    ++ m_file_counter;
    if ((m_recording_file_num > 0) && (m_file_counter >= m_recording_file_num)) {
      NOTICE("File counter reach the value of recording file num, stop file writing");
      m_file_writing = false;
      m_file_counter = 0;
      m_stop_recording_pts = 0;
      break;
    }
  }while(0);
  return ret;
}

AM_STATE AMMuxerJpegBase::on_eos_packet(AMPacket* packet)
{
  AM_STATE ret = AM_STATE_OK;
  m_run = false;
  NOTICE("Receive eos packet in %s, exit the main loop", m_muxer_name.c_str());
  return ret;
}

AM_STATE AMMuxerJpegBase::on_info_packet(AMPacket* packet)
{
  AM_STATE ret = AM_STATE_OK;
  NOTICE("Receive info packet in %s", m_muxer_name.c_str());
  do {
    AM_VIDEO_INFO* video_info = (AM_VIDEO_INFO*) (packet->get_data_ptr());
    if (!video_info) {
      ERROR("%s received video info is null", m_muxer_name.c_str());
      ret = AM_STATE_ERROR;
    }
    INFO("\n%s receive INFO:\n"
        "stream_id: %u\n"
        "     size: %d x %d\n"
        "        M: %d\n"
        "        N: %d\n"
        "     rate: %d\n"
        "    scale: %d\n"
        "      fps: %d\n"
        "      mul: %u\n"
        "      div: %u\n",
        m_muxer_name.c_str(),
        video_info->stream_id, video_info->width,
        video_info->height, video_info->M, video_info->N,
        video_info->rate, video_info->scale, video_info->fps,
        video_info->mul, video_info->div);
  } while(0);
  return ret;
}
