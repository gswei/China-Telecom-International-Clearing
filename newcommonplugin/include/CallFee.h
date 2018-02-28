/****************************************************************
 filename: CallFee.h
 module: 用户自定义插件头文件
 created by:	ouyh
 create date:	2010-07-04
 version: 3.0.0
 description: 
            主要功能: 计费免费分析
 update:

 *****************************************************************/
#ifndef _C_CALLFEE_H_
#define _C_CALLFEE_H_	1
 
#include "CF_CPlugin.h"
//#include "C_AccessShm.h"
#include "CallNumber.h"

class C_CallFee: public BasePlugin
{

private:
	C_CallNumber m_callnumber;
	BaseAccessMem *table;

	int Init( PacketParser& pps );
	int getInputParams( PacketParser& pps );
	int sendOutputParams(ResParser& pps);
	int setParam();
	int getParam();

	int initFlag;

	//用于输入的字段
	char m_szSourceGroupId[FIELD_LEN+1];
	char m_szServiceId[FIELD_LEN+1];
	char m_szServCatId[FIELD_LEN+1];
	char m_szSourceId[FIELD_LEN+1];
	char m_szBefTollcode[FIELD_LEN+1];
	char m_szCallNbr[FIELD_LEN+1];
	char m_szNbrType[FIELD_LEN+1];
	char m_szCdrBegin[FIELD_LEN+1];

	//用于输出的字段
	char m_szChargeFlag[FIELD_LEN+1];//接入码

	int m_iAbnReason;
	char m_sAbnReason[FIELD_LEN+1];//无资料原因
	char m_szAbnField[FIELD_LEN+1];//无资料内容

	char m_LastSerCatId[FIELD_LEN+1];
	char m_LastSourceId[FIELD_LEN+1];

public:
	C_CallFee();
	~C_CallFee();
	void init(char *szSourceGroupID, char *szServiceID, int index);
	void execute(PacketParser& pps,ResParser& retValue);
	void message(MessageParser&  pMessage);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
};




#endif


 
