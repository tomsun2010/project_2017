/*
 * image data
 *
 * History:
 *    Author: Lu Wang <lwang@ambarella.com>
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
 */

#include "img_adv_struct_arch.h"

img_aeb_tile_config_t ov2718_aeb_tile_config ={
        .header = {
                AEB_TILE_CONFIG,
                1,
                {1, 0, 0, 0},
         },
        .tile_config ={
                1,
                1,

                32,
                32,
                0,
                0,
                128,
                128,
                128,
                128,
                0,
                0x3fff,

                12,
                8,
                0,
                0,
                340,
                512,

                12,
                8,
                128,
                8,
                320,
                510,
                320,
                510,

                0,
                16383,
        },
};

img_aeb_expo_lines_t ov2718_aeb_expo_lines={
        .header = {
                AEB_EXPO_LINES,
                1,				// total ae table number
                {4, 0, 0, 0},		// ae lines per ae table
        },

        .expo_tables[0] ={{
                {
                {{SHUTTER_1BY32000, ISO_100, 0}}, {{SHUTTER_1BY100, ISO_100,0}}
                },

                {
                {{SHUTTER_1BY100, ISO_100, 0}}, {{SHUTTER_1BY100, ISO_300, 0}}
                },

                {
                {{SHUTTER_1BY50, ISO_100, 0}}, {{SHUTTER_1BY50, ISO_800,0}}
                },

                {
                {{SHUTTER_1BY30, ISO_100, 0}}, {{SHUTTER_1BY30, ISO_102400,0}}
                }
        }},
 };

img_aeb_expo_lines_t ov2718_aeb_expo_lines_m13vp288ir ={
        .header = {
                AEB_EXPO_LINES_PIRIS_LINEAR1,
		  		1,					// total ae table number
                {8, 0, 0, 0},		// ae lines per ae table
        },

        .expo_tables[0] ={{
				{
				{{SHUTTER_1BY32000, ISO_100, APERTURE_F11}}, {{SHUTTER_1BY1024, ISO_100,APERTURE_F11}}
				},

				{
				{{SHUTTER_1BY1024, ISO_100, APERTURE_F11}}, {{SHUTTER_1BY1024, ISO_100, APERTURE_F5P6}}
				},

				{
				{{SHUTTER_1BY1024, ISO_100, APERTURE_F5P6}}, {{SHUTTER_1BY100, ISO_100, APERTURE_F5P6}}
				},

				{
				{{SHUTTER_1BY100, ISO_100, APERTURE_F5P6}}, {{SHUTTER_1BY100, ISO_100, APERTURE_F2P8}}
				},

				{
				{{SHUTTER_1BY100, ISO_100, APERTURE_F2P8}}, {{SHUTTER_1BY100, ISO_300, APERTURE_F2P8}}
				},

				{
				{{SHUTTER_1BY50, ISO_100, APERTURE_F2P8}}, {{SHUTTER_1BY50, ISO_100, APERTURE_F1P2}}
				},

				{
				{{SHUTTER_1BY50, ISO_100, APERTURE_F1P2}}, {{SHUTTER_1BY50, ISO_800,APERTURE_F1P2}}
				},

				{
				{{SHUTTER_1BY30, ISO_100, APERTURE_F1P2}}, {{SHUTTER_1BY30, ISO_51200,APERTURE_F1P2}}
				}
        }},
};

