
//2013-07-23 ,将是否输出空文件，输出路径设置成可配置的，c_source_audit_env

#include<iostream>
#include <vector>

#include "bill_process.h"

#include "psutil.h"
#include "tp_log_code.h"

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

	 //int getSourceFilter(char* source,char* filter);
	 //int LoadSourceCfg();
	
	 //int deleteFile(char* path,char* filter);
	 //int commitErrMsg();	//提交错误记录信息
	
	 int writeFile();

	 int updateDB(char* sql);
	 int scanSQLFile();

	 void run();
	
   private:
	
	 DBConnection conn;
	 //map<string,SOURCECFG> m_SourceCfg;   //存放数据源的配置信息
	 //map<string,SOURCECFG>::const_iterator it ;

	 //char m_szSrcGrpID[8];  //数据源组
	 //char m_szService[8];	//serviceID
	 //char m_szSourceID[8];  //数据源ID
	 //char mServCatId[5];
	 //char m_szOutTypeId[8];  //文件输出格式

	 //char input_path[256];     //输入相对路径
	 //char out_path[1024];	   //输出相对路径
	  //char erro_path[256];  //错误文件路径
	 char sql[1024];
	 char m_szFileName[256];  //文件名
	 //char outFileName[256];		  //输出文件名
	 //CF_CFscan scan;	//文件扫描接口	 
	
	 char currTime[15];

	 //int record_num;  //文件记录数
	 char sqlFile[1024];
	
	 //int status ;//话单块状态
	 //int file_status ; //文件状态 

	 //char erro_sql[1024]; //错误sql文件登记名
	 char erro_msg[1024];
	 
	 vector<string> fileList;
	 map< string,vector<string> > sourceMap ;
	 map< string,AduitEnv > fileNameMap ;
};





