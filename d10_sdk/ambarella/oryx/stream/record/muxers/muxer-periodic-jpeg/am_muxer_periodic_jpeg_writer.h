/*******************************************************************************
 * am_muxer_periodic_jpeg_writer.h
 *
 * History:
 *   2016-04-20 - [ccjing] created file
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
#ifndef AM_MUXER_PERIODIC_JPEG_WRITER_H_
#define AM_MUXER_PERIODIC_JPEG_WRITER_H_
class AMIFileWriter;
struct AMMuxerCodecJpegConfig;

class AMPeriodicJpegWriter
{
  public:
    static AMPeriodicJpegWriter* create(AMMuxerCodecPeriodicJpegConfig
                                        *config);

  private:
    AMPeriodicJpegWriter();
    AM_STATE init(AMMuxerCodecPeriodicJpegConfig *config);
    virtual ~AMPeriodicJpegWriter();

  public:
    void destroy();
    AM_STATE set_file_name(const char* file_name);
    AM_STATE write_jpeg_data(uint8_t* data, uint32_t data_len);
    AM_STATE write_text_data(uint8_t* data, uint32_t data_len);
    AM_STATE create_next_files();
    void close_files();
    bool is_file_open();
  private:
    char                           *m_file_name;
    char                           *m_tmp_jpeg_name;
    char                           *m_tmp_text_name;
    char                           *m_path_name;
    char                           *m_base_name;
    AMIFileWriter                  *m_jpeg_writer;
    AMIFileWriter                  *m_text_writer;
    AMMuxerCodecPeriodicJpegConfig *m_config;
    uint32_t                        m_file_counter;
    std::string                     m_name;
};
#endif /* AM_MUXER_PERIODIC_JPEG_WRITER_H_ */
