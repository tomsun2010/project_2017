/*******************************************************************************
 * am_muxer_mp4_file_writer.cpp
 *
 * History:
 *   2015-1-9 - [ccjing] created file
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
#include "am_log.h"
#include "am_define.h"

#include "am_muxer_codec_info.h"
#include "am_muxer_mp4_file_writer.h"
#include "am_muxer_mp4_config.h"
#include "am_utils.h"
#include "mp4_file_writer.h"
#include "am_file_sink_if.h"
#include "am_file.h"
#include "iso_base_defs.h"

#include <libgen.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <iostream>
#include <fstream>
#include <sys/time.h>

#ifndef EXTRA_NAME_LEN
#define EXTRA_NAME_LEN 128
#endif

AMMp4FileWriter* AMMp4FileWriter::create (
    AMMuxerCodecMp4Config *muxer_mp4_config, AM_VIDEO_INFO *video_info)
{
  AMMp4FileWriter* result = new AMMp4FileWriter();
  if(AM_UNLIKELY(result && (result->init(muxer_mp4_config, video_info)
      != AM_STATE_OK))) {
    delete result;
    result = NULL;
  }
  return result;
}

AMMp4FileWriter::AMMp4FileWriter():
    m_cur_file_offset(0ll),
    m_begin_packet_pts(0ll),
    m_end_packet_pts(0ll),
    m_file_duration(0ll),
    m_muxer_mp4_config(nullptr),
    m_file_name(nullptr),
    m_tmp_file_name(nullptr),
    m_path_name(nullptr),
    m_base_name(nullptr),
    m_file_writer(nullptr),
    m_max_file_num_per_folder(0),
    m_file_target_duration(0),
    m_hls_sequence(0),
    m_file_counter(0),
    m_dir_counter(0)
{
}

AMMp4FileWriter::~AMMp4FileWriter()
{
  std::ofstream file;
  if(m_muxer_mp4_config->muxer_attr == AM_MUXER_FILE_NORMAL) {
    file.open("/etc/oryx/stream/muxer/muxer_file.conf");
    file << "dir_number:" << m_dir_counter << std::endl;
    file << "file_number:" << m_file_counter << std::endl;
    file.close();
  }
  AM_DESTROY(m_file_writer);
  delete[] m_file_name;
  delete[] m_tmp_file_name;
  delete[] m_path_name;
  delete[] m_base_name;
}

void AMMp4FileWriter::destroy()
{
  delete this;
}

AM_STATE AMMp4FileWriter::init(AMMuxerCodecMp4Config *muxer_mp4_config,
                               AM_VIDEO_INFO *video_info)
{
  AM_STATE ret = AM_STATE_OK;
  std::ifstream file;
  std::string read_line;
  do {
    if (AM_UNLIKELY((m_file_writer = Mp4FileWriter::create()) == NULL)) {
      ERROR("Failed to create file writer");
      ret = AM_STATE_ERROR;
      break;
    }
    m_muxer_mp4_config = muxer_mp4_config;
    if (video_info) {
      memcpy(&m_video_info, video_info, sizeof(AM_VIDEO_INFO));
    } else {
      ERROR("The video info is null.");
      ret = AM_STATE_ERROR;
      break;
    }
    if(muxer_mp4_config->max_file_num_per_folder > 0) {
      m_max_file_num_per_folder = muxer_mp4_config->max_file_num_per_folder;
    } else {
      WARN("max file number is invalid, set it 512 as default");
      m_max_file_num_per_folder = 512;
    }
    if(muxer_mp4_config->file_duration > 0) {
      m_file_target_duration = muxer_mp4_config->file_duration;
    } else {
      WARN("file target duration is invalid, set it 300 as default");
      m_file_target_duration = 300;
    }
    if(muxer_mp4_config->muxer_attr == AM_MUXER_FILE_NORMAL) {
      file.open("/etc/oryx/stream/muxer/muxer_file.conf");
      if(getline(file, read_line)) {
        size_t find_str_position = 0;
        std::string find_str = "dir_number:";
        char* end_ptr = nullptr;
        if((find_str_position = read_line.find(find_str))
            != std::string::npos) {
          m_dir_counter = strtol(
              &(read_line.c_str()[find_str_position + 11]),
              &end_ptr, 10);
          INFO("m dir counter is %d", m_dir_counter);
        }
      }
      if(getline(file, read_line)) {
        size_t find_str_position = 0;
        std::string find_str = "file_number:";
        char* end_ptr = nullptr;
        if((find_str_position = read_line.find(find_str))
            != std::string::npos) {
          m_file_counter = strtol(
              &(read_line.c_str()[find_str_position + 12]),
              &end_ptr, 10);
          INFO("m file counter is %d", m_file_counter);
        }
      }
      file.close();
    }
    if(m_dir_counter < 0) {
      m_dir_counter = 0;
    }
    if(m_file_counter < 0) {
      m_file_counter = 0;
    }
  } while (0);

  return ret;
}

bool AMMp4FileWriter::set_video_info(AM_VIDEO_INFO *vinfo)
{
  bool ret = true;
  if (vinfo) {
    memcpy(&m_video_info, vinfo, sizeof(AM_VIDEO_INFO));
  } else {
    ERROR("The video info is null.");
    ret = false;
  }
  return ret;
}

AM_STATE AMMp4FileWriter::set_media_sink(const char* file_name)
{
  char *copy_name = NULL;
  char *str = NULL;
  AM_STATE ret = AM_STATE_OK;
  do {
    if (AM_UNLIKELY(file_name == NULL)) {
      ERROR("File's name should not be empty.\n");
      ret = AM_STATE_ERROR;
      break;
    }

    INFO("file_name = %s\n", file_name);

    delete[] m_file_name;
    if (AM_UNLIKELY(NULL == (m_file_name = amstrdup(file_name)))) {
      ERROR("Failed to duplicate file name string!");
      ret = AM_STATE_NO_MEMORY;
      break;
    }
    delete[] m_tmp_file_name;
    if (AM_UNLIKELY((m_tmp_file_name = new char [strlen (file_name) +
                                            EXTRA_NAME_LEN]) == NULL)) {
      ERROR("Failed to allocate memory for tmp name.\n");
      ret = AM_STATE_NO_MEMORY;
      break;
    }
    memset(m_tmp_file_name, 0, strlen(file_name) + EXTRA_NAME_LEN);

    /* Allocate memory for copyName */
    if (AM_UNLIKELY((copy_name = new char [strlen (file_name) +
                                           EXTRA_NAME_LEN]) == NULL)) {
      ERROR("Failed to allocate memory for copy name.\n");
      ret = AM_STATE_NO_MEMORY;
      break;
    }

    /* Allocate memory for mpPathName */
    delete[] m_path_name;
    if (AM_UNLIKELY((m_path_name = new char [strlen (file_name) +
                                             EXTRA_NAME_LEN]) == NULL)) {
      ERROR("Failed to allocate memory for m_path_name.\n");
      ret = AM_STATE_NO_MEMORY;
      break;
    } else {
      /* Fetch path name from file_name. */
      strcpy(copy_name, file_name);
      str = dirname(copy_name);
      strcpy(m_path_name, str);
      INFO("Directory's path: %s", m_path_name);
    }

    /* Allocate memory for mpBaseName */
    delete[] m_base_name;
    if (AM_UNLIKELY((m_base_name = new char [strlen (file_name)]) == NULL)) {
      ERROR("Failed to allocate memory for m_base_name.\n");
      ret = AM_STATE_NO_MEMORY;
      break;
    } else {
      /* Fetch base name from file_name. */
      strcpy(copy_name, file_name);
      str = basename(copy_name);
      strcpy(m_base_name, str);
      INFO("Base name: %s", m_base_name);
    }
  } while (0);

  if (AM_LIKELY(copy_name)) {
    delete[] copy_name;
    copy_name = NULL;
  }
  return ret;
}

