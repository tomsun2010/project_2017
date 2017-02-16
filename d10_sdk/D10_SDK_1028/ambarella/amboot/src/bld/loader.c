/**
 * bld/loader.c
 *
 * History:
 *    2005/03/08 - [Charles Chiou] created file
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
#include <ambhw/nand.h>
#include <ambhw/spinor.h>
#include <ambhw/cortex.h>
#include <eth/network.h>
#include <sdmmc.h>

#if defined(SECURE_BOOT)
#include "secure/secure_boot.h"
#endif


#ifdef AMBA_SDK_XIAOYI_ALTER  
#define UBIFS_SEL_LNX 0
#define UBIFS_SEL_RMD 1

#endif


/*===========================================================================*/
void bld_loader_display_part(const char *s, const flpart_t *part)
{
	if ((part->img_len == 0) || (part->img_len == 0xffffffff))
		return;

	if (part->magic != FLPART_MAGIC) {
		putstr(s);
		putstr(": partition appears damaged...\r\n");
		return;
	}

	putstr(s);
	putstr(": 0x");
	puthex(part->crc32);
	putstr(" ");
	putdec(part->ver_num >> 16);
	putstr(".");
	putdec(part->ver_num & 0xffff);
	putstr(" \t(");
	if ((part->ver_date >> 16) == 0)
		putstr("0000");
	else
		putdec(part->ver_date >> 16);
	putstr("/");
	putdec((part->ver_date >> 8) & 0xff);
	putstr("/");
	putdec(part->ver_date & 0xff);
	putstr(")\t0x");
	puthex(part->mem_addr);
	putstr(" 0x");
	puthex(part->flag);
	putstr(" (");
	putdec(part->img_len);
	putstr(")\r\n");
}

void bld_loader_display_ptb_content(const flpart_table_t *ptb)
{
	int i;

	for (i = 0; i < HAS_IMG_PARTS; i++) {
		bld_loader_display_part(get_part_str(i), &ptb->part[i]);
	}
	putstr("\r\n");
}

/*===========================================================================*/
#if defined(CONFIG_AMBOOT_ENABLE_NAND)
static int bld_loader_load_partition_nand(int part_id,
	u32 mem_addr, u32 img_len, u32 flag)
{
	int ret_val = -1;

	ret_val = nand_read_data((u8 *)mem_addr,
		(u8 *)(flnand.sblk[part_id] * flnand.block_size), img_len);
	if (ret_val == img_len) {
		ret_val = 0;
	} else {
		ret_val = -1;
	}

	return ret_val;
}
#endif

#if defined(CONFIG_AMBOOT_ENABLE_SD)
static int bld_loader_load_partition_sm(int part_id,
	u32 mem_addr, u32 img_len, u32 flag)
{
	u32 sectors = (img_len + sdmmc.sector_size - 1) / sdmmc.sector_size;

	return sdmmc_read_sector(sdmmc.ssec[part_id],
			sectors, (u8 *)mem_addr);
}
#endif

#if defined(CONFIG_AMBOOT_ENABLE_SPINOR)
static int bld_loader_load_partition_spinor(int part_id,
	uintptr_t mem_addr, u32 img_len, u32 flag)
{
	u32 address;
	int ret_val;

	address = flspinor.ssec[part_id] * flspinor.sector_size;

	ret_val = spinor_read_data(address, (void *)mem_addr, img_len);
	if (ret_val < 0)
		return ret_val;

	return 0;
}

#endif

#if defined(CONFIG_AMBOOT_ENABLE_SPINAND)
static int bld_loader_load_partition_spinand(int part_id,
	uintptr_t mem_addr, u32 img_len, u32 flag)
{
	int ret_val;

	ret_val = spinand_read_data((u8 *)mem_addr,
		(u8 *)(flspinand.sblk[part_id] * flspinand.block_size), img_len);

	if (ret_val < 0)
		return ret_val;

	return 0;
}

#endif

#ifdef AMBA_SDK_XIAOYI_ALTER  

