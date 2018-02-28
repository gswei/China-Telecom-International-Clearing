#include "process.h"
#include "pub_def.h"
#include "tmpl_table.h"
#include "table_def.h"
#include "CfgParam.h"
#include "SysCtrl.h"
using namespace _SYS_;

bool gbExitSig=false;
bool gbBreakSig=false;
long PS_Process::DSPCH_PRC_ID=-1;
long PS_Process::EVTSCAN_MDL_ID=-1;

//pthread_mutex_t PS_Process::m_LogMutex=PTHREAD_MUTEX_INITIALIZER;
bool PS_Process::m_bTblInit=false;
Table<struct TP_MODULE> PS_Process::m_Tbl_TP_MODULE;
TableIndex<TP_MODULE>* PS_Process::m_TblIdx_TP_MODULE;
CWriteLogInfo PS_Process::m_LogInfo;

void sig_prc(int sig_type)
{
	cerr<<"  接收到退出信号!!!\n";
	signal(sig_type, sig_prc);
	gbExitSig=true;
}

void usersig_prc(int sig_type)
{
	cerr<<"接收到通知信号!!!\n";
	signal(sig_type, usersig_prc);
	gbBreakSig=true;
}

PS_Process::PS_Process()
{
	m_lsysPrcID=-1;
	m_iFlow_ID=-1;
	m_iMdl_ID=-1;
	m_iInst_ID=1;
	m_iHost_ID=-1;
	m_bReg=false;
}

PS_Process::~PS_Process()
{
	m_LogInfo.shutdown();

	//如果进程未注销，进程注销
	if(m_bReg)
	{
		if( putEvt(0, EVT_RPT_UNREG, m_lPrc_ID, getpid(), DSPCH_PRC_ID) ) m_bReg=false;
	}
}

void PS_Process::setSignalPrc()
{
	for(int i=1; i<256; i++)
	{
		switch(i)
		{
		case SIGINT:
		case SIGQUIT:
		case SIGTERM:
			signal(i, sig_prc);
			break;
		case SIGUSR1:
			signal(i, usersig_prc);
			break;
		case SIGCHLD:
			break;
		default:
			signal(i, SIG_IGN);
		}
	}
}

bool PS_Process::chkArgs(int argc, char *argv[])
{
	for(int i=1;i<argc;i++)
	{
		if(argv[i][0]!='-') continue;
		switch(argv[i][1])
		{
		case 'f':
			m_iFlow_ID=atoi(argv[i]+2);
			break;
		case 'i':
			m_iInst_ID=atoi(argv[i]+2);
			break;
		default:
			break;
		}
	}

	if(m_iFlow_ID==-1)
	{
		cerr<<"命令行参数不足，请加-f参数(生产线号)，非流水线进程为-f0"<<endl;
		return false;
	}

	if(m_iInst_ID<1)
	{
		cerr<<"进程实例号不正确，-i参数值应该大于0"<<endl;
		return false;
	}

	return true;
}

int PS_Process::getMdlIDByName(char *mdl_name)
{
	if( !m_bTblInit )
	{
		//模块表
		if( !m_Tbl_TP_MODULE.attachSHM(false) )
		{
			cerr<<"连接TP_MODULE表共享内存失败\n";
			return -1;
		}
		if( m_Tbl_TP_MODULE.loadSHM() < 0 )
		{
			cerr<<"加载TP_MODULE表到私有内存失败\n";
			return -1;
		}
		m_TblIdx_TP_MODULE=m_Tbl_TP_MODULE.createIndex("module_code");
		m_TblIdx_TP_MODULE->build();
		m_bTblInit=true;
	}

	struct TP_MODULE tp_module_key;
	const struct TP_MODULE *p_tp_module;
	
	strcpy(tp_module_key.module_code, mdl_name);
	p_tp_module = m_TblIdx_TP_MODULE->find(tp_module_key);
	if(p_tp_module==NULL)
	{
		cerr<<"读取模块信息失败，请确认进程名 "<<mdl_name<<" 在表 TP_MODULE 中配置并上载\n";
		return -1;
	}
	return p_tp_module->module_id;
}

long PS_Process::getPrcIDByName(char *mdl_name, int flow_id, int inst_id)
{
	int mdl_id=getMdlIDByName(mdl_name);
	if( mdl_id < 0 ) return -1;
	return flow_id*1000000 + mdl_id*100 + inst_id;
}

