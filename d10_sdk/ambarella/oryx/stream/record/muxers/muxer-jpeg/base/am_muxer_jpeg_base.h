/*******************************************************************************
 * am_muxer_jpeg_base.h
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
#ifndef AM_MUXER_JPEG_H_
#define AM_MUXER_JPEG_H_
#include "am_muxer_codec_if.h"
#include "am_muxer_jpeg_config.h"
#include "am_video_types.h"
#include "am_mutex.h"

#include <deque>
#include <list>
#include <string>

#define FILENAME_TIME_FORMAT "%Y%m%d%H%M%S"
#define EXIF_TIME_FORMAT "%Y:%m:%d %H:%M:%S"

class AMPacket;
class AMJpegFileWriter;
class AMThread;
struct AMMuxerCodecJpegConfig;
class AMMuxerJpegConfig;
class AMMuxerJpegExif;

#define JPEG_DATA_PKT_ERROR_NUM             50
#define CHECK_FREE_SPACE_FREQUENCY_FOR_JPEG 50

enum AM_EXIF_METERING_MODE {
  AM_METERING_UNKNOWN = 0,
  AM_METERING_AVERAGE,
  AM_METERING_CENTER_WEIGHTED_AVERAGE,
  AM_METERING_SPOT,
  AM_METERING_MULTI_SPOT,
  AM_METERING_PATTERN,
  AM_METERING_PARTIAL,
  AM_METERING_OTHER = 255,
};

enum AM_EXIF_WHITE_BALANCE {
  AM_AUTO_WHITE_BALANCE = 0,
  AM_MANUAL_WHITE_BALANCE,
};

enum AM_EXIF_LIGHT_SOURCE {
  AM_LIGHT_UNKNOWN = 0,
  AM_LIGHT_DAYLIGHT,
  AM_LIGHT_FLUORESCENT,
  AM_LIGHT_TUNGSTEN,
  AM_LIGHT_FLASH,
  AM_LIGHT_FINE_WEATHER = 9,
  AM_LIGHT_CLOUDY_WEATHER,
  AM_LIGHT_SHADE,
  AM_LIGHT_DAYLIGHT_FLUORESCENT,
  AM_LIGHT_DAY_WHITE_FLUORESCENT,
  AM_LIGHT_COOL_WHITE_FLUORESCENT,
  AM_LIGHT_WHITE_FLUORESCENT,
  AM_LIGHT_WARM_WHITE_FLUORESCENT,
  AM_LIGHT_STANDARD_LIGHT_A,
  AM_LIGHT_STANDARD_LIGHT_B,
  AM_LIGHT_STANDARD_LIGHT_C,
  AM_LIGHT_D55,
  AM_LIGHT_D65,
  AM_LIGHT_D75,
  AM_LIGHT_D50,
  AM_LIGHT_ISO_STUDIO_TUNGSTEN,
  AM_LIGHT_OTHER_LIGHT_SOURCE = 255,
};

struct AMExifParameter
{
  uint32_t iso;
  uint32_t exposure_time_den;
  AM_EXIF_METERING_MODE metering_mode;
  AM_EXIF_WHITE_BALANCE white_balance;
  AM_EXIF_LIGHT_SOURCE  light_source;
  AMExifParameter() :
    iso(100),
    exposure_time_den(1),
    metering_mode(AM_METERING_UNKNOWN),
    white_balance(AM_AUTO_WHITE_BALANCE),
    light_source(AM_LIGHT_UNKNOWN) {}

};

class AMMuxerJpegBase : public AMIMuxerCodec
{
    typedef std::list<std::string> string_list;
  public:
    typedef std::deque<AMPacket*> packet_queue;
    /*interface of AMIMuxerCodec*/
    virtual AM_STATE start();
    virtual AM_STATE stop();
    virtual bool start_file_writing();
    virtual bool stop_file_writing();
    virtual bool set_recording_file_num(uint32_t file_num);
    virtual bool set_recording_duration(int32_t duration);
    virtual bool is_running();
    virtual AM_MUXER_CODEC_STATE get_state();
    virtual AM_MUXER_ATTR get_muxer_attr() = 0;
    virtual uint8_t get_muxer_codec_stream_id();
    virtual uint32_t get_muxer_id();
    virtual void     feed_data(AMPacket* packet) = 0;
    virtual AM_STATE set_config(AMMuxerCodecConfig *config);
    virtual AM_STATE get_config(AMMuxerCodecConfig *config);
  private :
    virtual void main_loop() = 0;
    virtual AM_STATE generate_file_name(char file_name[]) = 0;
  protected :
    AMMuxerJpegBase();
    virtual ~AMMuxerJpegBase();
    AM_STATE init(const char* config_file);
    static void thread_entry(void *p);
    AM_MUXER_CODEC_STATE create_resource();
    void release_resource();
    bool get_current_time_string(char *time_str, int32_t len,
                                 const char *format);
    bool get_proper_file_location(std::string& file_location);
    void check_storage_free_space();
    AM_STATE timer_set(int time);
    static void timer_action(int signo);
    static AM_STATE update_exif_parameters();
    AM_STATE create_exif_data_packet(uint8_t *&date, uint32_t &len,
                                     AMPacket *packet);
  protected :
    AM_STATE on_info_packet(AMPacket* packet);
    virtual AM_STATE on_data_packet(AMPacket* packet);
    AM_STATE on_eos_packet(AMPacket* packet);

  protected:
    int64_t                 m_stop_recording_pts;
    AMThread               *m_thread;
    AMMuxerCodecJpegConfig *m_muxer_jpeg_config;/*do not need to delete*/
    AMJpegExifConfig       *m_exif_config;
    AMMuxerJpegConfig      *m_config;
    packet_queue           *m_packet_queue;
    char                   *m_config_file;
    AMJpegFileWriter       *m_file_writer;
    AMMuxerJpegExif        *m_jpeg_exif;
    AM_MUXER_CODEC_STATE    m_state;
    uint32_t                m_stream_id;
    uint32_t                m_recording_file_num;
    int32_t                 m_recording_duration;
    uint32_t                m_file_counter;
    uint32_t                m_exif_image_width;
    uint32_t                m_exif_image_height;
    bool                    m_file_writing;
    bool                    m_update_exif_once;
    std::atomic_bool        m_run;
    std::string             m_muxer_name;
    std::string             m_file_location;
    std::string             m_exif_config_file;
    AMMemLock               m_state_lock;
    AMMemLock               m_interface_lock;
    static AMExifParameter  m_exif_param;
};

#ifdef BUILD_AMBARELLA_ORYX_CONF_DIR
#define ORYX_MUXER_CONF_DIR \
  ((const char*)BUILD_AMBARELLA_ORYX_CONF_DIR"/stream/muxer")
#else
#define ORYX_MUXER_CONF_DIR ((const char*)"/etc/oryx/stream/muxer")
#endif

#endif /* AM_MUXER_JPEG_H_ */
