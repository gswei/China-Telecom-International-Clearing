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

#include "MainFlow.h"
//#include "CF_CEnv.h"
#include "CF_CFscan.h"
#include "CF_CError.h"
//#include "CF_string.h"
#include "Common.h"
#include "config.h"
#include "COracleDB.h"
#include "CFmt_Change.h"
#include "CF_MemFileIO.h"
#include "ProcessRec_Mgr.h"
#include "C_PluginMethod.h"
#include "CF_Lack.h"
#include "CF_CErrorLackRec.h"
#include "wrlog.h"
#include "encrypt.h"
#include "CF_InfoPoint.h"
#include "interrupt.h"
#include "CF_CRecStat.h"
#include "CF_CDealLog.h"


#ifndef SZ2
#define SZ2
#endif


#define  ERR_GETENV					8001	////获取环境变量错误
#define	ERR_REQU_MEM				8002	//申请内存出错
#define	ERR_OPENFILE				8003	//打开文件出错
#define	ERR_SELECT  				8004	//数据库里找不到记录
#define  ERR_COLNAME_NOT_FINE		8006
#define  ERR_PK_DUPLICATE         8005  //数据表主键重复





int dealfile(struct SParameter &Param, struct SFileStruct &filestruct, int file, int ldStatFileNumber);

#endif

