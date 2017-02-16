#!/bin/sh
usages()
{
 echo "usgaes: $0 [w|r]"
 echo "example:"
 echo "        w  --this means red to white"
 echo "      r  --this means white to red"
 exit 2
}

if [ $# -eq 0 ]; then
usages
fi

opmode=$1

echo "######## start switch ########"

if [ "$opmode"x = "w"x ]; then

amba_debug -g 24 -d 1
amba_debug -g 25 -d 0
sleep 0.06
amba_debug -g 24 -d 0
amba_debug -g 25 -d 0

elif [ "$opmode"x = "r"x ]; then


amba_debug -g 24 -d 0
amba_debug -g 25 -d 1
sleep 0.06
amba_debug -g 24 -d 0
amba_debug -g 25 -d 0

fi

echo "######## switch end ########"
