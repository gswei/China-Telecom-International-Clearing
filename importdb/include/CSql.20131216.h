/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-08-16
File:			 CSql.h
Description:	 实时SQL入库模块
	对petri状态进行获取，判断是否为可写入
	获取目录下的sql文件，并入库

	输入指定目录下的SQL文件 C_GLOBAL_ENV
    输出SQL文件登记表 D_SQL_FILEREG

    接口说明  输入命令：jsextSQL 
**************************************************************************/
//2013-11-22 sql文件仲裁数量多个
//2013-12-05 仲裁失败时,需要缩小范围查询原因,两个参数(仲裁文件个数,最小仲裁单位个数)配置到数据库中

#ifndef CSQL_H
#define CSQL_H

#include <iostream>
#include <vector>
#include <dirent.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>  //stat()函数，查询文件信息
#include <unistd.h>     //读取当前程序运行目录
#include <fstream>

//#include "bill_process.h"
#include "process.h"

#include "psutil.h"
#include "tp_log_code.h"
#include "RTInfo.h"   //获取petri网状态
#include "dr_api.h"

using namespace tpss;  //和psutil.h对应

#include "CF_CFscan.h"
#include "CF_Common.h"
#include "CF_CLogger.h"

using namespace std;

#define SQL_COMMIT_COUNT	1000	

const int JS_MAX_SQL_LEN			=	1024;
const int JS_MAX_FILENAME_LEN		= 	256;
const int JS_MAX_FILEPATH_LEN		=	256;
const int JS_MAX_FILEFULLPATH_LEN	=	512;
const int JS_MAX_ERRMSG_LEN			=	2048;
const int JS_MAX_RECORD_LEN			=	2048;
//const int JS_MINI_AUDIT_NUM			=	1;

//按流水线为单位
struct SParameter
{
	//char szSrcGrpID[6];							//数据源组
	char szService[6];							//serviceID
	//char  iWorkflowId[10];						//工作流模板ID  
	int  iflowID;								//流水线ID
	int  iModuleId;								//模块标识
	//char  iInputId[10];						//输入接口ID
	//char  iOutputId[10];						//输出接口ID
	//char szSchCtlTabname[32+1];				//调度表表

	char szInPath[JS_MAX_FILEFULLPATH_LEN];		//输入路径	
	char szBakPath[JS_MAX_FILEFULLPATH_LEN];	//备份文件路径
	char szErrPath[JS_MAX_FILEFULLPATH_LEN];	//错误路径
	char szFailPath[JS_MAX_FILEFULLPATH_LEN];	//仲裁失败路径

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
		memset(szInPath,0,sizeof(szInPath));
		memset(szErrPath,0,sizeof(szErrPath));
		memset(szFailPath,0,sizeof(szFailPath));
		memset(szBakPath,0,sizeof(szBakPath));	
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


class CSql: public PS_Process
{

private:
    
	char  m_Filename[JS_MAX_FILENAME_LEN];		//文件名
	char  filenames[JS_MAX_FILEFULLPATH_LEN];   //全路径文件名
	//char  input_path[512];					//文件查找路径
	//char  output_path[512];					//文件输出路径
	//char  erro_path[512];						//文件失败路径
	//char  fail_path[512];						//仲裁失败路径
	char  erro_msg[JS_MAX_ERRMSG_LEN];			//错误信息	
	char  currTime[15];
	CF_CFscan scan;	//文件扫描接口
    DBConnection conn;//数据库连接

	SParameter mConfParam;
	DRparameter mdrParam;
	
	bool mini_flag			 ;		//2013-12-05
	vector<string> mMiniVfile;

	vector<string> mVfile;		//数组文件
	vector<char> vDealFlag;
	short petri_status ;
	int	  audit_file_num ;
	int   audit_fail_mini_num;
    
public:
	CSql();
	~CSql();
	bool init(int argc,char** argv);   
	void run();
	int getFileExist();		//获取文件名

	bool doAllSQL();//针对某个文件，获取里面SQL语句，并执行
	void saveLog();  //每处理一个文件都保存到D_SQL_FILEREG表中
	bool moveFiles(int flag);//将已经处理后的文件移动到指定备份目录
	void prcExit();

	//容灾平台
    bool drInit();
	bool CheckTriggerFile();
	int  drVarGetSet(char* m_SerialString);
	int IsAuditSuccess(const char* dealresult);
};

#endif 





























