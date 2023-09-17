
#include <stm32_uart3.h>


void HAL_AT_Send(char *buf, int len)
{
	USART3_Wirte(buf, len);
}

void HAL_AT_Secv(unsigned char *c, int timeout)
{
	/*从环形缓冲区读出数据*/
	/*无数据则阻塞*/
	USART3_Read(c, timeout);
}

