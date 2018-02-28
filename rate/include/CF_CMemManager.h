/*******************************************************************
*CF_CMemManager.h
*��������������ڴ湩���۹���ʹ�ã��Ա�����������ڴ���ɵ��ڴ���Ƭ
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




//�����ڴ��н�����Ϣ�ṹ
struct S_Process
{
	pid_t iProcessID;
	int iSleepFlag;  /*0��ʾ�
			   1��ʾ�ý�������	*/
	int isLoadFlag;  /*0��ʾ����Ҫ����
			   1��ʾ��Ҫ����	*/
	S_Process()
	{
		iProcessID = 0;
		iSleepFlag = 0;
		isLoadFlag = 0;
	}		   
};

struct ShmStruct
{
	int 	iRuleCount;//��ǰ�洢�Ĺ�������
	int		iMemLock;//�ڶ��鹲���ڴ���������1ʱ����ʶ�н�����Ҫ����
	int		iProcNum;//��ǰ�ж��ٽ�����ʹ�ù����ڴ�
	int		iCurSize;//ʵ�ʹ����ڴ��СΪ���ļ���С+iCurSize*50M��
	int   iMaxRule;//Ŀǰ�����е���������
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

		int Init(const char *szComFile);//ͨ����ȷ�����ļ�����ʼ�������ڴ�
		void Rebuild(int block);//�ռ䲻��,����block��50M;
		int IsUseMemory();
		void Release(); 

		MemManager();
		~MemManager();

	public:
		//��ȷ������ļ���(ȫ·��)
		char m_szExactFile[500];
		char szMsg[LOG_MSG_LEN+1];		
		ShmStruct* pShm;
		SRuleVar* RealVarShm;//�ڶ��������������ڴ����ʼ��ַ
		SRuleValue* RealValueShm;//���������ȡֵ�����ڴ����ʼ��ַ
		SRuleVar*	ShmVarPos; //�ڶ��������������ڴ��ƫ�Ƶ�ַ
		SRuleValue*	ShmValuePos; //���������ȡֵ�����ڴ��ƫ�Ƶ�ַ
		
				
		int ishmid;
		C_CFileLock filelock;
};






#endif 

