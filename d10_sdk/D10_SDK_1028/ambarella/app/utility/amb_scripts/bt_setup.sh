#!/bin/sh
work=$1
mac=$2

usages()
{
  echo "usage: $0 [run|add|del] <mac addr>"
  echo "example:"
  echo "        bt_setup.sh run --this means run bt app"
  echo "        bt_setup.sh add --this means wait a new remote device matching it"
  echo "        bt_setup.sh del xx:xx:xx:xx:xx:xx --this means delete a matched device from it" 
  exit 2
}

if [ $# -eq 0 ]; then
usages
fi

load=$(lsmod | grep "bluetooth" | grep "bnep" )

if [[ "$load" == "" ]]; then

brcm_patchram_plus_fake_open /dev/ttyS1
brcm_patchram --tosleep=200000 --no2bytes --enable_hci --baudrate 1500000 \
--patchram /lib/firmware/broadcom/ap6255/BCM4345C0.hcd /dev/ttyS1 &
sleep 15
hciconfig hci0 up
sleep 1
systemctl daemon-reload
systemctl start bluetooth
systemctl enable bluetooth

fi

if [ "$work"x = "add"x ]; then

/usr/bin/expect << EOF
set timeout 20
spawn bluetoothctl
expect "*Ambarella*" { send "power on\r" }
expect "*Changing power on succeeded*" { send "discoverable on\r" }
expect "*Changing discoverable on succeeded*" { send "agent on\r" }
expect "*Agent registered*" { send "default-agent\r" }
expect "*Default agent request successful*" {
                        expect "*yes/no*" { send "yes\r" }
                        expect "Paired: yes" { sleep 2;send "quit\r"}
                }
expect eof
EOF

elif [ "$work"x = "del"x ]; then

if [ "$mac"x = ""x ]; then
echo "mac canot be empty!\r"
exit 2
fi

/usr/bin/expect << EOF
set timeout 20
spawn bluetoothctl                                                 
expect "*Ambarella*" { send "remove $mac\r" }                         
expect {
        "*Device has been removed*" { send "quit\r" }
        "*not available*" {  send_user "  mac is error\r";sleep 1; send "quit\r" }
       }
expect eof                                                     
EOF

elif [ "$work"x = "run"x ]; then

/usr/bin/expect << EOF
set timeout 20
spawn bluetoothctl         
expect "*Ambarella*" { send "power on\r" }
expect "*Changing power on succeeded*" { send "discoverable on\r" }
expect "*Changing discoverable on succeeded*" { send "quit\r" }
expect eof 
EOF

fi

if [[ "$load" == "" ]]; then
sdptool add --channel=22 SP
fi

if [ "$work"x = "run"x ]; then
/usr/local/bin/test_bt
fi
