

void HAL_AT_Send(char *buf, int len)
{
	USART3_Wirte(buf, len);
}

