/*
 * test_efm_src_service_air_api.h
 *
 * @Author: Zhi He
 * @Email : zhe@ambarella.com
 * @Time  : 04/05/2016 [Created]
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

#include <signal.h>
#include "am_base_include.h"
//#include "am_log.h"

#include "am_api_helper.h"
#include "am_api_media.h"
#include "am_api_efm_src.h"

static bool g_efm_src_service_running = true;

static void sigstop (int32_t arg)
{
  printf ("Signal has been captured, test_efm_src_service_air_api quits!\n");
  g_efm_src_service_running = false;
}

static void __print_ut_options()
{
  printf ("\n=============== oryx efm source service test =================\n\n");
  printf("\t'-i [filename]': '-i' specify input file name (can be .mjpeg, .h264, .h265)\n");
  printf("\t'--feed-es': es file --> yuv --> efm\n");
  printf("\t'--transcode-es': es file --> yuv --> efm --> mp4\n");
  printf("\t'--feed-usbcam': USB camera --> yuv --> efm\n");
  printf("\t'--end-feed': end feed\n");
  printf("\t'--end-transcode': end transcode\n");
  printf("\t'-f [%%d]': '-f' specify prefer fps of USB camera\n");
  printf("\t'-s [%%dx%%d]': '-s' specify prefer video resolution of USB camera\n");
  printf("\t'--dump-efm-src [dump file name]': dump efm source, yuv420 format\n");
  printf("\t'-A': use stream 0 as EFM\n");
  printf("\t'-B': use stream 1 as EFM\n");
  printf("\t'-C': use stream 2 as EFM\n");
  printf("\t'-D': use stream 3 as EFM\n");
  printf ("\n=========================================================\n\n");
}

static void __print_ut_cmds()
{
  printf("test_efm_src_service_air_api runtime cmds: press cmd + Enter\n");
  printf("\t'q': Quit unit test\n");
}

enum {
  EFEED_EFM_SOURCE_INVALID = 0x00,
  EFEED_EFM_SOURCE_LOCAL_ES_FILE = 0x01,
  EFEED_EFM_SOURCE_USB_CAMERA = 0x02,
  EFEED_EFM_TRANSCODE_ES_FILE = 0x03,
  EEND_FEED_EFM = 0x04,
  EEND_TRANSCODE = 0x05,
};

struct test_efm_source_service_air_api_setting_t {
  unsigned char efm_src_operation_mode;
  unsigned char reserved0;
  unsigned char reserved1;
  unsigned char reserved2;
};

static int __init_test_efm_src_air_api_params(int argc, char **argv, am_efm_src_context_t *ctx, test_efm_source_service_air_api_setting_t *setting)
{
  int i = 0;
  for (i = 1; i < argc; i++) {
    if (!strcmp("-i", argv[i])) {
      if ((i + 1) < argc) {
        snprintf(ctx->input_url, AM_EFM_SRC_MAX_URL_LENGTH, "%s", argv[i + 1]);
        i ++;
        printf("[input argument] -i: input url: %s.\n", ctx->input_url);
      } else {
        printf("[input argument] -i: should follow input filename.\n");
      }
    } else if (!strcmp("--feed-es", argv[i])) {
      setting->efm_src_operation_mode = EFEED_EFM_SOURCE_LOCAL_ES_FILE;
      printf("[input argument] --feed-es\n");
    } else if (!strcmp("--feed-usbcam", argv[i])) {
      setting->efm_src_operation_mode = EFEED_EFM_SOURCE_USB_CAMERA;
      printf("[input argument] --feed-usbcam\n");
    } else if (!strcmp("--transcode-es", argv[i])) {
      setting->efm_src_operation_mode = EFEED_EFM_TRANSCODE_ES_FILE;
      printf("[input argument] --transcode-es\n");
    } else if (!strcmp("--end-feed", argv[i])) {
      setting->efm_src_operation_mode = EEND_FEED_EFM;
      printf("[input argument] --end-feed\n");
    } else if (!strcmp("--end-transcode", argv[i])) {
      setting->efm_src_operation_mode = EEND_TRANSCODE;
      printf("[input argument] --end-transcode\n");
    } else if (!strcmp("-f", argv[i])) {
      if ((i + 1) < argc) {
        int ret = sscanf(argv[i + 1], "%d", &ctx->prefer_video_fps);
        if (1 == ret) {
          printf("[input argument] -f: %d.\n", ctx->prefer_video_fps);
        } else {
          printf("[input argument] -f: should follow 'fps', %%d.\n");
          return (-1);
        }
        i ++;
      } else {
        printf("[input argument] -f: should follow 'fps', %%d.\n");
        return (-1);
      }
    } else if (!strcmp("-s", argv[i])) {
      if ((i + 1) < argc) {
        int ret = sscanf(argv[i + 1], "%dx%d", &ctx->prefer_video_width, &ctx->prefer_video_height);
        if (2 == ret) {
          printf("[input argument] -s: %dx%d.\n", ctx->prefer_video_width, ctx->prefer_video_height);
        } else {
          printf("[input argument] -s: should follow 'width x height', %%dx%%d.\n");
          return (-1);
        }
        i ++;
      } else {
        printf("[input argument] -s: should follow 'width x height'\n");
        return (-1);
      }
    } else if (!strcmp("--dump-efm-src", argv[i])) {
      if ((i + 1) < argc) {
        snprintf(ctx->dump_efm_src_url, AM_EFM_SRC_MAX_URL_LENGTH, "%s", argv[i + 1]);
        i ++;
        printf("[input argument] --dump-efm-src: filename %s.\n", ctx->dump_efm_src_url);
      } else {
        printf("[input argument] --dump-efm-src: should follow dump file name.\n");
        return (-1);
      }
      ctx->b_dump_efm_src = 1;
    } else if (!strcmp("--capyuv", argv[i])) {
      ctx->prefer_format = AM_EFM_SRC_FORMAT_YUYV;
    } else if (!strcmp("--capjpeg", argv[i])) {
      ctx->prefer_format = AM_EFM_SRC_FORMAT_MJPEG;
    } else if (!strcmp("-A", argv[i])) {
      ctx->stream_index = 0;
    } else if (!strcmp("-B", argv[i])) {
      ctx->stream_index = 1;
    } else if (!strcmp("-C", argv[i])) {
      ctx->stream_index = 2;
    } else if (!strcmp("-D", argv[i])) {
      ctx->stream_index = 3;
    } else if (!strcmp("--help", argv[i])) {
      __print_ut_options();
      __print_ut_cmds();
    } else {
      printf("error: NOT processed option(%s).\n", argv[i]);
      __print_ut_options();
      __print_ut_cmds();
      return (-1);
    }
  }
  return 0;
}

static int __feed_es_to_efm(am_efm_src_context_t *ctx)
{
  am_service_result_t service_result;

  AMAPIHelperPtr api_helper = NULL;
  if ((api_helper = AMAPIHelper::get_instance ()) == NULL) {
    printf ("Failed to get an instance of AMAPIHelper!");
    return AM_EFM_SRC_SERVICE_RETCODE_NOT_AVIALABLE;
  }

  api_helper->method_call(
      AM_IPC_MW_CMD_FEED_EFM_FROM_LOCAL_ES_FILE,
      ctx,
      sizeof (am_efm_src_context_t),
      &service_result,
      sizeof (am_service_result_t));

  if (service_result.ret < 0) {
    printf ("feed es to EFM fail, ret %d\n", service_result.ret);
  }

  return service_result.ret;
}

static int __feed_usbcam_to_efm(am_efm_src_context_t *ctx)
{
  am_service_result_t service_result;

  AMAPIHelperPtr api_helper = NULL;
  if ((api_helper = AMAPIHelper::get_instance ()) == NULL) {
    printf ("Failed to get an instance of AMAPIHelper!");
    return AM_EFM_SRC_SERVICE_RETCODE_NOT_AVIALABLE;
  }

  api_helper->method_call(
      AM_IPC_MW_CMD_FEED_EFM_FROM_USB_CAMERA,
      ctx,
      sizeof (am_efm_src_context_t),
      &service_result,
      sizeof (am_service_result_t));

  if (service_result.ret < 0) {
    printf ("feed usb camera to EFM fail, ret %d\n", service_result.ret);
  }

  return service_result.ret;
}

static int __start_file_recording(uint32_t request_muxer_id)
{
  am_service_result_t service_result;
  uint32_t muxer_id = 0x01 << request_muxer_id;

  AMAPIHelperPtr api_helper = NULL;
  if ((api_helper = AMAPIHelper::get_instance ()) == NULL) {
    printf ("Failed to get an instance of AMAPIHelper!");
    return AM_EFM_SRC_SERVICE_RETCODE_NOT_AVIALABLE;
  }

  api_helper->method_call(AM_IPC_MW_CMD_MEDIA_START_FILE_RECORDING,
    &muxer_id, sizeof(muxer_id), &service_result, sizeof(service_result));
  if (service_result.ret < 0) {
    printf ("start file recording fail, ret %d\n", service_result.ret);
  }

  return service_result.ret;
}

static int __stop_file_recording(uint32_t request_muxer_id)
{
  am_service_result_t service_result;
  uint32_t muxer_id = 0x01 << request_muxer_id;

  AMAPIHelperPtr api_helper = NULL;
  if ((api_helper = AMAPIHelper::get_instance ()) == NULL) {
    printf ("Failed to get an instance of AMAPIHelper!");
    return AM_EFM_SRC_SERVICE_RETCODE_NOT_AVIALABLE;
  }

  api_helper->method_call(AM_IPC_MW_CMD_MEDIA_STOP_FILE_RECORDING,
    &muxer_id, sizeof(muxer_id), &service_result, sizeof(service_result));
  if (service_result.ret < 0) {
    printf ("stop file recording fail, ret %d\n", service_result.ret);
  }

  return service_result.ret;
}

static int __end_feed(am_efm_src_instance_id_t *ctx)
{
  am_service_result_t service_result;

  AMAPIHelperPtr api_helper = NULL;
  if ((api_helper = AMAPIHelper::get_instance ()) == NULL) {
    printf ("Failed to get an instance of AMAPIHelper!");
    return AM_EFM_SRC_SERVICE_RETCODE_NOT_AVIALABLE;
  }

  api_helper->method_call(
      AM_IPC_MW_CMD_END_FEED_EFM,
      ctx,
      sizeof (am_efm_src_instance_id_t),
      &service_result,
      sizeof (am_service_result_t));

  if (service_result.ret < 0) {
    printf ("end feed efm fail, ret %d\n", service_result.ret);
  }

  return service_result.ret;
}

int main (int argc, char **argv)
{
  am_efm_src_context_t ctx;
  test_efm_source_service_air_api_setting_t setting;
  int ret = 0;

  signal (SIGINT , sigstop);
  signal (SIGTERM, sigstop);
  signal (SIGQUIT, sigstop);

  memset(&setting, 0x0, sizeof(setting));
  memset(&ctx, 0x0, sizeof(ctx));
  ctx.instance_id = 0;

  if (argc < 2) {
    __print_ut_options();
    return 0;
  }

  ret = __init_test_efm_src_air_api_params(argc, argv, &ctx, &setting);
  if (ret) {
    return (-1);
  }

  if (EFEED_EFM_SOURCE_LOCAL_ES_FILE == setting.efm_src_operation_mode) {
    printf("EFM stream index %d\n", ctx.stream_index);
    ret = __feed_es_to_efm(&ctx);
  } else if (EFEED_EFM_SOURCE_USB_CAMERA == setting.efm_src_operation_mode) {
    printf("EFM stream index %d\n", ctx.stream_index);
    ret = __feed_usbcam_to_efm(&ctx);
  } else if (EFEED_EFM_TRANSCODE_ES_FILE == setting.efm_src_operation_mode) {
    printf("EFM stream index %d\n", ctx.stream_index);
    ret = __start_file_recording(11);
    ret = __feed_es_to_efm(&ctx);
  } else if (EEND_FEED_EFM == setting.efm_src_operation_mode) {
    am_efm_src_instance_id_t instance_id;
    instance_id.instance_id = ctx.instance_id;
    ret = __end_feed(&instance_id);
    return ret;
  } else if (EEND_TRANSCODE == setting.efm_src_operation_mode) {
    am_efm_src_instance_id_t instance_id;
    instance_id.instance_id = ctx.instance_id;
    ret = __end_feed(&instance_id);
    ret = __stop_file_recording(11);
    return ret;
  }

  return ret;
}
