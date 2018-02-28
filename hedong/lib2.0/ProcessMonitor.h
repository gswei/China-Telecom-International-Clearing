// ProcessMonitor.h: interface for the CProcessMonitor class.
//
//////////////////////////////////////////////////////////////////////
/********************************************************************
 *	Usage		: ���̼�ع����ڴ�
 *				: CProcInfo--������Ϣ��
 *				: CProcInfoManager--�����ڴ������
 *				: CProcessMonitor--���̼����
 *	Author		: ���Ǿ�
 *  Create date	: 2005-04-28
 *	Version		: 1.0.0
 *	Updatelist	:
 *				  2005-05-18 ���Ǿ� ���쳣����ȼ����м���Ϊ�߼�; �޸�CProcessMonitor��Attach(), 
 *									Detach(), Init()����, ��ԭ���Ĵ��󷵻ظ�Ϊ�׳��쳣
 *				  2005-05-30 ���Ǿ� �޸�CProcessMonitor::Detach()����, ���Ͽ�ʱ��״̬��Ϊͨ�����
 *									����ָ��;
 *				  2006-01-23 ���Ǿ� �޸�CProcessMonitor��Init(),������״̬��ΪEXITʱ,�����̺�
 *									��Ӧ�Ľ����Ƿ���Ĵ���,������׳��쳣,��������ظ�����;����
 *									������,�����ֱ������
 *				  2006-02-15 ���Ǿ� �޸�CProcessMonitor::Detach(),��δ��ʼ�����Ͽ�ʱ,�ж�m_nIdx
 *									�Ƿ�Ϊ-1�ٸ��¹����ڴ�״̬
 *				  2006-02-16 ���Ǿ� �޸�CProcInfoManager::DestroyShareMem(),��ȫɾ��ʱ,���ж�m_pProcInfo
 *									�Ƿ�ΪNULL,��ȥ��ȡ�����ڴ�״̬
 *				  2006-02-22 ���Ǿ� ��ԭ��д����KEY_ID��Ϊ������������HOMEĿ¼ֵ,Ȼ���ٹ���KEY_ID
 *				  2006-03-18 ���Ǿ� ����һ����ȡKEY_ID�Ľӿ�
 ********************************************************************/

#if !defined(_PROCESSMONITOR_H_)
#define _PROCESSMONITOR_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <sys/shm.h>
#include "config.h"

#define	MAX_PROC_COUNT	500

#define MSG_ATTACH_FAIL	"Failed to attach the share memory!"

class CProcInfoManager;
class CProcessMonitor;

class CProcInfo
{
	friend class CProcInfoManager;
	friend class CProcessMonitor;
private:
	//��������Ψһȷ��һ����
	char	m_pchPipeID[5+1];			//��ˮ��
	int		m_nProcessID;				//����������к�

	char	m_pchProcName[30];			//������
	long	m_nProcID;					//����ID, ϵͳ����Ľ��̺�
	long	m_nStartTime;				//��������ʱ��
	long	m_nLastTime;				//�����ϴλʱ��
	char	m_pchStatus[10];			//����״̬
	char	m_pchDealFileName[255+1];	//���һ�δ�����ļ���
	int		m_nSpeed;					//������������(��/��)
	char	m_pchUsedCpu[10+1];			//���̵�cpuʹ����
	char	m_pchUsedMem[10+1];			//����ռ�õ��ڴ�(KB)
	long	m_nCounter;					//���̴��������
	unsigned long	m_nTotalCount;		//�����������ڴ���Ļ�����

public:
	typedef enum 
	{ 
		PROC_BUSY, 
		PROC_IDLE, 
		PROC_EXIT, 
		PROC_UNKNOWN 
	}ProcStatus;

	CProcInfo();
	CProcInfo(char* pchPipeID, int nProcessID, char* pchProcName, int nProcID = 0);
	CProcInfo(const CProcInfo &right);
	virtual ~CProcInfo();

	CProcInfo& operator =(const CProcInfo &right);
	bool operator ==(const CProcInfo &right) const;

	void Reset(void);
	void SetPipeID(char* pchPipeID);
	void SetProcessID(int nProcessID);
	void SetProcName(char* pchProcName);
	void SetProcStatus(ProcStatus status = PROC_EXIT);

	char* GetPipeID(void);
	char* GetProcName(void);
	char* GetProcStatus(void);
	char* GetDealFileName(void);
	int	  GetProcessID(void);
	int   GetProcID(void);
	int	  GetSpeed(void);
	long  GetStartTime(void);
	long  GetLastTime(void);
	long  GetTotalCount(void);

};

class CProcInfoManager
{
private:
	CProcInfo	*m_pProcInfo;
	int			m_nShmID;
	key_t		m_shmKey;
	
public:
	CProcInfoManager();
	virtual ~CProcInfoManager();

	bool CreateShareMem(void);
	bool DestroyShareMem(bool bSafeDel = true);
	CProcInfo* GetShareMem(void);
	void ResetShareMem(void);

	bool AttachShareMem(bool bReAttach = true);
	bool DetachShareMem(void);

	int GetProcInfoIndex(const CProcInfo &procInfo);
	int WriteProcInfo(int nIdx, const CProcInfo &procInfo);
	int ReadProcInfo(int nIdx, CProcInfo &procInfo);
	
	key_t GetKeyID(void);
};

class CProcessMonitor  
{
private:
	CProcInfoManager	m_ProcInfoMgr;
	int					m_nIdx;
	bool				m_bReAttach;
	
public:
	CProcInfo			m_ProcInfo;

public:
	CProcessMonitor(bool bReAttach = true);
	virtual ~CProcessMonitor();

	bool Attach(void);
	bool Detach(CProcInfo::ProcStatus status = CProcInfo::PROC_EXIT);

	int Init(char* pchPipeID, int nProcessID, char* pchProcName, int nIdx);
	bool SetProcInfoIndex(int nIdx);

	void UpdateMonitor(CProcInfo::ProcStatus status = CProcInfo::PROC_IDLE, 
		char *pchDealFileName = NULL);
	void ReadProcInfo();

	CProcessMonitor& operator ++();
	CProcessMonitor& operator ++(int n);
};

#endif // !defined(_PROCESSMONITOR_H_)
