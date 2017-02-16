/*******************************************************************************
 * wowl.c
 *
 * History:
 *    2015/4/1 - [Tao Wu] Create
 *
 * Copyright (c) 2016 Ambarella, Inc.
 *
 * This file and its contents ( "Software" ) are protected by intellectual
 * property rights including, without limitation, U.S. and/or foreign
 * copyrights. This Software is also the confidential and proprietary
 * information of Ambarella, Inc. and its licensors. You may not use, reproduce,
 * disclose, distribute, modify, or otherwise prepare derivative works of this
 * Software or any portion thereof except pursuant to a signed license agreement
 * or nondisclosure agreement with Ambarella, Inc. or its authorized affiliates.
 * In the absence of such an agreement, you agree to promptly notify and return
 * this Software to Ambarella, Inc.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
 * MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL AMBARELLA, INC. OR ITS AFFILIATES BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; COMPUTER FAILURE OR MALFUNCTION; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
******************************************************************************/

#define __USE_BSD		/* Using BSD IP header 	*/
#include <netinet/ip.h>	/* Internet Protocol 		*/
#define __FAVOR_BSD	/* Using BSD TCP header	*/
#include <netinet/tcp.h>	/* Transmission Control Protocol	*/

#include <pcap.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <ctype.h>
#include <poll.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/ether.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>

typedef unsigned char	u8;
typedef unsigned int	u32;
typedef unsigned char	uint8;	/**< UNSIGNED 8-bit data type */
typedef unsigned short	uint16;/**< UNSIGNED 16-bit data type */
typedef unsigned int	uint;	/**< UNSIGNED 32-bit data type */
typedef unsigned int 	uint32;/**< UNSIGNED 32-bit data type */
typedef signed int		int32;	/**< SIGNED 32-bit data type */
typedef unsigned int	uintptr;

#define MAXBYTES2CAPTURE	(2048)
#define DATA_MAXSIZE 		(32)
#define FILTER_MAXSIZE		(1024)
#define CMD_MAXSIZE		(1024)
#define DEFAULT_HOST		"127.0.0.1"
#define DEFAULT_PORT		(7877)
#define DEFAULT_INTERVAL	(10)
#define DEFAULT_DTIM_INTERVAL	(1000)
#define DEFAULT_MSG		"*"
#define DEFAULT_MSG_HEX	0x2a // char is DEFAULT_MSG

#define DEFAULT_NET_FILTER	"tcp src port 7877"

#define SUSPEND_CMD		"echo mem > /sys/power/state"

#define PROC_WIFI_STATE	"/proc/ambarella/wifi_pm_state"

#define SET_TCP_FIX			(0xBADBEEF)

#define BRCM_TOOL	"wl"

#define NO_ARG		(0)
#define HAS_ARG		(1)

#define PROP_VALUE_MAX	(16)
#define WLC_GET_VERSION	(1)
#define WLC_GET_REVINFO	(98)

#define WLC_IOCTL_MAXLEN	(8192)
#define WLC_GET_VAR		(262)
#define WLC_SET_VAR		(263)
#define WLC_GET_BCNPRD	(75)
#define WLC_GET_DTIMPRD	(77)

#define IPV4_ADDR_LEN		(4)
typedef struct ipv4_addr {
        uint8   addr[IPV4_ADDR_LEN];
} ipv4_addr_t;

/* Linux network driver ioctl encoding */
typedef struct wl_ioctl {
	uint cmd;	/* common ioctl definition */
	void *buf;	/* pointer to user buffer */
	uint len;		/* length of user buffer */
	unsigned char set;		/* 1=set IOCTL; 0=query IOCTL */
	uint used;	/* bytes read or written (optional) */
	uint needed;	/* bytes needed (optional) */
} wl_ioctl_t;

#pragma pack(4)
typedef struct tcpka_conn {
	uint32 sess_id;
	struct ether_addr dst_mac; /* Destinition Mac */
	struct ipv4_addr  src_ip;   	/* Sorce IP */
	struct ipv4_addr  dst_ip;   	/* Destinition IP */

	uint16 ipid;		/* Ip Identification */
	uint16 srcport;	/* Source Port Address */
	uint16 dstport;	/* Destination Port Address */
	uint32 seq;		/* TCP Sequence Number */
	uint32 ack;		/* TCP Ack Number */
	uint16 tcpwin;	/* TCP window */
	uint32 tsval;		/* Timestamp Value */
	uint32 tsecr;		/* Timestamp Echo Reply */
	uint32 len;                /* last packet payload len */
	uint32 ka_payload_len;     /* keep alive payload length */
	uint8  ka_payload[1];      /* keep alive payload */
} tcpka_conn_t;

#pragma pack(1)
typedef struct wl_mtcpkeep_alive_timers_pkt {
	uint16 interval;		/* interval timer */
	uint16 retry_interval;	/* retry_interval timer */
	uint16 retry_count;	/* retry_count */
} wl_mtcpkeep_alive_timers_pkt_t;

typedef struct tcpka_conn_sess_ctl {
	uint32 sess_id;	/* session id */
	uint32 flag;		/* enable/disable flag */
} tcpka_conn_sess_ctl_t;

typedef struct tcpka_conn_sess {
	uint32 sess_id;	/* session id */
	uint32 flag;		/* enable/disable flag */
	wl_mtcpkeep_alive_timers_pkt_t  tcp_keepalive_timers;
} tcpka_conn_sess_t;

#define WL_PKT_FILTER_FIXED_LEN		  OFFSETOF(wl_pkt_filter_t, u)
#define WL_PKT_FILTER_PATTERN_FIXED_LEN	  OFFSETOF(wl_pkt_filter_pattern_t, mask_and_pattern)

typedef enum {
	wowl_pattern_type_bitmap = 0,
	wowl_pattern_type_arp,
	wowl_pattern_type_na
} wowl_pattern_type_t;

typedef struct wl_wowl_pattern_s {
	uint		masksize;	/* Size of the mask in #of bytes */
	uint		offset;		/* Pattern byte offset in packet */
	uint		patternoffset;/* Offset of start of pattern in the structure */
	uint		patternsize;	/* Size of the pattern itself in #of bytes */
	ulong	id;			/* id */
	uint		reasonsize;	/* Size of the wakeup reason code */
	//wowl_pattern_type_t type;		/* Type of pattern */
	/* Mask follows the structure above */
	/* Pattern follows the mask is at 'patternoffset' from the start */
} wl_wowl_pattern_t;

typedef struct wl_wowl_pattern_s_bcm43438 {
	uint		masksize;	/* Size of the mask in #of bytes */
	uint		offset;		/* Pattern byte offset in packet */
	uint		patternoffset;/* Offset of start of pattern in the structure */
	uint		patternsize;	/* Size of the pattern itself in #of bytes */
	ulong	id;			/* id */
	uint		reasonsize;	/* Size of the wakeup reason code */
	wowl_pattern_type_t type;		/* Type of pattern */
	/* Mask follows the structure above */
	/* Pattern follows the mask is at 'patternoffset' from the start */
} wl_wowl_pattern_t_bcm43438;

#define	OFFSETOF(type, member)	((uint)(uintptr)&((type *)0)->member)
typedef struct wl_mkeep_alive_pkt {
	uint16	version;		/* Version for mkeep_alive */
	uint16	length;		/* length of fixed parameters in the structure */
	uint32	period_msec;
	uint16	len_bytes;
	uint8	keep_alive_id; /* 0 - 3 for N = 4 */
	uint8	data[1];
} wl_mkeep_alive_pkt_t;

#define WL_MKEEP_ALIVE_VERSION		1
#define WL_MKEEP_ALIVE_FIXED_LEN	OFFSETOF(wl_mkeep_alive_pkt_t, data)
typedef struct wl_pkt_filter_pattern {
	uint32	offset;	/* Offset within received packet to start pattern matching.
					 * Offset '0' is the first byte of the ethernet header.*/
	uint32	size_bytes;	/* Size of the pattern.  Bitmask must be the same size. */
	uint8   mask_and_pattern[1]; /* Variable length mask and pattern data.  mask starts
								* at offset 0.  Pattern immediately follows mask.*/
} wl_pkt_filter_pattern_t;

typedef struct wl_pkt_filter {
	uint32	id;		/* Unique filter id, specified by app. */
	uint32	type;	/* Filter type (WL_PKT_FILTER_TYPE_xxx). */
	uint32	negate_match;	/* Negate the result of filter matches */
	union {			/* Filter definitions */
		wl_pkt_filter_pattern_t pattern;	/* Pattern matching filter */
	} u;
} wl_pkt_filter_t;

#define WL_PKT_FILTER_FIXED_LEN		  OFFSETOF(wl_pkt_filter_t, u)
#define WL_PKT_FILTER_PATTERN_FIXED_LEN	  OFFSETOF(wl_pkt_filter_pattern_t, mask_and_pattern)
typedef struct wl_pkt_filter_enable {
	uint32	id;		/* Unique filter id */
	uint32	enable;	/* Enable/disable bool */
} wl_pkt_filter_enable_t;

typedef struct wl_pkt_filter_stats {
	uint32	num_pkts_matched;		/* # filter matches for specified filter id */
	uint32	num_pkts_forwarded;	/* # packets fwded from dongle to host for all filters */
	uint32	num_pkts_discarded;	/* # packets discarded by dongle for all filters */
} wl_pkt_filter_stats_t;

typedef struct tcpka_conn_info {
	uint32 tcpka_sess_ipid;
	uint32 tcpka_sess_seq;
	uint32 tcpka_sess_ack;
} tcpka_conn_sess_info_t;

#pragma pack(4)

typedef enum {
	WIFI_BCM43340 = 0,
	WIFI_BCM43438 = 1,
	WIFI_CHIP_TOTAL,
	WIFI_CHIP_FIRST = WIFI_BCM43340,
	WIFI_CHIP_LAST = WIFI_BCM43438,
} wifi_chip_t;

typedef enum {
	CPU_SUSPEND = 0,
	CPU_NORMAL = 1,
	CPU_UNKNOWN = 2,
} cpu_state;

