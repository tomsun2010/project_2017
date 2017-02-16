/*
 * kernel/private/drivers/ambarella/vin/sensors/sony_imx290/imx290_table.c
 *
 * History:
 *    2015/03/23 - [Long Zhao] Create
 *
 *
 * Copyright (c) 2015 Ambarella, Inc.
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
 */

static struct vin_video_pll imx290_plls[] = {
	{0, 37125000, 148500000}, /* for 1080p */
	{0, 37125000, 148500000}, /* for 720p */
};

static struct vin_reg_16_8 imx290_pll_regs[][6] = {
	{	/* for 1080p */
		{IMX290_INCKSEL1, 0x18},
		{IMX290_INCKSEL2, 0x00},
		{IMX290_INCKSEL3, 0x20},
		{IMX290_INCKSEL4, 0x01},
		{IMX290_INCKSEL5, 0x1A},
		{IMX290_INCKSEL6, 0x1A},
	},
	{	/* for 720p */
		{IMX290_INCKSEL1, 0x20},
		{IMX290_INCKSEL2, 0x00},
		{IMX290_INCKSEL3, 0x20},
		{IMX290_INCKSEL4, 0x01},
		{IMX290_INCKSEL5, 0x1A},
		{IMX290_INCKSEL6, 0x1A},
	},
};