AM_STATE AMMp4FileWriter::create_next_file()
{
  AM_STATE ret = AM_STATE_OK;
  INFO("create next split file.");
  char* copy_name = NULL;
  char* dir_name = NULL;
  do {
    if((m_file_counter % m_max_file_num_per_folder) == 0) {
      ++ m_dir_counter;
    }
    if(m_muxer_mp4_config->muxer_attr == AM_MUXER_FILE_NORMAL) {
      snprintf(m_tmp_file_name,
               strlen(m_file_name) + EXTRA_NAME_LEN,
               "%s/%d/%s_%u_%u.mp4",
               m_path_name, m_dir_counter, m_base_name,
               m_dir_counter,
               (m_file_counter ++) % m_max_file_num_per_folder);
    } else if(m_muxer_mp4_config->muxer_attr == AM_MUXER_FILE_EVENT) {
      snprintf(m_tmp_file_name,
               strlen(m_file_name) + EXTRA_NAME_LEN,
               "%s/%s_%u.mp4",
               m_path_name, m_base_name,
               m_file_counter ++);
    } else {
      ERROR("muxer attr error.");
      ret = AM_STATE_ERROR;
      break;
    }
    copy_name = amstrdup(m_tmp_file_name);
    dir_name = dirname(copy_name);
    if (!AMFile::exists(dir_name)) {
      if (!AMFile::create_path(dir_name)) {
        ERROR("Failed to create file path: %s!", dir_name);
        ret = AM_STATE_IO_ERROR;
        break;
      }
    }
    if (AM_UNLIKELY(!m_file_writer->create_file(m_tmp_file_name))) {
      ERROR("Failed to create %s: %s\n", m_tmp_file_name, strerror (errno));
      ret = AM_STATE_IO_ERROR;
      break;
    }
    m_cur_file_offset = 0;
    if (AM_UNLIKELY(!m_file_writer->set_buf(IO_TRANSFER_BLOCK_SIZE))) {
      NOTICE("Failed to set IO buffer.\n");
    }

    INFO("File create: %s\n", m_tmp_file_name);
  } while (0);
  delete[] copy_name;
  return ret;
}

