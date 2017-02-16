/*******************************************************************************
 * am_avqueue.cpp
 *
 * History:
 *   Sep 21, 2016 - [Shupeng Ren] created file
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

#include "am_log.h"
#include "am_define.h"
#include "am_thread.h"

#include "am_amf_types.h"
#include "am_amf_interface.h"
#include "am_amf_packet.h"
#include "am_amf_queue.h"
#include "am_amf_base.h"
#include "am_amf_packet_pool.h"

#include "am_video_types.h"
#include "am_audio_define.h"
#include "am_muxer_codec_info.h"
#include "am_record_event_sender.h"
#include "am_ring_queue.h"
#include "am_avqueue.h"

#define STREAM_MAX_NUM 4
#define H264_SCALE 90000

using namespace std::placeholders;

static std::mutex m_mutex;

template <typename T>
AMAVQueuePtr AMAVQueue::create(AVQueueConfig &config,
                               recv_cb_t callback, T *context)
{
  AMAVQueue *result = new AMAVQueue();
  if (result && (result->construct(config, callback, context) != AM_STATE_OK)) {
    delete result;
    result = nullptr;
  }
  return AMAVQueuePtr(result, [](AMAVQueue *p){delete p;});
}

template <typename T>
AM_STATE AMAVQueue::construct(AVQueueConfig &config,
                              recv_cb_t callback, T *context)
{
  AM_STATE state = AM_STATE_OK;
  do {
    if (context) {
      m_recv_cb = std::bind(T::callback, context, _1);
    } else {
      m_recv_cb = std::bind(callback, _1);
    }

    m_config = config;

    if (!(m_packet_pool = AMAVQueuePacketPool::create(this,
                                                      "AVQueuePacketPool",
                                                      128))) {
      state = AM_STATE_NO_MEMORY;
      ERROR("Failed to create avqueue packet pool!");
      break;
    }

    if (!(m_send_normal_pkt_thread = AMThread::create("send_normal_packet_thread",
                                               static_send_normal_packet_thread,
                                               this))) {
      state = AM_STATE_OS_ERROR;
      ERROR("Failed to create send_normal_packet_thread!");
      break;
    }
    if (!(m_send_event_pkt_thread = AMThread::create("send_event_packet_thread",
                                              static_send_event_packet_thread,
                                              this))) {
      state = AM_STATE_OS_ERROR;
      ERROR("Failed to create send_event_packet_thread!");
      break;
    }
  } while (0);
  return state;
}

AM_STATE AMAVQueue::process_packet(AMPacket *packet)
{
  AM_STATE state = AM_STATE_OK;

  if (!m_stop.load()) {
    switch (packet->get_type()) {
      case AMPacket::AM_PAYLOAD_TYPE_INFO: {
        state = on_info(packet);
      } break;
      case AMPacket::AM_PAYLOAD_TYPE_DATA: {
        state = on_data(packet);
      } break;
      case AMPacket::AM_PAYLOAD_TYPE_EVENT: {
        state = on_event(packet);
      } break;
      case AMPacket::AM_PAYLOAD_TYPE_EOS: {
        state = on_eos(packet);
      } break;
      default:
        break;
    }
  }
  packet->release();
  return state;
}

void AMAVQueue::release_packet(AMPacket *packet)
{
  uint32_t stream_id = packet->get_stream_id();
  switch (packet->get_attr()) {
    case AMPacket::AM_PAYLOAD_ATTR_VIDEO: {
      if (packet->get_packet_type() & AMPacket::AM_PACKET_TYPE_EVENT) {
        m_video_queue[stream_id]->event_release((ExPayload*)packet->get_payload());
      } else {
        m_video_queue[stream_id]->normal_release((ExPayload*)packet->get_payload());
      }
    } break;

    case AMPacket::AM_PAYLOAD_ATTR_AUDIO: {
      if (packet->get_packet_type() & AMPacket::AM_PACKET_TYPE_EVENT) {
        m_audio_queue[stream_id]->event_release((ExPayload*)packet->get_payload());
      } else {
        m_audio_queue[stream_id]->normal_release((ExPayload*)packet->get_payload());
      }
    } break;
    default: break;
  }
}

void AMAVQueue::static_send_normal_packet_thread(void *p)
{
  ((AMAVQueue*)p)->send_normal_packet_thread();
}

void AMAVQueue::static_send_event_packet_thread(void *p)
{
  ((AMAVQueue*)p)->send_event_packet_thread();
}

void AMAVQueue::send_normal_packet_thread()
{
  while (true) {
    //Send normal packets
    static uint32_t count = 0;
    if (!m_stop.load()) {
      if (send_normal_packet() != AM_STATE_OK) {
        break;
      }
    } else {
      uint32_t video_count = 0;
      uint32_t audio_count = 0;
      for (auto &m : m_video_queue) {
        if (!m.second->in_use()) {
          ++video_count;
        }
      }
      for (auto &m : m_audio_queue) {
        if (!m.second->in_use()) {
          ++audio_count;
        }
      }
      if (video_count == m_video_queue.size() &&
          audio_count == m_audio_queue.size()) {
        break;
      } else {
        usleep(50000);
        if (++ count > 20) {
          NOTICE("Count is bigger than 20, exit");
          break;
        }
      }
    }
  };
  INFO("%s exits!", m_send_normal_pkt_thread->name());
}

void AMAVQueue::send_event_packet_thread()
{
  while (true) {
    //Send event packets
    if (m_in_event.load()) {
      if (send_event_packet() != AM_STATE_OK) {
        break;
      }
      if (m_event_video_eos || m_stop.load()) {
        m_in_event = false;
        m_event_video_eos = false;
      }
    }
  };
  INFO("%s exits!", m_send_event_pkt_thread->name());
}

AM_STATE AMAVQueue::send_normal_packet()
{
  AM_STATE       state          = AM_STATE_OK;
  AM_PTS         min_pts        = INT64_MAX;//(0x7fffffffffffffffLL)
  AMPacket      *send_packet    = nullptr;
  AMRingQueue   *queue_ptr      = nullptr;
  AMPacket::AM_PAYLOAD_TYPE payload_type;
  AMPacket::AM_PAYLOAD_ATTR payload_attr;
  uint32_t stream_id;
  if (m_video_come.load()) {
    /* compare every video and audio packets,
     * find a minimum PTS packet to send
     */
    for (auto &m : m_audio_queue) {
      if (!m_first_audio[m.first].first || !m.second) {
        continue;
      }
      if (!(m_run & (1 << (m.first + STREAM_MAX_NUM)))) {
        continue;
      }
      {
        std::unique_lock<std::mutex> lk(m_send_mutex);
        if (m.second->empty(queue_mode::normal)) {
          switch (m_send_audio_state[m.first]) {
            case 0: //Normal
              m_send_audio_state[m.first] = 1;
              m_audio_last_pts[m.first] += m_audio_pts_increment - 100;
              break;
            case 1: //Try to get audio
              break;
            case 2: //Need block to wait audio packet
              while (!m_stop && m.second->empty(queue_mode::normal)) {
                m_audio_send_block.first = m.first;
                m_audio_send_block.second = true;
                m_send_cond.wait(lk);
                m_audio_send_block.second = false;
              }
              m_audio_last_pts[m.first] = m.second->get()->m_data.m_payload_pts;
              m_send_audio_state[m.first] = 0;
              break;
            default:
              break;
          }
        } else {
          m_audio_last_pts[m.first] = m.second->get()->m_data.m_payload_pts;
          if (m_send_audio_state[m.first]) {
            m_send_audio_state[m.first] = 0;
          }
        }
      }
      if (m_stop) {
        return state;
      }

      if (m_audio_last_pts[m.first] < min_pts) {
        min_pts = m_audio_last_pts[m.first];
        queue_ptr = m.second;
      }
    }
    for (auto &m : m_video_queue) {
      if (!m_first_video[m.first].first || !m.second) {
        continue;
      }
      if (!(m_run & (1 << m.first))) {
        continue;
      }
      {
        std::unique_lock<std::mutex> lk(m_send_mutex);
        while (!m_stop && m.second->empty(queue_mode::normal)) {
          m_video_send_block.first = m.first;
          m_video_send_block.second = true;
          m_send_cond.wait(lk);
          m_video_send_block.second = false;
        }
      }
      if (m_stop) {
        return state;
      }
      if (m.second->get()->m_data.m_payload_pts < min_pts) {
        min_pts = m.second->get()->m_data.m_payload_pts;
        queue_ptr = m.second;
      }
    }
    for (auto &m : m_audio_queue) {
      if (queue_ptr == m.second && m_send_audio_state[m.first]) {
        m_send_audio_state[m.first] = 2; //Need block to wait
        return state;
      }
    }
    if (queue_ptr) {
      if (!m_packet_pool->alloc_packet(send_packet)) {
        WARN("Failed to allocate packet!");
        return AM_STATE_ERROR;
      }
      send_packet->set_payload(queue_ptr->get());
      payload_type = send_packet->get_type();
      payload_attr = send_packet->get_attr();
      stream_id = send_packet->get_stream_id();
      if (payload_type == AMPacket::AM_PAYLOAD_TYPE_EOS) {
        switch (payload_attr) {
          case AMPacket::AM_PAYLOAD_ATTR_VIDEO:
            m_run &= ~(1 << stream_id);
            break;
          case AMPacket::AM_PAYLOAD_ATTR_AUDIO:
            m_run &= ~(1 << (stream_id + STREAM_MAX_NUM));
            break;
          default:
            break;
        }
      }
      queue_ptr->read_pos_inc(queue_mode::normal);
      m_recv_cb(send_packet);
    } else {
      usleep(20000);
    }
  } else {
    usleep(20000);
  }

  return state;
}

