/*******************************************************************************
 * am_muxer_AV3_config.cpp
 *
 * History:
 *   2016-08-30 - [ccjing] created file
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
#include "am_base_include.h"
#include "am_define.h"
#include "am_log.h"

#include "am_configure.h"
#include "am_muxer_AV3_config.h"
#include "am_muxer_codec_info.h"

AMMuxerAV3Config::AMMuxerAV3Config() :
    m_config(NULL),
    m_muxer_AV3_config(NULL)
{
}

AMMuxerAV3Config::~AMMuxerAV3Config()
{
  delete m_config;
  delete m_muxer_AV3_config;
}

bool AMMuxerAV3Config::set_config(AMMuxerCodecAV3Config* config)
{
  bool ret = true;
  do{
    if(AM_UNLIKELY(config == NULL)) {
      ERROR("Input config struct is NULL.");
      ret = false;
      break;
    }
    AMConfig &config_AV3 = *m_config;
    /* file_duration */
    if(AM_LIKELY(config_AV3["file_duration"].exists())) {
      config_AV3["file_duration"] = config->file_duration;
    }else {
      NOTICE("\"file_duration\" is not exist.");
    }
    /*smallest_free_space*/
    if(AM_LIKELY(config_AV3["smallest_free_space"].exists())) {
      config_AV3["smallest_free_space"] = config->smallest_free_space;
    }else {
      NOTICE("\"smallest_free_space\" is not exist.");
    }
    /*file_name_prefix*/
    if(AM_LIKELY(config_AV3["file_name_prefix"].exists())) {
      config_AV3["file_name_prefix"] = config->file_name_prefix;
    }else{
      NOTICE("\"file_name_prefix\" is not exist.");
    }
    /*max_file_num_per_folder*/
    if(AM_LIKELY(config_AV3["max_file_num_per_folder"].exists())) {
      config_AV3["max_file_num_per_folder"] = config->max_file_num_per_folder;
    } else {
      NOTICE("\"max_file_num_per_folder\" is not exist.");
    }
    /*recording_file_num*/
    if(AM_LIKELY(config_AV3["recording_file_num"].exists())) {
      config_AV3["recording_file_num"] = config->recording_file_num;
    } else {
      NOTICE("\"recording_file_num\" is not exist.");
    }
    /*recording_duration*/
    if(AM_LIKELY(config_AV3["recording_duration"].exists())) {
      config_AV3["recording_duration"] = config->recording_duration;
    } else {
      NOTICE("\"recording_duration\" is not exist.");
    }
    /*write_index_to_nand_frequency*/
    if (AM_LIKELY(config_AV3["write_index_to_nand_frequency"].exists())) {
      config_AV3["write_index_to_nand_frequency"] =
          config->write_index_to_nand_frequency;
    } else {
      NOTICE("\"write_index_to_nand_frequency\" is not exist.");
    }
    /*file_location*/
    if(AM_LIKELY(config_AV3["file_location"].exists())) {
      config_AV3["file_location"] = config->file_location;
    } else {
      NOTICE("\"file_location\" is not exist.");
    }
    /*file_location_auto_parse*/
    if(AM_LIKELY(config_AV3["file_location_auto_parse"].exists())) {
      config_AV3["file_location_auto_parse"] = config->file_location_auto_parse;
    } else {
      NOTICE("\"file_location_auto_parse\" is not exist");
    }
    /*need_sync*/
    if(AM_LIKELY(config_AV3["need_sync"].exists())) {
      config_AV3["need_sync"] = config->need_sync;
    } else {
      NOTICE("\"need_sync\" is not exist.");
    }
    /*AV3_reconstruct_flag*/
    if (AM_LIKELY(config_AV3["AV3_reconstruct_flag"].exists())) {
      config_AV3["AV3_reconstruct_flag"] = config->AV3_reconstruct_flag;
    } else {
      NOTICE("\"AV3_reconstruct_flag\" is not exist.");
    }
    /*auto_file_writing*/
    if(AM_LIKELY(config_AV3["auto_file_writing"].exists())) {
      config_AV3["auto_file_writing"] = config->auto_file_writing;
    } else {
      NOTICE("\"auto_file_writing\" is not exist.");
    }
    /*event id*/
    if (AM_LIKELY(config_AV3["event_id"].exists())) {
      config_AV3["event_id"] = config->event_id;
    } else {
      NOTICE("\"event_id\" is not exist.");
    }
    /*video id*/
    if (AM_LIKELY(config_AV3["video_id"].exists())) {
      config_AV3["video_id"] = config->video_id;
    } else {
      NOTICE("\"video_id\" is not exist.");
    }
    /*audio_sample_rate*/
    if (AM_LIKELY(config_AV3["audio_sample_rate"].exists())) {
      config_AV3["audio_sample_rate"] = config->audio_sample_rate;
    } else {
      NOTICE("\"audio_sample_rate\" is not exist.");
    }
    /*muxer id*/
    if (AM_LIKELY(config_AV3["muxer_id"].exists())) {
      config_AV3["muxer_id"] = config->muxer_id;
    } else {
      NOTICE("\"muxer_id\" is not exist.");
    }
    /*audio_type*/
    if (AM_LIKELY(config_AV3["audio_type"].exists())) {
      switch (config->audio_type) {
        case AM_AUDIO_AAC : {
          config_AV3["audio_type"] = std::string("aac");
        } break;
        case AM_AUDIO_NULL : {
          config_AV3["audio_type"] = std::string("none");
        } break;
        default : {
          WARN("The audio type %u is not supported, set none as default.");
          config_AV3["audio_type"] = std::string("none");
        }
      }
    }
    /*file type*/
    if(AM_LIKELY(config_AV3["muxer_attr"].exists())) {
      if(config->muxer_attr == AM_MUXER_FILE_NORMAL) {
        config_AV3["muxer_attr"] = std::string("normal");
      } else if(config->muxer_attr == AM_MUXER_FILE_EVENT) {
        config_AV3["muxer_attr"] = std::string("event");
      } else {
        WARN("File type error, set normal as default.");
        config_AV3["muxer_attr"] = std::string("normal");
      }
    }
    if(AM_UNLIKELY(!config_AV3.save())) {
      ERROR("Failed to save config information.");
      ret = false;
      break;
    }
  } while(0);

  return ret;
}

