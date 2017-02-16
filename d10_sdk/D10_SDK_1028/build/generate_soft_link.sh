AMB_DIR=$(pwd)/../ambarella
CHIP_NUMBER=hawthorn

cd ${AMB_DIR}/../ipc/usr/lib
ln -s libmosquitto.so libmosquitto.so.1
ln -s libmp4v2.so libmp4v2.so.2
ln -s libexpat.so libexpat.so.0
ln -s libapr-1.so libapr-1.so.0
ln -s libaprutil-1.so libaprutil-1.so.0
ln -s libssl.so libssl.so.1.0.0
ln -s libcrypto.so libcrypto.so.1.0.0
ln -s liboss_c_sdk.so liboss_c_sdk.so.1.0.0



