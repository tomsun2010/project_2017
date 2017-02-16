/*******************************************************************************
 * am_api_media.h
 *
 * History:
 *   2015-2-25 - [ccjing] created file
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
/*! @file am_api_media.h
 *  @brief This header file contains a class used to add file to
 *         playback in the media service.
 */
#ifndef AM_API_MEDIA_H_
#define AM_API_MEDIA_H_

#include <string>
#include "commands/am_api_cmd_media.h"

/*! @defgroup airapi-datastructure-media Data Structure of Media Service
 *  @ingroup airapi-datastructure
 *  @brief All Oryx Media Service related method call data structures
 *  @{
 */

/*! @class AMIApiPlaybackUnixDomainUri
 *  @brief This class is used as the parameter of the method_call function.
 *         method_call function is contained in the AMAPIHelper class which
 *         is used to interact with oryx.
 */
class AMIApiPlaybackUnixDomainUri
{
  public :
    /*! Create function.
     */
    static AMIApiPlaybackUnixDomainUri* create();
    /*! Destructor function
     */
    virtual ~AMIApiPlaybackUnixDomainUri() {}
    /*! This function is used to set audio type as aac.
     */
    virtual void set_audio_type_aac()    = 0;
    /*! This function is used to set audio type as opus.
     */
    virtual void set_audio_type_opus()    = 0;
    /*! This function is used to set audio type as g711A.
     */
    virtual void set_audio_type_g711A()    = 0;
    /*! This function is used to set audio type as g711U.
     */
    virtual void set_audio_type_g711U()    = 0;
    /*! This function is used to set audio type as g726_40.
     */
    virtual void set_audio_type_g726_40()    = 0;
    /*! This function is used to set audio type as g726_32.
     */
    virtual void set_audio_type_g726_32()    = 0;
    /*! This function is used to set audio type as g726_24.
     */
    virtual void set_audio_type_g726_24()    = 0;
    /*! This function is used to set audio type as g726_16.
     */
    virtual void set_audio_type_g726_16()    = 0;
    /*! This function is used to set audio type as speex.
     */
    virtual void set_audio_type_speex()    = 0;
    /*! This function is used to set sample rate.
     *  @param The param is audio sample rate.
     */
    virtual void set_sample_rate(uint32_t sample_rate) = 0;
    /*! This function is used to set audio channel.
     *  @param The param is audio channel.
     */
    virtual void set_audio_channel(uint32_t channel) = 0;
    /*! This function is used to set unix domain name.
     *  @param The param is unix domain name and name length.
     */
    virtual bool set_unix_domain_name(const char* name, uint32_t name_len) = 0;
    /*! This function is used to set playback id.
     * @return return true if success, otherwise return false.
     */
    virtual bool set_playback_id(int32_t id) = 0;
    /*! This function is used to get all params data.
     *  @return params data.
     */
    virtual char* get_data()                           = 0;
    /*! This function is used to get params data size.
     *  @return params data size.
     */
    virtual uint32_t get_data_size()                   = 0;
};

/*! @class AMIApiRecordingParam
 *  @brief This class is used as the parameter of the method_call function.
 *         method_call function is contained in the AMAPIHelper class which
 *         is used to interact with oryx.
 */
class AMIApiAudioCodecParam
{
  public :
    /*! Create function.
     */
    static AMIApiAudioCodecParam* create();
    /*! Destructor function
     */
    virtual ~AMIApiAudioCodecParam() {}
    /*! This function is used to set audio type as aac.
     */
    virtual void set_audio_type_aac()    = 0;
    /*! This function is used to set audio type as opus.
     */
    virtual void set_audio_type_opus()    = 0;
    /*! This function is used to set audio type as g711A.
     */
    virtual void set_audio_type_g711A()    = 0;
    /*! This function is used to set audio type as g711U.
     */
    virtual void set_audio_type_g711U()    = 0;
    /*! This function is used to set audio type as g726_40.
     */
    virtual void set_audio_type_g726_40()    = 0;
    /*! This function is used to set audio type as g726_32.
     */
    virtual void set_audio_type_g726_32()    = 0;
    /*! This function is used to set audio type as g726_24.
     */
    virtual void set_audio_type_g726_24()    = 0;
    /*! This function is used to set audio type as g726_16.
     */
    virtual void set_audio_type_g726_16()    = 0;
    /*! This function is used to set audio type as speex.
     */
    virtual void set_audio_type_speex()    = 0;
    /*! This function is used to set sample rate.
     *  @param The param is audio sample rate.
     */
    virtual void set_sample_rate(uint32_t sample_rate) = 0;
    /*! This function is used to get audio sample rate.
     *  @return sample rate.
     */
    virtual uint32_t get_sample_rate()                 = 0;
    /*! This function is used to enable audio codec.
     *  @param The param is enable flag.
     */
    virtual void enable(bool enable)                   = 0;
    /*! This function is used to get is enable or disable.
     *  @return true means enable, false menas disable.
     */
    virtual bool is_enable()                           = 0;
    /*! This function is used to get all params data.
     *  @return params data.
     */
    virtual char* get_data()                           = 0;
    /*! This function is used to get params data size.
     *  @return params data size.
     */
    virtual uint32_t get_data_size()                   = 0;
};

