

void HAL_AT_Send(char *buf, int len)
{
	USART3_Wirte(buf, len);
}

void HAL_AT_Secv(char *c, int timeout)
{
	
}

