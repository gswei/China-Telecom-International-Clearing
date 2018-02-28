 /****************************************************************
 filename: Comquery.h
 module: 用户自定义插件头文件
 created by:	ouyh
 create date:	2010-06-24
 version: 3.0.0
 description: 
	通用查询
 update:

 *****************************************************************/

#ifndef _COMQUERY_H_
#define _COMQUERY_H_  1
 
#include <sys/time.h>

#include "CF_CPlugin.h"
#include "CF_CException.h"
#include "CF_Common.h"
#include "ComFunction.h"

#define ERR_LACK_PARAM      5003

const int MAX_TABLE_NUM = 256;

/*
struct tableVersion
{
	string tableName;
	int Version;
	tableVersion()
	{
		tableName.clear();
		Version=0;
	};
	tableVersion(string a, int b)
	{
		tableName = a;
		Version = b;
	};
	friend bool operator < (const tableVersion x,  const tableVersion y);
};
*/

class C_Comquery: public BasePlugin
{
public:	
	C_Comquery();
	void init(char *szSourceGroupID, char *szServiceID, int index);
	void execute(PacketParser& pps,ResParser& retValue);
	void message(MessageParser&  pMessage);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
	~C_Comquery();

private:
//	CommonMemClient* table[MAX_TABLE_NUM];
//	map<tableVersion, CommonMemClient*> tableMap;
//	map<tableVersion, int> tableIndex;
//	int m_tableNum;
	DataStruct inData;
	DataStruct outData;

private:
	int dealMethod;

	char m_szServiceId[RECORD_LENGTH+1];
	char m_szIniPath[RECORD_LENGTH+1];
	char m_szSourceGroupId[RECORD_LENGTH+1];
	char m_szServerId[RECORD_LENGTH+1];;

	BaseAccessMem *table;

/*
	unsigned long executeTimeP1;
	unsigned long executeTimeP2;
	unsigned long executeTimeP3;
	unsigned long executeTimeP4;
	unsigned long executeTimeP5;
	unsigned long executeTime1;
	unsigned long executeTime2;
	unsigned long queryTime;
	unsigned long executeCounter;
	unsigned long queryCounter;
	*/

//	string strIn;

//	map<string, TableIndex> mapTableIndex;
//	map<string, TableIndex>::iterator ite;

//	typedef map<string, TableIndex>::value_type valueType;
	
};


#endif


