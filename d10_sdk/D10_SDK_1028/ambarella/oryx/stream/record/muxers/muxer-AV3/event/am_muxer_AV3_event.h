/*******************************************************************************
 * am_muxer_AV3_event.h
 *
 * History:
 *   2016-09-07 - [ccjing] created file
 *
 * Copyright (c) 2016 Ambarella, Inc.
 *
 * This file and its contents (“Software”) are protected by intellectual
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
#ifndef AM_MUXER_AV3_EVENT_H_
#define AM_MUXER_AV3_EVENT_H_
#include "am_muxer_AV3_base.h"

class AMMuxerAV3Event : public AMMuxerAV3Base
{
    typedef AMMuxerAV3Base inherited;
  public :
    static AMMuxerAV3Event* create(const char* config_file);
    /* Interface Of AMIMuxerCodec */
    virtual AM_MUXER_ATTR get_muxer_attr();
    virtual void feed_data(AMPacket* packet);

    /* Interface Of AMIFileOperation */
    virtual bool start_file_writing();

  private :
    AMMuxerAV3Event();
    AM_STATE init(const char* config_file);
    virtual ~AMMuxerAV3Event();
    virtual AM_STATE generate_file_name(std::string &file_name);
    virtual AM_STATE on_info_pkt(AMPacket* packet);
    virtual AM_STATE on_data_pkt(AMPacket* packet);
    virtual AM_STATE check_video_pts(AMPacket* packet);
    virtual void clear_params_for_new_file();
    virtual void main_loop();
  private :
    AM_STATE update_filename_timestring();
  private :
    bool m_video_feeded = false;
};

#endif /* AM_MUXER_AV3_EVENT_H_ */
