#ifndef __PUBLIC_HEAD_H
#define __PUBLIC_HEAD_H

#include <sys/ioctl.h>
#include <net/if.h>

#include "log.h"
#include "fileio.h"

#define DIS_PORT	9527
#define DIS_IP	"127.0.0.1"
#define QUEUE_MAX	100

#define IF_NAME	"eth0"

typedef enum
{
	cmd_connect = 0,
	cmd_backconnect,
	cmd_unconnect,
	cmd_register,
	cmd_unregister,
	cmd_heart,
	cmd_max
} command;

typedef struct __serverinfo
{
	int id;
	char ip[16];
	in_port_t port;
}serverinfo;

typedef struct __messages
{
	command cmd;
	int arglen;
	unsigned char argv[0];
}message;

typedef struct __connectback
{
	int back;
	int id;
}connectback;

#define handle_info(msg) \
        do { write_logfile(SC_LOG_INFO, stderr, \
            "file:%s line:%d errorno:%d message:%s", \
            __FILE__, __LINE__, errno, msg); \
            } while (0)

#define handle_info(msg) \
        do { write_logfile(SC_LOG_INFO, stderr, \
            "file:%s line:%d errorno:%d message:%s", \
            __FILE__, __LINE__, errno, msg); \
            } while (0)

#define handle_warning(msg) \
        do { write_logfile(SC_LOG_WARNING, stderr, \
            "file:%s line:%d errorno:%d message:%s", \
            __FILE__, __LINE__, errno, msg); \
            } while (0)

#define handle_error(msg) \
        do { write_logfile(SC_LOG_ERROR, stderr, \
            "file:%s line:%d errorno:%d message:%s", \
            __FILE__, __LINE__, errno, msg); \
            exit(errno); } while (0)

static inline int get_local_ip(char * ifname, char * ip)
{
	char *temp = NULL;
	int inet_sock;
	struct ifreq ifr;

	inet_sock = socket(AF_INET, SOCK_DGRAM, 0);

	memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
	memcpy(ifr.ifr_name, ifname, strlen(ifname));

	if (0 != ioctl(inet_sock, SIOCGIFADDR, &ifr))
	{
		perror("ioctl error");
		return -1;
	}

	temp = inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr);
	memcpy(ip, temp, strlen(temp));

	close(inet_sock);

	return 0;
}

#endif  //__PUBLIC_HEAD_H
