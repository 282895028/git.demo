
#include "at_command.h"
#include "at_uart_hal.h"
#include <platform_mutex.h>


#define AT_CMD_TIMOUT 1000
#define AT_RESP_LEN   100


static platform_mutex_t at_ret_mutex;
static platform_mutex_t at_packet_mutex;

static int g_at_status;
static char g_at_resp[AT_RESP_LEN];
static char g_at_packet[AT_RESP_LEN];
static int g_at_packet_len;


/* status
 *   0 - ok
 *  -1 - err
 *  -2 - timeout
 */
void SetATStatus(int status)
{
	 g_at_status = status;
}

int GetATStatus(int statuc)
{
	return g_at_status;
}
void ATInit(void)
{
	platform_mutex_init(&at_ret_mutex);
	platform_mutex_lock(&at_ret_mutex); //mutex = 0;
	
	platform_mutex_init(&at_packet_mutex);
	platform_mutex_lock(&at_packet_mutex); //mutex = 0;
}


/*eg. buf = "AT+CIPMPODE=1"
 *	  len = strlen(buf);
 *    timeout : ms
 */
int ATSendCmd(char *buf, int len, char *resp, int resp_len, int timeout)
{
	int ret;
	int err;
	/*发生AT命令*/
	HAL_AT_Send(buf, len);
	HAL_AT_Send("\r\n", 2);
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

int GetSpecialATString(char *buf)
{
	if (strstr(buf, "+IPD,"))
		return 1;
	else
		return 0;
}

void ProcessSpecialATString(char *buf)
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
			HAL_AT_Secv(&buf[i], portMAX_DELAY);
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
			HAL_AT_Secv(&buf[i], portMAX_DELAY);
			if(i < AT_RESP_LEN)
			{
				g_at_packet[i] = buf[i];	
				g_at_packet_len = i;
			}
				
			i++;
		}
		/* 解锁 */
		platform_mutex_unlock(&at_packet_mutex);
	}
}

int ATReadPacket(char *buf, int len, int *resp_len, int timeout)
{	int ret;
	ret = platform_mutex_lock_timeout(&at_packet_mutex, timeout);
	if (ret)
	{
		*resp_len =  len > g_at_packet_len? g_at_packet_len : len;
		memcpy(buf, g_at_packet_len, *resp_len);
		return AT_OK;
	}
	else
	{
		return AT_CMD_TIMOUT;
	}
		
}

void ATRecvParser(void * params)
{
	char buf[AT_RESP_LEN];
	int i = 0;
	
	while(1)
	{
		/*读取wifi模块发来的数据：使用阻塞方式*/
		HAL_AT_Secv(&buf[i], portMAX_DELAY);
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
			}
			else if (strstr(buf, "ERROR\r\n"))
		    {
				SetATStatus(AT_ERR);
				platform_mutex_unlock(&at_ret_mutex);
	        }
			else if (GetSpecialATString(buf))
			{
				ProcessSpecialATString(buf);
				i = -1;
			}
			i++;
			
	}
}

void Task_ATTest(void *Parm)
{
	int ret;
	while(1)
	{
		ret = ATSendCmd("AT", 2, NULL, 0, 1000);
		printf("ATSendCmd ret = %d\r\n", ret);
	}
	
}


