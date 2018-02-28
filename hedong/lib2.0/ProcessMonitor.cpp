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

//判断左右两边是否是同一进程信息
bool CProcInfo::operator ==(const CProcInfo &right) const
{
	if(strcmp(m_pchPipeID, right.m_pchPipeID) == 0 
		&& m_nProcessID == right.m_nProcessID)
		return true;
	else
		return false;
}

/***********************************************************************
*函数介绍:		将进程信息复位
*输入参数:		None
*输出参数:		None
*返回值:		None
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
*函数介绍:		创建进程监控信息共享内存
*输入参数:		None
*输出参数:		None
*返回值:		true:		成功创建
*				false:		创建失败
***********************************************************************/
bool CProcInfoManager::CreateShareMem(void)
{
	m_nShmID = shmget(m_shmKey, sizeof(CProcInfo)*MAX_PROC_COUNT, 0666|IPC_CREAT|IPC_EXCL);
	if(m_nShmID <= 0)
	{
		if(errno == EEXIST)//共享内存已经存在
		{
			return AttachShareMem();
		}
		else
			return false;
	}
	
	m_pProcInfo = (CProcInfo *)shmat(m_nShmID, 0, 0);

	/*连接到共享内存*/
	/*m_pProcInfo=-1表示shmat失败*/
	if (m_pProcInfo == (CProcInfo *)-1)
		return false;

	ResetShareMem();

	return true;	
}

/***********************************************************************
*函数介绍:		删除进程监控信息共享内存
*输入参数:		bSafeDel:	安全删除标识, true: 检测还有没有进程连接到共享内存,如有, 删除失败
*输出参数:		None
*返回值:		true:		成功删除
*				false:		删除失败
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
*函数介绍:		获取共享内存地址
*输入参数:		None
*输出参数:		None
*返回值:		共享内存地址, 失败返回NULL
***********************************************************************/
CProcInfo* CProcInfoManager::GetShareMem(void)
{
	return m_pProcInfo;
}

/***********************************************************************
*函数介绍:		将进程监控信息共享内存复位
*输入参数:		None
*输出参数:		None
*返回值:		None
***********************************************************************/
void CProcInfoManager::ResetShareMem(void)
{
	for(int i = 0; i < MAX_PROC_COUNT; i++)
		(m_pProcInfo + i)->Reset();
}

/***********************************************************************
*函数介绍:		连接进程监控信息共享内存
*输入参数:		bReAttach:	强制重连标识, true表示需强制重连, false表示不需重连
*输出参数:		None
*返回值:		true:		连接成功
*				false:		连接失败
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
		
		if (m_nShmID <= 0)//共享内存不存在
			return false;
	}
	
	if (!m_pProcInfo)
	{
		m_pProcInfo = (CProcInfo *)shmat(m_nShmID, 0, 0);
		
		/*连接到共享内存*/
		/*m_pProcInfo=-1表示shmat失败*/
		if (m_pProcInfo == (CProcInfo *)-1)
			return false;
	}
	
	return true;	
}

/***********************************************************************
*函数介绍:		断开和进程监控信息共享内存的连接
*输入参数:		None
*输出参数:		None
*返回值:		true:		断开连接成功
*				false:		断开连接失败
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
*函数介绍:		获取指定进程信息在共享内存中的索引
*输入参数:		procInfo:	指定的进程信息
*输出参数:		None
*返回值:		i(>=0):		获取成功返回索引
*				-1:			未连接到共享内存
*				-2:			未找到索引
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
*函数介绍:		将指定进程的监控信息写入共享内存
*输入参数:		nIdx:		指定的进程信息在共享内存中的索引
*				procInfo:	指定的进程信息
*输出参数:		None
*返回值:		i(>=0):		写入成功, 返回索引
*				-1:			未连接到共享内存
*				-2:			错误的索引号
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
*函数介绍:		从共享内存中读取进程的监控信息
*输入参数:		nIdx:		指定的的索引
*输出参数:		procInfo:	进程信息
*返回值:		i(>=0):		读取成功, 返回索引
*				-1:			未连接到共享内存
*				-2:			错误的索引号
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
*函数介绍:		初始化进程监控信息
*输入参数:		pchPipeID:		进程的PIPE_ID
*				nProcessID:		处理过程序列号
*				pchProcName:	进程的名字
*				nIdx:			进程信息在内存中的索引
*输出参数:		None
*返回值:		nIdx(>=0):		初始化成功, 返回索引
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

	//同一PIPE下的同一程序不能同时启动多个
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
*函数介绍:		更新进程监控信息状态和处理文件名
*输入参数:		status:				进程的状态
*				pchDealFileName:	处理文件名
*输出参数:		None
*返回值:		None
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
*函数介绍:		设定进程监控信息的索引
*输入参数:		nIdx:				索引号
*输出参数:		None
*返回值:		true:				设定成功
*				false:				设定失败
***********************************************************************/
bool CProcessMonitor::SetProcInfoIndex(int nIdx)
{
	if(nIdx < 0 || nIdx > MAX_PROC_COUNT)
		return false;

	m_nIdx = nIdx;
	return true;
}

/***********************************************************************
*函数介绍:		从共享内存中读取进程的监控信息
*输入参数:		None
*输出参数:		None
*返回值:		None
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
	
	//计算处理速度
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