typedef struct net_socket_s
{
	int fd;
	int is_session;
	int is_get_tcp_info;
	int is_close;
	int is_tcp;
	int is_add_tcp_payload;
	int is_enable_pattern;
	int is_wowl_udpka;
	int is_enable_suspend;
	int is_add_resume;
	int is_poll;
	int port;
	int interval;
	int dtim_interval;
	int is_sys_exec;
	wifi_chip_t wifi_chip_id;
	cpu_state sys_state;
	int verbose;

	tcpka_conn_sess_info_t tcp_info_old;
	tcpka_conn_sess_info_t tcp_info;

	char iface[PROP_VALUE_MAX];
	char srv_ip[DATA_MAXSIZE];
	char client_ip[DATA_MAXSIZE];
	char send_msg[DATA_MAXSIZE];
	char recv_msg[DATA_MAXSIZE];

	char net_filter[FILTER_MAXSIZE];
} net_socket_t;

static net_socket_t net_socket;
static int pipefd[2] = {-1, -1};

struct hint_s {
	const char *arg;
	const char *str;
};

static const char *short_options = "i:c:p:tgusm:d:f:bnyloeaxw:v";
static struct option long_options[] = {
	{"interface",	HAS_ARG, 0, 'i'},
	{"host",		HAS_ARG, 0, 'c'},
	{"port",		HAS_ARG, 0, 'p'},
	{"udp",			NO_ARG,  0, 'u'},
	{"tcp",			NO_ARG,  0, 't'},
	{"get",			NO_ARG,  0, 'g'},
	{"session",		NO_ARG,  0, 's'},
	{"kalive-tvl",	HAS_ARG, 0, 'm'},
	{"dtim-tvl",	HAS_ARG, 0, 'd'},
	{"filter", 		HAS_ARG, 0, 'f'},
	{"no-payload", 	NO_ARG, 0, 'b'},
	{"no-pattern",	NO_ARG,  0, 'n'},
	{"wowl-udp",	NO_ARG,  0, 'y'},
	{"suspend", 	NO_ARG,  0, 'l'},
	{"poll", 		NO_ARG,  0, 'o'},
	{"close",		NO_ARG,  0, 'e'},
	{"resume",	NO_ARG, 0, 'a'},
	{"system",		NO_ARG,  0, 'x'},
	{"wifi-chip",	HAS_ARG, 0, 'w'},
	{"verbose",		NO_ARG,  0, 'v'},

	{0, 0, 0, 0}
};

static const struct hint_s hint[] = {
	{"", "Listen on interface"},
	{"", "Host IP address."},
	{"", "Host Port, default is [7877]"},
	{"", "UDP Keep Alive and Wakeup"},
	{"", "TCP Keep Alive and Wakeup"},
	{"", "Get Tcp Keep Alive Session Info, including the ipid, sequence number and acknowledge number. \n" \
	"\t It's indicated the information that the next packet should be used to continue to transfer over the TCP session."},
	{"", "Create one TCP/UDP session"},
	{"", "Keep Alive interval time, in seconds"},
	{"", "DTIM interval time, in mseconds. Equal to AP's DTIM when set 0"},
	{"", "Setting Sniffer Network Filter"},
	{"", "Do not add Payload in TCP KeepAlive"},
	{"", "Do not set Net Pattern"},
	{"", "Set wowl in UDP KeepAlive"},
	{"", "Add suspend after set TCP KeepAlive"},
	{"", "Use poll to detect Wifi driver resume state, otherwise use sleep after suspend"},
	{"", "Close fd after exit, it is disable by default"},
	{"", "Add TCP resume and transfer data."},
	{"", "Using system exec to call wl(Keep-alive, Wakeup Pattern)"},
	{"", "WiFi chip ID: 0=BCM43340; 1=BCM43438/BCM43455. default is 0."},
	{"", "Print verbose info"},
};

static void usage(void)
{
	u32 i = 0;
	char *itself = "wowl";

	printf("This program used to set wake up on wireless (include keepalive and wakeup pattern)\n");
	printf("\n");
	for (i = 0; i < sizeof(long_options) / sizeof(long_options[0]) - 1; i++) {
		if (isalpha(long_options[i].val))
			printf("-%c ", long_options[i].val);
		else
			printf("   ");
		printf("--%s", long_options[i].name);
		if (hint[i].arg[0] != 0)
			printf(" [%s]", hint[i].arg);
		printf("\t%s\n", hint[i].str);
	}

	printf("Example:\n");

	printf("\t TCP Resume # %s -w 0 -i wlan0 -c 127.0.0.1 -p 7877 -t -s -m 10 -d 1000 -f \"tcp dst port 7877\" -l -e -o -a \n", itself);
	printf("\t UDP wowl   # %s -w 0 -i wlan0 -c 127.0.0.1 -p 7877 -u -s -m 10 -f \"udp dst port 7877\" -y \n", itself);

	printf("\t TCP # %s -w 0 -i wlan0 -c 127.0.0.1 -p 7877 -t -s -m 10 -f \"tcp dst port 7877\" \n", itself);
	printf("\t UDP # %s -w 0 -i wlan0 -c 127.0.0.1 -p 7877 -u -s -m 10 -f \"udp dst port 7877\" \n", itself);

	printf("\t Get TCP session info # %s -i wlan0 -g\n", itself);
}

