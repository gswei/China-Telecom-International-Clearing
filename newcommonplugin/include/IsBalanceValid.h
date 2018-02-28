/****************************************************************
 filename: IsBalanceValid.h
 module: �û��Զ�����ͷ�ļ�
 created by:	ouyh
 create date:	2010-06-24
 version: 3.0.0
 description: 
	�ж������Ч�����Ƿ�������ļ�ʱ��֮��
	����ǵĻ�����Ϊδ����0��
	������ڣ�����Ч�����������ڣ�����Ϊ1��
	���������������������Ϊ2
 update:
20080319   �����Ƿ��¹��ڣ�0��ʾ�����Ч�ڸ�����ļ�ʱ�䲻��ͬһ�����ڣ�
                             1��ʾ�����Ч�ڸ�����ļ�ʱ����ͬһ������
 *****************************************************************/
#ifndef C_BALANCE_ANALYZE_H
#define C_BALANCE_ANALYZE_H

#include "CF_CPlugin.h"
#include "ComFunction.h"

class C_IsBalanceValid : public BasePlugin
{
public:
	C_IsBalanceValid(){}
	~C_IsBalanceValid(){}
	
	void init(char *szSourceGroupID, char *szServiceID, int index);
	void execute(PacketParser& pps,ResParser& retValue);
	void message(MessageParser&  pMessage);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();
		
private:
	char input_time[RECORD_LENGTH+1];
	char valid_time[RECORD_LENGTH+1];
	
	int year_month_i, year_month_v;
	int day_i, day_v;
	string buff;
};



#endif

