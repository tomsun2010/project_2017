/*******************************************************************************
 * am_media_service_msg_action.cpp
 *
 * History:
 *   2014-9-12 - [lysun] created file
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
#include "am_service_impl.h"
#include "am_service_frame_if.h"
#include "am_media_service_instance.h"
#include "am_media_service_data_structure.h"

extern AM_SERVICE_STATE g_service_state;
extern AMMediaService  *g_media_instance;
extern AMIServiceFrame *g_service_frame;

void ON_SERVICE_INIT(void *msg_data,
                     int msg_data_size,
                     void *result_addr,
                     int result_max_size)
{
  am_service_result_t *service_result = (am_service_result_t *) result_addr;
  //if it's init done, then
  service_result->ret = 0;
  service_result->state = g_service_state;
  switch(g_service_state) {
    case AM_SERVICE_STATE_INIT_DONE: {
      INFO("Media service Init Done...");
    }break;
    case AM_SERVICE_STATE_ERROR: {
      ERROR("Failed to initialize Media service...");
    }break;
    case AM_SERVICE_STATE_NOT_INIT: {
      INFO("Media service is still initializing...");
    }break;
    default:break;
  }
}

void ON_SERVICE_DESTROY(void *msg_data,
                     int msg_data_size,
                     void *result_addr,
                     int result_max_size)
{
  int ret = 0;
  PRINTF("ON MEDIA SERVICE DESTROY.");
  if (g_media_instance) {
    g_media_instance->stop_media();
    g_service_state = AM_SERVICE_STATE_STOPPED;
  } else {
    ERROR("Media service: Failed to get AMMediaService instance!");
    g_service_state = AM_SERVICE_STATE_ERROR;
    ret = -1;
  }
  ((am_service_result_t*)result_addr)->ret = ret;
  ((am_service_result_t*)result_addr)->state = g_service_state;
  g_service_frame->quit();
}

void ON_SERVICE_START(void *msg_data,
                      int msg_data_size,
                      void *result_addr,
                      int result_max_size)
{
  PRINTF("ON MEDIA SERVICE START.");
  if (!g_media_instance) {
    ERROR("media instance is not created.");
    g_service_state = AM_SERVICE_STATE_ERROR;
  } else {
    if (g_service_state == AM_SERVICE_STATE_STARTED) {
      INFO("media instance is already started.");
    } else {
      if (!g_media_instance->start_media()) {
        ERROR("Media service: start media failed!");
        g_service_state = AM_SERVICE_STATE_ERROR;
      } else {
        g_service_state = AM_SERVICE_STATE_STARTED;
      }
    }
  }
  //if it's init done, then
  ((am_service_result_t*)result_addr)->ret = 0;
  ((am_service_result_t*)result_addr)->state = g_service_state;
}

void ON_SERVICE_STOP(void *msg_data,
                     int msg_data_size,
                     void *result_addr,
                     int result_max_size)
{
  PRINTF("ON MEDIA SERVICE STOP.");
  if (!g_media_instance) {
    ERROR("AMMediaInstance instance is not created.");
    g_service_state = AM_SERVICE_STATE_ERROR;
  } else {
    if(!g_media_instance->stop_media()) {
      ERROR("Media service: stop media failed!");
      g_service_state = AM_SERVICE_STATE_ERROR;
    } else {
      g_service_state = AM_SERVICE_STATE_STOPPED;
    }
  }
  ((am_service_result_t*)result_addr)->ret = 0;
  ((am_service_result_t*)result_addr)->state = g_service_state;
}

void ON_SERVICE_RESTART(void *msg_data,
                     int msg_data_size,
                     void *result_addr,
                     int result_max_size)
{
  PRINTF("ON MEDIA SERVICE RESTART.");
  if (!g_media_instance) {
    ERROR("media instance is not created.");
    g_service_state = AM_SERVICE_STATE_ERROR;
  } else {
    if (!g_media_instance->stop_media()) {
      ERROR("Media service: stop media failed!");
      g_service_state = AM_SERVICE_STATE_ERROR;
    } else if(!g_media_instance->start_media()) {
      ERROR("Media service: start media failed!");
      g_service_state = AM_SERVICE_STATE_ERROR;
    } else {
      g_service_state = AM_SERVICE_STATE_STARTED;
    }
  }
  ((am_service_result_t*)result_addr)->ret = 0;
  ((am_service_result_t*)result_addr)->state = g_service_state;
}

void ON_SERVICE_STATUS(void *msg_data,
                     int msg_data_size,
                     void *result_addr,
                     int result_max_size)
{
  PRINTF("ON MEDIA SERVICE STATUS.");
  ((am_service_result_t*)result_addr)->ret = 0;
  ((am_service_result_t*)result_addr)->state = g_service_state;
}

void ON_AM_IPC_MW_CMD_MEDIA_EVENT_RECORDING_START(void* msg_data,
                                                  int msg_data_size,
                                                  void *result_addr,
                                                  int result_max_size)
{
  PRINTF("ON_AM_IPC_MW_CMD_MEDIA_EVENT_RECORDING_START");
  int32_t ret = 0;
  AMEventStruct* event = (AMEventStruct*)msg_data;
  if (!g_media_instance->send_event(*event)) {
    ERROR("Failed to send event.");
    ret = -1;
  }
  if (result_addr) {
     memcpy(result_addr, &ret, sizeof(int32_t));
   }
}

void ON_AM_IPC_MW_CMD_MEDIA_PERIODIC_JPEG_RECORDING(void* msg_data,
                                                  int msg_data_size,
                                                  void *result_addr,
                                                  int result_max_size)
{
  PRINTF("ON_AM_IPC_MW_CMD_MEDIA_PERIODIC_JPEG_RECORDING");
  int32_t ret = 0;
  AMEventStruct* event = (AMEventStruct*)msg_data;
  if (!g_media_instance->send_event(*event)) {
    ERROR("Faile to send event.");
    ret = -1;
  }
  if (result_addr) {
    memcpy(result_addr, &ret, sizeof(int32_t));
  }
}

void ON_AM_IPC_MW_CMD_MEDIA_ADD_AUDIO_FILE(void* msg_data,
                                           int msg_data_size,
                                           void *result_addr,
                                           int result_max_size)
{
  PRINTF("ON_AM_IPC_MW_CMD_MEDIA_ADD_AUDIO_FILE");
  int ret = 0;
  AudioFileList* audio_file = (AudioFileList*)msg_data;
  AMIPlaybackPtr g_playback = g_media_instance->get_playback_instance();
  do{
    if(audio_file->file_number == 0) {
      ERROR("file list is empty\n");
      ret = -1;
      break;
    }
    if(g_playback->is_paused() || g_playback->is_playing()) {
      break;
    }
    INFO("audio file num: %d", audio_file->file_number);
    for(uint32_t i = 0; i< audio_file->file_number; ++ i) {
      INFO("file name :%s",audio_file->file_list[i]);
      AMPlaybackUri uri;
      uri.type = AM_PLAYBACK_URI_FILE;
      memcpy(uri.media.file, audio_file->file_list[i],
             sizeof(audio_file->file_list[i]));
      if(!g_playback->add_uri(uri)) {
        ERROR("Failed to add %s to play list",
               audio_file->file_list[i]);
        ret = -1;
        break;
      }
    }
    if(ret == -1) {
      break;
    }
  }while(0);
  if (result_addr) {
    memcpy(result_addr, &ret, sizeof(int));
  }
}

void ON_AM_IPC_MW_CMD_MEDIA_START_PLAYBACK_AUDIO_FILE(void *msg_data,
                                                      int msg_data_size,
                                                      void *result_addr,
                                                      int result_max_size)
{
  PRINTF("ON_AM_IPC_MW_CMD_MEDIA_START_PLAYBACK_AUDIO_FILE");
  int ret = 0;
  AMIPlaybackPtr g_playback = g_media_instance->get_playback_instance();
  do{
    if(g_playback->is_paused()) {
      if(!g_playback->pause(false)) {
        PRINTF("Failed to resume playback.");
        ret = -1;
        break;
      }
    } else if(g_playback->is_playing()) {
      PRINTF("Playback is already playing.");
      break;
    } else {
      if(!g_playback->play()) {
        PRINTF("Failed to start playing.");
        ret = -1;
        break;
      }
    }
  }while(0);
  if(result_addr) {
    memcpy(result_addr, &ret, sizeof(int));
  }
}

void ON_AM_IPC_MW_CMD_MEDIA_PAUSE_PLAYBACK_AUDIO_FILE(void *msg_data,
                                                      int msg_data_size,
                                                      void *result_addr,
                                                      int result_max_size)
{
  PRINTF("ON_AM_IPC_MW_CMD_MEDIA_PAUSE_PLAYBACK_AUDIO_FILE");
  int ret = 0;
  AMIPlaybackPtr g_playback = g_media_instance->get_playback_instance();
  do{
    if(!g_playback->pause(true)) {
      PRINTF("Failed to pause playing.");
      ret = -1;
      break;
    }
  }while(0);
  if(result_addr) {
    memcpy(result_addr, &ret, sizeof(int));
  }
}

void ON_AM_IPC_MW_CMD_MEDIA_STOP_PLAYBACK_AUDIO_FILE(void *msg_data,
                                                     int msg_data_size,
                                                     void *result_addr,
                                                     int result_max_size)
{
  PRINTF("ON_AM_IPC_MW_CMD_MEDIA_STOP_PLAYBACK_AUDIO_FILE");
  int ret = 0;
  AMIPlaybackPtr g_playback = g_media_instance->get_playback_instance();
  do {
    if(!g_playback->stop()) {
      PRINTF("Failed to stop playing.");
      ret = -1;
      break;
    }
  }while(0);
  if(result_addr) {
    memcpy(result_addr, &ret, sizeof(int));
  }
}

void ON_AM_IPC_MW_CMD_MEDIA_START_RECORDING(void *msg_data,
                                            int msg_data_size,
                                            void *result_addr,
                                            int result_max_size)
{
  PRINTF("ON_AM_IPC_MW_CMD_MEDIA_START_RECORDING");
  int ret = 0;
  AMIRecordPtr g_record = g_media_instance->get_record_instance();
  do {
    if(g_record->is_recording()) {
      PRINTF("The media service is already recording.");
      break;
    }
    if(!g_record->start()) {
      PRINTF("Failed to start recording.");
      ret = -1;
      break;
    }
  }while(0);
  if(result_addr) {
    memcpy(result_addr, &ret, sizeof(int));
  }
}

void ON_AM_IPC_MW_CMD_MEDIA_STOP_RECORDING(void *msg_data,
                                           int msg_data_size,
                                           void *result_addr,
                                           int result_max_size)
{
  PRINTF("ON_AM_IPC_MW_CMD_MEDIA_STOP_RECORDING");
  int ret = 0;
  AMIRecordPtr g_record = g_media_instance->get_record_instance();
  do {
    if(!g_record->is_recording()) {
      PRINTF("The media service recording is already stopped.");
      break;
    }
    if(!g_record->stop()) {
      PRINTF("Failed to stop recording.");
      ret = -1;
      break;
    }
  }while(0);
  if(result_addr) {
    memcpy(result_addr, &ret, sizeof(int));
  }
}

void ON_AM_IPC_MW_CMD_MEDIA_START_FILE_RECORDING(void *msg_data,
                                                 int msg_data_size,
                                                 void *result_addr,
                                                 int result_max_size)
{
  PRINTF("ON_AM_IPC_MW_CMD_MEDIA_START_FILE_RECORDING");
  int ret = 0;
  uint32_t muxer_id = 0xffffffff;
  AMIRecordPtr g_record = g_media_instance->get_record_instance();
  do {
    if (msg_data) {
      muxer_id = *((uint32_t*) msg_data);
    } else {
      ERROR("The msg data of start file recording is invalid.");
      ret = -1;
      break;
    }
    if (!g_record->start_file_recording(muxer_id)) {
      PRINTF("Failed to start file recording.");
      ret = -1;
      break;
    }
  } while (0);
  if (result_addr) {
    memcpy(result_addr, &ret, sizeof(int));
  }
}

void ON_AM_IPC_MW_CMD_MEDIA_STOP_FILE_RECORDING(void *msg_data,
                                                int msg_data_size,
                                                void *result_addr,
                                                int result_max_size)
{
  PRINTF("ON_AM_IPC_MW_CMD_MEDIA_STOP_FILE_RECORDING");
  int ret = 0;
  uint32_t muxer_id = 0xffffffff;
  AMIRecordPtr g_record = g_media_instance->get_record_instance();
  if(msg_data) {
    muxer_id = *((uint32_t*)msg_data);
  }
  do {
    if(!g_record->stop_file_recording(muxer_id)) {
      PRINTF("Failed to stop file recording.");
      ret = -1;
      break;
    }
  }while(0);
  if(result_addr) {
    memcpy(result_addr, &ret, sizeof(int));
  }
}

void ON_AM_IPC_MW_CMD_MEDIA_SET_RECORDING_FILE_NUM(void *msg_data,
                                                   int msg_data_size,
                                                   void *result_addr,
                                                   int result_max_size)
{
  PRINTF("ON_AM_IPC_MW_CMD_MEDIA_SET_RECORDING_FILE_NUM");
  int ret = 0;
  AMIRecordPtr g_record = g_media_instance->get_record_instance();
  AMRecordingParam *param = nullptr;
  do {
    if(msg_data) {
      param = ((AMRecordingParam*)msg_data);
    } else {
      ERROR("The msg data is invalid in ON_AM_IPC_MW_CMD_MEDIA_SET_RECORDING_FILE_NUM");
      ret = -1;
      break;
    }
    if(!g_record->set_recording_file_num(param->muxer_id,
                                         param->recording_file_num)) {
      PRINTF("Failed to set recording file num for muxer%u.", param->muxer_id);
      ret = -1;
      break;
    }
  }while(0);
  if(result_addr) {
    memcpy(result_addr, &ret, sizeof(int));
  }
}

void ON_AM_IPC_MW_CMD_MEDIA_SET_RECORDING_DURATION(void *msg_data,
                                                   int msg_data_size,
                                                   void *result_addr,
                                                   int result_max_size)
{
  PRINTF("ON_AM_IPC_MW_CMD_MEDIA_SET_RECORDING_DURATION");
  int ret = 0;
  AMIRecordPtr g_record = g_media_instance->get_record_instance();
  AMRecordingParam *param = nullptr;
  do {
    if(msg_data) {
      param = ((AMRecordingParam*)msg_data);
    } else {
      ERROR("The msg data is invalid in ON_AM_IPC_MW_CMD_MEDIA_SET_RECORDING_DURATION");
      ret = -1;
      break;
    }
    if(!g_record->set_recording_duration(param->muxer_id,
                                         param->recording_duration)) {
      PRINTF("Failed to set recording duration for muxer%u.", param->muxer_id);
      ret = -1;
      break;
    }
  }while(0);
  if(result_addr) {
    memcpy(result_addr, &ret, sizeof(int));
  }
}

void ON_AM_IPC_MW_CMD_MEDIA_ENABLE_AUDIO_CODEC(void *msg_data,
                                               int msg_data_size,
                                               void *result_addr,
                                               int result_max_size)
{
  PRINTF("ON_AM_IPC_MW_CMD_MEDIA_ENABLE_AUDIO_CODEC");
  int ret = 0;
  AMIRecordPtr g_record = g_media_instance->get_record_instance();
  AudioCodecParam *param = nullptr;
  do {
    if(msg_data) {
      param = ((AudioCodecParam*)msg_data);
    } else {
      ERROR("The msg data is invalid in ON_AM_IPC_MW_CMD_MEDIA_ENABLE_AUDIO_CODEC");
      ret = -1;
      break;
    }
    if(!g_record->enable_audio_codec(param->type, param->sample_rate,
                                     param->enable)) {
      PRINTF("Failed to enable audio codec.");
      ret = -1;
      break;
    }
  }while(0);
  if(result_addr) {
    memcpy(result_addr, &ret, sizeof(int));
  }
}
