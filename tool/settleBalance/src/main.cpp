//2013-12-25 �����ô�����ʹ��� �����Ͷ��ŵ�̯��ƽ��
//2014-01-07 ������ʹ�������ʱ �����ܸ��ı�D_GWSF_SHARE_RESULT
//2014-04-24 ��������������,����������ֶα�ʾ�����µ�
//2014-05-05 �����Ļ���ȡֵ�б� ȥȡȥ��תȥ dir=2,3
//2014-05-06 ̯�ֵ�ʡ�Ľ������2λС��

#include "Balance.h"

CLog theJSLog;

int main(int argc,char** argv)
{
	cout<<"********************************************* "<<endl;
	cout<<"*    China Telecom. Telephone Network         "<<endl;
	cout<<"*    InterNational Account Settle System      "<<endl;
	cout<<"*                                             "<<endl;
	cout<<"*           jssettleBalance                   "<<endl;
	cout<<"*           sys.GJZW.Version 1.0	             "<<endl;
	cout<<"*    created time :      2013-12-23 by  hed   "<<endl;
	cout<<"*    last update time :  2014-05-05 by  hed	 "<<endl;
	cout<<"********************************************* "<<endl;

	Balance bb;

	if(!bb.init(argc,argv))
	{
		return 1;
	}
	
	bb.run();

	return 0;
}
