/****************************************************************
 filename: CtjfAnaRoute.h
 module: 用户自定义插件头文件
 created by:	ouyh
 create date:	2010-07-13
 version: 3.0.0
 description: 
	20080121 修改成不返回无资料代码
 update:

 *****************************************************************/
#ifndef _C_CTJFANAROUTE_H_
#define _C_CTJFANAROUTE_H_

#include "CF_CPlugin.h"
#include "ComFunction.h"

class	C_CtjfAnaRoute: public BasePlugin
{
private:
public:
	C_CtjfAnaRoute();
	~C_CtjfAnaRoute();	
	void init(char *szSourceGroupID, char *szServiceID, int index);
	void execute(PacketParser& pps,ResParser& retValue);
	void message(MessageParser&  pMessage);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();

private:	
	BaseAccessMem *table;
	char m_szTableName[TABLENAME_LEN+1];
	int m_iTableOffset;
	int m_iIndex;
	DataStruct m_InData;
	DataStruct m_OutData;
};

#endif