AM_STATE AMAVQueue::send_event_packet()
{
  AM_STATE state = AM_STATE_OK;

  do {
    AM_PTS       min_pts        = INT64_MAX;
    AMPacket    *send_packet    = nullptr;
    ExPayload   *send_payload   = nullptr;
    AMRingQueue *queue_ptr      = nullptr;

    while (!m_stop.load() &&
        m_video_queue[m_event_video_id]->empty(queue_mode::event)) {
      std::unique_lock<std::mutex> lk(m_event_send_mutex);
      m_event_video_block = true;
      m_event_send_cond.wait(lk);
      m_event_video_block = false;
    }
    if (m_stop.load()) {break;}

    send_payload = m_video_queue[m_event_video_id]->event_get();
    if (send_payload->m_data.m_payload_pts < min_pts) {
      min_pts = send_payload->m_data.m_payload_pts;
      queue_ptr = m_video_queue[m_event_video_id];
    }

    while (!m_stop.load() &&
        m_audio_queue[m_event_audio_id]->empty(queue_mode::event)) {
      std::unique_lock<std::mutex> lk(m_event_send_mutex);
      m_event_audio_block = true;
      m_event_send_cond.wait(lk);
      m_event_audio_block = false;
    }
    if (m_stop.load()) {break;}

    send_payload = m_audio_queue[m_event_audio_id]->event_get();
    if (send_payload->m_data.m_payload_pts < min_pts) {
      min_pts = send_payload->m_data.m_payload_pts;
      queue_ptr = m_audio_queue[m_event_audio_id];
    }
    send_payload = queue_ptr->event_get();

    if (queue_ptr) {
      if (!m_packet_pool->alloc_packet(send_packet)) {
        WARN("Failed to allocate event packet!");
        return AM_STATE_ERROR;
      }

      send_packet->set_payload(send_payload);
      send_packet->set_event_id(0);

      if (min_pts >= m_event_end_pts) {
        m_event_video_eos = true;
        send_packet->set_packet_type(AMPacket::AM_PACKET_TYPE_EVENT |
                                     AMPacket::AM_PACKET_TYPE_STOP);
        INFO("Event stop PTS: %lld.", min_pts);
      } else {
        send_packet->set_packet_type(AMPacket::AM_PACKET_TYPE_EVENT);
      }

      queue_ptr->read_pos_inc(queue_mode::event);
      m_recv_cb(send_packet);
    }
  } while (0);
  return state;
}

