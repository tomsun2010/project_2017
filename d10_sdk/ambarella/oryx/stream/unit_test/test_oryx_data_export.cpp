/*******************************************************************************
 * test_oryx_data_export.cpp
 *
 * History:
 *   2015-01-04 - [Zhi He]      created file
 *   2015-04-02 - [Shupeng Ren] modified  file
 *   2016-07-08 - [Guohua Zheng] modified  file
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

#include <signal.h>
#include <map>

#include <getopt.h>

#include "am_base_include.h"
#include "am_define.h"
#include "am_log.h"
#include "am_export_if.h"

#define NO_ARG 0
#define HAS_ARG 1

using std::map;

std::string namebase;
bool write_flag = 0;

map<uint8_t, FILE*> video_output_file;
map<uint8_t, map <uint8_t, FILE*>> audio_output_file;

static AMIExportClient* g_client = nullptr;
static bool running_flag = true;

static struct option long_options[]=
{
 {"help",            NO_ARG,   0, 'h'},
 {"nodump",         NO_ARG,   0, 'n'},
 {"filename",        HAS_ARG,  0, 'f'},
 {"stream",          HAS_ARG,  0, 's'},
};

static const char *short_options = "hnf:s:";

static void show_usage()
{
  printf("test_oryx_data_export usage:\n");
  printf("\t-f [%%s]: specify output filename base, "
      "final video file name will be 'filename_video_%%d.h264', "
      "same with '--filename'\n");
  printf("\t--filename [%%s]: specify output filename base, "
      "final video file name will be 'filename_video_%%d.h264', "
      "same with '-f'\n");
  printf("\t--stream: specify output stream, "
      "same with '-s'\n");
  printf("\t--nodump: will not save file, print only, "
      "same with '-n'\n ");
  printf("\t--help: show usage\n");
}

static int init_params(int argc, char **argv) {
  int32_t ch;
  int32_t option_index = 0;
  while ((ch = getopt_long(argc, argv, short_options, long_options,
                           &option_index)) != -1) {
    switch (ch) {
      case 'h':
        return -1;
      case 'n':
        write_flag = 1;
        break;
      case 'f':
        namebase = optarg;
        if (namebase.c_str() == nullptr) {
          ERROR("input filename error");
          return -1;
        }
        break;
      case 's':
        break;
      default:
        ERROR("unknown options");
        return -1;
    }
  }
  return 0;
}

static void __sigstop(int i)
{
  running_flag = false;
}

static void open_video_output_file(uint32_t stream_index,
                                   uint8_t packet_format)
{
  char filename[512] = {0};
  switch (packet_format) {
    case AM_EXPORT_PACKET_FORMAT_AVC: {
      snprintf(filename, 512, "%s_video_%d.h264",
               namebase.c_str(), stream_index);
      FILE *check_file = fopen(filename, "wb+");
      if (AM_LIKELY(check_file != nullptr)) {
        video_output_file[stream_index] = check_file;
      } else {
        ERROR("open a file failed");
      }
    } break;
    case AM_EXPORT_PACKET_FORMAT_HEVC: {
      snprintf(filename, 512, "%s_video_%d.h265",
               namebase.c_str(), stream_index);
      FILE *check_file = fopen(filename, "wb+");
      if (AM_LIKELY(check_file != nullptr)) {
        video_output_file[stream_index] = check_file;
      } else {
        ERROR("open a file failed");
      }
    } break;
    case AM_EXPORT_PACKET_FORMAT_MJPEG: {
      snprintf(filename, 512, "%s_video_%d.mjpeg",
               namebase.c_str(), stream_index);
      FILE *check_file = fopen(filename, "wb+");
      if (AM_LIKELY(check_file != nullptr)) {
        video_output_file[stream_index] = check_file;
      } else {
        ERROR("open a file failed");
      }
    } break;
    default:
      ERROR("BAD video format %d", packet_format);
      break;
  }
}

static void open_audio_output_file(uint32_t stream_index,
                                   uint8_t packet_format,
                                   uint8_t sample_rate)
{
  char filename[512] = {0};
  uint8_t tem_index = (sample_rate / 16) > 1 ? 2:(sample_rate / 16);
  switch (packet_format) {
    case AM_EXPORT_PACKET_FORMAT_AAC: {
      snprintf(filename, 512, "%s_audio_%dkHz_%d.aac",
               namebase.c_str(), sample_rate,stream_index);
      FILE* tem_file = fopen(filename, "wb+");
      if (AM_LIKELY(tem_file != nullptr)) {
        audio_output_file[stream_index][tem_index] = tem_file;
      } else {
        ERROR("open a file failed");
      }
    } break;
    case AM_EXPORT_PACKET_FORMAT_G711MuLaw: {
      snprintf(filename, 512, "%s_audio_g711mu_%dkHz_%d.g711",
               namebase.c_str(), sample_rate, stream_index);
      FILE* tem_file = fopen(filename, "wb+");
      if (AM_LIKELY(tem_file != nullptr)) {
        audio_output_file[stream_index][tem_index] = tem_file;
      } else {
        ERROR("open a file failed");
      }
    } break;
    case AM_EXPORT_PACKET_FORMAT_G711ALaw: {
      snprintf(filename, 512, "%s_audio_g711a_%dkHz_%d.g711",
               namebase.c_str(), sample_rate, stream_index);
      FILE* tem_file = fopen(filename, "wb+");
      if (AM_LIKELY(tem_file != nullptr)) {
        audio_output_file[stream_index][tem_index] = tem_file;
      } else {
        ERROR("open a file failed");
      }
    } break;
    case AM_EXPORT_PACKET_FORMAT_G726_40: {
      snprintf(filename, 512, "%s_audio_g726_40_%dkHz_%d.g726",
               namebase.c_str(), sample_rate, stream_index);
      FILE* tem_file = fopen(filename, "wb+");
      if (AM_LIKELY(tem_file != nullptr)) {
        audio_output_file[stream_index][tem_index] = tem_file;
      } else {
        ERROR("open a file failed");
      }
    } break;
    case AM_EXPORT_PACKET_FORMAT_G726_32: {
      snprintf(filename, 512, "%s_audio_g726_32_%dkHz_%d.g726",
               namebase.c_str(), sample_rate, stream_index);
      FILE* tem_file = fopen(filename, "wb+");
      if (AM_LIKELY(tem_file != nullptr)) {
        audio_output_file[stream_index][tem_index] = tem_file;
      } else {
        ERROR("open a file failed");
      }
    } break;
    case AM_EXPORT_PACKET_FORMAT_G726_24: {
      snprintf(filename, 512, "%s_audio_g726_24_%dkHz_%d.g726",
               namebase.c_str(), sample_rate, stream_index);
      FILE* tem_file = fopen(filename, "wb+");
      if (AM_LIKELY(tem_file != nullptr)) {
        audio_output_file[stream_index][tem_index] = tem_file;
      } else {
        ERROR("open a file failed");
      }
    } break;
    case AM_EXPORT_PACKET_FORMAT_G726_16: {
      snprintf(filename, 512, "%s_audio_g726_16_%dkHz_%d.g726",
               namebase.c_str(), sample_rate, stream_index);
      FILE* tem_file = fopen(filename, "wb+");
      if (AM_LIKELY(tem_file != nullptr)) {
        audio_output_file[stream_index][tem_index] = tem_file;
      } else {
        ERROR("open a file failed");
      }
    } break;
    case AM_EXPORT_PACKET_FORMAT_PCM: {
      snprintf(filename, 512, "%s_audio_%dkHz_%d.pcm",
               namebase.c_str(), sample_rate, stream_index);
      FILE* tem_file = fopen(filename, "wb+");
      if (AM_LIKELY(tem_file != nullptr)) {
        audio_output_file[stream_index][tem_index] = tem_file;
      } else {
        ERROR("open a file failed");
      }
    } break;
    case AM_EXPORT_PACKET_FORMAT_OPUS: {
      snprintf(filename, 512, "%s_audio_%dkHz_%d.opus",
               namebase.c_str(), sample_rate, stream_index);
      FILE* tem_file = fopen(filename, "wb+");
      if (AM_LIKELY(tem_file != nullptr)) {
        audio_output_file[stream_index][tem_index] = tem_file;
      } else {
        ERROR("open a file failed");
      }
    } break;
    case AM_EXPORT_PACKET_FORMAT_BPCM: {
      snprintf(filename, 512, "%s_audio_%dkHz_%d.bpcm",
               namebase.c_str(), sample_rate, stream_index);
      FILE* tem_file = fopen(filename, "wb+");
      if (AM_LIKELY(tem_file != nullptr)) {
        audio_output_file[stream_index][tem_index] = tem_file;
      } else {
        ERROR("open a file failed");
      }
    } break;
    case AM_EXPORT_PACKET_FORMAT_SPEEX: {
      snprintf(filename, 512, "%s_audio_%dkHz_%d.speex",
               namebase.c_str(), sample_rate, stream_index);
      FILE* tem_file = fopen(filename, "wb+");
      if (AM_LIKELY(tem_file != nullptr)) {
        audio_output_file[stream_index][tem_index] = tem_file;
      } else {
        ERROR("open a file failed");
      }
    } break;
    default:
      ERROR("BAD audio format %d", packet_format);
      break;
  }
}

static void write_packet(AMExportPacket* packet)
{
  uint8_t tem_index = (packet->audio_sample_rate)/16 > 1
      ? 2 : (packet->audio_sample_rate)/16;
  switch (packet->packet_type) {
    case AM_EXPORT_PACKET_TYPE_VIDEO_DATA:
      if (video_output_file.find(packet->stream_index) ==
          video_output_file.end()) {
        open_video_output_file(packet->stream_index, packet->packet_format);
      }
      fwrite(packet->data_ptr, 1, packet->data_size,
             video_output_file[packet->stream_index]);
      break;
    case AM_EXPORT_PACKET_TYPE_AUDIO_DATA:
      if (audio_output_file.find(packet->stream_index) ==
          audio_output_file.end() || audio_output_file
          [packet->stream_index].find(tem_index) ==
              audio_output_file[packet->stream_index].end()) {
        open_audio_output_file(packet->stream_index,
                               packet->packet_format,
                               packet->audio_sample_rate);
      }
      fwrite(packet->data_ptr, 1, packet->data_size,
             audio_output_file[packet->stream_index][tem_index]);
      break;
    default:
      NOTICE("discard non-video-audio packet here\n");
      break;
  }
}

int main(int argc, char *argv[])
{
  if (AM_UNLIKELY(2 > argc)) {
    show_usage();
    return (-10);
  }

  int ret = 0;
  AMExportPacket packet;
  AMExportConfig config = {0};
  if ((ret = init_params(argc, argv)) < 0) {
    show_usage();
    return (-1);
  } else if (ret) {
    return ret;
  }

  do {
    if (!(g_client = am_create_export_client(AM_EXPORT_TYPE_UNIX_DOMAIN_SOCKET,
                                             &config))) {
      ERROR("am_create_export_client() failed");
      ret = (-3);
      break;
    }

    signal(SIGINT, __sigstop);
    signal(SIGQUIT, __sigstop);
    signal(SIGTERM, __sigstop);

    if (!g_client->connect_server(DEXPORT_PATH)) {
      ERROR("p_client->connect() failed");
      ret = (-4);
      break;
    }

    NOTICE("read packet loop start");
    while (running_flag) {
      if (g_client->receive(&packet)) {
        switch (packet.packet_type) {
          case AM_EXPORT_PACKET_TYPE_VIDEO_INFO: {
            AMExportVideoInfo *video_info = (AMExportVideoInfo*)packet.data_ptr;
            printf("Video INFO[%d]: "
                "width: %d, height: %d, framerate factor: %d/%d\n",
                packet.stream_index,
                video_info->width, video_info->height,
                video_info->framerate_num, video_info->framerate_den);
          } break;
          case AM_EXPORT_PACKET_TYPE_AUDIO_INFO: {
            AMExportAudioInfo *audio_info = (AMExportAudioInfo*)packet.data_ptr;
            printf("Audio INFO[%d]: "
                "samplerate: %d, frame size: %d, "
                "bitrate: %d, channel: %d, sample size: %d\n",
                packet.stream_index,
                audio_info->samplerate, audio_info->frame_size,
                audio_info->bitrate, audio_info->channels,
                audio_info->sample_size);
          } break;
          case AM_EXPORT_PACKET_TYPE_VIDEO_DATA:
          case AM_EXPORT_PACKET_TYPE_AUDIO_DATA: {
            if (!write_flag) {
              write_packet(&packet);
            } else {
              printf("receive a packet, stream index %d, format %d, size %d\n",
                     packet.stream_index, packet.packet_type, packet.data_size);
            }
          } break;
          default:
            break;
        }
        g_client->release(&packet);
      } else {
        running_flag = false;
        WARN("receive_packet failed, server shut down");
        break;
      }
    }
    NOTICE("read packet loop end");
  } while (0);

  if (g_client) {
    g_client->disconnect_server();
    g_client->destroy();
  }

  for (auto &m : video_output_file) {
    fclose(m.second);
  }

  for (auto &m : audio_output_file) {
    for (auto &m_sub : m.second) {
      fclose(m_sub.second);
    }
  }
  return ret;
}
