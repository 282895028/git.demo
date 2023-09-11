
#define AT_CMD_TIMOUT 1000
#define AT_RESP_LEN   100




static platform_mutex_t at_ret_mutex;
static int g_at_status;
static char g_at_resp[];
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
int ATInit(void)
{
	platform_mutex_init(&at_ret_mutex);
	platform_mutex_lock(&at_ret_mutex); //mutex = 0;
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
		return 
	}
		

	
}

