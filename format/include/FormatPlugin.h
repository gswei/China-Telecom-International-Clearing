/******************************************************************
	Copyright (c) 2012, GUANGDONG YOSON TECHNOLOGY CO., LTD
	All rights reserved.

	Created:		2013/2/25
	Filename: 		FormatPlugin.h
	Description:	结算六期系统的格式化文件类
					格式化文件，写话单块

	History:
	<table>
		Revision	Author			Date		Description
		--------	------			----		-----------
		v1.0		hed		 2012/3/22	       完成程序初稿
		v2.0		hed		 2013/3/31	
	</table>
*******************************************************************/
//CALLNO_HEADER
//BILLTIME_BETWEEN_FILE
//FAKEHEADER

#include <sys/types.h>
#include <sys/stat.h>  //stat()函数，查询文件信息
#include <dirent.h> //_chdir() _getcwd() 读取文件目录等，浏览文件夹信息

#include "process.h"
#include "Ipc.h"
#include "RTInfo.h"

#include "CF_CFscan.h"
#include "CF_CMemFileIO.h"
#include "CF_CFmtChange.h"
#include "CF_CRecordChange.h"
//#include "psutil.h"
#include "CF_CPlugin.h"
#include "voiceformat.h"
#include "bill_statistic.h"
#include "bill_statistic_route.h"
#include "bill_statistic_filenametime.h"
#include "CF_Comp_Exp.h"
#include "formatcheck.h"
#include "CF_Common.h"
#include "CF_CFmtChange.h"
#include "CF_CLogger.h"
#include "CF_CHash.h"

//#include "dr_api.h"
#include "dr_deal.h"

#define	INROUTE_NAME				"InRoute"
#define	OUTROUTE_NAME				"OutRoute"
#define	RECORD_NAME				    "Record"
#define	ORARECORD_NAME				"OraRecord"
#define	ERR_SELECT_NONE				4007	//数据库里找不到记录
#define	ERR_GET_RECORDFMT			4008	//获取当前记录格式出错
#define  ERR_FILEFMT               4009   //文件类型找不到
#define  ERR_ANNFILE               4010   //ANN_commit出错
#define	ERR_UPDATE_STATICS			4004	//统计话单流量出错
#define	ERR_GET_MsgDirection		4005	//获取话单方向出错

#define CHECK_SUCCESS     1
#define CHECK_FAIL		  0

//using namespace ipc;
//using namespace tpss;

const int JS_MAX_SQL_LEN			=	1024;
const int JS_MAX_FILENAME_LEN		= 	256;
const int JS_MAX_FILEPATH_LEN		=	256;
const int JS_MAX_FILEFULLPATH_LEN	=	512;
const int JS_MAX_ERRMSG_LEN			=	1024;
const int JS_MAX_RECORD_LEN			=	1024;

struct SOURCECFG
{
  char szSourceId[6];
  char szFile_Fmt[6];
  char szInFileFmt[6];
  char szSourcePath[256];
  char szTollCode[16];
  int FieldIndex[20];		//压缩字段
  char Fmt_Time_fmt[30];
  char szZDate_Fmt[16];
  char szzTime_Fmt[16];

  char serverCatID[6];		//2013-09-09

  char filterRule[50] ;  //过滤规则，add by hed  2013-03-12
  int  file_begin;		  //截取文件名上面的时间
  int  file_length;

  int  iMsgDirBegin;
  char  FnSep;
  int   FnIndex;
  int   FnBegin;
  int   FnLen;

  char FntSep;
  int   FntIndex;
  int   FntBegin;
  int   FntLen;
  
  char chIs_Bill_Statics;
  char chIs_Bill_Statics_Route;
  char chIs_Bill_Statics_fnTime;
  char chIs_TimeFile;
  
