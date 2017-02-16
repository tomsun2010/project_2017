/*******************************************************************************
 * am_muxer_export_uds.cpp
 *
 * History:
 *   2015-01-04 - [Zhi He]      created file
 *   2015-04-01 - [Shupeng Ren] modified file
 *   2016-07-08 - [Guohua Zheng] modified file
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

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>

#include "am_base_include.h"
#include "am_define.h"
#include "am_log.h"
#include "am_file.h"
#include "am_thread.h"
#include "am_event.h"
#include "am_mutex.h"

#include "am_amf_types.h"
#include "am_export_if.h"
#include "am_amf_packet.h"
#include "am_muxer_codec_if.h"
#include "am_audio_define.h"
#include "am_video_types.h"
#include "am_muxer_export_uds.h"

AMIMuxerCodec* am_create_muxer_export_uds()
{
  return AMMuxerExportUDS::create();
}

AMMuxerExportUDS *AMMuxerExportUDS::create()
{
  AMMuxerExportUDS *result = new AMMuxerExportUDS();

  if (result && !result->init()) {
    delete result;
    result = nullptr;
  }

  return result;
}

void AMMuxerExportUDS::destroy()
{
  delete this;
}

bool AMMuxerExportUDS::init()
{
  bool ret = false;

  do {
    if (!(m_thread_wait = AMEvent::create())) {
      ERROR("Failed to create m_thread_wait!");
      break;
    }

    if (pipe(m_control_fd) < 0) {
      ERROR("Failed to create control fd!");
      break;
    }

    if (!(m_accept_thread = AMThread::create("export_thread",
                                             thread_entry,
                                             this))) {
      ERROR("Failed to create accept_thread!");
      break;
    }
    ret = true;
  } while (0);

  return ret;
}

AMMuxerExportUDS::AMMuxerExportUDS()
{}

void AMMuxerExportUDS::clean_resource()
{
  reset_resource();
  if (m_control_fd[0] > 0) {
    close(m_control_fd[0]);
    m_control_fd[0] = -1;
  }
  if (m_control_fd[1] > 0) {
    close(m_control_fd[1]);
    m_control_fd[1] = -1;
  }
  m_thread_exit = true;
  m_thread_wait->signal();
  AM_DESTROY(m_accept_thread);
  AM_DESTROY(m_thread_wait);
}

void AMMuxerExportUDS::reset_resource()
{
  if (m_connect_fd > 0) {
    close(m_connect_fd);
    m_connect_fd = -1;
  }

  if (m_socket_fd > 0) {
    close(m_socket_fd);
    m_socket_fd = -1;
    unlink(DEXPORT_PATH);
  }

  m_audio_last_pts.clear();
  m_audio_state.clear();
  m_video_infos.clear();
  m_audio_infos.clear();
  m_video_export_infos.clear();
  m_audio_export_infos.clear();
  m_video_export_packets.clear();
  m_audio_export_packets.clear();
  m_video_info_send_flag.clear();
  m_audio_info_send_flag.clear();
  while (!m_packet_queue.empty()) {
    m_packet_queue.front()->release();
    m_packet_queue.pop();
  }
  for (auto &m : m_video_queue) {
    while (!m.second.empty()) {
      m.second.front()->release();
      m.second.pop();
    }
  }
  m_video_queue.clear();
  for (auto &m : m_audio_queue) {
    while (!m.second.empty()) {
      m.second.front()->release();
      m.second.pop();
    }
  }
  m_audio_queue.clear();
  m_video_map = 0;
  m_audio_map = 0;
  m_send_block = false;
  m_video_send_block.second = false;
  m_audio_send_block.second = false;
  m_send_cond.notify_all();
}

AMMuxerExportUDS::~AMMuxerExportUDS()
{
  clean_resource();
}

AM_STATE AMMuxerExportUDS::start()
{
  AM_STATE ret_state = AM_STATE_OK;

  do {
    INFO("ExportUDS start!");
    FD_ZERO(&m_all_set);
    FD_ZERO(&m_read_set);
    FD_SET(m_control_fd[0], &m_all_set);

    if ((m_socket_fd = socket(PF_UNIX, SOCK_SEQPACKET, 0)) < 0) {
      ERROR("Failed to create socket!\n");
      ret_state = AM_STATE_OS_ERROR;
      break;
    }

    FD_SET(m_socket_fd, &m_all_set);
    m_max_fd = AM_MAX(m_socket_fd, m_control_fd[0]);

    if (AMFile::exists(DEXPORT_PATH)) {
      NOTICE("%s already exists! Remove it first!", DEXPORT_PATH);
      if (unlink(DEXPORT_PATH) < 0) {
        PERROR("unlink");
        ret_state = AM_STATE_OS_ERROR;
        break;
      }
    }

    if (!AMFile::create_path(AMFile::dirname(DEXPORT_PATH).c_str())) {
      ERROR("Failed to create path %s", AMFile::dirname(DEXPORT_PATH).c_str());
      ret_state = AM_STATE_OS_ERROR;
      break;
    }

    memset(&m_addr, 0, sizeof(sockaddr_un));
    m_addr.sun_family = AF_UNIX;
    snprintf(m_addr.sun_path, sizeof(m_addr.sun_path), "%s", DEXPORT_PATH);

    if (bind(m_socket_fd, (sockaddr*)&m_addr, sizeof(sockaddr_un)) < 0) {
      ERROR("bind(%d) failed!", m_socket_fd);
      ret_state = AM_STATE_OS_ERROR;
      break;
    }

    if (listen(m_socket_fd, 5) < 0) {
      ERROR("listen(%d) failed", m_socket_fd);
      ret_state = AM_STATE_OS_ERROR;
      break;
    }

    m_running = true;
    m_client_connected = false;
    m_thread_wait->signal();
  } while(0);

  return ret_state;
}

AM_STATE AMMuxerExportUDS::stop()
{
  NOTICE("AMMuxerExportUDS stop is called!");
  if (m_socket_fd > 0) {
    m_running = false;
    if (m_control_fd[1] > 0) {
      write(m_control_fd[1], "q", 1);
    }
    shutdown(m_socket_fd, SHUT_RD);
    reset_resource();
  }

  return AM_STATE_OK;
}

bool AMMuxerExportUDS::start_file_writing()
{
  return true;
}

bool AMMuxerExportUDS::stop_file_writing()
{
  return true;
}

bool AMMuxerExportUDS::set_recording_file_num(uint32_t file_num)
{
  return true;
}

bool AMMuxerExportUDS::set_recording_duration(int32_t duration)
{
  return true;
}

bool AMMuxerExportUDS::is_running()
{
  return m_running;
}

AM_STATE AMMuxerExportUDS::set_config(AMMuxerCodecConfig *config)
{
  return AM_STATE_OK;
}

AM_STATE AMMuxerExportUDS::get_config(AMMuxerCodecConfig *config)
{
  config->type = AM_MUXER_CODEC_EXPORT;
  return AM_STATE_OK;
}

AM_MUXER_ATTR AMMuxerExportUDS::get_muxer_attr()
{
  return AM_MUXER_EXPORT_NORMAL;
}

uint8_t AMMuxerExportUDS::get_muxer_codec_stream_id()
{
  return 0x0F;
}

uint32_t AMMuxerExportUDS::get_muxer_id()
{
  return 31;// There is no config file in export muxer, so set 31 as default.
}

AM_MUXER_CODEC_STATE AMMuxerExportUDS::get_state()
{
  return m_running ? AM_MUXER_CODEC_RUNNING : AM_MUXER_CODEC_STOPPED;
}

void AMMuxerExportUDS::feed_data(AMPacket *packet)
{
  if (!packet) {return;}
  uint32_t stream_id = packet->get_stream_id();
  switch (packet->get_type()) {
    case AMPacket::AM_PAYLOAD_TYPE_INFO: {
      save_info(packet);
    } break;
    case AMPacket::AM_PAYLOAD_TYPE_DATA:
    case AMPacket::AM_PAYLOAD_TYPE_EOS: {
      if (!m_client_connected) {break;}
      if (!m_config.need_sort) {
        std::unique_lock<std::mutex> lk(m_send_mutex);
        if (m_packet_queue.size() < 32) {
          packet->add_ref();
          m_packet_queue.push(packet);
          if (m_send_block) {m_send_cond.notify_one();}
        } else {
          NOTICE("Too much data in queue, drop data!");
        }
      } else {
        switch(packet->get_attr()) {
          case AMPacket::AM_PAYLOAD_ATTR_VIDEO: {
            std::unique_lock<std::mutex> lk(m_send_mutex);
            if ((m_video_queue.find(stream_id) == m_video_queue.end()) ||
                (m_video_queue[stream_id].size() < 32)) {
              packet->add_ref();
              m_video_queue[stream_id].push(packet);
              if (m_video_send_block.second &&
                  (m_video_send_block.first == stream_id)) {
                m_send_cond.notify_one();
              }
            } else {
              ERROR("Too much data in Video[%u] queue, drop data!", stream_id);
            }
          }break;
          case AMPacket::AM_PAYLOAD_ATTR_AUDIO: {
            std::unique_lock<std::mutex> lk(m_send_mutex);
            if ((m_audio_queue.find(stream_id) == m_audio_queue.end()) ||
                (m_audio_queue[stream_id].size() < 32)) {
              packet->add_ref();
              m_audio_queue[stream_id].push(packet);
              if (m_audio_send_block.second &&
                  (m_audio_send_block.first == stream_id)) {
                m_send_cond.notify_one();
              }
            } else {
              ERROR("Too much data in Audio[%u] queue, drop data!", stream_id);
            }
          }break;
          default: {
            WARN("Current packet with attribute %d is not handled!",
                 packet->get_attr());
          }break;
        }
      }
    }break;
    default: break;
  }
  packet->release();
}

void AMMuxerExportUDS::thread_entry(void *arg)
{
  if (!arg) {
    ERROR("Thread argument is null!");
    return;
  }
  AMMuxerExportUDS *p = (AMMuxerExportUDS*)arg;
  do {
    p->m_thread_wait->wait();
    p->main_loop();
  } while (!p->m_thread_exit);
}

void AMMuxerExportUDS::main_loop()
{
  int ret = 0;
  m_export_state = EExportState_no_client_connected;

  while (m_running) {
    switch (m_export_state) {
      case EExportState_no_client_connected: {
        m_read_set = m_all_set;
        if ((ret = select(m_max_fd + 1, &m_read_set,
                          nullptr, nullptr, nullptr)) < 0) {
          FD_CLR(m_socket_fd, &m_all_set);
          m_export_state = EExportState_error;
        } else if (ret == 0) {
          WARN("Select timeout!"); /* Impossible to be here */
        } else {
          if (FD_ISSET(m_control_fd[0], &m_read_set)) {
            char read_char;
            read(m_control_fd[0], &read_char, 1);
          } else if (FD_ISSET(m_socket_fd, &m_read_set)) {
            socklen_t addr_len;
            if (m_connect_fd > 0) {
              close(m_connect_fd);
              m_connect_fd = -1;
            }
            if ((m_connect_fd = accept(m_socket_fd,
                                       (sockaddr*)&m_addr,
                                       &addr_len)) < 0) {
              NOTICE("muxer-export will exit!");
              m_export_state = EExportState_halt;
            } else {
              NOTICE("Client connected, fd %d\n", m_connect_fd);
              if (read(m_connect_fd, &m_config, sizeof(m_config)) !=
                  sizeof(m_config)) {
                ERROR("Failed to read config from client!");
              } else {
                m_client_connected = true;
                m_export_state = EExportState_running;
              }
            }
          }
        }
      }break;

      case EExportState_running: {
        if (!send_info(m_connect_fd) || !send_packet(m_connect_fd)) {
          NOTICE("Send failed, client exits!");
          m_client_connected = false;
          m_export_state = EExportState_no_client_connected;

          for (auto &m : m_video_queue) {
            while (!m.second.empty()) {
              m.second.front()->release();
              m.second.pop();
            }
          }

          for (auto &m : m_audio_queue) {
            while (!m.second.empty()) {
              m.second.front()->release();
              m.second.pop();
            }
          }

          while (!m_packet_queue.empty()) {
            m_packet_queue.front()->release();
            m_packet_queue.pop();
          }

          for (auto &m : m_video_info_send_flag) {
            m.second = false;
          }
          for (auto &m : m_audio_info_send_flag) {
            m.second = false;
          }
        }
      }break;

      case EExportState_error:
      case EExportState_halt: break;
      default: break;
    }
  }
  INFO("Export.Main exits mainloop!");
}

