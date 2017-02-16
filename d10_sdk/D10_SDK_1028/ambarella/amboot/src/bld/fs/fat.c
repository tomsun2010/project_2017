/**
 * fs/fat.c
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
#include <sdmmc.h>
#include <ambhw/cache.h>
#include <ambhw/vic.h>
#include <ambhw/sd.h>
#ifdef AMBA_SDK_XIAOYI_ALTER
#include <ambhw/gpio.h>
#endif
#include "fat.h"

#define AMBA_FAT_DBG 0
#define AMBA_FAT_EXT 0

#ifdef AMBA_SDK_XIAOYI_ALTER

	#define FILE_LIST_MAX  (4*SIZE_1MB/265)
#else
	#define FILE_LIST_MAX  4 * 16
#endif
char cur_dir[] = {0x2e, 0x20, 0x20, 0x20, 0x20, 0x20 ,0x20, 0x20};
char pre_dir[] = {0x2e, 0x2e, 0x20, 0x20, 0x20, 0x20 ,0x20, 0x20};

struct disk_info mydisk;

u8 *load_addr = (u8 *)DEFAULT_BUF_START;

#ifdef AMBA_SDK_XIAOYI_ALTER
u8 *tbl_load_addr = (u8 *)DEFAULT_BUF_START - 32 * SIZE_1MB;

/* fat32 default cluster start is 2 */
static u32 curcluster = 2;


static struct file* file_list=(struct file*)(DEFAULT_BUF_START + 2*SIZE_1MB);

#else
u8 *tbl_load_addr = (u8 *)DEFAULT_BUF_START - 16 * SIZE_1MB;

/* fat32 default cluster start is 2 */
static u32 curcluster = 2;
static struct file file_list[FILE_LIST_MAX]
    __attribute__ ((section(".bss.noinit")));

#endif





/* file_list offset */
static int list_offset;
#ifdef AMBA_SDK_XIAOYI_ALTER
struct file *file_up = NULL;
#endif

static void downcase(char *str)
{
	while(*str != '\0'){
		if(*str <= 'Z' && *str >= 'A'){
			*str +='a' - 'A';
		}
		str++;
	}
}

static int fat_disk_write(u32 sector, u32 nr_sectors, unsigned int *buffer)
{

	int ret_val = 0;

	ret_val = sdmmc_write_sector(mydisk.boot_sector + sector, nr_sectors,
			(u8 *)buffer);
	if (ret_val < 0) {
		putstrhex("failed write at 0x", sector);
		return -1;
	}
	return 0;
}

static int fat_disk_read(u32 sector, u32 nr_sectors, unsigned int *buffer)
{

	int ret_val = 0;

	ret_val = sdmmc_read_sector(mydisk.boot_sector + sector, nr_sectors,
			(u8 *)buffer);
	if (ret_val < 0) {
		putstrhex("failed cluster read at 0x", sector);
		return -1;
	}
	return 0;
}

static int fat_disk_write_cluster(u32 cluster, unsigned int *buffer)
{
	int ret_val = 0;
	u32 start = CLUST2SECTOR(cluster);
	u32 nr_sectors = mydisk.clust_size;

	ret_val = fat_disk_write(start, nr_sectors, (unsigned int *)buffer);
	if (ret_val < 0) {
		putstrhex("failed cluster write at:", cluster);
		return -1;
	}
	return 0;
}

static int fat_disk_read_cluster(u32 cluster, unsigned int *buffer)
{
	int ret_val = 0;
	u32 start = CLUST2SECTOR(cluster);
	u32 nr_sectors = mydisk.clust_size;

	ret_val = fat_disk_read(start, nr_sectors, (unsigned int *)buffer);
	if (ret_val < 0) {
		putstrhex("failed Read at:", cluster);
		return -1;
	}
	return 0;
}