static int init_param(int argc, char **argv)
{
	int ch = 0;
	int value = 0;
	int option_index = 0;

	opterr = 0;
	while ((ch = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
		switch (ch) {
		case 'i':
			strncpy(net_socket.iface, optarg, sizeof(net_socket.iface));
			break;
		case 'c':
			strncpy(net_socket.srv_ip, optarg, sizeof(net_socket.srv_ip));
			break;
		case 'p':
			value = atoi(optarg);
			if ((value < 0) || (value > 65535)) {
				printf("Please set port in %d ~ %d.\n", 0, 65535);
				return -1;
			}
			net_socket.port = value;
			break;
		case 'm':
			value = atoi(optarg);
			if ((value < 0) || (value > 65535)) {
				printf("Please set interval in %d ~ %d.\n", 0, 65535);
				return -1;
			}
			net_socket.interval = value;
			break;
		case 'd':
			value = atoi(optarg);
			if ((value < 0) || (value > 65535)) {
				printf("Please set DTIM interval in %d ~ %d.\n", 0, 65535);
				return -1;
			}
			net_socket.dtim_interval = value;
			break;
		case 't':
			net_socket.is_tcp = 1;
			break;
		case 'u':
			net_socket.is_tcp = 0;
			break;
		case 'a':
			net_socket.is_add_resume = 1;
			break;
		case 'g':
			net_socket.is_get_tcp_info = 1;
			break;
		case 's':
			net_socket.is_session = 1;
			break;
		case 'f':
			strncpy(net_socket.net_filter, optarg, sizeof(net_socket.net_filter));
			break;
		case 'b':
			net_socket.is_add_tcp_payload = 0;
			break;
		case 'n':
			net_socket.is_enable_pattern = 0;
			break;
		case 'y':
			net_socket.is_wowl_udpka = 1;
			break;
		case 'l':
			net_socket.is_enable_suspend = 1;
			break;
		case 'o':
			net_socket.is_poll = 1;
			break;
		case 'e':
			net_socket.is_close = 1;
			break;
		case 'x':
			net_socket.is_sys_exec = 1;
			break;
		case 'w':
			value = atoi(optarg);
			if ((value < WIFI_CHIP_FIRST) || (value > WIFI_CHIP_LAST)) {
				printf("Please chosse WiFi Chip ID in %d ~ %d.\n",
					WIFI_CHIP_FIRST, WIFI_CHIP_LAST);
				return -1;
			}
			net_socket.wifi_chip_id = value;
			break;
		case 'v':
			net_socket.verbose = 1;
			break;
		default:
			printf("unknown option found: %c\n", ch);
			return -1;
			break;
		}
	}

	return 0;
}

static void close_fd(int *fd)
{
	if (net_socket.is_close && (*fd > 0)) {
		close(*fd);
		printf("Close [%d].\n", *fd);
		*fd = -1;
	}
}

static char *iptoa(const struct ipv4_addr *n)
{
	static char iptoa_buf[IPV4_ADDR_LEN * 4];

	sprintf(iptoa_buf, "%u.%u.%u.%u", n->addr[0], n->addr[1], n->addr[2], n->addr[3]);

	return iptoa_buf;
}

#if 0
static int atoip(const char *a, struct ipv4_addr *n)
{
	char *c = NULL;
	int i = 0;

	for (;;) {
		n->addr[i++] = (uint8)strtoul(a, &c, 0);
		if (*c++ != '.' || i == IPV4_ADDR_LEN)
			break;
		a = c;
	}
	return (i == IPV4_ADDR_LEN);
}

static void in_addr_to_ipv4(struct ipv4_addr *ipa, u32 ip)
{
	ipa->addr[0] = (u8)(ip >> 0) & 0xFF;
	ipa->addr[1] = (u8)(ip >> 8) & 0xFF;
	ipa->addr[2] = (u8)(ip >> 16) & 0xFF;
	ipa->addr[3] = (u8)(ip >> 24) & 0xFF;
}

static ssize_t readn( int inSock, void *outBuf, size_t inLen )
{
	size_t  nleft = 0;
	ssize_t nread = 0;
	char *ptr;

	assert( inSock >= 0 );
	assert( outBuf != NULL );
	assert( inLen > 0 );

	ptr   = (char*) outBuf;
	nleft = inLen;

	while ( nleft > 0 ) {
		nread = read( inSock, ptr, nleft );
		if ( nread < 0 ) {
			if ( errno == EINTR ) {
				nread = 0;  /* interupted, call read again */
			}
			else{
				return -1;  /* error */
			}
		} else if ( nread == 0 ) {
			break;        /* EOF */
		}
		nleft -= nread;
		ptr   += nread;
	}

	return(inLen - nleft);
} /* end readn */
#endif

static ssize_t writen( int inSock, const void *inBuf, size_t inLen )
{
	size_t  nleft = 0;
	ssize_t nwritten = 0;
	const char *ptr;

	assert( inSock >= 0 );
	assert( inBuf != NULL );
	assert( inLen > 0 );

	ptr   = (char*) inBuf;
	nleft = inLen;

	while ( nleft > 0 ) {
		nwritten = write( inSock, ptr, nleft );
		if ( nwritten <= 0 ) {
			if ( errno == EINTR ) {
				nwritten = 0; /* interupted, call write again */
			} else {
				return -1;    /* error */
			}
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}

	return inLen;
} /* end writen */

static int wl_pattern_atoh(char *src, char *dst)
{
	int i = 0;

	if (strncmp(src, "0x", 2) != 0 && strncmp(src, "0X", 2) != 0) {
		printf("Data invalid format. Needs to start with 0x\n");
		return -1;
	}
	src = src + 2; /* Skip past 0x */
	if (strlen(src) % 2 != 0) {
		printf("Data invalid format. Needs to be of even length\n");
		return -1;
	}
	for (i = 0; *src != '\0'; i++) {
		char num[3];
		strncpy(num, src, 2);
		num[2] = '\0';
		dst[i] = (uint8)strtoul(num, NULL, 16);
		src += 2;
	}

	return i;
}

static uint wl_iovar_mkbuf(const char *name, char *data, uint datalen,
	char *iovar_buf, uint buflen, int *perr)
{
	uint iovar_len = 0;

	iovar_len = strlen(name) + 1;

	/* check for overflow */
	if ((iovar_len + datalen) > buflen) {
		*perr = -1;
		return 0;
	}

	/* copy data to the buffer past the end of the iovar name string */
	if (datalen > 0) {
		memmove(&iovar_buf[iovar_len], data, datalen);
	}

	/* copy the name to the beginning of the buffer */
	strcpy(iovar_buf, name);

	*perr = 0;
	return (iovar_len + datalen);
}

/*  TCP Keep alive */
static int wl_tcpka_conn_add(tcpka_conn_t tcpka)
{
	char bufdata[WLC_IOCTL_MAXLEN];
	char buf_src_addr[DATA_MAXSIZE];
	char buf_dst_addr[DATA_MAXSIZE];
	char CMD[CMD_MAXSIZE];
	struct ifreq ifr;
	wl_ioctl_t ioc;
	int ret_size = 0;
	int ret = 0;
	int s = 0;
	int i = 0;

	memset(buf_src_addr, 0, DATA_MAXSIZE);
	memset(buf_dst_addr, 0, DATA_MAXSIZE);

	if (net_socket.is_sys_exec) {
		memset(CMD, 0, CMD_MAXSIZE);
		strncpy(buf_src_addr, iptoa(&tcpka.src_ip), sizeof(buf_src_addr));
		strncpy(buf_dst_addr, iptoa(&tcpka.dst_ip), sizeof(buf_dst_addr));
		snprintf(CMD, sizeof(CMD), "%s tcpka_conn_add %s %s %s %d %d %d %u %u %d %u %u %u %u \"%s\"\n",
			BRCM_TOOL, ether_ntoa(&tcpka.dst_mac), buf_src_addr, buf_dst_addr,
			tcpka.ipid, tcpka.srcport, tcpka.dstport, tcpka.seq, tcpka.ack, tcpka.tcpwin,
			tcpka.tsval, tcpka.tsecr, tcpka.len, tcpka.ka_payload_len, tcpka.ka_payload);

		system(CMD);
		printf("CMD: %s\n", CMD);
		return 0;
	}

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("cannot open socket %s - %s\n", net_socket.iface, strerror(errno));
		return -1;
	}

	ret_size = wl_iovar_mkbuf("tcpka_conn_add", (char *)&tcpka, (sizeof(tcpka_conn_t) + tcpka.ka_payload_len - 1),
		bufdata, WLC_IOCTL_MAXLEN, &ret);

	if (ret) {
		printf("Failed to build buffer, %s.\n", __FUNCTION__);
		return -1;
	}

	ioc.cmd = WLC_GET_VAR;
	ioc.buf = &bufdata;
	ioc.len = WLC_IOCTL_MAXLEN;
	ioc.set = 0;

	strncpy(ifr.ifr_name, net_socket.iface, IFNAMSIZ);
	ifr.ifr_data = (caddr_t) &ioc;

	if (net_socket.verbose) {
		printf("DUMP:");
		for ( i = 0; i <ret_size; i++ ) {
			printf("%02x", bufdata[i]);
		}
		printf("\n");
	}

	if (net_socket.verbose) {
		strncpy(buf_src_addr, iptoa(&tcpka.src_ip), sizeof(buf_src_addr));
		strncpy(buf_dst_addr, iptoa(&tcpka.dst_ip), sizeof(buf_dst_addr));
		printf("Session ID [%d]\n", tcpka.sess_id);
		printf("tcpka_conn_add %s %s %s %d %d %d %u %u %d %u %u %u %u \"%s\"\n",
			ether_ntoa(&tcpka.dst_mac), buf_src_addr, buf_dst_addr,
			tcpka.ipid, tcpka.srcport, tcpka.dstport, tcpka.seq, tcpka.ack, tcpka.tcpwin,
			tcpka.tsval, tcpka.tsecr, tcpka.len, tcpka.ka_payload_len, tcpka.ka_payload);
	}

	//printf("--- cmd: %d ---\n", ioc.cmd);
	if ((ret = ioctl(s, SIOCDEVPRIVATE, &ifr)) < 0) {
		printf("fail tcpka_conn_add, %s\n", strerror(errno));
		return ret;
	}

	return ret;
}

static int wl_tcpka_conn_disable(u32 sess_id)
{
	char bufdata[WLC_IOCTL_MAXLEN];
	char CMD[CMD_MAXSIZE];
	struct ifreq ifr;
	wl_ioctl_t ioc;
	int ret = 0;
	int s = 0;
	tcpka_conn_sess_ctl_t conn;

	conn.sess_id = sess_id;
	conn.flag = 0;

	if (net_socket.is_sys_exec) {
		memset(CMD, 0, CMD_MAXSIZE);
		snprintf(CMD, sizeof(CMD), "%s tcpka_conn_enable %u %u", BRCM_TOOL,
			conn.sess_id, conn.flag);
		system(CMD);
		printf("CMD: %s\n", CMD);
		return 0;
	}

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("cannot open socket %s - %s\n", net_socket.iface, strerror(errno));
		return -1;
	}

	wl_iovar_mkbuf("tcpka_conn_enable", (char *)&conn, sizeof(tcpka_conn_sess_ctl_t),
		bufdata, WLC_IOCTL_MAXLEN, &ret);

	if (ret) {
		printf("Failed to build buffer, %s.\n", __FUNCTION__);
		return -1;
	}

	ioc.cmd = WLC_SET_VAR;
	ioc.buf = &bufdata;
	ioc.len = WLC_IOCTL_MAXLEN;
	ioc.set = 1;

	strncpy(ifr.ifr_name, net_socket.iface, IFNAMSIZ);
	ifr.ifr_data = (caddr_t) &ioc;

	if ((ret = ioctl(s, SIOCDEVPRIVATE, &ifr)) < 0) {
		printf("fail tcpka_conn_disable, %s\n", strerror(errno));
		return ret;
	}

	return ret;
}

static int wl_tcpka_conn_enable(u32 sess_id)
{
	char bufdata[WLC_IOCTL_MAXLEN];
	char CMD[CMD_MAXSIZE];
	struct ifreq ifr;
	wl_ioctl_t ioc;
	tcpka_conn_sess_t tcpka_conn;
	int ret = 0;
	int s = 0;

	/* init tcpka_conn */
	tcpka_conn.sess_id = sess_id;
	tcpka_conn.flag = 1;
	tcpka_conn.tcp_keepalive_timers.interval = net_socket.interval; //senconds
	tcpka_conn.tcp_keepalive_timers.retry_interval = 2;
	tcpka_conn.tcp_keepalive_timers.retry_count = 2;

	if (net_socket.is_sys_exec) {
		memset(CMD, 0, CMD_MAXSIZE);
		snprintf(CMD, sizeof(CMD), "%s tcpka_conn_enable %u %u %u %u %u",
			BRCM_TOOL, tcpka_conn.sess_id, tcpka_conn.flag, net_socket.interval,
			tcpka_conn.tcp_keepalive_timers.retry_interval,
			tcpka_conn.tcp_keepalive_timers.retry_count);
		system(CMD);
		printf("CMD: %s\n", CMD);
		return 0;
	}

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("cannot open socket %s - %s\n", net_socket.iface, strerror(errno));
		return -1;
	}

	wl_iovar_mkbuf("tcpka_conn_enable", (char *)&tcpka_conn, sizeof(tcpka_conn_sess_t),
		bufdata, WLC_IOCTL_MAXLEN, &ret);

	if (ret) {
		printf("Failed to build buffer, %s.\n", __FUNCTION__);
		return -1;
	}

	ioc.cmd = WLC_SET_VAR;
	ioc.buf = &bufdata;
	ioc.len = WLC_IOCTL_MAXLEN;
	ioc.set = 1;

	strncpy(ifr.ifr_name, net_socket.iface, IFNAMSIZ);
	ifr.ifr_data = (caddr_t) &ioc;

	if (net_socket.verbose) {
		printf("TCP Interval [%d] seconds\n", tcpka_conn.tcp_keepalive_timers.interval);
	}

	//printf("--- cmd: %d ---\n", ioc.cmd);
	if ((ret = ioctl(s, SIOCDEVPRIVATE, &ifr)) < 0) {
		printf("fail set tcpka_conn_enable, %s\n", strerror(errno));
	}

	return ret;
}

