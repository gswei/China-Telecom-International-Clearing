#ifndef	_INTERRUPT__H_
#define	_INTERRUPT__H_

#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <iostream.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <stdlib.h>
#include <strings.h>
#include <time.h>
#include <sys/stat.h>

#include "wrlog.h"

#ifdef __cplusplus
extern "C"
{
#endif//__cplusplus
        void Acc_Interrupt(int Signo);
#ifdef __cplusplus
}
#endif//__cplusplus

void Acc_Interrupt(int Signo);

void  DenyInterrupt(char *,char*);
void  AllowInterrupt(void);
bool  GetInterruptFlag(void);

#endif//_INTERRUPT__H_