img_aeb_expo_lines_t ov2718_aeb_expo_lines_mz128bp2810icr ={
        .header = {
                AEB_EXPO_LINES_PIRIS_LINEAR2,
		  		1,					// total ae table number
                {8, 0, 0, 0},		// ae lines per ae table
        },

        .expo_tables[0] ={{
				{
				{{SHUTTER_1BY32000, ISO_100, APERTURE_F11}}, {{SHUTTER_1BY1024, ISO_100,APERTURE_F11}}
				},

				{
				{{SHUTTER_1BY1024, ISO_100, APERTURE_F11}}, {{SHUTTER_1BY1024, ISO_100, APERTURE_F5P6}}
				},

				{
				{{SHUTTER_1BY1024, ISO_100, APERTURE_F5P6}}, {{SHUTTER_1BY100, ISO_100, APERTURE_F5P6}}
				},

				{
				{{SHUTTER_1BY100, ISO_100, APERTURE_F5P6}}, {{SHUTTER_1BY100, ISO_100, APERTURE_F2P8}}
				},

				{
				{{SHUTTER_1BY100, ISO_100, APERTURE_F2P8}}, {{SHUTTER_1BY100, ISO_300, APERTURE_F2P8}}
				},

				{
				{{SHUTTER_1BY50, ISO_100, APERTURE_F2P8}}, {{SHUTTER_1BY50, ISO_100, APERTURE_F1P6}}
				},

				{
				{{SHUTTER_1BY50, ISO_100, APERTURE_F1P6}}, {{SHUTTER_1BY50, ISO_800,APERTURE_F1P6}}
				},

				{
				{{SHUTTER_1BY30, ISO_100, APERTURE_F1P6}}, {{SHUTTER_1BY30, ISO_51200,APERTURE_F1P6}}
				}
        }},
};


img_aeb_wb_param_t ov2718_aeb_wb_param ={
         .header = {
                AEB_WB_PARAM,
                1,
                {1, 0, 0, 0},
          },
          .wb_param ={
         {
                {2216, 1024, 1944},	//AUTOMATIC
                {1421, 1024, 3358},	//INCANDESCENT
                {2034, 1024, 2792},	//D4000
                {2216, 1024, 1944},	//D5000
                {2584, 1024, 1760},	//SUNNY
                {2978, 1024, 1618},	//CLOUDY
                {1750, 1024, 1400},	//FLASH
                {1150, 1024, 1900},	//FLUORESCENT
                {1575, 1024, 1350},	//FLUORESCENT_H
                {1600, 1024, 1380},	//UNDER WATER
                {1375, 1024, 1600},	//CUSTOM
                {2584, 1024, 1760},	//AUTOMATIC OUTDOOR
        },
        {
        	12,
        	{
        	{1014,1848,2744,3821,-1699,4696,-1469,6171,1100,1225,866,2745,1},	// 0	INCANDESCENT
        	{1216,2348,2084,3244,-1543,4484,-1268,5839,579,1121,955,1611,2},	// 1	D4000
        	{1639,2543,1514,2397,-1085,3790,-1231,5172,999,-656,768,914,4},	// 2	D5000
        	{2230,2994,1341,2119,-315,2189,-420,3165,1064,-1407,1187,-806,8},	// 3	SUNNY
        	{2682,3362,1327,1917,-274,2083,-236,2654,1338,-2779,1137,-1275,4},	// 4	CLOUDY
        	{2300, 2850, 1750, 2250, -1000, 4050, -1000, 4900,1000, -1000, 1000,  -80, 0 },	// 5	PROJECTOR
        	{1700,2200,2200,2800,-1600,5100,-1900,6900,2000,-1800,1400,260, 0},	// 6	GREEN REGION
        	{   0,    0,    0,    0,     0,    0,     0,    0,   0,   0,    0,    0, 0 },	// 7	FLASH
        	{   0,    0,    0,    0,     0,    0,     0,    0,   0,   0,    0,    0, 0 },	// 8	FLUORESCENT
        	{   0,    0,    0,    0,     0,    0,     0,    0,   0,   0,    0,    0, 0 },	// 9	FLUORESCENT_2
        	{   0,    0,    0,    0,     0,    0,     0,    0,   0,   0,    0,    0, 0 },	// 10 FLUORESCENT_3
        	{   0,    0,    0,    0,     0,    0,     0,    0,   0,   0,    0,    0, 0 }	// 11 CUSTOM
        	},
        },
        {	{ 0 ,6},	//LUT num. AUTOMATIC  INDOOR
        	{ 0, 1},	//LUT num. INCANDESCENT
        	{ 1, 1},	//LUT num. D4000
        	{ 2, 1},	//LUT num. D5000
        	{ 2, 5},	//LUT num. SUNNY
        	{ 4, 3},	//LUT num. CLOUDY
        	{ 7, 1},	//LUT num. FLASH
        	{ 8, 1},	//LUT num. FLUORESCENT
        	{ 9, 1},	//LUT num. FLUORESCENT_H
        	{11, 1},	//LUT num. UNDER WATER
        	{11, 1},	//LUT num. CUSTOM
        	{ 0, 7},	//LUT num. AUTOMATIC  OUTDOOR
         }
    },
};
img_aeb_sensor_config_t ov2718_aeb_sensor_config ={
	.header = {
		AEB_SENSOR_CONFIG,
		1,
		{1, 0, 0, 0},
	},
	.sensor_config  ={
			66,//max_gain_db
			46, // max global gain db
			0, // max single gain db
			2,//shutter_delay
			2,//agc_delay

		{	// hdr delay, useless for linear mode
			{{0, 0, 0, 0}},	// shutter
			{{0, 0, 0, 0}},	// agc
			{{1, 1, 1, 1}},	// dgain
			{{0, 0, 0, 0}},	// sensor offset
		},
		{	// hdr raw offset coef, non negative value is valid
			-1,		// long padding
			-1,		// short padding
		},
	},
};

