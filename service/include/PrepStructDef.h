/****************************************************************
filename: PrepStructDef.h
module: classify&analyse
created by: Wu Longfeng
create date: 20051220
version: 1.0.0
description:
	所有预处理模块主流程用到的结构体
update list:
	
*****************************************************************/

#ifndef _PREPSTRUCTDEF_H_
#define _PREPSTRUCTDEF_H_ 1

#include "CF_CFmtChange.h"
#include "CF_Common.h"
#include "CF_Config.h"
//#include "CF_COracleDB.h"
#include "CF_CErrorLackRec.h"
#include "CF_CProcessMonitor.h"
#include "CF_InfoPoint.h"
#include "Classify.h"
#include "CF_CPluginMessage.h"
//#include "main/pluginengine/zhjs/plugininitializer.h"
#include "plugininitializer.h"
//extern CFmt_Change g_outrcd;

const int SERVER_LEN			=10;
const int EXP_METHOD_LEN		= 512;
const int MAX_EXP_METHOD_COUNT	= 128;
const int STR_LEN			= 256;
const int FIELD_LEN				= 256;
const int DATE_LEN				= 8;
const int ERR_MSG_LEN			= 1024;

//using namespace zhjs;

/* 保存LIST_SQL表中的记录 */
struct SListSql{
	char m_szSqlId[16+1];
	char m_szUpdateFlag[1+1];
	char m_szUpdateSqlName[32+1];
};

/* 用于保存每个文件信息的结构体 */
struct SFileStruct
{
	char szRealFileName[FILE_NAME_LEN+1];//真实文件名(不加proc_index)
	char szFileName[FILE_NAME_LEN+1];    //文件名(含proc_index)
	char szSourceId[SOURCE_ID_LEN+1];    //所在数据源
	char szSourcePath[PATH_NAME_LEN+1];  //数据源路径
	char szDealStartTime[14+1];          //开始处理时间
	char szDealEndTime[14+1];
	int iPartID;
//	int  szSourceLackCode;
	char szLocalNet[2+1];                //默认归属地
	char szTollcode[10+1];               //默认区号
	char szServCat[SERVER_LEN+1];        //数据源默认的业务编码
	char szServCatConfig[30+1];					
	char szSourceFiletype[FIELD_LEN+1];  //该数据源对应的话单格式

	long lFileId;						//D_FILE_RECEIVED表中的文件ID
	char szReceiveTime[14+1];			//D_FILE_RECEIVED表中的RECEIVE_TIME
	int iDealFlag;						//处理标志 0:未处理 1:已处理 2:已提交文件 3:异常
	int iMaxCount;					//一个批次最大处理个数

	SFileStruct()
	{
		memset(szRealFileName, 0, sizeof(szRealFileName));
		memset(szFileName, 0, sizeof(szFileName));
		memset(szSourceId, 0, sizeof(szSourceId));
		memset(szSourcePath, 0, sizeof(szSourcePath));
		memset(szDealStartTime, 0, sizeof(szDealStartTime));
		memset(szDealEndTime, 0, sizeof(szDealEndTime));
		memset(szLocalNet, 0, sizeof(szLocalNet));
		memset(szTollcode, 0, sizeof(szTollcode));
		memset(szServCat, 0, sizeof(szServCat));
		memset(szServCatConfig, 0, sizeof(szServCatConfig));
		memset(szSourceFiletype, 0, sizeof(szSourceFiletype));
		lFileId = 0;
		iDealFlag = 0;
		iMaxCount = 0;
		iPartID = 0;
	};
};

struct SSourceStruct
{
	char szSourceId[SOURCE_ID_LEN+1];    //所在数据源
	char szSourcePath[PATH_NAME_LEN+1];  //数据源路径
//	char szLocalNet[2+1];                //默认归属地
	char szTollcode[10+1];               //默认区号
	char szServCat[SERVER_LEN+1];        //数据源默认的业务编码
	char szSourceFiletype[FIELD_LEN+1];  //该数据源对应的话单格式
	int iMaxCount;					//一个批次最大处理个数

	SSourceStruct()
	{
		memset(szSourceId, 0, sizeof(szSourceId));
		memset(szSourcePath, 0, sizeof(szSourcePath));
//		memset(szLocalNet, 0, sizeof(szLocalNet));
		memset(szTollcode, 0, sizeof(szTollcode));
		memset(szServCat, 0, sizeof(szServCat));
		memset(szSourceFiletype, 0, sizeof(szSourceFiletype));
		iMaxCount = 0;
	};
};

