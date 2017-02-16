/*******************************************************************************
 * am_record.h
 *
 * History:
 *   2014-12-2 - [ypchang] created file
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
#ifndef AM_RECORD_H_
#define AM_RECORD_H_

#include "am_record_if.h"
#include <atomic>
#include <mutex>

class AMIRecordEngine;
class AMRecord: public AMIRecord
{
  public:
    static AMIRecord* get_instance();

  public:
    bool start();
    bool stop();
    bool start_file_recording(uint32_t muxer_id);
    bool stop_file_recording(uint32_t muxer_id);
    bool set_recording_file_num(uint32_t muxer_id, uint32_t file_num);
    bool set_recording_duration(uint32_t muxer_id, int32_t duration);
    bool is_recording();
    bool enable_audio_codec(AM_AUDIO_TYPE type, uint32_t sample_rate,
                            bool enable);
  public:
    bool init();
    void set_msg_callback(AMRecordCallback callback, void *data);
    bool is_ready_for_event(AMEventStruct& event);
    bool send_event(AMEventStruct& event);

  protected:
    virtual void release();
    virtual void inc_ref();

  private:
    explicit AMRecord();
    virtual ~AMRecord();

  private:
    AMIRecordEngine   *m_engine;
    bool               m_is_initialized;
    std::atomic_int    m_ref_count;

  private:
    static AMRecord   *m_instance;
    static std::mutex  m_lock;
};

#endif /* AM_RECORD_H_ */
