/*******************************************************************************
 * am_muxer_time_elapse_mp4.h
 *
 * History:
 *   2016-05-11 - [ccjing] created file
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
#ifndef AM_MUXER_TIME_ELAPSE_MP4_H_
#define AM_MUXER_TIME_ELAPSE_MP4_H_
#include "am_muxer_codec_if.h"
#include "am_video_types.h"
#include "am_mutex.h"
#include <deque>
#include <list>
#include <string>

#include "am_time_elapse_mp4_config.h"
class AMPacket;
class AMTimeElapseMp4Builder;
class AMTimeElapseMp4FileWriter;
class AMThread;
struct TimeElapseMp4ConfigStruct;
class AMMuxerTimeElapseMp4Config;
#define ON_DATA_PKT_ERROR_NUM      50
#define CHECK_FREE_SPACE_FREQUENCY 50
#define PTS_TIME_FREQUENCY         90000
class AMMuxerTimeElapseMp4: public AMIMuxerCodec
{
    typedef std::deque<AMPacket*> packet_queue;
    typedef std::list<AMPacket*> packet_list;
    typedef std::list<std::string> string_list;
  public:
    /*interface of AMIMuxerCodec*/
    static AMMuxerTimeElapseMp4* create(const char* config_file);
    virtual AM_STATE start();
    virtual AM_STATE stop();
    virtual bool start_file_writing();
    virtual bool stop_file_writing();
    virtual bool set_recording_file_num(uint32_t file_num);
    virtual bool set_recording_duration(int32_t duration);
    virtual bool is_running();
    virtual AM_MUXER_CODEC_STATE get_state();
    virtual AM_MUXER_ATTR get_muxer_attr();
    virtual uint8_t get_muxer_codec_stream_id();
    virtual uint32_t get_muxer_id();
    virtual void feed_data(AMPacket* packet);
    virtual AM_STATE set_config(AMMuxerCodecConfig *config);
    virtual AM_STATE get_config(AMMuxerCodecConfig *config);
  private:
    AMMuxerTimeElapseMp4();
    AM_STATE init(const char* config_file);
    virtual ~AMMuxerTimeElapseMp4();
    static void thread_entry(void *p);
    void main_loop();
    AM_STATE generate_file_name(char* file_name);
    bool get_proper_file_location(std::string& file_location);
    bool get_current_time_string(char *time_str, int32_t len);
    void clear_params_for_new_file();
    AM_STATE on_info_pkt(AMPacket* packet);
    AM_STATE on_data_pkt(AMPacket* packet);
    AM_STATE on_eos_pkt(AMPacket* packet);
    AM_STATE on_eof_pkt(AMPacket* packet); //close current file and create new file.
    AM_STATE write_video_data_pkt(AMPacket* packet);
    AM_MUXER_CODEC_STATE create_resource();
    void release_resource();
    void clear_all_params();
    bool is_h265_IDR_first_nalu(AMPacket* packet);
    void check_storage_free_space();
  private:
    uint64_t                     m_file_duration;
    int64_t                      m_curr_file_boundary;
    AMThread                    *m_thread;
    packet_queue                *m_packet_queue;
    char                        *m_config_file;
    AMTimeElapseMp4Builder      *m_mp4_builder;
    AMTimeElapseMp4FileWriter   *m_file_writer;
    AMMuxerTimeElapseMp4Config  *m_config;
    TimeElapseMp4ConfigStruct   *m_muxer_mp4_config;/*do not need to delete*/
    uint32_t                     m_eos_map;
    uint32_t                     m_av_info_map;
    uint32_t                     m_video_frame_count;
    AM_MUXER_CODEC_STATE         m_state;
    uint16_t                     m_stream_id;
    bool                         m_is_first_video;
    bool                         m_need_splitted;
    bool                         m_new_info_coming;
    bool                         m_file_writing;
    bool                         m_need_sync;
    std::atomic_bool             m_run;
    AM_VIDEO_INFO                m_video_info;
    std::string                  m_muxer_name;
    std::string                  m_file_location;
    AMMemLock                    m_state_lock;
    AMMemLock                    m_interface_lock;
    AMMemLock                    m_file_writing_lock;
};

#endif /* AM_MUXER_TIME_ELAPSE_MP4_H_ */
