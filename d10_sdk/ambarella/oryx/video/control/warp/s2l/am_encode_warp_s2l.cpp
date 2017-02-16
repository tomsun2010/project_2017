/*******************************************************************************
 * am_encode_warp_s2l.cpp
 *
 * History:
 *   Nov 6, 2015 - [zfgong] created file
 *
 * Copyright (c) 2016 Ambarella, Inc.
 *
 * This file and its contents (“Software”) are protected by intellectual
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

#include "am_encode_warp_s2l.h"
#include "am_video_types.h"
#include "am_vin.h"

#include "lib_dewarp_header.h"
#include "lib_dewarp.h"
#include "iav_ioctl.h"

#ifdef __cplusplus
extern "C" {
#endif
AMIEncodePlugin* create_encode_plugin(void *data)
{
  AMPluginData *pdata = ((AMPluginData*)data);
  return AMEncodeWarpS2L::create(pdata->vin, pdata->stream);
}
#ifdef __cplusplus
}
#endif

AMEncodeWarpS2L *AMEncodeWarpS2L::create(AMVin *vin, AMEncodeStream *stream)
{
  AMEncodeWarpS2L *warp = new AMEncodeWarpS2L();
  do {
    if (!warp) {
      ERROR("Failed to create AMEncodeWarpS2L");
      break;
    }
    if (AM_RESULT_OK != warp->init(vin, stream)) {
      delete warp;
      warp = nullptr;
      break;
    }
    if (!(warp->m_config = AMWarpConfigS2L::get_instance())) {
      ERROR("Failed to create AMWarpConfig!");
      break;
    }
    if (AM_RESULT_OK != warp->load_config()) {
      break;
    }

  } while (0);
  return warp;
}

void AMEncodeWarpS2L::destroy()
{
  if (save_config() != AM_RESULT_OK) {
    ERROR("Failed to save warp config");
  }
  inherited::destroy();
}

void* AMEncodeWarpS2L::get_interface()
{
  return ((AMIEncodeWarp*)this);
}

AM_RESULT AMEncodeWarpS2L::set_ldc_strength(int region_id, float strength)
{
  AM_RESULT ret = AM_RESULT_OK;
  do {
    if ((strength > 20.0) || (strength < 0.0)) {
      ERROR("Invalid LDC strength: %f!", strength);
      ret = AM_RESULT_ERR_INVALID;
      break;
    }
    m_config_param.ldc_strength.first = true;
    m_config_param.ldc_strength.second = strength;
  }while(0);

  return ret;
}

AM_RESULT AMEncodeWarpS2L::get_ldc_strength(int region_id, float &strength)
{
  strength = m_config_param.ldc_strength.second;
  return AM_RESULT_OK;
}

AM_RESULT AMEncodeWarpS2L::set_pano_hfov_degree(int region_id, float degree)
{
  AM_RESULT ret = AM_RESULT_OK;

  do {
    if ((degree > 180.0) || (degree < 1.0)) {
      ERROR("Invalid Pano hfov degree: %f!", degree);
      ret = AM_RESULT_ERR_INVALID;
      break;
    }
    m_config_param.pano_hfov_degree.first = true;
    m_config_param.pano_hfov_degree.second = degree;
  }while(0);

  return ret;
}

AM_RESULT AMEncodeWarpS2L::get_pano_hfov_degree(int region_id, float &degree)
{
  degree = m_config_param.pano_hfov_degree.second;
  return AM_RESULT_OK;
}

AM_RESULT AMEncodeWarpS2L::set_warp_region_yaw(int region_id, int yaw)
{
  AM_RESULT ret = AM_RESULT_OK;
  do {
    if ((yaw > 90) || (yaw < -90)) {
      ERROR("Invalid warp region yaw: %d", yaw);
      ret = AM_RESULT_ERR_INVALID;
      break;
    }
    m_config_param.lens.first = true;
    m_config_param.lens.second.yaw = yaw;
  } while(0);

  return ret;
}

AM_RESULT AMEncodeWarpS2L::get_warp_region_yaw(int region_id, int &yaw)
{
  yaw = m_config_param.lens.second.yaw;
  return AM_RESULT_OK;
}

AM_RESULT AMEncodeWarpS2L::apply()
{
  AM_RESULT result = AM_RESULT_OK;

  do {
    AMVinAAAInfo aaa_info;
    AMResolution vin_res;
    AMResolution buf_res;
    AMBufferParamMap buf_param_map;

    warp_region_t lens_warp_region;
    warp_vector_t lens_warp_vector;
    dewarp_init_t param;
    if (!inherited::is_ldc_enable()) {
      ERROR("dewarp only valid when ldc enable");
      result = AM_RESULT_ERR_INVALID;
      break;
    }

    if (m_vin->size_get(vin_res) < 0) {
      ERROR("Failed to get VIN size!");
      result = AM_RESULT_ERR_DSP;
      break;
    }

    if (!m_pixel_width_um) {
      if (m_platform->get_aaa_info(aaa_info) < 0) {
        ERROR("Failed to get AAA info!\n");
        result = AM_RESULT_ERR_INVALID;
        break;
      }
      m_pixel_width_um = (aaa_info.pixel_size >> 16) +
          (1.0 / 0x10000) * (aaa_info.pixel_size & 0xFFFF);
    }
    if (m_buffer_config->get_config(buf_param_map) < 0) {
      ERROR("Failed to get buffer config!");
      result = AM_RESULT_ERR_DSP;
      break;
    }
    AMBufferConfigParam &buf_param = buf_param_map[AM_SOURCE_BUFFER_MAIN];

    memset(&param, 0, sizeof(dewarp_init_t));
    param.projection_mode   = PROJECTION_MODE(m_config_param.proj_mode.second);
    param.max_fov           = m_config_param.ldc_strength.second * 10;
    param.max_input_width   = vin_res.width;
    param.max_input_height  = vin_res.height;
    param.max_radius        = param.max_input_width / 2;
    param.lut_focal_length  = m_config_param.lens.second.efl_mm *
        1000 / m_pixel_width_um;
    param.lut_radius        = nullptr;
    param.main_buffer_width = buf_param.size.second.width;

    /* todo: calculate the center according to DPTZ info */
    if (m_config_param.lens.second.lens_center_in_max_input.x > 0) {
      param.lens_center_in_max_input.x =
          m_config_param.lens.second.lens_center_in_max_input.x;
    } else {
      param.lens_center_in_max_input.x = param.max_input_width / 2;
    }

    if (m_config_param.lens.second.lens_center_in_max_input.y > 0) {
      param.lens_center_in_max_input.y =
          m_config_param.lens.second.lens_center_in_max_input.y;
    } else {
      param.lens_center_in_max_input.y = param.max_input_height / 2;
    }

    if ((m_config_param.proj_mode.second == AM_WARP_PROJECTION_LOOKUP_TABLE) &&
        (!m_config_param.lens.second.lut_file.empty())) {
      FILE *fp = fopen(m_config_param.lens.second.lut_file.c_str(), "r");
      char line[1024] = {0};
      uint32_t i = 0;
      if (!fp) {
        ERROR("Failed to open file: %s for reading!",
              m_config_param.lens.second.lut_file.c_str());
        result = AM_RESULT_ERR_IO;
        break;
      }
      while (fgets(line, sizeof(line), fp) != nullptr) {
        m_distortion_lut[i] = (int)(atof(line) * param.max_input_width);
        ++ i;
      }
      fclose(fp);
      param.lut_radius = m_distortion_lut;
    }

    if (dewarp_init(&param) < 0) {
      ERROR("dewarp_init failed");
      result = AM_RESULT_ERR_INVALID;
      break;
    }
    memset(&lens_warp_region, 0, sizeof(lens_warp_region));
    lens_warp_region.output.width = buf_param.size.second.width;
    lens_warp_region.output.height = buf_param.size.second.height;
    lens_warp_region.output.upper_left.x = 0;
    lens_warp_region.output.upper_left.y = 0;
    lens_warp_region.pitch = m_config_param.lens.second.pitch;
    lens_warp_region.yaw = m_config_param.lens.second.yaw;

    /* Zoom factor */
    /* Fixme: Add lens zoom */
    lens_warp_region.zoom.num = m_config_param.lens.second.zoom.num;
    lens_warp_region.zoom.denom = m_config_param.lens.second.zoom.denom;
    lens_warp_region.hor_zoom = lens_warp_region.zoom;
    lens_warp_region.vert_zoom = lens_warp_region.zoom;

    memset(&lens_warp_vector, 0, sizeof(lens_warp_vector));
    lens_warp_vector.hor_map.addr = (data_t*)m_mem.addr;
    lens_warp_vector.ver_map.addr = (data_t*)(m_mem.addr +
        AM_MAX_WARP_TABLE_SIZE_LDC * sizeof(data_t));
    lens_warp_vector.me1_ver_map.addr = (data_t*)(m_mem.addr +
        AM_MAX_WARP_TABLE_SIZE_LDC * sizeof(data_t) * 2);

    switch (m_config_param.warp_mode.second) {
      case AM_WARP_MODE_RECTLINEAR:
        /* create vector */
        if (lens_wall_normal(&lens_warp_region, &lens_warp_vector) <= 0) {
          ERROR("lens_wall_normal");
          result = AM_RESULT_ERR_INVALID;
        }
        break;
      case AM_WARP_MODE_PANORAMA:
        /* create vector */
        if (lens_wall_panorama(&lens_warp_region,
                               m_config_param.pano_hfov_degree.second,
                               &lens_warp_vector) <= 0) {
          ERROR("lens_wall_panorama");
          result = AM_RESULT_ERR_INVALID;
        }
        break;
      case AM_WARP_MODE_SUBREGION:
      case AM_WARP_MODE_NO_TRANSFORM:
      default:
        ERROR("Warp mode not supported For LDC!\n");
        result = AM_RESULT_ERR_INVALID;
        break;
    }
    if (result != AM_RESULT_OK) {
      break;
    }

    iav_warp_ctrl warp_ctrl;
    iav_warp_area *area = &warp_ctrl.arg.main.area[0];

    do {
      memset(&warp_ctrl, 0, sizeof(struct iav_warp_ctrl));
      warp_ctrl.cid = IAV_WARP_CTRL_MAIN;
      u32 flags = (1 << IAV_WARP_CTRL_MAIN);

      area->enable = 1;
      area->input.width = lens_warp_vector.input.width;
      area->input.height = lens_warp_vector.input.height;
      area->input.x = lens_warp_vector.input.upper_left.x;
      area->input.y = lens_warp_vector.input.upper_left.y;
      area->output.width = lens_warp_vector.output.width;
      area->output.height = lens_warp_vector.output.height;
      area->output.x = lens_warp_vector.output.upper_left.x;
      area->output.y = lens_warp_vector.output.upper_left.y;
      area->rotate_flip = lens_warp_vector.rotate_flip;

      area->h_map.enable = ((lens_warp_vector.hor_map.rows) > 0 &&
                            (lens_warp_vector.hor_map.cols) > 0);
      area->h_map.h_spacing = m_platform->get_grid_spacing(
                              lens_warp_vector.hor_map.grid_width);
      area->h_map.v_spacing = m_platform->get_grid_spacing(
                              lens_warp_vector.hor_map.grid_height);
      area->h_map.output_grid_row = lens_warp_vector.hor_map.rows;
      area->h_map.output_grid_col = lens_warp_vector.hor_map.cols;
      area->h_map.data_addr_offset = 0;

      area->v_map.enable = ((lens_warp_vector.ver_map.rows) > 0 &&
                            (lens_warp_vector.ver_map.cols) > 0);
      area->v_map.h_spacing = m_platform->get_grid_spacing(
                              lens_warp_vector.ver_map.grid_width);
      area->v_map.v_spacing = m_platform->get_grid_spacing(
                              lens_warp_vector.ver_map.grid_height);
      area->v_map.output_grid_row = lens_warp_vector.ver_map.rows;
      area->v_map.output_grid_col = lens_warp_vector.ver_map.cols;
      area->v_map.data_addr_offset = MAX_WARP_TABLE_SIZE_LDC * sizeof(s16);

      // ME1 Warp grid / spacing
      area->me1_v_map.h_spacing = m_platform->get_grid_spacing(
          lens_warp_vector.me1_ver_map.grid_width);
      area->me1_v_map.v_spacing = m_platform->get_grid_spacing(
          lens_warp_vector.me1_ver_map.grid_height);
      area->me1_v_map.output_grid_row = lens_warp_vector.me1_ver_map.rows;
      area->me1_v_map.output_grid_col = lens_warp_vector.me1_ver_map.cols;
      area->me1_v_map.data_addr_offset = MAX_WARP_TABLE_SIZE_LDC *
                                         sizeof(s16) * 2;

      int fd_iav = -1;
      if ((fd_iav = open("/dev/iav", O_RDWR, 0)) < 0) {
        PERROR("/dev/iav");
        result = AM_RESULT_ERR_IO;
        break;
      }
      if (ioctl(fd_iav, IAV_IOC_CFG_WARP_CTRL, &warp_ctrl) < 0) {
        PERROR("IAV_IOC_CFG_WARP_CTRL");
        result = AM_RESULT_ERR_DSP;
        break;
      }
      if (ioctl(fd_iav, IAV_IOC_APPLY_WARP_CTRL, &flags) < 0) {
        PERROR("IAV_IOC_APPLY_WARP_CTRL");
        result = AM_RESULT_ERR_DSP;
        break;
      }
      close(fd_iav);
    } while (0);

    dewarp_deinit();

  } while (0);
  return result;
}

