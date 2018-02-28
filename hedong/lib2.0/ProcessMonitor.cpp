// ProcessMonitor.cpp: implementation of the CProcessMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include <errno.h>
#include "Common.h"
#include "CF_CError.h"
#include "ProcessMonitor.h"

#define READ_WRITE_FAIL(errcode) \
	{\
		char pchMsg[ERROR_MSG_LEN]; \
		sprintf(pchMsg, "Failed to read or write the share memory! nIdx = %d.", m_nIdx); \
		throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG, errcode, errno, pchMsg, __FILE__, __LINE__); \
	}

//////////////////////////////////////////////////////////////////////
// class CProcInfo
//////////////////////////////////////////////////////////////////////
CProcInfo::CProcInfo()
{
	SetProcStatus();
	
	memset(m_pchPipeID, 0, sizeof(m_pchPipeID));
	memset(m_pchProcName, 0, sizeof(m_pchProcName));
	memset(m_pchDealFileName, 0, sizeof(m_pchDealFileName));
	memset(m_pchUsedCpu, 0, sizeof(m_pchUsedCpu));
	memset(m_pchUsedMem, 0, sizeof(m_pchUsedMem));
	
	m_nProcessID = 0;
	m_nProcID = 0;
	m_nStartTime = 0;
	m_nLastTime = 0;
	m_nTotalCount = 0;
	m_nCounter = 0;
	m_nSpeed = 0;
}

CProcInfo::CProcInfo(char* pchPipeID, int nProcessID, char* pchProcName, int nProcID)
{
	SetProcStatus();
	
	strcpy(m_pchPipeID, pchPipeID);
	strcpy(m_pchProcName, pchProcName);

	memset(m_pchDealFileName, 0, sizeof(m_pchDealFileName));
	memset(m_pchUsedCpu, 0, sizeof(m_pchUsedCpu));
	memset(m_pchUsedMem, 0, sizeof(m_pchUsedMem));
	
	m_nProcessID = nProcessID;
	m_nProcID = nProcID;
	m_nStartTime = 0;
	m_nLastTime = 0;
	m_nTotalCount = 0;
	m_nCounter = 0;
	m_nSpeed = 0;
}

CProcInfo::CProcInfo(const CProcInfo &right)
{
	strcpy(m_pchStatus, right.m_pchStatus);
	strcpy(m_pchPipeID, right.m_pchPipeID);
	strcpy(m_pchProcName, right.m_pchProcName);
	strcpy(m_pchDealFileName, right.m_pchDealFileName);
	strcpy(m_pchUsedCpu, right.m_pchUsedCpu);
	strcpy(m_pchUsedMem, right.m_pchUsedMem);
	
	m_nProcessID = right.m_nProcessID;
	m_nProcID = right.m_nProcID;
	m_nStartTime = right.m_nStartTime;
	m_nLastTime = right.m_nLastTime;
	m_nTotalCount = right.m_nTotalCount;
	m_nCounter = right.m_nCounter;
	m_nSpeed = right.m_nSpeed;
}

CProcInfo::~CProcInfo()
{
}

CProcInfo& CProcInfo::operator =(const CProcInfo &right)
{
	if (this == &right) 
		return *this;

	strcpy(m_pchStatus, right.m_pchStatus);
	strcpy(m_pchPipeID, right.m_pchPipeID);
	strcpy(m_pchProcName, right.m_pchProcName);
	strcpy(m_pchDealFileName, right.m_pchDealFileName);
	strcpy(m_pchUsedCpu, right.m_pchUsedCpu);
	strcpy(m_pchUsedMem, right.m_pchUsedMem);
	
	m_nProcessID = right.m_nProcessID;
	m_nProcID = right.m_nProcID;
	m_nStartTime = right.m_nStartTime;
	m_nLastTime = right.m_nLastTime;
	m_nTotalCount = right.m_nTotalCount;
	m_nCounter = right.m_nCounter;
	m_nSpeed = right.m_nSpeed;

	return *this;
}

//�ж����������Ƿ���ͬһ������Ϣ
bool CProcInfo::operator ==(const CProcInfo &right) const
{
	if(strcmp(m_pchPipeID, right.m_pchPipeID) == 0 
		&& m_nProcessID == right.m_nProcessID)
		return true;
	else
		return false;
}