static int wl_tcpka_conn_sess_info(u32 sess_id)
{
	struct ifreq ifr;
	wl_ioctl_t ioc;
	char bufdata[WLC_IOCTL_MAXLEN];
	int ret = 0;
	int s = 0;
	uint32 id = 0;
	tcpka_conn_sess_info_t *sess_info;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("cannot open socket %s - %s", net_socket.iface, strerror(errno));
		return -1;
	}

	/* Set tcpka id */
	id = sess_id;

	wl_iovar_mkbuf("tcpka_conn_sess_info", (char *)&id, sizeof(uint32), bufdata, WLC_IOCTL_MAXLEN, &ret);
	if (ret) {
		printf("Failed to build buffer, %s.\n", __FUNCTION__);
		return -1;
	}

	ioc.cmd = WLC_GET_VAR;
	ioc.buf = &bufdata;
	ioc.len = WLC_IOCTL_MAXLEN;
	ioc.set = 0;

	strncpy(ifr.ifr_name, net_socket.iface, IFNAMSIZ);
	ifr.ifr_data = (caddr_t) &ioc;

	if ((ret = ioctl(s, SIOCDEVPRIVATE, &ifr)) < 0) {
		printf("fail get tcpka_conn_sess_info, %s\n", strerror(errno));
	}

	if(!ret) {
		sess_info = (tcpka_conn_sess_info_t *) bufdata;
		printf("Get Session Id(%d) info : \n"
		"Keep alive ipid       : %u\n"
		"Keep alive seq num    : %u\n"
		"Keep alive ack num    : %u\n",
		id,
		sess_info->tcpka_sess_ipid,
		sess_info->tcpka_sess_seq,
		sess_info->tcpka_sess_ack);
	}
	net_socket.tcp_info.tcpka_sess_ipid = sess_info->tcpka_sess_ipid;
	net_socket.tcp_info.tcpka_sess_seq = sess_info->tcpka_sess_seq;
	net_socket.tcp_info.tcpka_sess_ack = sess_info->tcpka_sess_ack;

	return ret;
}

static int wl_tcpka_conn_del(u32 sess_id)
{
	char bufdata[WLC_IOCTL_MAXLEN];
	char CMD[CMD_MAXSIZE];
	struct ifreq ifr;
	wl_ioctl_t ioc;
	u32 id = 0;
	int ret = 0;
	int s = 0;

	/* init tcpka */
	id = sess_id;

	if (net_socket.is_sys_exec) {
		memset(CMD, 0, CMD_MAXSIZE);
		snprintf(CMD, sizeof(CMD), "%s tcpka_conn_del %u", BRCM_TOOL, id);
		system(CMD);
		printf("CMD: %s\n", CMD);
		return 0;
	}

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("cannot open socket %s - %s\n", net_socket.iface, strerror(errno));
		return -1;
	}

	wl_iovar_mkbuf("tcpka_conn_del", (char *)&id, sizeof(int32),
		bufdata, WLC_IOCTL_MAXLEN, &ret);

	if (ret) {
		printf("Failed to build buffer, %s.\n", __FUNCTION__);
		return -1;
	}

	ioc.cmd = WLC_SET_VAR;
	ioc.buf = &bufdata;
	ioc.len = WLC_IOCTL_MAXLEN;
	ioc.set = 1;

	strncpy(ifr.ifr_name, net_socket.iface, IFNAMSIZ);
	ifr.ifr_data = (caddr_t) &ioc;

	//printf("--- cmd: %d ---\n", ioc.cmd);
	if ((ret = ioctl(s, SIOCDEVPRIVATE, &ifr)) < 0) {
		printf("fail set tcpka_conn_del, %s\n", strerror(errno));
	}

	return ret;
}

static int wl_wowl_pattern_bcm43340(char *wowl_pattern)
{
	char bufdata[WLC_IOCTL_MAXLEN];
	char CMD[CMD_MAXSIZE];
	struct ifreq ifr;
	wl_ioctl_t ioc;
	const char *str;
	wl_wowl_pattern_t *wl_pattern;
	char *buf, *mask_and_pattern;
	char *mask, *pattern;
	int str_len = 0;
	uint buf_len = 0;
	int ret = 0;
	int s = 0;
	int i = 0;

	if (net_socket.is_sys_exec) {
		memset(CMD, 0, CMD_MAXSIZE);
		if (net_socket.is_wowl_udpka) {
			snprintf(CMD, sizeof(CMD), "%s wowl_pattern add 26 0xff0f %s",
			BRCM_TOOL, wowl_pattern);
		} else {
			snprintf(CMD, sizeof(CMD), "%s wowl_pattern add 26 0xff0f0000000f %s",
			BRCM_TOOL, wowl_pattern);
		}
		system(CMD);
		printf("CMD: %s\n", CMD);
		return 0;
	}

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("cannot open socket %s - %s\n", net_socket.iface, strerror(errno));
		return -1;
	}

	/* wl wowl_pattern add 0 0x0f 0x77616b65 : Receive "wake" tcp data */
	str = "wowl_pattern";
	str_len = strlen(str);
	strncpy(bufdata, str, str_len);
	bufdata[str_len] = '\0';
	buf = bufdata + strlen(str) + 1;
	buf_len = str_len + 1;

	str = "add";
	strncpy(buf, str, strlen(str));
	buf_len += strlen(str) + 1;

	wl_pattern = (wl_wowl_pattern_t *)(buf + strlen(str) + 1);
	mask_and_pattern = (char*)wl_pattern + sizeof(wl_wowl_pattern_t);

	wl_pattern->offset = 26;

	/* Parse the mask */
	if (net_socket.is_wowl_udpka) {
		mask = "0xff0f";
		wl_pattern->masksize = 2;
	} else {
		mask = "0xff0f0000000f";
		wl_pattern->masksize = 6;
	}
	wl_pattern_atoh(mask, mask_and_pattern);
	mask_and_pattern += wl_pattern->masksize;
	wl_pattern->patternoffset = sizeof(wl_wowl_pattern_t) +	wl_pattern->masksize;

	/* Parse the pattern */
	pattern = wowl_pattern;
	wl_pattern->patternsize = (strlen(pattern)-2)/2;
	wl_pattern_atoh(pattern, mask_and_pattern);

	buf_len += sizeof(wl_wowl_pattern_t) + wl_pattern->patternsize + wl_pattern->masksize;

	ioc.cmd = WLC_SET_VAR;
	ioc.buf = &bufdata;
	ioc.len = buf_len;
	ioc.set = 1;

	strncpy(ifr.ifr_name, net_socket.iface, IFNAMSIZ);
	ifr.ifr_data = (caddr_t) &ioc;

	if (net_socket.verbose) {
		printf("wowl_pattern[%d]=%s\n", strlen(pattern), pattern);
		printf("DUMP:");
		for ( i = 0; i <buf_len; i++ ) {
			printf("%02x", bufdata[i]);
		}
		printf("\n");
	}

	//printf("--- cmd: %d ---\n", ioc.cmd);
	if ((ret = ioctl(s, SIOCDEVPRIVATE, &ifr)) < 0) {
		printf("fail set wowl_pattern add, %s\n", strerror(errno));
	}

	return ret;
}

static int wl_wowl_pattern_bcm43438(char *wowl_pattern)
{
	char bufdata[WLC_IOCTL_MAXLEN];
	char CMD[CMD_MAXSIZE];
	struct ifreq ifr;
	wl_ioctl_t ioc;
	const char *str;
	wl_wowl_pattern_t_bcm43438 *wl_pattern;
	char *buf, *mask_and_pattern;
	char *mask, *pattern;
	int str_len = 0;
	uint buf_len = 0;
	int ret = 0;
	int s = 0;
	int i = 0;

	if (net_socket.is_sys_exec) {
		memset(CMD, 0, CMD_MAXSIZE);
		if (net_socket.is_wowl_udpka) {
			snprintf(CMD, sizeof(CMD), "%s wowl_pattern add 26 0xff0f %s",
			BRCM_TOOL, wowl_pattern);
		} else {
			snprintf(CMD, sizeof(CMD), "%s wowl_pattern add 26 0xff0f0000000f %s",
			BRCM_TOOL, wowl_pattern);
		}
		system(CMD);
		printf("CMD: %s\n", CMD);
		return 0;
	}

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("cannot open socket %s - %s\n", net_socket.iface, strerror(errno));
		return -1;
	}

	/* wl wowl_pattern add 0 0x0f 0x77616b65 : Receive "wake" tcp data */
	str = "wowl_pattern";
	str_len = strlen(str);
	strncpy(bufdata, str, str_len);
	bufdata[str_len] = '\0';
	buf = bufdata + strlen(str) + 1;
	buf_len = str_len + 1;

	str = "add";
	strncpy(buf, str, strlen(str));
	buf_len += strlen(str) + 1;

	wl_pattern = (wl_wowl_pattern_t_bcm43438 *)(buf + strlen(str) + 1);
	mask_and_pattern = (char*)wl_pattern + sizeof(wl_wowl_pattern_t_bcm43438);

	wl_pattern->type = 0;
	wl_pattern->offset = 26;

	/* Parse the mask */
	if (net_socket.is_wowl_udpka) {
		mask = "0xff0f";
		wl_pattern->masksize = 2;
	} else {
		mask = "0xff0f0000000f";
		wl_pattern->masksize = 6;
	}
	wl_pattern_atoh(mask, mask_and_pattern);

	mask_and_pattern += wl_pattern->masksize;
	wl_pattern->patternoffset = sizeof(wl_wowl_pattern_t_bcm43438) + wl_pattern->masksize;

	/* Parse the pattern */
	pattern = wowl_pattern;
	wl_pattern->patternsize = (strlen(pattern)-2)/2;
	wl_pattern_atoh(pattern, mask_and_pattern);

	buf_len += sizeof(wl_wowl_pattern_t_bcm43438) + wl_pattern->patternsize +
		wl_pattern->masksize;

	ioc.cmd = WLC_SET_VAR;
	ioc.buf = &bufdata;
	ioc.len = buf_len;
	ioc.set = 1;

	strncpy(ifr.ifr_name, net_socket.iface, IFNAMSIZ);
	ifr.ifr_data = (caddr_t) &ioc;

	if (net_socket.verbose) {
		printf("wowl_pattern[%d]=%s\n", strlen(pattern), pattern);
		printf("DUMP:");
		for ( i = 0; i <buf_len; i++ ) {
			printf("%02x", bufdata[i]);
		}
		printf("\n");
	}

	//printf("--- cmd: %d ---\n", ioc.cmd);
	if ((ret = ioctl(s, SIOCDEVPRIVATE, &ifr)) < 0) {
		printf("fail set wowl_pattern add, %s\n", strerror(errno));
	}

	return ret;
}