void AMMuxerExportUDS::save_info(AMPacket *packet)
{
  uint32_t stream_id = packet->get_stream_id();
  AMExportPacket export_packet = {0};
  export_packet.stream_index = stream_id;
  export_packet.is_direct_mode = 0;

  switch (packet->get_attr()) {
    case AMPacket::AM_PAYLOAD_ATTR_AUDIO: {
      AM_AUDIO_INFO *audio_info = (AM_AUDIO_INFO*)packet->get_data_ptr();
      m_audio_infos[stream_id] = *audio_info;
      AMExportAudioInfo export_audio_info;
      export_audio_info.samplerate = audio_info->sample_rate;
      export_audio_info.channels = audio_info->channels;
      export_audio_info.sample_size = audio_info->sample_size;
      export_audio_info.pts_increment = audio_info->pkt_pts_increment;
      m_audio_pts_increment = audio_info->pkt_pts_increment;
      m_audio_export_infos[stream_id] = export_audio_info;
      export_packet.packet_type = AM_EXPORT_PACKET_TYPE_AUDIO_INFO;
      export_packet.data_size = sizeof(AMExportAudioInfo);
      switch (audio_info->type) {
        case AM_AUDIO_AAC:
          export_packet.packet_format = AM_EXPORT_PACKET_FORMAT_AAC;
          break;
        case AM_AUDIO_OPUS:
          export_packet.packet_format = AM_EXPORT_PACKET_FORMAT_OPUS;
          break;
        case AM_AUDIO_G711A:
          export_packet.packet_format = AM_EXPORT_PACKET_FORMAT_G711ALaw;
          break;
        case AM_AUDIO_G711U:
          export_packet.packet_format = AM_EXPORT_PACKET_FORMAT_G711MuLaw;
          break;
        case AM_AUDIO_G726_16:
          export_packet.packet_format = AM_EXPORT_PACKET_FORMAT_G726_16;
          break;
        case AM_AUDIO_G726_24:
          export_packet.packet_format = AM_EXPORT_PACKET_FORMAT_G726_24;
          break;
        case AM_AUDIO_G726_32:
          export_packet.packet_format = AM_EXPORT_PACKET_FORMAT_G726_32;
          break;
        case AM_AUDIO_G726_40:
          export_packet.packet_format = AM_EXPORT_PACKET_FORMAT_G726_40;
          break;
        case AM_AUDIO_LPCM:
          export_packet.packet_format = AM_EXPORT_PACKET_FORMAT_PCM;
          break;
        case AM_AUDIO_BPCM:
          export_packet.packet_format = AM_EXPORT_PACKET_FORMAT_BPCM;
          break;
        case AM_AUDIO_SPEEX:
          export_packet.packet_format = AM_EXPORT_PACKET_FORMAT_SPEEX;
          break;
        default:
          export_packet.packet_format = AM_EXPORT_PACKET_FORMAT_INVALID;
          break;
      }
      m_audio_export_packets[stream_id] = export_packet;
      m_audio_info_send_flag[stream_id] = false;
      m_audio_map |= 1 << stream_id;
      m_audio_state[stream_id] = 0;
      INFO("Save Audio[%d] INFO: "
          "samplerate: %d, framesize: %d, "
          "bitrate: %d, channels: %d, samplerate: %d",
          stream_id,
          m_audio_export_infos[stream_id].samplerate,
          m_audio_export_infos[stream_id].frame_size,
          m_audio_export_infos[stream_id].bitrate,
          m_audio_export_infos[stream_id].channels,
          m_audio_export_infos[stream_id].samplerate);
    } break;
    case AMPacket::AM_PAYLOAD_ATTR_VIDEO: {
      AM_VIDEO_INFO *video_info = (AM_VIDEO_INFO*)packet->get_data_ptr();
      m_video_infos[stream_id] = *video_info;
      AMExportVideoInfo export_video_info;
      export_video_info.width = video_info->width;
      export_video_info.height = video_info->height;
      export_video_info.framerate_num = video_info->mul;
      export_video_info.framerate_den = video_info->div;
      m_video_export_infos[stream_id] = export_video_info;
      export_packet.packet_type = AM_EXPORT_PACKET_TYPE_VIDEO_INFO;
      export_packet.data_size = sizeof(AMExportVideoInfo);
      switch (video_info->type) {
        case AM_VIDEO_H264:
          export_packet.packet_format = AM_EXPORT_PACKET_FORMAT_AVC;
          break;
        case AM_VIDEO_H265:
          export_packet.packet_format = AM_EXPORT_PACKET_FORMAT_HEVC;
          break;
        case AM_VIDEO_MJPEG:
          export_packet.packet_format = AM_EXPORT_PACKET_FORMAT_MJPEG;
          break;
        default:
          export_packet.packet_format = AM_EXPORT_PACKET_FORMAT_INVALID;
          break;
      }
      m_video_export_packets[stream_id] = export_packet;
      m_video_info_send_flag[stream_id] = false;
      m_video_map |= 1 << stream_id;
      INFO("Save Video[%d] INFO: "
          "width: %d, height: %d, frame factor %d/%d",
          stream_id,
          m_video_export_infos[stream_id].width,
          m_video_export_infos[stream_id].height,
          m_video_export_infos[stream_id].framerate_num,
          m_video_export_infos[stream_id].framerate_den);
    } break;
    default: break;
  }
}

