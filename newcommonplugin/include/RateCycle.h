/****************************************************************
 filename: RateCycle.h
 module: 用户自定义插件头文件
 created by:	ouyh
 create date:	2010-07-04
 version: 3.0.0
 description: 
	获取业务所属账期
 update:

 *****************************************************************/

#ifndef _RATECYCLE_H_
#define _RATECYCLE_H_	1

#include "CF_CPlugin.h"
#include "CF_CException.h"
#include "CF_Common.h"
#include "ComFunction.h"
//#include "/home/zhjs/work/sunh/src/CommonMemClient.h"

class C_RateCycle: public BasePlugin
{
public:
	C_RateCycle();
	~C_RateCycle();
	void init(char *szSourceGroupID, char *szServiceID, int index);
	void execute(PacketParser& pps,ResParser& retValue);
	void message(MessageParser&  pMessage);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
		
private:
	BaseAccessMem *table;

	int m_iIndex;
	char m_szSourceId[RECORD_LENGTH+1];
	
	DataStruct inData;
	DataStruct outData;

	char* getNextMonth(char* thisMonth);
};

#endif

