/****************************************************************
 filename: AccessMem.h
 module:
 created by:
 create date:
 version: 3.0.0
 description:

 update:

 *****************************************************************/
#ifndef _PLUGIN_ACCESS_MEM_H_
#define _PLUGIN_ACCESS_MEM_H_ 1

#include <string>
#include <sys/time.h>
#include "CF_Common.h"
#include "CF_CPluginMessage.h"
#include "CF_CommemClient.h"
#include "CF_PREP_Error.h"
#include "CommonMemClient.h"
#include "psutil.h"

const int MAX_COLUMN = 30;          //输入输出最大个数
const int MAX_TABLE_NUM = 256;

struct tableIndex
{
	char szName[TABLENAME_LEN+1];
	int iIndex;
	tableIndex()
	{
		memset(szName, 0, sizeof(szName));
		iIndex = 0;
	}
	tableIndex(char* name, int index)
	{
		strcpy(szName, name);
		iIndex = index;
	}
	friend bool operator < (const struct tableIndex &ls, const struct tableIndex &rs);
};

class C_AccessMem : public BaseAccessMem
{
private:
	char m_serverId[11];
	char m_iniPath[PATH_NAME_LEN+1];
//	CommonMemClient* table[MAX_TABLE_NUM];
//	map<string, int> tableInfo;
//	map<string, CommonMemClient*> tableMap;
	tableStruct *tableInfo;
	map<tableIndex, int> tableSearchType;
	int m_iTableNum;
	C_NRWLock m_oRWLock;
	S_MemManager* m_pCommonMem;
	DBConnection conn;//数据库连接
	
	//struct timeval start, finish;
	//long long timePart1, timePart2;
	int searchNum;
public:
	C_AccessMem();
	~C_AccessMem();
	//int Init(char* serverId, CBindSQL &sql,char* iniPath);
	int init(char* serverId, char* iniPath);
	int LoadOrUpdate();
	//int getData(char* tableName, const DataStruct* in_DataStruct, DataStruct* out_DataStruct, int iSearchIndex);
	int getColNum(char* tableName);
	int getSearchType(char* tableName, int iTableIndex);
	int getTableOffset(char* tableName);
	int checkSearchIndex(char* tableName, int iSearchIndex);
	int getData(int tableOffset, const DataStruct* in_DataStruct, DataStruct* out_DataStruct, int iSearchIndex=1);
	void printTime();
};

#endif
