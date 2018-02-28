/****************************************************************
 filename: CallNumber.h
 module: 用户自定义插件头文件
 created by:	ouyh
 create date:	2010-07-04
 version: 3.0.0
 description: 
            用于整合业务长途计费业务计费阶段主叫号码规整，用于后续分析
            主要功能: 
				结合号码资料，对主叫、被叫、计费等号码统一规整为"区号+号码"的格式，被叫接入码输出到指定字段。
 update:

 *****************************************************************/

#ifndef _CALLINGREGULAR_H_
#define _CALLINGREGULAR_H_	1
 
#include "CF_CPlugin.h"
//#include "C_AccessShm.h"
#include "CallNumber.h"

/* C_CallingRegular Class */
class C_CallingRegular: public BasePlugin
{
private:
	C_CallNumber m_callnumber;
	BaseAccessMem *table;

	int Init( PacketParser& pps );
	int getInputParams( PacketParser& pps );
	int sendOutputParams(ResParser& pps);
	int setParam();
	int getParam();
	
//用于输入的字段
	char m_szSourceGroupId[FIELD_LEN+1];
	//int m_iProcessId;
	char m_szServiceId[FIELD_LEN+1];
	char m_szServCatId[FIELD_LEN+1];
	char m_szSourceId[FIELD_LEN+1];
	char m_szIniPath[PATH_NAME_LEN+1];
	char m_szCallNbr[FIELD_LEN+1];
	char m_szNbrType[FIELD_LEN+1];
	char m_szCdrBegin[FIELD_LEN+1];
	char m_szDealType[FIELD_LEN+1];
	char m_szDefTollcode[FIELD_LEN+1];


//用于输出的字段
	char m_szBefTollTel[FIELD_LEN+1];//接入码
	char m_szTollTel[FIELD_LEN+1];//带0区号+号码（固话或手机号码）
	char m_szAfterTollTel[FIELD_LEN+1];	//号码（固话或手机号码）
	char m_szTollcode[FIELD_LEN+1];//带0区号
	char m_szCallNumber[FIELD_LEN+1];//接入码+带0区号+号码（固话或手机号码）

	int m_iAbnReason;
	char m_sAbnReason[FIELD_LEN+1];//无资料原因
	char m_szAbnField[FIELD_LEN+1];//无资料内容
	char m_LastSerCatId[FIELD_LEN+1];
	char m_LastSourceId[FIELD_LEN+1];

	int initFlag;

public:
	
	C_CallingRegular();
	~C_CallingRegular();
	void init(char *szSourceGroupID, char *szServiceID, int index);
	void execute(PacketParser& pps,ResParser& retValue);
	void message(MessageParser&  pMessage);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();

	//int execute(PacketParser& pps,ResParser& retValue);
	int reOutputNum(PacketParser & pps,ResParser& retValue);
	
};

#endif
 
