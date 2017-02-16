/*******************************************************************************
 * am_time_elapse_mp4_file_writer.h
 *
 * History:
 *   2016-05-11 - [ccjing] created file
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
#ifndef AM_TIME_ELAPSE_MP4_FILE_WRITER_H_
#define AM_TIME_ELAPSE_MP4_FILE_WRITER_H_
#include "am_video_types.h"
class Mp4FileWriter;
struct TimeElapseMp4ConfigStruct;
struct FileTypeBox;
struct MediaDataBox;
struct MovieBox;
struct FreeBox;
class AMTimeElapseMp4FileWriter
{
  public:
    static AMTimeElapseMp4FileWriter* create(TimeElapseMp4ConfigStruct
                  *muxer_mp4_config, AM_VIDEO_INFO *video_info);
  private:
    AMTimeElapseMp4FileWriter();
    AM_STATE init(TimeElapseMp4ConfigStruct *muxer_mp4_config,
                  AM_VIDEO_INFO *video_info);
    virtual ~AMTimeElapseMp4FileWriter();
  public:
    void destroy();
    AM_STATE set_media_sink(const char* file_name);
    AM_STATE write_data(uint8_t* data, uint32_t data_len);
    AM_STATE write_data_direct(uint8_t *data, uint32_t data_len);
    AM_STATE seek_data(uint32_t offset, uint32_t whence); //SEEK_SET ...
    bool is_file_open();
    AM_STATE write_file_type_box(FileTypeBox& box);
    AM_STATE write_media_data_box(MediaDataBox& box);
    AM_STATE write_movie_box(MovieBox& box);
    AM_STATE write_free_box(FreeBox& box);
    AM_STATE write_u8(uint8_t value);
    AM_STATE write_s8(int8_t value);
    AM_STATE write_be_u16(uint16_t value);
    AM_STATE write_be_s16(int16_t value);
    AM_STATE write_be_u24(uint32_t value);
    AM_STATE write_be_s24(int32_t value);
    AM_STATE write_be_u32(uint32_t value);
    AM_STATE write_be_s32(int32_t value);
    AM_STATE write_be_u64(uint64_t value);
    AM_STATE write_be_s64(int64_t value);
    uint64_t get_file_offset();
    AM_STATE create_next_file();
    AM_STATE create_m3u8_file();
    AM_STATE close_file(bool need_sync = false);
    void set_begin_packet_pts(int64_t pts);
    void set_end_packet_pts(int64_t pts);
    char* get_current_file_name();
    Mp4FileWriter* get_file_writer();

  private:
    uint64_t                   m_cur_file_offset;
    int64_t                    m_begin_packet_pts;
    int64_t                    m_end_packet_pts;
    int64_t                    m_file_duration;
    char                      *m_file_name;
    char                      *m_tmp_file_name;
    char                      *m_path_name;
    char                      *m_base_name;
    Mp4FileWriter             *m_file_writer;
    AM_VIDEO_INFO             *m_video_info;
    TimeElapseMp4ConfigStruct *m_muxer_mp4_config;
    uint32_t                   m_file_target_duration;
    uint32_t                   m_hls_sequence;
    int32_t                    m_file_counter;
};

#endif /* AM_TIME_ELAPSE_MP4_FILE_WRITER_H_ */
