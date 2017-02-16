/*******************************************************************************
 * am_video_injecter.cpp
 *
 * History:
 *    2015/12/28 - [Zhi He] create file
 *
 * Copyright (C) 2015 Ambarella, Inc.
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

#include "am_native.h"
#include "am_native_log.h"

#include "am_internal.h"

#include "am_osal.h"
#include "am_framework.h"
#include "am_modules_interface.h"
#include "am_codec_interface.h"

#include "am_video_injecter.h"

IFilter *gfCreateVideoInjecterFilter(const TChar *pName, const volatile SPersistMediaConfig *pPersistMediaConfig, IMsgSink *pEngineMsgSink, TU32 index)
{
  return CVideoInjecter::Create(pName, pPersistMediaConfig, pEngineMsgSink, index);
}

static EVideoInjecterModuleID _guess_video_injecter_type_from_string(TChar *string)
{
  if (!string) {
    LOG_NOTICE("NULL input in _guess_video_injecter_type_from_string, choose default\n");
    return EVideoInjecterModuleID_AMBAEFM;
  }
  if (!strncmp(string, DNameTag_AMBAEFM, strlen(DNameTag_AMBAEFM))) {
    return EVideoInjecterModuleID_AMBAEFM;
  }
  LOG_WARN("unknown string tag(%s) in _guess_video_injecter_type_from_string, choose default\n", string);
  return EVideoInjecterModuleID_AMBAEFM;
}

//-----------------------------------------------------------------------
//
// CVideoInjecter
//
//-----------------------------------------------------------------------
IFilter *CVideoInjecter::Create(const TChar *pName, const volatile SPersistMediaConfig *pPersistMediaConfig, IMsgSink *pEngineMsgSink, TU32 index)
{
  CVideoInjecter *result = new CVideoInjecter(pName, pPersistMediaConfig, pEngineMsgSink, index);
  if (result && result->Construct() != EECode_OK) {
    delete result;
    result = NULL;
  }
  return result;
}

CVideoInjecter::CVideoInjecter(const TChar *pName, const volatile SPersistMediaConfig *pPersistMediaConfig, IMsgSink *pEngineMsgSink, TU32 index)
  : inherited(pName, index, pPersistMediaConfig, pEngineMsgSink)
  , mpInjecter(NULL)
  , mpCurrentInputPin(NULL)
  , mnInputPinsNumber(0)
  , mpBuffer(NULL)
  , mVideoWidth(0)
  , mVideoHeight(0)
  , mVideoFramerateNum(0)
  , mVideoFramerateDen(0)
  , mFormat(StreamFormat_Invalid)
  , mModuleID(EVideoInjecterModuleID_Invalid)
  , mbPaused(0)
  , mbStartStastics(0)
  , mStasticsFrameCount(0)
  , mStasticsBeginTime(0)
  , mStasticsEndTime(0)
{
  TUint i = 0;
  for (i = 0; i < EConstVideoRendererMaxInputPinNumber; i ++) {
    mpInputPins[i] = 0;
  }
  mStreamIndex = 0xff;
  mpLastBufferForEOS = NULL;
  mbSendLastFrameFlag = 0;
}

EECode CVideoInjecter::Construct()
{
  DSET_MODULE_LOG_CONFIG(ELogModuleVideoRendererFilter);
  return inherited::Construct();
}

CVideoInjecter::~CVideoInjecter()
{
}

void CVideoInjecter::Delete()
{
  TUint i = 0;
  for (i = 0; i < EConstVideoRendererMaxInputPinNumber; i ++) {
    if (mpInputPins[i]) {
      mpInputPins[i]->Delete();
      mpInputPins[i] = NULL;
    }
  }
  if (mpInjecter) {
    mpInjecter->Destroy();
    mpInjecter = NULL;
  }
  inherited::Delete();
}

EECode CVideoInjecter::Initialize(TChar *p_param)
{
  mModuleID = _guess_video_injecter_type_from_string(p_param);
  return EECode_OK;
}

EECode CVideoInjecter::Finalize()
{
  if (mpInjecter) {
    mpInjecter->DestroyContext();
  }
  return EECode_OK;
}

void CVideoInjecter::PrintState()
{
  LOGM_FATAL("TO DO\n");
}

EECode CVideoInjecter::Run()
{
  DASSERT(mpEngineMsgSink);
  DASSERT(mpPersistMediaConfig);
  DASSERT(mpInputPins[0]);
  inherited::Run();
  return EECode_OK;
}

EECode CVideoInjecter::Start()
{
  DASSERT(mpEngineMsgSink);
  DASSERT(mpPersistMediaConfig);
  DASSERT(mpInputPins[0]);
  return inherited::Start();
}

EECode CVideoInjecter::Stop()
{
  DASSERT(mpEngineMsgSink);
  DASSERT(mpPersistMediaConfig);
  DASSERT(mpInputPins[0]);
  inherited::Stop();
  return EECode_OK;
}

void CVideoInjecter::Pause()
{
  DASSERT(mpEngineMsgSink);
  DASSERT(mpPersistMediaConfig);
  DASSERT(mpInputPins[0]);
  inherited::Pause();
  return;
}

void CVideoInjecter::Resume()
{
  DASSERT(mpEngineMsgSink);
  DASSERT(mpPersistMediaConfig);
  DASSERT(mpInputPins[0]);
  inherited::Resume();
  return;
}

void CVideoInjecter::Flush()
{
  DASSERT(mpEngineMsgSink);
  DASSERT(mpPersistMediaConfig);
  DASSERT(mpInputPins[0]);
  inherited::Flush();
  return;
}

void CVideoInjecter::ResumeFlush()
{
  LOGM_FATAL("TO DO\n");
  return;
}

EECode CVideoInjecter::Suspend(TUint release_context)
{
  LOGM_FATAL("TO DO\n");
  return EECode_NoImplement;
}

EECode CVideoInjecter::ResumeSuspend(TComponentIndex input_index)
{
  LOGM_FATAL("TO DO\n");
  return EECode_NoImplement;
}

EECode CVideoInjecter::SwitchInput(TComponentIndex focus_input_index)
{
  LOGM_FATAL("TO DO\n");
  return EECode_NoImplement;
}

EECode CVideoInjecter::FlowControl(EBufferType flowcontrol_type)
{
  LOGM_FATAL("TO DO\n");
  return EECode_NoImplement;
}

IInputPin *CVideoInjecter::GetInputPin(TUint index)
{
  if (EConstVideoRendererMaxInputPinNumber > index) {
    return mpInputPins[index];
  } else {
    LOGM_ERROR("BAD index %d, max value %d, in CVideoInjecter::GetInputPin()\n", index, EConstVideoRendererMaxInputPinNumber);
  }
  return NULL;
}

IOutputPin *CVideoInjecter::GetOutputPin(TUint index, TUint sub_index)
{
  LOGM_FATAL("CVideoInjecter do not have output pin\n");
  return NULL;
}

EECode CVideoInjecter::AddInputPin(TUint &index, TUint type)
{
  if (StreamType_Video == type) {
    if (mnInputPinsNumber >= EConstVideoRendererMaxInputPinNumber) {
      LOGM_ERROR("Max input pin number reached.\n");
      return EECode_InternalLogicalBug;
    }
    index = mnInputPinsNumber;
    DASSERT(!mpInputPins[mnInputPinsNumber]);
    if (mpInputPins[mnInputPinsNumber]) {
      LOGM_FATAL("mpInputPins[mnInputPinsNumber] here, must have problems!!! please check it\n");
    }
    mpInputPins[mnInputPinsNumber] = CQueueInputPin::Create("[video input pin for CVideoInjecter]", (IFilter *) this, StreamType_Video, mpWorkQ->MsgQ());
    DASSERT(mpInputPins[mnInputPinsNumber]);
    if (!mnInputPinsNumber) {
      mpCurrentInputPin = mpInputPins[0];
    }
    mnInputPinsNumber ++;
    return EECode_OK;
  } else {
    LOGM_FATAL("BAD input pin type %d\n", type);
    return EECode_InternalLogicalBug;
  }
  return EECode_OK;
}

EECode CVideoInjecter::AddOutputPin(TUint &index, TUint &sub_index, TUint type)
{
  LOGM_FATAL("CVideoInjecter can not add a output pin\n");
  return EECode_OK;
}

void CVideoInjecter::EventNotify(EEventType type, TU64 param1, TPointer param2)
{
  switch (type) {
    default:
      LOG_FATAL("to do\n");
      break;
  }
}

EECode CVideoInjecter::FilterProperty(EFilterPropertyType property, TUint set_or_get, void *p_param)
{
  EECode err = EECode_OK;
  switch (property) {
    case EFilterPropertyType_set_stream_id: {
        SConfigVideoStreamID *p_vsid = (SConfigVideoStreamID *) p_param;
        DASSERT(p_vsid);
        if (p_vsid) {
          DASSERT(EGenericEngineConfigure_VideoStreamID == p_vsid->check_field);
          if (p_vsid->stream_id & (1 << 0)) {
            mStreamIndex = 0;
          } else if (p_vsid->stream_id & (1 << 1)) {
            mStreamIndex = 1;
          } else if (p_vsid->stream_id & (1 << 2)) {
            mStreamIndex = 2;
          } else if (p_vsid->stream_id & (1 << 3)) {
            mStreamIndex = 3;
          } else if (p_vsid->stream_id & (1 << 4)) {
            mStreamIndex = 4;
          } else if (p_vsid->stream_id & (1 << 5)) {
            mStreamIndex = 5;
          } else if (p_vsid->stream_id & (1 << 6)) {
            mStreamIndex = 6;
          } else if (p_vsid->stream_id & (1 << 7)) {
            mStreamIndex = 7;
          } else {
            LOGM_FATAL("do not find stream index 0~7, 0x%08x\n", p_vsid->stream_id);
            return EECode_BadParam;
          }
        }
      }
      break;
    default:
      LOGM_FATAL("BAD property 0x%08x\n", property);
      err = EECode_InternalLogicalBug;
      break;
  }
  return err;
}

void CVideoInjecter::GetInfo(INFO &info)
{
  info.mPriority = 0;
  info.mFlags = 0;
  info.nInput = 1;
  info.nOutput = 0;
  info.pName = mpModuleName;
}

void CVideoInjecter::PrintStatus()
{
  TUint i = 0;
  LOGM_PRINTF("\t[PrintStatus] CVideoInjecter[%d]: msState=%d, %s\n", mIndex, msState, gfGetModuleStateString(msState));
  for (i = 0; i < mnInputPinsNumber; i ++) {
    if (mpInputPins[i]) {
      mpInputPins[i]->PrintStatus();
    }
  }
  return;
}

EECode CVideoInjecter::flushInputData(TU8 disable_pin)
{
  if (mpBuffer) {
    mpBuffer->Release();
    mpBuffer = NULL;
  }
  if (mpLastBufferForEOS) {
    mpLastBufferForEOS->Release();
    mpLastBufferForEOS = NULL;
  }
  LOGM_FLOW("before purge input pins\n");
  if (mpCurrentInputPin) {
    mpCurrentInputPin->Purge(disable_pin);
  }
  LOGM_FLOW("after purge input pins\n");
  return EECode_OK;
}

EECode CVideoInjecter::processCmd(SCMD &cmd)
{
  EECode err = EECode_OK;
  switch (cmd.code) {
    case ECMDType_ExitRunning:
      mbRun = 0;
      flushInputData(1);
      msState = EModuleState_Halt;
      mpWorkQ->CmdAck(EECode_OK);
      break;
    case ECMDType_Start:
      if (EModuleState_WaitCmd == msState) {
        msState = EModuleState_Idle;
      }
      mStasticsFrameCount = 0;
      mpWorkQ->CmdAck(EECode_OK);
      break;
    case ECMDType_Stop:
      if (!mbSendLastFrameFlag) {
        hackFeedLastFrame();
      }
      flushInputData(1);
      msState = EModuleState_WaitCmd;
      mpWorkQ->CmdAck(EECode_OK);
      break;
    case ECMDType_Flush:
      if (mpBuffer) {
        mpBuffer->Release();
        mpBuffer = NULL;
      }
      msState = EModuleState_Stopped;
      mpWorkQ->CmdAck(EECode_OK);
      break;
    case ECMDType_Step:
      msState = EModuleState_Step;
      break;
    case ECMDType_Pause:
      if (mpBuffer) {
        injectBuffer();
        mpBuffer->Release();
        mpBuffer = NULL;
      }
      mbPaused = 1;
      msState = EModuleState_Pending;
      break;
    case ECMDType_Resume:
      if (mpBuffer) {
        injectBuffer();
        mpBuffer->Release();
        mpBuffer = NULL;
      }
      mbPaused = 0;
      msState = EModuleState_Idle;
      break;
    default:
      LOGM_ERROR("processCmd, wrong cmd %d, %s.\n", cmd.code, gfGetCMDTypeString((ECMDType)cmd.code));
      break;
  }
  return err;
}

EECode CVideoInjecter::injectBuffer()
{
  if (DLikely(mpInjecter && mpBuffer)) {
      if (0xff != mStreamIndex) {
        EECode err = EECode_OK;
        TU32 is_last_frame = mpBuffer->GetBufferFlags() & CIBuffer::LAST_FRAME_INDICATOR;
        if (mpLastBufferForEOS) {
          mpLastBufferForEOS->Release();
          mpLastBufferForEOS = NULL;
        }
        mpLastBufferForEOS = mpBuffer;
        mpLastBufferForEOS->AddRef();
        if (is_last_frame) {
          mbSendLastFrameFlag = 1;
        }
        err = mpInjecter->Inject(mpBuffer, mStreamIndex);
        if (DUnlikely(EECode_OK != err)) {
          LOGM_ERROR("inject fail, err %d, %s\n", err, gfGetErrorCodeString(err));
          return err;
        }
        if (is_last_frame) {
          mpBuffer->ClearBufferFlagBits(CIBuffer::LAST_FRAME_INDICATOR);
          if (!strcmp(gfGetDSPPlatformName(), "S2L")) {
            LOGM_NOTICE("feed extra frame on S2L\n");
            mpInjecter->Inject(mpBuffer, mStreamIndex);
          }
        }
        if (!mStasticsFrameCount) {
          mStasticsBeginTime = gfGetRalativeTime();
        }
        mStasticsFrameCount ++;
        if (!(mStasticsFrameCount & 0xff)) {
          mStasticsEndTime = gfGetRalativeTime();
          LOGM_PRINTF("EFM feed FPS: %lf\n", (double) ((double) mStasticsFrameCount * (double) 1000000) / (double) ((double) mStasticsEndTime - (double) mStasticsBeginTime));
        }
      } else {
        LOGM_FATAL("inject stream index not specified yet\n");
      }
  } else {
    LOGM_FATAL("null pointer, %p, %p\n", mpInjecter, mpBuffer);
    return EECode_InternalLogicalBug;
  }
  return EECode_OK;
}

EECode CVideoInjecter::hackFeedLastFrame()
{
  if (DLikely(mpInjecter && mpLastBufferForEOS)) {
    if (0xff != mStreamIndex) {
      EECode err = EECode_OK;
      DASSERT(!mbSendLastFrameFlag);
      mpLastBufferForEOS->SetBufferFlagBits(CIBuffer::LAST_FRAME_INDICATOR);
      err = mpInjecter->Inject(mpLastBufferForEOS, mStreamIndex);
      mbSendLastFrameFlag = 1;
      if (DUnlikely(EECode_OK != err)) {
        LOGM_ERROR("inject fail, err %d, %s\n", err, gfGetErrorCodeString(err));
        return err;
      }
      mpLastBufferForEOS->ClearBufferFlagBits(CIBuffer::LAST_FRAME_INDICATOR);
      if (!strcmp(gfGetDSPPlatformName(), "S2L")) {
        LOGM_NOTICE("feed extra frame on S2L\n");
        mpInjecter->Inject(mpLastBufferForEOS, mStreamIndex);
      }
    } else {
      LOGM_FATAL("inject stream index not specified yet\n");
    }
  } else {
    LOGM_FATAL("null pointer, %p, %p\n", mpInjecter, mpBuffer);
    return EECode_InternalLogicalBug;
  }
  return EECode_OK;
}

void CVideoInjecter::OnRun()
{
  SCMD cmd;
  CIQueue::QType type;
  CIQueue::WaitResult result;
  TU32 renew = 0;
  EECode err = EECode_OK;
  mbRun = 1;
  msState = EModuleState_WaitCmd;
  mpWorkQ->CmdAck(EECode_OK);
  //LOGM_INFO("OnRun loop, start\n");
  while (mbRun) {
    LOGM_STATE("start switch, msState=%d, %s\n", msState, gfGetModuleStateString(msState));
    switch (msState) {
      case EModuleState_WaitCmd:
        mpWorkQ->WaitMsg(&cmd, sizeof(cmd));
        processCmd(cmd);
        break;
      case EModuleState_Halt:
        mpWorkQ->WaitMsg(&cmd, sizeof(cmd));
        processCmd(cmd);
        break;
      case EModuleState_Idle:
        if (mbPaused) {
          msState = EModuleState_Pending;
          break;
        }
        type = mpWorkQ->WaitDataMsg(&cmd, sizeof(cmd), &result);
        if (type == CIQueue::Q_MSG) {
          processCmd(cmd);
        } else {
          mpCurrentInputPin = (CQueueInputPin *)result.pOwner;
          DASSERT(!mpBuffer);
          if (mpCurrentInputPin->PeekBuffer(mpBuffer)) {
            msState = EModuleState_Running;
          } else {
            LOGM_FATAL("mpCurrentInputPin->PeekBuffer(mpBuffer) fail?\n");
            msState = EModuleState_Error;
            flushInputData(1);
          }
        }
        break;
      case EModuleState_Completed:
        type = mpWorkQ->WaitDataMsg(&cmd, sizeof(cmd), &result);
        if (type == CIQueue::Q_MSG) {
          processCmd(cmd);
        } else {
          msState = EModuleState_Idle;
        }
        break;
      case EModuleState_Stopped:
      case EModuleState_Pending:
        mpWorkQ->WaitMsg(&cmd, sizeof(cmd));
        processCmd(cmd);
        break;
      case EModuleState_Error:
        if (!mbInputPinDisabled) {
          mbInputPinDisabled = 1;
          if (mpCurrentInputPin) {
            mpCurrentInputPin->Purge(1);
          }
        }
        mpWorkQ->WaitMsg(&cmd, sizeof(cmd));
        processCmd(cmd);
        break;
      case EModuleState_Running:
        DASSERT(mpBuffer);
        //DASSERT(mpInjecter);
        if (EBufferType_FlowControl_EOS == mpBuffer->GetBufferType()) {
          LOGM_NOTICE("get EOS\n");
          if (mpEngineMsgSink) {
            SMSG msg;
            msg.code = EMSGType_VideoEditEOS;
            msg.p_owner = (TULong)((IFilter *)this);
            msg.p3 = (mpBuffer->mNativeDTS >> 32) & 0xffffffff;
            msg.p4 = (mpBuffer->mNativeDTS) & 0xffffffff;
            mpEngineMsgSink->MsgProc(msg);
          }
          mpBuffer->Release();
          mpBuffer = NULL;
          msState = EModuleState_Completed;
          break;
        } else {
          if (DUnlikely(mpBuffer->GetBufferFlags() & (CIBuffer::NEW_SEQUENCE | CIBuffer::SYNC_POINT))) {
            TU32 cur_width = mpBuffer->mVideoWidth;
            TU32 cur_height = mpBuffer->mVideoHeight;
            TU32 cur_fr_num = mpBuffer->mVideoFrameRateNum;
            TU32 cur_fr_den = mpBuffer->mVideoFrameRateDen;
            StreamFormat cur_format = (StreamFormat) mpBuffer->mContentFormat;
            if ((!cur_width) || (!cur_height) || (!cur_fr_num) || (!cur_fr_den)) {
              LOGM_FATAL("zero config value: %dx%d, %d/%d, buffer flags 0x%08x\n", cur_width, cur_height, cur_fr_num, cur_fr_den, mpBuffer->GetBufferFlags());
              flushInputData(1);
              msState = EModuleState_Error;
              break;
            }
            if (mpInjecter) {
              if ((cur_width != mVideoWidth) || (cur_height != mVideoHeight)) {
                LOGM_CONFIGURATION("resolution change: %dx%d --> %dx%d\n", mVideoWidth, mVideoHeight, cur_width, cur_height);
                renew = 1;
              }
              if ((cur_fr_num != mVideoFramerateNum) || (cur_fr_den != mVideoFramerateDen)) {
                LOGM_CONFIGURATION("framerate change: %d/%d --> %d/%d\n", mVideoFramerateNum, mVideoFramerateDen, cur_fr_num, cur_fr_den);
                renew = 1;
              }
              if (cur_format != mFormat) {
                LOGM_CONFIGURATION("format change: 0x%08x --> 0x%08x\n", mFormat, cur_format);
                renew = 1;
              }
              if (renew) {
                mpInjecter->Destroy();
                mpInjecter = NULL;
              }
              mVideoWidth = cur_width;
              mVideoHeight = cur_height;
              mVideoFramerateNum = cur_fr_num;
              mVideoFramerateDen = cur_fr_den;
              mFormat = cur_format;
            }
            if (DUnlikely(!mpInjecter)) {
              mpInjecter = gfModuleFactoryVideoInjecter(mModuleID, mpPersistMediaConfig, mpEngineMsgSink);
              if (DUnlikely(!mpInjecter)) {
                LOGM_FATAL("gfModuleFactoryVideoInjecter fail, format 0x%08x\n", mModuleID);
                flushInputData(1);
                msState = EModuleState_Error;
                break;
              }
            }
            SVideoInjectParams params;
            params.width = mVideoWidth;
            params.height = mVideoHeight;
            params.framerate_num = mVideoFramerateNum;
            params.framerate_den = mVideoFramerateDen;
            params.format = mFormat;
            err = mpInjecter->SetupContext(&params);
            if (DUnlikely(EECode_OK != err)) {
              LOGM_FATAL("SetupContext fail, err 0x%08x, %s\n", err, gfGetErrorCodeString(err));
              flushInputData(1);
              msState = EModuleState_Error;
              break;
            }
          }
          err = injectBuffer();
          if (DUnlikely(EECode_OK != err)) {
            LOGM_FATAL("injectBuffer fail, err 0x%08x, %s\n", err, gfGetErrorCodeString(err));
            flushInputData(1);
            msState = EModuleState_Error;
            break;
          }
        }
        mpBuffer->Release();
        mpBuffer = NULL;
        msState = EModuleState_Idle;
        break;
      case EModuleState_WaitTiming:
        mpWorkQ->WaitMsg(&cmd, sizeof(cmd));
        processCmd(cmd);
        break;
      default:
        LOGM_ERROR("CVideoInjecter: OnRun: state invalid: 0x%x\n", (TUint)msState);
        msState = EModuleState_Error;
        flushInputData(1);
        break;
    }
  }
}