AM_STATE AMAVQueue::send_event_info()
{
  AM_STATE state = AM_STATE_OK;
  AMPacket *send_packet = nullptr;

  do {
    if (!m_packet_pool->alloc_packet(send_packet)) {
      WARN("Failed to allocate event packet!");
      state = AM_STATE_ERROR;
      break;
    }

    send_packet->set_packet_type(AMPacket::AM_PACKET_TYPE_EVENT);
    send_packet->set_payload(&m_video_info_payload[m_event_video_id]);
    send_packet->set_data_ptr((uint8_t*)&m_video_info[m_event_video_id]);
    send_packet->set_data_size(sizeof(AM_VIDEO_INFO));
    send_packet->set_event_id(0);
    m_recv_cb(send_packet);

    send_packet->set_packet_type(AMPacket::AM_PACKET_TYPE_EVENT);
    send_packet->set_payload(&m_audio_info_payload[m_event_audio_id]);
    send_packet->set_data_ptr((uint8_t*)&m_audio_info[m_event_audio_id]);
    send_packet->set_data_size(sizeof(AM_AUDIO_INFO));
    send_packet->set_event_id(0);
    m_recv_cb(send_packet);
  } while (0);

  return state;
}

AM_STATE AMAVQueue::on_info(AMPacket *packet)
{
  AM_STATE state = AM_STATE_OK;

  if (!packet) {
    ERROR("Packet is null!");
    return AM_STATE_ERROR;
  }

  uint32_t stream_id = packet->get_stream_id();
  AM_AUDIO_INFO *audio_info = nullptr;
  AM_VIDEO_INFO *video_info = nullptr;

  switch (packet->get_attr()) {
    case AMPacket::AM_PAYLOAD_ATTR_VIDEO: {
      video_info = (AM_VIDEO_INFO*)packet->get_data_ptr();
      if ((stream_id != m_config.video_id) ||
          (video_info->type != AM_VIDEO_H264 &&
              video_info->type != AM_VIDEO_H265)) {
        break;
      }
      m_video_info[stream_id] = *video_info;
      switch (video_info->type) {
        case AM_VIDEO_H264:
          INFO("H264 INFO[%d]: Width: %d, Height: %d",
               stream_id,
               video_info->width,
               video_info->height);
          break;
        case AM_VIDEO_H265:
          INFO("H265 INFO[%d]: Width: %d, Height: %d",
               stream_id,
               video_info->width,
               video_info->height);
          break;
        case AM_VIDEO_MJPEG:
          INFO("MJPEG INFO[%d]: Width: %d, Height: %d",
               stream_id,
               video_info->width,
               video_info->height);
          break;
        default:
          state = AM_STATE_ERROR;
          ERROR("Unknown video type: %d!", video_info->type);
          break;
      }

      if ((m_run != uint32_t(~0)) && (m_run & (1 << stream_id))) {
        break;
      }

      uint32_t video_payload_count = m_video_payload_count;
      if (m_config.event_enable) {
        m_video_info_payload[stream_id] = *packet->get_payload();
        video_payload_count = m_config.event_max_history_duration *
            video_info->scale / video_info->rate + 4 * video_info->N;
      }

      if (!(m_video_queue[stream_id] =
          AMRingQueue::create(video_payload_count))) {
        ERROR("Failed to create video queue[%d]!", stream_id);
        return AM_STATE_ERROR;
      }
      INFO("Video Queue[%d] count: %d", stream_id, video_payload_count);
      m_first_video[stream_id].first = false;
      m_write_video_state[stream_id] = 0;

      m_run = (m_run == (uint32_t)(~0)) ?
          (1 << stream_id) : (m_run | (1 << stream_id));
    } break;

    case AMPacket::AM_PAYLOAD_ATTR_AUDIO: {
      audio_info = (AM_AUDIO_INFO*)packet->get_data_ptr();
      bool valid_type = false;
      for (auto &m : m_config.audio_types) {
        if (m.first == audio_info->type) {
          for (auto &v : m.second) {
            if (v == audio_info->sample_rate) {
              m_audio_ids.push_back(v);
              valid_type = true;
            }
          }
        }
      }
      if ((m_config.event_audio.first == audio_info->type) &&
          (m_config.event_audio.second == audio_info->sample_rate)) {
        valid_type = true;
      }
      if (!valid_type) {
        break;
      }

      m_audio_info[stream_id] = *audio_info;
      INFO("Audio[%d]: channels: %d, sample rate: %d, "
          "chunk size: %d, pts_increment: %d, sample size : %d",
          stream_id,
          audio_info->channels,
          audio_info->sample_rate,
          audio_info->chunk_size,
          audio_info->pkt_pts_increment,
          audio_info->sample_size);
      m_audio_pts_increment = audio_info->pkt_pts_increment;

      if ((m_run != uint32_t(~0)) &&
          (m_run & (1 << (stream_id + STREAM_MAX_NUM)))) {
        break;
      }

      uint32_t audio_payload_count = m_audio_payload_count;

      if (m_config.event_enable &&
          (m_config.event_audio.first == audio_info->type) &&
          (m_config.event_audio.second == audio_info->sample_rate)) {
        uint32_t packet_per_sec = audio_info->sample_rate /
            audio_info->chunk_size *
            audio_info->sample_size *
            audio_info->channels + 1;
        uint32_t count = packet_per_sec *
            (m_config.event_max_history_duration + 20);
        if (count > audio_payload_count) {
          audio_payload_count = count;
        }
        m_audio_info_payload[stream_id] = *packet->get_payload();
      }

      if (!(m_audio_queue[stream_id] =
          AMRingQueue::create(audio_payload_count))) {
        ERROR("Failed to create audio queue[%d]!", stream_id);
        return AM_STATE_ERROR;
      }
      INFO("Audio Queue[%d] count: %d", stream_id, audio_payload_count);
      m_send_audio_state[stream_id] = 0;
      m_write_audio_state[stream_id] = 0;
      m_first_audio[stream_id].first = false;

      m_run = (m_run == (uint32_t)(~0)) ?
          (1 << (stream_id + STREAM_MAX_NUM)) :
          (m_run | (1 << (stream_id + STREAM_MAX_NUM)));
    } break;
    default:
      break;
  }

  return state;
}

