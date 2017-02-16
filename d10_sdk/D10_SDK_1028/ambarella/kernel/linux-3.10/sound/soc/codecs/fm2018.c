/*
 * sound/soc/ambarella/fm2018.c
 *
  * History:
 *	2014/04/17 - [Ken He] Created file
 *
 * Copyright (C) 2014-2018, Ambarella, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/skbuff.h>
#include <linux/workqueue.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <plat/audio.h>
#include <linux/proc_fs.h>

#define DRIVER_NAME "FM2018"


struct fm2018_data {
	unsigned int rst_pin;
	unsigned int rst_active;
	struct i2c_client	*client;
	struct device		*dev;
};

struct fm2018_workqueue{
	struct fm2018_data *data;
	struct work_struct download_binary;
};

static struct fm2018_workqueue fm2018_work;
static struct workqueue_struct *fm2018_wq;

static unsigned char fm2018dspPara[] = {
    0xFC, 0xF3, 0x3B, 0x1E, 0x30, 0x02, 0x33,
    0xFC, 0xF3, 0x3B, 0x1E, 0x34, 0x00, 0x65,
    0xFC, 0xF3, 0x3B, 0x1E, 0x36, 0x00, 0x1D,
    0xFC, 0xF3, 0x3B, 0x1E, 0x3D, 0x0A, 0x00,
    0xFC, 0xF3, 0x3B, 0x1E, 0x3E, 0x01, 0x01,
    0xFC, 0xF3, 0x3B, 0x1E, 0x41, 0x01, 0x01,
    0xFC, 0xF3, 0x3B, 0x1E, 0x44, 0x00, 0x01,
    0xFC, 0xF3, 0x3B, 0x1E, 0x45, 0x03, 0xCF,
    0xFC, 0xF3, 0x3B, 0x1E, 0x46, 0x00, 0x11,
    0xFC, 0xF3, 0x3B, 0x1E, 0x47, 0x15, 0x00,
    0xFC, 0xF3, 0x3B, 0x1E, 0x48, 0x10, 0x00,
    0xFC, 0xF3, 0x3B, 0x1E, 0x49, 0x10, 0x00,
    0xFC, 0xF3, 0x3B, 0x1E, 0x4D, 0x01, 0x00,
    0xFC, 0xF3, 0x3B, 0x1E, 0x52, 0x00, 0x13,
    0xFC, 0xF3, 0x3B, 0x1E, 0x58, 0x80, 0x02,
    0xFC, 0xF3, 0x3B, 0x1E, 0x60, 0x00, 0x00,
    0xFC, 0xF3, 0x3B, 0x1E, 0x63, 0x00, 0x01,
    0xFC, 0xF3, 0x3B, 0x1E, 0x70, 0x05, 0xC0,
    0xFC, 0xF3, 0x3B, 0x1E, 0x86, 0x00, 0x0B,
    0xFC, 0xF3, 0x3B, 0x1E, 0x87, 0x00, 0x01,
    0xFC, 0xF3, 0x3B, 0x1E, 0x88, 0x60, 0x00,
    0xFC, 0xF3, 0x3B, 0x1E, 0x89, 0x00, 0x01,
    0xFC, 0xF3, 0x3B, 0x1E, 0x8B, 0x00, 0x80,
    0xFC, 0xF3, 0x3B, 0x1E, 0x8C, 0x00, 0x10,
    0xFC, 0xF3, 0x3B, 0x1E, 0x90, 0x00, 0x09,
    0xFC, 0xF3, 0x3B, 0x1E, 0x91, 0x00, 0x04,
    0xFC, 0xF3, 0x3B, 0x1E, 0x92, 0x60, 0x00,
    0xFC, 0xF3, 0x3B, 0x1E, 0xA0, 0x20, 0x00,
    0xFC, 0xF3, 0x3B, 0x1E, 0xA1, 0x33, 0x00,
    0xFC, 0xF3, 0x3B, 0x1E, 0xA2, 0x32, 0x00,
    0xFC, 0xF3, 0x3B, 0x1E, 0xC0, 0x26, 0x80,
    0xFC, 0xF3, 0x3B, 0x1E, 0xC1, 0x10, 0x80,
    0xFC, 0xF3, 0x3B, 0x1E, 0xC5, 0x2B, 0x06,
    0xFC, 0xF3, 0x3B, 0x1E, 0xC6, 0x0C, 0x1F,
    0xFC, 0xF3, 0x3B, 0x1E, 0xC7, 0x03, 0x00,
    0xFC, 0xF3, 0x3B, 0x1E, 0xC8, 0x70, 0x00,
    0xFC, 0xF3, 0x3B, 0x1E, 0xC9, 0x75, 0xAB,
    0xFC, 0xF3, 0x3B, 0x1E, 0xCA, 0x70, 0x26,
    0xFC, 0xF3, 0x3B, 0x1E, 0xCB, 0x7F, 0xFF,
    0xFC, 0xF3, 0x3B, 0x1E, 0xCC, 0x7F, 0xFE,
    0xFC, 0xF3, 0x3B, 0x1E, 0xF8, 0x04, 0x00,
    0xFC, 0xF3, 0x3B, 0x1E, 0xF9, 0x01, 0x00,
    0xFC, 0xF3, 0x3B, 0x1E, 0xFF, 0x4B, 0x00,
    0xFC, 0xF3, 0x3B, 0x1F, 0x00, 0x7F, 0xFF,
    0xFC, 0xF3, 0x3B, 0x1F, 0x0A, 0x03, 0x00,
    0xFC, 0xF3, 0x3B, 0x1F, 0x0C, 0x01, 0x00,
    0xFC, 0xF3, 0x3B, 0x1F, 0x0D, 0x78, 0x00,
    0xFC, 0xF3, 0x3B, 0x1E, 0x3A, 0x00, 0x00,
};


#if 0
static int fm2018_read_reg(struct fm2018_data *fm2018, u8 reg)
{
	int ret = i2c_smbus_read_byte_data(fm2018->client, reg);
	if (ret > 0)
		ret &= 0xff;

	return ret;
}

static int fm2018_write_reg(struct fm2018_data *fm2018,
		u8 reg, u8 value)
{
	return i2c_smbus_write_byte_data(fm2018->client, reg, value);
}
#endif

static int i2c_write(struct i2c_client *client, char *buf, int len){
	if(len != i2c_master_send(client, buf, len)){
		printk("[FMDSP] i2c_write: i2c_master_send error\n");
		return -1;
	}
	return 0;
}


static int i2c_read(struct i2c_client *client, char *buf, int len){
	if(len != i2c_master_send(client, buf, len)){
		printk("[FMDSP] i2c_read: i2c_master_send error\n");
		return -1;
	}

	if(1 != i2c_master_recv(client, buf, 1)){
		printk("[FMDSP] i2c_read: i2c_master_recv error\n");
		buf[0] = 0xff;
		return -1;
	}
	return 0;
}

static int fm2018_write_reg(struct fm2018_data *fm2018, u16 reg, u32 data)
{
	int rval;
	struct i2c_client *client;
	struct i2c_msg msgs[1];
    u8 pbuf[] = {0xfc, 0xf3, 0x3b, 0x00, 0x00, 0x00, 0x00};

	client = fm2018->client;

	pbuf[3] = (reg & 0xff00) >> 8;
	pbuf[4] = reg & 0xff;
	pbuf[5] = (data & 0xff00) >> 8;
	pbuf[6] = data & 0xff;

	msgs[0].len = sizeof(pbuf);
	msgs[0].addr = client->addr;
	msgs[0].flags = client->flags;
	msgs[0].buf = pbuf;

	rval = i2c_transfer(client->adapter, msgs, 1);
	if (rval < 0) {
		printk("failed(%d): [0x%x:0x%x]\n", rval, reg, data);
		return rval;
	}

	return 0;
}

static int fm2018_read_reg(struct fm2018_data *fm2018, u16 reg, u16 *data)
{
	uint8_t wBuf[5];
	uint8_t rBuf[4];
	int dataH, dataL;
    uint8_t regH;
    uint8_t regL;
	struct i2c_client *client = fm2018->client;

    regH = (reg & 0xff00) >> 8;
    regL = reg & 0xff;

	wBuf[0]=0xfC;
	wBuf[1]=0xf3;
	wBuf[2]=0x37;
	wBuf[3]=regH;
	wBuf[4]=regL;
	i2c_write(client, wBuf, 5);

	// read hi byte data (register 0x26)
	rBuf[0]=0xfc;
	rBuf[1]=0xf3;
	rBuf[2]=0x60;
	rBuf[3]=0x26;
	i2c_read(client, rBuf, 4);
	dataH = rBuf[0];

	// read low byte data (register 0x25)
	rBuf[0]=0xfc;
	rBuf[1]=0xf3;
	rBuf[2]=0x60;
	rBuf[3]=0x25;
	i2c_read(client, rBuf, 4);
	dataL = rBuf[0];

    *data = (dataH << 8) + dataL;
    printk("reg: 0x%x, data: 0x%x\r\n", reg, *data);

	return 0;
}


#if 0
static int fm2018_read_reg(struct fm2018_data *fm2018, u16 reg, u16 *data)
{
	int rval = 0;
	struct i2c_client *client;
	struct i2c_msg msgs[1];
	u8 read_mem[] = {0xfc, 0xf3, 0x37, 0x00, 0x00};
    u8 get_hi[] = {0xfc, 0xf3, 0x37, 0x00, 0x00};
	//u8 pbuf[2] = {0};
	u16 val = 0;
    
	client = fm2018->client;

	read_mem[3] = (reg &0xff00) >> 8;
	read_mem[4] = reg & 0xff;

	msgs[0].len = sizeof(read_mem);
	msgs[0].addr = client->addr;
	msgs[0].flags = client->flags;
	msgs[0].buf = read_mem;

	rval = i2c_transfer(client->adapter, msgs, 1);
	if (rval < 0){
		printk("failed(%d): [0x%x]\n", rval, reg);
		return rval;
	}

	*data = val;

    printk("read i2c 0x%x is: 0x%x\r\n", reg, val);

	return 0;
}
#endif



static int fm2018_write_command(struct fm2018_data *fm2018, u8 *pbuf, unsigned int length)
{
	int ret;
	struct i2c_msg msg[1];
	struct i2c_client *client;
	client = fm2018->client;

	/*begin to encapsulate the message for i2c*/
	msg[0].addr = client->addr;
	msg[0].len = length;
	msg[0].flags = client->flags;
	msg[0].buf = pbuf;

	ret = i2c_transfer(client->adapter, msg, 1);
	udelay(100);
	if (ret < 0) {
		dev_err(&client->dev, "Failed writing para!\n");
		return ret;
	}
	return 0;
}