/*! @class AMIApiRecordingParam
 *  @brief This class is used as the parameter of the method_call function.
 *         method_call function is contained in the AMAPIHelper class which
 *         is used to interact with oryx.
 */
class AMIApiRecordingParam {
  public :
    /*! Create function.
     */
    static AMIApiRecordingParam* create();
    /*! Create function.
     */
    static AMIApiRecordingParam* create(AMIApiRecordingParam *data);
    /*! Destructor function
     */
    virtual ~AMIApiRecordingParam() {};
    /*! This function is used to set muxer_id_bit_map value.
     *  @param The param is muxer_id_bit_map.
     *  @return true if success, otherwise return false.
     */
    virtual bool set_muxer_id_bit_map(uint32_t muxer_id_bit_map) = 0;
    /*! This function is used to set recording_file_num value.
     *  @param The param is recording_file_num.
     *  @return true if success, otherwise return false.
     */
    virtual bool set_recording_file_num(uint32_t file_num) = 0;
    /*! This function is used to set recording_duration value.
     *  @param The param is recording_duration.
     *  @return true if success, otherwise return false.
     */
    virtual bool set_recording_duration(int32_t duration) = 0;
    /*! This function is used to set file duration value.
     *  @param The param is file duration.
     *  @return true if success, otherwise return false.
     */
    virtual bool set_file_duration(int32_t duration) = 0;
    /*! This function is used to apply value to config file.
     *  @param The param is enable.
     *  @return true if success, otherwise return false.
     */
    virtual bool apply_conf_file(bool enable) = 0;
    /*! This function is used to get muxer_id_bit_map value.
     *  @param void
     *  @return muxer_id_bit_map value.
     */
    virtual uint32_t get_muxer_id_bit_map() = 0;
    /*! This function is used to get recording_file_num value.
     *  @param void
     *  @return recording_file_num value.
     */
    virtual uint32_t get_recording_file_num() = 0;
    /*! This function is used to get recording_duration value.
     *  @param void
     *  @return recording_duration value.
     */
    virtual int32_t get_recording_duration() = 0;
    /*! This function is used to get all params data.
     *  @return params data.
     */
    virtual char* get_data() = 0;
    /*! This function is used to get params data size.
     *  @return params data size.
     */
    virtual uint32_t get_data_size() = 0;
    /*! This function is used to clear the data.
     */
    virtual void clear() = 0;
};

/*! @class am_api_playback_audio_file_list_t
 *  @brief This class is used as the parameter of the method_call function.
 *         method_call function is contained in the AMAPIHelper class which
 *         is used to interact with oryx.
 */
class AMIApiPlaybackAudioFileList
{
  public:
    /*! Create function.
     */
    static AMIApiPlaybackAudioFileList* create();
    /*! Create function.
     */
    static AMIApiPlaybackAudioFileList* create(AMIApiPlaybackAudioFileList*
                                               audio_file);
    /*! Destructor function
     */
    virtual ~AMIApiPlaybackAudioFileList(){}
    /*! This function is used to add file to the class.
     *  @param The param is file name. The max length of it is 490 bytes
     *  @return true if success, otherwise return false.
     */
    virtual bool add_file(const std::string &file_name) = 0;
    /*! This function is used to set playback id.
     * @return return true if success, otherwise return false.
     */
    virtual bool set_playback_id(int32_t id) = 0;
    /*! This function is used to get file which was added to this class before.
     *  @param The param is file number. For example, 1 means the first file.
     *  @return true if file number is valid, otherwise false.
     */
    virtual std::string get_file(uint32_t file_number) = 0;
    /*! This function is used to get how many files in the class.
     *  @return Return how many files contained in this class.
     */
    virtual uint32_t get_file_number() = 0;
    /*! This function is used to check the file list full or not.
     * @return if it is full, return true, otherwise return false;
     */
    virtual bool is_full() = 0;
    /*! This function is used to get the whole file list.
     * @return return the address of the file list.
     */
    virtual char* get_file_list() = 0;
    /*! This function is used to get the size of file list.
     * @return return the size of the file list.
     */
    virtual uint32_t get_file_list_size() = 0;
    /*! This function is used to clear all files in the class.
     */
    virtual void clear_file() = 0;
};