long PS_Process::getSysPrcIDByName(char *mdl_name, int flow_id, int inst_id)
{
	int mdl_id=getMdlIDByName(mdl_name);
	if( mdl_id < 0 ) return -1;
	char *sHostID=getenv(HOST_ID_NAME);
	if( sHostID==NULL )
	{
		cerr<<"环境变量中未配置参数"<<HOST_ID_NAME<<"\n";
		return -1;
	}
	int host_id=atoi(sHostID);
	return mdl_id*1000000 + host_id*10000 + flow_id*100 + inst_id;
}

bool PS_Process::init(int argc, char *argv[])
{
	cerr<<"进程模板版本 "<<VERSION<<endl;

	DSPCH_PRC_ID=getPrcIDByName(DSPCH_MDL_NAME);
	if(	DSPCH_PRC_ID == -1 )
	{
		cerr<<"获取调度进程号失败！\n";
		return false;
	}
	/*EVTSCAN_MDL_ID=getPrcIDByName("evtscan");
	if(	EVTSCAN_MDL_ID == -1 )
	{
		cerr<<"获取事件扫描进程号失败！\n";
		return false;
	}*/

	if( !chkArgs(argc, argv) ) return false;

	char *pMldName;
	pMldName=strrchr(argv[0], '/');
	if(pMldName==NULL) pMldName=argv[0];
	else pMldName++;

	m_iMdl_ID=getMdlIDByName(pMldName);
	if( m_iMdl_ID < 0 ) return false;

	char *sHostID=getenv(HOST_ID_NAME);
	if( sHostID==NULL )
	{
		cerr<<"环境变量中未配置参数"<<HOST_ID_NAME<<"\n";
		return false;
	}
	m_iHost_ID=atoi(sHostID);
	
	m_lPrc_ID=m_iFlow_ID*1000000+m_iMdl_ID*100+m_iInst_ID;
	m_lsysPrcID=m_iMdl_ID*1000000 + m_iHost_ID*10000 + m_iFlow_ID*100 + m_iInst_ID;

	/*if( pthread_mutex_init(&m_Mutex, NULL) != 0 )
	{
		cerr<<"初始化线程锁失败！\n";
		return false;
	}
	if( pthread_rwlock_init(&m_RWLock, NULL) != 0 )
	{
		cerr<<"初始化线程读写锁失败！\n";
		return false;
	}*/

	/******* xieyzh **************************************************************/
	cout<<"模块号:"<<m_iMdl_ID<<" 进程号: "<<getPrcID()<<endl;
	
	if( !m_LogInfo.initialize(m_lsysPrcID, m_iMdl_ID) )
	{
		cerr << "日志、信息点接口初始化失败，errmsg="<<m_LogInfo.getLastErrorMessage()<<endl;
        return false;
	}
	/*g_LogMemMng = new IBC_LogMemMng;
	bool res = g_LogMemMng->onInit(getPrcID(), m_iMdl_ID, 590, IPUB_STAFF_ID);
    if (!res)
    {
        cerr << "初始化SGW_LogMemMng失败，errmsg="<< g_LogMemMng->getMessage()  <<  ", errcode=" << g_LogMemMng->getErrCode() <<endl;
        return false;
    }
	if( pthread_mutex_init(&m_LogMutex, NULL) != 0 )
	{
		cerr<<"初始化日志线程锁失败！\n";
		return false;
	}*/
	/************************************************************************/

	if( !param_cfg.bOnInit() )
	{
		string sErr;
		int nCodeId;
		param_cfg.getError(sErr,nCodeId);
		cerr<<"参数配置接口初始化失败！错误码="<<nCodeId<<", 错误信息="<<sErr<<endl;
		return false;
	}

	CString sKeyVal;
	key_t up_evtq_key, down_evtq_key, cmdq_key, taskq_key;//, chldprc_evtchnl_key;
	if( param_cfg.bGetMem("memory.MT_DSPCH_UP.msgq_key_value", sKeyVal) && sKeyVal.isNumber() )
	{
		up_evtq_key=sKeyVal.toInteger();
	}
	else
	{
		cerr<<"未配置上行事件通道键值\n";
		return false;
	}
	if( param_cfg.bGetMem("memory.MT_DSPCH_DOWN.msgq_key_value", sKeyVal) && sKeyVal.isNumber() )
	{
		down_evtq_key=sKeyVal.toInteger();
	}
	else
	{
		cerr<<"未配置下行事件通道键值\n";
		return false;
	}
	if( param_cfg.bGetMem("memory.MT_DSPCH_CMD.msgq_key_value", sKeyVal) && sKeyVal.isNumber() )
	{
		cmdq_key=sKeyVal.toInteger();
	}
	else
	{
		cerr<<"未配置命令通道键值\n";
		return false;
	}
	if( param_cfg.bGetMem("memory.MT_DSPCH_TASK.msgq_key_value", sKeyVal) && sKeyVal.isNumber() )
	{
		taskq_key=sKeyVal.toInteger();
	}
	else
	{
		cerr<<"未配置任务通道键值\n";
		return false;
	}
	
	//初始化
	m_iUpEvtMsgID=msgget(up_evtq_key, 0600);
	if(m_iUpEvtMsgID<0)
	{
		cerr<<"连接上行事件通道失败！\n";
		return false;
	}
	m_iDownEvtMsgID=msgget(down_evtq_key, 0600);
	if(m_iDownEvtMsgID<0)
	{
		cerr<<"连接下行事件通道失败！\n";
		return false;
	}
	m_iCmdMsgID=msgget(cmdq_key, 0600);
	if(m_iCmdMsgID<0)
	{
		cerr<<"连接命令通道失败！\n";
		return false;
	}
	m_iTaskMsgID=msgget(taskq_key, 0600);
	if(m_iTaskMsgID<0)
	{
		cerr<<"连接任务通道失败！\n";
		return false;
	}
	writeLog(LOG_CODE_PROCESS_START, "后台进程启动");

	int event_sn, event_type;
	long param1, param2, src_id;

	//将命令队列本进程的消息清空
	while(true)
	{
		int ret=getCmd(event_sn, event_type, param1, param2, src_id);
		if(ret<0) return false;
		else if(ret==0) break;
	}

	//进程注册
	if( !putEvt(0, EVT_REQ_REG, (int)m_lPrc_ID, getpid(), DSPCH_PRC_ID) )
	{
		cerr<<"请求注册失败！\n";
		return false;
	}
	
	while(true)
	{
		if( getEvt(event_sn, event_type, param1, param2, src_id)<0 ) return false;
		if(event_type==EVT_RESP_REG )
		{
			if(param1>0)
			{
				m_bHLLCtrl=(param1>1);
				m_bReg=true;
				if(param2==1)
				{
					cerr<<"进程被挂起！\n";
					m_bSuspSig=true;
				}
				break;
			}
			else
			{
				if(param2==0) cerr<<"注册失败！\n";
				else if(param2==2)
				{
					cerr<<"内存已使用量仅比核心参数设置的最大内存使用量少1M字节，进程启动失败！\n";
					cerr<<"可修改核心参数MEMORY.param.init_data_size的值以使得进程启动成功\n";
				}
				else
				{
					cerr<<"进程已在运行！\n";
					writeLog(LOG_CODE_APP_EXIST, "应用进程已经存在");
				}
				return false;
			}
		}
	}

	return true;
}