unsigned short fm2018_ram_read(struct fm2018_data *fm2018, char *sbuf, u8 slen, char *rbuf, u8 rlen){
	int ret;

	ret = i2c_smbus_read_i2c_block_data(fm2018->client, sbuf[0], rlen, rbuf);

	return 0;
}


static int fm2018_ram_download(struct fm2018_data *fm2018, u8 *data, int len)
{
	int ret;
    int timeout = 10;

	do 
    {   /*write Para to dsp */
		ret = fm2018_write_command(fm2018, data, len);
		if (ret != 0) 
        {
			pr_err(" write ram error in function  %s\n ",__FUNCTION__);
            timeout--;
		    udelay(15);
            continue;
		}
        else
        {
            return 0;
        }
	} while (timeout > 0);
    
	return -1;
}


#if 0
static int fm2018_ram_download_crc(struct fm2018_data *fm2018, u8 cmd, u8 *data, int len ,u16 crc_value)
{
	u16 crc_val = crc_value;
	u16 crc_flag = 1;
	char crc_tx[1] = { 0x72 };
	char crc_rx[2];
	int ret, ret1, timeout = crc_timeout; //timeout = 3; chane 3 times to 10 times

	do {	/*write PRAM or CRAM and get crc value*/
		ret1 = fm2018_write_command(fm2018, cmd, 0, data, len, &crc_val);
		if (ret1 != 0) {
			pr_err(" write ram error in function  %s\n ",__FUNCTION__);
			continue;
		}

		/*get the crc value by command 0x72*/
		 do {
			ret = fm2018_ram_read(fm2018, crc_tx, 1, crc_rx, 2);
			if(ret != 0)
				pr_err("%s:  read crc err\n",__FUNCTION__);
		 } while (ret != 0);

		ret  =  (crc_rx[0] << 8) + crc_rx[1];
		akdbgprt("pram : crc flag ret=%x \n" ,ret);
		crc_flag  =  (crc_val == ret);
		timeout--;
		udelay(10);
	} while (crc_flag == 0 && timeout > 0);

	akdbgprt("crc_flag = %d timeout = %d\n", crc_flag, timeout);
	if(timeout == 0)
		return -1;
	return 0;
}
#endif