/*! @class AMIApiMediaEventStruct
 *  @brief This class is used as the parameter of the method_call function.
 *         method_call function is contained in the AMAPIHelper class which
 *         is used to interact with oryx.
 */
class AMIApiMediaEvent
{
  public :
    /*! Create function.
     */
    static AMIApiMediaEvent* create();
    /*! Destructor function
     */
    virtual ~AMIApiMediaEvent() {}
    /*! This function is used to set event attr as mjpeg.
     */
    virtual void set_attr_mjpeg() = 0;
    /*! This function is used to set event attr as h264 or h265.
     */
    virtual void set_attr_h26X() = 0;
    /*! This function is used to set event attr as periodic mjpeg.
     */
    virtual void set_attr_periodic_mjpeg() = 0;
    /*! This function is used to get attr.
     *  @return true if attr is h264 or h265, otherwise return false.
     */
    virtual bool is_attr_h26X() = 0;
    /*! This function is used to get attr.
     *  @return true if attr is mjpeg, otherwise return false.
     */
    virtual bool is_attr_mjpeg() = 0;
    /*! This function is used to get attr.
     *  @return true if attr is periodic mjpeg, otherwise return false.
     */
    virtual bool is_attr_periodic_mjpeg() = 0;
    /*! This function is used to set event id for h26x.
     *  @param The param is event id.
     *  @return true if success, otherwise return false.
     */
    virtual bool set_event_id(uint32_t event_id) = 0;
    /*! This function is used to set stream id for mjpeg and periodic mjpeg.
     *  @param The param is stream id.
     *  @return true if success, otherwise return false.
     */
    virtual bool set_stream_id(uint32_t stream_id) = 0;
    /*! This function is used to get event id for h26x.
     *  @return event id.
     */
    virtual uint32_t get_event_id() = 0;
    /*! This function is used to get stream id for mjpeg and periodic mjpeg.
     *  @return stream id.
     */
    virtual uint32_t get_stream_id() = 0;
    /*! This function is used to set pre current pts number.
     *  @param The param is number.
     *  @return true if success, otherwise return false.
     */
    virtual bool set_pre_cur_pts_num(uint8_t num) = 0;
    /*! This function is used to get pre current pts number.
     *  @return pre_cur_pts_num.
     */
    virtual uint8_t get_pre_cur_pts_num() = 0;
    /*! This function is used to set after current pts number.
     *  @param The param is number.
     *  @return true if success, otherwise return false.
     */
    virtual bool set_after_cur_pts_num(uint8_t num) = 0;
    /*! This function is used to get after current pts number.
     *  @return after_cur_pts_num.
     */
    virtual uint8_t get_after_cur_pts_num() = 0;
    /*! This function is used to set closest current pts number.
     *  @param The param is number.
     *  @return true if success, otherwise return false.
     */
    virtual bool set_closest_cur_pts_num(uint8_t num) = 0;
    /*! This function is used to get closest current pts number.
     *  @return closest currrent pts number.
     */
    virtual uint8_t get_closest_cur_pts_num() = 0;
    /*! This function is used to set interval second.
     *  @param The param is interval second.
     *  @return true if success, otherwise return false.
     */
    virtual bool set_interval_second(uint32_t second) = 0;
    /*! This function is used to get interval second.
     *  @return interval second.
     */
    virtual uint32_t get_interval_second() = 0;
    /*! This function is used to set once jpeg number.
     *  @param The param is number.
     *  @return true if success, otherwise return false.
     */
    virtual bool set_once_jpeg_num(uint32_t num) = 0;
    /*! This function is used to get once jpeg number.
     *  @return once jpeg number.
     */
    virtual uint32_t get_once_jpeg_num() = 0;
    /*! This function is used to set start time hour.
     *  @param The param is hour.
     *  @return true if success, otherwise return false.
     */
    virtual bool set_start_time_hour(uint8_t hour) = 0;
    /*! This function is used to get start time hour.
     *  @return start time hour.
     */
    virtual uint8_t get_start_time_hour() = 0;
    /*! This function is used to set start time minute.
     *  @param The param is minute.
     *  @return true if success, otherwise return false.
     */
    virtual bool set_start_time_minute(uint8_t minute) = 0;
    /*! This function is used to get start time minute.
     *  @return start time minute.
     */
    virtual uint8_t get_start_time_minute() = 0;
    /*! This function is used to set start time second.
     *  @param The param is second.
     *  @return true if success, otherwise return false.
     */
    virtual bool set_start_time_second(uint8_t second) = 0;
    /*! This function is used to get start time second.
     *  @return start time second.
     */
    virtual uint8_t get_start_time_second() = 0;
    /*! This function is used to set end time hour.
     *  @param The param is hour.
     *  @return true if success, otherwise return false.
     */
    virtual bool set_end_time_hour(uint8_t hour) = 0;
    /*! This function is used to get end time hour.
     *  @return end time hour.
     */
    virtual uint8_t get_end_time_hour() = 0;
    /*! This function is used to set end time minute.
     *  @param The param is minute.
     *  @return true if success, otherwise return false.
     */
    virtual bool set_end_time_minute(uint8_t minute) = 0;
    /*! This function is used to get end time minute.
     *  @return end time minute.
     */
    virtual uint8_t get_end_time_minute() = 0;
    /*! This function is used to set end time second.
     *  @param The param is second.
     *  @return true if success, otherwise return false.
     */
    virtual bool set_end_time_second(uint8_t second) = 0;
    /*! This function is used to get end time second.
     *  @return end time second.
     */
    virtual uint8_t get_end_time_second() = 0;
    /*! This function is used to get all params data.
     *  @return params data.
     */
    virtual char* get_data()    = 0;
    /*! This function is used to get all params data size.
     *  @return params data size.
     */
    virtual uint32_t get_data_size() = 0;
};

