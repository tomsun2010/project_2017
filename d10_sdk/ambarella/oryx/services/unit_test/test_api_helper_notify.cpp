/*******************************************************************************
 * test_api_helper_notify.cpp
 *
 * History:
 *   2015-12-1 - [zfgong] created file
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include "am_base_include.h"
#include "am_log.h"
#include "am_api_helper.h"
#include "am_air_api.h"
#include "am_api_event.h"
#include "event/am_event_types.h"
#include "video/am_video_types.h"
#include "jpeg_snapshot/am_jpeg_snapshot_if.h"


static AMAPIHelperPtr api_helper = nullptr;
static AMIJpegSnapshotPtr jpeg_snapshot = nullptr;
static bool m_snapshot_by_long_pressed = false;

static void sigstop(int arg)
{
  PRINTF("catch sigstop, exit.");
  exit(1);
}

static int on_snapshot(void *data, int len)
{
  INFO("on_snapshot len = %d, add your data handle code to here", len);
  if (m_snapshot_by_long_pressed == true) {
    m_snapshot_by_long_pressed = false;
    jpeg_snapshot->capture_stop();
  }
  //there is no need to free data here!
  return 0;
}

static int motion_handle(AM_EVENT_MESSAGE *event_msg)
{
  const char *motion_type[AM_MD_MOTION_TYPE_NUM] =
  {"MOTION_NONE", "MOTION_START", "MOTION_LEVEL_CHANGED", "MOTION_END"};
  const char *motion_level[AM_MOTION_L_NUM] =
  {"MOTION_LEVEL_0", "MOTION_LEVEL_1", "MOTION_LEVEL_2"};
  PRINTF("pts: %llu, event number: %llu, %s, %s, ROI#%d",
         event_msg->pts,
         event_msg->seq_num,
         motion_type[event_msg->md_msg.mt_type],
         motion_level[event_msg->md_msg.mt_level],
         event_msg->md_msg.roi_id);
  switch (event_msg->md_msg.mt_type) {
  case AM_MD_MOTION_START:
  case AM_MD_MOTION_LEVEL_CHANGED:
    PRINTF("start jpeg snap shot");
    jpeg_snapshot->capture_start();
    break;
  case AM_MD_MOTION_END:
    PRINTF("stop jpeg snap shot");
    jpeg_snapshot->capture_stop();
    break;
  default:
    break;
  }

  return 0;
}

static int audio_alert_handle(AM_EVENT_MESSAGE *event_msg)
{
  PRINTF("add your handle code");
  return 0;
}

static int audio_analysis_handle(AM_EVENT_MESSAGE *event_msg)
{
  PRINTF("add your handle code");
  return 0;
}

static int face_handle(AM_EVENT_MESSAGE *event_msg)
{
  PRINTF("add your handle code");
  return 0;
}

static int key_handle(AM_EVENT_MESSAGE *event_msg)
{
  const char *key_state[AM_KEY_STATE_NUM] =
  {" ", "UP", "DOWN", "CLICKED", "LONG_PRESS" };
  PRINTF("\npts: %llu, event number: %llu, key code: %d, key state: %s\n",
       event_msg->pts,
       event_msg->seq_num,
       event_msg->key_event.key_value,
       key_state[event_msg->key_event.key_state]);

  switch (event_msg->key_event.key_state) {
  case AM_KEY_UP:
    PRINTF("key up");
    break;
  case AM_KEY_DOWN:
    PRINTF("key down");
    break;
  case AM_KEY_CLICKED:
    PRINTF("key clicked");
    break;
  case AM_KEY_LONG_PRESSED:
    PRINTF("key long pressed, capture one jpeg");
    jpeg_snapshot->capture_start();
    m_snapshot_by_long_pressed = true;
    jpeg_snapshot->set_source_buffer(AM_SOURCE_BUFFER_2ND);
    break;
  default:
    ERROR("unknown key event state");
    break;
  }

  return 0;
}

static int on_notify(void *msg_data, int msg_data_size)
{
  am_service_notify_payload *payload = (am_service_notify_payload *)msg_data;
  AM_EVENT_MESSAGE *event_msg = (AM_EVENT_MESSAGE *)payload->data;

  switch(event_msg->event_type) {
  case EV_MOTION_DECT:
    motion_handle(event_msg);
    break;
  case EV_AUDIO_ALERT_DECT:
    audio_alert_handle(event_msg);
    break;
  case EV_AUDIO_ANALYSIS_DECT:
    audio_analysis_handle(event_msg);
    break;
  case EV_FACE_DECT:
    face_handle(event_msg);
    break;
  case EV_KEY_INPUT_DECT:
    key_handle(event_msg);
    break;
  default:
    ERROR("unknown event_type");
    break;
  }

  return 0;
}

int main()
{
  signal(SIGINT, sigstop);
  signal(SIGQUIT, sigstop);
  signal(SIGTERM, sigstop);

  AM_RESULT result;
  jpeg_snapshot = AMIJpegSnapshot::get_instance();
  if (!jpeg_snapshot) {
    ERROR("unable to get AMIJpegSnapshotPtr instance\n");
    return -1;
  }
  string path("/mnt/jpeg");
  jpeg_snapshot->set_source_buffer(AM_SOURCE_BUFFER_MAIN);
  jpeg_snapshot->set_file_max_num(100);
  jpeg_snapshot->set_file_path(path);
  jpeg_snapshot->set_fps(1.0);
  jpeg_snapshot->save_file_enable();
  //if handle snapped data by yourself, please call set_data_cb
  jpeg_snapshot->set_data_cb(on_snapshot);
  result = jpeg_snapshot->run();
  if (result != AM_RESULT_OK) {
    ERROR("AMEncodeDevice: jpeg encoder start failed \n");
    return -1;
  }

  api_helper = AMAPIHelper::get_instance();
  if (!api_helper) {
    ERROR("unable to get AMAPIHelper instance\n");
    return -1;
  }
  api_helper->register_notify_cb(on_notify);
  while (1) {
    sleep(1);
  }

  return 0;
}
