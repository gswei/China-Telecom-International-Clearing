/****************************************************************
filename: dealfile.h
module:
created by: zou guodong
create date: 2005-03-31
version: 1.0.0
description:
    �����ļ�������ͷ�ļ�
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


#define  ERR_GETENV					8001	////��ȡ������������
#define	ERR_REQU_MEM				8002	//�����ڴ����
#define	ERR_OPENFILE				8003	//���ļ�����
#define	ERR_SELECT  				8004	//���ݿ����Ҳ�����¼
#define  ERR_COLNAME_NOT_FINE		8006
#define  ERR_PK_DUPLICATE         8005  //���ݱ������ظ�





int dealfile(struct SParameter &Param, struct SFileStruct &filestruct, int file, int ldStatFileNumber);

#endif

