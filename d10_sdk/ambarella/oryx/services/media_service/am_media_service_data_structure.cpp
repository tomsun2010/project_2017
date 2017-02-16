/*******************************************************************************
 * am_media_service_data_structure.cpp
 *
 * History:
 *   May 13, 2015 - [ccjing] created file
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
#include "am_media_service_data_structure.h"

AMIApiPlaybackAudioFileList* AMIApiPlaybackAudioFileList::create()
{
  AMApiPlaybackAudioFileList* m_list = new AMApiPlaybackAudioFileList();
  return m_list ? (AMIApiPlaybackAudioFileList*)m_list : nullptr;
}

AMIApiPlaybackAudioFileList* AMIApiPlaybackAudioFileList::create(
    AMIApiPlaybackAudioFileList* audio_file)
{
  AMApiPlaybackAudioFileList* m_list = new AMApiPlaybackAudioFileList(audio_file);
  return m_list ? (AMIApiPlaybackAudioFileList*)m_list : nullptr;
}

AMApiPlaybackAudioFileList::AMApiPlaybackAudioFileList()
{}

AMApiPlaybackAudioFileList::AMApiPlaybackAudioFileList(
    AMIApiPlaybackAudioFileList* audio_file)
{
  m_list.file_number = audio_file->get_file_number();
  for (uint32_t i = 0; i < m_list.file_number; ++ i) {
    memcpy(m_list.file_list[i],  audio_file->get_file(i).c_str(),
           sizeof(m_list.file_list[i]));
  }
}

AMApiPlaybackAudioFileList::~AMApiPlaybackAudioFileList()
{
  m_list.file_number = 0;
}

bool AMApiPlaybackAudioFileList::add_file(const std::string &file_name)
{
  bool ret = true;
  if(file_name.empty()) {
    ERROR("file_name is empty, can not add it to file list, drop it.");
  } else if (m_list.file_number >= AudioFileList::MAX_FILE_NUMBER) {
    ERROR("This file list is full, the max file number is %d, "
        "please create a new file list.", AudioFileList::MAX_FILE_NUMBER);
    ret = false;
  } else if (file_name.size() >= AudioFileList::MAX_FILENAME_LENGTH) {
    ERROR("file name is too length, the max file name length is %d",
          AudioFileList::MAX_FILENAME_LENGTH);
    ret = false;
  } else {
    memcpy(m_list.file_list[m_list.file_number], file_name.c_str(),
            file_name.size());
    ++ m_list.file_number;
  }
  return ret;
}

std::string AMApiPlaybackAudioFileList::get_file(uint32_t file_number)
{
  if (file_number > 0 && file_number <= m_list.file_number) {
    return std::string(m_list.file_list[file_number - 1]);
  } else {
    WARN("Do not have this file, return a empty string");
    return std::string("");
  }
}

uint32_t AMApiPlaybackAudioFileList::get_file_number()
{
  return m_list.file_number;
}

bool AMApiPlaybackAudioFileList::is_full()
{
  return (m_list.file_number >= AudioFileList::MAX_FILE_NUMBER);
}

char* AMApiPlaybackAudioFileList::get_file_list()
{
  return (char*)&m_list;
}

uint32_t AMApiPlaybackAudioFileList::get_file_list_size()
{
  return sizeof(AudioFileList);
}

void AMApiPlaybackAudioFileList::clear_file()
{
  for (uint32_t i = 0; i < AudioFileList::MAX_FILE_NUMBER; ++ i) {
    memset(m_list.file_list[i], 0, sizeof(m_list.file_list[i]));
  }
  m_list.file_number = 0;
}

void AMApiMediaEvent::set_attr_mjpeg()
{
  m_event.attr = AM_EVENT_MJPEG;
}

bool AMApiMediaEvent::is_attr_mjpeg()
{
  return (m_event.attr == AM_EVENT_MJPEG);
}

void AMApiMediaEvent::set_attr_h26X()
{
  m_event.attr = AM_EVENT_H26X;
}

bool AMApiMediaEvent::is_attr_h26X()
{
  return (m_event.attr == AM_EVENT_H26X);
}

void AMApiMediaEvent::set_attr_periodic_mjpeg()
{
  m_event.attr = AM_EVENT_PERIODIC_MJPEG;
}

bool AMApiMediaEvent::is_attr_periodic_mjpeg()
{
  return (m_event.attr == AM_EVENT_PERIODIC_MJPEG);
}

bool AMApiMediaEvent::set_event_id(uint32_t event_id)
{
  bool ret = true;
  do {
    if (m_event.attr != AM_EVENT_H26X) {
      ERROR("Only h26x event use event id, others use stream id");
      ret = false;
      break;
    }
    if (event_id >= 0) {
      m_event.h26x.event_id = event_id;
    } else {
      ERROR("Event id is invalid.");
      ret = false;
      break;
    }
  } while(0);
  return ret;
}

uint32_t AMApiMediaEvent::get_event_id()
{
  uint32_t event_id = 0;
  switch (m_event.attr) {
    case AM_EVENT_H26X : {
      event_id = m_event.h26x.event_id;
    } break;
    default : {
      ERROR("Only h26x use event id.");
    } break;
  }
  return event_id;
}

bool AMApiMediaEvent::set_stream_id(uint32_t stream_id)
{
  bool ret = true;
  switch (m_event.attr) {
    case AM_EVENT_H26X : {
      ERROR("H26x do not  use stream id, please use event id");
      ret = false;
    } break;
    case AM_EVENT_PERIODIC_MJPEG : {
      if (stream_id >= 0) {
        m_event.periodic_mjpeg.stream_id = stream_id;
      } else {
        ERROR("Stream id is invalid.");
        ret = false;
      }
    } break;
    case  AM_EVENT_MJPEG : {
      if (stream_id >= 0) {
        m_event.mjpeg.stream_id = stream_id;
      } else {
        ERROR("Stream id is invalid.");
        ret = false;
      }
    } break;
    default : {
      ERROR("The attr is error.");
      ret = false;
    } break;
  }
  return ret;
}

uint32_t AMApiMediaEvent::get_stream_id()
{
  uint32_t stream_id = 0;
  switch (m_event.attr) {
    case AM_EVENT_H26X : {
      ERROR("H26x do not  use stream id, please use event id");
    } break;
    case AM_EVENT_PERIODIC_MJPEG : {
      stream_id = m_event.periodic_mjpeg.stream_id;
    } break;
    case  AM_EVENT_MJPEG : {
      stream_id = m_event.mjpeg.stream_id;
    } break;
    default : {
      ERROR("The attr is error.");
    } break;
  }
  return stream_id;
}

bool AMApiMediaEvent::set_pre_cur_pts_num(uint8_t num)
{
  bool ret = true;
  if (m_event.attr == AM_EVENT_MJPEG) {
    if (num >= 0) {
      m_event.mjpeg.pre_cur_pts_num = num;
    } else {
      ERROR("The pre cur pts num is invalid.");
      ret = false;
    }
  } else {
    ERROR("Pre cur pts num is only used for AM_EVENT_MJPEG");
    ret = false;
  }
  return ret;
}

uint8_t AMApiMediaEvent::get_pre_cur_pts_num()
{
  if (m_event.attr == AM_EVENT_MJPEG) {
    return m_event.mjpeg.pre_cur_pts_num;
  } else {
    ERROR("Pre cur pts num is only used for AM_EVENT_MJPEG");
    return 0;
  }
}

bool AMApiMediaEvent::set_after_cur_pts_num(uint8_t num)
{
  bool ret = true;
  if (m_event.attr == AM_EVENT_MJPEG) {
    if (num >= 0) {
      m_event.mjpeg.after_cur_pts_num = num;
    } else {
      ERROR("The after cur pts num is invalid.");
      ret = false;
    }
  } else {
    ERROR("After cur pts num is only used for AM_EVENT_MJPEG");
    ret = false;
  }
  return ret;
}

uint8_t AMApiMediaEvent::get_after_cur_pts_num()
{
  if (m_event.attr == AM_EVENT_MJPEG) {
    return m_event.mjpeg.after_cur_pts_num;
  } else {
    ERROR("After cur pts num is only used for AM_EVENT_MJPEG");
    return 0;
  }
}

bool AMApiMediaEvent::set_closest_cur_pts_num(uint8_t num)
{
  bool ret = true;
  if (m_event.attr == AM_EVENT_MJPEG) {
    if (num >= 0) {
      m_event.mjpeg.closest_cur_pts_num = num;
    } else {
      ERROR("The closest cur pts num is invalid.");
      ret = false;
    }
  } else {
    ERROR("Closest cur pts num is only used for AM_EVENT_MJPEG");
    ret = false;
  }
  return ret;
}

uint8_t AMApiMediaEvent::get_closest_cur_pts_num()
{
  if (m_event.attr == AM_EVENT_MJPEG) {
    return m_event.mjpeg.closest_cur_pts_num;
  } else {
    ERROR("Closest cur pts num is only used for AM_EVENT_MJPEG");
    return 0;
  }
}

char* AMApiMediaEvent::get_data()
{
  return (char*)(&m_event);
}

uint32_t AMApiMediaEvent::get_data_size()
{
  return sizeof(m_event);
}

AMIApiMediaEvent* AMIApiMediaEvent::create()
{
  AMApiMediaEvent* result = new AMApiMediaEvent();
  return result ? (AMIApiMediaEvent*)result : nullptr;
}

bool AMApiMediaEvent::set_interval_second(uint32_t second)
{
  bool ret = true;
  if (m_event.attr == AM_EVENT_PERIODIC_MJPEG) {
    if ((second > 0) && (second <= 3600)) {
      m_event.periodic_mjpeg.interval_second = second;
      ret = true;
    } else {
      ERROR("The interval second should be between 0 and 3600");
      ret = false;
    }
  } else {
    ERROR("Interval second is only used for AM_EVENT_PERIODIC_MJPEG");
    ret = false;
  }
  return ret;
}

uint32_t AMApiMediaEvent::get_interval_second()
{
  if (m_event.attr == AM_EVENT_PERIODIC_MJPEG) {
    return m_event.periodic_mjpeg.interval_second;
  } else {
    ERROR("Interval second is only used for AM_EVENT_PERIODIC_MJPEG");
    return 0;
  }
}

bool AMApiMediaEvent::set_once_jpeg_num(uint32_t num)
{
  bool ret = true;
   if (m_event.attr == AM_EVENT_PERIODIC_MJPEG) {
     if (num > 0) {
       m_event.periodic_mjpeg.once_jpeg_num = num;
       ret = true;
     } else {
       ERROR("The once jpeg number is invalid.");
       ret = false;
     }
   } else {
     ERROR("Once jpeg number is only used for AM_EVENT_PERIODIC_MJPEG");
     ret = false;
   }
   return ret;
}

uint32_t AMApiMediaEvent::get_once_jpeg_num()
{
  if (m_event.attr == AM_EVENT_PERIODIC_MJPEG) {
    return m_event.periodic_mjpeg.once_jpeg_num;
  } else {
    ERROR("Once jpeg number is only used for AM_EVENT_PERIODIC_MJPEG");
    return 0;
  }
}

bool AMApiMediaEvent::set_start_time_hour(uint8_t hour)
{
  bool ret = true;
  if (m_event.attr == AM_EVENT_PERIODIC_MJPEG) {
    if ((hour >= 0) && (hour < 24)) {
      m_event.periodic_mjpeg.start_time_hour = hour;
      ret = true;
    } else {
      ERROR("The start time hour is invalid.");
      ret = false;
    }
  } else {
    ERROR("Start time hour is only used for AM_EVENT_PERIODIC_MJPEG");
    ret = false;
  }
  return ret;
}

uint8_t AMApiMediaEvent::get_start_time_hour()
{
  if (m_event.attr == AM_EVENT_PERIODIC_MJPEG) {
    return m_event.periodic_mjpeg.start_time_hour;
  } else {
    ERROR("Start time hour is only used for AM_EVENT_PERIODIC_MJPEG");
    return 0;
  }
}

bool AMApiMediaEvent::set_start_time_minute(uint8_t minute)
{
  bool ret = true;
  if (m_event.attr == AM_EVENT_PERIODIC_MJPEG) {
    if ((minute >= 0) && (minute <= 59)) {
      m_event.periodic_mjpeg.start_time_minute = minute;
      ret = true;
    } else {
      ERROR("The start time minute is invalid.");
      ret = false;
    }
  } else {
    ERROR("Start time minute is only used for AM_EVENT_PERIODIC_MJPEG");
    ret = false;
  }
  return ret;
}

uint8_t AMApiMediaEvent::get_start_time_minute()
{
  if (m_event.attr == AM_EVENT_PERIODIC_MJPEG) {
    return m_event.periodic_mjpeg.start_time_minute;
  } else {
    ERROR("Start time minute is only used for AM_EVENT_PERIODIC_MJPEG");
    return 0;
  }
}

bool AMApiMediaEvent::set_start_time_second(uint8_t second)
{
  bool ret = true;
  if (m_event.attr == AM_EVENT_PERIODIC_MJPEG) {
    if ((second >= 0) && (second <= 59)) {
      m_event.periodic_mjpeg.start_time_second = second;
      ret = true;
    } else {
      ERROR("The start time second is invalid.");
      ret = false;
    }
  } else {
    ERROR("Start time second is only used for AM_EVENT_PERIODIC_MJPEG");
    ret = false;
  }
  return ret;
}

uint8_t AMApiMediaEvent::get_start_time_second()
{
  if (m_event.attr == AM_EVENT_PERIODIC_MJPEG) {
    return m_event.periodic_mjpeg.start_time_second;
  } else {
    ERROR("Start time second is only used for AM_EVENT_PERIODIC_MJPEG");
    return 0;
  }
}

bool AMApiMediaEvent::set_end_time_hour(uint8_t hour)
{
  bool ret = true;
  if (m_event.attr == AM_EVENT_PERIODIC_MJPEG) {
    if ((hour >= 0) && (hour < 24)) {
      m_event.periodic_mjpeg.end_time_hour = hour;
      ret = true;
    } else {
      ERROR("The end time hour is invalid.");
      ret = false;
    }
  } else {
    ERROR("End time hour is only used for AM_EVENT_PERIODIC_MJPEG");
    ret = false;
  }
  return ret;
}

uint8_t AMApiMediaEvent::get_end_time_hour()
{
  if (m_event.attr == AM_EVENT_PERIODIC_MJPEG) {
    return m_event.periodic_mjpeg.end_time_hour;
  } else {
    ERROR("End time hour is only used for AM_EVENT_PERIODIC_MJPEG");
    return 0;
  }
}

bool AMApiMediaEvent::set_end_time_minute(uint8_t minute)
{
  bool ret = true;
  if (m_event.attr == AM_EVENT_PERIODIC_MJPEG) {
    if ((minute >= 0) && (minute <= 59)) {
      m_event.periodic_mjpeg.end_time_minute = minute;
      ret = true;
    } else {
      ERROR("The end time minute is invalid.");
      ret = false;
    }
  } else {
    ERROR("End time minute is only used for AM_EVENT_PERIODIC_MJPEG");
    ret = false;
  }
  return ret;
}

uint8_t AMApiMediaEvent::get_end_time_minute()
{
  if (m_event.attr == AM_EVENT_PERIODIC_MJPEG) {
    return m_event.periodic_mjpeg.end_time_minute;
  } else {
    ERROR("End time minute is only used for AM_EVENT_PERIODIC_MJPEG");
    return 0;
  }
}

bool AMApiMediaEvent::set_end_time_second(uint8_t second)
{
  bool ret = true;
  if (m_event.attr == AM_EVENT_PERIODIC_MJPEG) {
    if ((second >= 0) && (second <= 59)) {
      m_event.periodic_mjpeg.end_time_second = second;
      ret = true;
    } else {
      ERROR("The end time second is invalid.");
      ret = false;
    }
  } else {
    ERROR("End time second is only used for AM_EVENT_PERIODIC_MJPEG");
    ret = false;
  }
  return ret;
}

uint8_t AMApiMediaEvent::get_end_time_second()
{
  if (m_event.attr == AM_EVENT_PERIODIC_MJPEG) {
    return m_event.periodic_mjpeg.end_time_second;
  } else {
    ERROR("End time second is only used for AM_EVENT_PERIODIC_MJPEG");
    return 0;
  }
}

bool AMApiRecordingParam::set_muxer_id(uint32_t muxer_id)
{
  bool ret = true;
  if (muxer_id > 0) {
    m_data.muxer_id = muxer_id;
  } else {
    ERROR("The muxer_id is invalid.");
    ret = false;
  }
  return ret;
}

bool AMApiRecordingParam::set_recording_file_num(uint32_t file_num)
{
  m_data.recording_file_num = file_num;
  return true;
}

bool AMApiRecordingParam::set_recording_duration(int32_t duration)
{
  m_data.recording_duration = duration;
  return true;
}

uint32_t AMApiRecordingParam::get_muxer_id()
{
  return m_data.muxer_id;
}

uint32_t AMApiRecordingParam::get_recording_file_num()
{
  return m_data.recording_file_num;
}

int32_t AMApiRecordingParam::get_recording_duration()
{
  return m_data.recording_duration;
}

char* AMApiRecordingParam::get_data() {
  return (char*)(&m_data);
}

uint32_t AMApiRecordingParam::get_data_size()
{
  return sizeof(AMRecordingParam);
}

void AMApiRecordingParam::clear()
{
  memset(&m_data, 0, sizeof(m_data));
}

AMApiRecordingParam::AMApiRecordingParam(
    AMIApiRecordingParam *data)
{
  m_data.recording_file_num = data->get_recording_file_num();
  m_data.muxer_id = data->get_muxer_id();
  m_data.recording_duration = data->get_recording_duration();
}

AMIApiRecordingParam* AMIApiRecordingParam::create()
{
  AMApiRecordingParam *result = new AMApiRecordingParam();
  return result ? (AMIApiRecordingParam*)result : nullptr;
}

AMIApiRecordingParam* AMIApiRecordingParam::create(
    AMIApiRecordingParam *data)
{
  AMApiRecordingParam *result = new AMApiRecordingParam(data);
  return result ? (AMIApiRecordingParam*)result : nullptr;
}

AMIApiAudioCodecParam* AMIApiAudioCodecParam::create()
{
  AMApiAudioCodecParam* result = new AMApiAudioCodecParam();
  return result ? (AMIApiAudioCodecParam*)result : nullptr;
}

void AMApiAudioCodecParam::set_audio_type_aac()
{
  m_data.type = AM_AUDIO_AAC;
}

void AMApiAudioCodecParam::set_audio_type_opus()
{
  m_data.type = AM_AUDIO_OPUS;
}

void AMApiAudioCodecParam::set_audio_type_g711A()
{
  m_data.type = AM_AUDIO_G711A;
}

void AMApiAudioCodecParam::set_audio_type_g711U()
{
  m_data.type = AM_AUDIO_G711U;
}

void AMApiAudioCodecParam::set_audio_type_g726_40()
{
  m_data.type = AM_AUDIO_G726_40;
}

void AMApiAudioCodecParam::set_audio_type_g726_32()
{
  m_data.type = AM_AUDIO_G726_32;
}

void AMApiAudioCodecParam::set_audio_type_g726_24()
{
  m_data.type = AM_AUDIO_G726_24;
}

void AMApiAudioCodecParam::set_audio_type_g726_16()
{
  m_data.type = AM_AUDIO_G726_16;
}

void AMApiAudioCodecParam::set_audio_type_speex()
{
  m_data.type = AM_AUDIO_SPEEX;
}

void AMApiAudioCodecParam::set_sample_rate(uint32_t sample_rate)
{
  m_data.sample_rate = sample_rate;
}

uint32_t AMApiAudioCodecParam::get_sample_rate()
{
  return m_data.sample_rate;
}

void AMApiAudioCodecParam::enable(bool enable)
{
  m_data.enable = enable;
}

bool AMApiAudioCodecParam::is_enable()
{
  return m_data.enable;
}

char* AMApiAudioCodecParam::get_data()
{
  return (char*)(&m_data);
}

uint32_t AMApiAudioCodecParam::get_data_size()
{
  return sizeof(AudioCodecParam);
}
