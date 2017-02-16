/**
 * test_oryx_video_reader.cpp
 *
 *  History:
 *    Aug 11, 2015 - [Shupeng Ren] created file
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
#include "am_log.h"
#include "am_video_reader_if.h"
#include "am_video_address.h"

#define NO_ARG    0
#define HAS_ARG   1

static bool file_name_base_flag = false;
std::string file_name_base;
static bool dump_video_flag = false;
static bool dump_yuv_flag = false;
static uint32_t dump_yuv_buffer_id = 0;
static bool dump_luma_flag = false;
static uint32_t dump_luma_buffer_id = 0;
static bool dump_raw_flag = false;

static AMIVideoReaderPtr g_reader = nullptr;

enum numeric_short_options {
  DUMP_VIDEO  = 'z' + 1,
  DUMP_YUV    = 'z' + 2,
  DUMP_LUMA   = 'z' + 3,
  DUMP_RAW    = 'z' + 4
};

static struct option long_options[]={
  {"filename_base",   HAS_ARG,  0, 'f'},
  {"video",           NO_ARG,   0, DUMP_VIDEO },
  {"yuv",             HAS_ARG,  0, DUMP_YUV },
  {"luma",            HAS_ARG,  0, DUMP_LUMA},
  {"raw",             NO_ARG,   0, DUMP_RAW},
  {0, 0, 0, 0}
};

static const char *short_options = "f:";

struct hint_s {
  const char *arg;
  const char *str;
};

static const struct hint_s hint[] = {
  {"filename_base", "specify filename with path"},
  {"", "\t\tdump encoded video stream(s) to file(s)"},
  {"0~3", "\t\tbuffer_id, dump yuv of this buffer to file"},
  {"0~3", "\t\tbuffer_id, dump me1 (luma) of this buffer to file"},
  {"", "\t\tdump Bayer pattern RAW format to file"},
};

static void usage(int argc, char **argv)
{
  uint32_t i;
  printf("%s usage:\n", argv[0]);
  for (i = 0; i < sizeof(long_options) / sizeof(long_options[0]) - 1; i++) {
    if (isalpha(long_options[i].val))
      printf("-%c ", long_options[i].val);
    else
      printf("   ");
    printf("--%s", long_options[i].name);
    if (hint[i].arg[0] != 0)
      printf(" [%s]", hint[i].arg);
    printf("\t%s\n", hint[i].str);
  }
  printf("\n");
}

static int init_param(int argc, char **argv)
{

  int ch;
  int option_index = 0;

  opterr = 0;
  while ((ch = getopt_long(argc,
                           argv,
                           short_options,
                           long_options,
                           &option_index)) != -1) {
    switch (ch) {
      case 'f':
        std::string tmp = optarg;
        file_name_base += optarg;
        file_name_base_flag = true;
        break;

      case DUMP_VIDEO:
        dump_video_flag = true;
        break;

      case DUMP_YUV:
        dump_yuv_buffer_id = atoi(optarg);
        dump_yuv_flag = true;
        break;

      case DUMP_LUMA:
        dump_luma_buffer_id = atoi(optarg);
        dump_luma_flag = true;
        break;

      case DUMP_RAW:
        dump_raw_flag = true;
        break;

      default:
        printf("unknown command %s \n", optarg);
        return -1;
        break;

    }

  }

  if (!file_name_base_flag) {
    ERROR("please enter a file base name by '-f'\n");
    return -1;

  }

  return 0;
}

void sigstop(int)
{
  INFO("Force quit by signal\n");
//  quit_flag  = true;
}

static AM_RESULT dump_video()
{
  AM_RESULT result = AM_RESULT_OK;

  do {
    AMQueryFrameDesc frame_desc;

  } while (0);

  return result;
}

int main(int argc, char **argv)
{
  int ret = 0;
  signal(SIGINT, sigstop);
  signal(SIGQUIT, sigstop);
  signal(SIGTERM, sigstop);

  do {
    if (argc < 2) {
      usage(argc, argv);
      ret = -1;
      break;
    }

    if (init_param(argc, argv) < 0) {
      ERROR("Failed to init param!");
      ret = -2;
      break;
    }
  } while (0);
  return 0;
}
