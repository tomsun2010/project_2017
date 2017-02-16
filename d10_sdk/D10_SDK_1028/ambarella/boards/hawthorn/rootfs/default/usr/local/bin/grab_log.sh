#!/bin/sh
MODE=$1
LOG_PATH=/sdcard/log/
LOG_DSP_PATH=/sdcard/log/dsp/
LOG_TAR_NAME=/sdcard/ipc_log_`date +%Y%m%d%H%M`.tar.gz

LOG_TMP_PATH=/tmp/upload_log/
LOG_TMP_DSP_PATH=/tmp/upload_log/dsp/

########### function ##############
usage()
{
    echo "usage: $0  [cat|tmp logname|save]"
}

cat_sys_log()
{
    date
    uptime
    cat /tmp/productionread.inf
    cat /home/web/os-release
    md5sum /home/web/ipc
    df
    top -m -n 2 &
    free &
    mount
    ls -hl /dev/
    ls -hl /home/ptrace/
    cat /home/ptrace/stack
    cat /home/ptrace/maps
}

cat_app_log()
{
    ls -hl /tmp/log/
    cat /tmp/log/app_dbg.log
    cat /tmp/log/ipc.log.0
    cat /tmp/log/ipc.log.1
    cat /tmp/log/miio_client.log.0
    cat /tmp/log/miio_client.log.1
}

cat_upgrade_log()
{
    ls -hl /mnt/cfg/log
    cat /mnt/cfg/log/upgrade_success.count
    cat /mnt/cfg/log/upgrade_fail.count
}

save_sys_log_dsp()
{
	cat /proc/interrupts > $LOG_DSP_PATH/interrupt.txt
	sleep 1
	cat /proc/interrupts >> $LOG_DSP_PATH/interrupt.txt
	dmesg > $LOG_DSP_PATH/dmesg.txt
	lsmod > $LOG_DSP_PATH/lsmod.txt 
	amba_debug -r 0x80000 -s 0x20000 -f $LOG_DSP_PATH/dsplog_ambadebug.bin
	amba_debug -w 0xec118000 -d 0x1000 && amba_debug -r 0xec110000 -s 0x120 > $LOG_DSP_PATH/vin_register.txt 
	amba_debug -r 0xec160020 -s 0x20 > $LOG_DSP_PATH/0xec160020_0x20.txt 
	amba_debug -r 0xec160200 -s 0x200 > $LOG_DSP_PATH/0xec160200_0x200.txt 
	amba_debug -r 0xec150028 -s 0x20 > $LOG_DSP_PATH/0xec150028_0x20.txt 
	amba_debug -r 0xec101c00 -s 0x80 > $LOG_DSP_PATH/0xec101c00_0x80.txt	
	cat /proc/ambarella/iav > $LOG_DSP_PATH/iav.txt
	cat /proc/ambarella/clock > $LOG_DSP_PATH/clock.txt
	cat /proc/meminfo > $LOG_DSP_PATH/meminfo.txt
	free > $LOG_DSP_PATH/free.txt & 
	top -m -n 1 > $LOG_DSP_PATH/top.txt &
	lsmod > $LOG_DSP_PATH/lsmod.txt 
	test_encode --show-a > $LOG_DSP_PATH/test_encode_show_a.txt &
	load_ucode /lib/firmware/ > $LOG_DSP_PATH/ucode.txt
}

save_sys_log_sd()
{
    date >> $LOG_PATH/sys.log 
    uptime >> $LOG_PATH/sys.log
    cat /tmp/productionread.inf >> $LOG_PATH/sys.log
    md5sum /home/web/ipc >> $LOG_PATH/sys.log
    cat /home/web/os-release >> $LOG_PATH/sys.log
    df >> $LOG_PATH/sys.log
    top -m -n 2 >> $LOG_PATH/sys.log &
    free >> $LOG_PATH/sys.log &
    mount >> $LOG_PATH/sys.log
    ls -hl /dev/ >> $LOG_PATH/sys.log
    ls -hl /home/ptrace/ >> $LOG_PATH/sys.log
    cat /home/ptrace/stack >> $LOG_PATH/sys.log
    cat /home/ptrace/maps >> $LOG_PATH/sys.log

}

save_app_log_sd()
{
    ls -hl /tmp/log/ >> $LOG_PATH/app.log
    cp -rf /tmp/log/ $LOG_PATH/app_log/
    cp /mnt/cfg/ipc_config/ipc.config $LOG_PATH/app_log/
    sed -i '/passwd=*/d' $LOG_PATH/app_log/ipc.config
}

save_upgrade_log_sd()
{
    ls -hl /mnt/cfg/log >> $LOG_PATH/upgrade.log
    cat /mnt/cfg/log/upgrade_success.count >> $LOG_PATH/upgrade.log
    cat /mnt/cfg/log/upgrade_fail.count >> $LOG_PATH/upgrade.log
}

if [ $# -lt 1 ]; then
    usage
    exit 1
fi

if [ $MODE == "cat" ]; then
    cat_sys_log
    cat_app_log
    cat_upgrade_log
fi

if [ $MODE == "save" ]; then
    rm -rf     $LOG_PATH
    mkdir -p $LOG_PATH
    mkdir -p $LOG_DSP_PATH
    save_sys_log_sd
    save_sys_log_dsp
    save_app_log_sd
    save_upgrade_log_sd
    cd /sdcard
    tar zcvf  $LOG_TAR_NAME log/*     
    rm -rf    $LOG_PATH
fi

if [ $MODE == "tmp" ]; then
    if [ $# -lt 2 ]; then
        usage
        exit 1
    fi
    
    LOG_PATH=$LOG_TMP_PATH
    LOG_DSP_PATH=$LOG_TMP_DSP_PATH
    LOG_TAR_NAME=$2.tmp

    rm -rf $LOG_PATH
    rm -rf /tmp/*.tar.gz*
    mkdir -p $LOG_PATH
    mkdir -p $LOG_DSP_PATH
    save_sys_log_sd
    save_sys_log_dsp
    save_app_log_sd
    save_upgrade_log_sd
    cd /tmp/
    tar zcf $LOG_TAR_NAME upload_log/*
    mv $LOG_TAR_NAME $2
    rm -rf $LOG_PATH
fi