/* 用于保存数据源信息的结构体 */
/*
struct SSourceStruct
{
	char szSourceId[5+1];
	char szSourcePath[PATH_NAME_LEN+1];
};
*/

/* 用于在DealFile.cpp和MainFlow.cpp中传递参数 */
struct SParameter
{
	char szServiceId[MAXLENGTH+1];  //服务编号
	char szSourceGroupId[MAXLENGTH+1]; //数据源组
	int iProcessId; //进程索引号
	
	char szWorkflowId[SERVER_LEN+1];
	char szServerId[SERVER_LEN+1];
	int  iInputId;                  //输入接口ID
	int  iOutputId;                  //输出接口ID

	CProcessMonitor ProcMonitor;		//进程管理程序

	char szLogTabname[TABLENAME_LEN+1];       //日志表
	bool bBakFlag;					//是否保留入口文件
	bool bCommemFlag;				//是否连接共享内存
	char szIsFmtFirst[2];            //是否配有格式化插件
	char szSlPath[PATH_NAME_LEN+1];  //插件文件所在路径
	char szSlName[FILE_NAME_LEN+1];
	CF_CErrorLackRec lack_info;      //错单/无资料单接口类
	CF_CErrorLackRec abnormal;      //错单/无资料单接口类
	zhjs::PluginInitializer pluginInitializer;		//插件接口
//	char FileNameConstrain[EXP_METHOD_LEN+1];
	CClassify classify;          //分拣类
	TMP_VAR map_DefVar;				//默认变量名
	//CVariableContainer defaultVar;
	
	char szInputFiletypeId[5+1];     //输入文件类型
	char szOutputFiletypeId[5+1];    //输出文件类型
	char szInrcdType[2];           //输入文件记录类型
	char szOutrcdType[2];          //输出文件记录类型
	char szInPath[PATH_NAME_LEN+1];  //输入路径（相对路径）
	char szOutPath[PATH_NAME_LEN+1]; //输出路径（相对路径）
	
	bool bOutputFile;     //是否输出文件

	char szMsgKey[10+1];

	CF_MemFileO fmt_err2file;	//格式化错单入文件
	CF_CErrorLackRec fmt_err2table;	//格式化错单入统计表
	char szFmtErr2Table[2];     //Y表示入库
	char szFmtErrSaveTableId[TABLENAME_LEN+1];  //格式化错单表
	char szFmtErrStatTableId[TABLENAME_LEN+1];  //格式化错单统计表
	char szFmtErr2File[2];      //Y表示入文件
	char szFmtErrDir[PATH_NAME_LEN+1]; //错单入文件路径
	char szFmtTimeOutDir[PATH_NAME_LEN+1];	//超时单入文件路径
	char szFmtOtherDir[PATH_NAME_LEN+1];	//未定义格式错单入文件路径

//	char szLack2Table[2];
//	char szLackStatConfig[2];
	char szLackSaveTableId[TABLENAME_LEN+1];
	char szLackStatTableId[TABLENAME_LEN+1];
	char szAbnSaveTableId[TABLENAME_LEN+1];
	char szAbnStatTableId[TABLENAME_LEN+1];
	
	int iServCatConfig;     //SERV_CAT_ID在话单中的字段号
//	int iMaxNum;   //一个批次最大的文件数

	InfoLog info;						//信息点日志

	int iSleepTime;

	SParameter()
	{
		memset(szServiceId, 0, sizeof(szServiceId));
		memset(szSourceGroupId, 0, sizeof(szSourceGroupId));
		iProcessId = 0;

		memset(szWorkflowId, 0, sizeof(szWorkflowId));
		memset(szServerId, 0, sizeof(szServerId));

		memset( szInputFiletypeId, 0, sizeof(szInputFiletypeId) );
		memset( szOutputFiletypeId, 0, sizeof(szOutputFiletypeId) );
		memset( szInrcdType, 0, sizeof(szInrcdType) );
		memset( szOutrcdType, 0, sizeof(szOutrcdType) );
		memset( szInPath, 0, sizeof(szInPath) );
		memset( szOutPath, 0, sizeof(szOutPath) );
	};
};


#endif

