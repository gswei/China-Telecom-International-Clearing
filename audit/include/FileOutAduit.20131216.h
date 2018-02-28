
//2013-07-23 ,将是否输出空文件，输出路径设置成可配置的，c_source_audit_env
//2013-08-27  增加容灾平台,每日处理
//2013-08-30 不容灾，,通过读取数据库状态判断是否(由于sql语句会影响下一个循环执行)
#include<iostream>
#include <vector>

//#include "bill_process.h"
#include "process.h"

#include "psutil.h"
#include "tp_log_code.h"

#include "RTInfo.h"
#include "dr_api.h"

using namespace tpss;  //和psutil.h对应
using namespace std;

const int JS_MAX_SQL_LEN			=	1024;
const int JS_MAX_FILENAME_LEN		= 	256;
const int JS_MAX_FILEPATH_LEN		=	256;
const int JS_MAX_FILEFULLPATH_LEN	=	512;
const int JS_MAX_ERRMSG_LEN			=	1024;
const int JS_MAX_RECORD_LEN			=	1024;

//按流水线为单位
struct SParameter
{
	//char szSrcGrpID[6];							//数据源组
	char szService[6];							//serviceID
	//char  iWorkflowId[10];						//工作流模板ID  
	int  iflowID;								//流水线ID
	int  iModuleId;								//模块标识
	//char  iInputId[10];							//输入接口ID
	//char  iOutputId[10];						//输出接口ID
	//char szSchCtlTabname[32+1];					//调度表表

	//char szInPath[JS_MAX_FILEPATH_LEN];			//输入路径(相对路径）	
	//char szOutPath[JS_MAX_FILEPATH_LEN];		//错误文件路径
			 
	int szSleepTime;							//每次休眠时间

	//char szDebugFlag[50];						//是否输出运行日志

	SParameter()
	{
		iflowID = -1;
		iModuleId = 0;		
		szSleepTime = 5;
		//memset(iWorkflowId,0,sizeof(iWorkflowId));
		//memset(szSrcGrpID,0,sizeof(szSrcGrpID));
		memset(szService,0,sizeof(szService));
		//memset(szSchCtlTabname,0,sizeof(szSchCtlTabname));
		//memset(iInputId,0,sizeof(iInputId));
		//memset(iOutputId,0,sizeof(iOutputId));
		//memset(szInPath,0,sizeof(szInPath));
		//memset(szOutPath,0,sizeof(szOutPath));	
	}
};

struct DRparameter
{
	 bool m_enable ;						//容灾状态
	 int  drStatus;							//系统状态 0主系统,1备系统,2非容灾系统
	 char m_SerialString[4096];				//同步串
	 char m_AuditMsg[4096];					//仲裁串

	 DRparameter()
	 {
		m_enable = false;
		drStatus = 2;
		memset(m_SerialString,0,sizeof(m_SerialString));
		memset(m_AuditMsg,0,sizeof(m_AuditMsg));
	 }
};

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
	 void run();
	 int updateDB();
	 void prcExit();

	 bool drInit();
	 bool CheckTriggerFile();
	 int  drVarGetSet(char* m_SerialString);
	 int IsAuditSuccess(const char* dealresult);

	 bool checkAuditBefore(char* date);

	 void clearMap();
	
   private:
	
	 DBConnection conn;
	 
	 SParameter mConfParam;
	 DRparameter mdrParam;

	 char currTime[15];
	 char m_szFileName[JS_MAX_FILENAME_LEN];  //文件名 
	 char sql[JS_MAX_SQL_LEN];
	 char erro_msg[JS_MAX_ERRMSG_LEN];
	 
	 short petri_status;					//获取petri网状态 

	 vector<string> fileList;
	 map< string,vector<string> > sourceMap ;
	 map< string,AduitEnv > fileNameMap ;

	 vector<string>  vsql;
};