bool AMMuxerExportUDS::send_info(int client_fd)
{
  bool ret = true;

  do {
    for (auto &m : m_video_info_send_flag) {
      if (m.second) {
        continue;
      }

      if (write(client_fd, &m_video_export_packets[m.first],
                sizeof(AMExportPacket)) != sizeof(AMExportPacket)) {
        ERROR("Write header failed, close socket!");
        ret = false;
        break;
      }
      if (write(client_fd, &m_video_export_infos[m.first],
                sizeof(AMExportVideoInfo)) != sizeof(AMExportVideoInfo)) {
        ERROR("Write video info[%d] failed, close socket!", m.first);
        ret = false;
        break;
      }
      m.second = true;
    }
    if (!ret) {break;}

    for (auto &m: m_audio_info_send_flag) {
      if (m.second) {
        continue;
      }

      if (write(client_fd, &m_audio_export_packets[m.first],
                sizeof(AMExportPacket)) != sizeof(AMExportPacket)) {
        ERROR("Write header failed, close socket!");
        ret = false;
        break;
      }
      if (write(client_fd, &m_audio_export_infos[m.first],
                sizeof(AMExportAudioInfo)) != sizeof(AMExportAudioInfo)) {
        ERROR("Write audio info[%d] failed, close socket!", m.first);
        ret = false;
        break;
      }
      m.second = true;
    }
    if (!ret) {break;}
  } while (0);

  return ret;
}