  int iRcd_Arr_Dur;
  int iERcd_Arr_Dur;
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
	char szOutPath[JS_MAX_FILEPATH_LEN];		//输出路径(相对路径）
	char szBakPath[JS_MAX_FILEPATH_LEN];		//备份路径
	char szErroPath[JS_MAX_FILEPATH_LEN];		//错误文件路径
	char szTimeoutPath[JS_MAX_FILEPATH_LEN];	//输入路径(相对路径）
	char szOtherPath[JS_MAX_FILEPATH_LEN];		//输入路径(相对路径）
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
		bak_flag = 'Y';							//默认不备份

		memset(iWorkflowId,0,sizeof(iWorkflowId));
		memset(szSrcGrpID,0,sizeof(szSrcGrpID));
		memset(szService,0,sizeof(szService));
		memset(szSchCtlTabname,0,sizeof(szSchCtlTabname));
		memset(szOutputFiletypeId,0,sizeof(szOutputFiletypeId));
		
		memset(iInputId,0,sizeof(iInputId));
		memset(iOutputId,0,sizeof(iOutputId));

		memset(szInPath,0,sizeof(szInPath));
		memset(szOutPath,0,sizeof(szOutPath));
		memset(szErroPath,0,sizeof(szErroPath));
		memset(szBakPath,0,sizeof(szBakPath));	
		memset(szTimeoutPath,0,sizeof(szTimeoutPath));
		memset(szOtherPath,0,sizeof(szOtherPath));
	}
};

/*
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
*/

//校验文件的格式
struct FileNameFmt
{
	int number;     //校验序号
	int index;		//校验索引位置
	int len;		//校验字段长度
	char check_type;  //校验值类型
	char check_value[50];  //校验默认值
	char err_code[6];
};

//校验文件记录的头尾
struct RecordHTFmt
{
	int  number;
	char name[50];
	int index;
	int len;
	char seperator[2];			//字段的分隔符
	char check_type;			//校验值类型
	char check_value[50];		//校验默认值
	char ht_flag;				//头尾标志
	char spec_flag[20];			//2013-07-26,用来去账务日期
	char err_code[6];
};

//2014-07-01
struct C_ADJ_CYCEL_DEF
{
	int monthnum;				//可调整的账期间隔 默认为 1
};

class FormatPlugin:public PS_Process,public BasePlugin
{
	//CDatabase *m_DBConn;
	DBConnection conn;//数据库连接
	Statement stmt;
//	CLogger *m_Logger;

	char m_szSrcGrpID[8];
	char m_szService[8];
	char m_szSourceID[8];
	char mServCatId[6];
	int iProcIndex;
	char m_szFileName[256];		//原始文件名
	char m_szOutTypeId[8];
	char szDebugFlag[2];
	char fileName[512] ;		//全路径文件名
	char outFileName[512];		//输出文件名

	int Redo_Begin;
	char Redo_What[8];
	int iErrorBack_Flag;

  	C_BillStat_Route Bill_Route;
  	C_BillStat_fnTime Bill_fnTime;
  	C_BillStat Bill;

	CFormat m_txtformat;
	CFormatCheck m_formatcheck;
	STxtFileFmtDefine *pCur_TxtFmtDefine;
	SInput2Output  *pCur_Input2Output;

	map<string,SOURCECFG> m_SourceCfg;
	map<string,SOURCECFG>::iterator mCur_SourceCfg;
	
	char m_chBillSta_Filter_Cond;
	char m_chIs_Ann;
	char szAnn_Dir[32];
	char szLastAnn_Dir[256];
	char szLastAnn_Abbr[8];
	

	CFmt_Change outrcd;
	int iRecord_Index;
	int iOraRcdIdx;

	Txt_Fmt OTxt_Fmt;
	vector<Comp_Exp> oComp_Exp;
  	int iComp_Exp_Num;

  	char szEarliestTime[15];
	char szLatestTime[15];

	int iTotalNum;
	int iRightNum;
	int iLackNum;
	int iErrorNum;
	int iPickNum;
	int iOtherNum;
	char trans_table[256][2]; /*BCD-ASCII table*/
	char trans_tablex[256][2];/*HEX-ASCII table*/
	
	map<string,SOURCECFG>::const_iterator it ;
	
	SParameter mConfParam;
	//DRparameter mdrParam;
	DR_Deal	 mdrDeal;

