//2013-07-20 常驻进程，扫描错误文件信息表，将出错的信息写异常说明文件，并把错误的源文件返回给上游系统，再登记异常说明文件
//2013-07-22 将出错信息写两份写到输出文件登记目录
//2013-08-17    增加获取petri状态,当数据库只读时，只写文件
//2013-08-27  新增容灾平台

#include<iostream>
#include <vector>
#include <errno.h>

#include "bill_process.h"
#include "CF_CFmtChange.h"

#include "psutil.h"
#include "tp_log_code.h"
//#include "RTInfo.h"

#include "dr_api.h"

using namespace tpss;  //和psutil.h对应

using namespace std;

//数据源信息
struct SOURCECFG
{
  char szSourceId[6];		//数据源ID
  //char szFile_Fmt[6];		//数据源对应的格式
  //char szInFileFmt[6];		
  char szSourcePath[256];   //数据源所在路径
  //char szTollCode[16];      //本地网区号
  //char filterRule[256] ;	//数据源的过滤规则

  char serverCatID[5];
};


//异常说明文件信息
struct ERRINFO
{
	string filename;
	char source_id[6];
	//char dealtime[15];
	char err_msg[2];
	char err_code[10];
	int  err_col;
	int	 err_line;
};

class ExceptionOut : public PS_Process
{
    public:
     ExceptionOut();
     ~ExceptionOut();    
	 
	 bool init(int argc,char** argv);                                               

	 int getSourceFilter(char* source,char* filter);
	 int LoadSourceCfg();

	 //int updateDB(char* sql);
	 //int scanSQLFile();
	 //int writeSQL(char* sql);

	 void run();

	 bool drInit();
	 bool CheckTriggerFile();
	 int  drVarGetSet(char* m_SerialString);
	 bool IsAuditSuccess(const char* dealresult);
	
   private:
	
	 DBConnection conn;
	 map<string,SOURCECFG> m_SourceCfg;   //存放数据源的配置信息
	 map<string,SOURCECFG>::const_iterator it ;

	 //char m_szSrcGrpID[8];  //数据源组
	 //char m_szService[8];	//serviceID
	 char m_szSourceID[8];  //数据源ID
	 //char mServCatId[5];
	 //char m_szOutTypeId[8];  //文件输出格式

	 char input_path[256];     //输入相对路径
	 char out_path[256];	   //输出绝对路径

	 char m_szFileName[256];  //原始文件名
	 char outFileName[256];	  //输出文件名 
	
	 char currTime[15];
	 //short petri_status;	   //获取petri网状态 

	 char erro_msg[1024];

	 map< string,vector<ERRINFO> > erroinfoMap ;

	  bool m_enable ;	//容灾状态
	 int drStatus;     //系统状态 0主系统,1备系统,2非容灾系统
	 //string m_triggerFile;
	 char m_SerialString[4096];
	 char m_AuditMsg[1024];

};





