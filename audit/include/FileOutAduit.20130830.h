
//2013-07-23 ,将是否输出空文件，输出路径设置成可配置的，c_source_audit_env
//2013-08-27  增加容灾平台,每日处理

#include<iostream>
#include <vector>

#include "bill_process.h"

#include "psutil.h"
#include "tp_log_code.h"

#include "dr_api.h"

using namespace tpss;  //和psutil.h对应

using namespace std;

//存放审核配置信息
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
	 char m_szFileName[256];  //文件名 
	
	 char currTime[15];

	 char sqlFile[1024];
	
	 char erro_msg[1024];
	 
	 vector<string> fileList;
	 map< string,vector<string> > sourceMap ;
	 map< string,AduitEnv > fileNameMap ;

	 bool m_enable ;	//容灾状态
	 int drStatus;     //系统状态 0主系统,1备系统,2非容灾系统
	 //string m_triggerFile;
	 char m_SerialString[4096];
	 char m_AuditMsg[1024];
};