static int bld_loader_validate_image(u8 *image, unsigned int len,u32 crc32_ptb)
{
	u32 raw_crc32 = 0;

	K_ASSERT(image != NULL);
	K_ASSERT(len > sizeof(partimg_header_t));

	putstr("zwz verifying image crc ... ");
	raw_crc32 = crc32((u8 *)(image),len);
	if (raw_crc32 != crc32_ptb) {
		putstr("0x");
		puthex(raw_crc32);
		putstr(" != 0x");
		puthex(crc32_ptb);
		putstr(" failed!\r\n");
		return -1;
	} else {
	    putstr("0x");
		puthex(raw_crc32);
		putstr(" == 0x");
		puthex(crc32_ptb);
		putstr("  zwz done\r\n");
        return 0;       
	}

	;
}


int bld_loader_validate_partition(int part_id,
	const flpart_table_t *pptb, int verbose)
{
	int ret_val = -1;
	const char *ppart_name;
	const flpart_t *pptb_part;
	u32 mem_addr = 0x0;
	u32 img_len = 0x0;
	u32 boot_from;

	if ((part_id >= HAS_IMG_PARTS) || (part_id == PART_PTB)) {
		if (verbose) {
			putstr("zwz bld_loader_validate_partition illegal partition ID.\r\n");
		}
		goto bld_loader_load_partition_exit;
	}
	ppart_name = get_part_str(part_id);
	pptb_part = &pptb->part[part_id];

	if (pptb_part->magic != FLPART_MAGIC) {
		if (verbose) {
			putstr(ppart_name);
			putstr("zwz bld_loader_validate_partition partition appears damaged... skipping\r\n");
		}
		goto bld_loader_load_partition_exit;
	}
	if ((pptb_part->mem_addr < DRAM_START_ADDR) ||
		(pptb_part->mem_addr > (DRAM_START_ADDR + DRAM_SIZE - 1))){
		if (verbose) {
			putstr(ppart_name);
			putstr("zwz bld_loader_validate_partition wrong load address... skipping\r\n");
		}
		goto bld_loader_load_partition_exit;
	}
	if ((pptb_part->img_len == 0) || (pptb_part->img_len == 0xffffffff)) {
		if (verbose) {
			putstr(ppart_name);
			putstr("zwz bld_loader_validate_partition image absent... skipping\r\n");
		}
		goto bld_loader_load_partition_exit;
	}

	img_len = pptb_part->img_len;
	mem_addr = 0x100000;//pptb_part->mem_addr;
	if (verbose) {
		putstr("loading ");
		putstr(ppart_name);
		putstr(" to 0x");
		puthex(mem_addr);
		putstr("\r\n");
	}


	boot_from = get_part_dev(part_id);
    putstr("zwz bld_loader_validate_partition zwz boot_from ");
    putstr(ppart_name);
    putstr("  boot_from= ");
    putdec(boot_from);
    putstr("  crc32= ");
    puthex(pptb_part->crc32);
    putstr("  image len= ");
    puthex(pptb_part->img_len);

    putstr("\r\n");

#if defined(CONFIG_AMBOOT_ENABLE_NAND)
	if ((ret_val < 0) && (boot_from & PART_DEV_NAND)) {
		ret_val = bld_loader_load_partition_nand(part_id,
			mem_addr, img_len, pptb_part->flag);
	}
#endif
#if defined(CONFIG_AMBOOT_ENABLE_SD)
	if ((ret_val < 0) && (boot_from & PART_DEV_EMMC)) {
		ret_val = bld_loader_load_partition_sm(part_id,
			mem_addr, img_len, pptb_part->flag);
	}
#endif
#if defined(CONFIG_AMBOOT_ENABLE_SPINOR)
	if ((ret_val < 0) && (boot_from & PART_DEV_SPINOR)) {
		ret_val = bld_loader_load_partition_spinor(part_id,
			mem_addr, img_len, pptb_part->flag);
	}
#endif

#if defined(CONFIG_AMBOOT_ENABLE_SPINAND)
	if ((ret_val < 0) && (boot_from & PART_DEV_SPINAND)) {
		ret_val = bld_loader_load_partition_spinand(part_id,
			mem_addr, img_len, pptb_part->flag);
	}
#endif


	if (ret_val < 0) {
		if (verbose) {
			putstr(ppart_name);
			putstr("[");
			puthex(boot_from);
			putstr("] - load_partition() failed!\r\n");
		}
		goto bld_loader_load_partition_exit;
	}

    //crc verify:
    //ret_val = bld_loader_validate_image((u8 *)mem_addr,img_len,pptb_part->crc32);


bld_loader_load_partition_exit:
	return ret_val;
}