static int fat_filesys_check(void)
{

	/* Make sure it has a valid FAT header */
	if (fat_disk_read(0, 1, (unsigned int *)load_addr)) {
		return -1;
	}

	/* Check if it's actually a DOS volume */
	if (memcmp(load_addr + DOS_BOOT_MAGIC_OFFSET, "\x55\xAA", 2)) {
		return -1;
	}

	/* Check for FAT12/FAT16/FAT32 filesystem */
	if (!memcmp(load_addr + DOS_FS_TYPE_OFFSET, "FAT", 3))
		return 0;
	if (!memcmp(load_addr + DOS_FS32_TYPE_OFFSET, "FAT32", 5))
		return 0;

	return -2;
}

static int sector_magic_check(unsigned char *sector)
{

	if(memcmp(sector + DOS_PART_MAGIC_OFFSET , "\x55\xAA", 2)){
		putstr("bad sector magic\r\n");
		return -1;
	}
	return 0;
}


static int disk_dbr_info(void)
{
	struct boot_sector *dbr;

	if(fat_filesys_check()){
		putstr("filesystem not been found\r\n");
		return -1;
	}

	if(fat_disk_read(0, 1, (unsigned int *)load_addr))
		return -1;

	if(sector_magic_check(load_addr))
		return -1;

	dbr = (struct boot_sector *)load_addr;


	mydisk.data_sector = dbr->fats * dbr->fat32_length + dbr->reserved;
	mydisk.tbl_addr = dbr->reserved;
	mydisk.fatlength = dbr->fat32_length;
	mydisk.fat_sect = dbr->reserved;
	mydisk.sect_size = *(u16 *)dbr->sector_size;
	mydisk.clust_size = dbr->cluster_size;

	curcluster = 2;

	return 0;
}

static int disk_mbr_info(void)
{
#ifdef AMBA_SDK_XIAOYI_ALTER
    //struct boot_sector *dbr;
#else
	struct boot_sector *dbr;
#endif	
    dos_partition_t *partition;
    int ret_val = 0;

    ret_val = sdmmc_read_sector(0, 1, load_addr);
    if (ret_val < 0) {
        putstr("failed Read at 0 sector\r\n");
        return -1;
    }

    if(sector_magic_check(load_addr))
        return -1;

    /* The first sector is MBR */

#ifdef AMBA_SDK_XIAOYI_ALTER
    if(fat_filesys_check()==-1)
    {
        putstr("fat_filesys_check fail\r\n");
        return    -1;
    }
    
    partition = (dos_partition_t *)(load_addr + DOS_PART_TBL_OFFSET);
    
    mydisk.boot_sector = *(int*)partition->start4;
    mydisk.total_sector = *(int*)partition->size4;
#else
    if(fat_filesys_check() == -2){

		partition = (dos_partition_t *)(load_addr + DOS_PART_TBL_OFFSET);

		mydisk.boot_sector = *(int*)partition->start4;
		mydisk.total_sector = *(int*)partition->size4;
	}else{

		/* The first sector is DBR */

		dbr = (struct boot_sector *)load_addr;

		mydisk.boot_sector = 0;
		mydisk.total_sector = dbr->total_sect;
	}
#endif
    return 0;
}

static int disk_probe_once(void)
{
	memset(&mydisk, 0 , sizeof(mydisk));

	if(disk_mbr_info())
		return -1;

	if(disk_dbr_info())
		return -1;

	return 0;
}

int file_fat_info(void)
{

	disk_probe_once();

	putstrhex("DBR[sector]: 0x", mydisk.boot_sector);
	putstrhex("Total[sector]: 0x", mydisk.total_sector);

	putstrhex("Data[sector]: 0x", mydisk.data_sector);
	putstrhex("Fat addr[sector]: 0x", mydisk.tbl_addr);
	putstrhex("Fat length[sector]: 0x", mydisk.fatlength);
	putstrhex("Sector[byte]: 0x", mydisk.sect_size);
	putstrhex("Cluster[sector]: 0x", mydisk.clust_size);

	return 0;
}

