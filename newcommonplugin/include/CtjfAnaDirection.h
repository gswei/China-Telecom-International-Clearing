/****************************************************************
 filename: CtjfAnaDirection.h
 module: 用户自定义插件头文件
 created by:	ouyh
 create date:	2010-07-13
 version: 3.0.0
 description: 
 update:

 *****************************************************************/
#ifndef _C_CTJFANADIRECTION_H_
#define _C_CTJFANADIRECTION_H_ 1

#include "CF_CPlugin.h"
#include "ComFunction.h"

class C_CtjfAnaDirection: public BasePlugin
{
public:	
	C_CtjfAnaDirection();
	~C_CtjfAnaDirection();
	void init(char *szSourceGroupID, char *szServiceID, int index);
	void execute(PacketParser& pps,ResParser& retValue);
	void message(MessageParser&  pMessage);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();

public:	
	BaseAccessMem *table;
	int m_iIndex;
	char table_RouteUseWay[TABLENAME_LEN+1];
	char table_RouteType[TABLENAME_LEN+1];
	int m_iTableRouteUseWayOffset;
	int m_iTableRouteTypeOffset;
	
	char m_szInRouteType[RECORD_LENGTH];
	char m_szInUseWay[RECORD_LENGTH];
	char m_szOutRouteType[RECORD_LENGTH];
	char m_szOutUseWay[RECORD_LENGTH];	
	char m_szDirectFlag[RECORD_LENGTH];
	char m_szType[RECORD_LENGTH];
	
	DataStruct m_InData;
	DataStruct m_OutData;
};

#endif

