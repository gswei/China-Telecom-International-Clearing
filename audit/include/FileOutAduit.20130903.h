
//2013-07-23 ,���Ƿ�������ļ������·�����óɿ����õģ�c_source_audit_env
//2013-08-27  ��������ƽ̨,ÿ�մ���
//2013-08-30 �����֣�,ͨ����ȡ���ݿ�״̬�ж��Ƿ�(����sql����Ӱ����һ��ѭ��ִ��)
#include<iostream>
#include <vector>

#include "bill_process.h"

#include "psutil.h"
#include "tp_log_code.h"

#include "RTInfo.h"

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

	 int updateDB();
	
   private:
	
	 DBConnection conn;
	 
	 char currTime[15];
	 char m_szFileName[256];  //�ļ��� 
	 char sql[512];
	 char sqlFile[512];
	 char erro_msg[512];
	 
	 short petri_status;	   //��ȡpetri��״̬ 

	 vector<string> fileList;
	 map< string,vector<string> > sourceMap ;
	 map< string,AduitEnv > fileNameMap ;

	  vector<string>  vsql;
};





