// ProcessMonitor.h: interface for the CProcessMonitor class.
//
//////////////////////////////////////////////////////////////////////
/********************************************************************
 *	Usage		: 进程监控共享内存
 *				: CProcInfo--进程信息类
 *				: CProcInfoManager--共享内存管理类
 *				: CProcessMonitor--进程监控类
 *	Author		: 周亚军
 *  Create date	: 2005-04-28
 *	Version		: 1.0.0
 *	Updatelist	:
 *				  2005-05-18 周亚军 将异常错误等级由中级改为高级; 修改CProcessMonitor的Attach(), 
 *									Detach(), Init()函数, 将原来的错误返回改为抛出异常
 *				  2005-05-30 周亚军 修改CProcessMonitor::Detach()函数, 将断开时的状态改为通过入口
 *									参数指定;
 *				  2006-01-23 周亚军 修改CProcessMonitor的Init(),当进程状态不为EXIT时,检测进程号
 *									对应的进程是否真的存在,如存在抛出异常,避免进程重复启动;否则
 *									不处理,程序可直接运行
 *				  2006-02-15 周亚军 修改CProcessMonitor::Detach(),当未初始化而断开时,判断m_nIdx
 *									是否为-1再更新共享内存状态
 *				  2006-02-16 周亚军 修改CProcInfoManager::DestroyShareMem(),安全删除时,先判断m_pProcInfo
 *									是否为NULL,再去读取共享内存状态
 *				  2006-02-22 周亚军 将原来写死的KEY_ID改为读环境变量的HOME目录值,然后再构建KEY_ID
 *				  2006-03-18 周亚军 增加一个读取KEY_ID的接口
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
	//以下两项唯一确定一进程
	char	m_pchPipeID[5+1];			//流水号
	int		m_nProcessID;				//处理过程序列号

	char	m_pchProcName[30];			//进程名
	long	m_nProcID;					//进程ID, 系统分配的进程号
	long	m_nStartTime;				//进程启动时间
	long	m_nLastTime;				//进程上次活动时间
	char	m_pchStatus[10];			//进程状态
	char	m_pchDealFileName[255+1];	//最后一次处理的文件名
	int		m_nSpeed;					//处理话单的速率(条/秒)
	char	m_pchUsedCpu[10+1];			//进程的cpu使用率
	char	m_pchUsedMem[10+1];			//进程占用的内存(KB)
	long	m_nCounter;					//进程处理计数器
	unsigned long	m_nTotalCount;		//从启动到现在处理的话单数

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
