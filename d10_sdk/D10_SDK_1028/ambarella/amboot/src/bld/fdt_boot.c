/**
 * bld/fdt_boot.c
 *
 * History:
 *    2013/08/15 - [Cao Rongrong] created file
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

#include <bldfunc.h>
#include <libfdt.h>
#include <ambhw/cortex.h>

static int isnum(char s)
{
	if (s <= '9' && s >= '0')
		return 1;
	else
		return 0;
}

static unsigned int strtonum(char *str)
{
	char *s;
	int len = 0;
	unsigned int num = 0;

	while (*str == ' ')
		str++;

	s = str;

	while (isnum(*str))
		str++;

	len = str - s;

	if ((*str != 'M' && *str != 'm')
			|| !len)
		return 0;

	while (len--){
		num = num * 10 + (*s - '0');
		s++;
	}

	return num * 1024 * 1024;

}

static int fdt_cmdline_mem(const char *cmdline)
{
	void *fdt;
	int rval;
	int offset;
	int len = 0;
	unsigned int size = 0;

	char *str = NULL;
	char *s = "mem=";
	char *path = "/chosen";

	fdt = (void *)bld_hugebuf_addr;

	rval = flprog_get_dtb((void *)fdt);
	if (rval < 0) {
		putstr("Get dtb failed\r\n");
		return size;
	}

	offset = fdt_path_offset(fdt, path);
	if (offset < 0) {
		putstr("libfdt fdt_path_offset() error: ");
		putstr(fdt_strerror(offset));
		putstr("\r\n");
		return size;
	}

	len = fdt_size_dt_strings(fdt) - offset;
	if (len < 0)
		return size;

	if (cmdline) {

		str = strstr(cmdline, s, strlen(cmdline));
		if (str)
			size = strtonum(str + strlen(s));

		if(size)
			return size;
	}

	str = strstr((char *)fdt + offset, s, len);
	if (!str)
		return size;

	size = strtonum(str + strlen(s));
	return size;
}

void fdt_print_error(const char *str, int err)
{
	putstr(str);
	putstr(fdt_strerror(err));
	putstr("\r\n");
}

static int fdt_update_chosen(void *fdt, const char *cmdline, u32 cpux_jump,
	u32 initrd2_start, u32 initrd2_size)
{
	int ret_val = 0;
	int offset;

	offset = fdt_path_offset (fdt, "/chosen");
	if (offset < 0) {
		ret_val = offset;
		goto fdt_update_chosen_exit;
	}

	if ((cmdline != NULL) && (cmdline[0] != '\0')) {
		ret_val = fdt_setprop_string(fdt, offset, "bootargs", cmdline);
		if (ret_val < 0) {
			goto fdt_update_chosen_exit;
		}
	}

	if (cpux_jump) {
		ret_val = fdt_setprop_u32(fdt, offset,
			"ambarella,cpux_jump", cpux_jump);
		if (ret_val < 0) {
			goto fdt_update_chosen_exit;
		}
	}

	if ((initrd2_start != 0x0) && (initrd2_size != 0x0)) {
		ret_val = fdt_setprop_u32(fdt, offset, "linux,initrd-start",
				ARM11_TO_CORTEX(initrd2_start));
		if (ret_val < 0) {
			goto fdt_update_chosen_exit;
		}
		ret_val = fdt_setprop_u32(fdt, offset, "linux,initrd-end",
				ARM11_TO_CORTEX(initrd2_start) + initrd2_size);
		if (ret_val < 0) {
			goto fdt_update_chosen_exit;
		}
	}

fdt_update_chosen_exit:
	return ret_val;
}

#if defined(CONFIG_AMBOOT_ENABLE_NAND) || \
	 defined(CONFIG_AMBOOT_ENABLE_SPINOR) || defined(CONFIG_AMBOOT_ENABLE_SPINAND)
static void __hex_to_str(const u32 hex, char *dest)
{
	char i, c;

	for (i = 0; i < 32; i += 4) {
		c = (hex >> (28 - i)) & 0xf;
		if (c >= 10)
			dest[i/4] = 'a' + c - 10;
		else
			dest[i/4] = '0' + c;
	}

	dest[8] = '\0';
}
#endif

#if defined(CONFIG_AMBOOT_ENABLE_NAND)
static int fdt_update_nand(void *fdt)
{
	int i, ret_val = -1;
	const char *pathp;
	int offset, suboffset;
	u32 val[6];

#ifdef AMBA_SDK_XIAOYI_ALTER  
    
    flpart_table_t ptb = {};

    ret_val = flprog_get_part_table(&ptb);
    if (ret_val < 0)
        return ret_val;
    int bak_sel = ptb.dev.bak_sel;
    putstr("  xxxx bak_sel = ");
    puthex(bak_sel);
    putstr("   ...\r\n");
#endif
    

	pathp = fdt_get_alias(fdt, "nand");
	if (pathp == NULL) {
		putstr("libfdt fdt_get_alias() failed\r\n");
		return ret_val;
	}

	offset = fdt_path_offset (fdt, pathp);
	if (offset < 0) {
		fdt_print_error("fdt_path_offset(nand) error:", offset);
		return ret_val;
	}

	val[0] = cpu_to_fdt32(flnand.nandtiming0);
	val[1] = cpu_to_fdt32(flnand.nandtiming1);
	val[2] = cpu_to_fdt32(flnand.nandtiming2);
	val[3] = cpu_to_fdt32(flnand.nandtiming3);
	val[4] = cpu_to_fdt32(flnand.nandtiming4);
	val[5] = cpu_to_fdt32(flnand.nandtiming5);
	ret_val = fdt_setprop(fdt, offset, "amb,timing", &val, sizeof(u32) * 6);
	if (ret_val < 0) {
		fdt_print_error("fdt_setprop(amb,timing) error:", ret_val);
		return ret_val;
	}

	for (i = HAS_IMG_PARTS - 1; i >= 0; i--) {
		char name[32], addr_str[16];
		u32 addr, size;

		if (flnand.nblk[i] == 0)
			continue;

		addr = flnand.sblk[i] * flnand.block_size;
		size = flnand.nblk[i] * flnand.block_size;

		strcpy(name, "partition@");
		__hex_to_str(addr, addr_str);
		strcat(name, addr_str);

		suboffset = fdt_add_subnode(fdt, offset, name);
		if (suboffset < 0) {
			fdt_print_error("fdt_add_subnode(partition) error:",
						ret_val);
			return ret_val;
		}

		ret_val = fdt_setprop_string(fdt, suboffset,
					"label", get_part_str(i));
		if (ret_val < 0) {
			fdt_print_error("fdt_setprop_string(label) error:",
						ret_val);
			return ret_val;
		}

		val[0] = cpu_to_fdt32(addr);
		val[1] = cpu_to_fdt32(size);
		ret_val = fdt_setprop(fdt, suboffset, "reg",
						&val, (sizeof(u32) * 2));
		if (ret_val < 0){
			fdt_print_error("fdt_setprop(reg) error:",
						ret_val);
			return ret_val;
		}

#ifdef AMBA_SDK_XIAOYI_ALTERxx  

        if(i<=PART_BLD)
        {
            ret_val = fdt_setprop(fdt, suboffset, "read-only", NULL, 0);
        
            if (ret_val < 0)
            {
                fdt_print_error("fdt_setprop(read-only) error:",ret_val);
                return ret_val;
            }
        }
#if 0
        else if( i==PART_BAK )
        {
            if( ptb.dev.bak_sel == 0)
            {
                putstr("use bak part, ro\r\n");
                ret_val = fdt_setprop(fdt, suboffset, "read-only", NULL, 0);
                if (ret_val < 0)
                {
                    fdt_print_error("fdt_setprop(read-only) error:",ret_val);
                    return ret_val;
                }
            }
            else
            {
                putstr("bak part rw\r\n");
            }
        }
        else if( i==PART_RMD)
        {
            if( ptb.dev.bak_sel == 1)
            {
                putstr("use rmd part, ro\r\n");
                ret_val = fdt_setprop(fdt, suboffset, "read-only", NULL, 0);
                if (ret_val < 0)
                {
                    fdt_print_error("fdt_setprop(read-only) error:",ret_val);
                    return ret_val;
                }
            }
            else
            {
                putstr("rmd part rw\r\n");
        
            }           
        }
#endif
        else if(i == PART_RMD && bak_sel == 0x01)
        {
            ret_val = fdt_setprop(fdt, suboffset, "read-only", NULL, 0);
        
            if (ret_val < 0)
            {
                fdt_print_error("fdt_setprop(read-only) error:",ret_val);
                return ret_val;
            }           
        }
        else if(i == PART_LNX && bak_sel == 0x00)
        {
            ret_val = fdt_setprop(fdt, suboffset, "read-only", NULL, 0);
        
            if (ret_val < 0)
            {
                fdt_print_error("fdt_setprop(read-only) error:",ret_val);
                return ret_val;
            }           
        }
#endif


        
	}

	return ret_val;
}
#endif

#if defined(CONFIG_AMBOOT_ENABLE_SPINOR)
static int fdt_update_spinor(void *fdt)
{
	int i, ret_val = -1;
	const char *pathp;
	int offset, suboffset;
	u32 val[6];

	pathp = fdt_get_alias(fdt, "spinor");
	if (pathp == NULL) {
		putstr("libfdt fdt_get_alias() failed\r\n");
		return ret_val;
	}

	offset = fdt_path_offset (fdt, pathp);
	if (offset < 0) {
		fdt_print_error("fdt_path_offset(spi) error:", offset);
		return ret_val;
	}

	for (i = HAS_IMG_PARTS - 1; i >= 0; i--) {
		char name[32], addr_str[16];
		u32 addr, size;

		if (flspinor.nsec[i] == 0)
			continue;

		addr = flspinor.ssec[i] * flspinor.sector_size;
		size = flspinor.nsec[i] * flspinor.sector_size;

		putstr("flspinor addr = 0x");
		puthex(addr);
		putstr(", size = 0x");
		puthex(size);
		putstr("\r\n");

		strcpy(name, "partition@");
		__hex_to_str(addr, addr_str);
		strcat(name, addr_str);

		suboffset = fdt_add_subnode(fdt, offset, name);
		if (suboffset < 0) {
			fdt_print_error("fdt_add_subnode(partition) error:",
						ret_val);
			return ret_val;
		}

		ret_val = fdt_setprop_string(fdt, suboffset,
					"label", get_part_str(i));
		if (ret_val < 0) {
			fdt_print_error("fdt_setprop_string(label) error:",
						ret_val);
			return ret_val;
		}

		val[0] = cpu_to_fdt32(addr);
		val[1] = cpu_to_fdt32(size);
		ret_val = fdt_setprop(fdt, suboffset, "reg",
						&val, (sizeof(u32) * 2));
		if (ret_val < 0){
			fdt_print_error("fdt_setprop(reg) error:",
						ret_val);
			return ret_val;
		}
	}

	return ret_val;
}
#endif

#if defined(CONFIG_AMBOOT_ENABLE_SPINAND)
static int fdt_update_spinand(void *fdt)
{
	int i, ret_val = -1;
	const char *pathp;
	int offset, suboffset;
	u32 val[6];

	pathp = fdt_get_alias(fdt, "spinand");
	if (pathp == NULL) {
		putstr("libfdt fdt_get_alias() failed\r\n");
		return ret_val;
	}

	offset = fdt_path_offset (fdt, pathp);
	if (offset < 0) {
		fdt_print_error("fdt_path_offset(spinand) error:", offset);
		return ret_val;
	}

	for (i = HAS_IMG_PARTS - 1; i >= 0; i--) {
		char name[32], addr_str[16];
		u32 addr, size;

		if (flspinand.nblk[i] == 0)
			continue;

		addr = flspinand.sblk[i] * flspinand.block_size;
		size = flspinand.nblk[i] * flspinand.block_size;

		putstr("flspinand addr = 0x");
		puthex(addr);
		putstr(", size = 0x");
		puthex(size);
		putstr("\r\n");

		strcpy(name, "partition@");
		__hex_to_str(addr, addr_str);
		strcat(name, addr_str);

		suboffset = fdt_add_subnode(fdt, offset, name);
		if (suboffset < 0) {
			fdt_print_error("fdt_add_subnode(partition) error:",
						ret_val);
			return ret_val;
		}

		ret_val = fdt_setprop_string(fdt, suboffset,
					"label", get_part_str(i));
		if (ret_val < 0) {
			fdt_print_error("fdt_setprop_string(label) error:",
						ret_val);
			return ret_val;
		}

		val[0] = cpu_to_fdt32(addr);
		val[1] = cpu_to_fdt32(size);
		ret_val = fdt_setprop(fdt, suboffset, "reg",
						&val, (sizeof(u32) * 2));
		if (ret_val < 0){
			fdt_print_error("fdt_setprop(reg) error:",
						ret_val);
			return ret_val;
		}
	}

	return ret_val;
}
#endif

#if defined(CONFIG_BOOT_MEDIA_EMMC)
static int fdt_update_emmc(void *fdt)
{
	int ret_val = -1, offset, suboffset;
	const char *pathp;
	u32 val, ptb_address;

	pathp = fdt_get_alias(fdt, "emmc");
	if (pathp == NULL) {
		putstr("libfdt fdt_get_alias() failed\r\n");
		return ret_val;
	}

	offset = fdt_path_offset (fdt, pathp);
	if (offset < 0) {
		fdt_print_error("fdt_path_offset(emmc) error:", offset);
		return ret_val;
	}

	ptb_address = sdmmc.ssec[PART_PTB];
	val = cpu_to_fdt32(ptb_address);
	ret_val = fdt_setprop(fdt, offset, "amb,ptb_address",
				&val, sizeof(u32));
	if (ret_val < 0){
		fdt_print_error("fdt_setprop(ptb_address) error:",
					ret_val);
		return ret_val;
	}

	suboffset = fdt_subnode_offset(fdt, offset, "slot@0");
	fdt_delprop(fdt, suboffset, "amb,caps-ddr");

	return ret_val;
}

#endif

static int fdt_update_memory(void *fdt,
	u32 kernel_start, u32 kernel_size,
	u32 iav_start, u32 iav_size,
	u32 frame_buf_start, u32 frame_buf_size)
{
	int ret_val = -1;
	int offset;
	u32 val[2];

	if (fdt == NULL)
		goto fdt_update_memory_exit;

	offset = fdt_node_offset_by_prop_value(fdt, -1,
			"device_type", "memory", 7);
	if (offset < 0) {
		ret_val = offset;
		goto fdt_update_memory_exit;
	}

	val[0] = cpu_to_fdt32(kernel_start);
	val[1] = cpu_to_fdt32(kernel_size);
	ret_val = fdt_setprop(fdt, offset, "reg", &val, (sizeof(u32) * 2));
	if (ret_val < 0)
		goto fdt_update_memory_exit;

	/* create a new node "/iavmem" (offset 0 is root level) */
	offset = fdt_add_subnode(fdt, 0, "iavmem");
	if (offset < 0)
		goto fdt_update_memory_exit;

	ret_val = fdt_setprop_string(fdt, offset, "device_type", "iavmem");
	if (ret_val < 0)
		goto fdt_update_memory_exit;

	val[0] = cpu_to_fdt32(iav_start);
	val[1] = cpu_to_fdt32(iav_size);
	ret_val = fdt_setprop(fdt, offset, "reg", &val, (sizeof(u32) * 2));
	if (ret_val < 0)
		goto fdt_update_memory_exit;

	/* create a new node "/fbmem" (offset 0 is root level) */
	offset = fdt_add_subnode(fdt, 0, "fbmem");
	if (offset < 0)
		goto fdt_update_memory_exit;

	ret_val = fdt_setprop_string(fdt, offset, "device_type", "fbmem");
	if (ret_val < 0)
		goto fdt_update_memory_exit;

	val[0] = cpu_to_fdt32(frame_buf_start);
	val[1] = cpu_to_fdt32(frame_buf_size);
	ret_val = fdt_setprop(fdt, offset, "reg", &val, (sizeof(u32) * 2));
	if (ret_val < 0)
		goto fdt_update_memory_exit;

