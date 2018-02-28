/****************************************************************
filename: dealfile.h
module:
created by: ouyh
create date: 
version: 3.0.0
description:
    话单文件处理函数
*****************************************************************/
#ifndef _DEALFILE_H_
#define _DEALFILE_H_ 1

#include <stdio.h>
#include <iostream.h>
#include <stdlib.h>
//#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>
#include <map>

#include "CF_Config.h"
#include "CF_CFscan.h"
#include "CF_Cerrcode.h"
#include "CF_Common.h"
#include "CF_COracleDB.h"
#include "CF_CMemFileIO.h"
#include "CF_CErrorLackRec.h"
#include "CF_CLogger.h"
//#include "CF_CEncrypt.h"
#include "CF_PREP_Error.h"
#include "PrepCommon.h"
#include "PrepStructDef.h"

const long MAX_TIME_IN_PLUGIN = 500;
//const int rec_len       = 5000;               /*每条记录的最大长度*/   
const char OFFSET_FIELD_NAME[] = "offset";

/*
struct SRec
{
	char record[RECLEN+1];
	SRec()
	{
		memset(record,0,sizeof(record));
	}
};*/

/*
struct SRename
{
	char m_szOutTmpFileName[FILE_NAME_LEN+PATH_NAME_LEN+1];
	char m_szOutRealFileName[FILE_NAME_LEN+PATH_NAME_LEN+1];
	char szOutputFiletypeId[5+1];    //输出文件类型
	char szOutrcdType;          //输出文件记录类型
	SRename()
	{
		memset(m_szOutTmpFileName, 0, sizeof(m_szOutTmpFileName));
		memset(m_szOutRealFileName, 0, sizeof(m_szOutRealFileName));
		memset( szOutputFiletypeId, 0, sizeof(szOutputFiletypeId) );
		szOutrcdType = 'c';
	}
};
*/

class CDealedFileInfo
{
public:
	char m_szOutTmpFileName[FILE_NAME_LEN+PATH_NAME_LEN+1];
	char m_szOutRealFileName[FILE_NAME_LEN+PATH_NAME_LEN+1];
	char szOutputFiletypeId[5+1];    //输出文件类型
	char szOutrcdType;          //输出文件记录类型

	char szFileName[FILE_NAME_LEN+1];
	char szSourceId[SOURCE_ID_LEN+1];
	char szDealStartTime[14+1];
	char szDealEndTime[14+1];

	CDealedFileInfo();
	~CDealedFileInfo();
	void clear();
};


class CDealFile
{
public:
	CDealFile();
	~CDealFile();
	int dealfile(struct SParameter &Param, struct SFileStruct &FileStruct, PacketParser* pps, ResParser* res);
	//int endfile(int iFileIndex, vector<SAlterRecordAfterDeal> &vecMod);
	int endfile(vector<SAlterRecordAfterDeal> &vecMod, InfoLog &info, bool OutputFile);
	int commit(bool OutputFile);
	int rollback();
private:
	vector<CDealedFileInfo> vecFile;
	//vector<SRec> vecRec;
	vector<string> vecRec;
	C_DealCalculator dealRcdNum;
};

#endif