static int slot2str(dir_slot *slotptr, char *l_name, int *idx)
{
	int j;

	for (j = 0; j <= 8; j += 2) {
		l_name[*idx] = slotptr->name0_4[j];
		if (l_name[*idx] == 0x00)
			return 1;
		(*idx)++;
	}
	for (j = 0; j <= 10; j += 2) {
		l_name[*idx] = slotptr->name5_10[j];
		if (l_name[*idx] == 0x00)
			return 1;
		(*idx)++;
	}
	for (j = 0; j <= 2; j += 2) {
		l_name[*idx] = slotptr->name11_12[j];
		if (l_name[*idx] == 0x00)
			return 1;
		(*idx)++;
	}

	return 0;
}

dir_entry * getvfatname(dir_entry *retdent, char *name, u8 *low)
{
	dir_entry *realdent;
	dir_slot *slotptr = (dir_slot *)retdent;
	u8 *limit = low + mydisk.sect_size * mydisk.clust_size;

	u8 counter = (slotptr->id & ~LAST_LONG_ENTRY_MASK) & 0xff;

	int idx = 0;

	if (counter > VFAT_MAXSEQ) {
		putstr("Error: VFAT name is too long\r\n");
		return NULL;
	}

	while ((u8 *)slotptr < limit) {
		if (counter == 0)
			break;
		if (((slotptr->id & ~LAST_LONG_ENTRY_MASK) & 0xff) != counter)
			return NULL;
		slotptr++;
		counter--;
	}

	if((u8 *)slotptr >= limit){
		putstr("no supported now\r\n");
		return NULL;
	}else{
		realdent = (dir_entry *)slotptr;
	}

	do {
		slotptr--;
		if (slot2str(slotptr, name, &idx))
			break;
	} while (!(slotptr->id & LAST_LONG_ENTRY_MASK));

	name[idx] = '\0';
	if (*name == DELETED_FLAG)
		*name = '\0';
	else if (*name == aRING)
		*name = DELETED_FLAG;

	return realdent;
}

static int fat_file_attr(dir_entry *entry)
{
	if((entry->attr & ATTR_DIR) == ATTR_DIR){
		putstr("/\t");
	}else if((entry->attr & ATTR_ARCH) == ATTR_ARCH){
		putstr("\t");
	}else{
		putstr("?\t");
	}
	return 0;
}

void fat_list_push(dir_entry *entry, char *name, int offset)
{
	strncpy(file_list[offset].name, name, strlen(name));

	file_list[offset].attr = entry->attr;
	file_list[offset].cluster = (entry->starthi << 16)|entry->start;
	file_list[offset].size = entry->size;
}

static int fat_filename_limit(const char *str)
{
	while(*str != '\0'){
		if(*str <= 'Z' && *str >= 'A')
			return -1;
		str++;
	}
	return 0;
}

static void get_name(dir_entry *dirent, char *s_name)
{
	char *ptr;

	memcpy(s_name, dirent->name, 8);
	s_name[8] = '\0';
	ptr = s_name;
	while (*ptr && *ptr != ' ')
		ptr++;
	if (dirent->ext[0] && dirent->ext[0] != ' ') {
		*ptr = '.';
		ptr++;
		memcpy(ptr, dirent->ext, 3);
		ptr[3] = '\0';
		while (*ptr && *ptr != ' ')
			ptr++;
	}
	*ptr = '\0';
	if (*s_name == DELETED_FLAG)
		*s_name = '\0';
	else if (*s_name == aRING)
		*s_name = DELETED_FLAG;

	downcase(s_name);
}

#ifdef AMBA_SDK_XIAOYI_ALTER


