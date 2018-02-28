//2013-07-20 常驻进程，扫描错误文件信息表，将出错的信息写异常说明文件，并把错误的源文件返回给上游系统，再登记异常说明文件
//2013-07-22 将出错信息写两份写到输出文件登记目录
//2013-08-17    增加获取petri状态,当数据库只读时，只写文件
//2013-08-27  新增容灾平台
//2013-08-30 删除容灾平台,sql不写文件通过读取数据库状态(由于sql语句会影响下一个循环执行)
//2013-09-03 增加容灾平台,扫描表时一个个文件的仲裁

#include<iostream>
#include <vector>
#include <errno.h>

//#include "bill_process.h"
#include "process.h"
#include "CF_CFmtChange.h"

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

//数据源信息
struct SOURCECFG
{
	char szSourceId[6];						  //数据源ID		
	char szSourcePath[JS_MAX_FILEPATH_LEN];   //数据源所在路径
	
	SOURCECFG()
	{
		memset(szSourceId,0,sizeof(szSourceId));
		memset(szSourcePath,0,sizeof(szSourcePath));
	}
};

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
	char szInPath[JS_MAX_FILEPATH_LEN];			//输入路径(相对路径）	
	char szOutPath[JS_MAX_FILEPATH_LEN];		//错误文件路径			 
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
		//memset(szOutputFiletypeId,0,sizeof(szOutputFiletypeId));	
		//memset(iInputId,0,sizeof(iInputId));
		//memset(iOutputId,0,sizeof(iOutputId));
		memset(szInPath,0,sizeof(szInPath));
		memset(szOutPath,0,sizeof(szOutPath));	
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

//异常说明文件信息
struct ERRINFO
{
	string filename;
	char source_id[6];
	char err_msg[2];
	char err_code[10];
	int  err_col;
	int	 err_line;
	long err_seq;
};

class ExceptionOut : public PS_Process
{
    public:
     ExceptionOut();
     ~ExceptionOut();    
	 
	 bool init(int argc,char** argv);                                               

	 int getSourceFilter(char* source,char* filter);
	 int LoadSourceCfg();

	 int updateDB();

	 bool drInit();
	 bool CheckTriggerFile();
	 int  drVarGetSet(char* m_SerialString);
	 int IsAuditSuccess(const char* dealresult);

	 void run();
	 void prcExit();

   private:
	
	 DBConnection conn;
	 map<string,SOURCECFG> m_SourceCfg;   //存放数据源的配置信息
	 map<string,SOURCECFG>::const_iterator it ;
	
	 SParameter mConfParam;
	 DRparameter mdrParam;

	 //char m_szSrcGrpID[8];  //数据源组
	 //char m_szService[8];	//serviceID
	 char m_szSourceID[8];  //数据源ID
	 //char mServCatId[5];
	 //char m_szOutTypeId[8];  //文件输出格式
	
	 char m_szFileName[JS_MAX_FILENAME_LEN];		//原始文件名
	 //char input_path[JS_MAX_FILEPATH_LEN];			//输入相对路径
	 //char out_path[JS_MAX_FILEFULLPATH_LEN];	    //输出绝对路径
 
	 char outFileName[JS_MAX_FILEFULLPATH_LEN];		//输出全路径文件名 
	 char currTime[15];
	 short petri_status;	   //获取petri网状态 
	
	 char sql[JS_MAX_SQL_LEN];
	 char erro_msg[JS_MAX_ERRMSG_LEN];

	 map< string,vector<ERRINFO> > erroinfoMap ;
	 vector<string>  vsql;

	 //bool m_enable ;	//容灾状态
	 //int drStatus;     //系统状态 0主系统,1备系统,2非容灾系统
	 //char m_SerialString[4096];
	 //char m_AuditMsg[4096];

};





