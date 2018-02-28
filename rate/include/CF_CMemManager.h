/*******************************************************************
*CF_CMemManager.h
*集中申请管理大块内存供批价规则使用，以避免各自申请内存造成的内存碎片
*created by tanj 2005.05.19
*******************************************************************/
#ifndef _CF_CMEMMANAGER_H_
#define _CF_CMEMMANAGER_H_

#include <vector>
#include <errno.h>
#include <iostream.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h> 

#include "CF_CLogger.h"
#include "CF_CException.h" 
#include "CF_Common.h"
#include "rate_error_define.h"
#include "RuleStruct.h"

#include "C_CFileLock.h"
#include "psutil.h"
#include "es/util/StringUtil.h"

const int SHARE_MEM_INDEX = 1;
const int EXACT_RULE_VAR_INDEX = 2;
const int EXACT_RULE_VALUE_INDEX = 3;
const int MAX_PROC_NUM = 100;




//共享内存中进程信息结构
struct S_Process
{
	pid_t iProcessID;
	int iSleepFlag;  /*0表示活动
			   1表示该进程休眠	*/
	int isLoadFlag;  /*0表示不需要更新
			   1表示需要更新	*/
	S_Process()
	{
		iProcessID = 0;
		iSleepFlag = 0;
		isLoadFlag = 0;
	}		   
};

struct ShmStruct
{
	int 	iRuleCount;//当前存储的规则条数
	int		iMemLock;//第二块共享内存锁，当置1时，标识有进程需要更新
	int		iProcNum;//当前有多少进程在使用共享内存
	int		iCurSize;//实际共享内存大小为：文件大小+iCurSize*50M；
	int   iMaxRule;//目前所运行的最大规则数
	int		MemVersion;
	int iTotalProcess;
	
	S_Process m_Process[MAX_PROC_NUM];
	
	ShmStruct()
	{
		iRuleCount = 0;
		iMemLock = 0;
		iProcNum = 0;
		iCurSize = 0;
		MemVersion = 0;
		iMaxRule = 0;
	}
};



class MemManager
{
	public:

		int Init(const char *szComFile);//通过精确规则文件来初始化共享内存
		void Rebuild(int block);//空间不够,增加block个50M;
		int IsUseMemory();
		void Release(); 

		MemManager();
		~MemManager();

	public:
		//精确规则的文件名(全路径)
		char m_szExactFile[500];
		char szMsg[LOG_MSG_LEN+1];		
		ShmStruct* pShm;
		SRuleVar* RealVarShm;//第二块规则变量共享内存的起始地址
		SRuleValue* RealValueShm;//第三块规则取值共享内存的起始地址
		SRuleVar*	ShmVarPos; //第二块规则变量共享内存的偏移地址
		SRuleValue*	ShmValuePos; //第三块规则取值共享内存的偏移地址
		
				
		int ishmid;
		C_CFileLock filelock;
};






#endif 

