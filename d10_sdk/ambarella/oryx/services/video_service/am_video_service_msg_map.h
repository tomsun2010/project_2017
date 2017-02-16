/*******************************************************************************
 * am_video_service_msg_map.h
 *
 * History:
 *   2014-9-17 - [lysun] created file
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
#ifndef AM_VIDEO_SERVICE_MSG_MAP_H_
#define AM_VIDEO_SERVICE_MSG_MAP_H_
#include "commands/am_api_cmd_video.h"
#include "commands/am_api_cmd_common.h"

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

void ON_CFG_ALL_LOAD(void *msg_data,
                     int msg_data_size,
                     void *result_addr,
                     int result_max_size);

void ON_CFG_FEATURE_GET(void *msg_data,
                     int msg_data_size,
                     void *result_addr,
                     int result_max_size);

void ON_CFG_FEATURE_SET(void *msg_data,
                     int msg_data_size,
                     void *result_addr,
                     int result_max_size);

void ON_CFG_VIN_GET(void *msg_data,
                    int msg_data_size,
                    void *result_addr,
                    int result_max_size);
void ON_CFG_VIN_SET(void *msg_data,
                    int msg_data_size,
                    void *result_addr,
                    int result_max_size);

void ON_CFG_VOUT_GET(void *msg_data,
                     int msg_data_size,
                     void *result_addr,
                     int result_max_size);
void ON_CFG_VOUT_SET(void *msg_data,
                     int msg_data_size,
                     void *result_addr,
                     int result_max_size);

void ON_CFG_BUFFER_GET(void *msg_data,
                       int msg_data_size,
                       void *result_addr,
                       int result_max_size);
void ON_CFG_BUFFER_SET(void *msg_data,
                       int msg_data_size,
                       void *result_addr,
                       int result_max_size);

void ON_CFG_STREAM_FMT_GET(void *msg_data,
                           int msg_data_size,
                           void *result_addr,
                           int result_max_size);
void ON_CFG_STREAM_FMT_SET(void *msg_data,
                           int msg_data_size,
                           void *result_addr,
                           int result_max_size);

void ON_CFG_STREAM_H26x_GET(void *msg_data,
                            int msg_data_size,
                            void *result_addr,
                            int result_max_size);
void ON_CFG_STREAM_H26x_SET(void *msg_data,
                            int msg_data_size,
                            void *result_addr,
                            int result_max_size);

void ON_CFG_STREAM_MJPEG_GET(void *msg_data,
                             int msg_data_size,
                             void *result_addr,
                             int result_max_size);
void ON_CFG_STREAM_MJPEG_SET(void *msg_data,
                             int msg_data_size,
                             void *result_addr,
                             int result_max_size);

void ON_DYN_VOUT_HALT(void *msg_data,
                      int msg_data_size,
                      void *result_addr,
                      int result_max_size);

void ON_DYN_BUFFER_STATE_GET(void *msg_data,
                             int msg_data_size,
                             void *result_addr,
                             int result_max_size);

void ON_DYN_BUFFER_FMT_GET(void *msg_data,
                           int msg_data_size,
                           void *result_addr,
                           int result_max_size);
void ON_DYN_BUFFER_FMT_SET(void *msg_data,
                           int msg_data_size,
                           void *result_addr,
                           int result_max_size);

void ON_DYN_STREAM_MAX_NUM_GET(void *msg_data,
                               int msg_data_size,
                               void *result_addr,
                               int result_max_size);

void ON_DYN_BUFFER_MAX_NUM_GET(void *msg_data,
                               int msg_data_size,
                               void *result_addr,
                               int result_max_size);

void ON_DYN_STREAM_STATUS_GET(void *msg_data,
                              int msg_data_size,
                              void *result_addr,
                              int result_max_size);

void ON_DYN_STREAM_START(void *msg_data,
                         int msg_data_size,
                         void *result_addr,
                         int result_max_size);

void ON_DYN_STREAM_STOP(void *msg_data,
                        int msg_data_size,
                        void *result_addr,
                        int result_max_size);

void ON_DYN_STREAM_BITRATE_GET(void *msg_data,
                               int msg_data_size,
                               void *result_addr,
                               int result_max_size);

void ON_DYN_STREAM_BITRATE_SET(void *msg_data,
                               int msg_data_size,
                               void *result_addr,
                               int result_max_size);

void ON_DYN_STREAM_FRAMERATE_GET(void *msg_data,
                                 int msg_data_size,
                                 void *result_addr,
                                 int result_max_size);

void ON_DYN_STREAM_FRAMERATE_SET(void *msg_data,
                                 int msg_data_size,
                                 void *result_addr,
                                 int result_max_size);

void ON_DYN_MJPEG_QUALITY_GET(void *msg_data,
                              int msg_data_size,
                              void *result_addr,
                              int result_max_size);

void ON_DYN_MJPEG_QUALITY_SET(void *msg_data,
                              int msg_data_size,
                              void *result_addr,
                              int result_max_size);

void ON_DYN_H26x_GOP_GET(void *msg_data,
                         int msg_data_size,
                         void *result_addr,
                         int result_max_size);

void ON_DYN_H26x_GOP_SET(void *msg_data,
                         int msg_data_size,
                         void *result_addr,
                         int result_max_size);

void ON_DYN_STREAM_TYPE_GET(void *msg_data,
                            int msg_data_size,
                            void *result_addr,
                            int result_max_size);

void ON_DYN_STREAM_TYPE_SET(void *msg_data,
                            int msg_data_size,
                            void *result_addr,
                            int result_max_size);

void ON_DYN_STREAM_SIZE_GET(void *msg_data,
                            int msg_data_size,
                            void *result_addr,
                            int result_max_size);

void ON_DYN_STREAM_SIZE_SET(void *msg_data,
                            int msg_data_size,
                            void *result_addr,
                            int result_max_size);

void ON_DYN_STREAM_OFFSET_GET(void *msg_data,
                              int msg_data_size,
                              void *result_addr,
                              int result_max_size);

void ON_DYN_STREAM_OFFSET_SET(void *msg_data,
                              int msg_data_size,
                              void *result_addr,
                              int result_max_size);

void ON_VIN_SET(void *msg_data,
                int msg_data_size,
                void *result_addr,
                int result_max_size);

void ON_VIN_STOP(void *msg_data,
                 int msg_data_size,
                 void *result_addr,
                 int result_max_size);

void ON_STREAM_FMT_GET(void *msg_data,
                       int msg_data_size,
                       void *result_addr,
                       int result_max_size);
void ON_STREAM_FMT_SET(void *msg_data,
                       int msg_data_size,
                       void *result_addr,
                       int result_max_size);

void ON_STREAM_CFG_GET(void *msg_data,
                       int msg_data_size,
                       void *result_addr,
                       int result_max_size);
void ON_STREAM_CFG_SET(void *msg_data,
                       int msg_data_size,
                       void *result_addr,
                       int result_max_size);

void ON_DYN_DPTZ_RATIO_SET(void *msg_data,
                           int msg_data_size,
                           void *result_addr,
                           int result_max_size);

void ON_DYN_DPTZ_RATIO_GET(void *msg_data,
                           int msg_data_size,
                           void *result_addr,
                           int result_max_size);

void ON_DYN_DPTZ_SIZE_SET(void *msg_data,
                          int msg_data_size,
                          void *result_addr,
                          int result_max_size);

void ON_DYN_DPTZ_SIZE_GET(void *msg_data,
                          int msg_data_size,
                          void *result_addr,
                          int result_max_size);

void ON_DYN_WARP_SET(void *msg_data,
                     int msg_data_size,
                     void *result_addr,
                     int result_max_size);

void ON_DYN_WARP_GET(void *msg_data,
                     int msg_data_size,
                     void *result_addr,
                     int result_max_size);

void ON_DYN_LBR_SET(void *msg_data,
                    int msg_data_size,
                    void *result_addr,
                    int result_max_size);

void ON_DYN_LBR_GET(void *msg_data,
                    int msg_data_size,
                    void *result_addr,
                    int result_max_size);

void ON_VIDEO_ENCODE_START(void *msg_data,
                           int msg_data_size,
                           void *result_addr,
                           int result_max_size);
void ON_VIDEO_ENCODE_STOP(void *msg_data,
                          int msg_data_size,
                          void *result_addr,
                          int result_max_size);

void ON_VIDEO_DYN_FORCE_IDR(void *msg_data,
                            int msg_data_size,
                            void *result_addr,
                            int result_max_size);

void ON_COMMON_GET_EVENT(void *msg_data,
                         int msg_data_size,
                         void *result_addr,
                         int result_max_size);

void ON_VIDEO_OVERLAY_GET_MAX_NUM(void *msg_data,
                                  int msg_data_size,
                                  void *result_addr,
                                  int result_max_size);

void ON_VIDEO_OVERLAY_DESTROY(void *msg_data,
                              int msg_data_size,
                              void *result_addr,
                              int result_max_size);

void ON_VIDEO_OVERLAY_SAVE(void *msg_data,
                           int msg_data_size,
                           void *result_addr,
                           int result_max_size);

void ON_VIDEO_OVERLAY_INIT(void *msg_data,
                           int msg_data_size,
                           void *result_addr,
                           int result_max_size);

void ON_VIDEO_OVERLAY_DATA_ADD(void *msg_data,
                              int msg_data_size,
                              void *result_addr,
                              int result_max_size);

void ON_VIDEO_OVERLAY_DATA_UPDATE(void *msg_data,
                                  int msg_data_size,
                                  void *result_addr,
                                  int result_max_size);

void ON_VIDEO_OVERLAY_DATA_GET(void *msg_data,
                               int msg_data_size,
                               void *result_addr,
                               int result_max_size);

void ON_VIDEO_OVERLAY_SET(void *msg_data,
                          int msg_data_size,
                          void *result_addr,
                          int result_max_size);

void ON_VIDEO_OVERLAY_GET(void *msg_data,
                          int msg_data_size,
                          void *result_addr,
                          int result_max_size);

void ON_VIDEO_EIS_SET(void *msg_data,
                      int msg_data_size,
                      void *result_addr,
                      int result_max_size);

void ON_VIDEO_EIS_GET(void *msg_data,
                      int msg_data_size,
                      void *result_addr,
                      int result_max_size);

void ON_VIDEO_MOTION_DETECT_SET(void *msg_data,
                                int msg_data_size,
                                void *result_addr,
                                int result_max_size);

void ON_VIDEO_MOTION_DETECT_GET(void *msg_data,
                                int msg_data_size,
                                void *result_addr,
                                int result_max_size);

void ON_VIDEO_MOTION_DETECT_STOP(void *msg_data,
                                 int msg_data_size,
                                 void *result_addr,
                                 int result_max_size);

void ON_VIDEO_MOTION_DETECT_START(void *msg_data,
                                  int msg_data_size,
                                  void *result_addr,
                                  int result_max_size);

BEGIN_MSG_MAP(API_PROXY_TO_VIDEO_SERVICE)
MSG_ACTION(AM_IPC_SERVICE_INIT, ON_SERVICE_INIT)
MSG_ACTION(AM_IPC_SERVICE_DESTROY, ON_SERVICE_DESTROY)
MSG_ACTION(AM_IPC_SERVICE_START, ON_SERVICE_START)
MSG_ACTION(AM_IPC_SERVICE_STOP, ON_SERVICE_STOP)
MSG_ACTION(AM_IPC_SERVICE_RESTART, ON_SERVICE_RESTART)
MSG_ACTION(AM_IPC_SERVICE_STATUS, ON_SERVICE_STATUS)

MSG_ACTION(AM_IPC_MW_CMD_VIDEO_CFG_ALL_LOAD, ON_CFG_ALL_LOAD)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_CFG_FEATURE_GET, ON_CFG_FEATURE_GET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_CFG_FEATURE_SET, ON_CFG_FEATURE_SET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_CFG_VIN_GET, ON_CFG_VIN_GET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_CFG_VIN_SET, ON_CFG_VIN_SET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_CFG_BUFFER_GET, ON_CFG_BUFFER_GET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_CFG_BUFFER_SET, ON_CFG_BUFFER_SET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_CFG_STREAM_FMT_GET, ON_CFG_STREAM_FMT_GET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_CFG_STREAM_FMT_SET, ON_CFG_STREAM_FMT_SET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_CFG_STREAM_H26x_GET, ON_CFG_STREAM_H26x_GET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_CFG_STREAM_H26x_SET, ON_CFG_STREAM_H26x_SET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_CFG_STREAM_MJPEG_GET, ON_CFG_STREAM_MJPEG_GET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_CFG_STREAM_MJPEG_SET, ON_CFG_STREAM_MJPEG_SET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_CFG_VOUT_SET, ON_CFG_VOUT_SET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_CFG_VOUT_GET, ON_CFG_VOUT_GET)

MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_VOUT_HALT, ON_DYN_VOUT_HALT)

MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_BUFFER_STATE_GET, ON_DYN_BUFFER_STATE_GET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_BUFFER_FMT_GET, ON_DYN_BUFFER_FMT_GET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_BUFFER_FMT_SET, ON_DYN_BUFFER_FMT_SET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_STREAM_MAX_NUM_GET, ON_DYN_STREAM_MAX_NUM_GET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_BUFFER_MAX_NUM_GET, ON_DYN_BUFFER_MAX_NUM_GET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_LBR_SET, ON_DYN_LBR_SET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_LBR_GET, ON_DYN_LBR_GET)

MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_STREAM_START, ON_DYN_STREAM_START)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_STREAM_STOP, ON_DYN_STREAM_STOP)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_STREAM_BITRATE_GET, ON_DYN_STREAM_BITRATE_GET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_STREAM_BITRATE_SET, ON_DYN_STREAM_BITRATE_SET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_STREAM_FRAMERATE_GET, ON_DYN_STREAM_FRAMERATE_GET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_STREAM_FRAMERATE_SET, ON_DYN_STREAM_FRAMERATE_SET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_MJPEG_QUALITY_GET, ON_DYN_MJPEG_QUALITY_GET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_MJPEG_QUALITY_SET, ON_DYN_MJPEG_QUALITY_SET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_H26x_GOP_GET, ON_DYN_H26x_GOP_GET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_H26x_GOP_SET, ON_DYN_H26x_GOP_SET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_STREAM_TYPE_GET, ON_DYN_STREAM_TYPE_GET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_STREAM_TYPE_SET, ON_DYN_STREAM_TYPE_SET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_STREAM_SIZE_GET, ON_DYN_STREAM_SIZE_GET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_STREAM_SIZE_SET, ON_DYN_STREAM_SIZE_SET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_STREAM_OFFSET_GET, ON_DYN_STREAM_OFFSET_GET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_STREAM_OFFSET_SET, ON_DYN_STREAM_OFFSET_SET)

MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_FORCE_IDR, ON_VIDEO_DYN_FORCE_IDR)

MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_STREAM_STATUS_GET, ON_DYN_STREAM_STATUS_GET)

MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_DPTZ_RATIO_SET, ON_DYN_DPTZ_RATIO_SET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_DPTZ_RATIO_GET, ON_DYN_DPTZ_RATIO_GET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_DPTZ_SIZE_SET, ON_DYN_DPTZ_SIZE_SET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_DPTZ_SIZE_GET, ON_DYN_DPTZ_SIZE_GET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_WARP_SET, ON_DYN_WARP_SET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_WARP_GET, ON_DYN_WARP_GET)

MSG_ACTION(AM_IPC_MW_CMD_VIDEO_ENCODE_START, ON_VIDEO_ENCODE_START)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_ENCODE_STOP, ON_VIDEO_ENCODE_STOP)

MSG_ACTION(AM_IPC_MW_CMD_COMMON_GET_EVENT, ON_COMMON_GET_EVENT)

MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_OVERLAY_MAX_NUM_GET,
           ON_VIDEO_OVERLAY_GET_MAX_NUM)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_OVERLAY_DESTROY, ON_VIDEO_OVERLAY_DESTROY)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_OVERLAY_SAVE, ON_VIDEO_OVERLAY_SAVE)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_OVERLAY_INIT, ON_VIDEO_OVERLAY_INIT)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_OVERLAY_DATA_ADD, ON_VIDEO_OVERLAY_DATA_ADD)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_OVERLAY_DATA_UPDATE,
           ON_VIDEO_OVERLAY_DATA_UPDATE)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_OVERLAY_DATA_GET, ON_VIDEO_OVERLAY_DATA_GET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_OVERLAY_SET, ON_VIDEO_OVERLAY_SET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_OVERLAY_GET, ON_VIDEO_OVERLAY_GET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_EIS_SET, ON_VIDEO_EIS_SET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_EIS_GET, ON_VIDEO_EIS_GET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_MOTION_DETECT_CONFIG_SET,
           ON_VIDEO_MOTION_DETECT_SET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_MOTION_DETECT_CONFIG_GET,
           ON_VIDEO_MOTION_DETECT_GET)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_MOTION_DETECT_STOP,
           ON_VIDEO_MOTION_DETECT_STOP)
MSG_ACTION(AM_IPC_MW_CMD_VIDEO_DYN_MOTION_DETECT_START,
           ON_VIDEO_MOTION_DETECT_START)

END_MSG_MAP()

#endif /* AM_VIDEO_SERVICE_MSG_MAP_H_ */
