echo "#######################"
echo "This is first test: MFG"
echo "#######################"
modprobe ehci-hcd
echo host > /proc/ambarella/usbphy0
modprobe mlan
modprobe usb8801 drv_mode=1 mfg_mode=1 fw_name=mrvl/usb8801_uapsta_mfg.bin
sleep 2s
cd /bak/usr/script
./mfgbridge