fdt_update_memory_exit:
	return ret_val;
}

static int fdt_update_misc(void *fdt)
{
	int ret_val;

	if (fdt == NULL)
		return -1;

#if defined(CONFIG_AMBOOT_ENABLE_NAND)
	ret_val = fdt_update_nand((void *)fdt);
	if (ret_val < 0) {
		fdt_print_error("fdt_update_nand:", ret_val);
		return ret_val;
	}
#endif

#if defined(CONFIG_AMBOOT_ENABLE_SPINOR)
	ret_val = fdt_update_spinor((void *)fdt);
	if (ret_val < 0) {
		fdt_print_error("fdt_update_spinor:", ret_val);
		return ret_val;
	}
#endif

#if defined(CONFIG_AMBOOT_ENABLE_SPINAND)
	ret_val = fdt_update_spinand((void *)fdt);
	if (ret_val < 0) {
		fdt_print_error("fdt_update_spinand:", ret_val);
		return ret_val;
	}
#endif

#if defined(CONFIG_BOOT_MEDIA_EMMC)
	ret_val = fdt_update_emmc((void *)fdt);
	if (ret_val < 0) {
		fdt_print_error("fdt_update_emmc:", ret_val);
		return ret_val;
	}
#endif

#if defined(CONFIG_AMBOOT_ENABLE_ETH)
    flpart_table_t ptb;
    const char *pathp;
    int offset;
    
	ret_val = flprog_get_part_table(&ptb);
	if (ret_val < 0) {
		putstr("fdt_update_misc: PTB load error!\r\n");
		return ret_val;
	}

	if (ptb.dev.eth[0].mac != 0) {
		pathp = fdt_get_alias(fdt, "ethernet0");

		offset = fdt_path_offset (fdt, pathp);
		if (offset < 0) {
			fdt_print_error("fdt_path_offset(mac) error:", offset);
			return ret_val;
		}

		ret_val = fdt_setprop(fdt, offset, "local-mac-address",
				ptb.dev.eth[0].mac, 6);
		if (ret_val < 0) {
			fdt_print_error("fdt_setprop(mac) error:", ret_val);
			return ret_val;
		}
	}

	if ((u32)ptb.dev.wifi[0].mac[0] != 0) {
		putstr("NOT implemented yet\r\n");
	}
#endif

	return 0;
}