static int wl_wowl_pattern(char *wowl_pattern)
{
	if (net_socket.wifi_chip_id == WIFI_BCM43438) {
		return wl_wowl_pattern_bcm43438(wowl_pattern);
	} else {
		return wl_wowl_pattern_bcm43340(wowl_pattern);
	}
}

static int wl_wowl(void)
{
	char bufdata[WLC_IOCTL_MAXLEN];
	char CMD[CMD_MAXSIZE];
	struct ifreq ifr;
	wl_ioctl_t ioc;
	int ret = 0;
	int s = 0;
	uint32 wowl = 0;

	if (net_socket.is_sys_exec) {
		memset(CMD, 0, CMD_MAXSIZE);
		snprintf(CMD, sizeof(CMD), "%s wowl 0x00016", BRCM_TOOL);
		system(CMD);
		printf("CMD: %s\n", CMD);
		return 0;
	}

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("cannot open socket %s - %s\n", net_socket.iface, strerror(errno));
		return -1;
	}

	/* wl wowl 0x20016 */
	wowl = 0x00016;
	wl_iovar_mkbuf("wowl", (char *)&wowl, sizeof(uint32), bufdata, WLC_IOCTL_MAXLEN, &ret);

	if (ret) {
		printf("Failed to build buffer, %s.\n", __FUNCTION__);
		return -1;
	}

	ioc.cmd = WLC_SET_VAR;
	ioc.buf = &bufdata;
	ioc.len = WLC_IOCTL_MAXLEN;
	ioc.set = 1;

	strncpy(ifr.ifr_name, net_socket.iface, IFNAMSIZ);
	ifr.ifr_data = (caddr_t) &ioc;

	//printf("--- cmd: %d ---\n", ioc.cmd);
	if ((ret = ioctl(s, SIOCDEVPRIVATE, &ifr)) < 0) {
		printf("fail set wowl, %s\n", strerror(errno));
	}

	return ret;
}

static int wl_wowl_activate(void)
{
	char bufdata[WLC_IOCTL_MAXLEN];
	char CMD[CMD_MAXSIZE];
	struct ifreq ifr;
	wl_ioctl_t ioc;
	int ret = 0;
	int s = 0;
	uint32 wowl_activate = 0;

	if (net_socket.is_sys_exec) {
		memset(CMD, 0, CMD_MAXSIZE);
		snprintf(CMD, sizeof(CMD), "%s wowl_activate 1", BRCM_TOOL);
		system(CMD);
		printf("CMD: %s\n", CMD);
		return 0;
	}

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("cannot open socket %s - %s", net_socket.iface, strerror(errno));
		return -1;
	}

	/* wl wowl_activate 1 */
	wowl_activate = 1;
	wl_iovar_mkbuf("wowl_activate", (char *)&wowl_activate, sizeof(uint32),
		bufdata, WLC_IOCTL_MAXLEN, &ret);

	if (ret) {
		printf("Failed to build buffer, %s.\n", __FUNCTION__);
		return -1;
	}

	ioc.cmd = WLC_SET_VAR;
	ioc.buf = &bufdata;
	ioc.len = WLC_IOCTL_MAXLEN;
	ioc.set = 1;

	strncpy(ifr.ifr_name, net_socket.iface, IFNAMSIZ);
	ifr.ifr_data = (caddr_t) &ioc;

	//printf("--- cmd: %d ---\n", ioc.cmd);
	if ((ret = ioctl(s, SIOCDEVPRIVATE, &ifr)) < 0) {
		printf("fail set wowl_activate, %s\n", strerror(errno));
	}

	return ret;
}

static int wl_wowl_clear(void)
{
	char bufdata[WLC_IOCTL_MAXLEN];
	char CMD[CMD_MAXSIZE];
	struct ifreq ifr;
	wl_ioctl_t ioc;
	int ret = 0;
	int s = 0;

	if (net_socket.is_sys_exec) {
		memset(CMD, 0, CMD_MAXSIZE);
		snprintf(CMD, sizeof(CMD), "%s wowl_clear", BRCM_TOOL);
		system(CMD);
		printf("CMD: %s\n", CMD);
		return 0;
	}

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("cannot open socket %s - %s\n", net_socket.iface, strerror(errno));
		return -1;
	}

	/* wl wowl_clear */
	wl_iovar_mkbuf("wowl_clear", NULL, 0, bufdata, WLC_IOCTL_MAXLEN, &ret);

	if (ret) {
		printf("Failed to build buffer, %s.\n", __FUNCTION__);
		return -1;
	}

	ioc.cmd = WLC_SET_VAR;
	ioc.buf = &bufdata;
	ioc.len = WLC_IOCTL_MAXLEN;
	ioc.set = 1;

	strncpy(ifr.ifr_name, net_socket.iface, IFNAMSIZ);
	ifr.ifr_data = (caddr_t) &ioc;

	//printf("--- cmd: %d ---\n", ioc.cmd);
	if ((ret = ioctl(s, SIOCDEVPRIVATE, &ifr)) < 0) {
		printf("fail wowl_clear, %s\n", strerror(errno));
	}

	return ret;
}

/*  UDP Keep alive */
static int wl_mkeep_alive(u32 length, const u_char *packet)
{
	char bufdata[WLC_IOCTL_MAXLEN];
	char CMD[CMD_MAXSIZE];
	char CMD_BAK[CMD_MAXSIZE];
	struct ifreq ifr;
	wl_ioctl_t ioc;
	const char *str;
	wl_mkeep_alive_pkt_t  mkeep_alive_pkt;
	wl_mkeep_alive_pkt_t  *mkeep_alive_pktp;
	int str_len = 0;
	uint buf_len = 0;
	int ret = 0;
	int s = 0;
	int i = 0;

	if (net_socket.is_sys_exec) {
		memset(CMD, 0, CMD_MAXSIZE);
		memset(CMD_BAK, 0, CMD_MAXSIZE);
		snprintf(CMD, sizeof(CMD), "%s mkeep_alive 1 %d",
			BRCM_TOOL, net_socket.interval * 1000);
		strcat(CMD, " 0x");
		for (i = 0; i < length; i++) {
			strcpy(CMD_BAK, CMD);
			snprintf(CMD, sizeof(CMD), "%s%02x", CMD_BAK, packet[i]);
		}
		system(CMD);
		printf("CMD: %s\n", CMD);
		return 0;
	}

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("cannot open socket %s - %s\n", net_socket.iface, strerror(errno));
		return -1;
	}

	/* init mkeep_alive data */
	str = "mkeep_alive";
	str_len = strlen(str);
	strncpy(bufdata, str, str_len);
	bufdata[ str_len ] = '\0';
	buf_len = str_len + 1;
	mkeep_alive_pktp = (wl_mkeep_alive_pkt_t *) (bufdata + str_len + 1);

	memset(&mkeep_alive_pkt, 0, sizeof(wl_mkeep_alive_pkt_t));
	mkeep_alive_pkt.period_msec = net_socket.interval * 1000; // milliseconds
	mkeep_alive_pkt.version = WL_MKEEP_ALIVE_VERSION;
	mkeep_alive_pkt.length = WL_MKEEP_ALIVE_FIXED_LEN;
	mkeep_alive_pkt.keep_alive_id = 1;
	mkeep_alive_pkt.len_bytes = length;
	memcpy(mkeep_alive_pktp->data, packet, length);
	buf_len += WL_MKEEP_ALIVE_FIXED_LEN + mkeep_alive_pkt.len_bytes;

	memcpy((char *)mkeep_alive_pktp, &mkeep_alive_pkt, WL_MKEEP_ALIVE_FIXED_LEN);

	ioc.cmd = WLC_SET_VAR;
	ioc.buf = &bufdata;
	ioc.len = buf_len;
	ioc.set = 0;

	strncpy(ifr.ifr_name, net_socket.iface, IFNAMSIZ);
	ifr.ifr_data = (caddr_t) &ioc;

	if (net_socket.verbose) {
		printf("UDP Interval [%d] seconds\n", net_socket.interval);
	}

	//printf("--- cmd: %d ---\n", ioc.cmd);
	if ((ret = ioctl(s, SIOCDEVPRIVATE, &ifr)) < 0) {
		printf("fail mkeep_alive, %s\n", strerror(errno));
		return ret;
	}

	return ret;
}