/***********************************************************************
*��������:		��������Ϣ��λ
*�������:		None
*�������:		None
*����ֵ:		None
***********************************************************************/
void CProcInfo::Reset(void)
{
	SetProcStatus();
	
	memset(m_pchPipeID, 0, sizeof(m_pchPipeID));
	memset(m_pchProcName, 0, sizeof(m_pchProcName));
	memset(m_pchDealFileName, 0, sizeof(m_pchDealFileName));
	memset(m_pchUsedCpu, 0, sizeof(m_pchUsedCpu));
	memset(m_pchUsedMem, 0, sizeof(m_pchUsedMem));
	
	m_nProcessID = 0;
	m_nProcID = 0;
	m_nStartTime = 0;
	m_nLastTime = 0;
	m_nTotalCount = 0;
	m_nCounter = 0;
	m_nSpeed = 0;
}

void CProcInfo::SetPipeID(char* pchPipeID)
{
	if(pchPipeID != NULL)
		strcpy(m_pchPipeID, pchPipeID);
}

void CProcInfo::SetProcessID(int nProcessID)
{
	m_nProcessID = nProcessID;
}

void CProcInfo::SetProcName(char* pchProcName)
{
	if(pchProcName != NULL)
		strcpy(m_pchProcName, pchProcName);
}

void CProcInfo::SetProcStatus(ProcStatus status)
{
	switch(status) 
	{
	case PROC_BUSY:
		strcpy(m_pchStatus, "BUSY");
		break;
	case PROC_IDLE:
		strcpy(m_pchStatus, "IDLE");
		break;
	case PROC_EXIT:
		strcpy(m_pchStatus, "EXIT");
		break;
	default:
		strcpy(m_pchStatus, "UNKNOWN");
		break;
	}
}

char* CProcInfo::GetPipeID(void)
{
	return m_pchPipeID;
}

char* CProcInfo::GetProcName(void)
{
	return m_pchProcName;
}

char* CProcInfo::GetProcStatus(void)
{
	return m_pchStatus;
}

char* CProcInfo::GetDealFileName(void)
{
	return m_pchDealFileName;
}

int CProcInfo::GetProcessID(void)
{
	return m_nProcessID;
}

int CProcInfo::GetProcID(void)
{
	return m_nProcID;
}

int	CProcInfo::GetSpeed(void)
{
	return m_nSpeed;
}

long CProcInfo::GetStartTime(void)
{
	return m_nStartTime;
}

long CProcInfo::GetLastTime(void)
{
	return m_nLastTime;
}

long CProcInfo::GetTotalCount(void)
{
	return m_nTotalCount;
}

//////////////////////////////////////////////////////////////////////
// class CProcInfoManager
//////////////////////////////////////////////////////////////////////
CProcInfoManager::CProcInfoManager()
: m_pProcInfo(NULL), m_nShmID(0)
{
	m_shmKey = ftok(getenv("HOME"), 0x2F);	
}

CProcInfoManager::~CProcInfoManager()
{
}

/***********************************************************************
*��������:		�������̼����Ϣ�����ڴ�
*�������:		None
*�������:		None
*����ֵ:		true:		�ɹ�����
*				false:		����ʧ��
***********************************************************************/
bool CProcInfoManager::CreateShareMem(void)
{
	m_nShmID = shmget(m_shmKey, sizeof(CProcInfo)*MAX_PROC_COUNT, 0666|IPC_CREAT|IPC_EXCL);
	if(m_nShmID <= 0)
	{
		if(errno == EEXIST)//�����ڴ��Ѿ�����
		{
			return AttachShareMem();
		}
		else
			return false;
	}
	
	m_pProcInfo = (CProcInfo *)shmat(m_nShmID, 0, 0);

	/*���ӵ������ڴ�*/
	/*m_pProcInfo=-1��ʾshmatʧ��*/
	if (m_pProcInfo == (CProcInfo *)-1)
		return false;

	ResetShareMem();

	return true;	
}

