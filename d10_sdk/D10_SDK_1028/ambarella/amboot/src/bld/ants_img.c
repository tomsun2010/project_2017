#include <amboot.h>
#include <bldfunc.h>
#include <ambhw/vic.h>
#include <ambhw/uart.h>
#include <ambhw/nand.h>
#include <ambhw/spinor.h>
#include <libfdt.h>
#include <sdmmc.h>
#include <flash/flash.h>

#define     UPGRADE_IMAGE_MAGIC           "ANTSIMG"
#define		UPGRADE_IMAGE_VER_STR 		  "3.1.1_"

#define     BYTES_ALIGN_64              (64)

#define        KERNEL_BOOT_ADDR            (0x00208000)
#define        IMAGE_MAGIC_LEN                (8)
#define		   IMAGE_VER_LEN				(6)	
#define        UPGRADE_FILE_NAME_LEN        (64)

#define        PAGE_ALIGN_ADDR                (100*1024*1024)
#define        PAGE_ALICN_AREA                (60*1024*1024)
#define		OLD_VERSION					(20160318)
#define		OLD_VERSION_STR				(20160407)


typedef struct  upgrade_image_head_s
{
    unsigned char magic_num[IMAGE_MAGIC_LEN];  // contain  /0
    unsigned int  ver_num;
    unsigned int  ver_date;
    unsigned int  image_head_size;
    unsigned int  payload_head_num;
    unsigned int  payload_head_size;
    unsigned int  payload_data_offset;
    unsigned int  payload_data_size;
    unsigned int  dtb_flag;
	unsigned int  crc;
	unsigned char ver_str[12];
	unsigned int  reserved[50];

}upgrade_image_head_t;

typedef struct upgrade_payload_head_s
{
    unsigned int  flash_addr;
    unsigned int  flash_offset;
    unsigned int  flash_type;
    unsigned char file_name[UPGRADE_FILE_NAME_LEN];
    unsigned int  file_offset;
    unsigned int  file_size;
    unsigned int  file_type;
    unsigned int  flag;
    unsigned int  crc;
    unsigned char reserved[160];
}upgrade_payload_head_t;

typedef struct upgrade_payload_file_s
{
    upgrade_payload_head_t  head;
    char                    name[UPGRADE_FILE_NAME_LEN];
}upgrade_payload_file_t;



#if 0
static u32 get_bst_id(void)
{
    u32 bst_id = 0;

#if defined(BST_IMG_ID)
    return BST_IMG_ID;
#endif

#if defined(BST_SEL0_GPIO)
    gpio_config_sw_in(BST_SEL0_GPIO);
    bst_id = gpio_get(BST_SEL0_GPIO);
#endif

#if defined(BST_SEL1_GPIO)
    gpio_config_sw_in(BST_SEL1_GPIO);
    bst_id |= gpio_get(BST_SEL1_GPIO) << 1;
#endif

#if defined(BST_SEL0_GPIO) || defined(BST_SEL1_GPIO)
    putstr("BST ID = ");
    putdec(bst_id);
    putstr("\r\n");
#endif

    return bst_id;
}

static int select_bst_fw(u8 *img_in, int len_in, u8 **img_out, int *len_out)
{
    int bst_id, fw_size;

    bst_id = get_bst_id();
    fw_size = AMBOOT_BST_FIXED_SIZE + sizeof(partimg_header_t);

    /* Sanity check */
    if (len_in % fw_size != 0 || (bst_id + 1) * fw_size > len_in ) {
        putstr("invalid bst_id or firmware length: ");
        putdec(bst_id);
        putstr(", ");
        putdec(len_in);
        putstr("(");
        putdec(fw_size);
        putstr(")\r\n");
        return FLPROG_ERR_LENGTH;
    }

    *img_out = img_in + fw_size * bst_id;
    *len_out = fw_size;

    return 0;
}
#endif

extern unsigned char __sys_stack_start[];
extern unsigned char __sys_stack_end[];

