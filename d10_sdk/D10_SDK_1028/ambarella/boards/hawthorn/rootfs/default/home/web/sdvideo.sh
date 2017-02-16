test_image -i 0 &
test_encode -i 0 -V480i --cvbs --raw-capture 1
cd /sdcard
test_stream -f ./ &
test_encode -A -h1080p --bitrate 500000 -e