u32 fdt_update_tags(void *jump_addr, const char *cmdline, u32 cpux_jump,
	u32 initrd2_start, u32 initrd2_size, int verbose)
{
	u32 mem_base, mem_size, dtb_addr, cmdline_mem_size;
	u32 kernelp, kernels;
	u32 idspp, idsps;
	u32 fbp, fbs;
	int ret_val;

	mem_base = (((u32)jump_addr) & (~SIZE_1MB_MASK));
	mem_size = (IDSP_RAM_START - mem_base - FRAMEBUFFER_SIZE);
	K_ASSERT(mem_base >= DRAM_START_ADDR);
	K_ASSERT(mem_size <= DRAM_SIZE);

#ifdef CONFIG_AMBOOT_FDT_LOW_ADDR
	dtb_addr = mem_base;
#else
	cmdline_mem_size = fdt_cmdline_mem(cmdline);
	if (cmdline_mem_size)
		mem_size = cmdline_mem_size;
	dtb_addr = mem_base + mem_size - SIZE_1MB * 2;
#endif
	ret_val = flprog_get_dtb((u8 *)dtb_addr);
	if (ret_val < 0) {
		putstr("Get dtb failed\r\n");
		return ret_val;
	}

	ret_val = fdt_update_chosen((void *)dtb_addr, cmdline,
				cpux_jump, initrd2_start, initrd2_size);
	if (ret_val < 0) {
		if (verbose)
			fdt_print_error("fdt_update_chosen:", ret_val);
		goto fdt_update_tags_exit;
	}
	if (verbose) {
		if (cmdline) {
			putstr("cmdline: ");
			putstr(cmdline);
			putstr("\r\n");
		}

		putstr("cpux_jump: 0x");
		puthex(cpux_jump);
		putstr("\r\n");

		putstr("initrd2_start: 0x");
		puthex(initrd2_start);
		putstr(" initrd2_size: 0x");
		puthex(initrd2_size);
		putstr("\r\n");
	}

	kernelp = ARM11_TO_CORTEX(mem_base);
	kernels = mem_size;
	idspp = ARM11_TO_CORTEX(IDSP_RAM_START);
	idsps = (DRAM_SIZE - (IDSP_RAM_START - DRAM_START_ADDR));
        fbp = ARM11_TO_CORTEX(IDSP_RAM_START - FRAMEBUFFER_SIZE);
        fbs = FRAMEBUFFER_SIZE;
	if (verbose) {
		putstr("kernelp: 0x");
		puthex(kernelp);
		putstr(" kernels: 0x");
		puthex(kernels);
		putstr("\r\n");

		putstr("idspp: 0x");
		puthex(idspp);
		putstr(" idsps: 0x");
		puthex(idsps);
		putstr("\r\n");

		putstr("fbp: 0x");
		puthex(fbp);
		putstr(" fbs: 0x");
		puthex(fbs);
		putstr("\r\n");
	}
	ret_val = fdt_update_memory((void *)dtb_addr,
				kernelp, kernels, idspp, idsps, fbp, fbs);
	if (ret_val < 0) {
		if (verbose)
			fdt_print_error("fdt_update_memory:", ret_val);
		goto fdt_update_tags_exit;
	}

	ret_val = fdt_update_misc((void *)dtb_addr);

fdt_update_tags_exit:
	return ARM11_TO_CORTEX(dtb_addr);
}

int fdt_update_cmdline(void *fdt, const char *cmdline)
{
	return fdt_update_chosen(fdt, cmdline, 0, 0, 0);
}