AM_STATE AMAVQueue::on_data(AMPacket* packet)
{
  std::unique_lock<std::mutex> lk(m_mutex);
  AM_STATE state = AM_STATE_OK;
  if (!packet) {
    ERROR("Packet is null!");
    return AM_STATE_ERROR;
  }
  if (packet->get_pts() < 0) {
    ERROR("Packet pts: %d!", packet->get_pts());
    return AM_STATE_ERROR;
  }

  queue_mode event_mode;
  uint32_t stream_id = packet->get_stream_id();
  AMPacket::Payload *payload = packet->get_payload();
  switch (packet->get_attr()) {
    case AMPacket::AM_PAYLOAD_ATTR_SEI: {
    } break;
    case AMPacket::AM_PAYLOAD_ATTR_GPS: {
    } break;
    case AMPacket::AM_PAYLOAD_ATTR_VIDEO: {
      if (m_video_queue.find(stream_id) == m_video_queue.end()) {
        break;
      }

      event_mode = (m_in_event && (stream_id == m_event_video_id)) ?
          queue_mode::event : queue_mode::normal;

      switch (m_write_video_state[stream_id]) {
        case 1: //Video queue is full, wait for sending
          if (m_video_queue[stream_id]->full(event_mode)) {
            break;
          } else {
            m_write_video_state[stream_id] = 2;
            WARN("Video[%d] queue wait I frame!", stream_id);
          }
        case 2: //Video Queue is not full, wait I frame
          if (packet->get_frame_type() == AM_VIDEO_FRAME_TYPE_IDR ||
              packet->get_frame_type() == AM_VIDEO_FRAME_TYPE_I) {
            WARN("Video[%d] I frame comes, stop dropping packet!", stream_id);
            m_write_video_state[stream_id] = 0;
          } else {
            break;
          }
        case 0: //Normal write
          if (m_video_queue[stream_id]->full(event_mode)) {
            m_write_video_state[stream_id] = 1;
            WARN("Video[%d] queue is full, start to drop packet", stream_id);
          } else {
            if (!m_video_queue[stream_id]->write(payload, event_mode)) {
              ERROR("Failed to write payload to video queue[%d]!", stream_id);
              state = AM_STATE_ERROR;
              break;
            }

            if (!m_video_come.load()) {
              m_video_come = true;
            }

            if ((m_first_video.find(stream_id) == m_first_video.end()) ||
                !m_first_video[stream_id].first) {
              m_first_video[stream_id].second = packet->get_pts();
              m_first_video[stream_id].first = true;
              INFO("First video[%d] frame type: %d, PTS: %lld.",
                   stream_id,
                   packet->get_frame_type(),
                   m_first_video[stream_id].second);
            }

            {
              std::unique_lock<std::mutex> lk(m_send_mutex);
              if (m_video_send_block.second &&
                  (m_video_send_block.first == stream_id)) {
                m_send_cond.notify_one();
              }
            }

            {
              std::unique_lock<std::mutex> lk(m_event_send_mutex);
              if (m_event_video_block && (m_event_video_id == stream_id)) {
                m_event_send_cond.notify_one();
              }
            }
          }
          break;
        default: break;
      }
    } break;

    case AMPacket::AM_PAYLOAD_ATTR_AUDIO: {
      if (m_audio_queue.find(stream_id) == m_audio_queue.end()) {
        break;
      }
      event_mode = (m_in_event && (stream_id == m_event_audio_id)) ?
          queue_mode::event : queue_mode::normal;

      switch (m_write_audio_state[stream_id]) {
        case 1: //Audio queue is full, wait for sending
          if (m_audio_queue[stream_id]->full((event_mode))) {
            break;
          } else {
            m_write_audio_state[stream_id] = 0;
            WARN("Audio[%d] stop dropping packet!", stream_id);
          }
        case 0: { //Normal write
          if (m_audio_queue[stream_id]->full(event_mode)) {
            WARN("Audio[%d] queue is full, start to drop packet", stream_id);
            m_write_audio_state[stream_id] = 1;
          } else {
            if (!m_audio_queue[stream_id]->write(payload, event_mode)) {
              ERROR("Failed to write payload to audio queue[%d]", stream_id);
              state = AM_STATE_ERROR;
              break;
            }

            if ((m_first_audio.find(stream_id) == m_first_audio.end()) ||
                !m_first_audio[stream_id].first) {
              m_first_audio[stream_id].second = packet->get_pts();
              m_first_audio[stream_id].first = true;
              INFO("First audio PTS: %lld.", m_first_audio[stream_id].second);
            }

            {
              std::unique_lock<std::mutex> lk(m_send_mutex);
              if (m_audio_send_block.second &&
                  (m_audio_send_block.first == stream_id)) {
                m_send_cond.notify_one();
              }
            }

            {
              std::unique_lock<std::mutex> lk(m_event_send_mutex);
              if (m_event_audio_block && (stream_id == m_event_audio_id)) {
                m_event_send_cond.notify_one();
              }
            }
          }
        } break;
        default: break;
      }
    } break;

    default:
      ERROR("Invalid data type!");
      state = AM_STATE_ERROR;
      break;
  }

  return state;
}

