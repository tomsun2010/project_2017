/*******************************************************************************
 * am_amf_types.h
 *
 * History:
 *   2014-7-22 - [ypchang] created file
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
#ifndef AM_AMF_TYPES_H_
#define AM_AMF_TYPES_H_
/*
 * types
 */
enum AM_STATE
{
  AM_STATE_OK = 0,
  AM_STATE_ERROR,
  AM_STATE_PENDING,
  AM_STATE_CLOSED,
  AM_STATE_BUSY,
  AM_STATE_FILE_END,
  AM_STATE_TIMEOUT,
  AM_STATE_OS_ERROR,
  AM_STATE_IO_ERROR,
  AM_STATE_TOO_MANY,

  AM_STATE_NOT_IMPL,
  AM_STATE_NOT_EXIST,
  AM_STATE_NOT_SUPPORTED,

  AM_STATE_NO_MEMORY,
  AM_STATE_NO_ACTION,
  AM_STATE_NO_INTERFACE,

  AM_STATE_BAD_STATE,
  AM_STATE_BAD_PARAM,
  AM_STATE_BAD_COMMAND,
  AM_STATE_BAD_FORMAT,
};

enum AM_FILE_TYPE
{
  AM_FILE_NULL = -3,
  AM_FILE_TS   = 13,
  AM_FILE_MP4  = 14,
  AM_FILE_MKV  = 15,
};

typedef int int_ptr;

/*
 * IID
 */

struct AMGuid
{
    uint32_t x;
    uint16_t s1;
    uint16_t s2;
    uint8_t  c[8];
};

typedef struct AMGuid AM_IID;
typedef struct AMGuid AM_GUID;
typedef const AM_IID&  AM_REFIID;
typedef const AM_GUID& AM_REFGUID;

inline bool operator==(AM_REFGUID guid1, AM_REFGUID guid2)
{
  bool ret = false;
  if (&guid1 == &guid2) {
    ret = true;
  } else {
    uint32_t *addr1 = (uint32_t*)&guid1;
    uint32_t *addr2 = (uint32_t*)&guid2;
    ret = ((addr1[0] == addr2[0]) &&
           (addr1[1] == addr2[1]) &&
           (addr1[2] == addr2[2]) &&
           (addr1[3] == addr2[3]));
  }

  return ret;
}

#define AM_DEFINE_IID(name, x, s1, s2, c0, c1, c2, c3, c4, c5, c6, c7) \
  extern const AM_IID name = {x, s1, s2, {c0, c1, c2, c3, c4, c5, c6, c7}}

#define AM_DEFINE_GUID(name, x, s1, s2, c0, c1, c2, c3, c4, c5, c6, c7) \
  extern const AM_GUID name = {x, s1, s2, {c0, c1, c2, c3, c4, c5, c6, c7}}

#endif /* AM_AMF_TYPES_H_ */