AMMuxerCodecAV3Config* AMMuxerAV3Config::get_config(const std::string& config_file)
{
  AMMuxerCodecAV3Config *config = NULL;
  do {
    if (AM_LIKELY(NULL == m_muxer_AV3_config)) {
      m_muxer_AV3_config = new AMMuxerCodecAV3Config();
    }
    if (AM_UNLIKELY(!m_muxer_AV3_config)) {
      ERROR("Failed to create Am_muxer_AV3_config struct.");
      break;
    }
    delete m_config;
    m_config = AMConfig::create(config_file.c_str());
    if (AM_LIKELY(m_config)) {
      AMConfig &config_AV3 = *m_config;
      /*file type*/
      if (AM_LIKELY(config_AV3["muxer_attr"].exists())) {
        std::string muxer_attr =
            config_AV3["muxer_attr"].get<std::string>("normal");
        if (muxer_attr == "normal") {
          m_muxer_AV3_config->muxer_attr = AM_MUXER_FILE_NORMAL;
        } else if (muxer_attr == "event") {
          m_muxer_AV3_config->muxer_attr = AM_MUXER_FILE_EVENT;
        } else {
          NOTICE("muxer_attr is error, use default.");
          m_muxer_AV3_config->muxer_attr = AM_MUXER_FILE_NORMAL;
        }
      } else {
        NOTICE("file type is not specified, use default!");
        m_muxer_AV3_config->muxer_attr = AM_MUXER_FILE_NORMAL;
      }
      /* file_duration */
      if (AM_LIKELY(config_AV3["file_duration"].exists())) {
        m_muxer_AV3_config->file_duration =
            config_AV3["file_duration"].get<signed>(300);
      } else {
        NOTICE("\"file_duration\" is not specified, use default!");
        m_muxer_AV3_config->file_duration = 300;
      }
      /*smallest_free_space*/
      if (AM_LIKELY(config_AV3["smallest_free_space"].exists())) {
        m_muxer_AV3_config->smallest_free_space =
            config_AV3["smallest_free_space"].get<unsigned>(20);
      } else {
        NOTICE("\"smallest_free_space\" is not specified, use default!");
        m_muxer_AV3_config->smallest_free_space = 20;
      }
      /*file_name_prefix*/
      if (AM_LIKELY(config_AV3["file_name_prefix"].exists())) {
        m_muxer_AV3_config->file_name_prefix =
            config_AV3["file_name_prefix"].get<std::string>("S2L");
      } else {
        NOTICE("\"file_name_prefix\" is not specified, use default!");
        m_muxer_AV3_config->file_name_prefix = "S2L";
      }
      /*max_file_num_per_folder*/
      if (AM_LIKELY(config_AV3["max_file_num_per_folder"].exists())) {
        m_muxer_AV3_config->max_file_num_per_folder =
            config_AV3["max_file_num_per_folder"].get<unsigned>(512);
      } else {
        NOTICE("\"max_file_num_per_folder\" is not specified, use 512 as default!");
        m_muxer_AV3_config->max_file_num_per_folder = 512;
      }
      /*recording_file_num*/
      if (AM_LIKELY(config_AV3["recording_file_num"].exists())) {
        m_muxer_AV3_config->recording_file_num =
            config_AV3["recording_file_num"].get<unsigned>(0);
      } else {
        NOTICE("\"recording_file_num\" is not specified, use 0 as default!");
        m_muxer_AV3_config->recording_file_num = 0;
      }
      /*recording_duration*/
      if (AM_LIKELY(config_AV3["recording_duration"].exists())) {
        m_muxer_AV3_config->recording_duration =
            config_AV3["recording_duration"].get<unsigned>(0);
      } else {
        NOTICE("\"recording_duration\" is not specified, use 0 as default!");
        m_muxer_AV3_config->recording_duration = 0;
      }
      /*file_location*/
      if (AM_LIKELY(config_AV3["file_location"].exists())) {
        m_muxer_AV3_config->file_location =
            config_AV3["file_location"].get<std::string>("/storage/sda1/video");
      } else {
        NOTICE("\"file_location\" is not specified, use default!");
        m_muxer_AV3_config->file_location = "/storage/sda1/video";
      }
      /*key_file_location*/
      if (AM_LIKELY(config_AV3["key_file_location"].exists())) {
        m_muxer_AV3_config->key_file_location =
            config_AV3["key_file_location"].get<std::string>(
                "/etc/oryx/stream/muxer/private.pem");
      } else {
        NOTICE("\"key file_location\" is not specified, use default!");
        m_muxer_AV3_config->key_file_location =
            "/etc/oryx/stream/muxer/private.pem";
      }
      /*file_location_auto_parse*/
      if(AM_LIKELY(config_AV3["file_location_auto_parse"].exists())) {
        m_muxer_AV3_config->file_location_auto_parse =
            config_AV3["file_location_auto_parse"].get<bool>(true);
      } else {
        NOTICE("\"file_location_auto_parse\" is not specified, use default!");
        m_muxer_AV3_config->file_location_auto_parse = true;
      }
      /*need_sync*/
      if(AM_LIKELY(config_AV3["need_sync"].exists())) {
        m_muxer_AV3_config->need_sync =
            config_AV3["need_sync"].get<bool>(false);
      } else {
        NOTICE("\"need_sync\" is not specified, use default.");
        m_muxer_AV3_config->need_sync = false;
      }
      /*AV3_reconstruct_flag*/
      if (AM_LIKELY(config_AV3["AV3_reconstruct_flag"].exists())) {
        m_muxer_AV3_config->AV3_reconstruct_flag =
            config_AV3["AV3_reconstruct_flag"].get<bool>(false);
      } else {
        NOTICE("\"AV3_reconstruct_flag\" is not specified, use default.");
        m_muxer_AV3_config->AV3_reconstruct_flag = false;
      }
      /*auto_file_writing*/
      if(AM_LIKELY(config_AV3["auto_file_writing"].exists())) {
        m_muxer_AV3_config->auto_file_writing =
            config_AV3["auto_file_writing"].get<bool>(true);
      } else {
        NOTICE("\"auto_file_writing\" is not specified, use default.");
        m_muxer_AV3_config->auto_file_writing = true;
      }
      /* event_id */
      if (AM_LIKELY(config_AV3["event_id"].exists())) {
        m_muxer_AV3_config->event_id =
            config_AV3["event_id"].get<unsigned>(0);
      } else {
        NOTICE("\"event_id\" is not specified, use default!");
        m_muxer_AV3_config->event_id = 0;
      }
      /* video_id */
      if (AM_LIKELY(config_AV3["video_id"].exists())) {
        m_muxer_AV3_config->video_id =
            config_AV3["video_id"].get<unsigned>(0);
      } else {
        NOTICE("\"video_id\" is not specified, use default!");
        m_muxer_AV3_config->video_id = 0;
      }
      /* muxer_id */
      if (AM_LIKELY(config_AV3["muxer_id"].exists())) {
        m_muxer_AV3_config->muxer_id = config_AV3["muxer_id"].get<unsigned>(0);
      } else {
        NOTICE("\"muxer_id\" is not specified, use default!");
        m_muxer_AV3_config->muxer_id = 0;
      }
      /* write_index_to_nand_frequency */
      if (AM_LIKELY(config_AV3["write_index_to_nand_frequency"].exists())) {
        m_muxer_AV3_config->write_index_to_nand_frequency =
            config_AV3["write_index_to_nand_frequency"].get<unsigned>(0);
      } else {
        NOTICE("\"write_index_to_nand_frequency\" is not specified, use default!");
        m_muxer_AV3_config->write_index_to_nand_frequency = 0;
      }
      /* max file size */
      if (AM_LIKELY(config_AV3["max_file_size"].exists())) {
        m_muxer_AV3_config->max_file_size =
            config_AV3["max_file_size"].get<unsigned>(1024);
      } else {
        NOTICE("\"max_file_size\" is not specified, use default!");
        m_muxer_AV3_config->max_file_size = 1024;
      }
      /*audio_sample_rate*/
      if (AM_LIKELY(config_AV3["audio_sample_rate"].exists())) {
        m_muxer_AV3_config->audio_sample_rate =
            config_AV3["audio_sample_rate"].get<unsigned>(0);
      } else {
        NOTICE("\"audio_sample_rate\" is not specified, use 0(auto) as default!");
        m_muxer_AV3_config->audio_sample_rate = 0;
      }
      /*audio_type*/
      if (AM_LIKELY(config_AV3["audio_type"].exists())) {
        std::string audio = config_AV3["audio_type"].get<std::string>("aac");
        if(audio == "aac") {
          m_muxer_AV3_config->audio_type = AM_AUDIO_AAC;
        } else if (audio == "none") {
          m_muxer_AV3_config->audio_type = AM_AUDIO_NULL;
        } else {
          m_muxer_AV3_config->audio_type = AM_AUDIO_NULL;
          WARN("audio type %s is not supported, set NULL as default.",
               audio.c_str());
        }
      } else {
        NOTICE("\"audio_type\" is not specified, use default!");
        m_muxer_AV3_config->audio_type = AM_AUDIO_NULL;
      }
    } else {
      ERROR("Failed to create AMConfig object.");
      break;
    }
    config = m_muxer_AV3_config;

  } while (0);

  return config;
}
