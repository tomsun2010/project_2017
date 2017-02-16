/*******************************************************************************
 * am_muxer_jpeg_config.h
 *
 * History:
 *   2015-10-8 - [ccjing] created file
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
#ifndef AM_MUXER_JPEG_CONFIG_H_
#define AM_MUXER_JPEG_CONFIG_H_

struct AMMuxerCodecJpegConfig
{
    uint32_t       max_file_number;
    uint32_t       smallest_free_space;
    uint32_t       muxer_id;
    uint32_t       stream_id;
    uint32_t       recording_file_num;
    uint32_t       recording_duration;
    AM_MUXER_ATTR  muxer_attr;
    uint8_t        buffer_pkt_num;
    uint8_t        buffer_size;
    bool           auto_file_writing;
    bool           file_location_auto_parse;
    std::string    file_name_prefix;
    std::string    file_location;
};

struct AMFraction
{
  uint32_t num = 0;
  uint32_t denom = 1;
};

struct AMJpegExifConfig
{
  std::string make;
  std::string model;
  std::string artist;
  std::string image_description;
  bool enable = false;
  AMFraction focal_length;
  AMFraction aperture_value;
};

class AMConfig;
class AMMuxerJpegConfig
{
  public:
    AMMuxerJpegConfig();
    virtual ~AMMuxerJpegConfig();
    AMMuxerCodecJpegConfig* get_config(const std::string& config_file);
    bool set_config(AMMuxerCodecJpegConfig* config);

    AMJpegExifConfig* get_exif_config(const std::string& config_file);
    //bool set_exif_config(AMJpegExifConfig *config);

  private:
    AMConfig               *m_config;
    AMMuxerCodecJpegConfig *m_muxer_jpeg_config;
    AMJpegExifConfig       *m_exif_config;
};

#endif /* AM_MUXER_JPEG_CONFIG_H_ */
