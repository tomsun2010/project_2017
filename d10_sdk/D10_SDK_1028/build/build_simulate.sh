AMB_DIR=$(pwd)/../ambarella

#source ${AMB_DIR}/build/env/Linaro-multilib-gcc4.9.env
#cd ${AMB_DIR}/boards/hawthorn
#make sync_build_mkcfg
#make s2l_ipcam_d10_simulate_config

cp -rf ${AMB_DIR}/../ipc/* ${AMB_DIR}/boards/hawthorn/rootfs/default

cd ${AMB_DIR}/boards/hawthorn
source ../../build/env/armv7ahf-linaro-gcc.env
make clean
make distclean
make sync_build_mkcfg
make s2l_ipcam_d10_simulate_config
make -j8
