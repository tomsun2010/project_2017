aplay -l
arecord -d 10 -f dat /sdcard/rec.wav -vv
amba_debug -g 49 -d 1
amba_debug -g 50 -d 1
aplay /sdcard/rec.wav -vv
