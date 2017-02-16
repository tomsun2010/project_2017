/*******************************************************************************
 * am_efm_src_service_main.cpp
 *
 * History:
 *   2016-05-04 - [Zhi He] created file
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
#include "am_define.h"
#include "am_log.h"
#include "am_efm_src_service_msg_map.h"
#include "am_pid_lock.h"
#include "am_service_frame_if.h"
#include "am_api_efm_src.h"
#include "am_signal.h"

AM_SERVICE_STATE  g_service_state  = AM_SERVICE_STATE_NOT_INIT;
AMIServiceFrame  *g_service_frame  = nullptr;

static void user_input_callback(char ch)
{
  switch(ch) {
    case 'Q':
    case 'q': {
      NOTICE("Quit Media.Service!");
      g_service_frame->quit();
    }break;
    default: break;
  }
}

int main(int argc, char *argv[])
{
  int ret = 0;
  AMIPCSyncCmdServer ipc;
  g_service_frame = AMIServiceFrame::create("EFMSource.Service");

  if (AM_LIKELY(g_service_frame)) {
    signal(SIGINT,  SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    register_critical_error_signal_handler();

    do {
      AMPIDLock lock;
      if ((argc > 1) && is_str_equal(argv[1], "debug")) {
        NOTICE("Running EFMSource service in debug mode, press 'q' to exit!");
        g_service_frame->set_user_input_callback(user_input_callback);
      } else {
        if (AM_LIKELY(lock.try_lock() < 0)) {
          ERROR("Unable to lock PID, EFM Source service is already running!");
          ret = -1;
          break;
        }
        if (ipc.create(AM_IPC_EFM_SOURCE_NAME) < 0) {
          ret = -1;
          g_service_state = AM_SERVICE_STATE_ERROR;
          break;
        } else {
          g_service_state = AM_SERVICE_STATE_INIT_PROCESS_CREATED;
          ipc.REGISTER_MSG_MAP(API_PROXY_TO_EFM_SRC_SERVICE);
          ipc.complete();
        }
      }
      g_service_state = AM_SERVICE_STATE_INIT_DONE;
      NOTICE("Entering EFM Source service main loop!");
      g_service_frame->run();
      NOTICE("Exit EFM Source service main loop!");
    } while (0);

    g_service_frame->destroy();
    g_service_frame = nullptr;
    PRINTF("EFM Source service destroyed!");
  } else {
    ERROR("Failed to create service framework for EFM Source service!");
    g_service_state = AM_SERVICE_STATE_ERROR;
    ret = -3;
  }

  return ret;
}
