/*******************************************************************************
 * am_muxer_export_uds.h
 *
 * History:
 *   2015-01-04 - [Zhi He] created file
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

#ifndef __AM_MUXER_EXPORT_UDS_H__
#define __AM_MUXER_EXPORT_UDS_H__
#include <map>
#include <queue>
#include <memory>
#include <condition_variable>
#include "am_muxer_codec_info.h"

#define DMAX_CACHED_PACKET_NUMBER 64

enum
{
  EExportState_not_inited           = 0x0,
  EExportState_no_client_connected  = 0x1,
  EExportState_running              = 0x2,
  EExportState_error                = 0x3,
  EExportState_halt                 = 0x4,
};

using std::map;
using std::queue;
using std::pair;

typedef queue<AMPacket*> AMPacketQueue;
typedef map<uint32_t, AMPacketQueue> AMPacketQueueMap;

class AMMuxerExportUDS final: public AMIMuxerCodec
{
  public:
    static AMMuxerExportUDS* create();
    void destroy();

  public:
    virtual AM_STATE start()                                          override;
    virtual AM_STATE stop()                                           override;
    virtual bool start_file_writing()                                 override;
    virtual bool stop_file_writing()                                  override;
    virtual bool set_recording_file_num(uint32_t file_num)            override;
    virtual bool set_recording_duration(int32_t duration)             override;
    virtual bool is_running()                                         override;
    virtual AM_STATE set_config(AMMuxerCodecConfig *config)           override;
    virtual AM_STATE get_config(AMMuxerCodecConfig *config)           override;
    virtual AM_MUXER_ATTR get_muxer_attr()                            override;
    virtual uint8_t get_muxer_codec_stream_id()                       override;
    virtual uint32_t get_muxer_id()                                   override;
    virtual AM_MUXER_CODEC_STATE get_state()                          override;
    virtual void feed_data(AMPacket *packet)                          override;

  protected:
    AMMuxerExportUDS();
    virtual ~AMMuxerExportUDS();

  private:
    bool init();
    bool send_info(int client_fd);
    bool send_packet(int client_fd);
    void main_loop();
    void save_info(AMPacket *packet);
    bool fill_export_packet(AMPacket *packet, AMExportPacket *export_packet);
    void clean_resource();
    void reset_resource();
    static void thread_entry(void *arg);

  private:
    AMEvent       *m_thread_wait         = nullptr;
    AMThread      *m_accept_thread       = nullptr;
    int            m_export_state        = EExportState_not_inited;
    int            m_max_fd              = -1;;
    int            m_socket_fd           = -1;
    int            m_connect_fd          = -1;
    uint32_t       m_video_map           = 0;
    uint32_t       m_audio_map           = 0;
    uint32_t       m_audio_pts_increment = 0;
    int            m_control_fd[2]       = {-1, -1};
    bool           m_running             = false;
    bool           m_thread_exit         = false;
    bool           m_client_connected    = false;
    bool           m_send_block          = false;
    fd_set         m_all_set;
    fd_set         m_read_set;
    AMExportConfig m_config              = {0};
    sockaddr_un    m_addr                = {0};
    std::mutex     m_send_mutex;
    std::condition_variable           m_send_cond;
    AMPacketQueue                     m_packet_queue;
    AMPacketQueueMap                  m_video_queue;
    AMPacketQueueMap                  m_audio_queue;
    map<uint32_t, AM_VIDEO_INFO>      m_video_infos;
    map<uint32_t, AM_AUDIO_INFO>      m_audio_infos;
    map<uint32_t, AMExportVideoInfo>  m_video_export_infos;
    map<uint32_t, AMExportAudioInfo>  m_audio_export_infos;
    map<uint32_t, AMExportPacket>     m_video_export_packets;
    map<uint32_t, AMExportPacket>     m_audio_export_packets;
    map<uint32_t, bool>               m_video_info_send_flag;
    map<uint32_t, bool>               m_audio_info_send_flag;
    pair<uint32_t, bool>              m_video_send_block  = {0, false};
    pair<uint32_t, bool>              m_audio_send_block  = {0, false};
    map<uint32_t, AM_PTS>             m_audio_last_pts;
    map<uint32_t, uint32_t>           m_audio_state;
};
#endif
