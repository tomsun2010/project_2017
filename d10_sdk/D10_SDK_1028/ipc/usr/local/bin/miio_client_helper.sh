#!/bin/sh

MQTT_TOPIC_INTERNAL_MIIO_CLIENT_IN="_internal/miio_client_in"
WIFI_CONF_FILE="/mnt/cfg/miio/wifi.conf"
MQTT_RECV_LINE="/usr/local/bin/mqtt_recv_line"

# contains(string, substring)
#
# Returns 0 if the specified string contains the specified substring,
# otherwise returns 1.
contains() {
    string="$1"
    substring="$2"
    if test "${string#*$substring}" != "$string"
    then
        return 0    # $substring is in $string
    else
        return 1    # $substring is not in $string
    fi
}

send_helper_ready() {
    ready_msg="{\"method\":\"_internal.helper_ready\"}"
    echo $ready_msg
    /usr/local/bin/mosquitto_pub -h localhost  -t ${MQTT_TOPIC_INTERNAL_MIIO_CLIENT_IN} -m "$ready_msg"
}

req_wifi_conf_status() {
    REQ_WIFI_CONF_STATUS_RESPONSE=""
    if [ -e $WIFI_CONF_FILE ]; then
	REQ_WIFI_CONF_STATUS_RESPONSE="{\"method\":\"_internal.res_wifi_conf_status\",\"params\":1}"
    else
	REQ_WIFI_CONF_STATUS_RESPONSE="{\"method\":\"_internal.res_wifi_conf_status\",\"params\":0}"
    fi
}

main() {

    while true; do
	BUF=`$MQTT_RECV_LINE`
	if contains $BUF "_internal.info"; then
	    STRING=`wpa_cli status`

	    ifname=${STRING#*\'}
	    ifname=${ifname%%\'*}
	    echo "ifname: $ifname"

            rssi=`iwconfig mlan0 | grep "Signal level" | cut -d= -f3 | awk '{print $1}'`
            echo "rssi:$rssi"

	    ssid=`cat ${WIFI_CONF_FILE} | grep ssid | cut -d= -f2`
	    echo "ssid: $ssid"

	    bssid=${STRING##*bssid=}
	    bssid=`echo ${bssid} | cut -d ' ' -f 1 | tr '[:lower:]' '[:upper:]'`
	    echo "bssid: $bssid"

	    ip=${STRING##*ip_address=}
	    ip=`echo ${ip} | cut -d ' ' -f 1`
	    echo "ip: $ip"

	    STRING=`ifconfig ${ifname}`

	    netmask=${STRING##*Mask:}
	    netmask=`echo ${netmask} | cut -d ' ' -f 1`
	    echo "netmask: $netmask"

	    gw=`route -n|grep 'UG'|tr -s ' ' | cut -f 2 -d ' '`
	    echo "gw: $gw"

	    # get vendor and then version
	    vendor=`grep "vendor" /mnt/cfg/miio/device.conf | cut -f 2 -d '=' | tr '[:lower:]' '[:upper:]'`
	    sw_version=`grep "${vendor}_VERSION" /home/web/os-release | cut -f 2 -d '='`
	    if [ -z $sw_version ]; then
		sw_version="unknown"
	    fi

	    msg="{\"method\":\"_internal.info\",\"partner_id\":\"\",\"params\":{\
\"hw_ver\":\"Linux\",\"fw_ver\":\"$sw_version\",\
\"ap\":{\
 \"rssi\":$rssi,\"ssid\":\"$ssid\",\"bssid\":\"$bssid\"\
},\
\"netif\":{\
 \"localIp\":\"$ip\",\"mask\":\"$netmask\",\"gw\":\"$gw\"\
}}}"
	    echo $msg
	    /usr/local/bin/mosquitto_pub -h localhost  -t ${MQTT_TOPIC_INTERNAL_MIIO_CLIENT_IN} -m "$msg"
	elif contains $BUF "_internal.wifi_start"; then
	    id=${BUF#*\"id\":}
	    id=${id%%,*}

		msg="{\"id\":$id,\"method\":\"_internal.wifi_connected\"}"
		echo $msg
		/usr/local/bin/mosquitto_pub -h localhost  -t ${MQTT_TOPIC_INTERNAL_MIIO_CLIENT_IN} -m "$msg"
	elif contains $BUF "_internal.req_wifi_conf_status"; then
	    echo "Got _internal.req_wifi_conf_status"
	    req_wifi_conf_status
	    echo $REQ_WIFI_CONF_STATUS_RESPONSE
	    /usr/local/bin/mosquitto_pub -h localhost  -t ${MQTT_TOPIC_INTERNAL_MIIO_CLIENT_IN} -m "$REQ_WIFI_CONF_STATUS_RESPONSE"
	else
	    sleep 1
	fi
    done
}

send_helper_ready
main
