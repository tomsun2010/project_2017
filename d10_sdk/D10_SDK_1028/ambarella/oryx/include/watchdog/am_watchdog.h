/*******************************************************************************
 * watchdog_service_instance.h
 *
 * History:
 *   2014年5月14日 - [ypchang] created file
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
#ifndef WATCHDOG_SERVICE_INSTANCE_H_
#define WATCHDOG_SERVICE_INSTANCE_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <pthread.h>
#include "../services/commands/am_service_impl.h"
#define SERVICE_TIMEOUT_SECONDS 10
#define WATCHDOG_IOCTL_BASE 'W'

#define WDIOC_GETSUPPORT    _IOR(WATCHDOG_IOCTL_BASE, 0, struct watchdog_info)
#define WDIOC_GETSTATUS     _IOR(WATCHDOG_IOCTL_BASE, 1, int)
#define WDIOC_GETBOOTSTATUS _IOR(WATCHDOG_IOCTL_BASE, 2, int)
#define WDIOC_GETTEMP       _IOR(WATCHDOG_IOCTL_BASE, 3, int)
#define WDIOC_SETOPTIONS    _IOR(WATCHDOG_IOCTL_BASE, 4, int)
#define WDIOC_KEEPALIVE     _IOR(WATCHDOG_IOCTL_BASE, 5, int)
#define WDIOC_SETTIMEOUT    _IOWR(WATCHDOG_IOCTL_BASE, 6, int)
#define WDIOC_GETTIMEOUT    _IOR(WATCHDOG_IOCTL_BASE, 7, int)
#define WDIOC_SETPRETIMEOUT _IOWR(WATCHDOG_IOCTL_BASE, 8, int)
#define WDIOC_GETPRETIMEOUT _IOR(WATCHDOG_IOCTL_BASE, 9, int)
#define WDIOC_GETTIMELEFT   _IOR(WATCHDOG_IOCTL_BASE, 10, int)

#define WDIOF_UNKNOWN   -1  /* Unknown flag error */
#define WDIOS_UNKNOWN   -1  /* Unknown status error */

#define WDIOF_OVERHEAT    0x0001  /* Reset due to CPU overheat */
#define WDIOF_FANFAULT    0x0002  /* Fan failed */
#define WDIOF_EXTERN1   0x0004  /* External relay 1 */
#define WDIOF_EXTERN2   0x0008  /* External relay 2 */
#define WDIOF_POWERUNDER  0x0010  /* Power bad/power fault */
#define WDIOF_CARDRESET   0x0020  /* Card previously reset the CPU */
#define WDIOF_POWEROVER   0x0040  /* Power over voltage */
#define WDIOF_SETTIMEOUT  0x0080  /* Set timeout (in seconds) */
#define WDIOF_MAGICCLOSE  0x0100  /* Supports magic close char */
#define WDIOF_PRETIMEOUT  0x0200  /* Pretimeout (in seconds), get/set */
#define WDIOF_KEEPALIVEPING 0x8000  /* Keep alive ping reply */

#define WDIOS_DISABLECARD 0x0001  /* Turn off the watchdog timer */
#define WDIOS_ENABLECARD  0x0002  /* Turn on the watchdog timer */
#define WDIOS_TEMPPANIC   0x0004  /* Kernel panic on temperature trip */

#define WATCHDOG_DEVICE "/dev/watchdog"

#define SYS_SERVICE_NAME   (const char*)"system service"
#define MED_SERVICE_NAME   (const char*)"media service"
#define EVT_SERVICE_NAME   (const char*)"event service"
#define IMG_SERVICE_NAME   (const char*)"image service"
#define VCTRL_SERVICE_NAME (const char*)"video control service"
#define NET_SERVICE_NAME   (const char*)"network control service"
#define AUD_SERVICE_NAME (const char*)"audio control service"
#define RTSP_SERVICE_NAME (const char*)"rtsp control service"
#define SIP_SERVICE_NAME (const char*)"sip service"
#define PLBK_SERVICE_NAME (const char*)"playback service"
#define VEDIT_SERVICE_NAME (const char*)"video edit service"
#define EFM_SERVICE_NAME (const char*)"efm src service"

typedef enum
{
  IMAGE_SERVICE = 0,
  MEDIA_SERVICE,
  VIDEO_CONTROL_SERVICE,
  AUDIO_CONTROL_SERVICE,
  EVENT_SERVICE,
  NETWORK_CONTROL_SERVICE,
  SYSTEM_SERVICE,
  USER_SERVICE,
  RTSP_CONTROL_SERVICE,
  SIP_SERVICE,
  PLAYBACK_SERVICE,
  VIDEO_EDIT_SERVICE,
  EFM_SRC_SERVICE,
  API_PROXY_SERVER,
  MAX_SERVICE_NUM,
} service_num_t;
typedef enum
{
  AMBA_VDSP                 = 0,
  AMBA_VIN0_IDSP_LAST_PIXEL = 1,
  AMBA_VNI0_IDSP            = 2,
  AMBA_DSP_INTR_NUM
} dsp_intr_t;

struct SrvData;
struct WdData;
struct DspData;
struct NetDevice;

class AMWatchdogService
{
  public:
    AMWatchdogService();
    virtual ~AMWatchdogService();

  public:
    bool init(am_service_attribute *m_service_list);
    bool start();
    bool stop();
    void run();
    void quit();
    //void set_encode_state(bool encode);

  private:
    bool start_feeding_thread();
    bool stop_feeding_thread();
    bool set_watchdog_timeout(int sec);
    bool enable_watchdog(int sec);
    bool disable_watchdog();
    bool init_semaphore(SrvData& data);
    bool init_service_data();
    bool clean_service_data();
    bool check_service_timeout();
    bool get_dsp_intr_value(FILE* intr,
                            const char* intr_name,
                            uint32_t& intr_val);
    bool check_dsp_intr(FILE* intr);
    void abort();
    void* watchdog_feeding_thread(void* data);
    static void* static_watchdog_feeding_thread(void* data);

  private:
    pthread_mutex_t mDspLock;
    pthread_mutex_t mSrvLock;
    pthread_t       mWdFeedingThread;
    SrvData        *mSrvDataList;
    DspData        *mDspData;
    WdData         *mWdData;
   // NetDevice      *mNetDevList[2];
    int             mWdFd;
   // int             mNetDevIdx;
    int             mSrvCtrl[2];
    int             mInitTimeout;
    int             mWorkTimeout;
    bool            mIsInited;
    bool            mIsDspEncoding;
    bool            mIsFeeding;
    bool            mRun;
    bool            mNeedReboot;
};

#endif /* WATCHDOG_SERVICE_INSTANCE_H_ */