char* AMMp4FileWriter::get_current_file_name()
{
  return m_tmp_file_name;
}

AM_STATE AMMp4FileWriter::create_m3u8_file()
{
  AM_STATE ret = AM_STATE_OK;
  AMIFileWriter *file_writer = NULL;
  do {
    if(AM_UNLIKELY((file_writer = AMIFileWriter::create())
                   == NULL)) {
      ERROR("Failed to create file writer.");
      ret = AM_STATE_IO_ERROR;
      break;
    }
    char m3u8_name[512] = { 0 };
    if(m_muxer_mp4_config->muxer_attr == AM_MUXER_FILE_NORMAL) {
      snprintf(m3u8_name, 512, "%s/%d/%s_%u_%u.m3u8",
               m_path_name,m_dir_counter, m_base_name,
               m_dir_counter,
               (m_file_counter -1) % m_max_file_num_per_folder);
    } else if(m_muxer_mp4_config->muxer_attr == AM_MUXER_FILE_EVENT) {
      snprintf(m3u8_name, 512, "%s/%s_%u.m3u8",
               m_path_name, m_base_name,
               (m_file_counter -1) % m_max_file_num_per_folder);
    } else {
      ERROR("mp4 muxer attr error.");
      ret = AM_STATE_ERROR;
      break;
    }
    if(AM_UNLIKELY(!file_writer->create_file(m3u8_name))) {
      ERROR("Failed to create file for m3u8 file.");
      ret = AM_STATE_IO_ERROR;
      break;
    }
    m_file_duration = (m_end_packet_pts - m_begin_packet_pts) /
        (270000000LL
         / ((int64_t) (100ULL * m_video_info.scale / m_video_info.rate)));
    if(m_file_duration < 0) {
      ERROR("Mp4 file duration error, pts is not correct?");
      m_file_duration = (int64_t)m_muxer_mp4_config->file_duration;
    }
    char mp4_name[512] = { 0 };
    char file_content[512] = { 0 };
    if(m_muxer_mp4_config->muxer_attr == AM_MUXER_FILE_NORMAL) {
      snprintf(mp4_name, 512, "%d/%s_%u_%u.mp4",
               m_dir_counter, m_base_name, m_dir_counter,
               (m_file_counter -1) % m_max_file_num_per_folder);
    } else if(m_muxer_mp4_config->muxer_attr == AM_MUXER_FILE_EVENT) {
      snprintf(mp4_name, 512, "event/%s_%u.mp4",
                m_base_name,
               (m_file_counter -1) % m_max_file_num_per_folder);
    } else {
      ERROR("mp4 muxer attr error.");
      ret = AM_STATE_ERROR;
      break;
    }
    snprintf(file_content, 512, "#EXTM3U\n"
             "#EXT-X-ALLOW-CACHE:NO\n"
             "#EXT-X-TARGETDURATION:%u\n"
             "#EXT-X-MEDIA-SEQUENCE:%u\n\n"
             "#EXTINF:%lld,\n"
             "media/media_file/%s\n"
             "#EXT-X-ENDLIST",
             m_file_target_duration + 20,
             m_hls_sequence,
             m_file_duration,
             mp4_name);
    if(AM_UNLIKELY(!file_writer->write_file(file_content,
                           strlen(file_content)))) {
      ERROR("Failed to write file to m3u8 file.");
      ret = AM_STATE_IO_ERROR;
      break;
    }
    char link_name[512] = { 0 };
    char temp_name[512] = { 0 };
    if(m_muxer_mp4_config->muxer_attr == AM_MUXER_FILE_NORMAL) {
      snprintf(temp_name, 512, "%s/%d/%s_%u_%u.m3u8",
               m_path_name, m_dir_counter, m_base_name,
               m_dir_counter,
               (m_file_counter -1) % m_max_file_num_per_folder);
    } else if(m_muxer_mp4_config->muxer_attr == AM_MUXER_FILE_EVENT) {
      snprintf(temp_name, 512, "%s/%s_%u.m3u8",
               m_path_name, m_base_name,
               (m_file_counter -1) % m_max_file_num_per_folder);
    } else {
      ERROR("mp4 muxer attr error.");
      ret = AM_STATE_ERROR;
      break;
    }
    snprintf(link_name, 512, "/webSvr/web/%s_%u_%u.m3u8",
             m_base_name,m_dir_counter,
             (m_file_counter - 1) % m_max_file_num_per_folder);
    if(AM_UNLIKELY((symlink(temp_name, link_name)
        < 0) && (errno != EEXIST))) {
      PERROR("Failed to symlink hls media location.");
      break;
    }
  } while(0);
  if(file_writer) {
    file_writer->close_file();
    file_writer->destroy();
  }
  return ret;
}

