
#include "at_command.h"
#include "at_uart_hal.h"
#include <platform_mutex.h>
#include <string.h>
#include <stdio.h>
#include <ring_buffer.h>

#include "mqttclient.h"

#define AT_CMD_TIMOUT 1000
#define AT_RESP_LEN   100

static ring_buffer g_packet_buffer;

static platform_mutex_t at_ret_mutex;
static platform_mutex_t at_packet_mutex;

static int g_at_status;
static char g_at_resp[AT_RESP_LEN];
static char *g_cur_cmd;

/* status
 *   0 - ok
 *  -1 - err
 *  -2 - timeout
 */
void SetATStatus(int status)
{
	 g_at_status = status;
}

int GetATStatus(void)
{
	return g_at_status;
}
int ATInit(void)
{
	platform_mutex_init(&at_ret_mutex);
	platform_mutex_lock(&at_ret_mutex); //mutex = 0;
	
	platform_mutex_init(&at_packet_mutex);
	platform_mutex_lock(&at_packet_mutex); //mutex = 0;

	ring_buffer_init(&g_packet_buffer);
	
	return 0;
}
int ATSendData(char *buf,int len, int timeout)
{
	int ret;
	int err;
	/*发生AT命令*/
	HAL_AT_Send(buf, len);
	/* 等待结果
	 * 1 :成功得到mutex
	 * 0 :超时返回
	 */
	
	ret = platform_mutex_lock_timeout(&at_ret_mutex, AT_CMD_TIMOUT);
	if(ret)
	{
		/*判断返回值*/
		
		/*存储resp*/
		err = GetATStatus();
		return err;
	}
	else
	{
		return AT_CMD_TIMOUT;
	}
			
}


int ATReadData(unsigned char *c , int timeout)
{
	int ret;

	do{
		if(0 == ring_buffer_read(c , &g_packet_buffer))
			return AT_OK;
		else
		{
			ret = platform_mutex_lock_timeout(&at_packet_mutex, timeout);
			if(0 == ret)
				return AT_TIMEOUT;
		}
	}while(ret == 1);
	return 0;
}


/*eg. buf = "AT+CIPMPODE=1"
 *    timeout : ms
 */
int ATSendCmd(char *buf,char *resp, int resp_len, int timeout)
{
	int ret;
	int err;
	/*发生AT命令*/
	HAL_AT_Send(buf, strlen(buf));
	HAL_AT_Send("\r\n", 2);

	g_cur_cmd = buf;
	/* 等待结果
	 * 1 :成功得到mutex
	 * 0 :超时返回
	 */
	
	ret = platform_mutex_lock_timeout(&at_ret_mutex, AT_CMD_TIMOUT);
	if(ret)
	{
		/*判断返回值*/
		
		/*存储resp*/
		err = GetATStatus();
		if(!err && resp)
		{
			memcpy(resp, g_at_resp, resp_len > AT_RESP_LEN ? AT_RESP_LEN : resp_len);
		}
		return err;
	}
	else
	{
		return AT_CMD_TIMOUT;
	}
			
}

static int GetCIPSEMDResult(char *buf)
{
	if(g_cur_cmd && strstr(g_cur_cmd, "AT+CIPSEND=") && buf[0] =='>')
		return 1;
	else
		return 0;
}

static int GetSpecialATString(char *buf)
{
	if (strstr(buf, "+IPD,"))
		return 1;
	else
	    return 0;
}

static void ProcessSpecialATString(char *buf)
{
	int i =0;
	int len = 0;

	/* +IPD,n:xxxxxxxx */
	
	while(1)
	{
		/* 读出后续字符 */
		/* 解析出长度 */
		i = 0;
		while(1)
		{
			HAL_AT_Secv(&buf[i], (int)portMAX_DELAY);
			if(buf[i] == ':')
			{
				break;		
			}
			else
			{
				len = len *10 + (buf[i] - '0');
			}
			i++;
		}

		/* 读出真正的数据*/
		i = 0;
		while(i < len)
		{
			HAL_AT_Secv(&buf[i], (int)portMAX_DELAY);
			if(i < AT_RESP_LEN)
			{
				/* 把数据放入环形buffer */
				ring_buffer_write(buf[i], &g_packet_buffer);
				
				/* wake up */
				/* 解锁 */
				platform_mutex_unlock(&at_packet_mutex);
			}
				
			i++;
		}
	}
}

#if 0
int ATReadPacket(char *buf, int len, int *resp_len, int timeout)
{	int ret;
	ret = platform_mutex_lock_timeout(&at_packet_mutex, timeout);
	if (ret)
	{
		*resp_len =  len > g_at_packet_len? g_at_packet_len : len;
		memcpy(buf, g_at_packet, *resp_len);
		return AT_OK;
	}
	else
	{
		return AT_CMD_TIMOUT;
	}
		
}
#endif

void ATRecvParser(void * params)
{
	char buf[AT_RESP_LEN];
	int i = 0;
	
	while(1)
	{
		/*读取wifi模块发来的数据：使用阻塞方式*/
		HAL_AT_Secv(&buf[i], (int)portMAX_DELAY);
		/*解析结果*/
		/* 1. 何时解析?
	     * 1.1收到"\r\n" 
	     * 1.2收到特殊字符："+IPD,"
	     */
	     if(i && buf[i-1] == '\r' && buf[i] == '\n')
	     {
	     	/*得到了回车换行 */
			buf[i+1] = '\0';
			/* 2. 怎么解析 */
			if(strstr(buf, "OK\r\n"))
			{
				/*记录数据*/
				memcpy(g_at_resp, buf, i);
				SetATStatus(AT_OK);
				platform_mutex_unlock(&at_ret_mutex);
				i = 0;
			}
			else if (strstr(buf, "ERROR\r\n"))
		    {
				SetATStatus(AT_ERR);
				platform_mutex_unlock(&at_ret_mutex);
				i = 0;
	        }
			else if(GetCIPSEMDResult(buf))
			{
				SetATStatus(AT_ERR);
				platform_mutex_lock(&at_ret_mutex);
				i=0;
			}
			else if (GetSpecialATString(buf))
			{
				ProcessSpecialATString(buf);
				i = 0;
			}
			
		}
		 else
		 {
		 	i++;
		 }
		 if(i >= AT_RESP_LEN)
		 	i = 0;
	}
}
	

static void topic1_handler(void* client, message_data_t* msg)
{
    (void) client;
    MQTT_LOG_I("-----------------------------------------------------------------------------------");
    MQTT_LOG_I("%s:%d %s()...\ntopic: %s\nmessage:%s", __FILE__, __LINE__, __FUNCTION__, msg->topic_name, (char*)msg->message->payload);
    MQTT_LOG_I("-----------------------------------------------------------------------------------");
}

void MQTT_Client_Task(void *Parm)
{ 
    mqtt_client_t *client = NULL;
    mqtt_log_init();

    client = mqtt_lease();
	
    mqtt_set_port(client, "1883");
	
    mqtt_set_host(client, "192.168.199.148");
    mqtt_set_client_id(client, random_string(10));
    mqtt_set_user_name(client, random_string(10));
    mqtt_set_password(client, random_string(10));
    mqtt_set_clean_session(client, 1);

    if(0 != mqtt_connect(client))
	{
		printf("can not err\r\n");
		vTaskDelete(NULL);
	}
	
    mqtt_subscribe(client, "topic1", QOS0, topic1_handler);
    mqtt_subscribe(client, "topic2", QOS1, NULL);
    mqtt_subscribe(client, "topic3", QOS2, NULL);

	while(1){
			vTaskDelay(100);
		}
}


