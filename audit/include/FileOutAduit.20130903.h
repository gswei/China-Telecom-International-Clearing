
//2013-07-23 ,将是否输出空文件，输出路径设置成可配置的，c_source_audit_env
//2013-08-27  增加容灾平台,每日处理
//2013-08-30 不容灾，,通过读取数据库状态判断是否(由于sql语句会影响下一个循环执行)
#include<iostream>
#include <vector>

#include "bill_process.h"

#include "psutil.h"
#include "tp_log_code.h"

#include "RTInfo.h"

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

	 int updateDB();
	
   private:
	
	 DBConnection conn;
	 
	 char currTime[15];
	 char m_szFileName[256];  //文件名 
	 char sql[512];
	 char sqlFile[512];
	 char erro_msg[512];
	 
	 short petri_status;	   //获取petri网状态 

	 vector<string> fileList;
	 map< string,vector<string> > sourceMap ;
	 map< string,AduitEnv > fileNameMap ;

	  vector<string>  vsql;
};