bool AMMuxerExportUDS::send_packet(int client_fd)
{
  bool           ret           = true;
  AM_PTS         min_pts       = INT64_MAX;
  AMPacket      *am_packet     = nullptr;
  AMExportPacket export_packet = {0};

  do {
    if (m_config.need_sort) {
      std::unique_lock<std::mutex> lk(m_send_mutex);
      for (auto &m : m_audio_queue) {
        if (!(m_audio_map & (1 << m.first))) {continue;}
        if (m.second.empty()) {
          switch (m_audio_state[m.first]) {
            case 0: //Normal
              m_audio_state[m.first] = 1;
              m_audio_last_pts[m.first] += m_audio_pts_increment - 100;
              break;
            case 1: //Try to get audio
              break;
            case 2: //Need block to wait audio packet
              while (m_running && m.second.empty()) {
                m_audio_send_block.first = m.first;
                m_audio_send_block.second = true;
                m_send_cond.wait(lk);
                m_audio_send_block.second = false;
              }
              m_audio_last_pts[m.first] = m.second.front()->get_pts();
              m_audio_state[m.first] = 0;
              break;
            default: break;
          }
        } else {
          m_audio_last_pts[m.first] = m.second.front()->get_pts();
          if (m_audio_state[m.first]) {
            m_audio_state[m.first] = 0;
          }
        }

        if (!m_running) {break;}
        if (m_audio_last_pts[m.first] < min_pts) {
          min_pts = m_audio_last_pts[m.first];
          am_packet = m.second.front();
        }
      }
      lk.unlock();

      lk.lock();
      for (auto &m : m_video_queue) {
        if (!(m_video_map & (1 << m.first))) {
          continue;
        }

        while (m_running && m.second.empty()) {
          m_video_send_block.first = m.first;
          m_video_send_block.second = true;
          m_send_cond.wait(lk);
          m_video_send_block.second = false;
        }

        if (!m_running) {break;}
        if (m.second.front()->get_pts() < min_pts) {
          min_pts = m.second.front()->get_pts();
          am_packet = m.second.front();
        }
      }

      for (auto &m : m_audio_queue) {
        if ((am_packet == m.second.front()) && m_audio_state[m.first]) {
          m_audio_state[m.first] = 2; //Need block to wait
          return ret;
        }
      }
      lk.unlock();

      lk.lock();
      if (am_packet) {
        if (am_packet->get_attr() == AMPacket::AM_PAYLOAD_ATTR_VIDEO) {
          m_video_queue[am_packet->get_stream_id()].pop();
        } else if (am_packet->get_attr() == AMPacket::AM_PAYLOAD_ATTR_AUDIO) {
          m_audio_queue[am_packet->get_stream_id()].pop();
        }
      }
    } else {
      std::unique_lock<std::mutex> lk(m_send_mutex);
      while (m_running && m_packet_queue.empty()) {
        m_send_block = true;
        m_send_cond.wait(lk);
      }
      m_send_block = false;
      if (m_running && !m_packet_queue.empty()) {
        am_packet = m_packet_queue.front();
        m_packet_queue.pop();
      }
    }

    if (!am_packet) {break;}

    if (am_packet->get_type() == AMPacket::AM_PAYLOAD_TYPE_EOS) {
      uint32_t stream_id = am_packet->get_stream_id();
      switch (am_packet->get_attr()) {
        case AMPacket::AM_PAYLOAD_ATTR_VIDEO:
          m_video_map &= ~(1 << stream_id);
          m_video_infos.erase(stream_id);
          m_video_export_packets.erase(stream_id);
          m_video_info_send_flag.erase(stream_id);
          if (m_config.need_sort) {
            for (auto &m : m_video_queue) {
              while (!m.second.empty()) {
                m.second.front()->release();
                m.second.pop();
              }
              m_video_queue.erase(m.first);
            }
          }
          break;
        case AMPacket::AM_PAYLOAD_ATTR_AUDIO:
          m_audio_map &= ~(1 << stream_id);
          m_audio_infos.erase(stream_id);
          m_audio_export_packets.erase(stream_id);
          m_audio_info_send_flag.erase(stream_id);
          if (m_config.need_sort) {
            for (auto &m : m_audio_queue) {
              while (!m.second.empty()) {
                m.second.front()->release();
                m.second.pop();
              }
              m_audio_queue.erase(m.first);
            }
          }
          break;
        default:
          break;
      }
    }

    if (fill_export_packet(am_packet, &export_packet)) {
      if (write(client_fd, &export_packet, sizeof(AMExportPacket)) !=
          sizeof(AMExportPacket)) {
        ret = false;
        NOTICE("Write header failed, close socket!");
        am_packet->release();
        am_packet = nullptr;
        break;
      }

      if (!export_packet.is_direct_mode) {
        if ((uint32_t)write(client_fd,
                            export_packet.data_ptr,
                            export_packet.data_size) !=
            export_packet.data_size) {
          ret = false;
          NOTICE("Write data(%d) failed, close socket!",
                 export_packet.data_size);
        }
      }
    }
    am_packet->release();
  } while (0);
  return ret;
}