#endif

int bld_loader_load_partition(int part_id,
	const flpart_table_t *pptb, int verbose)
{
	int ret_val = -1;
	const char *ppart_name;
	const flpart_t *pptb_part;
	u32 mem_addr = 0x0;
	u32 img_len = 0x0;
	u32 boot_from;

	if ((part_id >= HAS_IMG_PARTS) || (part_id == PART_PTB)) {
		if (verbose) {
			putstr("illegal partition ID.\r\n");
		}
		goto bld_loader_load_partition_exit;
	}
	ppart_name = get_part_str(part_id);
	pptb_part = &pptb->part[part_id];

	if (pptb_part->magic != FLPART_MAGIC) {
		if (verbose) {
			putstr(ppart_name);
			putstr(" partition appears damaged... skipping\r\n");
		}
		goto bld_loader_load_partition_exit;
	}
	if ((pptb_part->mem_addr < DRAM_START_ADDR) ||
		(pptb_part->mem_addr > (DRAM_START_ADDR + DRAM_SIZE - 1))){
		if (verbose) {
			putstr(ppart_name);
			putstr(" wrong load address... skipping\r\n");
		}
		goto bld_loader_load_partition_exit;
	}
	if ((pptb_part->img_len == 0) || (pptb_part->img_len == 0xffffffff)) {
		if (verbose) {
			putstr(ppart_name);
			putstr(" image absent... skipping\r\n");
		}
		goto bld_loader_load_partition_exit;
	}
	if (pptb_part->flag & PART_NO_LOAD) {
		if (verbose) {
			putstr(ppart_name);
			putstr(" has no-load flag set... skipping\r\n");
		}
		goto bld_loader_load_partition_exit;
	}

	img_len = pptb_part->img_len;
	mem_addr = pptb_part->mem_addr;
	if (verbose) {
		putstr("loading ");
		putstr(ppart_name);
		putstr(" to 0x");
		puthex(mem_addr);
		putstr("\r\n");
	}
#ifdef AMBA_SDK_XIAOYI_ALTER  

	boot_from = get_part_dev(part_id);
    putstr("zwz boot_from ");
    putstr(ppart_name);
    putstr("boot_from= ");
    putdec(boot_from);
    putstr("  crc32= ");
    puthex(pptb_part->crc32);
    
    putstr("\r\n");
#endif
#if defined(CONFIG_AMBOOT_ENABLE_NAND)
	if ((ret_val < 0) && (boot_from & PART_DEV_NAND)) {
		ret_val = bld_loader_load_partition_nand(part_id,
			mem_addr, img_len, pptb_part->flag);
	}
#endif
#if defined(CONFIG_AMBOOT_ENABLE_SD)
	if ((ret_val < 0) && (boot_from & PART_DEV_EMMC)) {
		ret_val = bld_loader_load_partition_sm(part_id,
			mem_addr, img_len, pptb_part->flag);
	}
#endif
#if defined(CONFIG_AMBOOT_ENABLE_SPINOR)
	if ((ret_val < 0) && (boot_from & PART_DEV_SPINOR)) {
		ret_val = bld_loader_load_partition_spinor(part_id,
			mem_addr, img_len, pptb_part->flag);
	}
#endif

#if defined(CONFIG_AMBOOT_ENABLE_SPINAND)
	if ((ret_val < 0) && (boot_from & PART_DEV_SPINAND)) {
		ret_val = bld_loader_load_partition_spinand(part_id,
			mem_addr, img_len, pptb_part->flag);
	}
#endif


	if (ret_val < 0) {
		if (verbose) {
			putstr(ppart_name);
			putstr("[");
			puthex(boot_from);
			putstr("] - load_partition() failed!\r\n");
		}
		goto bld_loader_load_partition_exit;
	}

    //crc verify:
#ifdef AMBA_SDK_XIAOYI_ALTER  
    ret_val = bld_loader_validate_image((u8 *)mem_addr,img_len,pptb_part->crc32);
#endif

bld_loader_load_partition_exit:
	return ret_val;
}

