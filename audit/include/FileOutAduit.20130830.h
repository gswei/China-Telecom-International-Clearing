
//2013-07-23 ,���Ƿ�������ļ������·�����óɿ����õģ�c_source_audit_env
//2013-08-27  ��������ƽ̨,ÿ�մ���

#include<iostream>
#include <vector>

#include "bill_process.h"

#include "psutil.h"
#include "tp_log_code.h"

#include "dr_api.h"

using namespace tpss;  //��psutil.h��Ӧ

using namespace std;

//������������Ϣ
struct AduitEnv
{
	vector<string> arrayFile;
	char out_path[512];
	char null_out_flag;
};

class FileOutAduit : public PS_Process
{
    public:
     FileOutAduit();
     ~FileOutAduit();    
	 
	 bool init(int argc,char** argv);                                               
	
	 int writeFile();
	 void run();

	 bool drInit();
	 bool CheckTriggerFile();
	 int  drVarGetSet(char* m_SerialString);
	 bool IsAuditSuccess(const char* dealresult);
	
   private:
	
	 DBConnection conn;
	 char sql[1024];
	 char m_szFileName[256];  //�ļ��� 
	
	 char currTime[15];

	 char sqlFile[1024];
	
	 char erro_msg[1024];
	 
	 vector<string> fileList;
	 map< string,vector<string> > sourceMap ;
	 map< string,AduitEnv > fileNameMap ;

	 bool m_enable ;	//����״̬
	 int drStatus;     //ϵͳ״̬ 0��ϵͳ,1��ϵͳ,2������ϵͳ
	 //string m_triggerFile;
	 char m_SerialString[4096];
	 char m_AuditMsg[1024];
};





