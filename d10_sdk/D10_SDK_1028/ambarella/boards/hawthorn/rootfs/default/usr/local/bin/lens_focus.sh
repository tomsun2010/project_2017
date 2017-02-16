/bak/usr/local/bin/usb2eth.sh
/bak/usr/local/bin/init.sh --na && modprobe mn34220pl
/bak/usr/local/bin/test_lens_focus &
/bak/usr/local/bin/test_encode -i 1080p -f 25 --enc-mode 4 --hdr-expo 2 --hdr-mode 1 -J --btype off -K --btype off -X --bmaxsize 1920x1080 --bsize 1920x1080 -Y --bmaxsize 640x360 --bsize 640x360 -A -h 1920x1080 --bitrate 1200000 --smaxsize 1920x1080 -B -m 640x360 --smaxsize 640x360 --lens-warp 1 --max-padding-width 256
/bak/usr/local/bin/rtsp_server &
/bak/usr/local/bin/test_encode -A -h 1080p -e --bitrate 6000000
