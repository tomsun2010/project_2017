AMB_DIR=$(pwd)/../ambarella
CHIP_NUMBER=hawthorn

source ${AMB_DIR}/build/env/armv7ahf-linaro-gcc.env

cd ${AMB_DIR}/boards/hawthorn



rm -rf ${AMB_DIR}/out/hawthorn
sync


cp -rf ${AMB_DIR}/../ipc/* ${AMB_DIR}/boards/${CHIP_NUMBER}/rootfs/default


cd ${AMB_DIR}/boards/${CHIP_NUMBER}

make amboot    

echo "zwz==== amboot first over,only for generating ver number!"

echo "##########################################"
echo "###  Generate Version Info #############"
echo "##########################################"
rm -rf  ${AMB_DIR}/boards/${CHIP_NUMBER}/rootfs/default/home/web/os-release
#TIME=`date +%Y%m%d%H%M00`
TIME=`cat "${AMB_DIR}/amboot/ver"`
IPC_VERSION=3.1.1_${TIME}
echo "zwz==== ${IPC_VERSION}"
echo "YUNYI_VERSION=${IPC_VERSION}" > ${AMB_DIR}/boards/${CHIP_NUMBER}/rootfs/default/home/web/os-release




make -j4



if [ ! -e ${AMB_DIR}/rootfs/cfg_jffs2.sh ]
then
	echo "@@@quit, cfg_jffs2.sh does not exist@@@"
        exit 3
else
        cp ${AMB_DIR}/rootfs/cfg_jffs2.sh                    	${AMB_DIR}/out/hawthorn/rootfs
fi

cd ${AMB_DIR}/out/hawthorn/rootfs
./cfg_jffs2.sh






echo "##########################################"
echo "###  Start to build bin package ##########"
echo "##########################################"

cd ${AMB_DIR}/boards/hawthorn

make amboot

echo "######### build completed ################"

echo "${TIME}" > ${AMB_DIR}/out/hawthorn/images/time.info
chmod 0777 ${AMB_DIR}/out/hawthorn/images/time.info

