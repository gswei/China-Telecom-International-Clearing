//2014-05-05 ����������Ӫ����֧ ���ʻ�ͨ������֧
//2014-09-22 �����������ļ����
//2014-11-07 ��Ԫ��2��������������

#include "gwCarrierAct.h"

CLog theJSLog;

int main(int argc,char** argv)
{
	cout<<"********************************************* "<<endl;
	cout<<"*    China Telecom. Telephone Network         "<<endl;
	cout<<"*    InterNational Account Settle System      "<<endl;
	cout<<"*                                             "<<endl;
	cout<<"*           jssettleBalance                   "<<endl;
	cout<<"*           sys.GJZW.Version 1.0	             "<<endl;
	cout<<"*    created time :      2014-05-05 by  hed   "<<endl;
	cout<<"*    last update time :  2014-11-07 by  hed	 "<<endl;
	cout<<"********************************************* "<<endl;

	CarrierAct bb;

	if(!bb.init(argc,argv))
	{
		return 1;
	}
	
	bb.dealType();

	return 0;
}



