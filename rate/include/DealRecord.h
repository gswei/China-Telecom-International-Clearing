/****************************************************************
filename: dealfile.h
module:
created by: zou guodong
create date: 2005-03-31
version: 1.0.0
description:
    话单文件处理函数头文件
*****************************************************************/

#ifndef _DEALFILE_H_
#define _DEALFILE_H_ 1

#include <stdio.h>
#include <iostream.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>
#include <map>
#include "psutil.h"
//#include "Formula.h"

#include "settlerule.h"
//#include "tp_log_code.h"
const int MAX_OUT_RECORD_COUNT = 10;	//最大分单数量


int dealRecord(struct SParameter &Param, PacketParser& ps,ResParser& retValue ,CF_MemFileO &pAuditFile,CF_MemFileO *pDispFiles,char *ratecycle);
int ReplaceStr(char *sSrc, char *sMatchStr, char *sReplaceStr, char *sDest);
#endif


