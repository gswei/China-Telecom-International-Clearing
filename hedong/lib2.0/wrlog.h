/********************************************************************
filename:wrlog.h
module:WF - write log
created by: chen bingji
create date: 2000-9-21
updatelist:
  update by wh 2000-12-27, change the env file path
  update by xiaow 2002-10-10 change the esql from esqlc from pro*c
version;1.1.1
description:
 headfile of wrlogmain.cc and wrlogproc.ec
 headfile of wrlogmain.pc
*******************************************************************/
#ifndef _wrlog_h_
#define _wrlog_h_


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

#define  MSGKEY  1234
#define ENVFILEPATHLEN	256

#define OPENLOGFILEERROR 112
#define TIMEPARAMETERERROR 113
#define TOLLCODEERROR  116
#define OPENENVERROR  114
#define ENVFILEERROR  115
#define MODUL_IDERROR 117
#define ERR_READ_LOGDIR	118
#define ERR_NOFOUND_ENV	119
#define ORACLE_CONNECT_ERROR 120
#define WRITE_INFLOG_ERROR 121
#define MSGSNDERROR  122
#define ERRLOG_PATH		"ERRLOG_DIR"
#define MSGLOG_PATH  "INFOPOINT_DIR"

#define EMPTY_FLAG	"__"
#define EMPTYCHAR_FLAG	'_'

struct msg_body {
    char pipeid[10];
    char moduleid[10];
    char errtime[15];
    char errtype[2];
    char errcode[10];
    char msgdata[BUFSIZ];
};
    
struct errlog_msg_st {
    long int msg_type;
    struct msg_body msgbody;
};

//function defined in wrlogmain.cc

int wrlog_setEnvPath(char *path);
//20050520 add by xiaoyh 
int WrLogQueue(char* pipeid,char* moduleid,char* errtype,char *errcode,char* errtime,char * buf);

int wrlog(char* tollcode,int opl_modul_id,char* opl_filename,char* starttime,
		char* endtime,long opl_total_count,long opl_mainflow_count,long 
		opl_lackinfo_count,long opl_error_count);
		
int wrlog(char* tollcode,char* gat_filename,char* gat_receivetime,char*
 		gat_endpoint,char gat_state,char gat_direction);

int wrlog(char* tollcode,int erl_modul_id,char* erl_filename,char erl_cat,char
 		erl_level,int erl_code1,int erl_code2,char* erl_time,char* erl_desc,char *errHappenFile=(char *)"",int errHappenLine=(-1));

int wrlog(char* tollcode,int erl_modul_id,char* erl_filename,char* erl_time,CF_CError & CError);

//add by zhoulh 20050704
//增加接口
int wrlog(char* tollcode,char* erl_modul_id,char* erl_filename,char erl_cat,char
 		erl_level,int erl_code1,int erl_code2,char* erl_time,char* erl_desc,char *errHappenFile=(char *)"",int errHappenLine=(-1));

int wrlog(char* tollcode,char* erl_modul_id,char* erl_filename,char* erl_time,CF_CError & CError);
//end add



int msglog(char *infoId,char *checkName,char *infoValue,char *objName,char *batch,char *begTime,char *endTime);

//function defined in wrlogproc.cc
int wrlogproc(int flag, char* buffer);
#endif
