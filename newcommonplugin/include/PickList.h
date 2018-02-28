 /****************************************************************
 filename: PickList.h
 module: 用户自定义插件头文件
 created by:	ouyh
 create date:	2012-12-17
 version: 3.0.0
 description: 
	通用查询
 update:

 *****************************************************************/

#ifndef _PICKLIST_H_
#define _PICKLIST_H_  1
 
#include <sys/time.h>

#include "CF_CPlugin.h"
#include "CF_CException.h"
#include "CF_Common.h"
#include "ComFunction.h"

//#define ERR_LACK_PARAM      5003

//const int MAX_TABLE_NUM = 256;

class C_PickList: public BasePlugin
{
public:	
	C_PickList();
	void init(char *szSourceGroupID, char *szServiceID, int index);
	void execute(PacketParser& pps,ResParser& retValue);
	void message(MessageParser&  pMessage);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
	~C_PickList();

private:
	DataStruct inData;
	DataStruct outData;

private:
	int dealMethod;

	char m_szServiceId[RECORD_LENGTH+1];
	char m_szIniPath[RECORD_LENGTH+1];
	char m_szSourceGroupId[RECORD_LENGTH+1];
	char m_szServerId[RECORD_LENGTH+1];;

	BaseAccessMem *table;
	
};


#endif


