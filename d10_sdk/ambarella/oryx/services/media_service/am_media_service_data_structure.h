/*******************************************************************************
 * am_media_service_data_structure.h
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
#ifndef AM_MEDIA_SERVICE_DATA_STRUCTURE_H_
#define AM_MEDIA_SERVICE_DATA_STRUCTURE_H_
#include "am_record_event_sender.h"
#include "am_api_media.h"
#include "am_audio_define.h"

struct AudioFileList{
  public:
    enum {
      MAX_FILENAME_LENGTH = 490,
      MAX_FILE_NUMBER     = 2,
    };
    AudioFileList(){
      file_number = 0;
      for(uint32_t i = 0; i< MAX_FILE_NUMBER; ++ i) {
        memset(file_list[i], 0, sizeof(file_list[i]));
      }
    }
  public:
    uint32_t file_number;
    char file_list[MAX_FILE_NUMBER][MAX_FILENAME_LENGTH];
};

class AMApiPlaybackAudioFileList : public AMIApiPlaybackAudioFileList
{

  public:
    AMApiPlaybackAudioFileList();
    AMApiPlaybackAudioFileList(AMIApiPlaybackAudioFileList* audio_file);
    virtual ~AMApiPlaybackAudioFileList();
    virtual bool add_file(const std::string &file_name);
    virtual std::string get_file(uint32_t file_number);
    virtual uint32_t get_file_number();
    virtual bool is_full();
    virtual char* get_file_list();
    virtual uint32_t get_file_list_size();
    virtual void clear_file();
  private:
    AudioFileList m_list;
};

class AMApiMediaEvent : public AMIApiMediaEvent
{
  public :
    virtual ~AMApiMediaEvent() {}
    virtual void set_attr_mjpeg();
    virtual void set_attr_h26X();
    virtual void set_attr_periodic_mjpeg();
    virtual bool is_attr_mjpeg();
    virtual bool is_attr_h26X();
    virtual bool is_attr_periodic_mjpeg();
    virtual bool set_event_id(uint32_t event_id);
    virtual uint32_t get_event_id();
    virtual bool set_stream_id(uint32_t stream_id);
    virtual uint32_t get_stream_id();
    virtual bool set_pre_cur_pts_num(uint8_t num);
    virtual uint8_t get_pre_cur_pts_num();
    virtual bool set_after_cur_pts_num(uint8_t num);
    virtual uint8_t get_after_cur_pts_num();
    virtual bool set_closest_cur_pts_num(uint8_t num);
    virtual uint8_t get_closest_cur_pts_num();
    virtual bool set_interval_second(uint32_t second);
    virtual uint32_t get_interval_second();
    virtual bool set_once_jpeg_num(uint32_t num);
    virtual uint32_t get_once_jpeg_num();
    virtual bool set_start_time_hour(uint8_t hour);
    virtual uint8_t get_start_time_hour();
    virtual bool set_start_time_minute(uint8_t minute);
    virtual uint8_t get_start_time_minute();
    virtual bool set_start_time_second(uint8_t second);
    virtual uint8_t get_start_time_second();
    virtual bool set_end_time_hour(uint8_t hour);
    virtual uint8_t get_end_time_hour();
    virtual bool set_end_time_minute(uint8_t minute);
    virtual uint8_t get_end_time_minute();
    virtual bool set_end_time_second(uint8_t second);
    virtual uint8_t get_end_time_second();
    virtual char* get_data();
    virtual uint32_t get_data_size();
  private :
    AMEventStruct m_event;
};

struct AMRecordingParam
{
    uint32_t muxer_id;
    uint32_t recording_file_num;
    int32_t recording_duration;
};

class AMApiRecordingParam : public AMIApiRecordingParam
{
  public :
    AMApiRecordingParam() {}
    AMApiRecordingParam(AMIApiRecordingParam *data);
    virtual ~AMApiRecordingParam() {}
    virtual bool set_muxer_id(uint32_t muxer_id);
    virtual bool set_recording_file_num(uint32_t file_num);
    virtual bool set_recording_duration(int32_t duration);
    virtual uint32_t get_muxer_id();
    virtual uint32_t get_recording_file_num();
    virtual int32_t get_recording_duration();
    virtual char* get_data();
    virtual uint32_t get_data_size();
    virtual void clear();
  private :
    AMRecordingParam  m_data;
};

struct AudioCodecParam
{
    AM_AUDIO_TYPE  type;
    uint32_t       sample_rate;
    bool           enable;
};

class AMApiAudioCodecParam : public AMIApiAudioCodecParam
{
  public :
    AMApiAudioCodecParam() {}
    virtual ~AMApiAudioCodecParam() {}
    virtual void set_audio_type_aac();
    virtual void set_audio_type_opus();
    virtual void set_audio_type_g711A();
    virtual void set_audio_type_g711U();
    virtual void set_audio_type_g726_40();
    virtual void set_audio_type_g726_32();
    virtual void set_audio_type_g726_24();
    virtual void set_audio_type_g726_16();
    virtual void set_audio_type_speex();
    virtual void set_sample_rate(uint32_t sample_rate);
    virtual uint32_t get_sample_rate();
    virtual void enable(bool enable);
    virtual bool is_enable();
    virtual char* get_data();
    virtual uint32_t get_data_size();
  private :
    AudioCodecParam m_data;
};
#endif /* AM_MEDIA_SERVICE_DATA_STRUCTURE_H_ */
