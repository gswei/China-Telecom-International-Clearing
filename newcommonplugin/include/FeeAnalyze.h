/****************************************************************
 filename: C_FeeAnalyze.h
 module: �û��Զ�����ͷ�ļ�
 created by:	ouyh
 create date:	2010-07-13
 version: 3.0.0
 description: 
	1.	��#��Ϊ�ָ��������ֿ����е��˱�
	2.	���˱����Ͳ�Ϊ1001��1002��1003���˱���������
	3.	���˱������ڵ�������˱����䶯����Ϊʵ�տ۷�
	4.	���˱����С��0���˱�����realDelta=Balance-Delta>0����ʵ�տ۷�ΪrealDelta��
                               ��realDelta<0��ʵ�տ۷�Ϊ0
	5.	���������˱�����Ϊ1001���˱���ʵ�տ۷ѵĺͣ����Ϊ����ʵ�ʿ۷�
	6.	���������˱�����Ϊ1002��1003���˱�ʵ�տ۷ѵĺͣ����Ϊ����ʵ�ʿ۷�

 update:
20080318      ����һ���ж��ֶΣ������0�Ļ������ܳ���>0��delta
                                �����1�Ļ������ܳ���<0��delta
20080326      �޸���������Ļ�����#��β���µ�����
              �����ж�ÿһ����Ϣ�ĳ����Ƿ���ȷ
 *****************************************************************/
#ifndef C_FEE_ANALYZE_H
#define C_FEE_ANALYZE_H

#include "CF_CPlugin.h"
#include "ComFunction.h"

class C_FeeAnalyze : public BasePlugin
{
public:
	C_FeeAnalyze(){}
	~C_FeeAnalyze(){}
	
	void init(char *szSourceGroupID, char *szServiceID, int index);
	void execute(PacketParser& pps,ResParser& retValue);
	void message(MessageParser&  pMessage);
	std::string getPluginName();
	std::string getPluginVersion();
	void printMe();

protected:
	int char2vector( string in , vector<string> &out, string sep);

private:
	char buff[RECORD_LENGTH+1];
	
	int fee_org;
	int fee_add;
	int realDelta;
	
	vector<string> input_group;
	
	int mode;
	char lackinfo[RECORD_LENGTH+1];
};



#endif

