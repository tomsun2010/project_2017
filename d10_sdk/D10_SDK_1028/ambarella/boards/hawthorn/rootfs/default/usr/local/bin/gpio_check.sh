GPIO_DIR=/sys/class/gpio

GPIO_PIN1=31
GPIO_PIN2=32

pin1_value=1
pin2_value=1

echo ${GPIO_PIN1} > ${GPIO_DIR}/export
echo ${GPIO_PIN2} > ${GPIO_DIR}/export

if [ -f ${GPIO_DIR}/gpio${GPIO_PIN1}/direction ]
then
	echo in > ${GPIO_DIR}/gpio${GPIO_PIN1}/direction
	pin1_value=`cat ${GPIO_DIR}/gpio${GPIO_PIN1}/value`
fi

if [ -f ${GPIO_DIR}/gpio${GPIO_PIN2}/direction ]
then
	echo in > ${GPIO_DIR}/gpio${GPIO_PIN2}/direction
	pin2_value=`cat ${GPIO_DIR}/gpio${GPIO_PIN2}/value`
fi

echo "gpio31 level : ${pin1_value}"
echo "gpio32 level : ${pin2_value}"

if [ ${pin1_value} = 1 ] && [ ${pin2_value} = 1 ]
then
	echo "@@@normal boot@@@"
	exit 1
elif [ ${pin1_value} = 0 ] && [ ${pin2_value} = 1 ]
then
	echo "@@@focus mode@@@"
	exit 2
elif [ ${pin1_value} = 1 ] && [ ${pin2_value} = 0 ]
then
       echo "@@@wifi mode@@@"
       exit 3
elif [ ${pin1_value} =0 ] && [ ${pin2_value} = 0 ]
then
	echo "@@@reserved mode@@@"
	exit 4
else
	echo "###!!!!!!!!!!!!!###"
	exit 5
fi

