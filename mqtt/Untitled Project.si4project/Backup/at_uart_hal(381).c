
#ifndef _AT_UART_H
#define _AT_UART_H
void HAL_AT_Send(char *buf, int len)
{
	USART3_Wirte(buf, len);
}

void HAL_AT_Secv(char *c, int timeout)
{
	/*从环形缓冲区读出数据*/
	/*无数据则阻塞*/
	USART3_Read(c, timeout);
}

#endif
