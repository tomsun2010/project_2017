/*******************************************************************************
 * am_media_service_msg_map.h
 *
 * History:
 *   2014-10-21 - [lysun] created file
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


#ifndef AM_MEDIA_SERVICE_MSG_MAP_H_
#define AM_MEDIA_SERVICE_MSG_MAP_H_
#include "commands/am_api_cmd_media.h"
void ON_SERVICE_INIT(void *msg_data,
                     int msg_data_size,
                     void *result_addr,
                     int result_max_size);
void ON_SERVICE_DESTROY(void *msg_data,
                     int msg_data_size,
                     void *result_addr,
                     int result_max_size);
void ON_SERVICE_START(void *msg_data,
                     int msg_data_size,
                     void *result_addr,
                     int result_max_size);
void ON_SERVICE_STOP(void *msg_data,
                     int msg_data_size,
                     void *result_addr,
                     int result_max_size);
void ON_SERVICE_RESTART(void *msg_data,
                     int msg_data_size,
                     void *result_addr,
                     int result_max_size);
void ON_SERVICE_STATUS(void *msg_data,
                     int msg_data_size,
                     void *result_addr,
                     int result_max_size);
void ON_AM_IPC_MW_CMD_MEDIA_EVENT_RECORDING_START(void* msg_data,
                                                  int msg_data_size,
                                                  void *result_addr,
                                                  int result_max_size);
void ON_AM_IPC_MW_CMD_MEDIA_PERIODIC_JPEG_RECORDING(void* msg_data,
                                                  int msg_data_size,
                                                  void *result_addr,
                                                  int result_max_size);
void ON_AM_IPC_MW_CMD_MEDIA_ADD_AUDIO_FILE(void* msg_data,
                                           int msg_data_size,
                                           void *result_addr,
                                           int result_max_size);
void ON_AM_IPC_MW_CMD_MEDIA_START_PLAYBACK_AUDIO_FILE(void *msg_data,
                                                      int msg_data_size,
                                                      void *result_addr,
                                                      int result_max_size);
void ON_AM_IPC_MW_CMD_MEDIA_PAUSE_PLAYBACK_AUDIO_FILE(void *msg_data,
                                                      int msg_data_size,
                                                      void *result_addr,
                                                      int result_max_size);
void ON_AM_IPC_MW_CMD_MEDIA_STOP_PLAYBACK_AUDIO_FILE(void *msg_data,
                                                     int msg_data_size,
                                                     void *result_addr,
                                                     int result_max_size);
void ON_AM_IPC_MW_CMD_MEDIA_START_RECORDING(void *msg_data,
                                            int msg_data_size,
                                            void *result_addr,
                                            int result_max_size);
void ON_AM_IPC_MW_CMD_MEDIA_STOP_RECORDING(void *msg_data,
                                           int msg_data_size,
                                           void *result_addr,
                                           int result_max_size);
void ON_AM_IPC_MW_CMD_MEDIA_START_FILE_RECORDING(void *msg_data,
                                                 int msg_data_size,
                                                 void *result_addr,
                                                 int result_max_size);
void ON_AM_IPC_MW_CMD_MEDIA_STOP_FILE_RECORDING(void *msg_data,
                                                int msg_data_size,
                                                void *result_addr,
                                                int result_max_size);
void ON_AM_IPC_MW_CMD_MEDIA_SET_RECORDING_FILE_NUM(void *msg_data,
                                                   int msg_data_size,
                                                   void *result_addr,
                                                   int result_max_size);
void ON_AM_IPC_MW_CMD_MEDIA_SET_RECORDING_DURATION(void *msg_data,
                                                   int msg_data_size,
                                                   void *result_addr,
                                                   int result_max_size);
void ON_AM_IPC_MW_CMD_MEDIA_ENABLE_AUDIO_CODEC(void *msg_data,
                                               int msg_data_size,
                                               void *result_addr,
                                               int result_max_size);
BEGIN_MSG_MAP(API_PROXY_TO_MEDIA_SERVICE)
MSG_ACTION(AM_IPC_SERVICE_INIT, ON_SERVICE_INIT)
MSG_ACTION(AM_IPC_SERVICE_DESTROY, ON_SERVICE_DESTROY)
MSG_ACTION(AM_IPC_SERVICE_START, ON_SERVICE_START)
MSG_ACTION(AM_IPC_SERVICE_STOP, ON_SERVICE_STOP)
MSG_ACTION(AM_IPC_SERVICE_RESTART, ON_SERVICE_RESTART)
MSG_ACTION(AM_IPC_SERVICE_STATUS, ON_SERVICE_STATUS)
MSG_ACTION(AM_IPC_MW_CMD_MEDIA_EVENT_RECORDING_START,
           ON_AM_IPC_MW_CMD_MEDIA_EVENT_RECORDING_START)
MSG_ACTION(AM_IPC_MW_CMD_MEDIA_PERIODIC_JPEG_RECORDING,
           ON_AM_IPC_MW_CMD_MEDIA_PERIODIC_JPEG_RECORDING)
MSG_ACTION(AM_IPC_MW_CMD_MEDIA_ADD_AUDIO_FILE,
           ON_AM_IPC_MW_CMD_MEDIA_ADD_AUDIO_FILE)
MSG_ACTION(AM_IPC_MW_CMD_MEDIA_START_PLAYBACK_AUDIO_FILE,
           ON_AM_IPC_MW_CMD_MEDIA_START_PLAYBACK_AUDIO_FILE)
MSG_ACTION(AM_IPC_MW_CMD_MEDIA_PAUSE_PLAYBACK_AUDIO_FILE,
           ON_AM_IPC_MW_CMD_MEDIA_PAUSE_PLAYBACK_AUDIO_FILE)
MSG_ACTION(AM_IPC_MW_CMD_MEDIA_STOP_PLAYBACK_AUDIO_FILE,
           ON_AM_IPC_MW_CMD_MEDIA_STOP_PLAYBACK_AUDIO_FILE)
MSG_ACTION(AM_IPC_MW_CMD_MEDIA_START_RECORDING,
           ON_AM_IPC_MW_CMD_MEDIA_START_RECORDING)
MSG_ACTION(AM_IPC_MW_CMD_MEDIA_STOP_RECORDING,
           ON_AM_IPC_MW_CMD_MEDIA_STOP_RECORDING)
MSG_ACTION(AM_IPC_MW_CMD_MEDIA_START_FILE_RECORDING,
           ON_AM_IPC_MW_CMD_MEDIA_START_FILE_RECORDING)
MSG_ACTION(AM_IPC_MW_CMD_MEDIA_STOP_FILE_RECORDING,
           ON_AM_IPC_MW_CMD_MEDIA_STOP_FILE_RECORDING)
MSG_ACTION(AM_IPC_MW_CMD_MEDIA_SET_RECORDING_FILE_NUM,
           ON_AM_IPC_MW_CMD_MEDIA_SET_RECORDING_FILE_NUM)
MSG_ACTION(AM_IPC_MW_CMD_MEDIA_SET_RECORDING_DURATION,
           ON_AM_IPC_MW_CMD_MEDIA_SET_RECORDING_DURATION)
MSG_ACTION(AM_IPC_MW_CMD_MEDIA_ENABLE_AUDIO_CODEC,
           ON_AM_IPC_MW_CMD_MEDIA_ENABLE_AUDIO_CODEC)
END_MSG_MAP()


#endif /* AM_MEDIA_SERVICE_MSG_MAP_H_ */