int    image_parse_and_erase(u8* image_addr, u32 old_date, u32 old_ver)
{
    int ret_val = 0;
//    u32 boot_from;
//    u32 part_dev;
//    u8 code;
    u8 *image;
//    int part_len[HAS_IMG_PARTS];
//    int begin_image[HAS_IMG_PARTS];
//    u8 buffer[512];
    int i;
    flpart_table_t ptb;
    int    part_num;
    int    part_nums;
    int    dtb=0;
    void*  fdt;
    int    fdt_offset=0;
    int    fdt_length=0;
	int    err_cnt=0;
	int    ver_date=0;
	int    ver_num=0;

	int    no_check_header=0;

	unsigned int tmp_crc32;
	unsigned int raw_crc32;
  
    int part_to_burn[HAS_IMG_PARTS];
    int image_offset[HAS_IMG_PARTS];
    int image_length[HAS_IMG_PARTS];

    partimg_header_t        part_headers[HAS_IMG_PARTS];

    partimg_header_t*         part_header_ptr;

    upgrade_image_head_t*     img_header_ptr;
    upgrade_payload_head_t* img_payload_head_ptr;

    img_header_ptr=(upgrade_image_head_t*)image_addr;

    putstr("image addr=0x");
    puthex((u32)image_addr);
    putstr("\r\n");

    putstr("stack top addr=0x");
    puthex((u32)__sys_stack_start);
    putstr("\r\n");

    putstr("stack bottom addr=0x");
    puthex((u32)__sys_stack_end);
    putstr("\r\n");
    
    putstr("image magic=");
    for(i=0; i<IMAGE_MAGIC_LEN; i++)
    {
        putchar(img_header_ptr->magic_num[i]);
    }
    putstr("\r\n");
    
    if(strncmp((const char*)img_header_ptr->magic_num, UPGRADE_IMAGE_MAGIC, IMAGE_MAGIC_LEN-1) != 0)
    {
        putstr("image is invalid, exit\r\n");
        return    -1;
    }

	if(img_header_ptr->ver_date > OLD_VERSION_STR)
	{
		putstr("img string check\r\n");
		if(strncmp((const char*)img_header_ptr->ver_str, UPGRADE_IMAGE_VER_STR, IMAGE_VER_LEN) != 0)
		{
			putstr("img string is invalide, exit\r\n");
			return    -1;
		}
	}

    putstr("old ver: \r\n");
    putdec(old_date);
    putstr("@");
    putdec(old_ver);
    putstr("\r\nnew ver: \r\n");
    putdec(img_header_ptr->ver_date);
    putstr("@");
    putdec(img_header_ptr->ver_num);
    putstr("\r\n");

	ver_date	=img_header_ptr->ver_date;
	ver_num		=img_header_ptr->ver_num;
	
    if((old_date == img_header_ptr->ver_date) && (old_ver == img_header_ptr->ver_num))
    {
        putstr("same version, exit\r\n");
        return  -1;
    }

	if(ver_date < OLD_VERSION)
	{
		no_check_header=1;
		putstr("!!!no check header\r\n");
	}
	else
	{
		no_check_header=0;
	}
    #if 0
    putstr("image ver date=");
    puthex(img_header_ptr->ver_date);
    putstr("\r\n");
    
    putstr("image ver num=");
    puthex(img_header_ptr->ver_num);
    putstr("\r\n");
    #endif

    putstr("part total num=");
    puthex(img_header_ptr->payload_head_num);
    putstr("\r\n");

    part_nums=img_header_ptr->payload_head_num;

    if(part_nums>HAS_IMG_PARTS+1 || part_nums==0)
    {
        putstr("image has too many or no parts, exit\r\n");
        return  -1;
    }

	//we need to verify ourselves
	tmp_crc32=img_header_ptr->crc;
	img_header_ptr->crc=0;

	raw_crc32=crc32((u8 *)(image_addr), img_header_ptr->image_head_size + sizeof(upgrade_payload_head_t)*part_nums);

	if(no_check_header == 0)
	{
		if(tmp_crc32 != raw_crc32)
		{
			putstr("#img header crc32 failed\r\n");
			goto _CRC32_ERROR;
		}
		else
		{
			putstr("@img header crc32 verified\r\n");
		}
	}

    if(img_header_ptr->dtb_flag)
    {
        putstr("we find dtb info\r\n");
        part_nums-=1;
        dtb=1;
        putstr("valid part total num=");
        puthex(part_nums);
        putstr("\r\n");

        img_payload_head_ptr=(upgrade_payload_head_t*)(image_addr + img_header_ptr->image_head_size + sizeof(upgrade_payload_head_t)*part_nums);
 
        fdt_offset=img_payload_head_ptr->file_offset;
        fdt_length=img_payload_head_ptr->file_size;

		//we need to verify crc32 of dtb, we has no other good position to put the code.
		raw_crc32 = crc32((u8 *)(image_addr + fdt_offset), fdt_length);
		if(img_payload_head_ptr->crc != raw_crc32)
		{

			putstr("#dtb crc32 failed!\r\n");
			tmp_crc32=img_payload_head_ptr->crc;

			goto _CRC32_ERROR;

		}
		else
		{
			putstr("@dtb crc32 verified\r\n");
		}
    }


    img_payload_head_ptr=(upgrade_payload_head_t*)(image_addr + img_header_ptr->image_head_size);

    memzero(part_to_burn, sizeof(part_to_burn));
    memzero(part_headers, sizeof(part_headers));


    for(i=0; i<part_nums; i++)
    {
        part_num=(img_payload_head_ptr+i)->flash_addr;
        putstr("part num=");
        puthex(part_num);
        putstr("\r\n");

        part_to_burn[part_num]=1;
    
        image_offset[part_num]=(img_payload_head_ptr+i)->file_offset;
        image_length[part_num]=(img_payload_head_ptr+i)->file_size;

        putstr("image offset=");
        puthex(image_offset[part_num]);
        putstr("\r\n");

        putstr("image length=");
        puthex(image_length[part_num]);
        putstr("\r\n");

        //save part header
        part_headers[part_num].crc32=(img_payload_head_ptr+i)->crc;
        part_headers[part_num].ver_num=img_header_ptr->ver_num;
        part_headers[part_num].ver_date=img_header_ptr->ver_date;
        part_headers[part_num].img_len=(img_payload_head_ptr+i)->file_size;
        if(part_num==PART_PRI) //pri kernel partition
        {
            part_headers[part_num].mem_addr=KERNEL_BOOT_ADDR;
            part_headers[part_num].flag=PART_LOAD;
        }
#ifdef AMBA_SDK_XIAOYI_ALTER
        else if(part_num == PART_BAK) //bak kernel partition
        {
            part_headers[part_num].mem_addr=KERNEL_BOOT_ADDR;
            part_headers[part_num].flag=PART_LOAD;
        } 
#endif        
        else if(part_num==PART_BLD)
        {
            part_headers[part_num].mem_addr=0x0;
            part_headers[part_num].flag=PART_LOAD;
        }
        else
        {
            part_headers[part_num].mem_addr=0x0;
            part_headers[part_num].flag=PART_NO_LOAD;
        }

        part_headers[part_num].magic=PARTHD_MAGIC;
    }


	//at first, we verify crc32 of partitions
    for (i = PART_BST; i < HAS_IMG_PARTS; i++) 
    {
		if (part_to_burn[i] > 0) 
        {
        	raw_crc32 = crc32((u8 *)(image_addr + image_offset[i]), image_length[i]);
			
			if(part_headers[i].crc32!=raw_crc32)
			{
				tmp_crc32=part_headers[i].crc32;

				putstr(get_part_str(i));
				putstr("#partition crc32 failed!\r\n");

				goto _CRC32_ERROR;
			}
			else
			{
				putstr(get_part_str(i));
				putstr("@parition ");

				putstr("0x");
				puthex(raw_crc32);
				putstr(" == 0x");
				puthex(part_headers[i].crc32);
				putstr(" crc32 verified\r\n");
			}
		}
    }


    for (i = PART_BST; i < HAS_IMG_PARTS; i++) 
    {
        if (part_to_burn[i] > 0) 
        {
            putstr("\r\n");
            putstr(get_part_str(i));
            putstr(" code found in firmware!\r\n");

            #if 0
            image = (u8 *)(image_addr + image_offset[i] - sizeof(partimg_header_t));
            part_header_ptr=(partimg_header_t*)image;
            //add part header
            memcpy((void*)part_header_ptr, (const void*)&part_headers[i], sizeof(partimg_header_t));

            memzero((void*)PAGE_ALIGN_ADDR, PAGE_ALICN_AREA);
            memcpy((u8*)PAGE_ALIGN_ADDR, image, image_length[i] + sizeof(partimg_header_t));

            #if 1
            if (i == PART_BST) 
            {
                ret_val = select_bst_fw(image, image_length[i],&image, &image_length[i]);
                if (ret_val < 0)
                    goto select_bst_err;
            }

            ret_val = flprog_write_partition(i, (u8*)PAGE_ALIGN_ADDR, image_length[i]);
            #endif
            #endif

            #if 1
            image = (u8 *)(image_addr + image_offset[i]);
            //part_header_ptr=(partimg_header_t*)image;
            //add part header
            //memcpy((void*)part_header_ptr, (const void*)&part_headers[i], sizeof(partimg_header_t));
            if(((u32)image)% BYTES_ALIGN_64 == 0)
            {
                putstr("image has been aligned\r\n");
                image = (u8 *)(image_addr + image_offset[i] - sizeof(partimg_header_t));
                part_header_ptr=(partimg_header_t*)image;
                //add part header
                memcpy((void*)part_header_ptr, (const void*)&part_headers[i], sizeof(partimg_header_t));
            }
            else
            {
                putstr("no align, we align manually\r\n");
                memzero((void*)PAGE_ALIGN_ADDR, PAGE_ALICN_AREA);
                memcpy((u8*)PAGE_ALIGN_ADDR + sizeof(partimg_header_t), image, image_length[i]);
                memcpy((u8*)PAGE_ALIGN_ADDR, (const void*)&part_headers[i], sizeof(partimg_header_t));
                image=(u8*)PAGE_ALIGN_ADDR;
            }

            output_header((partimg_header_t*)image , 0);
            
            #if 1

            /*
            if (i == PART_BST) 
            {
                ret_val = select_bst_fw(image, image_length[i],&image, &image_length[i]);
                if (ret_val < 0)
                    goto select_bst_err;
            }
                    */
	
            validate_image_disable();
            flash_erase_led_init();
            ret_val = flprog_write_partition(i, image, image_length[i]);
            flash_erase_led_exit();
            #endif
            #endif

     
//select_bst_err:

            //image = (u8 *)(image_addr + image_offset[i]);
            #if 0
            image=(u8*)0x200000;

            my_flprog_write_partition_nand(i,image,image_length[i]);
            #endif
            
            output_failure(ret_val);

			if(ret_val<0)
			{
				err_cnt++;
			}
        }
        
    }    

    if(dtb && fdt_offset>0 && (fdt_offset%BYTES_ALIGN_64)==0 && fdt_length>0)
    {
        putstr("we start to upgrade dtb......\r\n");
        fdt = (void*)(image_addr + fdt_offset);

		if (fdt && fdt_check_header(fdt) == 0) 
        {
			putstr("\r\nDTB found in firmware!\r\n");
			fdt_update_cmdline(fdt, CONFIG_AMBOOT_BD_CMDLINE);
			ret_val = flprog_set_dtb(fdt, fdt_length, 1);
			output_failure(ret_val);

			if(ret_val<0)
			{
				err_cnt++;
			}
		} 
        else 
        {
			putstr("\r\nInvalid DTB!\r\n");
            return  -1;
		}
    }

	putstr("err_cnt=");
	puthex(err_cnt);
	putstr("\r\n");

	if(err_cnt==0)
	{
		ret_val = flprog_get_part_table(&ptb);
		if(ret_val<0)
		{
			return 	-1;
		}

		
		for (i = PART_BST; i < HAS_IMG_PARTS; i++) 
		{
			if (part_to_burn[i] > 0) 
			{
				ptb.part[i].ver_date=ver_date;
	        	ptb.part[i].ver_num=ver_num;
			}
		}

		ptb.part[0].ver_date=ver_date;
		ptb.part[0].ver_num=ver_num;

		//we only support on-line update rollback function
		ptb.part[1].ver_date=ver_date;
		ptb.part[1].ver_num=ver_num;
		
		ret_val=flprog_set_part_table(&ptb);

		if(ret_val<0)
		{
			return	-1;
		}
	}
	else
	{
		return -1;
	}
	
	putstr("ver update succeed\r\n");
	
    return    0;

_CRC32_ERROR:

	putstr("0x");
	puthex(raw_crc32);
	putstr(" != 0x");
	puthex(tmp_crc32);
	putstr(" crc32 verified failed!\r\n");

	return 	 -1;
}
