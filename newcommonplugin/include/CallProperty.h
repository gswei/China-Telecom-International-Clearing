/****************************************************************
 filename: CallProperty.h
 module: 用户自定义插件头文件
 created by:	ouyh
 create date:	2010-06-24
 version: 3.0.0
 description: 
	号码属性分析插件
 update:

 *****************************************************************/


#ifndef _CALLPROPERTY_H_
#define  _CALLPROPERTY_H_ 1

#include "CF_CPlugin.h"
#include "ComFunction.h"
#include "CallNumber.h"

/* C_CallingRegular Class */
class C_CallProperty: public BasePlugin
{
private:

	//用于输入的字段
	char m_szSourceGroupId[FIELD_LEN+1];
	char m_szServiceId[FIELD_LEN+1];
	char m_szServCatId[FIELD_LEN+1];
	char m_szIniPath[PATH_NAME_LEN+1];
	char m_szSourceId[FIELD_LEN+1];

	char m_szBefTollcode[FIELD_LEN+1];
	char m_szCallNbr[FIELD_LEN+1];//接入码+带0区号+号码（固话或手机号码）
	char m_szNbrType[FIELD_LEN+1];
	char m_szCdrBegin[FIELD_LEN+1];
	char m_szDealType[FIELD_LEN+1];

	//用于输出的字段

	//接入码
	char m_szBefTollTel[FIELD_LEN+1];
	//行政区编码
	char m_szDistrict[FIELD_LEN+1];
	//运营商
	char m_szBusiAfter[FIELD_LEN+1];
	char m_szBusiBefore[FIELD_LEN+1];
	//为0则取m_szBusiBefore的运营商作为号码的运营商标识
	char m_szBusiPriority[FIELD_LEN+1];
	//移动类型
	char m_szMobile[FIELD_LEN+1];
	//区号前业务类型
	char m_szSvrBefore[FIELD_LEN+1];
	//区号后业务类型
	char m_szSvrAfter[FIELD_LEN+1];	
	//区号
	char m_szTollcode[FIELD_LEN+1];

	int m_iAbnReason;
	char m_sAbnReason[FIELD_LEN+1];//无资料原因
	char m_szAbnField[FIELD_LEN+1];//无资料内容
	char m_LastSerCatId[FIELD_LEN+1];
	char m_LastSourceId[FIELD_LEN+1];
	char m_szCallNumber[FIELD_LEN+1];

	int initFlag;

	C_CallNumber m_callnumber;
	BaseAccessMem *table;

	int Init( PacketParser& pps );
	int getInputParams( PacketParser& pps );
	int sendOutputParams(ResParser& pps);
	int setParam();
	int getParam();

public:

	C_CallProperty();
	~C_CallProperty();
	void init(char *szSourceGroupID, char *szServiceID, int index);
	void execute(PacketParser& pps,ResParser& retValue);
	void message(MessageParser&  pMessage);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();

	//int execute(PacketParser& pps,ResParser& retValue);

};

#endif