/*
bool PS_Process::threadLock()
{
	return ( pthread_mutex_lock(&m_Mutex)==0 );
}

bool PS_Process::threadUnlock()
{
	return ( pthread_mutex_unlock(&m_Mutex)==0 );
}

bool PS_Process::threadReadLock()
{
	return ( pthread_rwlock_rdlock(&m_RWLock)==0 );
}

bool PS_Process::threadWriteLock()
{
	return ( pthread_rwlock_wrlock(&m_RWLock)==0 );
}

bool PS_Process::threadRWUnlock()
{
	return ( pthread_rwlock_unlock(&m_RWLock)==0 );
}
*/

int PS_Process::getEvt(int &event_sn, int &event_type, long &param1, long &param2, long &src_id, bool sync)
{
	if(m_iDownEvtMsgID==-1) return -1;
	EVENT_MSG event_msg;
	while(true)
	{
		if( msgrcv(m_iDownEvtMsgID, &event_msg, sizeof(EVENT), m_lPrc_ID, sync?0:IPC_NOWAIT)<0 )
		{
			if(errno==ENOMSG || errno==0) return 0;
			else if(errno==EINTR && !gbExitSig)continue;
			else return -1;
		}
		else break;
	}
	event_sn=event_msg.event.event_sn;
	event_type=event_msg.event.event_type;
	param1=event_msg.event.param1;
	param2=event_msg.event.param2;
	src_id=event_msg.event.src_id;
	return 1;
}