/***********************************************************************
*��������:		ɾ�����̼����Ϣ�����ڴ�
*�������:		bSafeDel:	��ȫɾ����ʶ, true: ��⻹��û�н������ӵ������ڴ�,����, ɾ��ʧ��
*�������:		None
*����ֵ:		true:		�ɹ�ɾ��
*				false:		ɾ��ʧ��
***********************************************************************/
bool CProcInfoManager::DestroyShareMem(bool bSafeDel)
{
	m_nShmID = shmget(m_shmKey, 0, 0666);
	if (m_nShmID <= 0)
	{
		return false;
	}

	if(bSafeDel)
	{
		if(m_pProcInfo == NULL)
		{
			m_pProcInfo = (CProcInfo *)shmat(m_nShmID, 0, 0);
			if (m_pProcInfo == (CProcInfo *)-1)
				return false;
		}
		
		for(int i = 0; i < MAX_PROC_COUNT; i++)
		{
			if(strcmp((m_pProcInfo + i)->GetProcStatus(), "EXIT") != 0)
				return false;
		}
	}

	if(-1 == shmctl(m_nShmID, IPC_RMID, 0))
	{
		return false;
	}

	return true;
}

/***********************************************************************
*��������:		��ȡ�����ڴ��ַ
*�������:		None
*�������:		None
*����ֵ:		�����ڴ��ַ, ʧ�ܷ���NULL
***********************************************************************/
CProcInfo* CProcInfoManager::GetShareMem(void)
{
	return m_pProcInfo;
}

/***********************************************************************
*��������:		�����̼����Ϣ�����ڴ渴λ
*�������:		None
*�������:		None
*����ֵ:		None
***********************************************************************/
void CProcInfoManager::ResetShareMem(void)
{
	for(int i = 0; i < MAX_PROC_COUNT; i++)
		(m_pProcInfo + i)->Reset();
}

/***********************************************************************
*��������:		���ӽ��̼����Ϣ�����ڴ�
*�������:		bReAttach:	ǿ��������ʶ, true��ʾ��ǿ������, false��ʾ��������
*�������:		None
*����ֵ:		true:		���ӳɹ�
*				false:		����ʧ��
***********************************************************************/
bool CProcInfoManager::AttachShareMem(bool bReAttach)
{
	if(bReAttach)
	{
		if (m_pProcInfo)
			shmdt((char *)m_pProcInfo);

		m_nShmID = 0;
		m_pProcInfo = NULL;
	}

	if (m_nShmID <= 0)
	{
		m_nShmID = shmget(m_shmKey, sizeof(CProcInfo)*MAX_PROC_COUNT, 0666);
		
		if (m_nShmID <= 0)//�����ڴ治����
			return false;
	}
	
	if (!m_pProcInfo)
	{
		m_pProcInfo = (CProcInfo *)shmat(m_nShmID, 0, 0);
		
		/*���ӵ������ڴ�*/
		/*m_pProcInfo=-1��ʾshmatʧ��*/
		if (m_pProcInfo == (CProcInfo *)-1)
			return false;
	}
	
	return true;	
}

/***********************************************************************
*��������:		�Ͽ��ͽ��̼����Ϣ�����ڴ������
*�������:		None
*�������:		None
*����ֵ:		true:		�Ͽ����ӳɹ�
*				false:		�Ͽ�����ʧ��
***********************************************************************/
bool CProcInfoManager::DetachShareMem(void)
{
	if (m_pProcInfo)
	{
		shmdt((char *)m_pProcInfo);
		m_pProcInfo = NULL;
	}

	return true;
}

/***********************************************************************
*��������:		��ȡָ��������Ϣ�ڹ����ڴ��е�����
*�������:		procInfo:	ָ���Ľ�����Ϣ
*�������:		None
*����ֵ:		i(>=0):		��ȡ�ɹ���������
*				-1:			δ���ӵ������ڴ�
*				-2:			δ�ҵ�����
***********************************************************************/
int CProcInfoManager::GetProcInfoIndex(const CProcInfo &procInfo)
{
	if(!m_pProcInfo)
		return -1;

	int i = 0;
	for(; i < MAX_PROC_COUNT; i++)
	{
		if(*(m_pProcInfo + i) == procInfo)
			return i;
	}

	return -2;
}

/***********************************************************************
*��������:		��ָ�����̵ļ����Ϣд�빲���ڴ�
*�������:		nIdx:		ָ���Ľ�����Ϣ�ڹ����ڴ��е�����
*				procInfo:	ָ���Ľ�����Ϣ
*�������:		None
*����ֵ:		i(>=0):		д��ɹ�, ��������
*				-1:			δ���ӵ������ڴ�
*				-2:			�����������
***********************************************************************/
int CProcInfoManager::WriteProcInfo(int nIdx, const CProcInfo &procInfo)
{
	if(!m_pProcInfo)
		return -1;

	if(nIdx < 0 || nIdx > MAX_PROC_COUNT)
		return -2;

	*(m_pProcInfo + nIdx) = procInfo;
	return nIdx;
}

