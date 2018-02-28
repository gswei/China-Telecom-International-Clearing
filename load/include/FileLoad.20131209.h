/******************************************************************
	Copyright (c) 2012, GUANGDONG YOSON TECHNOLOGY CO., LTD
	All rights reserved.

	Created:		2013/2/25
	Filename: 		FileLoad.h
	Description:	结算六期系统的文件加载类
					
	History:
	<table>
		Revision	Author			Date		Description
		--------	------			----		-----------
		v1.0		hed		 2012/6/21	       完成程序初稿
	
	</table>
*******************************************************************/

#include<iostream>
#include <vector>

#include "bill_process.h"
#include "CF_CPkgBlock.h"
#include "psutil.h"
#include "tp_log_code.h"
#include "RTInfo.h"

//#include "tp_log_code.h"
#include "CF_Common.h"
#include "CF_CFscan.h"
#include "CF_CLogger.h"
#include "CF_CFmtChange.h"
#include "CF_CMemFileIO.h"

#include "dr_api.h"

using namespace std;
using namespace tpss;  //和psutil.h对应

/*
#define JS_MAX_SQL_LEN					1024
#define JS_MAX_FILENAME_LEN				256
#define JS_MAX_FILEPATH_LEN				256
#define JS_MAX_FILEFULLPATH_LEN			512
#define JS_MAX_ERRMSG_LEN				1024
#define JS_MAX_RECORD_LEN			    1024
*/

const int JS_MAX_SQL_LEN			=	1024;
const int JS_MAX_FILENAME_LEN		= 	256;
const int JS_MAX_FILEPATH_LEN		=	256;
const int JS_MAX_FILEFULLPATH_LEN	=	512;
const int JS_MAX_ERRMSG_LEN			=	1024;
const int JS_MAX_RECORD_LEN			=	1024;

const char FILE_ID[20]="FileID";

//数据源信息
struct SOURCECFG
{
	char szSourceId[6];							//数据源ID
	char szInFileFmt[6];						//文件格式
	char szSourcePath[JS_MAX_FILEPATH_LEN];		//数据源所在路径
	char filterRule[50] ;						//数据源的过滤规则
	char serverCatID[6];

	SOURCECFG()
	{
		memset(szSourceId,0,sizeof(szSourceId));
		memset(szInFileFmt,0,sizeof(szInFileFmt));
		memset(szSourcePath,0,sizeof(szSourcePath));
		memset(filterRule,0,sizeof(filterRule));
		memset(serverCatID,0,sizeof(serverCatID));
	}
};

//按流水线为单位
struct SParameter
{
	char szSrcGrpID[6];							//数据源组
	char szService[6];							//serviceID
	char  iWorkflowId[10];						//工作流模板ID  
	int  iflowID;								//流水线ID
	int  iModuleId;								//模块标识
	char  iInputId[10];							//输入接口ID
	char  iOutputId[10];						//输出接口ID

	char szOutputFiletypeId[6];					//输出文件类型
	char szSchCtlTabname[32+1];					//调度表表

	char szInPath[JS_MAX_FILEPATH_LEN];			//输入路径(相对路径）	
	char szBakPath[JS_MAX_FILEPATH_LEN];		//备份路径
	char szErroPath[JS_MAX_FILEPATH_LEN];		//错误文件路径
	char bak_flag ;
		
	int maxRecord_num;							//每个话单块的最大记录数
	int source_file_num ;						//扫描数据源的最大个数	 
	int szSleepTime;							//每次休眠时间

	//char szDebugFlag[50];						//是否输出运行日志

	SParameter()
	{
		iflowID = -1;
		iModuleId = 0;
		source_file_num = 0;
		maxRecord_num = 0;
		szSleepTime = 5;
		bak_flag = 'N';							//默认不备份

		memset(iWorkflowId,0,sizeof(iWorkflowId));
		memset(szSrcGrpID,0,sizeof(szSrcGrpID));
		memset(szService,0,sizeof(szService));
		memset(szSchCtlTabname,0,sizeof(szSchCtlTabname));
		memset(szOutputFiletypeId,0,sizeof(szOutputFiletypeId));
		
		memset(iInputId,0,sizeof(iInputId));
		memset(iOutputId,0,sizeof(iOutputId));

		memset(szInPath,0,sizeof(szInPath));
		memset(szErroPath,0,sizeof(szErroPath));
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


class FileLoad : public PS_BillProcess
{
   public:

     FileLoad();
     ~FileLoad();    
	 
	 bool init(int argc,char** argv);
	 int getSourceFilter(char* source,char* filter);
	 int LoadSourceCfg();	

	 int dealFile();  //处理文件，将主备份系统合并处理
	 void prcExit();

	 //容灾平台
	 bool drInit();
	 bool CheckTriggerFile();
	 int  drVarGetSet(char* m_SerialString);
	 int IsAuditSuccess(const char* dealresult);

   protected:
    //子进程需要实现的函数 继承PS_BillProcess
	int onBeforeTask();
	int onTaskBegin(void *task_addr);
	bool onChildInit();
	int onTask(void *task_addr, int offset, int ticket_num);
	//int onTask(int event_sn, int event_type, long param1, long param2, long src_id);
	//bool onChildTask(int event_sn, int event_type, long param1, long param2, long src_id);
	void onChildExit();
	int onTaskOver(int child_ret);

   private:
   
 //********************************************************************************************
	 DBConnection conn;
	 Statement stmt;

	 map<string,SOURCECFG> m_SourceCfg;					//存放数据源的配置信息
	 map<string,SOURCECFG>::const_iterator it ;
	 
	 SParameter mConfParam;
	 DRparameter mdrParam;
	 
	 CFmt_Change outrcd;								//加载文件的记录，读取其中的某些字段 add by hed 2013-07-31		
	 //vector<PkgFmt> m_record ;						//私有内存记录
	 PkgFmt*  m_record;

	 CF_CFscan scan;									//文件扫描接口

	 char m_szSourceID[6];								//数据源ID
	 char mServCatId[6];
	 char m_szFileName[JS_MAX_FILENAME_LEN];			//原始文件名
	 char fileName[JS_MAX_FILEFULLPATH_LEN];			//全路径文件名
	 char file_name[JS_MAX_FILENAME_LEN];				//分割后的文件名

	 char sql[JS_MAX_SQL_LEN];
	 char erro_msg[JS_MAX_ERRMSG_LEN];					//错误信息
	 char currTime[14+1];
	 
	 long file_id;
	 int curr_record_num;
	 int record_num;									//文件记录数	
	 int split_num ;									//文件过大时进行分批的次数
	 int file_num ;										//每个数据源的每次扫描的最大文件个数 则跳到下个数据源	 
	 short petri_status;								//获取petri网状态
	 short petri_status_tmp;
};

