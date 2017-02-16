/*******************************************************************************
 * am_record_msg.h
 *
 * History:
 *   2014-12-2 - [ypchang] created file
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

/*! @file am_record_msg.h
 *  @brief defines record engine message types, data structures and callbacks
 */

#ifndef AM_RECORD_MSG_H_
#define AM_RECORD_MSG_H_

/*! @enum AM_RECORD_MSG
 *  @brief record engine message
 *         These message types indicate record engine status.
 */
enum AM_RECORD_MSG
{
  AM_RECORD_MSG_START_OK, //!< START is OK
  AM_RECORD_MSG_STOP_OK,  //!< STOP is OK
  AM_RECORD_MSG_ERROR,    //!< ERROR has occurred
  AM_RECORD_MSG_ABORT,    //!< Engine is aborted
  AM_RECORD_MSG_EOS,      //!< End of Stream
  AM_RECORD_MSG_OVER_FLOW,//!< IO Over flow
  AM_RECORD_MSG_TIMEOUT,  //!< Operation timeout
  AM_RECORD_MSG_NULL      //!< Invalid message
};

/*! @struct AMRecordMsg
 *  @brief This structure contains message that sent by record engine.
 *  It is usually used by applications to retrieve engine status.
 */
struct AMRecordMsg
{
    void         *data; //!< user data
    AM_RECORD_MSG msg;  //!< engine message
    AMRecordMsg() :
      data(nullptr),
      msg(AM_RECORD_MSG_NULL) {}
};

/*! @typedef AMRecordCallback
 *  @brief record callback function type
 *         Use this function to get message sent by record engine, usually
 *         this message contains engine status.
 * @param msg AMRecordMsg reference
 * @sa AMRecordMsg
 */
typedef void (*AMRecordCallback)(AMRecordMsg &msg);

#endif /* AM_RECORD_MSG_H_ */