/***********************************************************************
*��������:		�ӹ����ڴ��ж�ȡ���̵ļ����Ϣ
*�������:		nIdx:		ָ���ĵ�����
*�������:		procInfo:	������Ϣ
*����ֵ:		i(>=0):		��ȡ�ɹ�, ��������
*				-1:			δ���ӵ������ڴ�
*				-2:			�����������
***********************************************************************/
int CProcInfoManager::ReadProcInfo(int nIdx, CProcInfo &procInfo)
{
	if(!m_pProcInfo)
		return -1;

	if(nIdx < 0 || nIdx > MAX_PROC_COUNT)
		return -2;

	procInfo = *(m_pProcInfo + nIdx);
	return nIdx;	
}

key_t CProcInfoManager::GetKeyID(void)
{
	return m_shmKey;
}

//////////////////////////////////////////////////////////////////////
// class CProcessMonitor
//////////////////////////////////////////////////////////////////////
CProcessMonitor::CProcessMonitor(bool bReAttach)
: m_nIdx(-1), m_bReAttach(bReAttach)
{
}

CProcessMonitor::~CProcessMonitor()
{
}

bool CProcessMonitor::Attach(void)
{
	if(!m_ProcInfoMgr.AttachShareMem(m_bReAttach))
		throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG, 
			PROCINFO_ERR_ATTACH_FAIL, errno, 
			MSG_ATTACH_FAIL, __FILE__, __LINE__);
	
	return true;
}

bool CProcessMonitor::Detach(CProcInfo::ProcStatus status)
{
	if(m_nIdx != -1)
		UpdateMonitor(status);

	if(!m_ProcInfoMgr.DetachShareMem())
		throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG, 
			PROCINFO_ERR_DETACH_FAIL, errno, 
			MSG_ATTACH_FAIL, __FILE__, __LINE__);
	
	return true;
}

/***********************************************************************
*��������:		��ʼ�����̼����Ϣ
*�������:		pchPipeID:		���̵�PIPE_ID
*				nProcessID:		����������к�
*				pchProcName:	���̵�����
*				nIdx:			������Ϣ���ڴ��е�����
*�������:		None
*����ֵ:		nIdx(>=0):		��ʼ���ɹ�, ��������
***********************************************************************/
int CProcessMonitor::Init(char* pchPipeID, int nProcessID, char* pchProcName, int nIdx)
{
	if(nIdx < 0 || nIdx > MAX_PROC_COUNT)
		throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG, 
			PROCINFO_ERR_MEMIDX, errno, 
			MSG_ATTACH_FAIL, __FILE__, __LINE__);

	if(m_bReAttach)
	{
		if(!m_ProcInfoMgr.AttachShareMem(m_bReAttach))
			throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG, 
				PROCINFO_ERR_ATTACH_FAIL, errno, 
				MSG_ATTACH_FAIL, __FILE__, __LINE__);
	}

	//ͬһPIPE�µ�ͬһ������ͬʱ�������
	m_ProcInfoMgr.ReadProcInfo(nIdx, m_ProcInfo);
	//Modified by ZhouYajun, 20060123
	//if(strcmp(m_ProcInfo.m_pchStatus, "EXIT") != 0)
	if(strcmp(m_ProcInfo.m_pchStatus, "EXIT") != 0 && isProcExist(m_ProcInfo.m_nProcID) != 0)
	//=====End=====
		throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG, 
			PROCINFO_ERR_PROCESS_ALREADY_RUN, errno, 
			MSG_ATTACH_FAIL, __FILE__, __LINE__);

	strcpy(m_ProcInfo.m_pchPipeID, pchPipeID);
	m_ProcInfo.m_nProcessID = nProcessID;
	strcpy(m_ProcInfo.m_pchProcName, pchProcName);
	m_nIdx = nIdx;

	m_ProcInfo.SetProcStatus(CProcInfo::PROC_IDLE);

	time_t curtime;
	time(&curtime);
	m_ProcInfo.m_nStartTime = curtime;
	m_ProcInfo.m_nLastTime = curtime;
	
	m_ProcInfo.m_nProcID = getpid();
	m_ProcInfo.m_nTotalCount = 0;
	m_ProcInfo.m_nCounter = 0;
	m_ProcInfo.m_nSpeed = 0;

	if(m_ProcInfoMgr.WriteProcInfo(m_nIdx, m_ProcInfo) < 0)
		READ_WRITE_FAIL(PROCINFO_ERR_WRITE_FAIL);

	return m_nIdx;
}