static int fat_get_tbl_simple(void)
{
    u32 tbl_addr = mydisk.tbl_addr;
    u32 tbl_length = mydisk.fatlength;


    //putstr("tbl length=0x");
    //puthex(tbl_length);
    //putstr("\r\n");


    /* the first TBL may be enough */
    if(fat_disk_read(tbl_addr, 2*tbl_length, (unsigned int *)tbl_load_addr))
        return -1;

    if(*(int *)tbl_load_addr != FATTBL_MAGIC){
        putstrhex("Fat TBL format error", *(int *)tbl_load_addr);
        return -1;
    }	

	return 0;
}



static int do_fat_update(int flag)
{
    dir_entry *dirptr;
    char s_name[256];
	u32 clust_next=curcluster;


    list_offset = 0;
    memset(file_list , 0, 2*SIZE_1MB);
	memset(s_name, 0, sizeof(s_name));

	if(fat_get_tbl_simple()<0)
	{
		return -1;
	}

	do
	{
		if(fat_disk_read_cluster(clust_next,(unsigned int *)load_addr))
        {
        	return -1;
		}

	    dirptr = (dir_entry *)load_addr;

	    while(((u32)dirptr - (u32)load_addr) < CLUSTSIZE)
	    {

	        /* no need show deleted files ... */

	        if(dirptr->name[0] == DELETED_FLAG
	                || dirptr->name[0] == 0){
	            dirptr++;
	            continue;
	        }

	        if(dirptr->attr & ATTR_VOLUME){

	            if((dirptr->attr & ATTR_VFAT) == ATTR_VFAT
	                    && (dirptr->name[0] & LAST_LONG_ENTRY_MASK) ){

	                dirptr = getvfatname(dirptr, s_name, load_addr);
	                if(!dirptr)
	                    return -1;
	            }else{
	                /* XXX we need this ? */
	                dirptr++;
	                continue;
	            }
	        }else{
	            /* for having LFN without SFN . Cards have two diffrent rule . why ? */
	            get_name(dirptr, s_name);
	        }

#if AMBA_FAT_DBG
	        if(flag){
	            putstr("[");
	            puthex(dirptr->attr);
	            putstr(" ");
	            puthex((dirptr->starthi << 16)|dirptr->start);
	            putstr(" ");
	            puthex(dirptr->size);
	            putstr(" ");
	            putstr("]");
	        }
#endif

	        fat_list_push(dirptr, s_name, list_offset);
	        list_offset++;

	        if(flag){
	            putstr(s_name);
	            fat_file_attr(dirptr);
	            putstr("\r\n");

	            /* just show FILE_LIST_MAX files */
				#if 0
	            if(list_offset >= FILE_LIST_MAX){
	                putstr("... \r\n");
	                return -1;
	            }
				#endif
	        }
	        dirptr++;

		    if(list_offset >= FILE_LIST_MAX)
			{
		        putstr("... \r\n");
		        return -1;
		    }
	    }
	
        clust_next = *((int*)tbl_load_addr + clust_next);
		putstr("file cluster list=");
		puthex(clust_next);
		putstr("\r\n");
		


        if(clust_next == FATTBL_END
                || clust_next == FATTBL_BAD
                || clust_next == FATTBL_MAGIC
                || !clust_next)
        {
        	putstr("file cluster end@@@\r\n");
            break;
        }

	}while(1);
	
    return 0;
}
#else