bool AMMuxerExportUDS::fill_export_packet(AMPacket *packet,
                                          AMExportPacket *export_packet)
{
  bool ret = true;
  uint32_t t = 0;
  uint32_t t_sample_rate = 0;
  export_packet->data_ptr = nullptr;
  export_packet->data_size = packet->get_data_size();
  export_packet->pts = packet->get_pts();
  export_packet->stream_index = packet->get_stream_id();
  export_packet->is_key_frame = 0;
  export_packet->is_direct_mode = 1;

  switch (packet->get_attr()) {
    case AMPacket::AM_PAYLOAD_ATTR_VIDEO:
    case AMPacket::AM_PAYLOAD_ATTR_SEI: {
      export_packet->packet_type = AM_EXPORT_PACKET_TYPE_VIDEO_DATA;
      t = packet->get_frame_type();
      switch (t) {
        case AM_VIDEO_FRAME_TYPE_IDR:
          export_packet->frame_type = AM_EXPORT_VIDEO_FRAME_TYPE_IDR;
          switch(AM_VIDEO_TYPE(packet->get_video_type())) {
            case AM_VIDEO_H264:
              export_packet->packet_format = AM_EXPORT_PACKET_FORMAT_AVC;
              break;
            case AM_VIDEO_H265:
              export_packet->packet_format = AM_EXPORT_PACKET_FORMAT_HEVC;
              break;
            default:break;
          }
          export_packet->is_key_frame = 1;
          break;
        case AM_VIDEO_FRAME_TYPE_I:
          export_packet->frame_type = AM_EXPORT_VIDEO_FRAME_TYPE_I;
          switch(AM_VIDEO_TYPE(packet->get_video_type())) {
            case AM_VIDEO_H264:
              export_packet->packet_format = AM_EXPORT_PACKET_FORMAT_AVC;
              break;
            case AM_VIDEO_H265:
              export_packet->packet_format = AM_EXPORT_PACKET_FORMAT_HEVC;
              break;
            default:break;
          }
          break;
        case AM_VIDEO_FRAME_TYPE_P:
          export_packet->frame_type = AM_EXPORT_VIDEO_FRAME_TYPE_P;
          switch(AM_VIDEO_TYPE(packet->get_video_type())) {
            case AM_VIDEO_H264:
              export_packet->packet_format = AM_EXPORT_PACKET_FORMAT_AVC;
              break;
            case AM_VIDEO_H265:
              export_packet->packet_format = AM_EXPORT_PACKET_FORMAT_HEVC;
              break;
            default:break;
          }
          break;
        case AM_VIDEO_FRAME_TYPE_B:
          export_packet->frame_type = AM_EXPORT_VIDEO_FRAME_TYPE_B;
          switch(AM_VIDEO_TYPE(packet->get_video_type())) {
            case AM_VIDEO_H264:
              export_packet->packet_format = AM_EXPORT_PACKET_FORMAT_AVC;
              break;
            case AM_VIDEO_H265:
              export_packet->packet_format = AM_EXPORT_PACKET_FORMAT_HEVC;
              break;
            default:break;
          }
          break;
        case AM_VIDEO_FRAME_TYPE_MJPEG:
        case AM_VIDEO_FRAME_TYPE_SJPEG:
          export_packet->frame_type = AM_EXPORT_VIDEO_FRAME_TYPE_I;
          export_packet->packet_format = AM_EXPORT_PACKET_FORMAT_MJPEG;
          export_packet->is_key_frame = 1;
          break;
        default:
          ERROR("Not supported video frame type %d\n", t);
          ret = false;
          break;
      }
      export_packet->data_ptr = (uint8_t*)packet->get_addr_offset();
      export_packet->is_direct_mode = 1;
    } break;

    case AMPacket::AM_PAYLOAD_ATTR_AUDIO: {
      export_packet->packet_type = AM_EXPORT_PACKET_TYPE_AUDIO_DATA;
      t = packet->get_frame_type();
      t_sample_rate = packet->get_frame_attr();
      switch (t) {
        case AM_AUDIO_G711A:
          export_packet->frame_type = AM_EXPORT_PACKET_FORMAT_G711ALaw;
          export_packet->packet_format = AM_EXPORT_PACKET_FORMAT_G711ALaw;
          export_packet->is_key_frame = 1;
          export_packet->audio_sample_rate = t_sample_rate/1000;
          break;
        case AM_AUDIO_G711U:
          export_packet->frame_type = AM_EXPORT_PACKET_FORMAT_G711MuLaw;
          export_packet->packet_format = AM_EXPORT_PACKET_FORMAT_G711MuLaw;
          export_packet->audio_sample_rate = t_sample_rate/1000;
          break;
        case AM_AUDIO_G726_40:
          export_packet->frame_type = AM_EXPORT_PACKET_FORMAT_G726_40;
          export_packet->packet_format = AM_EXPORT_PACKET_FORMAT_G726_40;
          export_packet->audio_sample_rate = t_sample_rate/1000;
          break;
        case AM_AUDIO_G726_32:
          export_packet->frame_type = AM_EXPORT_PACKET_FORMAT_G726_32;
          export_packet->packet_format = AM_EXPORT_PACKET_FORMAT_G726_32;
          export_packet->audio_sample_rate = t_sample_rate/1000;
          break;
        case AM_AUDIO_G726_24:
          export_packet->frame_type = AM_EXPORT_PACKET_FORMAT_G726_24;
          export_packet->packet_format = AM_EXPORT_PACKET_FORMAT_G726_24;
          export_packet->audio_sample_rate = t_sample_rate/1000;
          break;
        case AM_AUDIO_G726_16:
          export_packet->frame_type = AM_EXPORT_PACKET_FORMAT_G726_16;
          export_packet->packet_format = AM_EXPORT_PACKET_FORMAT_G726_16;
          export_packet->audio_sample_rate = t_sample_rate/1000;
          break;
        case AM_AUDIO_AAC:
          export_packet->frame_type = AM_EXPORT_PACKET_FORMAT_AAC;
          export_packet->packet_format = AM_EXPORT_PACKET_FORMAT_AAC;
          export_packet->audio_sample_rate = t_sample_rate/1000;
          break;
        case AM_AUDIO_OPUS:
          export_packet->frame_type = AM_EXPORT_PACKET_FORMAT_OPUS;
          export_packet->packet_format = AM_EXPORT_PACKET_FORMAT_OPUS;
          export_packet->audio_sample_rate = t_sample_rate/1000;
          break;
        case AM_AUDIO_LPCM:
          export_packet->frame_type = AM_EXPORT_PACKET_FORMAT_PCM;
          export_packet->packet_format = AM_EXPORT_PACKET_FORMAT_PCM;
          export_packet->audio_sample_rate = t_sample_rate/1000;
          break;
        case AM_AUDIO_BPCM:
          export_packet->frame_type = AM_EXPORT_PACKET_FORMAT_BPCM;
          export_packet->packet_format = AM_EXPORT_PACKET_FORMAT_BPCM;
          export_packet->audio_sample_rate = t_sample_rate/1000;
          break;
        case AM_AUDIO_SPEEX:
          export_packet->frame_type = AM_EXPORT_PACKET_FORMAT_SPEEX;
          export_packet->packet_format = AM_EXPORT_PACKET_FORMAT_SPEEX;
          export_packet->audio_sample_rate = t_sample_rate/1000;
          break;
        default:
          ERROR("Not supported audio frame type %d\n", t);
          ret = false;
          break;
      }
      export_packet->is_direct_mode = 0;
      export_packet->data_ptr = packet->get_data_ptr();
    } break;
    default:
      ERROR("Not supported payload attr %d\n", packet->get_attr());
      ret = false;
      break;
  }

  return ret;
}