/***********************************************************************
*��������:		���½��̼����Ϣ״̬�ʹ����ļ���
*�������:		status:				���̵�״̬
*				pchDealFileName:	�����ļ���
*�������:		None
*����ֵ:		None
***********************************************************************/
void CProcessMonitor::UpdateMonitor(CProcInfo::ProcStatus status, char *pchDealFileName)
{
	if(m_bReAttach)
	{
		if(!m_ProcInfoMgr.AttachShareMem(m_bReAttach))
			throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG, 
				PROCINFO_ERR_ATTACH_FAIL, errno, 
				MSG_ATTACH_FAIL, __FILE__, __LINE__);
	}

	time_t curtime;
	time(&curtime);
	m_ProcInfo.m_nLastTime = curtime;

	m_ProcInfo.SetProcStatus(status);

	if(status != CProcInfo::PROC_BUSY)
	{
		m_ProcInfo.m_nCounter = 0;
		m_ProcInfo.m_nSpeed = 0;
	}

	if(pchDealFileName != NULL)
		strcpy(m_ProcInfo.m_pchDealFileName, pchDealFileName);

	if(m_ProcInfoMgr.WriteProcInfo(m_nIdx, m_ProcInfo) < 0)
		READ_WRITE_FAIL(PROCINFO_ERR_WRITE_FAIL);

}

/***********************************************************************
*��������:		�趨���̼����Ϣ������
*�������:		nIdx:				������
*�������:		None
*����ֵ:		true:				�趨�ɹ�
*				false:				�趨ʧ��
***********************************************************************/
bool CProcessMonitor::SetProcInfoIndex(int nIdx)
{
	if(nIdx < 0 || nIdx > MAX_PROC_COUNT)
		return false;

	m_nIdx = nIdx;
	return true;
}

/***********************************************************************
*��������:		�ӹ����ڴ��ж�ȡ���̵ļ����Ϣ
*�������:		None
*�������:		None
*����ֵ:		None
***********************************************************************/
void CProcessMonitor::ReadProcInfo()
{
	if(m_bReAttach)
	{
		if(!m_ProcInfoMgr.AttachShareMem(m_bReAttach))
			throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG, 
				PROCINFO_ERR_ATTACH_FAIL, errno, 
				MSG_ATTACH_FAIL, __FILE__, __LINE__);
	}
	
	if(m_ProcInfoMgr.ReadProcInfo(m_nIdx, m_ProcInfo) < 0)
		READ_WRITE_FAIL(PROCINFO_ERR_READ_FAIL);
}

CProcessMonitor& CProcessMonitor::operator ++()
{
	long nCurTime = 0;
	time_t curtime;
	time(&curtime);
	nCurTime = curtime;

	m_ProcInfo.m_nTotalCount++;
	m_ProcInfo.m_nCounter++;
	
	//���㴦���ٶ�
	if(nCurTime != m_ProcInfo.m_nLastTime)
	{
		if(m_bReAttach)
		{
			if(!m_ProcInfoMgr.AttachShareMem(m_bReAttach))
				throw CF_CError(ERR_TYPE_ELSE, ERR_LEVEL_HIG, 
				PROCINFO_ERR_ATTACH_FAIL, errno, 
				MSG_ATTACH_FAIL, __FILE__, __LINE__);
		}

		m_ProcInfo.m_nSpeed = m_ProcInfo.m_nCounter/(nCurTime - m_ProcInfo.m_nLastTime);
		m_ProcInfo.m_nLastTime = nCurTime;
		m_ProcInfo.m_nCounter = 0;

		if(m_ProcInfoMgr.WriteProcInfo(m_nIdx, m_ProcInfo) < 0)
			READ_WRITE_FAIL(PROCINFO_ERR_WRITE_FAIL);
	}

	return *this;
}

CProcessMonitor& CProcessMonitor::operator ++(int)
{
	++(*this);
	return *this;
}
