/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2020-01-10 23:45:59
 * @LastEditTime: 2020-04-25 17:50:58
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
#include "mqtt_log.h"
#include "platform_net_socket.h"
#include "at_command.h"

#define TEST_SSID   "Programmer"
#define TEST_PASSWD "100asktech"

/* return :<0 , err
 * 0 : ok
 */
int platform_net_socket_connect(const char *host, const char *port, int proto)
{
	int err;
	char cmd[100];
	/* 1.配置 WIFI模式*/
	err = ATSendCmd("AT+CWMODE=3",NULL, 0, 2000);
	if(err)
	{
		printf("AT+CWMODE=3 err = %d\n", err);
		return err;
	}
	/* 2. 连接路由器*/
	err = ATSendCmd("AT+CWJAP=\"" TEST_SSID "\",\""TEST_PASSWD "\"",NULL, 0, 2000);
	if(err)
	{
		printf("connect AP err = %d\n", err);
		return err;
	}
	/* 3. *连接到服务器*/
	if(proto == PLATFORM_NET_PROTO_TCP)
	{
		sprintf(cmd, "AT+CIPSTART=\"TCP\" ,\"%s\",%d",host, port);
		//AT+CIPSTART="TCP" , "192.168.3.116" ,8088
	}
	else
	{
		sprintf(cmd, "AT+CIPSTART=\"UDP\" ,\"%s\",%d",host, port);
	}
	err = ATSendCmd(cmd,NULL, 0, 2000);
	if(err)
	{
		printf("%s err = %d\n", cmd,err);	
	}
	return 0;
}

#if 0 
int platform_net_socket_recv(int fd, void *buf, size_t len, int flags)
{
    return 0;
}
#endif

/* 返回得到的字节数 */
int platform_net_socket_recv_timeout(int fd, unsigned char *buf, int len, int timeout)
{
	int i = 0;
	int err;
	/* 读数据，失败则阻塞*/
	while (i < len)
	{
		err = ATReadData(&buf[i], timeout);
		if(err)
		{
			return 0;	
		}
	}
	return len;
}

#if 0
int platform_net_socket_write(int fd, void *buf, size_t len)
{
	return 0;
}
#endif

int platform_net_socket_write_timeout(int fd, unsigned char *buf, int len, int timeout)
{
	int err;
	char cmd[10];

	sprintf(cmd, "AT+CIPSEND=%d", len);
	err = ATSendCmd(cmd,NULL, 0, 2000);
	if(err)
	{
		printf("%s err = %d\n", cmd,err);
		return err;
	}
	
	err = ATSendData((char *)buf, len, 2000);
	if(err)
	{
		printf("ATSendData err = %d\n", err);
		return err;
	}
	return 0;
}

int platform_net_socket_close(int fd)
{
	return ATSendCmd("AT+CIPCLOSE", NULL, 0, 2000);	
}

#if 0
int platform_net_socket_set_block(int fd)
{
	return 0;
}


int platform_net_socket_set_nonblock(int fd)
{
	return 0;
}

int platform_net_socket_setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
	return 0;
}
#endif

