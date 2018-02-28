/****************************************************************
 filename: JLDXAnaDownChargeFlag.h
 module: 用户自定义插件头文件
 created by:	ouyh
 create date:	2010-06-24
 version: 3.0.0
 description: 
	20080121 修改成不返回无资料代码
 update:

 *****************************************************************/
#ifndef C_ANA_DOWN_CHARGEFLAG_H
#define c_ANA_DOWN_CHARGEFLAG_H

#include "CF_CPlugin.h"
#include "ComFunction.h"

class C_JLDXAnaDownChargeFlag: public BasePlugin
{
public:	
	C_JLDXAnaDownChargeFlag();
	~C_JLDXAnaDownChargeFlag();
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
	
	DataStruct in;
	DataStruct out;
};

#endif
