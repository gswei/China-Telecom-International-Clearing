/****************************************************************
 filename: CalledRegular.h
 module: �û��Զ�����ͷ�ļ�
 created by:	ouyh
 create date:	2010-06-24
 version: 3.0.0
 description: 
            �޸Ĺ�������������ԭ����:
            ������Ҫ�ֻ�ǰȥ�����ţ�����ֻ�ǰ��0�������������жϣ�
		�����ֻ���ǰȥ�����ţ����ò�0
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


	//����������ֶ�
	char m_szServiceId[FIELD_LEN+1];
	char m_szSourceGroupId[FIELD_LEN+1];
	char m_szSourceId[FIELD_LEN];
	char m_szIniPath[PATH_NAME_LEN+1];
	char m_szServCatId[FIELD_LEN];
	char m_szCalling[FIELD_LEN];
	char m_szCalled[FIELD_LEN];

	//����������ֶ�
	char m_szMCalling[FIELD_LEN];
	char m_szMCalled[FIELD_LEN];

	int m_iAbnReason;
	char m_sAbnReason[FIELD_LEN];//������ԭ��
	char m_szAbnField[FIELD_LEN];//����������
	char m_LastSerCatId[FIELD_LEN];
	char m_LastSourceId[FIELD_LEN];
};

#endif