static int do_fat_update(int flag)
{
	dir_entry *dirptr;
	char s_name[256];

	list_offset = 0;
	memset(file_list , 0, sizeof(file_list));

	if(fat_disk_read_cluster(curcluster,(unsigned int *)load_addr))
		return -1;

	dirptr = (dir_entry *)load_addr;

	while(((u32)dirptr - (u32)load_addr) < CLUSTSIZE)
	{

		/* no need show deleted files ... */

		if(dirptr->name[0] == DELETED_FLAG
				|| dirptr->name[0] == 0){
			dirptr++;
			continue;
		}

		if(dirptr->attr & ATTR_VOLUME){

			if((dirptr->attr & ATTR_VFAT) == ATTR_VFAT
					&& (dirptr->name[0] & LAST_LONG_ENTRY_MASK) ){

				dirptr = getvfatname(dirptr, s_name, load_addr);
				if(!dirptr)
					return -1;
			}else{
				/* XXX we need this ? */
				dirptr++;
				continue;
			}
		}else{
			/* for having LFN without SFN . Cards have two diffrent rule . why ? */
			get_name(dirptr, s_name);
		}

#if AMBA_FAT_DBG
		if(flag){
			putstr("[");
			puthex(dirptr->attr);
			putstr(" ");
			puthex((dirptr->starthi << 16)|dirptr->start);
			putstr(" ");
			puthex(dirptr->size);
			putstr(" ");
			putstr("]");
		}
#endif

		fat_list_push(dirptr, s_name, list_offset);
		list_offset++;

		if(flag){
			putstr(s_name);
			fat_file_attr(dirptr);
			putstr("\r\n");

			/* just show FILE_LIST_MAX files */
			if(list_offset >= FILE_LIST_MAX){
				putstr("... \r\n");
				return -1;
			}
		}
		dirptr++;
	}
	return 0;
}

#endif


static int do_fat_chdir(const char *dir)
{
	int i;
	struct file *file;

	for(i = 0; i < list_offset; i++){

		if(!strcmp(dir, file_list[i].name)){

			if(file_list[i].attr == ATTR_DIR){

				file = &file_list[i];

				if(file->cluster != 0){
					curcluster = file->cluster;
				}else{
				/* if parent dir ".." is root dir, the cluster is 0 */
					curcluster = 2;
				}
#if AMBA_FAT_DBG
				putstrhex("*curcluster: 0x", curcluster);
#endif
				return 0;
			}else{
				putstr(dir);
				putstr(" is not a directory\r\n");
				return -1;
			}
		}
	}
	putstr(dir);
	putstr(" file no exist\r\n");
	return -1;
}

static int fat_set_tbl(void)
{
	u32 tbl_addr = mydisk.tbl_addr;
	u32 tbl_length = mydisk.fatlength;

	/* the first TBL may be enough */
	if(fat_disk_write(tbl_addr, tbl_length, (unsigned int *)tbl_load_addr))
		return -1;

	return 0;
}

static int fat_get_tbl(void)
{
    u32 tbl_addr = mydisk.tbl_addr;
    u32 tbl_length = mydisk.fatlength;
#ifdef AMBA_SDK_XIAOYI_ALTER
	
    int* cluster_ptr;
    int i;
    int j;

    putstr("tbl length=0x");
    puthex(tbl_length);
    putstr("\r\n");

    putstr("file start cluster=0x");
    puthex(file_up->cluster);
    putstr("\r\n");

    /* the first TBL may be enough */
    if(fat_disk_read(tbl_addr, 2*tbl_length, (unsigned int *)tbl_load_addr))
        return -1;

    if(*(int *)tbl_load_addr != FATTBL_MAGIC){
        putstrhex("Fat TBL format error", *(int *)tbl_load_addr);
        return -1;
    }

    cluster_ptr=(int*)tbl_load_addr;
    for(i=0;i<(2*tbl_length*512)/4; i++)
    {
        if(*(cluster_ptr+i)==(file_up->cluster+1))
        {
            putstr("1 cluster idx=0x");
            puthex(i);
            putstr("\r\n");

            for(j=0; j<20; j++)
            {
                puthex(j);
                putstr("cluster value=0x");
                puthex(*(cluster_ptr+i+j));
                putstr("\r\n");
            }
        }

        if(*(cluster_ptr+i)==file_up->cluster)
        {
            putstr("2 cluster idx=0x");
            puthex(i);
            putstr("\r\n");    
        }
    }
#else
	/* the first TBL may be enough */
	if(fat_disk_read(tbl_addr, tbl_length, (unsigned int *)tbl_load_addr))
		return -1;

	if(*(int *)tbl_load_addr != FATTBL_MAGIC){
		putstrhex("Fat TBL format error", *(int *)tbl_load_addr);
		return -1;
	}
#endif	

	return 0;
}

