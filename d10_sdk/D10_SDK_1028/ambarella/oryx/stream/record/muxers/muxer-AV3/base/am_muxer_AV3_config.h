/*******************************************************************************
 * am_muxer_AV3_config.h
 *
 * History:
 *   2016-08-30 - [ccjing] created file
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
#ifndef AM_MUXER_AV3_CONFIG_H_
#define AM_MUXER_AV3_CONFIG_H_
#include "am_audio_define.h"
#include "am_muxer_codec_info.h"
struct AMMuxerCodecAV3Config
{
    int32_t        file_duration;
    uint32_t       max_file_num_per_folder;
    uint32_t       recording_file_num;
    uint32_t       recording_duration;
    uint32_t       video_id;
    uint32_t       event_id;
    uint32_t       smallest_free_space;
    uint32_t       muxer_id;
    uint32_t       write_index_to_nand_frequency;
    uint32_t       max_file_size;
    uint32_t       audio_sample_rate;
    AM_AUDIO_TYPE  audio_type;
    AM_MUXER_ATTR  muxer_attr;
    bool           auto_file_writing;
    bool           file_location_auto_parse;
    bool           AV3_reconstruct_flag;
    bool           need_sync;
    std::string    file_name_prefix;
    std::string    file_location;
    std::string    key_file_location;
};

class AMConfig;
class AMMuxerAV3Config
{
  public:
    AMMuxerAV3Config();
    virtual ~AMMuxerAV3Config();
    AMMuxerCodecAV3Config* get_config(const std::string& config_file);
    //write config information into config file
    bool set_config(AMMuxerCodecAV3Config* config);


  private:
    AMConfig              *m_config;
    AMMuxerCodecAV3Config *m_muxer_AV3_config;
};

#endif /* AM_MUXER_AV3_CONFIG_H_ */
