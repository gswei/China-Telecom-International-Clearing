#include "FileLoad.h"

//CDatabase DBConn;
//CLog theLog;

//2013-07-05 ����sql������ص�ϵͳ����ά��̬
//2013-07-08 ��������ƽ̨����,��ϵͳ������һ���ļ�����ͬ������������ϵͳ
//2013-07-31 D_SCH_LOAD��file_id
//2013-08-16 ������������ÿ��ɨ������ԴĿ¼����ָ�������ļ�������¸�����Դ,���ӻ�ȡpertri��״̬
//2013-08-24 ��������ƽ̨,�����ݿ�ĸ��²���д�ļ�(���ø���ӿ�)
//2013-08-30 ȥ������ƽ̨
//2013-09-02 ��������ƽ̨,sqlд�ļ�,���ݲ�����ʱ��������,����·�����������д���
//2013-11-04 ��������ȷ������ݿ�״̬�л�״̬�Ĺ���,��ϵͳ������ϵͳ��״̬,Rʱ������ϵͳ,W���б�ϵͳ
//2013-12-01 �ٲ÷��������뻰�����,״̬��-1
//2013-12-06 ����vector�����ͷ��ڴ�,Ŀǰ����ָ����ʽ
//2013-12-09 sqlֻдinsert,��update

//CReadIni theCfgFile;
CLog theJSLog;

int main(int argc,char** argv)
{
	cout<<"********************************************* "<<endl;
	cout<<"*    China Telecom. Telephone Network         "<<endl;
	cout<<"*    InterNational Account Settle System      "<<endl;
	cout<<"*                                             "<<endl;
	cout<<"*           jsload							 "<<endl;
	cout<<"*           sys.GJZW.Version 1.0	             "<<endl;
	cout<<"*     created time :      2013-07-01 by  hed  "<<endl;
	cout<<"*     last update time :  2013-12-09 by  hed	 "<<endl;
	cout<<"**********************************************"<<endl;



	FileLoad load ;
	if(!load.init(argc,argv)) return -1;

	load.run();
	
    return 0;

}