img_aeb_sht_nl_table_t ov2718_aeb_sht_nl_table ={
        .header = {
                AEB_SHT_NL_TABLE,
                1,
                {32, 0, 0, 0},
        },
        .sht_nl_table ={
                0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
        },
};
img_aeb_gain_table_t ov2718_aeb_gain_table ={
        .header = {
                AEB_GAIN_TABLE,
                1,
                {641, 0, 0, 0},
        },
        .gain_table ={//66.09375db,step =0.09375db
	4096,4140,4185,4231,4277,4323,4370,4417,4465,4514,4563,4612,4662,4713,4764,4816,4868,4921,
	4974,5028,5083,5138,5194,5250,5307,5365,5423,5482,5541,5601,5662,5724,5786,5849,5912,5976,
	6041,6107,6173,6240,6308,6376,6445,6515,6586,6657,6730,6803,6876,6951,7026,7103,7180,7258,
	7336,7416,7497,7578,7660,7743,7827,7912,7998,8085,8173,8261,8351,8442,8533,8626,8719,8814,
	8910,9006,9104,9203,9303,9404,9506,9609,9713,9819,9925,10033,10142,10252,10363,10475,10589,
	10704,10820,10938,11056,11176,11298,11420,11544,11669,11796,11924,12053,12184,12316,12450,
	12585,12722,12860,12999,13140,13283,13427,13573,13720,13869,14020,14172,14326,14481,14638,
	14797,14958,15120,15284,15450,15617,15787,15958,16131,16306,16483,16662,16843,17026,17211,
	17397,17586,17777,17970,18165,18362,18561,18763,18966,19172,19380,19591,19803,20018,20235,
	20455,20677,20901,21128,21357,21589,21823,22060,22300,22542,22786,23034,23283,23536,23792,
	24050,24311,24575,24841,25111,25383,25659,25937,26219,26503,26791,27081,27375,27672,27973,
	28276,28583,28893,29207,29524,29844,30168,30495,30826,31161,31499,31841,32186,32536,32889,
	33246,33606,33971,34340,34712,35089,35470,35855,36244,36637,37035,37437,37843,38254,38669,
	39088,39513,39941,40375,40813,41256,41703,42156,42614,43076,43543,44016,44494,44976,45465,
	45958,46457,46961,47470,47985,48506,49033,49565,50103,50646,51196,51751,52313,52881,53455,
	54035,54621,55214,55813,56419,57031,57650,58275,58908,59547,60193,60846,61507,62174,62849,
	63531,64220,64917,65622,66334,67054,67781,68517,69260,70012,70772,71540,72316,73101,73894,
	74696,75507,76326,77154,77992,78838,79693,80558,81432,82316,83209,84112,85025,85948,86881,
	87823,88776,89740,90714,91698,92693,93699,94716,95744,96783,97833,98895,99968,101053,102149,
	103258,104378,105511,106656,107813,108983,110166,111361,112570,113792,115026,116275,117536,
	118812,120101,121405,122722,124054,125400,126761,128136,129527,130932,132353,133790,135241,
	136709,138193,139692,141208,142741,144290,145855,147438,149038,150655,152290,153943,155614,
	157302,159009,160735,162479,164242,166025,167826,169647,171488,173349,175231,177132,179054,
	180997,182962,184947,186954,188983,191034,193107,195202,197321,199462,201626,203814,206026,
	208262,210522,212807,215116,217450,219810,222195,224607,227044,229508,231998,234516,237061,
	239633,242234,244863,247520,250206,252921,255666,258440,261245,264080,266945,269842,272771,
	275731,278723,281747,284805,287896,291020,294178,297370,300597,303859,307157,310490,313859,
	317265,320708,324188,327706,331263,334857,338491,342164,345878,349631,353425,357260,361137,
	365056,369018,373022,377070,381162,385299,389480,393706,397979,402298,406663,411076,415537,
	420047,424605,429213,433870,438579,443338,448149,453012,457928,462898,467921,472999,478132,
	483320,488565,493867,499226,504644,510120,515656,521252,526908,532626,538406,544249,550155,
	556125,562160,568260,574427,580661,586962,593332,599770,606279,612858,619509,626232,633027,
	639897,646841,653860,660956,668128,675379,682708,690117,697606,705176,712828,720564,728383,
	736288,744278,752354,760519,768772,777114,785547,794072,802689,811400,820205,829106,838103,
	847198,856392,865685,875079,884576,894175,903878,913687,923602,933625,943756,953998,964351,
	974816,985394,996087,1006897,1017823,1028869,1040034,1051320,1062729,1074261,1085919,1097703,
	1109615,1121657,1133829,1146133,1158570,1171143,1183852,1196699,1209685,1222813,1236082,1249496,
	1263056,1276762,1290617,1304623,1318780,1333091,1347558,1362181,1376964,1391906,1407011,1422279,
	1437714,1453316,1469087,1485029,1501144,1517434,1533901,1550547,1567373,1584382,1601576,1618956,
	1636524,1654284,1672236,1690382,1708726,1727269,1746013,1764960,1784113,1803474,1823045,1842829,
	1862827,1883042,1903476,1924132,1945013,1966120,1987456,2009023,2030825,2052863,2075140,2097659,
	2120423,2143433,2166693,2190206,2213974,2237999,2262286,2286836,2311652,2336738,2362095,2387729,
	2413640,2439832,2466309,2493073,2520127,2547475,2575120,2603065,2631313,2659867,2688732,2717909,
	2747404,2777218,2807356,2837821,2868616,2899746,2931213,2963022,2995177,3027680,3060536,3093748,
	3127321,3161258,3195563,3230241,3265295,3300730,3336549,3372756,3409357,3446355,3483754,3521559,
	3559774,3598404,3637453,3676926,3716828,3757162,3797934,3839149,3880811,3922924,3965495,4008528,
	4052028,4096000,
    },
};

img_aeb_digit_wdr_param_t ov2718_aeb_digit_wdr_config = {
	.header = {
		AEB_DIGIT_WDR,
		1,
		{1, 0, 0, 0},
	},
	.digit_wdr_config = {
		0, 	// enable
		{	// strength
		0,	// 0db
		0,	// 0db
		0,	// 0db
		0,	// 6db
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		},
	},
};


