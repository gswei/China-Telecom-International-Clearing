/********************************************************************
filename:infolog.h
module:write infomation log
created by: zhangguoqiang
create date: 2007-10-9
updatelist:
         2008-03-06     ÐÞ¸Ä×Ö¶ÎBUFF,40000
version;1.0.1
description:
 headfile of  infolog.h
*******************************************************************/
#ifndef _INFO_LOG_H_
#define _INFO_LOG_H_ 1

#include "COracleDB.h"

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "CF_CError.h"
#include "Common.h"

#define  INFOLOG  (char*)"INFO_DETECT"
#define  BUSINESSNAME (char*)"BUSINESSNAME"
#define  INFOLOG_OPEN_ENV_ERROR  601
#define  INFOLOG_GET_ENV_ERROR   602
#define  INFOLOG_MDIR_ERROR      603
#define  INFOLOG_WRITEFILE_ERROR 604
#define  INFOLOG_OPENFILE_ERROR  605
#define  INFOLOG_MAX_LEN         40000

int infoLog_setEnvPath(char *path);

int configInfoLog(int,int ,char *,char *,char *,char *,char *);

int configInfoLog(int ,char *,char *,char *,char *,char *,char *);


int dataInfoLog(int ,int ,char *,char *,char *,int ,int ,int ,int ,int ,int ,int ,int ,int ,int ,int );
int dataInfoLog(int ,int ,char *,char *,char *,int ,int ,int ,int ,int ,int ,int ,int ,int ,int ,int ,CBindSQL &);

int collectLog(int ,int ,char *,char *,int ,char *);

int transInfoLog(char *,int ,char *,char *,int ,char *);


#endif