int PS_Process::getCmd(int &cmd_sn, int &cmd_type, long &param1, long &param2, long &src_id)
{
	if(m_iCmdMsgID==-1) return -1;
	EVENT_MSG event_msg;
	while(true)
	{
		if( msgrcv(m_iCmdMsgID, &event_msg, sizeof(EVENT), m_lPrc_ID, IPC_NOWAIT)<0 )
		{
			if(errno==ENOMSG || errno==0) return 0;
			else if(errno==EINTR && !gbExitSig) continue;
			else return -1;
		}
		else break;
	}
	cmd_sn=event_msg.event.event_sn;
	cmd_type=event_msg.event.event_type;
	param1=event_msg.event.param1;
	param2=event_msg.event.param2;
	src_id=event_msg.event.src_id;
	return 1;
}

int PS_Process::getTask(int &event_sn, int &event_type, long &param1, long &param2, long &src_id)
{
	if(m_iTaskMsgID==-1) return -1;
	EVENT_MSG event_msg;
	while(true)
	{
		if( msgrcv(m_iTaskMsgID, &event_msg, sizeof(EVENT), m_lPrc_ID, IPC_NOWAIT)<0 )
		{
			if(errno==ENOMSG || errno==0) return 0;
			else if(errno==EINTR && !gbExitSig) continue;
			else return -1;
		}
		else break;
	}
	event_sn=event_msg.event.event_sn;
	event_type=event_msg.event.event_type;
	param1=event_msg.event.param1;
	param2=event_msg.event.param2;
	src_id=event_msg.event.src_id;
	return 1;
}

bool PS_Process::putEvt(int event_sn, int event_type, long param1, long param2, long dest_id)
{
	if(m_iUpEvtMsgID==-1) return false;
	EVENT_MSG event_msg;
	event_msg.msgtype=dest_id;
	event_msg.event.event_sn=event_sn;
	event_msg.event.event_type=event_type;
	event_msg.event.param1=param1;
	event_msg.event.param2=param2;
	event_msg.event.src_id=m_lPrc_ID;
	time(&(event_msg.event.event_time));
	if(dest_id==DSPCH_PRC_ID)
	{
		if( msgsnd(m_iUpEvtMsgID, &event_msg, sizeof(EVENT), IPC_NOWAIT)<0 ) return false;
	}
	else
	/*// 增加判断是否命令，Added by zhongk on 2007/06/20
	if (event_type/1000 == 3)
	{
		if( msgsnd(m_iCmdMsgID, &event_msg, sizeof(EVENT), IPC_NOWAIT)<0 ) return false;
	}
	else
	// 判断结束*/
	{
		if( msgsnd(m_iDownEvtMsgID, &event_msg, sizeof(EVENT), IPC_NOWAIT)<0 ) return false;
	}
	return true;
}

bool PS_Process::putTask(int event_sn, int event_type, long param1, long param2, long dest_id)
{
	if(m_iTaskMsgID==-1) return false;
	EVENT_MSG event_msg;
	event_msg.msgtype=dest_id;
	event_msg.event.event_sn=event_sn;
	event_msg.event.event_type=event_type;
	event_msg.event.param1=param1;
	event_msg.event.param2=param2;
	event_msg.event.src_id=m_lPrc_ID;
	time(&(event_msg.event.event_time));
	if(dest_id==DSPCH_PRC_ID) return false;
	if( msgsnd(m_iTaskMsgID, &event_msg, sizeof(EVENT), IPC_NOWAIT)<0 ) return false;
	return true;
}

