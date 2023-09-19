#ifndef __AT_COMMAND_H
#define __AT_COMMAND_H

#define AT_OK  		 0
#define AT_ERR      -1
#define AT_TIMEOUT  -2



int ATInit(void);
int ATSendCmd(char *buf,char *resp, int resp_len, int timeout);
void ATRecvParser(void * params);
void Task_ATTest(void *Parm);
int ATSendData(char *buf,int len, int timeout);
int ATReadData(char *c , int timeout);


#endif