/*===========================================================================*/
int bld_loader_boot_partition(int verbose, u32 *pjump_addr,
	u32 *pinitrd2_start, u32 *pinitrd2_size)
{
	int ret_val;
	flpart_table_t ptb;
	u32 os_start = 0;
	u32 os_len = 0;
	u32 rmd_start = 0;
	u32 rmd_size = 0;

	ret_val = flprog_get_part_table(&ptb);
	if (ret_val < 0)
		return ret_val;

	if (verbose) {
		bld_loader_display_ptb_content(&ptb);
	}

#ifdef AMBA_SDK_XIAOYI_ALTER 
    int ret_val_pri = -1;
    int ret_val_bak = -1;
    int ret_val_lnx = -1;
    int ret_val_rmd = -1;

    int bak_sel = ptb.dev.bak_sel;


    int ubifs_sel_orig = UBIFS_SEL_LNX;//lnx
    int ubifs_sel_current = UBIFS_SEL_LNX;
    int kernel_first = PART_PRI;
    int kernel_second = PART_BAK;
    

    putstr("  cmd string = ");
    putstr(ptb.dev.cmdline);
    putstr("   ...\r\n");
    
    if (strstr(ptb.dev.cmdline,"ubi.mtd=rmd",strlen(ptb.dev.cmdline)))
    {
        ubifs_sel_orig = UBIFS_SEL_RMD;//rmd
    }
    else
    {
        ubifs_sel_orig = UBIFS_SEL_LNX;// lnx
    }

    if (strlen(ptb.dev.cmdline) == 0)
    {
        if (bak_sel == UBIFS_SEL_LNX)
        {
            strcpy(ptb.dev.cmdline,"fdt cmdline console=ttyS0 ubi.mtd=lnx root=ubi0:rootfs rw rootfstype=ubifs init=/linuxrc");
            parse_command("fdt cmdline console=ttyS0 ubi.mtd=lnx root=ubi0:rootfs rw rootfstype=ubifs init=/linuxrc");
            flprog_set_part_table(&ptb);
        }
        else
        {
            strcpy(ptb.dev.cmdline,"fdt cmdline console=ttyS0 ubi.mtd=rmd root=ubi0:rootfs rw rootfstype=ubifs init=/linuxrc");
            parse_command("fdt cmdline console=ttyS0 ubi.mtd=rmd root=ubi0:rootfs rw rootfstype=ubifs init=/linuxrc");
            flprog_set_part_table(&ptb);
        }
    }
    
    putstr("  bak_sel = ");
    puthex(bak_sel);
    putstr("   ...\r\n"); 

    putstr("  ubifs_sel_orig = ");
    puthex(ubifs_sel_orig);
    putstr("   ...\r\n"); 
    

    
    //judge current ubifs to mount:
    if (bak_sel == UBIFS_SEL_LNX)
    {

        ret_val_pri = bld_loader_validate_partition(PART_PRI, &ptb, 1);
        if (ret_val_pri != 0)
        {
            putstr("zwz check pri error!!!\r\n");
        }

        
        ret_val_lnx = bld_loader_validate_partition(PART_LNX, &ptb, 1);
        if (ret_val_lnx != 0)
        {
            putstr("zwz check lnx error!!!\r\n");   
        }

        if (ret_val_pri == 0 && ret_val_lnx == 0)
        {
            ubifs_sel_current = UBIFS_SEL_LNX;
        }
        else
        {
            ubifs_sel_current = UBIFS_SEL_RMD;
        }
    }
    else
    {  

        ret_val_bak = bld_loader_validate_partition(PART_BAK, &ptb, 1);
        if (ret_val_bak != 0)
        {
            putstr("zwz check bak error!!!\r\n");
        }
        
        ret_val_rmd = bld_loader_validate_partition(PART_RMD, &ptb, 1);
        if (ret_val_rmd != 0)
        {
            putstr("zwz check rmd error!!!\r\n");
        }

        if (ret_val_bak == 0 && ret_val_rmd == 0)
        {
            ubifs_sel_current = UBIFS_SEL_RMD;
        }
        else
        {
            ubifs_sel_current = UBIFS_SEL_LNX;
        }
        
    }

    if (ubifs_sel_current == UBIFS_SEL_LNX && ubifs_sel_orig != UBIFS_SEL_LNX)
    {
        strcpy(ptb.dev.cmdline,"fdt cmdline console=ttyS0 ubi.mtd=lnx root=ubi0:rootfs rw rootfstype=ubifs init=/linuxrc");
        parse_command("fdt cmdline console=ttyS0 ubi.mtd=lnx root=ubi0:rootfs rw rootfstype=ubifs init=/linuxrc");
        ptb.dev.bak_sel = UBIFS_SEL_LNX;
        flprog_set_part_table(&ptb);
        putstr("    zwz set lnx over!!!\r\n");

    }
    else if (ubifs_sel_current == UBIFS_SEL_RMD && ubifs_sel_orig != UBIFS_SEL_RMD)
    {
        strcpy(ptb.dev.cmdline,"fdt cmdline console=ttyS0 ubi.mtd=rmd root=ubi0:rootfs rw rootfstype=ubifs init=/linuxrc");
        parse_command("fdt cmdline console=ttyS0 ubi.mtd=rmd root=ubi0:rootfs rw rootfstype=ubifs init=/linuxrc");
        ptb.dev.bak_sel = UBIFS_SEL_RMD;
        flprog_set_part_table(&ptb);
        putstr("    zwz set rmd over!!!\r\n");

    }
    else
    {

    }

    
    

    
#endif


#ifdef AMBA_SDK_XIAOYI_ALTER  

    if (ubifs_sel_current == UBIFS_SEL_LNX)
    {
        kernel_first = PART_PRI;
        kernel_second = PART_BAK;
    }
    else
    {
        kernel_first = PART_BAK;
        kernel_second = PART_PRI;
    }
    ret_val = bld_loader_load_partition(kernel_first, &ptb, verbose);
    if (ret_val < 0) {


    ret_val = bld_loader_load_partition(kernel_second, &ptb, verbose);
    if (ret_val < 0)
        return ret_val;
    
    putstr("  boot to rmd!!!\r\n");
    putstr("[boot]: xiaoyi boot from PART_BAK\r\n");
    os_start = ptb.part[PART_BAK].mem_addr;
    os_len = ptb.part[PART_BAK].img_len;
    } else {
        putstr("  boot to lnx!!!\r\n");
        putstr("[boot]: xiaoyi boot from PART_PRI\r\n");
        os_start = ptb.part[PART_PRI].mem_addr;
        os_len = ptb.part[PART_PRI].img_len;
    }

        
#else
    ret_val = bld_loader_load_partition(PART_PRI, &ptb, verbose);
    if (ret_val < 0) {

    ret_val = bld_loader_load_partition(PART_SEC, &ptb, verbose);
    putstr("[boot]: boot from PART_SEC\r\n");
    if (ret_val < 0)
        return ret_val;

    os_start = ptb.part[PART_SEC].mem_addr;
    os_len = ptb.part[PART_SEC].img_len;
    } else {
        os_start = ptb.part[PART_PRI].mem_addr;
        os_len = ptb.part[PART_PRI].img_len;
    }

#endif



#if defined(SECURE_BOOT)
	if (ptb.dev.need_generate_firmware_hw_signature) {
		ret_val = generate_firmware_hw_signature((unsigned char *) os_start, os_len);
		if (0 == ret_val) {
			putstr("[secure boot]: generate hw signature done\r\n");
		} else {
			putstr("[secure boot]: generate hw signature fail\r\n");
			return FLPROG_ERR_FIRM_HW_SIGN_FAIL;
		}

		ptb.dev.need_generate_firmware_hw_signature = 0;
		flprog_set_part_table(&ptb);
	} else {
		if (0 == verify_firmware_hw_signature((unsigned char *) os_start, os_len)) {
			putstr("[secure boot]: verify firmware OK\r\n");
		} else {
			putstr("[secure boot]: verify firmware fail\r\n");
			return FLPROG_ERR_FIRM_HW_SIGN_VERIFY_FAIL;
		}
	}
#endif

	ret_val = bld_loader_load_partition(PART_RMD, &ptb, verbose);
	if (ret_val == 0x0) {
		rmd_start = ptb.part[PART_RMD].mem_addr;
		rmd_size  = ptb.part[PART_RMD].img_len;
	}

	if (pjump_addr) {
		*pjump_addr = os_start;
	}
	if (pinitrd2_start) {
		*pinitrd2_start = rmd_start;
	}
	if (pinitrd2_size) {
		*pinitrd2_size = rmd_size;
	}

	return 0;
}

