
//核对文件和月汇总文件备份按照ADU/SUM  核对文件下面再YYMM/DD建目录,过滤临时文件
//2013-07-26核对文件和月汇总文件核对结果放在同一张表里面，但是处理过程分开

//2013-08-07  将其放到格式化之前，对接收到的核对文件通过文件名，到接收目录下去查找，若有写文件找不到，则将其状态置N
//			  下次继续对核对失败的文件进行查找，若都找到，表示核对成功，通过文件名找到文件所属数据源(通配符)，将文件放到输出路径(格式化的输入路径)
//2013-08-27增加容灾平台,将日汇总和月汇总分开,月汇总一次启停
//2013-08-30 仅当核对文件中全部文件名找到了才发送同步信息,若核对失败则放到失败目录,不写数据库
//2013-09-02 月汇总增加容灾平台
//2013-09-19 月汇总增加将每个文件类型的记录入库 D_MONTH_CHECK_RESULT
//2013-10-12 核对文件列表增加数据源ID字段,便于后面的日汇总条件
//2013-10-20 修改仲裁条件,改为只仲裁记录的个数,防止文件数太多
//2013-10-24 D_CHECK_FILE_DETAIL，增加封帐标志字段(CYCLE_FLAG), C_RATE_CYCLE填写封帐标志,并且填写下个帐期的
//2013-11-12 月核对增加条件,当月进行完汇总时完毕,才进行核对
//2014-01-11 话单环境路径通过file.check.receive_path变量值第一位字符是'/'判断是相对路径还是绝对路径
//2014-08-04 月核对增加取数据源 费用支持多个字段,月汇总配置增加字段busi_type区分业务类型(月汇总文件格式不同),
//2014-10-15 封账时增加更新日期,月核对应该在封账之前,去掉条件;修复备系统无法区别日核对和月核对的处理(需要提前判断),

#include<iostream>
#include <vector>

#include "process.h"
//#include "bill_process.h"

//#include  "dr_api.h"
#include "dr_deal.h"
#include "RTInfo.h"

//#include "psutil.h"
//#include "tp_log_code.h"
#include "CF_Common.h"
#include "CF_CLogger.h"
#include "CF_CFscan.h"

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

//数据源信息
struct SOURCECFG
{
	char szSourceId[6];		//数据源ID		
	char szSourcePath[256];   //数据源所在路径
	char filterRule[50] ;    //过滤规则  通过匹配文件名找数据源
	int  file_begin;		   //截取文件名上面的时间
	int  file_length;

	SOURCECFG()
	{
		memset(szSourceId,0,sizeof(szSourceId));
		memset(szSourcePath,0,sizeof(szSourcePath));
		memset(filterRule,0,sizeof(filterRule));
		file_begin = -1;
		file_length =-1;
	}
};


//核对文件的记录格式
struct Check_Rec_Fmt
{
	string fileName;
	string rate_flag;
	string month;
};

//月汇总文件的配置文件格式
struct  Check_Sum_Conf
{
	string check_type;
	string source_id;
	string sum_table;
	string cdr_count ;
	string cdr_duration;
	vector<string> cdr_fee;
	string rate_cycle;
	int busi_type;

	void clear()
	{
		check_type = "";
		source_id = "";
		sum_table = "";
		cdr_count = "";
		cdr_duration = "";
		rate_cycle = "";
		cdr_fee.clear();
		busi_type = 0;
	}

};

//月汇总文件的记录格式
struct  Check_Sum_Rec_Fmt
{
	string file_type;
	long cdr_count ;
	long cdr_duration;
	long cdr_fee;
};

class FileInAduit : public PS_Process
{
    public:
     FileInAduit();
     ~FileInAduit();    
	 
	 bool init(int argc,char** argv);                                               
	 int  LoadSourceCfg();
	 int getSourceFilter(char* source,char* filter,int &index,int &length);

	 //int dealFile();		//处理核对文件
	 //int dealMonthFile();  //处理月汇总文件
	
	 void execute();
	 void run(int flag = 1);
	 void run2();

	 int checkFile();
	 int checkMonthFile();

	 //int check_before_file();
	 int check_file_exist(char* file);
	 bool checkFormat(const char *cmpString, const char *format);
	
	 int updateDB();
	 void prcExit();

	 //bool drInit();
	 //bool CheckTriggerFile();
	 //int  drVarGetSet(char* m_SerialString);
	 //int IsAuditSuccess(const char* dealresult);

   private:
	
	 DBConnection conn;
	 
	 SParameter mConfParam;
	 //DRparameter mdrParam;
	 DR_Deal	 mdrDeal;

	 //char m_szSrcGrpID[8];  //数据源组
	 //char m_szService[8];	//serviceID
	 //char m_szSourceID[8];  //数据源ID
	 //char mServCatId[5];
	 //char m_szOutTypeId[8];  //文件输出格式
	
	 char receive_path[256];	 //原文件接收路径
	 char output_path[256];     //原文件输出路径

	 char input_path[512];     //核对文件输入路径	
	 char month_input_path[512];	//月汇总文件输入路径
	
	 char bak_path1[512];	    //核对文件备份路径
	 char bak_path2[512];	    //月汇总文件备份路径
	 char fail_path[512];
	 char sql[1024];
	
	 char m_szSourceID[6];		//2013-10-12
	 char m_szFileName[256];  //文件名
	 char fileName[512];    //全路径文件名	 
	
	 char currTime[15];
	 char file_time[8+1];	 //从文件名上面截取

	 char erro_msg[1024];

	 short petri_status;	   //获取petri网状态
	 short petri_status_tmp;

	 vector<string>  m_vsql;

	 CF_CFscan scan;	//文件扫描接口	

	 map<string,SOURCECFG> m_SourceCfg; 
	 map<string,string>		 mapFileSource ;//文件名,数据源ID

	 vector<Check_Rec_Fmt> fileList;
	 map< string,Check_Sum_Conf > monthSumMap ;		//存放月汇总的配置文件信息
	 vector<Check_Sum_Rec_Fmt> fileList2;
};





