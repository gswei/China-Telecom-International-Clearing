#ifndef _PROCESS_H
#define _PROCESS_H

#include<string>
#include<sys/msg.h>
#include<signal.h>
#include<iostream>
#include<unistd.h>
#include<stdio.h>
#include<errno.h>
#include<time.h>
#include<sys/time.h>
#include<pthread.h>
//#include "Log.h"		//xieyzh
#include "table_def.h"
#include "tmpl_table.h"
#include "InfoData/writeInfoLog.hpp"
#include "tp_module.h"

#define VERSION "V1.0.0 Beta7"

using namespace std;

extern bool gbExitSig;
extern bool gbBreakSig;

#define	HOST_ID_NAME	"PSHOSTID"

#define DSPCH_MDL_NAME	"psdspch"
#define MASTER_MDL_NAME	"psmaster"
#define LSNR_MDL_NAME	"pslsnr"

#define PRCST_INACTV	0
#define PRCST_IDLE		1
#define PRCST_BUSY		2
#define PRCST_SUSP		3
#define PRCST_FAULT		4
#define PRCST_STARTING	5
#define PRCST_STOPPING	6

//报告类事件(无需响应)
#define EVT_RPT_UNREG		2001	//进程注销
#define EVT_RPT_HRTBT		2002	//心跳
#define EVT_RPT_TASKEND		2003	//任务结束
#define EVT_RPT_PRCST		2004	//报告进程状态
#define EVT_RPT_PERF		2005	//报告性能
#define EVT_RPT_HLLCTRL		2006	//高低水控制报告

//申请和响应类事件(1000以下为申请类，1000以上为响应类，模除1000相等的为一对申请和响应)
#define EVT_RESP_ERR		-1		//申请错误
#define EVT_REQ_REG			1		//进程申请注册
#define EVT_RESP_REG		1001	//响应进程注册
#define EVT_REQ_TASK		2		//请求任务
#define EVT_RESP_TASK		1002	//分配任务
#define EVT_REQ_SUSP		3		//请求挂起进程
#define EVT_RESP_SUSP		1003	//响应进程挂起
#define EVT_REQ_START		4		//请求启动进程
#define EVT_RESP_START		1004	//响应进程启动
#define EVT_REQ_STOP		5		//请求停止进程
#define EVT_RESP_STOP		1005	//响应进程停止
#define EVT_REQ_RESUME		6		//取消挂起
#define EVT_RESP_RESUME		1006	//响应取消挂起
#define EVT_REQ_REBSHM		7		//请求重建内存
#define EVT_RESP_REBSHM		1007	//响应请求重建内存
#define EVT_REQ_CHKSUSP		8		//查询挂起状态
#define EVT_RESP_CHKSUSP	1008	//响应查询挂起状态
#define EVT_REQ_CHKSTOP		9		//查询终止状态
#define EVT_RESP_CHKSTOP	1009	//响应查询终止状态
#define EVT_REQ_TRNUM		10		//申请传输话单
#define EVT_RESP_TRNUM  	1010	//相应传输话单


//命令类
#define EVT_CMD_SUSP		3001	//进程挂起
#define EVT_CMD_RESUME		3002	//进程取消挂起
#define EVT_CMD_STOP		3003	//进程退出
#define EVT_CMD_DETACHSHM	3004	//释放共享内存	Added by zhongk on 2007/06/20
#define EVT_CMD_HLLCTRL		3005	//高低水控制

//Added by zhongk on 2007/06/20
#define EVT_BROADCAST		1000	//广播事件

typedef struct
{
	int event_sn;
	int event_type;
	long param1;
	long param2;
	long src_id;
	time_t event_time;
}EVENT;

typedef struct
{
	long msgtype;
	EVENT event;
}EVENT_MSG;

class PS_Process
{
private:
	int   m_iUpEvtMsgID;
	int   m_iDownEvtMsgID;
	int   m_iCmdMsgID;
	int	  m_iTaskMsgID;
	//pthread_mutex_t m_Mutex;
	//pthread_rwlock_t m_RWLock;
	//static pthread_mutex_t m_LogMutex;
	long  m_lsysPrcID;
	int   m_iFlow_ID;
	int	  m_iMdl_ID;
	int   m_iInst_ID;
	int   m_iHost_ID;
	static Table<struct TP_MODULE> m_Tbl_TP_MODULE;		//模块表
	static TableIndex<TP_MODULE>* m_TblIdx_TP_MODULE;	//模块表索引
	static bool m_bTblInit;
	static CWriteLogInfo m_LogInfo;
	bool   chkArgs(int argc, char *argv[]);

public:
	void setSignalPrc();