AM_RESULT AMEncodeWarpS2L::load_config()
{
  AM_RESULT result = AM_RESULT_OK;

  do {
    if (!m_config) {
      result = AM_RESULT_ERR_INVALID;
      ERROR("m_config is null!");
      break;
    }
    if ((result = m_config->load_config()) != AM_RESULT_OK) {
      ERROR("Failed to get warp config!");
      break;
    }
    if ((result = m_config->get_config(m_config_param)) != AM_RESULT_OK) {
      ERROR("Failed to get warp config!");
      break;
    }
  } while (0);
  return result;
}

AM_RESULT AMEncodeWarpS2L::save_config()
{
  AM_RESULT result = AM_RESULT_OK;
  do {
    if (!m_config) {
      result = AM_RESULT_ERR_INVALID;
      ERROR("m_config is null!");
      break;
    }
    if ((result = m_config->set_config(m_config_param)) != AM_RESULT_OK) {
      ERROR("Failed to get warp config!");
      break;
    }
    if ((result = m_config->save_config()) != AM_RESULT_OK) {
      ERROR("Failed to set warp config!");
      break;
    }
  } while (0);
  return result;
}

AMEncodeWarpS2L::AMEncodeWarpS2L() :
    inherited("WARP.S2L"),
    m_config(nullptr)
{
}

AMEncodeWarpS2L::~AMEncodeWarpS2L()
{
  m_config = nullptr;
}
