test_image -i 0 &
test_encode -i 0 -V480i --cvbs
rtsp_server &
test_encode -A -h1080p -e