	//按模块名称获得模块ID， 返回值>0成功，<0失败
	static int getMdlIDByName(char *mdl_name);

	//按模块名称获得进程ID，返回值>0成功，<0失败
	static long getPrcIDByName(char *mdl_name, int flow_id=0, int inst_id=1);	//返回内部进程标识(不带2位主机号)，可用于消息传输的参数

	//按模块名称获得系统进程ID，返回值>0成功，<0失败
	static long getSysPrcIDByName(char *mdl_name, int flow_id=0, int inst_id=1); //返回内部进程标识(带2位主机号)，可用于初始化日志接口

	static CWriteLogInfo *getLogInfoPointer() { return &m_LogInfo; }

	//写发送消息包信息点
	bool writeSendPkgInfo(const void *head/*包头指针*/, int headSize/*包头大小*/, const void *body/*包体指针*/, int bodySize/*包体大小*/); 

	//写接收消息包信息点
	bool writeRecvPkgInfo(const void *head/*包头指针*/, int headSize/*包头大小*/, const void *body/*包体指针*/, int bodySize/*包体大小*/);

	//写日志，做成静态函数是因为信号捕捉函数中有调用
	static bool writeLog(unsigned int logcode/*日志编码*/, int ne_cind/*网元编码*/, unsigned char direct/*消息包方向*/, char *logcontent/*日志内容*/);
	static bool writeLog(unsigned int logcode, char *logcontent);

	//写日志
	//bool writeLog(int code_id, const char* log_desc=NULL);
	//bool writeLog(int code_id, const char* sLogMsg, const char * sParamName, int iSqlType, const char* log_desc=NULL);  //xieyzh
protected:
	PS_Process();
	virtual ~PS_Process();
	bool m_bReg;
	long m_lPrc_ID;
	bool m_bSuspSig;
	bool m_bHLLCtrl;
	static long DSPCH_PRC_ID;
	static long EVTSCAN_MDL_ID;
	//Log  m_Log;
	//InfoPoint m_InfoPoint;
	IBC_ParamCfgMng param_cfg;	//核心参数读取类

	//获得退出信号
	bool getExitSig() { return gbExitSig; }
	
	//bool getSuspSig() { return m_bSuspSig; }
	
	//获得打断信号
	bool getBreakSig(bool clear_sig=true);

	//获得生产线号
	int	 getFlowID()  { return m_iFlow_ID; }

	//获得进程标识
	long getPrcID()   { return m_lsysPrcID; }	//返回的是配置表中的process_id(带2位主机号)

	//获得模块标识
	int	 getModuleID(){ return m_iMdl_ID; }
	
	//获得进程实例号
	int	 getInstID()  { return m_iInst_ID; }
	
	//获得主机号
	int	 getHostID()  { return m_iHost_ID; }

	bool init(int argc, char *argv[]);
	int  getCmd(int &cmd_sn, int &cmd_type, long &param1, long &param2, long &src_id);
	int  getTask(int &event_sn, int &event_type, long &param1, long &param2, long &src_id);

	//线程加锁
	bool threadLock();

	//线程解锁，和threadLock对应
	bool threadUnlock();

	//加线程读锁
	bool threadReadLock();

	//加线程写锁
	bool threadWriteLock();

	//解线程读写锁，和threadReadLock或threadWriteLock对应
	bool threadRWUnlock();

	//获取事件
	int  getEvt(int &event_sn, int &event_type, long &param1, long &param2, long &src_id, bool sync=true);

	//发送事件
	bool putEvt(int event_sn, int event_type, long param1, long param2, long dest_id=DSPCH_PRC_ID);

	//广播事件
	bool broadcastEvt(int event_type, long param1, long param2, short flow_id =0, short module_id =0);	// 广播事件，Added by zhongk on 2007/06/20

	//发送任务
	bool putTask(int event_sn, int event_type, long param1, long param2, long dest_id);
	
	//挂起其他进程
	bool suspend(long param1, long param2);
	
	//解挂其他进程
	bool resume(long param1, long param2);

	//bool notify(int trigger_event_type);
	
	//高低水控制报告
	bool rpt_hllctrl(int cmd_sn, int hll_type, int child_num=1);

	void prcExit();
	
	virtual void run();
	
	//初始成功返回true，失败返回false
	virtual bool onInit();

	//处理成功返回1，处理失败返回-1，进程正常退出返回-10，异常退出返回-11
	virtual int  onCmd(int cmd_sn, int cmd_type, long param1, long param2, long src_id);
	
	virtual void onExit();
};

void sig_prc(int sig_type);
void usersig_prc(int sig_type);

#endif