static int wl_pkt_filter_add(char *filter_pattern)
{
	char bufdata[WLC_IOCTL_MAXLEN];
	char CMD[CMD_MAXSIZE];
	struct ifreq ifr;
	wl_ioctl_t ioc;
	const char *str;
	wl_pkt_filter_t	 pkt_filter;
	wl_pkt_filter_t	 *pkt_filterp;
	int str_len = 0;
	uint buf_len = 0;
	int ret = 0;
	int s = 0;
	uint32 mask_size = 0;
	uint32 pattern_size = 0;
	char *mask, *pattern;

	if (net_socket.is_sys_exec) {
		memset(CMD, 0, CMD_MAXSIZE);
		snprintf(CMD, sizeof(CMD), "%s pkt_filter_add 200 0 0 26 0xffffffffffffffffffffffff %s",
			BRCM_TOOL, filter_pattern);
		system(CMD);
		printf("CMD: %s\n", CMD);
		return 0;
	}

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("cannot open socket %s - %s\n", net_socket.iface, strerror(errno));
		return -1;
	}

	/* wl pkt_filter_add 200 0 0 36 0xffff 0x1EC5 : Receive UDP from port 7877 */
	str = "pkt_filter_add";
	str_len = strlen(str);
	strncpy(bufdata, str, str_len);
	bufdata[ str_len ] = '\0';
	buf_len = str_len + 1;

	pkt_filterp = (wl_pkt_filter_t *) (bufdata + str_len + 1);

	pkt_filter.id = 200;
	pkt_filter.negate_match = 0;
	pkt_filter.type = 0;
	pkt_filter.u.pattern.offset = 26;

	mask_size = 12;
	mask = "0xffffffffffffffffffffffff";
	wl_pattern_atoh(mask, (char*) pkt_filterp->u.pattern.mask_and_pattern);
	pattern = filter_pattern;
	pattern_size = (strlen(pattern)-2)/2;
	wl_pattern_atoh(pattern, (char*) &pkt_filterp->u.pattern.mask_and_pattern[mask_size]);

	pkt_filter.u.pattern.size_bytes = pattern_size;
	buf_len += WL_PKT_FILTER_FIXED_LEN;
	buf_len += (WL_PKT_FILTER_PATTERN_FIXED_LEN + 2 * mask_size);

	memcpy((char *)pkt_filterp, &pkt_filter,
		WL_PKT_FILTER_FIXED_LEN + WL_PKT_FILTER_PATTERN_FIXED_LEN);

	ioc.cmd = WLC_SET_VAR;
	ioc.buf = &bufdata;
	ioc.len = buf_len;
	ioc.set = 1;

	strncpy(ifr.ifr_name, net_socket.iface, IFNAMSIZ);
	ifr.ifr_data = (caddr_t) &ioc;

	if (net_socket.verbose) {
		printf("filter_pattern[%d]=%s\n", strlen(pattern), pattern);
	}

	//printf("--- cmd: %d ---\n", ioc.cmd);
	if ((ret = ioctl(s, SIOCDEVPRIVATE, &ifr)) < 0) {
		printf("fail set pkt_filter_add, %s\n", strerror(errno));
	}

	return ret;
}

static int pkt_filter_enable(void)
{
	char bufdata[WLC_IOCTL_MAXLEN];
	char CMD[CMD_MAXSIZE];
	struct ifreq ifr;
	wl_ioctl_t ioc;
	wl_pkt_filter_enable_t enable_parm;
	int ret = 0;
	int s = 0;

	if (net_socket.is_sys_exec) {
		memset(CMD, 0, CMD_MAXSIZE);
		snprintf(CMD, sizeof(CMD), "%s pkt_filter_enable 200 1", BRCM_TOOL);
		system(CMD);
		printf("CMD: %s\n", CMD);
		return 0;
	}

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("cannot open socket %s - %s\n", net_socket.iface, strerror(errno));
		return -1;
	}

	/* Init pkt_filter data */
	enable_parm.id = 200;
	enable_parm.enable = 1;

	wl_iovar_mkbuf("pkt_filter_enable", (char *)&enable_parm,
		sizeof(wl_pkt_filter_enable_t), bufdata, WLC_IOCTL_MAXLEN, &ret);

	if (ret) {
		printf("Failed to build buffer, %s.\n", __FUNCTION__);
		return -1;
	}

	ioc.cmd = WLC_SET_VAR;
	ioc.buf = &bufdata;
	ioc.len = WLC_IOCTL_MAXLEN;
	ioc.set = 1;

	strncpy(ifr.ifr_name, net_socket.iface, IFNAMSIZ);
	ifr.ifr_data = (caddr_t) &ioc;

	//printf("--- cmd: %d ---\n", ioc.cmd);
	if ((ret = ioctl(s, SIOCDEVPRIVATE, &ifr)) < 0) {
		printf("fail set pkt_filter_enable, %s\n", strerror(errno));
	}

	return ret;
}

#if 0
static int wl_pkt_filter_stats()
{
	char iface[PROP_VALUE_MAX];
	struct ifreq ifr;
	wl_ioctl_t ioc;
	wl_pkt_filter_stats_t *stats;
	uint32 id;
	char bufdata[WLC_IOCTL_MAXLEN];
	int ret = 0;
	int s;

	strncpy(iface, net_socket.iface, PROP_VALUE_MAX);

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("cannot open socket %s - %s\n", iface, strerror(errno));
		return -1;
	}

	//id =
	wl_iovar_mkbuf("pkt_filter_stats", (char *)&id, sizeof(id), bufdata, WLC_IOCTL_MAXLEN, &ret);
	if (ret) {
		return -1;
	}

	ioc.cmd = WLC_SET_VAR;
	ioc.buf = &bufdata;
	ioc.len = WLC_IOCTL_MAXLEN;
	ioc.set = 1;

	strncpy(ifr.ifr_name, iface, IFNAMSIZ);
	ifr.ifr_data = (caddr_t) &ioc;

	printf("--- cmd: %d ---\n", ioc.cmd);
	if ((ret = ioctl(s, SIOCDEVPRIVATE, &ifr)) < 0) {
		printf("fail set pkt_filter_stats, %s\n", strerror(errno));
	}
	printf("ret: %d", ret);

	stats = (wl_pkt_filter_stats_t *) bufdata;

	//stats->num_pkts_matched
	//stats->num_pkts_discarded
	//stats->num_pkts_forwarded

	return ret;
}
#endif

static int get_ap_beacon_interval(int *ap_beacon_interval)
{
	struct ifreq ifr;
	wl_ioctl_t ioc;
	int ret = 0;
	int s = 0;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("cannot open socket %s - %s\n", net_socket.iface, strerror(errno));
		return -1;
	}

	ioc.cmd = WLC_GET_BCNPRD;
	ioc.buf = ap_beacon_interval;
	ioc.len = 4;
	ioc.set = 0;

	strncpy(ifr.ifr_name, net_socket.iface, IFNAMSIZ);
	ifr.ifr_data = (caddr_t) &ioc;

	if ((ret = ioctl(s, SIOCDEVPRIVATE, &ifr)) < 0) {
		printf("fail get ap_beacon_interval, %s\n", strerror(errno));
	}

	return ret;
}

static int get_ap_dtim(int *ap_dtim)
{
	struct ifreq ifr;
	wl_ioctl_t ioc;
	int ret = 0;
	int s = 0;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("cannot open socket %s - %s\n", net_socket.iface, strerror(errno));
		return -1;
	}

	ioc.cmd = WLC_GET_DTIMPRD;
	ioc.buf = ap_dtim ;
	ioc.len = 4;
	ioc.set = 0;

	strncpy(ifr.ifr_name, net_socket.iface, IFNAMSIZ);
	ifr.ifr_data = (caddr_t) &ioc;

	if ((ret = ioctl(s, SIOCDEVPRIVATE, &ifr)) < 0) {
		printf("fail get ap_dtim, %s\n", strerror(errno));
	}

	return ret;
}

static int set_bcn_li_dtim(int bcn_li_dtim)
{
	char bufdata[WLC_IOCTL_MAXLEN];
	char CMD[CMD_MAXSIZE];
	struct ifreq ifr;
	wl_ioctl_t ioc;
	int ret = 0;
	int s = 0;

	if (net_socket.is_sys_exec) {
		memset(CMD, 0, CMD_MAXSIZE);
		snprintf(CMD, sizeof(CMD), "%s bcn_li_dtim %d", BRCM_TOOL, bcn_li_dtim);
		system(CMD);
		printf("CMD: %s\n", CMD);
		return 0;
	}

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("cannot open socket %s - %s\n", net_socket.iface, strerror(errno));
		return -1;
	}

	wl_iovar_mkbuf("bcn_li_dtim", (char *)&bcn_li_dtim,
		sizeof(bcn_li_dtim), bufdata, WLC_IOCTL_MAXLEN, &ret);

	if (ret) {
		printf("Failed to build buffer, %s.\n", __FUNCTION__);
		return -1;
	}

	ioc.cmd = WLC_SET_VAR;
	ioc.buf = &bufdata;
	ioc.len = sizeof(bufdata);
	ioc.set = 1;

	strncpy(ifr.ifr_name, net_socket.iface, IFNAMSIZ);
	ifr.ifr_data = (caddr_t) &ioc;

	if ((ret = ioctl(s, SIOCDEVPRIVATE, &ifr)) < 0) {
		printf("fail set bcn_li_dtim, %s\n", strerror(errno));
	}

	return ret;
}

static void set_dtim_interval(int dtim_interval)
{
	/* DTIM interval setting */
	int ap_beacon_interval = 100;
	int ap_dtim = 1;
	int sta_bcn_li_dtim = 3;

	get_ap_beacon_interval(&ap_beacon_interval);
	get_ap_dtim(&ap_dtim);
	if ((ap_beacon_interval == 0) || (ap_dtim == 0)) {
		printf("Get AP beacon error. ap_beacon_interval[%d], ap_dtim[%d]\n", ap_beacon_interval, ap_dtim);
		return ;
	}

	sta_bcn_li_dtim = (int)(dtim_interval / ( ap_beacon_interval * ap_dtim));
	if (sta_bcn_li_dtim > 0) {
		set_bcn_li_dtim(sta_bcn_li_dtim);
	}
	printf("ap_beacon_interval[%d], ap_dtim[%d], bcn_li_dtim[%d]. Result: DTIM Inteval is [%d] ms\n",
		ap_beacon_interval, ap_dtim, sta_bcn_li_dtim,
		(sta_bcn_li_dtim) ? (ap_beacon_interval * ap_dtim * sta_bcn_li_dtim):(ap_beacon_interval * ap_dtim));
}

