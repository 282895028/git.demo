
#ifndef _STM32_UART3_H
#define _STM32_UART3_H

void USART3_Wirte(char *buf, int len);
void UART3_Lock_Init(void);
void USART3_Read(unsigned char *buf, int timeout);


#endif

