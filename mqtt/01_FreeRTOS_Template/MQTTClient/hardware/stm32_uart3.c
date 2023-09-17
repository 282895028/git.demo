
#include "driver_usart.h"
#include "driver_key.h"
#include <stdio.h>
#include <platform_mutex.h>
#include <ring_buffer.h>
#include <at_uart_hal.h>


static ring_buffer uart3_buffer;
static platform_mutex_t uart_recv_mutex;
extern UART_HandleTypeDef huart3;

void USART3_IRQHandler(void)
{
	/* 如果发生RX中断
	 * 把数据读出来，存入环形buff	
     */
	uint32_t isrflags   = READ_REG(huart3.Instance->SR);
	uint32_t cr1its     = READ_REG(huart3.Instance->CR1);
	char c;
	/* UART in mode Receiver -------------------------------------------------*/
    if (((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
    {
      c = huart3.Instance->DR;
	  ring_buffer_write(c, &uart3_buffer);
	  platform_mutex_unlock(&uart_recv_mutex);
      return;
    }
}
void UART3_Lock_Init(void)
{

	platform_mutex_init(&uart_recv_mutex);
	platform_mutex_lock(&uart_recv_mutex); //mutex = 0;
	ring_buffer_init(&uart3_buffer);
}


void USART3_Wirte(char *buf, int len)
{
	int i = 0;
	while (i< len)
	{
		huart3.Instance->DR = buf[i];
		while ((huart3.Instance->SR & USART_SR_TXE) ==0);
		i++;
	}
}

void USART3_Read(unsigned char *buf, int timeout)
{
	while(1)
	{
		if(0 == ring_buffer_read(buf, &uart3_buffer))
			return;
		else
		{
			platform_mutex_lock_timeout(&uart_recv_mutex, timeout);
		}
	}

}


