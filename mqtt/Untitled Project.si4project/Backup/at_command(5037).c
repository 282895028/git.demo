
#define AT_CMD_TIMOUT 1000
//static SemaphoreHandle_t xSemaphore = NULL;
static platform_mutex_t at_ret_mutex;

int ATInit(void)
{
	platform_mutex_init(&at_ret_mutex);
}

/*eg. buf = "AT+CIPMPODE=1"
 *	  len = strlen(buf);
 *    timeout : ms
 */
int ATSendCmd(char *buf, int len,char *resp, int timeout)
{
	int ret;
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
	}

	
}