#if 0
static u32 get_tcp_info(int sockfd, u32 *seq, u32 *ack)
{
	int ret = 0;
	int queue = 0;
	int repair = 0;

	int len = 4;
	socklen_t s_len = 4;

	do {
		/* open repair */
		repair = 1;
		ret = setsockopt(sockfd, IPPROTO_TCP, TCP_REPAIR, &repair, len);
		if(ret) {
			perror("setsockopt TCP_REPAIR");
			break;
		}

		/* seq */
		queue = TCP_SEND_QUEUE;
		ret = setsockopt(sockfd, IPPROTO_TCP, TCP_REPAIR_QUEUE, &queue, len);
		if(ret) {
			perror("setsockopt TCP_REPAIR_QUEUE");
			break;
		}
		ret = getsockopt(sockfd, IPPROTO_TCP, TCP_QUEUE_SEQ, seq, &s_len);
		if(ret) {
			perror("setsockopt TCP_QUEUE_SEQ");
			break;
		}

		/* ack */
		queue = TCP_RECV_QUEUE;
		ret = setsockopt(sockfd, IPPROTO_TCP, TCP_REPAIR_QUEUE, &queue, len);
		if(ret) {
			perror("setsockopt TCP_REPAIR_QUEUE");
			break;
		}
		ret = getsockopt(sockfd, IPPROTO_TCP, TCP_QUEUE_SEQ, ack, &s_len);
		if(ret) {
			perror("setsockopt TCP_QUEUE_SEQ");
			break;
		}

		/* close repair */
		repair = 0;
		ret = setsockopt(sockfd, IPPROTO_TCP, TCP_REPAIR, &repair, len);
		if(ret) {
			perror("setsockopt TCP_REPAIR");
			break;
		}
	} while (0);

	return ret;
}
#endif

static int fix_tcp_seq_ack(int sock_fd)
{
	int ret = -1;

	if (sock_fd < 0) {
		printf("Socket fd is zero\n");
		return ret;
	}
	/*
	ret = get_tcp_info(sock_fd, &seq, &ack);
	if (ret < 0) {
		printf("Get TCP info failed\n");
		return -1;
	}
	printf("Current seq[%u], ack[%u]\n", seq, ack);
	*/
	//if ((seq - net_socket.tcp_info_old.tcpka_sess_seq) > 0) {
	printf("fd [%d], Stop Wifi KeepAlive and get TCP info\n", sock_fd);
	wl_tcpka_conn_disable(1);
	wl_tcpka_conn_sess_info(1);
	wl_tcpka_conn_del(1);
	if (net_socket.is_enable_pattern) {
		wl_wowl_clear();
	}
	set_bcn_li_dtim(0);

	printf("Resume done: ipid[%u], seq_num[%u], ack_num[%u].\n",
		net_socket.tcp_info.tcpka_sess_ipid,
		net_socket.tcp_info.tcpka_sess_seq,
		net_socket.tcp_info.tcpka_sess_ack);

	printf("Modify TCP info start\n");
	ret = ioctl(sock_fd, SET_TCP_FIX, &net_socket.tcp_info);
	if (ret < 0) {
		perror("SET_TCP_FIX");
	} else {
		printf("Modify TCP info done\n");
	}
	//}

	return ret;
}

static void poll_wait_system_state(void)
{
#define POLL_TIMEOUT (10000) // 10s

	int ret = -1;
	int state_fd = -1;
	cpu_state curr_state = CPU_UNKNOWN;
	struct pollfd fds;

	state_fd = open(PROC_WIFI_STATE, O_RDWR);
	if (state_fd < 0) {
		perror("open" PROC_WIFI_STATE );
		return;
	}
	fds.fd = state_fd;
	fds.events = POLLIN;

	do {
		ret = poll(&fds, 1, POLL_TIMEOUT);
		if(ret == 0) {
			printf("Poll Time out\n");
		} else {
			ret = read(state_fd, &curr_state, sizeof(curr_state));
			if (ret < 0) {
				perror("read\n");
			} else {
				printf("System current state [%d]\n", curr_state);
				break;
			}
		}
	} while (0);

	close(state_fd);
	state_fd = -1;
}

static void transfer_data_after_resume(int sock_fd, int sleeps)
{
#define SELECT_TIMEOUT (30)

	int ret = 0;
	int max_fd = -1;
	fd_set fdset;
	struct timeval tv;
	cpu_state curr_state = CPU_UNKNOWN;
	char msg_a[32] = "Online";

	while (1) {
		FD_ZERO(&fdset);
		FD_SET(pipefd[0], &fdset);
		FD_SET(sock_fd, &fdset);
		max_fd = (pipefd[0] > sock_fd) ? pipefd[0] : sock_fd;

		tv.tv_sec = SELECT_TIMEOUT;
		tv.tv_usec = 0;

		ret = select(max_fd + 1, &fdset, NULL, NULL, &tv);
		if (ret < 0) {
			perror("select");
			break;
		} else if (ret == 0) {
			//printf("Select timeout\n");
			continue;
		}

		if (FD_ISSET(pipefd[0], &fdset)) {
			ret = read(pipefd[0], &curr_state, sizeof(curr_state));
			if (ret < 0) {
				perror("read pipe");
				break;
			}
			printf("Read pipe msg state [%d]\n", curr_state);
			if (curr_state == CPU_NORMAL) {
				ret = writen(sock_fd, msg_a, strlen(msg_a));
				if (ret == strlen(msg_a)) {
					printf("Send Length[%d] [%s] OK\n", ret, msg_a);
				}
			}
		}

		if (FD_ISSET(sock_fd, &fdset)) {
			if (curr_state != CPU_NORMAL) {
				printf("System is not in NORMAL mode");
				break;
			}
			memset(net_socket.recv_msg, 0, sizeof(net_socket.recv_msg));
			ret = read(sock_fd, net_socket.recv_msg, sizeof(net_socket.recv_msg));
			if ( ret == 0) {
				printf("==== disconnect ====\n");
				break;
			} else if (ret < 0) {
				perror("recv");
				break;
			} else {
				printf("Receive Server MSG: %s.\n", net_socket.recv_msg);
				ret = writen(sock_fd, net_socket.recv_msg, strlen(net_socket.recv_msg));
				if (ret == strlen(net_socket.recv_msg)) {
					printf("Send Length[%d] [%s] OK\n", ret, net_socket.recv_msg);
				}
			}
		}
	}

	printf("%s exit\n", __FUNCTION__);
	return;
}

