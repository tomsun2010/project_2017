/*
 * am_watchdog.h
 *
 * @Author: Yang Wang
 * @Email : ywang@ambarella.com
 * @Time  : 09/09/2013 [Created]
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
 */
#ifndef __AM_WATCHDOG_H__
#define __AM_WATCHDOG_H__

#define HEART_BEAT_INTERVAL  2
#define SEM_SYS_SERVICE   (const char*)"/System.Service"
#define SEM_MED_SERVICE   (const char*)"/Media.Service"
#define SEM_EVT_SERVICE   (const char*)"/Event.Service"
#define SEM_IMG_SERVICE   (const char*)"/Image.Service"
#define SEM_VCTRL_SERVICE (const char*)"/Video.Service"
#define SEM_NET_SERVICE   (const char*)"/Network.Service"
#define SEM_AUD_SERVICE (const char*)"/Audio.Service"
#define SEM_RTSP_SERVICE (const char*)"/RTSP.Service"
#define SEM_SIP_SERVICE (const char*)"/SIP.Service"
#define SEM_PLBK_SERVICE (const char*)"/Playback.Service"
#define SEM_VEDIT_SERVICE (const char*)"/VideoEdit.Service"
#define SEM_EFM_SRC_SERVICE (const char*)"/EFMSource.Service"

#endif

