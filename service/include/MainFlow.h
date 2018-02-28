/****************************************************************
filename: mainflow.h
module:
created by: ouyh
create date: 
version: 3.0.0
description:
    插件运行平台
*****************************************************************/

#ifndef _MAINFLOW_H_
#define _MAINFLOW_H_ 1

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
#include <map>
#include <signal.h>

#include "CF_CMemFileIO.h"
#include "CF_Cerrcode.h"
#include "CF_CLogger.h"
#include "CF_Common.h"
#include "CF_CProcessMonitor.h"
#include "CF_CInterrupt.h"
#include "CF_CMessagePPQ.h"
#include "DealFile.h"
#include "PrepCommon.h"
#include "VersionInfo.h"
#include "AccessMem.h"

 #include "bill_process.h"

 #include "CF_CPkgBlock.h"
#include "psutil.h"
//#include "MdrNodeApi.h"
//#include "dr_api.h"
//#include "tp_log_code.h"
//add by weixy 20080530
//#include "C_SourcePriority.h"
//int grepAndSortFile(map<string, int> &sourcePriority, SFileStruct * sfile, int &fileNum);
//end add by weixy 20080530

class C_MainFlow: public PS_BillProcess
{
private:
	char m_szExeName[PATH_NAME_LEN+1];
	char m_szExePath[PATH_NAME_LEN+1];
	char m_szEnvPath[PATH_NAME_LEN+1];
	//char m_szEnvFile[FILE_NAME_LEN+1];
	//char m_szLastSource[MAXLENGTH+1];
	char m_szLastDay[8+1];
	
	/* 需要绑定到表达式上的变量 */
	char m_szSourceId[MAXLENGTH+1];
	//char m_szLocalTollcode[MAXLENGTH+1];
	//char m_szSysTime[MAXLENGTH+1];
	//char m_szFileName[MAXLENGTH+1];
	//char m_szNewName[MAXLENGTH+1];
//	char m_szReceiveTime[14+1];

	//CDealFile DealFile;
	SParameter Param;			//存储传递至dealfile函数变量的结构体
	//CMessagePPQ respMsgHandle;
	//vector<SFileStruct> vecFile;
	//vector<SFileStruct> nextBatch;
	map<string, SSourceStruct> mapSource;
	//Interpreter Compile;			//表达式，用于文件名检查
	//BaseAccessMem *memManager;	//共享内存相关
	C_AccessMem *memManager;	//共享内存相关
	
	filterchain::FilterChain *chain;
	ArgInit initPlugin;
	PacketParser *pps;
	ResParser *res;
	SListSql* pListSql;			//存储list_sql表的更新标志
	int iListCount;
	//int iUndoFlag;
	int iRad;
	//int iRunTime;
	//bool m_bDBLinkError;
    int process_index ;  
    long billing_line_id;
    int  procID;
    //char sourceID[20];
    char source_group[10];       
    char ext_param[50];
    char servCat[10];   
     int recordlenth;

    SSourceStruct source;//从函数getSourceInfo移到全局

    char szFileName[MAXLENGTH+1];
    char szSourceId[MAXLENGTH+1];
    //char szFileName[MAXLENGTH+1];
    //char szSourceId[MAXLENGTH+1];
    void *block_addr; //话单块首地址
    //vector<char *> Classify1; //分拣话单
    //vector<char *> Classify3; //分拣话单
    C_DealCalculator m_dealRcdNum;
    pid_t _iMainPid;    
    char m_szBeginTime[14+1];// 文件开始处理时间
  //  int tmp_childnum; //子进程的数目

   // int ruleid;
	std::vector<string> filenames;
    
    bool getParam();
	int getScheme(struct SParameter &Param);
	//char* getMsgKey(CReadIni &IniFile, char *szServiceId, char *szSourceGroupId);
//	int getFilesFromDB();
	//int getFilesFromPPQ();
	//void resetHFiles();
	void getSourceInfo();
//bool getFileInfo(struct SFileStruct &FileStruct, bool bNewFile=false);
	//void process();
	void releasePoint();
	//void checkTime();
//	void CheckWorkpath();
 //   bool mputChldEvt(int event_sn, int event_type, long param1, long param2, long dest_id);
 //   int mgetChldEvt(int &event_sn, int &event_type, long &param1, long &param2, long &src_id, bool sync=true);
    //MdrStatus syncInit();
    //填充主系统的各个参数
    //void fillMasterAuditInfo( MdrAuditInfo& audit_info, char* value );
    //主系统获取数据
    //bool masterAudit( MdrAuditInfo& audit_info, char* value  );
    //备系统获取数据
    //bool slaveAudit( MdrAuditInfo& audit_info,char *allval );
    //结果
    //void cmtResult( const MdrAuditInfo& audit_info );
    //bool runmdr( char* value  );
    //bool dealSyntable( MdrVarInfo &var_info_, std::string &Sql );
    
public:
	C_MainFlow();
	~C_MainFlow();
	void printVersion();
	bool checkArg(int argc, char** argv);
	bool init(int argc, char **argv);
	//void run();
	void Update(); //20140512 add lij
	//void exit();
//	bool DBLinkError();
	//void resetAll();

	DBConnection conn;//数据库连接
	//bool m_enable ;	//容灾状态
	//int drStatus;     //系统状态 0主系统,1备系统,2非容灾系统
    //CDatabase _DBConn[10]; //子进程

  protected:
    //子进程需要实现的函数 继承PS_BillProcess
	int onBeforeTask();
	int onTaskBeginForMainFlow(void *task_addr);
	bool onChildInitForMainFlow();
	int onTask(void *task_addr, int offset, int ticket_num);
	void onChildExit();
	int onTaskOver(int child_ret); 
	bool onChildInit();   
	int onTaskBegin(void *task_addr);
};

void dealSignal(int sig);

#endif


