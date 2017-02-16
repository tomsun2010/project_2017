/*******************************************************************************
 * am_air_api_helper.h
 *
 * History:
 *   2014-9-28 - [lysun] created file
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
#ifndef AM_AIR_API_HELPER_H_
#define AM_AIR_API_HELPER_H_

/*! @file am_api_helper.h
 *  @brief This file defines AMAPIHelper
 */

#include <atomic>
#include "am_pointer.h"

class AMAPIHelper;
class AMIPCSyncCmdClient;

/*! @defgroup airapi Air API
 *  @brief All Air APIs that can be used to interact with Oryx Services and
 *         helper class used to interact with Oryx Services.
 */

/*! @defgroup airapi-datastructure Air API Data Structure
 *  @ingroup airapi
 *  @brief All Oryx Services related data structures
 */

/*! @defgroup airapi-helperclass Air API Helper Class
 *  @ingroup airapi
 *  @brief Helper class used to connect with Oryx Services
 *         and call the functions defined by the service through command ID.
 *         Refer to @ref airapi-commandid "Air API Command IDs"
 *         to get command IDs.
 *  @{
 */

/*! @typedef AMAPIHelperPtr
 *  @brief Smart pointer type used to manage AMAPIHelper pointer.
 */
typedef AMPointer<AMAPIHelper> AMAPIHelperPtr;

/*! @class AMAPIHelper
 *  @brief Helper class used to interact with Oryx Services.
 *
 *  User needs to call get_instance() of this class to acquire an instance,
 *  then use method_call() with proper parameters to interact with the
 *  underlying service.
 */
class AMAPIHelper
{
    friend AMAPIHelperPtr;

  public:
    /*! @typedef AM_IPC_NOTIFY_CB
     *  @brief   IPC notify callback function type,
     *           which is used in AMAPIHelper::register_notify_cb
     *
     *  @param   msg_data      callback function data pointer
     *  @param   msg_data_size callback function data size
     *  @return               0 if success, otherwise return value less than 0.
     */
    typedef int (*AM_IPC_NOTIFY_CB)(void *msg_data, int msg_data_size);

  public:
    /*!
     * Constructor. Get a reference of the AMAPIHelper object.
     * @return AMAPIHelperPtr type.
     */
    static AMAPIHelperPtr get_instance();

    /*!
     * Make a function call to the service by providing corresponding
     * cmd_id of the service and related command structures which are defined
     * in related am_api_xxx.h\n
     * See \ref test_video_service.cpp \ref test_audio_service.cpp for more
     * information.
     *
     * @param cmd_id Command ID
     * @param msg_data [in] related input parameter data structure pointer
     * @param msg_data_size [in] related input parameter size
     * @param result_addr [out] related function call result
     *                    data structure pointer
     * @param result_max_size [in] related function call result
     *                        data structure size
     * @sa am_air_api.h
     */
    void method_call(uint32_t cmd_id,
                     void *msg_data,
                     int msg_data_size,
                     void *result_addr,
                     int result_max_size);

    /*!
     * Register a notify callback to make service notify the client possible
     *
     * @param cb will be called by service
     * @sa am_air_api.h
     */
    int register_notify_cb(AM_IPC_NOTIFY_CB cb);

    static void on_notify_callback(uint32_t context,
                                      void *msg_data,
                                      int msg_data_size,
                                      void *result_addr,
                                      int result_max_size);

  protected:
    /*!
     * Constructor.
     */
    AMAPIHelper();

    /*!
     * Destructor.
     */
    virtual ~AMAPIHelper();

    /*!
     * Initializer function.
     * @return 0 if success, otherwise return value less than 0.
     */
    int construct();

    /*!
     * Decrease the reference counter of the class object, when reference
     * counter reaches 0, destroy the object.
     */
    void release();

    /*!
     * Increase the reference counter of the class object.
     */
    void inc_ref();

  private:
    AMAPIHelper(AMAPIHelper const &copy) = delete;  //! Delete copy constructor
    AMAPIHelper& operator=(AMAPIHelper const &copy) = delete;//! Delete assign

  private:
    static AMAPIHelper *m_instance;
    AMIPCSyncCmdClient *m_air_api_ipc;
    AM_IPC_NOTIFY_CB    m_notify_cb;
    std::atomic_int     m_ref_counter;
};

/*!
 * @}
 */ /* End of Air API Helper Class */

#endif /* AM_AIR_API_HELPER_H_ */
