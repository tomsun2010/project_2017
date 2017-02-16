/*#############################################################################
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

/*###########################################################################*/

#ifndef SONYSD_H
#define SONYSD_H


#define SONYSD_SUCCESS 0
#define SONYSD_ERR_OTHER -1
#define SONYSD_ERR_NOCARD -2
#define SONYSD_ERR_UNSUPPORT -3


typedef struct _sonysd_info {
	unsigned long long life_information_num;
	unsigned long long life_information_den;
	unsigned long data_size_per_unit;
	unsigned long spare_block_rate;
	unsigned long num_of_sudden_power_failure;
	unsigned long operation_mode;
} sonysd_info;


typedef struct _sonysd_cmd {
	unsigned short cmd;
	unsigned long addr;
	unsigned long resp;
} sonysd_cmd;


typedef struct _sonysd_err {
	unsigned short error_no;
	unsigned char error_type;
	unsigned char block_type;
	unsigned char logical_CE;
	unsigned char physical_plane;
	unsigned short physical_block;
	unsigned short physical_page;
	unsigned short error_type_of_timeout;
	unsigned long erase_block_count;
	unsigned short write_page_error_count;
	unsigned short read_page_error_count;
	unsigned short erase_block_error_count;
	unsigned short timeout_count;
	unsigned short crc_error_count;
	sonysd_cmd command[6];
} sonysd_err;


typedef struct _sonysd_err_log {
	int count;
	sonysd_err items[64];
} sonysd_errlog;


#ifdef __cplusplus
extern "C" {
#endif

int sonysd_get_info(sonysd_info *info);
int sonysd_get_errlog(sonysd_errlog *log);
int sonysd_set_operation_mode(unsigned long on_off);

void sonysd_set_devpath(const char* path);
void sonysd_debug_print_info(sonysd_info *info);
void sonysd_debug_print_errlog(sonysd_errlog *log);

#ifdef __cplusplus
}
#endif

#endif



