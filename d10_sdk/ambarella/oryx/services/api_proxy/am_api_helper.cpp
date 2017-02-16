/*******************************************************************************
 * am_api_helper.cpp
 *
 * History:
 *   2014-9-28 - [lysun] created file
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

#include "am_api_helper.h"
#include "am_ipc_sync_cmd.h"

#include <mutex>
#include <signal.h>

#include "commands/am_service_impl.h"

AMAPIHelper * AMAPIHelper::m_instance = nullptr;

static std::mutex m_mtx;
#define  DECLARE_MUTEX  std::lock_guard<std::mutex> lck (m_mtx);

AMAPIHelper::AMAPIHelper() :
    m_air_api_ipc(nullptr),
    m_notify_cb(nullptr),
    m_ref_counter(0)
{

}

int AMAPIHelper::construct()
{
  int ret = 0;
  do {
    m_air_api_ipc = new AMIPCSyncCmdClient();
    if (!m_air_api_ipc) {
      ret = -1;
      ERROR("fail to create air api ipc\n");
      break;
    } else {
      if (m_air_api_ipc->create(AM_IPC_API_PROXY_NAME) < 0) {
        ret = -2;
        ERROR("fail to create air api ipc \n");
        break;
      }
    }
  } while (0);

  if (ret < 0) {
    delete m_air_api_ipc;
    m_air_api_ipc = nullptr;
  }

  return ret;
}

AMAPIHelperPtr AMAPIHelper::get_instance()
{
  DECLARE_MUTEX;
  if (!m_instance) {
    if ((m_instance = new AMAPIHelper()) && (m_instance->construct() < 0)) {
      ERROR("AMAPIHelper: construct error\n");
      delete m_instance;
      m_instance = nullptr;
    }
  }

  return m_instance;
}

void AMAPIHelper::release()
{
  DECLARE_MUTEX;
  if((m_ref_counter) > 0 && (--m_ref_counter == 0)) {
    delete m_instance;
    m_instance = nullptr;
  }
}

void AMAPIHelper::inc_ref()
{
  ++ m_ref_counter;
}

AMAPIHelper::~AMAPIHelper()
{
  delete m_air_api_ipc;
}

void AMAPIHelper::method_call(uint32_t cmd_id,
                              void *msg_data,
                              int msg_data_size,
                              void *result_addr,
                              int result_max_size)
{
  DECLARE_MUTEX;
  uint8_t cmd_buf[AM_MAX_IPC_MESSAGE_SIZE] = {0};
  am_ipc_message_header_t msg_header;
  if (!m_air_api_ipc) {
    ERROR("air api not setup, cannot do method call\n");
    return;
  }

  //pack the cmd into uniform method call

  if (msg_data_size + sizeof(am_ipc_message_header_t) >
      AM_MAX_IPC_MESSAGE_SIZE) {
    ERROR("unable to pack cmd 0x%x into AIR API container, total size %d\n",
        cmd_id, msg_data_size + sizeof(am_ipc_message_header_t));
    return;
  }

  memset(&msg_header, 0, sizeof(msg_header));
  msg_header.msg_id = cmd_id;
  msg_header.header_size = sizeof(msg_header);
  msg_header.payload_size = msg_data_size;
  //msg_header.time_stamp =
  memcpy(cmd_buf, &msg_header, sizeof(msg_header));//copy cmd header
  //copy cmd payload
  memcpy(cmd_buf + sizeof(msg_header), msg_data, msg_data_size);

  //call universal cmd that can pack the original cmd into
  //AM_IPC_MW_CMD_AIR_API_CONTAINER
  m_air_api_ipc->method_call(AM_IPC_MW_CMD_AIR_API_CONTAINER,
      cmd_buf, sizeof(msg_header) + msg_data_size,
      result_addr, result_max_size);
}

void AMAPIHelper::on_notify_callback(uint32_t context,
                                      void *msg_data,
                                      int msg_data_size,
                                      void *result_addr,
                                      int result_max_size)
{
  AMAPIHelper *pThis = nullptr;
  if (context == 0) {
    ERROR("AMAPIHelper: Null context in callback return \n");
  } else {
    pThis = (AMAPIHelper*)context;
  }

  if (pThis && pThis->m_notify_cb) {
    pThis->m_notify_cb(msg_data, msg_data_size);
  }
}

int AMAPIHelper::register_notify_cb(AM_IPC_NOTIFY_CB cb)
{
  int ret = 0;
  am_msg_handler_t msg_handler;
  do {
    if (!m_air_api_ipc) {
      ERROR("air api not setup, cannot do method call\n");
      ret = -1;
      break;
    }

    memset(&msg_handler, 0, sizeof(msg_handler));
    msg_handler.msgid = AM_IPC_SERVICE_NOTIF;
    msg_handler.callback = nullptr;
    msg_handler.context = (uint32_t) this;
    msg_handler.callback_ct = AMAPIHelper::on_notify_callback;

    //register the single entry msg map
    if (m_air_api_ipc->register_msg_map(&msg_handler, 1) < 0) {
      ERROR("AMAPIHelper: register msg map \n");
      ret = -2;
      break;
    }
    m_notify_cb = cb;
  } while (0);
  return ret;
}
