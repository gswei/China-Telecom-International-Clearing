/****************************************************************
  Project	
  Copyright (c)	2010-2011. All Rights Reserved.		广东亿迅科技有限公司 
  FUNCTION:	一次批价
  FILE:		C_BillRate.h
  AUTHOR:	liuw
  Create Time: 2010-05-10
==================================================================
  Description:  
		结合综合计费结算3.0系统的新框架设计，批价模块用插件形式实现
  UpdateRecord: 
==================================================================

 *****************************************************************/

#ifndef _C_BILLRATE_H_
#define _C_BILLRATE_H_	1
 
#include "CF_CPlugin.h" //插件基类文件

#include "CF_CInterpreter.h"
//#include "CF_COracleDB.h"
#include "CF_CPluginPacket.h"
#include "CF_Config.h"

//#include "CF_CPluginengine.h"
#include "CF_CPlugin.h"
#include "CF_CMessage.h"

#include <stdio.h>
#include <iostream.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>

#include "settlerule.h"
#include "psutil.h"



const int  CALCFEE_ERR_NOT_ENOUGH_MEMORY =   6001;  //动态分配内存失败
const int  CALCFEE_ERR_IN_READ_ENV_VAR   =   6002;  //读取环境变量出错
const int  CALCFEE_ERR_IN_SELECT_DB      =   6003;  //查询数据出错
const int  CALCFEE_ERR_IN_CONNECT_DB     =   6004;  //连接数据库出错
const int  CALCFEE_ERR_IN_CHECK_RULE     =   6005;  //检查批价规则实例表失败
const int  CALCFEE_ERR_IN_LOAD_TABLE     =   6006;  //载入数据表到内存失败
const int  CALCFEE_ERR_IN_SORT_TABLE     =   6007;  //内存数据表排序失败
const int  CALCFEE_ERR_UNKNOWN_CATCH     =   6008;  //捕捉到无法识别的错误类
const int  CALCFEE_ERR_NEED_RESTART      =   6009;  //严重错误，进程需重新启动
const int  CALCFEE_ERR_PK_DUPLICATE      =   6010;  //数据表主键重复
const int  CALCFEE_ERR_SLFILE_NOT_EXIT   =   6011;  //插件文件不存在
const int  CALCFEE_ERR_ACCESS_PROC_MEM   =   6012;  //存取进程监控共享内存出错
const int  CALCFEE_ERR_IN_OPEN_FILE			 =	 6013;  //打开文件错误 --add by liuw 20070214

class	BillRate: public BasePlugin
{
	
private:
    int initflag;
    CFmt_Change inrcd,outrcd;
    
  	char szErrMsg[ERROR_MSG_LEN+1];						//错误信息
  	char szCurDatetime[DATETIME_LEN+1];				//当前时间
  	char szStartupTime[DATETIME_LEN+1];				//模块启动时间
		char szLogStr[LOG_MSG_LEN+1];							//日志信息
		char szSqlTmp[10*SQL_LEN+1];							//存储临时sql语句的变量
		int  iRuleSavingInterval;	//新增加多少条精确规则后保存一次数据到文件
		CF_MemFileO pAuditFile;   //预留审核、拨测接口
		CF_MemFileO *pDispFiles;
		//DBConnection conn;//数据库连接
		char szTxtType[20]; //摊分时的摊分格式


public:
		BillRate();
		~BillRate();
		int dealFlag;
		void init(char *szSourceGroupID,char * serviceID,int index);
		void init(char *jobID, char *ratecycle);// 重构初始化函数，为结算摊分用

        //为独立程序时使用此函数进行初始化
		void m_init(char * sourceid,STparam &szTparam,char* job_id);
		
 		void execute(PacketParser& ps,ResParser& retValue);
 		
		void message(MessageParser&  pMessage);
		
		void printMe(){
			//打印版本号
			printf( "\t插件名称:批价,最后修改日期：2010-06-19 liuw,版本号：1.0.0 \n");
			return;
		};
		
		std::string getPluginName()
		{
			return "BillRate";
		}
		std::string getPluginVersion(){
			return "3.0.0";
		}


};

#endif


