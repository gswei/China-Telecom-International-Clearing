/****************************************************************
 filename: JLDXAnaCpId.h
 module: 用户自定义插件头文件
 created by:	ouyh
 create date:	2010-07-13
 version: 3.0.0
 description: 
 update:

 *****************************************************************/
#ifndef C_ANA_CP_ID_H
#define C_ANA_CP_ID_H 

#include "CF_CPlugin.h"
#include "ComFunction.h"

class C_JLDXAnaCpId: public BasePlugin
{
public:	
	C_JLDXAnaCpId();
	~C_JLDXAnaCpId();
	void init(char *szSourceGroupID, char *szServiceID, int index);
	void execute(PacketParser& pps,ResParser& retValue);
	void message(MessageParser&  pMessage);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
private:
	BaseAccessMem *table;
	int m_iTableOffset;
	int m_iIndex;
	char m_szTableName[TABLENAME_LEN+1];
	
	DataStruct in;
	DataStruct out;
};

#endif
