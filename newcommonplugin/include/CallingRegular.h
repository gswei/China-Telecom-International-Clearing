/****************************************************************
 filename: CallNumber.h
 module: �û��Զ�����ͷ�ļ�
 created by:	ouyh
 create date:	2010-07-04
 version: 3.0.0
 description: 
            ��������ҵ��;�Ʒ�ҵ��Ʒѽ׶����к�����������ں�������
            ��Ҫ����: 
				��Ϻ������ϣ������С����С��ƷѵȺ���ͳһ����Ϊ"����+����"�ĸ�ʽ�����н����������ָ���ֶΡ�
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
	
//����������ֶ�
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


//����������ֶ�
	char m_szBefTollTel[FIELD_LEN+1];//������
	char m_szTollTel[FIELD_LEN+1];//��0����+���루�̻����ֻ����룩
	char m_szAfterTollTel[FIELD_LEN+1];	//���루�̻����ֻ����룩
	char m_szTollcode[FIELD_LEN+1];//��0����
	char m_szCallNumber[FIELD_LEN+1];//������+��0����+���루�̻����ֻ����룩

	int m_iAbnReason;
	char m_sAbnReason[FIELD_LEN+1];//������ԭ��
	char m_szAbnField[FIELD_LEN+1];//����������
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
 
