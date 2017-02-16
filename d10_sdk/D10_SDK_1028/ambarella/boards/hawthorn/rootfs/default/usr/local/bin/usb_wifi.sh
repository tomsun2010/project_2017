modprobe ehci-hcd
modprobe ambarella_udc
modprobe g_mass_storage file=/dev/mmcblk0p1 stall=0 removable=1
echo device > /proc/ambarella/usbphy0
sleep 1s
/bak/usr/local/bin/amba_debug -g 26 -d 0x1
#DISK=`dmesg | tail -n 50 | grep "Linux File-Backed Storage" | wc -l`
sleep 2s
DISK=`cat /proc/ambarella/udc_usb | grep "=connected" | wc -l`

#cnt=0
#while [ ${cnt} -le 6 ]
#do
#	usleep 500000
#
#	DISK=`cat /proc/ambarella/udc_usb | grep "=connected" | wc -l` 
#	
#	if [ ${DISK} = 1 ]
#	then
#		break
#	fi
#		
#	cnt=`expr ${cnt} + 1`
#
#	echo ${cnt}
#done

#echo "#####################################"
#echo "#####USB STORAGE STATE=${DISK}#######"
#echo "#####################################"

if [ ${DISK} = 1 ]
then
	echo "######## PC CONNECTED #########"
	exit 1
else
	echo "######## wifi ########"
	echo host > /proc/ambarella/usbphy0
	modprobe mlan
	#hardware4.0
	modprobe usb8801
	#hardware3.0
	#modprobe usb8801 cal_data_cfg=mrvl/WlanCalData_ext.conf
	#modprobe usb8801 drv_mode=1 mfg_mode=1
	sleep 2
	ifconfig mlan0 up
	/usr/sbin/set_mac /tmp/productionread.inf

	echo "######## set wifi wmm #######" 
	/lib/firmware/mrvl/mlanutl mlan0 wmmparamcfg 0 2 3 2 150  1 2 3 2 150  2 2 3 2 150  3 2 3 2 150
	/lib/firmware/mrvl/mlanutl mlan0 macctrl 0x13
	/lib/firmware/mrvl/mlanutl mlan0 psmode 0

	echo "######## set wifi countrycode ########"
	/lib/firmware/mrvl/mlanutl mlan0 countrycode CN	

	exit 2
fi




