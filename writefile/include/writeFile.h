//2013-07-15	//当一个文件写多个话单块时某个话单块的记录信息出错，则回退该文件已经处理的话单块，将其删除，并记录错误信息给sql文件，
		        //当处理下个文件时，写错误登记表，并将源文件copy过来到目标文件
//2013-07-18	将日志的配置放到核心参数里面，错误日志写错误码
//2013-07-29	增加文件落地后入库功能,修改CF_CError_Table接口setFileName()，传Statement使其能回滚前面的数据
//2013-08-01    新增临时目录WR_FILE_TMP_DIR  入库失败后正常文件的存放目录
//2013-08-13	增加从文件名上截取时间入调度表(读取数据源过滤配置信息表C_FILE_REVEIVE_ENV)，方便后面的汇总核对模块
//2013-08-17    增加获取petri状态,当数据库只读时，只写文件
//2013-09-01    去除容灾平台,将入库动作,拆分,单独编写一个模块,异常错误sql和写调度表的sql写在一个文件里面,每次在文件结束时插入一条sql
//2013-09-16    修改话单块出错时去从格式化找原始文件的方式，直接根据当前时间去查找7天以内的时间目录(以前是通过格式化调度表的时间字段截取目录)
//2013-10-31    当文件全部话单块出错，或者只有最后一个正常，删除临时文件报错的情况(临时文件删除失败)，此时根本没有临时文件生成,先判断临时文件是否生成
//2013-11-04    由于数据库切换时不能保证两边数据库状态完全一致,所以都统一写sql文件
//2013-12-01	新增对jsload仲裁失败时的判断,话单块状态-1

#include<iostream>
//#include <vector>

#include "bill_process.h"
#include "CF_CPkgBlock.h"
//#include "CF_CFscan.h"
#include "CF_CFmtChange.h"
//#include "CF_CError_Table.h"

#include "psutil.h"
#include "tp_log_code.h"
//#include "RTInfo.h"

using namespace tpss;  //和psutil.h对应
using namespace std;

const int JS_MAX_SQL_LEN			=	1024;
const int JS_MAX_FILENAME_LEN		= 	256;
const int JS_MAX_FILEPATH_LEN		=	256;
const int JS_MAX_FILEFULLPATH_LEN	=	512;
const int JS_MAX_ERRMSG_LEN			=	1024;
const int JS_MAX_RECORD_LEN			=	1024;

const char ERR_CODE[20]="ErrCode";
const char ERR_COLINDEX[20]="ErrColIndex";
const char LINE_NUM[20]="LineID";
const char FILE_ID[20]="FileID";
const char SOURCE_ID[20]="SourceID";

//数据源信息
struct SOURCECFG
{
	char szSourceId[6];						//数据源ID
	//char szInFileFmt[6];		
	char szSourcePath[JS_MAX_FILEPATH_LEN];   //数据源所在路径
	char serverCatID[6];
	char filterRule[50] ;						//数据源的过滤规则
	int  file_begin;							//截取文件名上面的时间
	int  file_length;

	SOURCECFG()
	{
		file_begin = -1;
		file_length = -1;
		memset(szSourceId,0,sizeof(szSourceId));
		//memset(szInFileFmt,0,sizeof(szInFileFmt));
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

	char szOutPath[JS_MAX_FILEPATH_LEN];		//输出路径(相对路径）	
	char szSrcBakPath[JS_MAX_FILEPATH_LEN];		//原始话单备份路径
	char szErroPath[JS_MAX_FILEPATH_LEN];		//错误文件路径
		 
	int szSleepTime;							//每次休眠时间

	//char szDebugFlag[50];						//是否输出运行日志

	SParameter()
	{
		iflowID = -1;
		iModuleId = 0;		
		szSleepTime = 5;

		memset(iWorkflowId,0,sizeof(iWorkflowId));
		memset(szSrcGrpID,0,sizeof(szSrcGrpID));
		memset(szService,0,sizeof(szService));
		memset(szSchCtlTabname,0,sizeof(szSchCtlTabname));
		memset(szOutputFiletypeId,0,sizeof(szOutputFiletypeId));	
		memset(iInputId,0,sizeof(iInputId));
		memset(iOutputId,0,sizeof(iOutputId));
		memset(szOutPath,0,sizeof(szOutPath));
		memset(szErroPath,0,sizeof(szErroPath));
		memset(szSrcBakPath,0,sizeof(szSrcBakPath));	
	}
};


class Write_File : public PS_BillProcess
{
    public:

     Write_File();
     ~Write_File();    
	 
	 bool init(int argc,char** argv);                                                 
	 int getSourceFilter(char* source,char* filter,int &index,int &length);
	 int LoadSourceCfg();			 
	 int getBackTimeDir(char* dir,char* orgFileName);  //2013-09-16 获取格式化备份时间目录

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

	 DBConnection conn;
	 map<string,SOURCECFG> m_SourceCfg;   //存放数据源的配置信息
	 map<string,SOURCECFG>::const_iterator it ; 
	 SParameter mConfParam;
	 CFmt_Change outrcd;
	 //CF_CFscan scan;			//文件扫描接口	  
	
	 char m_szSourceID[8];		//数据源ID
	 char mServCatId[6];
	 char m_szFileName[JS_MAX_FILENAME_LEN];    //文件名
	 char sql[JS_MAX_SQL_LEN];
	 char erro_msg[JS_MAX_ERRMSG_LEN];
	 char currTime[15];
	
	 int status ;				//话单块状态
	 int file_status ;			//文件状态 
	 int record_num;			//文件记录数	 
	 long file_id;
	 char file_time[8+1];		//从文件名上面截取

};

