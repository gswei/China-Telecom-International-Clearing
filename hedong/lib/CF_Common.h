/***************************************************
filename: Common.h
description:
    define some common functions used by other module

create date: 2010-04-30
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
#include <ctype.h>

#include "CF_Config.h"
#include "CF_COracleDB.h"
#include "CF_CReadIni.h"

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
const int  MAX_FIELD_COUNT                =   100;  //
const int  FMT_MAX_FIELD_LEN                   =   100;  //

#define STR_DATETIME_LENGTH 15
#define STR_DATE_LENGTH 9
#define STR_ERROR_MSG_LENGTH 1024
#define STR_BUF_LENGTH 255
#define MAX_PATH_LENGTH 255
#define DBSERVER_ENV_NAME "SID"
#define DBUSER_ENV_NAME "UNAME"
#define DBPASS_ENV_NAME "UPASS"

class CF_CCommon
{
public:
	CF_CCommon();
	~CF_CCommon();
	static char* getVersion();
};

int connectDB(char *pchEnvFile, CDatabase &db);
int getEnvFromDB(const char *pchService, const  char *pchSourceGroup,const  char *pchSourceID, const char *pchVarName, char *pchVarValue);
void delSpace(char *ss, int ss_len);
int strncspn(const char* str, int c, int nIdx);
int strrncspn(const char* str, int c, int nIdx);
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
int toUpper(char *sstr);
int toLower(char *sstr);
void toUpper(string& buff);
void toLower(string& buff);
void initDaemon(bool bDaemonFlag);
int safeStrLen(const char* cpSrc);
int trimRight(char *s);
int trimLeft(char *s);
int trim(char *s);
char* fixStrLength(char *szSrc, unsigned int nLength, char ch);
int splitString(char *szSrc, const char *szSeparator, vector<string> &vecResultStr, bool bSkipSpace);
void string_replace(string & strBig, const string & strsrc, const string &strdst) ;

#endif
