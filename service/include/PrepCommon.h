/****************************************************************
filename: PrepCommon.h
module:预处理共有的模块
created by: ouyh
create date: 
version: 3.0.0
description:
    综合结算--话单分析类
*****************************************************************/

#ifndef _PREP_COMMON_H_
#define _PREP_COMMON_H_ 1

#include "CF_Common.h"
#include "CF_CPluginPacket.h"
#include "PrepStructDef.h"
#include "CF_CFscan.h"
#include <stdlib.h>
#include "psutil.h"
#include "RTInfo.h"
//#include "tp_log_code.h"

#define LACK_SAVE_TABLE    "LACK_SAVE_TABLE_ID"
#define LACK_STAT_TABLE    "LACK_STAT_TABLE_ID"
//#define COMMEM_ENV_FILE    "commonmem.env"
//#define SEM_PATH			"SEM_PATH"
//#define SHM_PATH			"SHM_PATH"
//#define LOG_DIR				"LOG_DIR"
//#define SERVER				"SERVER"
//#define PLUGIN_ENV_NAME		"PREP_PLUGIN"
//#define SOUCE_FILETYPE_ID	"FILETYPE_ID"

struct SRecordCount
{
	int iTotalNum, iRightNum, iLackNum, iErrorNum, iPickNum, iOtherNum;
	char szSerCatId[5+1];

	SRecordCount()
	{
		iTotalNum = 0;
		iRightNum = 0;
		iLackNum = 0;
		iErrorNum = 0;
		iPickNum = 0;
		iOtherNum = 0;
		memset(szSerCatId, 0, sizeof(szSerCatId));
	};
};

class C_DealCalculator
{
private:
	vector<SRecordCount> vRecordPacket;
	vector<SRecordCount>::iterator it_Num;
	DBConnection conn;//数据库连接
	SRecordCount recordCount;

	char szLogStr[LOG_MSG_LEN+1];

	int m_TotalNum, m_RightNum, m_LackNum, m_ErrorNum, m_PickNum, m_OtherNum;
	long long m_TotalFee, m_RightFee, m_LackFee, m_ErrorFee, m_PickFee, m_OtherFee;
	long long m_TotalTime, m_RightTime, m_LackTime, m_ErrorTime, m_PickTime, m_OtherTime;
	char outSqlStr[SQL_LEN+1]; 
public:
	C_DealCalculator();
	~C_DealCalculator();
	//vector<SRecordCount> vRecordPacket;
	//char outSqlStr[SQL_LEN+1];
	int clear();
	int check();
	int set(const char* servCat, int total, int right, int lack, int error, int pick, int other);
	int setFee(long long total, long long right, long long lack, long long error, long long pick, long long other);
	int setTime(long long total, long long right, long long lack, long long error, long long pick, long long other);
	//int updateTable(const char* tableName, const char* sourceId, const char* fileName, const char* startTime);
	int updateTable(const char* tableName, struct SFileStruct &filestruct, int proc_index);
	int updateTable(const char* tableName, char *filename, char *dealstarttime, char *sourceID,int proc_index);
	int getTotal();
	int getRight();
	int getLack();
	int getError();
	int getPick();
	int getOther();
	long long getTotalFee();
	long long getRightFee();
	long long getLackFee();
	long long getErrorFee();
	long long getPickFee();
	long long getOtherFee();
	long long getTotalTime();
	long long getRightTime();
	long long getLackTime();
	long long getErrorTime();
	long long getPickTime();
	long long getOtherTime();
	char *getSqlstr();
};

bool SearchAllFiles(const char *Pathname, const char *Format, vector<string> &vecFileSet);

int getPrePartID(int PartID);
int getNextPartID(int PartID);
int getPrePartID(char* PartID);
int getNextPartID(char* PartID);

#endif

