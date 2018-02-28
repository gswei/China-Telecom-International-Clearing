/***************************************************
filename: Common.h
description:
    define some common functions used by other module

create date: 2005-04-01
****************************************************/

#ifndef _COMMON_H_
#define _COMMON_H_ 1

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <iostream.h>
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>
#include <memory.h>
#include <math.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>

#include "COracleDB.h"

//const char ENV_FILE_NAME[]     = "zhjs.env";
//const char DBSERVER_ENV_NAME[] = "SID";
//const char DBUSER_ENV_NAME[]   = "UNAME";
//const char DBPASS_ENV_NAME[]   = "UPASS";
#define ENV_FILE_NAME "zhjs.env"
#define DBSERVER_ENV_NAME "SID"
#define DBUSER_ENV_NAME "UNAME"
#define DBPASS_ENV_NAME "UPASS"

const int  MAX_RECORD_LENGTH   = 2000;
const int  MAX_LINE_LENGTH     = 5000;
const int  ERROR_MSG_LEN       = 1024;
const int  LOG_MSG_LEN         = 256;
const int  SQL_LEN             = 512;
const int  DATETIME_LEN        = 14;
const int  FILE_NAME_LEN       = 256;
const int  PATH_NAME_LEN       = 256;
const int  USER_NAME_LEN       = 256;
const int  USER_PASS_LEN       = 256;
const int  TABLENAME_LEN       = 30;
const int  ERR_DATE_FORMAT     = -9999;
//add by weixy 20081201
const int  ERR_TIME_FORMAT     =-9998;
//end add by weixy 20081201

extern pthread_mutex_t g_logMutex;

void delSpace(char *ss, int ss_len);
int strncspn(const char* str, int c, int nIdx);
int strrncspn(const char* str, int c, int nIdx);

void expTrace(const char *szDebugFlag, const char *szFileName,
              int lineno, const char *msg, ...);
bool getField(long field, char sep, const char *strin, char *strout);
bool getField(int field, char sep, const char *strin, int &out);
bool getField(int field, char sep, const char *strin, long &out);

int chkDir( char *_dir );
int chkAllDir(char *path);
int completeDir(char *pchPath);
int dispartFullName(char *pchFullName, char *pchPath, char *pchFileName);
int copyFile(char *pchSrcFile,char *pchTgtFile);
long fgotoLine(FILE *fp, long nIndex, int nLength = -1);

bool checkDate(const char *pchString);
void getCurTime(char *cur_Time);
int getCurDate(char *pchDate);
long timeStr2Time(const char* timeStr);
char* time2TimeStr(long ret, char *timeStr);
char* addDays(int nDays, const char* pchOrgDate, char* pchTgtDate);
int minusDays(const char* pchFirstDate, const char* pchSecondDate);
//add by weixy 20081201
int minusTimes(const char* pchFirstTime, const char* pchSecondTime);
bool checkTime(const char *pchString);
//end add by weixy 20081201

int isProcExist(int nPid);
int isProcExist(const char* pchKeyWord);

int setLogPath(const char* pchPath, const char* pchPipeID = NULL);
int connectDB(char *pchEnvFile, CDatabase &db);
//int getEnvFromDB(CDatabase &db, const char *pchPipeID, const char *pchVarName, char *pchVarValue);
int getEnvFromDB(CDatabase &db, const char *pchPipeID, const int pchProcessID, const char *pchVarName, char *pchVarValue);
int getEnv(const char *fname, const char *comp, char *ret );

void initDaemon(char *cEnvPath);
int Update_ProSTS(int JobID,char Deal_STS,char* szProcess_log = NULL);
#endif
