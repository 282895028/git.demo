#ifndef __AT_COMMAND_H
#define __AT_COMMAND_H

#define AT_OK  		 0
#define AT_ERR      -1
#define AT_TIMEOUT  -2

void SetATStatus(int status);
int GetATStatus(int statuc);
int ATInit(void);
int ATSendCmd(char *buf, int len, char *resp, int resp_len, int timeout);
int GetSpecialATString(char *buf);
void ProcessSpecialATString(char *buf);
void ATReadPacket(char *buf, int len, int *resp_len, int timeout);
void ATRecvParser(void * params);
void Task_ATTest(void *Parm);


#endif
