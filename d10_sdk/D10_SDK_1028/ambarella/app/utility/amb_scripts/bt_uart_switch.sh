#!/bin/sh
usages()
{
	  echo "usgaes: $0 [b2u|u2b]"
		    echo "example:"
			  echo "        switch b2u  --this means disable bluetooth, enable uart"
			    echo "        switch u2b  --this means disable uart, enable bluetooth"
				  exit 2
}

if [ $# -eq 0 ]; then
usages
fi

opmode=$1

echo "######## start switch ########"

if [ "$opmode"x = "b2u"x ]; then

devmem 0xE8016004 32 0x80000380

devmem 0xE80160F0 32 0x1
devmem 0xE80160F0 32 0x0


devmem 0xE801600C 32 0x00003F8C
devmem 0xE8016010 32 0xFFC0000C

devmem 0xE80160F0 32 0x1
devmem 0xE80160F0 32 0x0

elif [ "$opmode"x = "u2b"x ]; then

devmem 0xE8016004 32 0x80078380      
                                     
devmem 0xE80160F0 32 0x1             
devmem 0xE80160F0 32 0x0        
                    
                               
devmem 0xE801600C 32 0x00003F80
devmem 0xE8016010 32 0xFFC00000
                               
devmem 0xE80160F0 32 0x1       
devmem 0xE80160F0 32 0x0 

fi

echo "######## switch end ########"
