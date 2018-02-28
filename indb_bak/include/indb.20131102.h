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

#include "CF_CFmtChange.h"
#include "new_table.h"
//#include "CF_CError_Table.h"

using namespace std;
using namespace tpss;  //和psutil.h对应

const int JS_MAX_SQL_LEN			=	1024;
const int JS_MAX_FILENAME_LEN		=	256;
const int JS_MAX_FILEPATH_LEN		=	256;
const int JS_MAX_FILEFULLPATH_LEN	=	512;
const int JS_MAX_ERRMSG_LEN			=	2048;
const int JS_MAX_RECORD_LEN			=   1024;

const char FILE_ID[20]="FileID";

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
	 bool IsAuditSuccess(const char* dealresult);

   private:
     
 //********************************************************************************************
	 DBConnection conn;
	 map<string,SOURCECFG> m_SourceCfg;   //存放数据源的配置信息
	 map<string,SOURCECFG>::const_iterator it ;

	 map<string,int> mapConfig;   //存放入库的表标志
	 CF_CNewError_Table	*TabConf;
	 map< string,int > mapTabConf;

	 char m_szSrcGrpID[8];  //数据源组
	 char m_szService[8];	//serviceID
	 char m_szSourceID[8];  //数据源ID
	 char mServCatId[6];

	 char input_path[JS_MAX_FILEPATH_LEN];  //文件输入相对路径
	 char sql[JS_MAX_SQL_LEN];
	 char erro_msg[JS_MAX_ERRMSG_LEN];		//错误信息
	 char bak_path[JS_MAX_FILEPATH_LEN];	//备份路径
	 char bak_flag ;						//备份标志
	 char erro_path[JS_MAX_FILEPATH_LEN];   //错误文件路径
	 char currTime[15]; 
	
	 char m_szFileName[JS_MAX_FILENAME_LEN];  //原始文件名
	 char fileName[JS_MAX_FILEFULLPATH_LEN];  //全路径文件名

	 char m_szOutTypeId[8];	  //文件记录格式
	 CFmt_Change outrcd;	  //加载文件的记录，读取其中的某些字段 add by hed 2013-07-31 
	 CF_CFscan scan;	//文件扫描接口
 
	 int record_num;  //文件记录数	
	 int file_num ;   //每个数据源的每次扫描的最大文件个数 则跳到下个数据源
	 int source_file_num ;		//扫描数据源的最大个数
	 
	 long file_id;
	 char file_time[8+1];		//从文件名上面截取
	 short petri_status;	   //获取petri网状态
	
	 bool m_enable ;	//容灾状态
	 int drStatus;     //系统状态 0主系统,1备系统,2非容灾系统
	 char m_SerialString[4096];
	 char m_AuditMsg[4096];	
};