AM_STATE AMMp4FileWriter::write_data(uint8_t* data, uint32_t data_len)
{
  AM_STATE ret = AM_STATE_OK;
  do {
    if (AM_UNLIKELY(data == NULL || data_len == 0)) {
      ERROR("Invalid parameter: %u bytes data at %p.", data_len, data);
      break;
    }
    if(AM_UNLIKELY(m_file_writer == NULL)) {
      ERROR("File writer was not initialized.");
      ret = AM_STATE_ERROR;
      break;
    }
    if(AM_UNLIKELY(m_file_writer->write_data(data, data_len) != true)) {
      ERROR("Failed to write data to file");
      ret = AM_STATE_IO_ERROR;
      break;
    }
    m_cur_file_offset += data_len;
  } while (0);
  return ret;
}

AM_STATE AMMp4FileWriter::write_data_direct(uint8_t *data, uint32_t data_len)
{
  AM_STATE state = AM_STATE_IO_ERROR;
  if (AM_LIKELY(AM_STATE_OK == write_data(data, data_len))) {
    m_cur_file_offset += data_len;
    state = m_file_writer->flush_file() ? AM_STATE_OK : AM_STATE_IO_ERROR;
  }

  return state;
}

AM_STATE AMMp4FileWriter::seek_data(uint32_t offset, uint32_t whence)
{
  AM_STATE ret = AM_STATE_OK;
  do {
    if (AM_UNLIKELY(m_file_writer->seek_file(offset, whence) != true)) {
      ERROR("Failed to seek file");
      ret = AM_STATE_IO_ERROR;
      break;
    }
    if (!m_file_writer->tell_file(m_cur_file_offset)) {
      ERROR("Failed to tell file");
      ret = AM_STATE_IO_ERROR;
      break;
    }
  } while (0);
  return ret;
}

bool AMMp4FileWriter::is_file_open()
{
  return m_file_writer->is_file_open();
}

AM_STATE AMMp4FileWriter::write_file_type_box(FileTypeBox& box)
{
  AM_STATE ret = AM_STATE_OK;
  do {
    if (!box.write_file_type_box(m_file_writer)) {
      ERROR("Failed to write file type box.");
      ret = AM_STATE_IO_ERROR;
      break;
    }
    m_cur_file_offset += box.m_base_box.m_size;
  } while(0);
  return ret;
}

AM_STATE AMMp4FileWriter::write_media_data_box(MediaDataBox& box)
{
  AM_STATE ret = AM_STATE_OK;
  do {
    if (!box.write_media_data_box(m_file_writer)) {
      ERROR("Failed to write media data box.");
      ret = AM_STATE_IO_ERROR;
      break;
    }
    m_cur_file_offset += DISOM_BASE_BOX_SIZE;
  } while(0);
  return ret;
}

AM_STATE AMMp4FileWriter::write_movie_box(MovieBox& box)
{
  AM_STATE ret = AM_STATE_OK;
  do {
    if (!box.write_movie_box(m_file_writer)) {
      ERROR("Failed to write movie box.");
      ret = AM_STATE_IO_ERROR;
      break;
    }
    m_cur_file_offset += box.m_base_box.m_size;
  } while(0);
  return ret;
}

AM_STATE AMMp4FileWriter::write_free_box(FreeBox& box)
{
  AM_STATE ret = AM_STATE_OK;
  do {
    if (!box.write_free_box(m_file_writer)) {
      ERROR("Failed to write free box.");
      ret = AM_STATE_IO_ERROR;
      break;
    }
    m_cur_file_offset += box.m_base_box.m_size;
  } while(0);
  return ret;
}

