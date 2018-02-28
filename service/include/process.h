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

//�������¼�(������Ӧ)
#define EVT_RPT_UNREG		2001	//����ע��
#define EVT_RPT_HRTBT		2002	//����
#define EVT_RPT_TASKEND		2003	//�������
#define EVT_RPT_PRCST		2004	//�������״̬
#define EVT_RPT_PERF		2005	//��������
#define EVT_RPT_HLLCTRL		2006	//�ߵ�ˮ���Ʊ���

//�������Ӧ���¼�(1000����Ϊ�����࣬1000����Ϊ��Ӧ�࣬ģ��1000��ȵ�Ϊһ���������Ӧ)
#define EVT_RESP_ERR		-1		//�������
#define EVT_REQ_REG			1		//��������ע��
#define EVT_RESP_REG		1001	//��Ӧ����ע��
#define EVT_REQ_TASK		2		//��������
#define EVT_RESP_TASK		1002	//��������
#define EVT_REQ_SUSP		3		//����������
#define EVT_RESP_SUSP		1003	//��Ӧ���̹���
#define EVT_REQ_START		4		//������������
#define EVT_RESP_START		1004	//��Ӧ��������
#define EVT_REQ_STOP		5		//����ֹͣ����
#define EVT_RESP_STOP		1005	//��Ӧ����ֹͣ
#define EVT_REQ_RESUME		6		//ȡ������
#define EVT_RESP_RESUME		1006	//��Ӧȡ������
#define EVT_REQ_REBSHM		7		//�����ؽ��ڴ�
#define EVT_RESP_REBSHM		1007	//��Ӧ�����ؽ��ڴ�
#define EVT_REQ_CHKSUSP		8		//��ѯ����״̬
#define EVT_RESP_CHKSUSP	1008	//��Ӧ��ѯ����״̬
#define EVT_REQ_CHKSTOP		9		//��ѯ��ֹ״̬
#define EVT_RESP_CHKSTOP	1009	//��Ӧ��ѯ��ֹ״̬
#define EVT_REQ_TRNUM		10		//���봫�仰��
#define EVT_RESP_TRNUM  	1010	//��Ӧ���仰��


//������
#define EVT_CMD_SUSP		3001	//���̹���
#define EVT_CMD_RESUME		3002	//����ȡ������
#define EVT_CMD_STOP		3003	//�����˳�
#define EVT_CMD_DETACHSHM	3004	//�ͷŹ����ڴ�	Added by zhongk on 2007/06/20
#define EVT_CMD_HLLCTRL		3005	//�ߵ�ˮ����

//Added by zhongk on 2007/06/20
#define EVT_BROADCAST		1000	//�㲥�¼�

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
	static Table<struct TP_MODULE> m_Tbl_TP_MODULE;		//ģ���
	static TableIndex<TP_MODULE>* m_TblIdx_TP_MODULE;	//ģ�������
	static bool m_bTblInit;
	static CWriteLogInfo m_LogInfo;
	bool   chkArgs(int argc, char *argv[]);

public:
	void setSignalPrc();

	//��ģ�����ƻ��ģ��ID�� ����ֵ>0�ɹ���<0ʧ��
	static int getMdlIDByName(char *mdl_name);

	//��ģ�����ƻ�ý���ID������ֵ>0�ɹ���<0ʧ��
	static long getPrcIDByName(char *mdl_name, int flow_id=0, int inst_id=1);	//�����ڲ����̱�ʶ(����2λ������)����������Ϣ����Ĳ���

	//��ģ�����ƻ��ϵͳ����ID������ֵ>0�ɹ���<0ʧ��
	static long getSysPrcIDByName(char *mdl_name, int flow_id=0, int inst_id=1); //�����ڲ����̱�ʶ(��2λ������)�������ڳ�ʼ����־�ӿ�

	static CWriteLogInfo *getLogInfoPointer() { return &m_LogInfo; }

	//д������Ϣ����Ϣ��
	bool writeSendPkgInfo(const void *head/*��ͷָ��*/, int headSize/*��ͷ��С*/, const void *body/*����ָ��*/, int bodySize/*�����С*/); 

	//д������Ϣ����Ϣ��
	bool writeRecvPkgInfo(const void *head/*��ͷָ��*/, int headSize/*��ͷ��С*/, const void *body/*����ָ��*/, int bodySize/*�����С*/);

	//д��־�����ɾ�̬��������Ϊ�źŲ�׽�������е���
	static bool writeLog(unsigned int logcode/*��־����*/, int ne_cind/*��Ԫ����*/, unsigned char direct/*��Ϣ������*/, char *logcontent/*��־����*/);
	static bool writeLog(unsigned int logcode, char *logcontent);

	//д��־
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
	IBC_ParamCfgMng param_cfg;	//���Ĳ�����ȡ��

	//����˳��ź�
	bool getExitSig() { return gbExitSig; }
	
	//bool getSuspSig() { return m_bSuspSig; }
	
	//��ô���ź�
	bool getBreakSig(bool clear_sig=true);

	//��������ߺ�
	int	 getFlowID()  { return m_iFlow_ID; }

	//��ý��̱�ʶ
	long getPrcID()   { return m_lsysPrcID; }	//���ص������ñ��е�process_id(��2λ������)

	//���ģ���ʶ
	int	 getModuleID(){ return m_iMdl_ID; }
	
	//��ý���ʵ����
	int	 getInstID()  { return m_iInst_ID; }
	
	//���������
	int	 getHostID()  { return m_iHost_ID; }

	bool init(int argc, char *argv[]);
	int  getCmd(int &cmd_sn, int &cmd_type, long &param1, long &param2, long &src_id);
	int  getTask(int &event_sn, int &event_type, long &param1, long &param2, long &src_id);

	//�̼߳���
	bool threadLock();

	//�߳̽�������threadLock��Ӧ
	bool threadUnlock();

	//���̶߳���
	bool threadReadLock();

	//���߳�д��
	bool threadWriteLock();

	//���̶߳�д������threadReadLock��threadWriteLock��Ӧ
	bool threadRWUnlock();

	//��ȡ�¼�
	int  getEvt(int &event_sn, int &event_type, long &param1, long &param2, long &src_id, bool sync=true);

	//�����¼�
	bool putEvt(int event_sn, int event_type, long param1, long param2, long dest_id=DSPCH_PRC_ID);

	//�㲥�¼�
	bool broadcastEvt(int event_type, long param1, long param2, short flow_id =0, short module_id =0);	// �㲥�¼���Added by zhongk on 2007/06/20

	//��������
	bool putTask(int event_sn, int event_type, long param1, long param2, long dest_id);
	
	//������������
	bool suspend(long param1, long param2);
	
	//�����������
	bool resume(long param1, long param2);

	//bool notify(int trigger_event_type);
	
	//�ߵ�ˮ���Ʊ���
	bool rpt_hllctrl(int cmd_sn, int hll_type, int child_num=1);

	void prcExit();
	
	virtual void run();
	
	//��ʼ�ɹ�����true��ʧ�ܷ���false
	virtual bool onInit();

	//����ɹ�����1������ʧ�ܷ���-1�����������˳�����-10���쳣�˳�����-11
	virtual int  onCmd(int cmd_sn, int cmd_type, long param1, long param2, long src_id);
	
	virtual void onExit();
};

void sig_prc(int sig_type);
void usersig_prc(int sig_type);

#endif