static struct vin_reg_16_8 imx290_mode_regs[][31] = {
	{	/* linear: 1080p120 10bits */
		{IMX290_ADBIT0,			0x00},
		{IMX290_ADBIT1,			0x1D},
		{IMX290_ADBIT2,			0x12},
		{IMX290_ADBIT3,			0x37},
		{IMX290_WINMODE,		0x00},
		{IMX290_FRSEL,			0x00},
		{IMX290_VMAX_LSB,		0x65},
		{IMX290_VMAX_MSB,		0x04},
		{IMX290_VMAX_HSB,		0x00},
		{IMX290_HMAX_LSB,		0x4C},
		{IMX290_HMAX_MSB,		0x04},
		{IMX290_ODBIT,			0xF0},
		{IMX290_BLKLEVEL_LSB,	0x3C},
		{IMX290_BLKLEVEL_MSB,	0x00},
		/* DOL related */
		{IMX290_WDMODE,		0x00},
		{IMX290_SHS1_LSB,		0x00},
		{IMX290_SHS1_MSB,		0x00},
		{IMX290_SHS1_HSB,		0x00},
		{IMX290_SHS2_LSB,		0x00},
		{IMX290_SHS2_MSB,		0x00},
		{IMX290_SHS2_HSB,		0x00},
		{IMX290_SHS3_LSB,		0x00},
		{IMX290_SHS3_MSB,		0x00},
		{IMX290_SHS3_HSB,		0x00},
		{IMX290_PATTERN,		0x01},
		{IMX290_XVSCNT_INT,		0x00},
		{IMX290_NULL0SIZE,		0x01},
		{IMX290_YOUTSIZE_LSB,	0x49},
		{IMX290_YOUTSIZE_MSB,	0x04},
		{IMX290_HBLANK_LSB,		0xFC},
		{IMX290_HBLANK_MSB,		0x00},
	},
	{	/* linear: 720p120 10bits */
		{IMX290_ADBIT0,			0x00},
		{IMX290_ADBIT1,			0x1D},
		{IMX290_ADBIT2,			0x12},
		{IMX290_ADBIT3,			0x37},
		{IMX290_WINMODE,		0x10},
		{IMX290_FRSEL,			0x00},
		{IMX290_VMAX_LSB,		0xEE},
		{IMX290_VMAX_MSB,		0x02},
		{IMX290_VMAX_HSB,		0x00},
		{IMX290_HMAX_LSB,		0x72},
		{IMX290_HMAX_MSB,		0x06},
		{IMX290_ODBIT,			0xE0},
		{IMX290_BLKLEVEL_LSB,	0x3C},
		{IMX290_BLKLEVEL_MSB,	0x00},
		/* DOL related */
		{IMX290_WDMODE,		0x00},
		{IMX290_SHS1_LSB,		0x00},
		{IMX290_SHS1_MSB,		0x00},
		{IMX290_SHS1_HSB,		0x00},
		{IMX290_SHS2_LSB,		0x00},
		{IMX290_SHS2_MSB,		0x00},
		{IMX290_SHS2_HSB,		0x00},
		{IMX290_SHS3_LSB,		0x00},
		{IMX290_SHS3_MSB,		0x00},
		{IMX290_SHS3_HSB,		0x00},
		{IMX290_PATTERN,		0x01},
		{IMX290_XVSCNT_INT,		0x00},
		{IMX290_NULL0SIZE,		0x01},
		{IMX290_YOUTSIZE_LSB,	0xD9},
		{IMX290_YOUTSIZE_MSB,	0x02},
		{IMX290_HBLANK_LSB,		0xFC},
		{IMX290_HBLANK_MSB,		0x00},
	},
	{	/* 2x DOL: 1080p60 10bits */
		{IMX290_ADBIT0,			0x00},
		{IMX290_ADBIT1,			0x1D},
		{IMX290_ADBIT2,			0x12},
		{IMX290_ADBIT3,			0x37},
		{IMX290_WINMODE,		0x00},
		{IMX290_FRSEL,			0x00},
		{IMX290_VMAX_LSB,		0x65},
		{IMX290_VMAX_MSB,		0x04},
		{IMX290_VMAX_HSB,		0x00},
		{IMX290_HMAX_LSB,		0x4C},
		{IMX290_HMAX_MSB,		0x04},
		{IMX290_ODBIT,			0xF0},
		{IMX290_BLKLEVEL_LSB,	0x3C},
		{IMX290_BLKLEVEL_MSB,	0x00},
		/* DOL related */
		{IMX290_WDMODE,		0x11},
		{IMX290_SHS1_LSB,		0x02},
		{IMX290_SHS1_MSB,		0x00},
		{IMX290_SHS1_HSB,		0x00},
		{IMX290_SHS2_LSB,		0xC9},
		{IMX290_SHS2_MSB,		0x06},
		{IMX290_SHS2_HSB,		0x00},
		{IMX290_SHS3_LSB,		0x00},
		{IMX290_SHS3_MSB,		0x00},
		{IMX290_SHS3_HSB,		0x00},
		{IMX290_PATTERN,		0x00},
		{IMX290_XVSCNT_INT,		0x90},
		{IMX290_NULL0SIZE,		0x00},
		{IMX290_YOUTSIZE_LSB,	0xB2},
		{IMX290_YOUTSIZE_MSB,	0x08},
		{IMX290_HBLANK_LSB,		0xB4},
		{IMX290_HBLANK_MSB,		0x02},
	},
	{	/* 3x DOL: 1080p30 10bits */
		{IMX290_ADBIT0,			0x00},
		{IMX290_ADBIT1,			0x1D},
		{IMX290_ADBIT2,			0x12},
		{IMX290_ADBIT3,			0x37},
		{IMX290_WINMODE,		0x00},
		{IMX290_FRSEL,			0x00},
		{IMX290_VMAX_LSB,		0x65},
		{IMX290_VMAX_MSB,		0x04},
		{IMX290_VMAX_HSB,		0x00},
		{IMX290_HMAX_LSB,		0x4C},
		{IMX290_HMAX_MSB,		0x04},
		{IMX290_ODBIT,			0xF0},
		{IMX290_BLKLEVEL_LSB,	0x3C},
		{IMX290_BLKLEVEL_MSB,	0x00},
		/* DOL related */
		{IMX290_WDMODE,		0x21},
		{IMX290_SHS1_LSB,		0x04},
		{IMX290_SHS1_MSB,		0x00},
		{IMX290_SHS1_HSB,		0x00},
		{IMX290_SHS2_LSB,		0x89},
		{IMX290_SHS2_MSB,		0x00},
		{IMX290_SHS2_HSB,		0x00},
		{IMX290_SHS3_LSB,		0x93},
		{IMX290_SHS3_MSB,		0x01},
		{IMX290_SHS3_HSB,		0x00},
		{IMX290_PATTERN,		0x00},
		{IMX290_XVSCNT_INT,		0xB0},
		{IMX290_NULL0SIZE,		0x00},
		{IMX290_YOUTSIZE_LSB,	0x17},
		{IMX290_YOUTSIZE_MSB,	0x0D},
		{IMX290_HBLANK_LSB,		0xB4},
		{IMX290_HBLANK_MSB,		0x02},
	},
	{	/* 2x DOL: 1080p30 12bits */
		{IMX290_ADBIT0,			0x01},
		{IMX290_ADBIT1,			0x00},
		{IMX290_ADBIT2,			0x00},
		{IMX290_ADBIT3,			0x0E},
		{IMX290_WINMODE,		0x00},
		{IMX290_FRSEL,			0x01},
		{IMX290_VMAX_LSB,		0x65},
		{IMX290_VMAX_MSB,		0x04},
		{IMX290_VMAX_HSB,		0x00},
		{IMX290_HMAX_LSB,		0x98},
		{IMX290_HMAX_MSB,		0x08},
		{IMX290_ODBIT,			0xE1},
		{IMX290_BLKLEVEL_LSB,	0xF0},
		{IMX290_BLKLEVEL_MSB,	0x00},
		/* DOL related */
		{IMX290_WDMODE,		0x11},
		{IMX290_SHS1_LSB,		0x02},
		{IMX290_SHS1_MSB,		0x00},
		{IMX290_SHS1_HSB,		0x00},
		{IMX290_SHS2_LSB,		0xC9},
		{IMX290_SHS2_MSB,		0x07},
		{IMX290_SHS2_HSB,		0x00},
		{IMX290_SHS3_LSB,		0x00},
		{IMX290_SHS3_MSB,		0x00},
		{IMX290_SHS3_HSB,		0x00},
		{IMX290_PATTERN,		0x00},
		{IMX290_XVSCNT_INT,		0x90},
		{IMX290_NULL0SIZE,		0x00},
		{IMX290_YOUTSIZE_LSB,	0xB2},
		{IMX290_YOUTSIZE_MSB,	0x08},
		{IMX290_HBLANK_LSB,		0xFC},
		{IMX290_HBLANK_MSB,		0x00},
	},
};

