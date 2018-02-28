/****************************************************************
 filename: CalledRegular.h
 module: 用户自定义插件头文件
 created by:	ouyh
 create date:	2010-06-24
 version: 3.0.0
 description: 
            修改规整后的正规号码原则是:
            被叫需要手机前去掉区号，异地手机前带0（根据主被叫判断）
		主叫手机号前去掉区号，不用补0
 update:

 *****************************************************************/
#ifndef _C_CALLMODIFY_H_
#define _C_CALLMODIFY_H_ 1

#include "CF_CPlugin.h"
#include "ComFunction.h"
#include "CallNumber.h"

class C_CallModify : public BasePlugin
{
public: 
	C_CallModify();
	~C_CallModify();
	void init(char *szSourceGroupID, char *szServiceID, int index);
	void execute(PacketParser& pps,ResParser& retValue);
	void message(MessageParser&  pMessage);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();

	int init();	
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
	char m_szServiceId[FIELD_LEN+1];
	char m_szSourceGroupId[FIELD_LEN+1];
	char m_szSourceId[FIELD_LEN];
	char m_szIniPath[PATH_NAME_LEN+1];
	char m_szServCatId[FIELD_LEN];
	char m_szCalling[FIELD_LEN];
	char m_szCalled[FIELD_LEN];

	//用于输出的字段
	char m_szMCalling[FIELD_LEN];
	char m_szMCalled[FIELD_LEN];

	int m_iAbnReason;
	char m_sAbnReason[FIELD_LEN];//无资料原因
	char m_szAbnField[FIELD_LEN];//无资料内容
	char m_LastSerCatId[FIELD_LEN];
	char m_LastSourceId[FIELD_LEN];
};

#endif