AM_STATE AMAVQueue::on_event(AMPacket *packet)
{
  std::unique_lock<std::mutex> lk(m_mutex);
  AM_STATE state = AM_STATE_OK;
  do {
    AMEventStruct *event = (AMEventStruct*)(packet->get_data_ptr());
    if (event->attr == AM_EVENT_MJPEG){
      INFO("AMEventSender send event packet, event attr is AM_EVENT_MJPEG,"
          "stream id is %u, pre num is %u, after number is %u, "
          "closest num is %u", event->mjpeg.stream_id,
          event->mjpeg.pre_cur_pts_num, event->mjpeg.after_cur_pts_num,
          event->mjpeg.closest_cur_pts_num);
      break;
    } else if (event->attr == AM_EVENT_PERIODIC_MJPEG) {
      INFO("AMEventSender send event packet, event attr is "
          "AM_EVENT_PERIODIC_MJPEG, stream id is %u, interval second is %u, "
          "start time is %u-%u-%u, end time is %u-%u-%u",
          event->periodic_mjpeg.stream_id, event->periodic_mjpeg.interval_second,
          event->periodic_mjpeg.start_time_hour,
          event->periodic_mjpeg.start_time_minute,
          event->periodic_mjpeg.start_time_second,
          event->periodic_mjpeg.end_time_hour,
          event->periodic_mjpeg.end_time_minute,
          event->periodic_mjpeg.end_time_second);
      break;
    } else if (event->attr == AM_EVENT_H26X) {
      INFO("AMEventSender send event packet, event attr is AM_EVENT_H26X,"
          "event id is %u", event->h26x.event_id);
    } else {
      ERROR("Event attr error.");
      state = AM_STATE_ERROR;
      break;
    }
    if ((m_event_video_id = event->h26x.event_id) != m_config.video_id) {
      break;
    }
    if (event->h26x.history_duration > m_config.event_max_history_duration) {
      event->h26x.history_duration = m_config.event_max_history_duration;
    }

    INFO("Event Video ID: %d, Audio ID: %d", m_event_video_id, m_event_audio_id);

    if (!is_ready_for_event(*event)) {
      ERROR("AVQueue is not available for event!");
      return state;
    }

    m_video_queue[m_event_video_id]->event_reset();
    m_audio_queue[m_event_audio_id]->event_reset();
    AM_PTS event_pts = packet->get_pts();
    INFO("Event occurrence PTS: %lld.", event_pts);

    AM_PTS video_start_pts = m_first_video[m_event_video_id].second;
    //AM_PTS audio_start_pts = m_first_audio[m_event_audio_id].second;

    if (event_pts > (video_start_pts +
                     event->h26x.history_duration * H264_SCALE)) {
      video_start_pts = event_pts - event->h26x.history_duration * H264_SCALE;
    }
    m_event_end_pts = event_pts + event->h26x.future_duration * H264_SCALE;
    INFO("Event start PTS: %lld, end PTS: %lld.",
         video_start_pts, m_event_end_pts);

    AM_PTS video_pts = 0;
    AM_PTS audio_pts = 0;
    ExPayload *video_payload = nullptr;
    ExPayload *audio_payload = nullptr;

    //Set Video ReadPos
    while (!m_video_queue[m_event_video_id]->event_empty()) {
      m_video_queue[m_event_video_id]->event_backtrack();
      video_payload = m_video_queue[m_event_video_id]->event_get();
      video_pts = video_payload->m_data.m_payload_pts;

      // Set Audio ReadPos
      while (!m_audio_queue[m_event_audio_id]->event_empty()) {
        audio_payload = m_audio_queue[m_event_audio_id]->event_get_prev();
        audio_pts = audio_payload->m_data.m_payload_pts;
        if (audio_pts < video_pts) {
          break;
        }
        m_audio_queue[m_event_audio_id]->event_backtrack();
      }
      if ((video_pts <= video_start_pts) &&
          (video_payload->m_data.m_frame_type == AM_VIDEO_FRAME_TYPE_IDR)) {
        break;
      }
    }
    INFO("Event: current audio PTS: %lld, video PTS: %lld.",
         audio_pts, video_pts);

    if (send_event_info() != AM_STATE_OK) {
      state = AM_STATE_ERROR;
      ERROR("Failed to send event information!");
      return state;
    }
    m_in_event = true;
  } while(0);
  return state;
}

AM_STATE AMAVQueue::on_eos(AMPacket* packet)
{
  switch (packet->get_attr()) {
    case AMPacket::AM_PAYLOAD_ATTR_VIDEO:
      INFO("Video[%d] EOS", packet->get_stream_id());
      break;
    case AMPacket::AM_PAYLOAD_ATTR_AUDIO:
      INFO("Audio[%d] EOS", packet->get_stream_id());
      break;
    default:
      break;
  }

  return on_data(packet);
}

AMAVQueue::AMAVQueue()
{
}

AMAVQueue::~AMAVQueue()
{
  m_stop = true;
  AM_DESTROY(m_send_normal_pkt_thread);
  AM_DESTROY(m_send_event_pkt_thread);
  AM_DESTROY(m_packet_pool);

  for (auto &m : m_video_queue) {
    AM_DESTROY(m.second);
  }
  for (auto &m : m_audio_queue) {
    AM_DESTROY(m.second);
  }
}