static struct vin_reg_16_8 imx290_share_regs[] = {
	/* chip ID: 02h */
	{0x300F, 0x00},
	{0x3010, 0x21},
	{0x3012, 0x64},
	{0x3016, 0x08}, /* set to 0x08 to fix flare issue for switching HCG/LCG mode */
	{0x3070, 0x02},
	{0x3071, 0x11},
	{0x309B, 0x10},
	{0x309C, 0x22},
	{0x30A2, 0x02},
	{0x30A6, 0x20},
	{0x30A8, 0x20},
	{0x30AA, 0x20},
	{0x30AC, 0x20},
	{0x30B0, 0x43},

	/* chip ID: 03h */
	{0x3119, 0x9E},
	{0x311C, 0x1E},
	{0x311E, 0x08},
	{0x3128, 0x05},
	{0x313D, 0x83},
	{0x3150, 0x03},
	{0x317E, 0x00},

	/* chip ID: 04h */
	{0x32B8, 0x50},
	{0x32B9, 0x10},
	{0x32BA, 0x00},
	{0x32BB, 0x04},
	{0x32C8, 0x50},
	{0x32C9, 0x10},
	{0x32CA, 0x00},
	{0x32CB, 0x04},

	/* chip ID: 05h */
	{0x332C, 0xD3},
	{0x332D, 0x10},
	{0x332E, 0x0D},
	{0x3358, 0x06},
	{0x3359, 0xE1},
	{0x335A, 0x11},
	{0x3360, 0x1E},
	{0x3361, 0x61},
	{0x3362, 0x10},
	{0x33B0, 0x50},
	{0x33B2, 0x1A},
	{0x33B3, 0x04},
};