static void fm2018_download_dsp_pro(struct fm2018_data *fm2018)
{
	int ret;
    u16 val = 0;

    #if 0
	if(aec == 1) {
		/* pram data is fm2018 dsp binary */
		ret = fm2018_ram_download_crc(fm2018, 0xB8, pram_data, ARRAY_SIZE(pram_data), 0xc4a9);
		if(ret < 0) {
			printk("fm2018:write dsp binary failed!!!\n");
		}
	} else if(aec == 0) {
		/* ak77dspPRAM is fm2018 bypass dsp binary */
		fm2018_ram_download_crc(fm2018, 0xB8, ak77dspPRAM, ARRAY_SIZE(ak77dspPRAM), 0x5bf4);
	}
    #endif

	ret = fm2018_ram_download(fm2018, fm2018dspPara, sizeof(fm2018dspPara));
	if(ret < 0)
		printk("fm2018:write para failed\n");
    
    msleep(50);
	fm2018_read_reg(fm2018, 0x1e3a, &val);
    printk("read 0x1e3a: 0x%x\r\n", val);

    return;
}

static void fm2018_download_work(struct work_struct *work)
{
	struct fm2018_workqueue *fm2018_work = container_of(work,struct fm2018_workqueue, download_binary);
	fm2018_download_dsp_pro(fm2018_work->data);
}