// 广播事件，Added by zhongk on 2007/06/20
bool PS_Process::broadcastEvt(int event_type, long param1, long param2, short flow_id, short module_id)
{
	if(m_iUpEvtMsgID == -1) return false;
	EVENT_MSG event_msg;
	if (flow_id==0 && module_id==0)
		event_msg.msgtype = 99;
	else
		event_msg.msgtype = flow_id*1000000 + module_id*100;
	event_msg.event.event_sn = event_type;
	event_msg.event.event_type = EVT_BROADCAST;
	event_msg.event.param1 = param1;
	event_msg.event.param2 = param2;
	event_msg.event.src_id = m_lPrc_ID;
	time(&(event_msg.event.event_time));
	if( msgsnd(m_iUpEvtMsgID, &event_msg, sizeof(EVENT), IPC_NOWAIT) < 0 )
		return false;
	return true;
}

bool PS_Process::suspend(long param1, long param2)
{
	int event_sn, resp_event_type;
	long resp_param1, resp_param2, src_id;
	if( !putEvt(0, EVT_REQ_SUSP, param1, param2, DSPCH_PRC_ID) ) return false;
	while(true)
	{
		if( getEvt(event_sn, resp_event_type, resp_param1, resp_param2, src_id)<0 ) return false;
		if(resp_event_type==EVT_RESP_SUSP)
		{
			if(resp_param1==1) break;
			else if(resp_param1==0) return false;
		}
	}
	while(true)
	{
		if( !putEvt(0, EVT_REQ_CHKSUSP, param1, param2, DSPCH_PRC_ID) ) return false;
		if( getEvt(event_sn, resp_event_type, resp_param1, resp_param2, src_id)<0 ) return false;
		if(resp_event_type==EVT_RESP_CHKSUSP && resp_param1==1) break;
		else sleep(1);
	}
	return true;
}

bool PS_Process::resume(long param1, long param2)
{
	int event_sn, resp_event_type;
	long resp_param1, resp_param2, src_id;
	if( !putEvt(0, EVT_REQ_RESUME, param1, param2, DSPCH_PRC_ID) ) return false;
	while(true)
	{
		if( getEvt(event_sn, resp_event_type, resp_param1, resp_param2, src_id)<0 ) return false;
		if(resp_event_type==EVT_RESP_RESUME)
		{
			if(resp_param1==1) break;
			else if(resp_param1==0) return false;
		}
	}
	return true;
}

/*bool PS_Process::notify(int trigger_event_type)
{
	int resp_event_sn, resp_event_type;
	long resp_param1, resp_param2;
	long src_id;
	vector<EventRelation> v_evt_relation;
	vector<EventRelation>::iterator it;
	InternalEventRelation int_evt_relation;
	int ret=int_evt_relation.getTargetEvent(trigger_event_type, v_evt_relation);
	if(ret==0) return true;
	else if(ret<0) return false;
	for(it=v_evt_relation.begin(); it!=v_evt_relation.end(); it++)
	{
		if(it->target_event_id==EVT_REQ_SUSP) return false;
		else
		{
			EVENT_MSG event_msg;
			event_msg.msgtype=it->process_id;
			event_msg.event.event_sn=0;
			event_msg.event.event_type=it->target_event_id;
			event_msg.event.param1=it->event_param1;
			event_msg.event.param2=it->event_param2;
			event_msg.event.src_id=m_lPrc_ID;
			time(&(event_msg.event.event_time));
			if( msgsnd(m_iTaskMsgID, &event_msg, sizeof(EVENT), IPC_NOWAIT)<0 ) return false;
			while(true)
			{
				if( getEvt(resp_event_sn, resp_event_type, resp_param1, resp_param2, src_id)<0 ) return false;
				if(resp_param1==1) break;
				else sleep(1);
			}
		}
	}
	return true;
}*/

bool PS_Process::rpt_hllctrl(int cmd_sn, int hll_type, int child_num)
{
	return putEvt(cmd_sn, EVT_RPT_HLLCTRL, hll_type, child_num, DSPCH_PRC_ID);
}

bool PS_Process::writeSendPkgInfo(const void *head, int headSize, const void *body, int bodySize)
{
	bool ret=m_LogInfo.writeSendPackage(head, headSize, body, bodySize);
	if(!ret)
	{
		cerr<<"写发送包信息点失败，errmsg="<<m_LogInfo.getLastErrorMessage()<<endl;
	}
	return ret;
}