#ifdef CONFIG_PM
static struct vin_reg_16_8 pm_regs[] = {
	{IMX290_VMAX_HSB, 0x00},
	{IMX290_VMAX_MSB, 0x00},
	{IMX290_VMAX_LSB, 0x00},
	{IMX290_SHS1_HSB, 0x00},
	{IMX290_SHS1_MSB, 0x00},
	{IMX290_SHS1_LSB, 0x00},
	{IMX290_SHS2_HSB, 0x00},
	{IMX290_SHS2_MSB, 0x00},
	{IMX290_SHS2_LSB, 0x00},
	{IMX290_SHS3_HSB, 0x00},
	{IMX290_SHS3_MSB, 0x00},
	{IMX290_SHS3_LSB, 0x00},
	{IMX290_GAIN, 0x00},
};
#endif

static struct vin_video_format imx290_formats[] = {
	{
		.video_mode	= AMBA_VIDEO_MODE_1080P,
		.def_start_x	= 4+8,
		.def_start_y	= 1+2+10+8,
		.def_width	= 1920,
		.def_height	= 1080,
		/* sensor mode */
		.device_mode	= 0,
		.pll_idx	= 0,
		.width		= 1920,
		.height		= 1080,
		.format		= AMBA_VIDEO_FORMAT_PROGRESSIVE,
		.type		= AMBA_VIDEO_TYPE_RGB_RAW,
		.bits		= AMBA_VIDEO_BITS_10,
		.ratio		= AMBA_VIDEO_RATIO_16_9,
		.max_fps	= AMBA_VIDEO_FPS_120,
		.default_fps	= AMBA_VIDEO_FPS_29_97,
		.default_agc	= 0,
		.default_shutter_time	= AMBA_VIDEO_FPS_60,
		.default_bayer_pattern	= VINDEV_BAYER_PATTERN_RG,
	},
	{
		.video_mode	= AMBA_VIDEO_MODE_720P,
		.def_start_x	= 4+8,
		.def_start_y	= 1+2+4+4,
		.def_width	= 1280,
		.def_height	= 720,
		/* sensor mode */
		.device_mode	= 1,
		.pll_idx	= 1,
		.width		= 1280,
		.height		= 720,
		.format		= AMBA_VIDEO_FORMAT_PROGRESSIVE,
		.type		= AMBA_VIDEO_TYPE_RGB_RAW,
		.bits		= AMBA_VIDEO_BITS_10,
		.ratio		= AMBA_VIDEO_RATIO_16_9,
		.max_fps	= AMBA_VIDEO_FPS_120,
		.default_fps	= AMBA_VIDEO_FPS_29_97,
		.default_agc	= 0,
		.default_shutter_time	= AMBA_VIDEO_FPS_60,
		.default_bayer_pattern	= VINDEV_BAYER_PATTERN_RG,
	},
	{
		.video_mode	= AMBA_VIDEO_MODE_1080P,
		.def_start_x	= 4+8,
		.def_start_y	= (1+2+10+8)*2,
		.def_width	= 1920,
		.def_height	= 1080 * 2 + (IMX290_1080P_2X_RHS1 - 1), /* (1080 + VBP1)*2 */
		.act_start_x	= 0,
		.act_start_y	= 0,
		.act_width	= 1920,
		.act_height	= 1080,
		.max_act_width = 1920,
		.max_act_height = IMX290_1080P_BRL,
		/* sensor mode */
		.hdr_mode = AMBA_VIDEO_2X_HDR_MODE,
		.device_mode	= 2,
		.pll_idx	= 0,
		.width		= IMX290_1080P_H_PIXEL*2+IMX290_1080P_HBLANK,
		.height		= 1080,
		.format		= AMBA_VIDEO_FORMAT_PROGRESSIVE,
		.type		= AMBA_VIDEO_TYPE_RGB_RAW,
		.bits		= AMBA_VIDEO_BITS_10,
		.ratio		= AMBA_VIDEO_RATIO_16_9,
#ifdef USE_1080P_2X_30FPS
		.max_fps	= AMBA_VIDEO_FPS_30,
#else
		.max_fps	= AMBA_VIDEO_FPS_60,
#endif
		.default_fps	= AMBA_VIDEO_FPS_29_97,
		.default_agc	= 0,
		.default_shutter_time	= AMBA_VIDEO_FPS_60,
		.default_bayer_pattern	= VINDEV_BAYER_PATTERN_RG,
		.hdr_long_offset = 0,
		.hdr_short1_offset = (IMX290_1080P_2X_RHS1 - 1) + 1, /* hdr_long_offset + 2 x VBP1 + 1 */
	},
	{
		.video_mode	= AMBA_VIDEO_MODE_1080P,
		.def_start_x	= 4+8,
		.def_start_y	= (1+2+10+8)*3,
		.def_width	= 1920,
		.def_height	= 1080 * 3 + (IMX290_1080P_3X_RHS2 - 2), /* (1080 + VBP2)*3 */
		.act_start_x	= 0,
		.act_start_y	= 0,
		.act_width	= 1920,
		.act_height	= 1080,
		.max_act_width = 1920,
		.max_act_height = IMX290_1080P_BRL,
		/* sensor mode */
		.hdr_mode = AMBA_VIDEO_3X_HDR_MODE,
		.device_mode	= 3,
		.pll_idx	= 0,
		.width		= IMX290_1080P_H_PIXEL*3+IMX290_1080P_HBLANK*2,
		.height		= 1080,
		.format		= AMBA_VIDEO_FORMAT_PROGRESSIVE,
		.type		= AMBA_VIDEO_TYPE_RGB_RAW,
		.bits		= AMBA_VIDEO_BITS_10,
		.ratio		= AMBA_VIDEO_RATIO_16_9,
		.max_fps	= AMBA_VIDEO_FPS_30,
		.default_fps	= AMBA_VIDEO_FPS_29_97,
		.default_agc	= 0,
		.default_shutter_time	= AMBA_VIDEO_FPS_60,
		.default_bayer_pattern	= VINDEV_BAYER_PATTERN_RG,
		.hdr_long_offset = 0,
		.hdr_short1_offset = (IMX290_1080P_3X_RHS1 - 1) + 1, /* hdr_long_offset + 3 x VBP1 + 1 */
		.hdr_short2_offset = (IMX290_1080P_3X_RHS2 - 2) + 2, /* hdr_long_offset + 3 x VBP2 + 2 */
	},
	{
		.video_mode	= AMBA_VIDEO_MODE_1080P,
		.def_start_x	= 4+8,
		.def_start_y	= (1+2+10+8)*2,
		.def_width	= 1920,
		.def_height	= 1080 * 2 + (IMX290_1080P_2X_12B_RHS1 - 1), /* (1080 + VBP1)*2 */
		.act_start_x	= 0,
		.act_start_y	= 0,
		.act_width	= 1920,
		.act_height	= 1080,
		.max_act_width = 1920,
		.max_act_height = IMX290_1080P_BRL,
		/* sensor mode */
		.hdr_mode = AMBA_VIDEO_2X_HDR_MODE,
		.device_mode	= 4,
		.pll_idx	= 0,
		.width		= IMX290_1080P_H_PIXEL*2+IMX290_1080P_HBLANK,
		.height		= 1080,
		.format		= AMBA_VIDEO_FORMAT_PROGRESSIVE,
		.type		= AMBA_VIDEO_TYPE_RGB_RAW,
		.bits		= AMBA_VIDEO_BITS_12,
		.ratio		= AMBA_VIDEO_RATIO_16_9,
		.max_fps	= AMBA_VIDEO_FPS_30,
		.default_fps	= AMBA_VIDEO_FPS_29_97,
		.default_agc	= 0,
		.default_shutter_time	= AMBA_VIDEO_FPS_60,
		.default_bayer_pattern	= VINDEV_BAYER_PATTERN_RG,
		.hdr_long_offset = 0,
		.hdr_short1_offset = (IMX290_1080P_2X_12B_RHS1 - 1) + 1, /* hdr_long_offset + 2 x VBP1 + 1 */
	},
};

#define IMX290_GAIN_MAX_DB  	240 /* 72dB */

