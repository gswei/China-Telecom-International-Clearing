/****************************************************************
 filename: AnaCallDirect.h
 module: 用户自定义插件头文件
 created by:	ouyh
 create date:	2010-06-24
 version: 3.0.0
 description: 
	20080121 修改成不返回无资料代码
 update:

 *****************************************************************/

#ifndef C_AnaCallDirect_H
#define C_AnaCallDirect_H

#include "CF_CPlugin.h"
#include "ComFunction.h"

bool checkFormat(const char *cmpString,const char *format);

class C_AnaCallDirect : public BasePlugin
{
public: 
	C_AnaCallDirect();
	~C_AnaCallDirect();
	//int init();	
	
	void init(char *szSourceGroupID, char *szServiceID, int index);
	void execute(PacketParser& pps,ResParser& retValue);
	void message(MessageParser&  pMessage);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();

/*
	int execute(BufferParser& bps,ResParser& retValue){	return 0;}
	int execute(PacketParser& pps,ResParser& retValue);
	char* execute( PacketParser& pps ){ return "true"; };*/
private:
	char m_szServiceId[RECORD_LENGTH+1];
//	char m_szIniPath[RECORD_LENGTH+1];
	char m_szSourceGroupId[RECORD_LENGTH+1];
	char m_szServerId[RECORD_LENGTH+1];
	int iVersion;
	int iIndex;

	char m_szTableName[TABLENAME_LEN+1];
	int m_iTableOffset;

	//int m_ProcessId;
	char SERV_CAT_ID[RECORD_LENGTH+1];
	//int sync_index_plugin_tollcode;

//	CommonMemClient* table_tollcode;
	BaseAccessMem *table;
	
	//记录相关信息
	char tollcode1[RECORD_LENGTH+1];
	char tollcode2[RECORD_LENGTH+1];
	
	char region1[RECORD_LENGTH+1];
	char region2[RECORD_LENGTH+1];
	
	DataStruct in;
	DataStruct out;
};






#endif


