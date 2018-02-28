/****************************************************************
 filename: AnaCallDirect.h
 module: �û��Զ�����ͷ�ļ�
 created by:	ouyh
 create date:	2010-07-13
 version: 3.0.0
 description: 
	1 ������� SWLHAnaCalledDirect:ͨ������������
	2 �ӿڹ���
	  ���ݱ������ţ�ȡ��TOLLCODE����ѯ���й�����-���з����Ӧ��ϵ��(SWLH_REGION_DIRECT)��ȡ���з���
	  �����������ˮ��ʶ��Serv_cat_id ����
	  ��������з���
	3 ƥ�����
	  SWLH_REGION_DIRECT���¼���й�����-���з����Ӧ��ϵ�����б��й�����ȡֵ������
	  a������ʡ���б��������ţ���ȷƥ�䣩
	  b��һ��0��ͷ�����ţ���������������a��Χ�ڣ���Ҫƥ�䵽�˼�¼��
	  c�����ֹ������ţ�ģ��ƥ�䣩
	  d��00*��c����δ�����Ĺ������Ų���Ҫƥ�䵽�˼�¼��
	  e��*��������������еĲ�ƥ�䵽�ˣ�
 update:

 *****************************************************************/

#ifndef SWLH_ANA_CALLED_DIRECT_H
#define SWLH_ANA_CALLED_DIRECT_H

#include "CF_CPlugin.h"
#include "ComFunction.h"

class C_SWLHAnaCalledDirect : public BasePlugin
{
public:
	C_SWLHAnaCalledDirect();
	~C_SWLHAnaCalledDirect();
	
	void init(char *szSourceGroupID, char *szServiceID, int index);
	void execute(PacketParser& pps,ResParser& retValue);
	void message(MessageParser&  pMessage);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();

protected:
	
private:
	BaseAccessMem *table;
	char m_szTableName[TABLENAME_LEN+1];
	int m_iTableOffset;
	int m_iIndex;
	DataStruct in;
	DataStruct out;
	
	char tollcode[RECORD_LENGTH+1];
};



#endif

