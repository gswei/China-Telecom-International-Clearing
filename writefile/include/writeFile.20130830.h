//2013-07-18	将日志的配置放到核心参数里面，错误日志写错误码
//2013-07-29	增加文件落地后入库功能,修改CF_CError_Table接口setFileName()，传Statement使其能回滚前面的数据
//2013-08-01    新增临时目录WR_FILE_TMP_DIR  入库失败后正常文件的存放目录
//2013-08-13	增加从文件名上截取时间入调度表(读取数据源过滤配置信息表C_FILE_REVEIVE_ENV)，方便后面的汇总核对模块
//2013-08-17    增加获取petri状态,当数据库只读时，只写文件

#include<iostream>
#include <vector>

#include "bill_process.h"
#include "CF_CPkgBlock.h"
#include "CF_CFscan.h"
#include "CF_CFmtChange.h"
#include "CF_CError_Table.h"

#include "psutil.h"
#include "tp_log_code.h"
#include "RTInfo.h"
#include "dr_api.h"

using namespace tpss;  //和psutil.h对应

using namespace std;

const char ERR_CODE[20]="ERR_CODE";
const char ERR_COLINDEX[20]="ERR_COLINDEX";
const char LINE_NUM[20]="LINE_NUM";
const char FILE_ID[20]="FileId";
const char SOURCE_ID[20]="SourceID";

//数据源信息
struct SOURCECFG
{
  char szSourceId[6];		//数据源ID
  //char szFile_Fmt[6];		//数据源对应的格式
  char szInFileFmt[6];		
  char szSourcePath[256];   //数据源所在路径
  char szTollCode[16];      //本地网区号
  char serverCatID[5];

  char filterRule[256] ;	//数据源的过滤规则
  int  file_begin;		  //截取文件名上面的时间
  int  file_length;

};


class Write_File : public PS_BillProcess
{
    public:
     Write_File();
     ~Write_File();    
	 
	 bool init(int argc,char** argv);
	 int  writeFile(char* fileName,PkgBlock) ;                    //读取内存写到文件                                                  

	 int getSourceFilter(char* source,char* filter,int &index,int &length);
	 int LoadSourceCfg();
	
	 //int dealFile();  //处理文件，将主备份系统合并处理
	 int deleteFile(char* path,char* filter);
	 int commitErrMsg();	//提交错误记录信息

	 int updateDB(char* sql);
	 int scanSQLFile();

	 int  indb(char* file,char* name);  //2013-07-29	文件入库

	 bool drInit();
	 bool CheckTriggerFile();
	 int  drVarGetSet(char* m_SerialString);
	 bool IsAuditSuccess(const char* dealresult);

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
	 
	 map<string,int> mapConfig;   //存放入库的表标志
	 map< string,CF_CError_Table> mapTabConf	;

	 char m_szSrcGrpID[8];  //数据源组
	 char m_szService[8];	//serviceID
	 char m_szSourceID[8];  //数据源ID

	 char mServCatId[5];
	 char m_szOutTypeId[8];  //文件输出格式
	 
	 char out_path[245];	   //输出相对路径
	 char erro_path[256];  //错误文件路径
	 char bak_path[256];
	 char other_path[256];     //文件入库失败的文件路径

	 char m_szFileName[256];  //原始文件名
	 char outFileName[256];		  //输出文件名
	 CF_CFscan scan;	//文件扫描接口	 
	 
	 CFmt_Change outrcd;

	 char currTime[15];

	 char sql[1024];

	 int status ;//话单块状态
	 int file_status ; //文件状态 

	 char erro_sql[1024]; //错误sql文件登记名
	 char erro_msg[1024];
		
	  int record_num;  //文件记录数

	  char file_time[8];			//从文件名上面截取
	  short petri_status;	   //获取petri网状态

	  bool m_enable ;	//容灾状态
	  int drStatus;     //系统状态 0主系统,1备系统,2非容灾系统
	  //string m_triggerFile;	父类已经实现
	  char m_SerialString[4096];
	  char m_AuditMsg[1024];
};