/*! @class AMIAPiFileOperationParam
 *  @brief This class is used as the parameter of the method_call function.
 *         method_call function is contained in the AMAPIHelper class which
 *         is used to interact with oryx.
 */
class AMIAPiFileOperationParam
{
  public :
    /*! Create function.
     */
    static AMIAPiFileOperationParam* create();
    /*! Destructor function
     */
    virtual ~AMIAPiFileOperationParam() {}
    /*! set muxer id bit map
     */
    virtual void set_muxer_id_bit_map(uint32_t muxer_id_bit_map)    = 0;
    /*! set file create notify
     */
    virtual void set_file_create_notify()                           = 0;
    /*! set file finish notify
     */
    virtual void set_file_finish_notify()                           = 0;
    /*! enable callback notify.
     */
    virtual void enable_callback_notify()                           = 0;
    /*! disable callback notify
     */
    virtual void disable_callback_notify()                          = 0;
    /*! This function is used to get all params data.
     *  @return params data.
     */
    virtual char* get_data()                                        = 0;
    /*! This function is used to get all params data size.
     *  @return params data size.
     */
    virtual uint32_t get_data_size()                                = 0;
    virtual void clear()                                            = 0;
};

/*! @enum AM_API_MEDIA_NOTIFY_TYPE
 *  @brief This enum is used to specify the notify type of media service.
 */
enum AM_API_MEDIA_NOTIFY_TYPE
{
  AM_API_MEDIA_NOTIFY_TYPE_NULL        = -1, //!<AM_API_MEDIA_NOTIFY_TYPE_NULL
  AM_API_MEDIA_NOTIFY_TYPE_FILE_FINISH = 0,  //!<AM_API_MEDIA_NOTIFY_TYPE_FILE_FINISH
  AM_API_MEDIA_NOTIFY_TYPE_FILE_CREATE = 1,  //!<AM_API_MEDIA_NOTIFY_TYPE_FILE_CREATE
  AM_API_MEDIA_NOTIFY_TYPE_NUM,              //!<AM_API_MEDIA_NOTIFY_TYPE_NUM
};

/*! @struct AMApiMediaFileInfo
 *  @brief This struct is used to define the information which was sent to app
 *         by media service.
 */
struct AMApiMediaFileInfo
{
    AM_API_MEDIA_NOTIFY_TYPE type                = AM_API_MEDIA_NOTIFY_TYPE_NULL;
    uint32_t                 stream_id           = 0;
    uint32_t                 muxer_id            = 0;
    char                     create_time_str[32] = { 0 };
    char                     finish_time_str[32] = { 0 };
    char                     file_name[128]      = { 0 };
};
/*!
 * @}
 */
#endif /* AM_API_MEDIA_H_ */