int boot(const char *cmdline, int verbose)
{
	u32 jump_addr = 0, rmd_start = 0, rmd_size = 0;
	u32 cortex_jump, cortex_atag;
	int ret_val;

	ret_val = bld_loader_boot_partition(verbose,
			&jump_addr, &rmd_start, &rmd_size);
	if (ret_val < 0) {
		goto boot_exit;
	}

	if (verbose) {
		putstr("Jumping to 0x");
		puthex(jump_addr);
		putstr(" ...\r\n");
	}
#if defined(CONFIG_AMBOOT_ENABLE_ETH)
	bld_net_down();
#endif

#if defined(AMBOOT_DEV_BOOT_CORTEX)
	cortex_jump = ARM11_TO_CORTEX((u32)cortex_processor_start);
#elif defined(AMBOOT_BOOT_SECONDARY_CORTEX)
	cortex_jump = (u32)secondary_cortex_jump;
#else
	cortex_jump = 0;
#endif

#if defined(CONFIG_AMBOOT_BD_FDT_SUPPORT)
	cortex_atag = fdt_update_tags((void *)jump_addr,
		cmdline, cortex_jump, rmd_start, rmd_size, verbose);
#elif defined(CONFIG_AMBOOT_BD_ATAG_SUPPORT)
	cortex_atag = setup_tags((void *)jump_addr,
		cmdline, cortex_jump, rmd_start, rmd_size, verbose);
#endif

#if defined(AMBOOT_BOOT_SECONDARY_CORTEX)
	bld_boot_secondary_cortex();
#endif

#if defined(AMBOOT_DEV_BOOT_CORTEX)
	*cortex_atag_data = cortex_atag;
	ret_val = bld_cortex_boot(verbose, jump_addr);
#else
	jump_to_kernel((void *)jump_addr, cortex_atag);
#endif

boot_exit:
	return ret_val;
}