static struct file *fat_read_detect(const char *filename, u32 addr)
{
	int i;
	struct file *file;

#ifdef AMBA_SDK_XIAOYI_ALTER
    for(i = 0; i < list_offset && i < FILE_LIST_MAX; i++){
		#if 0
		putstr(file_list[i].name);
		putstr("\r\n");
		#endif
#else
	for(i = 0; i < list_offset; i++){
#endif	

		if(!strcmp(filename, file_list[i].name)){
			if(file_list[i].attr == ATTR_ARCH){

				file = &file_list[i];
				if(addr + file->size > (u32)DEFAULT_BUF_START){
					putstr("file size is too big to read\r\n");
					return NULL;
				}

				return file;
			}else{
				putstr(filename);
				putstr(" is not a file\r\n");
				return NULL;
			}
		}
	}

	putstr(filename);
	putstr(" file no exist\r\n");
	return NULL;
}

static void showprogress(int progress)
{
	 int k = 0;
	 int j = 0;

	 if(progress > 100)
		 return ;
	 for (k = 0; k < 109; k++)
		 putchar('\b');

	 putchar('[');

	 for ( j = 0; j < progress; j++)
		 putchar('=');
	 putchar('>');

	 for ( j = 1; j <= 100 - progress; j++)
		 putchar(' ');

	 putstr("]%");
	 putdec(progress);

	 putchar('\r');
}

static int fat_load_file(u32 *tbl, u32 cluster, u32 addr, u32 nr_clust)
{
	int step, max;
	int progress = 0;
	int i = 0;
	u32 clust_next = cluster;

	step = max = nr_clust / 100;

	do{
		i++;
#if AMBA_FAT_DBG
		putstrhex("Read cluster: ", clust_next);
#endif
		fat_disk_read_cluster(clust_next, (unsigned int *)addr);

		addr += CLUSTSIZE;
		clust_next = *(tbl + clust_next);

		if(!--step){
			step = max;
			showprogress(progress++);
		}

		if(clust_next == FATTBL_END
				|| clust_next == FATTBL_BAD
				|| clust_next == FATTBL_MAGIC
				|| !clust_next)
			break;

	}while(1);

	if(max == 0)
		showprogress(100);

	putstr("\n");
	return 0;
}

static int fat_read_progress(struct file *file, u32 addr)
{
	u32 *fat_tbl = (u32 *)tbl_load_addr;
	u32 cluster_start = file->cluster;

	u32 nr_clust =  file->size / CLUSTSIZE;


	fat_load_file(fat_tbl, cluster_start, addr, nr_clust);

	putstr("[");
	puthex(file->size);
	putstr("]");
	putdec(file->size);
	putstr(" bytes read success ...\r\n");
	return 0;
}
#ifdef AMBA_SDK_XIAOYI_ALTER
typedef struct ver
{
    u32    ver_num;    /**< Version number */
    u32    ver_date;    /**< Version date */
}VER;

static  int do_file_check(const char *filename, u32 addr)
{


    //VER *pverinfo = NULL;

    file_up = fat_read_detect(filename, addr);
    if(!file_up)
        return -1;

    if(fat_get_tbl())
        return -1;

    return 0;

}

static  int do_fat_read(const char *filename, u32 addr, VER *pstVer)
{
    fat_read_progress(file_up ,addr);    
    return 0;
}
#else
static  int do_fat_read(const char *filename, u32 addr)
{

    struct file *file;

	file = fat_read_detect(filename, addr);
	if(!file)
		return -1;

	if(fat_get_tbl())
		return -1;

	fat_read_progress(file ,addr);

	return 0;
}
#endif

static struct file *fat_write_detect(const char *filename)
{

	int i;
	struct file *file = NULL;