#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)

struct fm2018_data *gfm2018 = NULL;

static int convert(char buf[])
{
    char *pTmp = buf;
    int sum = 0;
    int tmp = 0;
    while (*pTmp)
    {
        if (*pTmp >= '0' && *pTmp <= '9')
        {
            tmp = *pTmp - '0';
        }
        else if (*pTmp >= 'a' && *pTmp <= 'f')
        {
            tmp = *pTmp - 'a' + 10;
        }
        else if (*pTmp >= 'A' && *pTmp <= 'F')
        {
            tmp = *pTmp - 'A' + 10;
        }
        else
        {
            printk("input error\r\n");
            return 0;
        }
        
        sum = sum * 16 + tmp; 
        pTmp++;
    }

    return sum;
}

static void parse(char buf[], char outargv[8][32], int *pargc)
{
    int count= 0;
    char *pbuf = buf;
    char *pout = outargv[0];

    while (*pbuf)
    {
        if (' ' == *pbuf)
        {
            count++;
            pbuf++;
            pout = outargv[count];
            continue;
        }

        *pout = *pbuf;
        pbuf++;
        pout++;
    }

    count++;
    *pargc = count;
    return;

}



static int ambarella_dsp_write(struct file *file,
	const char __user *buffer, size_t count, loff_t *ppos)
{
	char n, str[128] = {0};
    char argv[8][32];
    int argc = 0;
    u16 ret;
    int reg, value;

	n = (count < 128) ? count : 128;

	if (copy_from_user(str, buffer, n))
		return -EFAULT;

	str[n - 1] = '\0';

    memset(argv, 0, sizeof(argv));

    parse(str, argv, &argc);

	if (0 == strcmp("read", argv[0]))
    {
        fm2018_read_reg(gfm2018, (u16)convert(argv[1]), &ret);
        printk("0x%x\r\n", ret);
    }
    else if (0 == strcmp("write", argv[0]))
    {
        reg = convert(argv[1]);
        value = convert(argv[2]);
        printk("write 0x%x, value: 0x%x, %s %s\r\n", reg, value, argv[1], argv[2]);
        fm2018_write_reg(gfm2018, reg, value);
    }
    else
    {
        printk("not support\r\n");
    }

	return count;
}




static const struct file_operations dsp_op = {
	.write = ambarella_dsp_write,
};