/*===========================================================================*/
static int bld_loader_bios_partition(int verbose, u32 *pjump_addr)
{
	flpart_table_t ptb;
	int ret_val;

	ret_val = flprog_get_part_table(&ptb);
	if (ret_val < 0)
		return ret_val;

	if (verbose) {
		bld_loader_display_ptb_content(&ptb);
	}

	ret_val = bld_loader_load_partition(PART_PBA, &ptb, verbose);
	if (ret_val < 0)
		return ret_val;

	if (pjump_addr) {
		*pjump_addr = ptb.part[PART_PBA].mem_addr;
	}

	return 0;
}

int bios(const char *cmdline, int verbose)
{
	u32 jump_addr = 0;
	u32 cortex_jump;
	u32 cortex_atag;
	int ret_val;

	ret_val = bld_loader_bios_partition(verbose, &jump_addr);
	if (ret_val < 0) {
		goto bios_exit;
	}

	if (verbose) {
		putstr("Jumping to 0x");
		puthex(jump_addr);
		putstr(" ...\r\n");
	}
#if defined(CONFIG_AMBOOT_ENABLE_ETH)
	bld_net_down();
#endif

#if defined(AMBOOT_DEV_BOOT_CORTEX)
	cortex_jump = ARM11_TO_CORTEX((u32)cortex_processor_start);
#elif defined(AMBOOT_BOOT_SECONDARY_CORTEX)
	cortex_jump = (u32)secondary_cortex_jump;
#else
	cortex_jump = 0;
#endif

#if defined(CONFIG_AMBOOT_BD_FDT_SUPPORT)
	cortex_atag = fdt_update_tags((void *)jump_addr,
			cmdline, cortex_jump, 0, 0, verbose);
#elif defined(CONFIG_AMBOOT_BD_ATAG_SUPPORT)
	cortex_atag = setup_tags((void *)jump_addr,
			cmdline, cortex_jump, 0, 0, verbose);
#endif

#if defined(AMBOOT_DEV_BOOT_CORTEX)
	*cortex_atag_data = cortex_atag;
	ret_val = bld_cortex_boot(verbose, jump_addr);
#else
	jump_to_kernel((void *)jump_addr, cortex_atag);
#endif

bios_exit:
	return ret_val;
}