bool PS_Process::writeRecvPkgInfo(const void *head, int headSize, const void *body, int bodySize)
{
	bool ret=m_LogInfo.writeRecvPackage(head, headSize, body, bodySize);
	if(!ret)
	{
		cerr<<"写接收包信息点失败，errmsg="<<m_LogInfo.getLastErrorMessage()<<endl;
	}
	return ret;
}

bool PS_Process::writeLog(unsigned int logcode, int ne_cind, unsigned char direct, char *logcontent)
{
	bool ret=m_LogInfo.writelog(logcode, ne_cind, direct, logcontent);
	if(!ret)
	{
		cerr<<"写日志失败，errmsg="<<m_LogInfo.getLastErrorMessage()<<endl;
	}
	return ret;
}

bool PS_Process::writeLog(unsigned int logcode, char *logcontent)
{
	bool ret=m_LogInfo.writelog(logcode, logcontent);
	if(!ret)
	{
		cerr<<"写日志失败，errmsg="<<m_LogInfo.getLastErrorMessage()<<endl;
	}
	return ret;
}

/******* xieyzh **************************************************************
bool PS_Process::writeLog(int code_id, const char* sLogMsg, const char * sParamName, int iSqlType, const char* log_desc)
{
	if(g_LogMemMng == NULL)
		return true;

	/ *if( pthread_mutex_lock(&m_LogMutex)!=0 )
	{
		cout << "写日志加线程锁失败！\n";
		return false;
	}* /
	bool res = g_LogMemMng->writeLogMem(code_id, sLogMsg, sParamName, iSqlType, log_desc);
    if (!res)
    {
		/ *if( pthread_mutex_unlock(&m_LogMutex)!=0 )
		{
			cout << "写日志解线程锁失败！\n";
		}* /
		cout << "写日志失败，errmsg="<< g_LogMemMng->getMessage()  <<  ", errcode=" << g_LogMemMng->getErrCode() <<endl;
		return false;
    }
	/ *if( pthread_mutex_unlock(&m_LogMutex)!=0 )
	{
		cout << "写日志解线程锁失败！\n";
		return false;
	}* /
	return true;
}
************************************************************************

bool PS_Process::writeLog(int code_id, const char* log_desc)
{
	//return m_Log.writeLog(code_id, log_desc);
	
	
	/ *******xieyzh*************************************************************
	if(g_LogMemMng == NULL)
		return true;

	/ *if( pthread_mutex_lock(&m_LogMutex)!=0 )
	{
		cout << "写日志加线程锁失败！\n";
		return false;
	}* /

	bool res = g_LogMemMng->writeLogMem(code_id, log_desc);
    if (!res)
    {
        / *if( pthread_mutex_unlock(&m_LogMutex)!=0 )
		{
			cout << "写日志解线程锁失败！\n";
		}* /
		cout << "写日志失败，errmsg="<< g_LogMemMng->getMessage()  <<  ", errcode=" << g_LogMemMng->getErrCode() <<endl;
		return false;
    }
	/ *if( pthread_mutex_unlock(&m_LogMutex)!=0 )
	{
		cout << "写日志解线程锁失败！\n";
		return false;
	}* /
	return true;
}
*/

void PS_Process::prcExit()
{
	//如果进程未注销，进程注销
	if(m_bReg)
	{
		if( putEvt(0, EVT_RPT_UNREG, m_lPrc_ID, getpid(), DSPCH_PRC_ID) ) m_bReg=false;
	}

	writeLog(LOG_CODE_PROCESS_STOP, "后台进程执行结束");

	m_LogInfo.shutdown();
	/********* xieyzh *********************************************************
	if (g_LogMemMng)
    {
        delete g_LogMemMng;
        g_LogMemMng = NULL;
		//cout<<"释放SGW_LogMemMng内存指针"<<endl;
    }
	************************************************************************/

	exit(0);
}

bool PS_Process::getBreakSig(bool clear_sig)
{ 
	if(gbBreakSig)
	{
		if(clear_sig) gbBreakSig=false;
		return true;
	}
	return false;
}

bool PS_Process::onInit()
{
	return true;
}

int PS_Process::onCmd(int cmd_sn, int cmd_type, long param1, long param2, long src_id)
{
	return 1;
}

void PS_Process::onExit()
{}

void PS_Process::run()
{
	cerr<<"没有实现run()函数\n";
}
