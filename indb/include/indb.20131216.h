/******************************************************************
	Copyright (c) 2012, GUANGDONG YOSON TECHNOLOGY CO., LTD
	All rights reserved.

	Created:		2013/2/25
	Filename: 		C_Indb.h
	Description:	结算六期系统的文件入库类
					
	History:
	<table>
		Revision	Author			Date		Description
		--------	------			----		-----------
		v1.0		hed		 2012/9/2	       完成程序初稿
	
	</table>
*******************************************************************/
//2013-09-01将写文件模块的正常文件进行入库,并容灾,建议在流水线上的配置在数据库上面,而非流水线上的配置在核心参数里面
//2013-10-24 修改文件入库程序在主备仲裁失败不写调度表的问题,备份按照时间目录

#include<iostream>
#include <vector>

#include "process.h"
#include "psutil.h"
#include "tp_log_code.h"

#include "RTInfo.h"
#include "dr_api.h"


#include "CF_Common.h"
#include "CF_CFscan.h"
#include "CF_CLogger.h"

#include "CF_CMemFileIO.h"
#include "CF_CFmtChange.h"
#include "new_table.h"
//#include "CF_CError_Table.h"

using namespace std;
using namespace tpss;  //和psutil.h对应

const int JS_MAX_SQL_LEN			=	1024;
const int JS_MAX_FILENAME_LEN		= 	256;
const int JS_MAX_FILEPATH_LEN		=	256;
const int JS_MAX_FILEFULLPATH_LEN	=	512;
const int JS_MAX_ERRMSG_LEN			=	1024;
const int JS_MAX_RECORD_LEN			=	1024;

const char FILE_ID[20]="FileID";
const char RATE_CYCLE[20]="RateCycle";

//数据源信息
struct SOURCECFG
{
	char szSourceId[6];		//数据源ID
	char szInFileFmt[6];		
	char szSourcePath[256];   //数据源所在路径
	char filterRule[50] ;	//数据源的过滤规则
	char serverCatID[6];
	int  file_begin;		  //截取文件名上面的时间
	int  file_length;
	
	SOURCECFG()
	{
		file_begin = -1;
		file_length = -1;
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
		
	int source_file_num ;						//扫描数据源的最大个数	 
	int szSleepTime;							//每次休眠时间

	//char szDebugFlag[50];						//是否输出运行日志

	SParameter()
	{
		iflowID = -1;
		iModuleId = 0;
		source_file_num = 0;
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


class C_Indb : public PS_Process
{
    public:
     C_Indb();
     ~C_Indb();    
	 
	 bool init(int argc,char** argv); 
	 int getSourceFilter(char* source,char* filter,int &index,int &length);
	 int LoadSourceCfg();
	 int dealFile();  //处理文件，将主备份系统合并处理
	 void run();
	 void prcExit();

	  //容灾平台
	 bool drInit();
	 bool CheckTriggerFile();
	 int  drVarGetSet(char* m_SerialString);
	 int IsAuditSuccess(const char* dealresult);

   private:   

 //********************************************************************************************
	 DBConnection conn;
	 map<string,SOURCECFG> m_SourceCfg;   //存放数据源的配置信息
	 map<string,SOURCECFG>::const_iterator it ;
	 map<string,int> mapConfig;   //存放入库的表标志
	 CF_CNewError_Table	*TabConf;
	 map< string,int > mapTabConf;
	 SParameter mConfParam;
	 DRparameter mdrParam;

	 char m_szSourceID[8];		//数据源ID
	 char mServCatId[6];
	 char m_szFileName[256];	//原始文件名
	 char fileName[512];		 //全路径文件名
	 char currTime[15]; 
	 char sql[1024];
	 char erro_msg[2048];		//错误信息 	 
	 
	 CFmt_Change outrcd;	    //加载文件的记录，读取其中的某些字段 add by hed 2013-07-31
	 CF_CFscan scan;		    //文件扫描接口
 
	 int record_num;            //文件记录数		
	 int file_num ;             //每个数据源的每次扫描的最大文件个数 则跳到下个数据源
	 long file_id;
	 char file_time[8+1];		//从文件名上面截取
	 short petri_status;	    //获取petri网状态
	 short petri_status_tmp;	
};
