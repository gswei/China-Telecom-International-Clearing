
//2013-07-08 �������ֵĴ���ģʽ
//2013-07-16 �����������ļ�ͷʱ����Ҫ���к�+1�������ļ�����ʱ��Ŀ¼��Ӧ�ý�����Ŀ¼YYYYMM/DD
//2013-07-19 �ļ����кŴ����ݿ�sequece��ȡ�ã���֤Ψһ����־·������ȴӺ��Ĳ�����ȡ�������ڴ���־�ӿ�
			 //��ʽУ����󵥶���������룬������д��ϸ������Ϣ
//2013-07-26 ͷβУ���ʽ�����ֶ�SPEC_FLAG��������־�������ڣ���Ҫ��������д��ÿ����¼���õ��ֶ�98,�ڷ���Ϣʱ����
//2013-07-30  ���ȱ�����file_id�ֶΣ�������ģ��ʹ��
//2013-08-01  C_FILE_RECEIVE_ENV�� �����ֶν�ȡʱ��,д��ÿ����¼���õ��ֶ�99,source id���ļ����ϵ�ʱ������ȡ���ŵ�sch���������Ҫ���������С�������ȡ��������ȡϵͳʱ�䡣ʱ����ʽΪYYYYMMDD
//2013-08-24��������ƽ̨ sql����д�ļ�,���ø���ӿ�ʵ��
//2013-10-15 У��������������ֶ�,�Զ������Զ���ȡ���ֶ�,
//			 ����Ԥ����У��ʧ�ܣ������쳣˵���ļ��ϴ����ļ���������������·� ��������
//2013-10-17 ��ϵͳfile_id����ϵͳ����ȥ
//2013-10-24 �޸�����ȡֵ��ʽ,�Ӻ˶Խ����ϸ���ȡ
//2013-10-27 ��ͷβ��¼�ֶ�ֵ��ʽ������¼����,Ū��������Ϣ input2ouput ͷ10��ͷ,Ϊ90��ͷ
//2013-11-27 �޸�дsql�ķ�ʽ,��ֱ��д����vecotor,�����ݿ�ֻ����д����vectorд�ļ�,���Ƴ���jsload,jsFileInAudit
//2013-12-10 ֱ��дinsert,ȥ��update sql���
//2013-12-16 �����ֵĺ������ýӿڷ�װ������,����ͳһ����
//2014-03-26 dealfile�����ļ�ʧ��,��¼����״̬
//2014-04-02 �ļ����ظ�ҲҪ�Ǽǵ��ȱ�,����ջ��ܲ�ƽ������
//2014-07-01 C_CYLCE_ADJ_DEFINE ���ӷ�����ɺ�������ļ������������ļ�(���ڳ�;ҵ���ʽ1)

#include "FormatPlugin.h"

//CDatabase DBConn;
//CLog theLog;
CLog theJSLog;
CReadIni theCfgFile;

int main(int argc,char** argv)
{

	/* �ӻ��������ļ��ж�ȡ���� */ 
	//CReadIni IniFile;
	//char* m_szEnvFile = "/mboss/home/zhjs/etc/zhjs/zhjs.ini" ;	
	//if(!IniFile.init(m_szEnvFile))
	//{
	//	cout<<"��INI�ļ����� "<<m_szEnvFile<<endl;
	//	return false;
	//}

	cout<<"********************************************** "<<endl;
	cout<<"*    China Telecom. Telephone Network          "<<endl;
	cout<<"*    InterNational Account Settle System       "<<endl;
	cout<<"*                                              "<<endl;
	cout<<"*           jsformat                           "<<endl;
	cout<<"*           sys.GJZW.Version 1.0	              "<<endl;
	cout<<"*    created time :     2013-06-01 by  hed 	  "<<endl;
	cout<<"*    last update time:  2014-07-01 by  hed	  "<<endl;
	cout<<"********************************************** "<<endl;

	FormatPlugin format ;
	if(!format.init(argc,argv)) return -1;
	//format.init("SERV1","CLYW1",1);
	//while(1)
	//{
		//theJSLog.reSetLog();
		format.run();
		//sleep(5);
	//}

    return 0;

}
