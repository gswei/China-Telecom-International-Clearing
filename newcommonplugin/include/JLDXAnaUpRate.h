/****************************************************************
 filename: JLDXAnaUpRate.h
 module: 用户自定义插件头文件
 created by:	ouyh
 create date:	2010-07-13
 version: 3.0.0
 description: 
 update:

 *****************************************************************/
#ifndef C_ANA_UP_RATE_H
#define C_ANA_UP_RATE_H

#include "CF_CPlugin.h"
#include "ComFunction.h"

class C_JLDXAnaUpRate: public BasePlugin
{
public:	
	C_JLDXAnaUpRate();
	~C_JLDXAnaUpRate();
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