	for(i = 0; i < list_offset; i++){
		if(!strncmp(filename, file_list[i].name, 8)){
				file = &file_list[i];
				break;
		}
	}
	return file;
}

static int fat_upload_file(dir_entry *dirptr, u32 addr, u32 size)
{

	int step, max;
	int progress = 0;
	u32 *fat_tbl = (u32 *)tbl_load_addr;
	int nr_clust =  size / CLUSTSIZE;

	u32 cluster ;
	u32 *next_clust = NULL;

	step = max = nr_clust / 100;

	if(nr_clust * CLUSTSIZE < size)
		nr_clust++;

	if(fat_get_tbl())
		return -1;

	while(((u32)fat_tbl - (u32)tbl_load_addr)
			< (mydisk.fatlength * mydisk.sect_size))
	{
		if(*fat_tbl){
			fat_tbl++;
			continue;
		}

		cluster = ((u32)fat_tbl - (u32)tbl_load_addr) / 4;

		/* next_clust record next cluster */
		if(next_clust != NULL)
			*next_clust = cluster;

		next_clust = fat_tbl;

		if(dirptr->start == 0 && dirptr->starthi == 0){
			dirptr->start = cluster & 0xffff;
			dirptr->starthi = (cluster >> 16) & 0xffff;
		}

		if(!--step){
			step = max;
			showprogress(progress++);
		}

		fat_disk_write_cluster(cluster, (u32 *)addr);

		if(!--nr_clust){
			*next_clust = FATTBL_END;
			break;
		}
		fat_tbl++;
	}

	if(max == 0)
		showprogress(100);

	putstr("\n");
	fat_set_tbl();
	return 0;
}

static int fat_write_progress(const char *filename, u32 addr, u32 size)
{
	int i;
	dir_entry *dirptr;
	char l_name[8];
	char default_ext[] = {0x20, 0x20, 0x20};

	memset(l_name, 0x20, 8);

	if(strlen(filename) >= 8)
		memcpy(l_name, filename, 8);
	else
		memcpy(l_name, filename, strlen(filename));


	if(fat_disk_read_cluster(curcluster,(unsigned int *)load_addr))
		return -1;

	dirptr = (dir_entry *)load_addr;

	for(i = 0; i < DIRENTSPERCLUST; i++){
		if(dirptr->name[0] != DELETED_FLAG
				&& dirptr->name[0] != 0){
			dirptr++;
			continue;
		}

		memset(dirptr, 0, 32);

		dirptr->attr = ATTR_ARCH;
		dirptr->size = size;
		memcpy(dirptr->name, l_name, 8);

#if  AMBA_FAT_EXT
		memcpy(dirptr->ext,"TXT",3);
#else
		memcpy(dirptr->ext, default_ext, 3);
#endif

		dirptr->lcase    = DEFAULT_LCASE;
		dirptr->ctime_ms = DEFAULT_CTIME_MS;
		dirptr->ctime    = DEFAULT_CTIME;
		dirptr->cdate    = DEFAULT_CDATE;
		dirptr->adate    = DEFAULT_ADATE;
		dirptr->time     = DEFAULT_TIME;
		dirptr->date     = DEFAULT_DATE;


		if(size != 0)
			fat_upload_file(dirptr, addr, size);

		if(fat_disk_write_cluster(curcluster,(unsigned int *)load_addr))
			return -1;

		putstr("[");
		puthex(size);
		putstr("]");
		putdec(size);
		putstr(" bytes write success ...\r\n");

		break;
	}

	return 0;

}


static int do_fat_write(const char *filename, u32 addr, u32 size)
{
	if(fat_write_detect(filename)){
		putstr("file is exist\r\n");
		return -1;
	}

	if(fat_get_tbl())
		return -1;

	fat_write_progress(filename, addr, size);

	return 0;
}


int file_fat_ls(const char *dir)
{
	if(disk_probe_once())
		return -1;

	do_fat_update(1);
	return 0;
}

int file_fat_chdir(const char *dir)
{
	if(disk_probe_once())
		return -1;

	do_fat_chdir(dir);
	do_fat_update(0);
	return 0;
}

int file_fat_read(const char *filename, u32 addr, int exec)
{
#ifdef AMBA_SDK_XIAOYI_ALTER
    flpart_table_t ptb;
    VER stVer = {0};
	flprog_get_part_table(&ptb);
#else
	void *ptr;
	void (*jump)(void) = (void *)addr;
#endif

    if(addr < DRAM_START_ADDR || addr > DRAM_START_ADDR + DRAM_SIZE-1){
        putstr("invalid addr\r\n");
        return -1;
    }

    if(disk_probe_once())
        return -1;

    do_fat_update(0);
#ifdef AMBA_SDK_XIAOYI_ALTER
    if(do_file_check(filename, addr)<0)
    {
        return    -1;
    }

    //start to load image
    //red_led_set(0);      /*xu.baikun 2015.12.31 masked codes*/
    //green_led_set(0);    /*xu.baikun 2015.12.31 masked codes*/
    //blue_led_set(1);     /*xu.baikun 2015.12.31 masked codes*/
    yellow_led_set(1);     /*xu.baikun 2015.12.31 added codes*/

    if(do_fat_read(filename, addr, &stVer))
    {
        return -1;
    }
    
    #if 0
    if ((stVer.ver_date == ptb.part[0].ver_date) && (stVer.ver_num == ptb.part[0].ver_num))
    {
        exec = 0;
    }
    putstr("old ver: \r\n");
    putdec(ptb.part[0].ver_date);
    putstr("@");
    putdec(ptb.part[0].ver_num);
    putstr("\r\nnew ver: \r\n");
    putdec(stVer.ver_date);
    putstr("@");
    putdec(stVer.ver_num);
    putstr("\r\n");
    #endif

    //blue_led_set(0);	/*xu.baikun 2015.12.31 masked codes*/	
    
    putstr("Start to erase image...\r\n");
    _clean_flush_all_cache();
    _disable_icache();
    _disable_dcache();
    disable_interrupts();

    //green_led_set(1);		/*xu.baikun 2015.12.31 masked codes*/
    image_parse_and_erase((u8*)addr, ptb.part[0].ver_date, ptb.part[0].ver_num);
    //while (i++ < 0xFFFFFFFF);


    //blue_led_set(0);
    //green_led_set(0);
    //red_led_set(1);
    putstr("finished\r\n");
    //while(1);

    

    #else

	if(do_fat_read(filename, addr))
		return -1;	
	
    if (exec) {
        putstr("Start to run...\r\n");
        _clean_flush_all_cache();
        _disable_icache();
        _disable_dcache();
        disable_interrupts();

		/* put the return address in 0xc00ffffc
		 * and let jump() to read and return back */
		ptr = &&_return_;
		*(volatile u32 *)(DRAM_START_ADDR + 0x000ffffc) = (u32) ptr;

		__asm__ __volatile__ ("nop");
		__asm__ __volatile__ ("nop");
		__asm__ __volatile__ ("nop");
		__asm__ __volatile__ ("nop");

		jump ();

		/* being here only if REBOOT_AFTER_BURNING */
_return_:
		__asm__ __volatile__ ("nop");
		__asm__ __volatile__ ("nop");
		__asm__ __volatile__ ("nop");
		__asm__ __volatile__ ("nop");

    }
    #endif
    
    return 0;
}

int file_fat_write(const char *filename, u32 addr, u32 size)
{

	if(disk_probe_once())
		return -1;

	do_fat_update(0);

	if(addr < DRAM_START_ADDR
			|| ( addr + size > DRAM_START_ADDR + DRAM_SIZE - 1)){
		putstr("invalid addr and size\r\n");
		return -1;
	}

	if(fat_filename_limit(filename)){
		putstr("file name only support lowcase! Sorry!\r\n");
		return -1;
	}

	do_fat_write(filename, addr, size);

	return 0;
}