static void fm2018_hw_init(void)
{
	gpio_set_value(51, 0);
	mdelay(10);
	gpio_set_value(48, 1);
//	gpio_set_value(dm_gpios.clk, 1);
	mdelay(5);
	gpio_set_value(50, 1);
	mdelay(30);
    
}
static int fm2018_i2c_probe(struct i2c_client *i2c,
			    const struct i2c_device_id *id)
{
	int rval = 0;
	enum of_gpio_flags flags;
	struct device_node *np = i2c->dev.of_node;
	struct fm2018_data *fm2018 = NULL;
	int rst_pin;

    struct proc_dir_entry *dir;
    

	dir = proc_mkdir("codec_dsp", NULL);
	if (!dir)
		return -ENOMEM;

    proc_create_data("dsp", S_IRUGO|S_IWUSR, dir, &dsp_op, NULL);

	fm2018 = devm_kzalloc(&i2c->dev, sizeof(struct fm2018_data), GFP_KERNEL);
	if (fm2018 == NULL) {
		printk("kzalloc for fm2018 is failed\n");
		return -ENOMEM;
	}

	rst_pin = of_get_gpio_flags(np, 0, &flags);
	if (rst_pin < 0 || !gpio_is_valid(rst_pin)) {
		printk("fm2018 rst pin is not working");
		rval = -ENXIO;
		goto out;
	}

	fm2018->rst_pin = rst_pin;
	fm2018->rst_active = !!(flags & OF_GPIO_ACTIVE_LOW);

	fm2018->client = i2c;
	i2c_set_clientdata(i2c, fm2018);
#if 0
	rval = devm_gpio_request(&i2c->dev, fm2018->rst_pin, "fm2018_reset");
	if (rval < 0){
		dev_err(&i2c->dev, "Failed to request rst_pin: %d\n", rval);
		goto out;
	}
    
    printk("AAAAAAAAAAA fm2018->rst_active: %d\r\n", fm2018->rst_active);
	gpio_direction_output(fm2018->rst_pin, !fm2018->rst_active);
	msleep(20);
#endif
#if 0
	fm2018_read_reg(fm2018, 0x50);
	fm2018_read_reg(fm2018, 0x51);
	fm2018_write_reg(fm2018, 0xd0, 0x1);
	fm2018_write_reg(fm2018, 0xd1, 0x1);
	fm2018_write_reg(fm2018, 0xc0, 0x20);
	fm2018_write_reg(fm2018, 0xc1, 0x30);
	fm2018_write_reg(fm2018, 0xc2, 0x64);
	fm2018_write_reg(fm2018, 0xc3, 0xb2);
	fm2018_write_reg(fm2018, 0xc4, 0xf0);
	fm2018_write_reg(fm2018, 0xc5, 0x0);
	fm2018_write_reg(fm2018, 0xc6, 0x0);
	//fm2018_write_reg(fm2018, 0xc7, 0x0);
	fm2018_write_reg(fm2018, 0xc8, 0x80);
//	fm2018_read_reg(fm2018, 0x50);
//	fm2018_read_reg(fm2018, 0x51);
//	fm2018_read_reg(fm2018, 0x47);

	fm2018_write_reg(fm2018, 0xc6, 0x20);
	msleep(1);
    #endif

    fm2018_hw_init();

    gfm2018 = fm2018;

	fm2018_wq = create_workqueue("fm2018_workqueue");
	fm2018_work.data = fm2018;
 	INIT_WORK(&(fm2018_work.download_binary), fm2018_download_work);
	queue_work(fm2018_wq, &(fm2018_work.download_binary));

    

	return 0;
out:
	devm_kfree(&i2c->dev, fm2018);
	return rval;
}

static int fm2018_i2c_remove(struct i2c_client *client)
{	struct fm2018_data *fm2018 = NULL;

	destroy_workqueue(fm2018_wq);
	fm2018_wq = NULL;
	fm2018_work.data = NULL;
	fm2018 = i2c_get_clientdata(client);
	devm_gpio_free(&client->dev, fm2018->rst_pin);
	devm_kfree(&client->dev, fm2018);

	return 0;
}

static const struct of_device_id fm2018_dt_ids[] = {
	{ .compatible = "ambarella,fm2018",},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, fm2018_dt_ids);

static const struct i2c_device_id fm2018_i2c_id[] = {
	{ "fm2018", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, fm2018_i2c_id);

static struct i2c_driver fm2018_i2c_driver = {
	.driver = {
		.name = "DRIVER_NAME",
		.owner = THIS_MODULE,
		.of_match_table = fm2018_dt_ids,
	},
	.probe		=	fm2018_i2c_probe,
	.remove		=	fm2018_i2c_remove,
	.id_table	=	fm2018_i2c_id,
};
#endif

static int __init fm2018_modinit(void)
{
	int ret;

#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)
	ret = i2c_add_driver(&fm2018_i2c_driver);
	if (ret != 0)
		pr_err("Failed to register AK7719 I2C driver: %d\n", ret);
#endif
	return ret;
}
module_init(fm2018_modinit);
static void __exit fm2018_exit(void)
{
#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)
	i2c_del_driver(&fm2018_i2c_driver);
#endif
}
module_exit(fm2018_exit);

MODULE_DESCRIPTION("Amabrella Board AK7719 DSP for Audio Codec");
MODULE_AUTHOR("Ken He <jianhe@ambarella.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:"DRIVER_NAME);
