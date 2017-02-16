LD_LIBRARY_PATH=/lib:/usr/lib:
export LD_LIBRARY_PATH

PATH=\
/bin:\
/sbin:\
/usr/bin:\
/usr/sbin:\
/usr/bin/x11:\
/usr/local/bin:\
/usr/local/sbin

export PATH

######## audio #######
#modprobe snd-soc-core
#modprobe snd-soc-ambarella
#modprobe snd-soc-ambarella-i2s
#modprobe snd-soc-ambdummy
#modprobe snd_soc_alc5633
#modprobe snd-soc-amba-board
#modprobe fm2018

######## debug #######
modprobe ambad
	
######## emd   #######
#modprobe emd

####### set sd driver : 12mA #######
#/usr/local/bin/amba_debug -w 0xec17031c  -d  0xffffffff
#/usr/local/bin/amba_debug -w 0xec170320  -d  0xf9ffffff

#/usr/local/bin/amba_debug -w 0xec170324  -d  0xffffffff
#/usr/local/bin/amba_debug -w 0xec170328  -d  0xffffe1ff

####### mount sd ########
#mkdir -p /sdcard
#mount -t vfat -o rw /dev/mmcblk0p1 /sdcard

#/usr/local/bin/productioninfoget.sh
#/usr/sbin/get_cal_data

if [ -f /sdcard/test/factory_test.sh ]; then
#	/sdcard/test/factory_test.sh
	exit
fi

#/usr/local/bin/gpio_check.sh
value=$?
echo ${value}

if [ ${value} = 2 ]
then
	echo "###Focus Mode###"
#	/usr/local/bin/lens_focus.sh
	exit
elif [ ${value} = 3 ]
then
	echo "###Wifi Mode###"
#	/usr/local/bin/wifi_mfg.sh
	exit
else
	echo "###Normal Boot###"
fi

######## get config ######
mkdir -p /mnt/cfg
#/usr/sbin/ubiattach /dev/ubi_ctrl -m 8
#mount -t ubifs ubi2_1 /mnt/cfg
mount -t  jffs2  /dev/mtdblock6  /mnt/cfg 

######## wifi ########
#/usr/local/bin/usb_wifi.sh
value=$?
echo ${value}

if [ ${value} = 1 ]
then
      echo "@@@we do not launch ipc@@@"
      exit
fi
### 2018 by pass
#/usr/local/bin/amba_debug -g 51 -d 0x1

######## cryptography engine ########
#modprobe ambarella_crypto config_polling_mode=1
#modprobe ambac

######## ipc #########
#modprobe pwm_bl

#modprobe mn34220pl bus_addr=0x36
#modprobe mn34220pl
#/usr/local/bin/init.sh --na

#miio
#/usr/local/bin/mosquitto -c /etc/mosquitto.conf -d


#real init ipc
#/home/web/show_stack &
#/home/web/ipc -w 2>&1 | /home/web/logrunner ipc.log &


