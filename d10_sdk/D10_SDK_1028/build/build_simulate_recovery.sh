AMB_DIR=$(pwd)/../ambarella

red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

function error()
{
    echo -e "$red $1 $reset"
}

function message()
{
    echo -e "$green $1 $reset"
}

#source ${AMB_DIR}/build/env/Linaro-multilib-gcc4.9.env
#cd ${AMB_DIR}/boards/hawthorn
#make sync_build_mkcfg
#make s2l_ipcam_d10_simulate_config

message $"prepare pba image"

rm -rf ${AMB_DIR}/boards/hawthorn/rootfs/default/bin
rm -rf ${AMB_DIR}/boards/hawthorn/rootfs/default/usr
rm -rf ${AMB_DIR}/boards/hawthorn/rootfs/default/home

cd ${AMB_DIR}/boards/hawthorn
source ../..//env/armv7ahf-linaro-gcc.env
make clean
make distclean
make sync_build_mkcfg
make s2l_ipcam_d10_simulate_cpio_config
make -j8

if [ ! -d ${AMB_DIR}/prebuild/sysimg/pba ]
then
	mkdir -p ${AMB_DIR}/prebuild/sysimg/pba
fi

if [ ! -e ${AMB_DIR}/out/hawthorn/kernel/zImage ]
then
    error $"error: zImage does not exist"
    exit 1
fi

cp ${AMB_DIR}/out/hawthorn/kernel/zImage ${AMB_DIR}/prebuild/sysimg/pba/S2PBA.bin

message $"build normal image"

cp -rf ${AMB_DIR}/../ipc/* ${AMB_DIR}/boards/hawthorn/rootfs/default

make clean
make s2l_ipcam_d10_simulate_pba_config
make -j8
