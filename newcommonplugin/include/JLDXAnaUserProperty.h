/****************************************************************
 filename: JLDXAnaUserProperty.h
 module: 用户自定义插件头文件
 created by:	ouyh
 create date:	2010-07-13
 version: 3.0.0
 description: 
	20080121 修改成不返回无资料代码
 update:

 *****************************************************************/
#ifndef C_ANA_USER_PROPERTY_H
#define C_ANA_USER_PROPERTY_H

#include "CF_CPlugin.h"
#include "ComFunction.h"

class C_JLDXAnaUserProperty: public BasePlugin
{
public:	
	C_JLDXAnaUserProperty();
	~C_JLDXAnaUserProperty();
	void init(char *szSourceGroupID, char *szServiceID, int index);
	void execute(PacketParser& pps,ResParser& retValue);
	void message(MessageParser&  pMessage);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();

private:
	BaseAccessMem *table;
	int m_iTableOffset;
	char m_szTableName[TABLENAME_LEN+1];
	int m_iIndex;
	
	DataStruct in;
	DataStruct out;
};

#endif