AM_STATE AMMp4FileWriter::write_u8(uint8_t value)
{
  AM_STATE ret = AM_STATE_OK;
  ++ m_cur_file_offset;
  if (!m_file_writer->write_be_u8(value)) {
    ERROR("Failed to write u8");
    ret = AM_STATE_IO_ERROR;
  }
  return ret;
}

AM_STATE AMMp4FileWriter::write_s8(int8_t value)
{
  AM_STATE ret = AM_STATE_OK;
  ++ m_cur_file_offset;
  if (!m_file_writer->write_be_s8(value)) {
    ERROR("Failed to write be s8");
    ret = AM_STATE_IO_ERROR;
  }
  return ret;
}

AM_STATE AMMp4FileWriter::write_be_u16(uint16_t value)
{
  AM_STATE ret = AM_STATE_OK;
  m_cur_file_offset += 2;
  if (!m_file_writer->write_be_u16(value)) {
    ERROR("Failed to write be u16");
    ret = AM_STATE_IO_ERROR;
  }
  return ret;
}

AM_STATE AMMp4FileWriter::write_be_s16(int16_t value)
{
  AM_STATE ret = AM_STATE_OK;
  m_cur_file_offset += 2;
  if (!m_file_writer->write_be_s16(value)) {
    ERROR("Failed to write be s16");
    ret = AM_STATE_IO_ERROR;
  }
  return ret;
}

AM_STATE AMMp4FileWriter::write_be_u24(uint32_t value)
{
  AM_STATE ret = AM_STATE_OK;
  m_cur_file_offset += 3;
  if (!m_file_writer->write_be_u24(value)) {
    ERROR("Failed to write be u24");
    ret = AM_STATE_IO_ERROR;
  }
  return ret;
}

AM_STATE AMMp4FileWriter::write_be_s24(int32_t value)
{
  AM_STATE ret = AM_STATE_OK;
  m_cur_file_offset += 3;
  if (!m_file_writer->write_be_s24(value)) {
    ERROR("Failed to write be s24");
    ret = AM_STATE_IO_ERROR;
  }
  return ret;
}

AM_STATE AMMp4FileWriter::write_be_u32(uint32_t value)
{
  AM_STATE ret = AM_STATE_OK;
  m_cur_file_offset += 4;
  if (!m_file_writer->write_be_u32(value)) {
    ERROR("Failed to write be u32");
    ret = AM_STATE_IO_ERROR;
  }
  return ret;
}

AM_STATE AMMp4FileWriter::write_be_s32(int32_t value)
{
  AM_STATE ret = AM_STATE_OK;
  m_cur_file_offset += 4;
  if (!m_file_writer->write_be_s32(value)) {
    ERROR("Failed to write be s32");
    ret = AM_STATE_IO_ERROR;
  }
  return ret;
}

AM_STATE AMMp4FileWriter::write_be_u64(uint64_t value)
{
  AM_STATE ret = AM_STATE_OK;
  m_cur_file_offset += 8;
  if (!m_file_writer->write_be_u64(value)) {
    ERROR("Failed to write be u64");
    ret = AM_STATE_IO_ERROR;
  }
  return ret;
}

AM_STATE AMMp4FileWriter::write_be_s64(int64_t value)
{
  AM_STATE ret = AM_STATE_OK;
  m_cur_file_offset += 8;
  if (!m_file_writer->write_be_s64(value)) {
    ERROR("Failed to write be s64");
    ret = AM_STATE_IO_ERROR;
  }
  return ret;
}

uint64_t AMMp4FileWriter::get_file_offset()
{
  return m_cur_file_offset;
}

AM_STATE AMMp4FileWriter::close_file(bool need_sync)
{
  AM_STATE ret = AM_STATE_OK;
  if(AM_LIKELY(m_file_writer)) {
    m_file_writer->close_file(need_sync);
  }
  if(m_muxer_mp4_config->hls_enable) {
    if(AM_UNLIKELY(create_m3u8_file() != AM_STATE_OK)) {
      ERROR("Failed to create m3u8 file.");
      ret = AM_STATE_IO_ERROR;
    }
  }
  return ret;
}

void AMMp4FileWriter::set_begin_packet_pts(int64_t pts)
{
  m_begin_packet_pts = pts;
}

void AMMp4FileWriter::set_end_packet_pts(int64_t pts)
{
  m_end_packet_pts = pts;
}

Mp4FileWriter* AMMp4FileWriter::get_file_writer()
{
  return m_file_writer;
}