	//char erro_path[256];       //错误文件相对路径
	//char input_path[256];     //文件输入相对路径
	//char output_path[256];       //文件输出相对路径  2013-06-19
	//char timeout_path[256];   //超时文件路径
	//char other_path[256];     //格式化未定义格式话单，入otherformat文件
	
	CF_CFscan scan;
	PacketParser *pps;
	ResParser *res;
	
	vector<string> file_head;
	vector<string> file_tail;
	vector<string> m_vsql;
	vector<string> record_array;
	
	map< string,vector<FileNameFmt> > mapFileNameFmt;
	map< string,vector<RecordHTFmt> > mapFileRecordHeadFmt;
	map< string,vector<RecordHTFmt> > mapFileRecordTailFmt;
	
	map<string,C_ADJ_CYCEL_DEF> mapAdjCycleBusi;

	char currTimeA[15];
	char currTimeB[15];
	char rate_cycle[15];				//2013-07-26 取账期到话单里面
	char rate_cycle_org[15];				//2014-07-02 取账期到话单里面
	char file_time[15];					//去文件名上的时间带到话单里面去
	char err_code[10];					//错误码 2013-07-19
	char sql[JS_MAX_SQL_LEN]; 
	char erro_msg[JS_MAX_ERRMSG_LEN];	//错误信息登记信息
	
	long file_id ;
	int  record_num;
	//int  source_file_num ;		//扫描数据源的最大个数 
	short petri_status;			//获取petri网状态 
	short petri_status_tmp;

private:
	int LoadSourceCfg();
	int InsTimeBetweenFile(char *SourceId,char *filename,char *EarlyTime,char *LastTime);
	void Build_trans_table();
	int transfer( char *block_buff, int len,char *szRtn,int flag = 1);
	void Route_Change(STxtFileFmtDefine *pTmp,CFmt_Change *inrcd);
	void Get_TxtCalledNo(CFmt_Change *inrcd);
	int GetStrFromFN(char *Res,char *Fn,char Spl,int Index,int Begin,int Len);
	int getFromSrcEnv( char *Value, char *Name, char *SourceId,char *szService);
	int SetAbnOutCdr(pluginAnaResult anaType,int iErrorType,CFmt_Change *inrcd,PacketParser& ps,ResParser& retValue);
	int initflag;

	int getSourceFilter(char* source,char* filter,int &index,int &length);  //add by hed  2013-03-12
public:
	FormatPlugin();		//{	initflag=0;m_bSuspSig=false;childNum = 0;}
	~FormatPlugin();	//{ m_Shm = NULL; m_pArgv=NULL;m_pTktBlkBase=NULL;m_DBConn=NULL;pCur_TxtFmtDefine=NULL;pCur_Input2Output=NULL; }
	void init(char *szSourceGroupID, char *szServiceID, int index);	
	void execute(PacketParser& ps,ResParser& retValue);
	void message(MessageParser&  pMessage);
	std::string getPluginName(){return "FormatPlugin";}
	std::string getPluginVersion(){return "3.0.0";}
	void printMe(){}

	bool init(int argc,char** argv);
	void run();
	int dealFile();					  //处理文件
	int dealAuditResult(int result);  //处理仲裁
	int writeFile(char* fileName);
	int updateDB();					  //数据库的更新 增删改
	void prcExit();
	
	int getmapAdjCycleBusi(string source); 

	int getFileNameFmt(string source);
	int getRecordHTFilter(char* fileType);	
	int checkFileRepeat(char* file);      //校验文件名是否重复

	int checkFileName(string source,char* fileName);		  //校验文件名的格式是否正确
	int checkRecorHT(char* fileType,char* head,char* tail);	  //校验文件记录头尾是否正确
	int checkRule(RecordHTFmt fmt,char* column_value);
	int checkDate(char* date);

	//bool drInit();
	//bool CheckTriggerFile();
	//int  drVarGetSet(char* m_SerialString);
	//int IsAuditSuccess(const char* dealresult);

};


