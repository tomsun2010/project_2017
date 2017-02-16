/*******************************************************************************
 * am_record.cpp
 *
 * History:
 *   2014-12-31 - [ypchang] created file
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

#include "am_record.h"

#include "am_amf_types.h"
#include "am_amf_interface.h"
#include "am_record_engine_if.h"

AMRecord *AMRecord::m_instance = nullptr;
std::mutex AMRecord::m_lock;

AMIRecordPtr AMIRecord::create()
{
  return AMRecord::get_instance();
}

AMIRecord* AMRecord::get_instance()
{
  m_lock.lock();

  if (AM_LIKELY(!m_instance)) {
    m_instance = new AMRecord();
    if (AM_UNLIKELY(!m_instance)) {
      ERROR("Failed to create instance of AMRecord!");
    }
  }

  m_lock.unlock();

  return m_instance;
}

bool AMRecord::start()
{
  return (m_engine && m_engine->record());
}

bool AMRecord::stop()
{
  return (m_engine && m_engine->stop());
}

bool AMRecord::start_file_recording(uint32_t muxer_id)
{
  INFO("start muxer%u file recording", muxer_id);
  return (m_engine && m_engine->start_file_recording(muxer_id));
}

bool AMRecord::stop_file_recording(uint32_t muxer_id)
{
  INFO("stop muxer%u file recording.", muxer_id);
  return (m_engine && m_engine->stop_file_recording(muxer_id));
}

bool AMRecord::set_recording_file_num(uint32_t muxer_id, uint32_t file_num)
{
  return (m_engine && m_engine->set_recording_file_num(muxer_id, file_num));
}

bool AMRecord::set_recording_duration(uint32_t muxer_id, int32_t duration)
{
  return (m_engine && m_engine->set_recording_duration(muxer_id, duration));
}

bool AMRecord::is_recording()
{
  return (m_engine && (m_engine->get_engine_status() ==
      AMIRecordEngine::AM_RECORD_ENGINE_RECORDING));
}

bool AMRecord::enable_audio_codec(AM_AUDIO_TYPE type, uint32_t sample_rate,
                            bool enable)
{
  return (m_engine && (m_engine->enable_audio_codec(type, sample_rate, enable)));
}

bool AMRecord::is_ready_for_event(AMEventStruct& event)
{
  return (m_engine && (m_engine->is_ready_for_event(event)));
}

bool AMRecord::send_event(AMEventStruct& event)
{
  return (m_engine && m_engine->send_event(event));
}

bool AMRecord::init()
{
  if (AM_LIKELY(!m_engine)) {
    m_engine = AMIRecordEngine::create();
  }

  if (AM_LIKELY(!m_is_initialized)) {
    m_is_initialized = m_engine && m_engine->create_graph();
  }

  return m_is_initialized;
}

void AMRecord::set_msg_callback(AMRecordCallback callback, void *data)
{
  if (AM_LIKELY(m_engine)) {
    m_engine->set_app_msg_callback(callback, data);
  }
}

void AMRecord::release()
{
  if ((m_ref_count >= 0) && (--m_ref_count <= 0)) {
    NOTICE("Last reference of AMRecord's object %p, release it!", m_instance);
    delete m_instance;
    m_instance = nullptr;
  }
}

void AMRecord::inc_ref()
{
  ++ m_ref_count;
}

AMRecord::AMRecord() :
    m_engine(NULL),
    m_is_initialized(false),
    m_ref_count(0)
{}

AMRecord::~AMRecord()
{
  AM_DESTROY(m_engine);
  DEBUG("~AMRecord");
}