static int net_socket_client_tcp(void)
{
	int fd_client = -1;
	int ret = -1;

	struct sockaddr_in addr_srv;

	if ((fd_client = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return -1;
	}
	net_socket.fd = fd_client;

	bzero(&addr_srv, sizeof(addr_srv));
	addr_srv.sin_family = AF_INET;
	addr_srv.sin_port = htons(net_socket.port);

	do {
		if (inet_pton(AF_INET, net_socket.srv_ip, &addr_srv.sin_addr) < 0 ) {
			perror("inet_pton");
			break;
		}
		if (connect(fd_client, (struct sockaddr *)&addr_srv, sizeof(struct sockaddr)) < 0) {
			perror("connect");
			break;
		}
		printf("Socket [%d], Host: %s:%d, Connect OK\n",
			fd_client, net_socket.srv_ip, net_socket.port);

		/* Send once */
		ret = writen(fd_client, net_socket.send_msg, strlen(net_socket.send_msg));
		if (ret == strlen(net_socket.send_msg)) {
			printf("Send Length[%d] OK\n", ret);
		}
	} while (0);

	if (net_socket.is_add_resume) {
		transfer_data_after_resume(net_socket.fd, net_socket.is_add_resume);
	}

	printf("%s exit\n", __FUNCTION__);
	close_fd(&fd_client);

	return ret;
}

static int net_socket_client_udp(void)
{
	int fd_client = -1;
	int ret = -1;

	struct sockaddr_in addr_srv;

	if ((fd_client = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return -1;
	}

	bzero(&addr_srv, sizeof(addr_srv));
	addr_srv.sin_family = AF_INET;
	addr_srv.sin_port = htons(net_socket.port);

	do {
		if (inet_pton(AF_INET, net_socket.srv_ip, &addr_srv.sin_addr) < 0 ) {
			perror("inet_pton");
			break;
		}
		printf("Socket [%d], Port [%d] OK\n", fd_client, net_socket.port);

		/* Send */
		ret = sendto(fd_client, net_socket.send_msg, strlen(net_socket.send_msg),
			0, (struct sockaddr *)&addr_srv, sizeof(addr_srv));
		if (ret == strlen(net_socket.send_msg)) {
			printf("Send Length[%d] OK\n", ret);
		}
	} while (0);

	printf("%s exit\n", __FUNCTION__);
	close_fd(&fd_client);

	return ret;
}

static void suspend_and_resume(void)
{
	int ret = 0;

	/* Prepare to suspend */
	net_socket.sys_state = CPU_SUSPEND;
	ret = write(pipefd[1], &net_socket.sys_state, sizeof(net_socket.sys_state));
	if (ret < 0) {
		perror("write pipe");
	}
	printf("Notify pipe SUSPEND mode\n");
	if (net_socket.is_enable_suspend) {
		printf("Go to suspend mode\n");
		printf("...\n");
		system(SUSPEND_CMD);
	}

	/* Resume back */
	if (net_socket.is_poll) {
		poll_wait_system_state();
	} else {
		/*  Wait system state change from respend to resume */
		sleep(1);
	}
	printf("Go to resume mode\n");
	printf("...\n");
	fix_tcp_seq_ack(net_socket.fd);
	net_socket.sys_state = CPU_NORMAL;
	ret = write(pipefd[1], &net_socket.sys_state, sizeof(net_socket.sys_state));
	if (ret < 0) {
		perror("write pipe");
	}
	printf("Notify pipe NORMAL mode\n");

}

void processPacket(u_char *arg, const struct pcap_pkthdr* pkthdr, const u_char * packet)
{
	int i = 0;

	int *counter = (int *)arg;
	char dst_mac[DATA_MAXSIZE];
	char tcp_wowl_pattern[1024];
	char udp_wowl_pattern[1024];

	struct ip *iphdr = NULL;		/* IPv4 Header */
	struct tcphdr *tcphdr = NULL;	/* TCP Header  */
	unsigned int *tsval;			/* Time Stamp (optional) */
	unsigned int *tsvalR;			/* Time Stamp Reply (optional) */

	tcpka_conn_t tcpka;

	printf("Packet Count: %d\n", ++(*counter));

	if (net_socket.verbose) {
		printf("Received Packet Size: %d\n", pkthdr->len);
		printf("Payload:\n");
		for(i = 0; i < pkthdr->len; ++i) {
			printf(" %02x", packet[i]);
			if( (i + 1) % 16 == 0 ) {
				printf("\n");
			}
		}
		printf("\n\n");
	}

	if (net_socket.is_tcp) {
		iphdr = (struct ip *)(packet + 14);
		tcphdr = (struct tcphdr *)(packet + 14 + 20);
		tsval = (unsigned int *)(packet + 58);
		tsvalR = tsval + 1;

		/* 1. filter PSH flag.
		 * 2. filter total size is 67 byte for when send one byte tcp data
		 * 3. filter one byte tcp data (*)
		 */
		if (tcphdr->psh && pkthdr->len == 67 && packet[66] == DEFAULT_MSG_HEX) {
			memset(dst_mac, 0, sizeof(dst_mac));
			sprintf(dst_mac, "%02X:%02X:%02X:%02X:%02X:%02X",
				packet[0], packet[1], packet[2], packet[3], packet[4], packet[5]);

			if (net_socket.verbose) {
				printf("	FLags: PSH [%d]\n", tcphdr->psh);

				printf("	DST MAC: %s\n", dst_mac);
				printf("	DST IP: %s\n", inet_ntoa(iphdr->ip_dst));
				printf("	SRC IP: %s\n", inet_ntoa(iphdr->ip_src));
				printf("	SRC PORT: %d\n", ntohs(tcphdr->th_sport));
				printf("	DST PORT: %d\n", ntohs(tcphdr->th_dport));
				printf("	ID: %d\n", ntohs(iphdr->ip_id));
				printf("	SEQ: %u\n", ntohl(tcphdr->th_seq));
				printf("	ACK: %u\n", ntohl(tcphdr->th_ack));
				printf("	Win: %d\n", ntohs(tcphdr->th_win));
				printf("	TS val: %u\n", ntohl(*tsval));
				printf("	TS valR: %u\n", ntohl(*tsvalR));
				printf("	Data: 0x%x\n", packet[66]);
			}

			/* Fill WiFi FW to KeepAlive and Wakeup Pattern, Session ID: 1 */
			tcpka.sess_id = 1;
			tcpka.dst_mac = (struct ether_addr)* ether_aton(dst_mac);

			memcpy(&tcpka.src_ip, &iphdr->ip_src.s_addr, IPV4_ADDR_LEN);
			memcpy(&tcpka.dst_ip, &iphdr->ip_dst.s_addr, IPV4_ADDR_LEN);

			tcpka.ipid = ntohs(iphdr->ip_id);
			tcpka.srcport = ntohs(tcphdr->th_sport);
			tcpka.dstport = ntohs(tcphdr->th_dport);
			tcpka.seq = ntohl(tcphdr->th_seq);
			tcpka.ack = ntohl(tcphdr->th_ack);
			tcpka.tcpwin = ntohs(tcphdr->th_win);
			tcpka.tsval = ntohl(*tsval);
			tcpka.tsecr = ntohl(*tsvalR);
			if (net_socket.is_add_tcp_payload) {
				tcpka.len = pkthdr->len - 66;
				tcpka.ka_payload_len = pkthdr->len - 66; // 66 is the lenght size of IP/TCP head
			} else {
				tcpka.len = 0;
				tcpka.ka_payload_len = 0;
			}
			tcpka.ka_payload[0] = DEFAULT_MSG_HEX;

			net_socket.tcp_info_old.tcpka_sess_ipid = tcpka.ipid;
			net_socket.tcp_info_old.tcpka_sess_seq = tcpka.seq;
			net_socket.tcp_info_old.tcpka_sess_ack = tcpka.ack;
			printf("Old TCP info: ipid[%u], seq[%u], ack[%u]\n",
				net_socket.tcp_info_old.tcpka_sess_ipid,
				net_socket.tcp_info_old.tcpka_sess_seq,
				net_socket.tcp_info_old.tcpka_sess_ack);

			memset(tcp_wowl_pattern, 0, sizeof(tcp_wowl_pattern));
			snprintf(tcp_wowl_pattern, sizeof(tcp_wowl_pattern),
				"0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"\
				"0000000000000000000000000000000000000000000000000000000077616b65",
				packet[30], packet[31], packet[32], packet[33],
				packet[26], packet[27], packet[28], packet[29],
				packet[36], packet[37], packet[34], packet[35]);

			wl_wowl_pattern(tcp_wowl_pattern);
			wl_tcpka_conn_add(tcpka);
			wl_tcpka_conn_enable(1);

			if (net_socket.is_enable_pattern) {
				wl_wowl();
				wl_wowl_activate();
			}

			set_dtim_interval(net_socket.dtim_interval);
			printf("Set TCP KeepAlive Done\n");

			if (net_socket.is_add_resume) {
				suspend_and_resume();
			}
		}
	} else {
		if (packet[42] == DEFAULT_MSG_HEX) {
			memset(udp_wowl_pattern, 0, sizeof(udp_wowl_pattern));
			snprintf(udp_wowl_pattern, sizeof(udp_wowl_pattern),
				"0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
				packet[30], packet[31], packet[32], packet[33],
				packet[26], packet[27], packet[28], packet[29],
				packet[36], packet[37], packet[34], packet[35]);

			wl_mkeep_alive(pkthdr->len, packet);

			if (net_socket.is_enable_pattern) {
				if (net_socket.is_wowl_udpka) {
					wl_wowl_pattern(udp_wowl_pattern);
					wl_wowl();
					wl_wowl_activate();
				} else {
					wl_pkt_filter_add(udp_wowl_pattern);
					pkt_filter_enable();
				}
			}

			set_dtim_interval(net_socket.dtim_interval);
			printf("Set UDP KeepAlive Done\n");
		}
	}
	return;
}

static void load_default_param(void)
{
	net_socket.is_session = 0;
	net_socket.is_get_tcp_info = 0;
	net_socket.is_close = 0;
	net_socket.is_tcp = 1;
	net_socket.is_add_resume = 0;
	net_socket.is_add_tcp_payload = 1;
	net_socket.is_enable_pattern = 1;
	net_socket.is_wowl_udpka = 0;
	net_socket.is_enable_suspend = 0;
	net_socket.is_poll = 0;
	net_socket.is_sys_exec = 0;
	net_socket.wifi_chip_id = WIFI_BCM43340;
	net_socket.sys_state = CPU_NORMAL;
	net_socket.verbose = 0;
	net_socket.port = DEFAULT_PORT;
	net_socket.interval = DEFAULT_INTERVAL;
	net_socket.dtim_interval = DEFAULT_DTIM_INTERVAL;

	memset(net_socket.iface, 0, sizeof(net_socket.iface));
	memset(net_socket.srv_ip, 0, sizeof(net_socket.srv_ip));
	memset(net_socket.client_ip, 0, sizeof(net_socket.client_ip));

	strncpy(net_socket.srv_ip, DEFAULT_HOST, sizeof(net_socket.srv_ip));
	strncpy(net_socket.send_msg, DEFAULT_MSG, sizeof(net_socket.send_msg));
	strncpy(net_socket.net_filter, DEFAULT_NET_FILTER, sizeof(net_socket.net_filter));
}

static void show_wifi_chip()
{
	char wifi_chip_str[32];

	switch(net_socket.wifi_chip_id) {
	case WIFI_BCM43340:
		sprintf(wifi_chip_str, "BCM43340");
		break;
	case WIFI_BCM43438:
		sprintf(wifi_chip_str, "BCM43438");
		break;
	default:
		sprintf(wifi_chip_str, "Unknown");
		break;
	}

	printf("WiFi Chip ID: %s\n", wifi_chip_str);
}

int main(int argc, char *argv[])
{
	int ret = -1;
	int count = 0;

	pthread_t tid = 0;
	pcap_t *descr = NULL;
	char *device = NULL;
	char errbuf[PCAP_ERRBUF_SIZE];
	struct bpf_program filter;

	if (argc < 2) {
		usage();
		return -1;
	}

	load_default_param();
	memset(errbuf, 0, PCAP_ERRBUF_SIZE);

	if (init_param(argc, argv) < 0) {
		usage();
		return -1;
	}

	printf("Date Version : %s\n", __TIME__);
	show_wifi_chip();

	if (net_socket.is_get_tcp_info) {
		wl_tcpka_conn_sess_info(1);
		return 0;
	}

	if (net_socket.iface[0] != '\0') {
		device = net_socket.iface;
	} else {
		if ( (device = pcap_lookupdev(errbuf)) == NULL) {
			printf("ERROR: %s\n", errbuf);
			return -1;
		}
	}

	printf("Opening interface: %s\n", device);

	if (pipe(pipefd) < 0) {
		perror("pipe");
		return -1;
	}
	printf("Pipe.0[%d], Pipe.1[%d]\n", pipefd[0], pipefd[1]);

	if ((descr = pcap_open_live(device, MAXBYTES2CAPTURE, 1, 512, errbuf)) == NULL) {
		printf("ERROR: %s\n", errbuf);
		return -1;
	}

	if (net_socket.net_filter[0] != '\0') {
		pcap_compile(descr, &filter, net_socket.net_filter, 1, 0);
		pcap_setfilter(descr, &filter);
		printf("Set Sniffer Network Filter: [%s]\n", net_socket.net_filter);
	}

	if (net_socket.is_session) {
		if (net_socket.is_tcp) {
			printf("Create TCP session\n");
			ret = pthread_create(&tid, NULL, (void *)net_socket_client_tcp, NULL);
			if (ret != 0) {
				printf("Create pthread TCP client error\n");
			}
		} else {
			printf("Create UDP session\n");
			ret = pthread_create(&tid, NULL, (void *)net_socket_client_udp, NULL);
			if (ret != 0) {
				printf("Create pthread UDP client error\n");
			}
		}
	}

	if (pcap_loop(descr, -1, processPacket, (u_char *)&count) == -1) {
		printf("ERROR: %s\n", pcap_geterr(descr) );
		return -1;
	}
	pcap_close(descr);

	return 0;
}
