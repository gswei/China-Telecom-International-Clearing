/*******************************************************************
*MemManager.cpp
*��������������ڴ湩���۹���ʹ�ã��Ա�����������ڴ���ɵ��ڴ���Ƭ
*created by tanj 2005.05.19
*******************************************************************/
#include "CF_CMemManager.h"

MemManager::MemManager()
{
	pShm = NULL;
}              

MemManager::~MemManager()
{

}

void MemManager::Release()
{
	shmdt(pShm);
	pShm = NULL;
	
	shmdt(RealVarShm);
	RealVarShm = NULL;
	
	shmdt(RealValueShm);
	RealValueShm = NULL;
	return;
}     
               
int MemManager::Init(const char *szComFile)
{
	strcpy(m_szExactFile, szComFile);
	//strcpy(m_szProcessId, "789");
	filelock.Init(m_szExactFile);
	//��ȫ����
	theJSLog<<"Lock file"<<endi;
	filelock.Lock();
	theJSLog<<"Lock success"<<endi;
	
	
	key_t ikey = ftok(m_szExactFile, SHARE_MEM_INDEX);

	//���Ĳ���key ��ȡ
	/*std::string commem_keyvalue;
	if( !tpss::getKenelParam( "business.gjrate.sharememvalue", commem_keyvalue ) ) {
		tpss::writelog( 0, "��ȡ�����ڴ�̶�key ֵ" );
		commem_keyvalue = "91670050";
	}
	int commenKeyValue = es::StringUtil::toInt( commem_keyvalue );	
	ikey=commenKeyValue;*/
	//////// key
	
	DEBUG_LOG<<"*****"<<SHARE_MEM_INDEX<<"******"<<"key_t******"<<ikey<<endd; 
	if(ikey == -1)
	{
		perror("ftok error");

		sprintf(szMsg, "get key_t error ! index=[%d], path=[%s]",SHARE_MEM_INDEX, m_szExactFile);
		throw jsexcp::CException(RATE_ERR_CREATE_MEMORY, szMsg, __FILE__, __LINE__);
		
	}
	errno = 0;
	ishmid = shmget(ikey, sizeof(ShmStruct), IPC_CREAT|IPC_EXCL|0666);
	if(errno == EEXIST)
	{
		// ����
		theJSLog << "Connecting head shm..  " <<endi;
		ishmid = shmget(ikey, 0, IPC_CREAT|0666);
		pShm = (ShmStruct*)shmat(ishmid,NULL,0);
		
		//�����ļ�ͷ
		theJSLog << "connecting rule var.. " <<endi;
		ikey = ftok(m_szExactFile, EXACT_RULE_VAR_INDEX); 

			//���Ĳ���key ��ȡ
	    //std::string commem_keyvalue;
	   /* if( !tpss::getKenelParam( "business.gjrate.rulevarvalue", commem_keyvalue ) ) {
		   tpss::writelog( 0, "��ȡ�����ڴ�̶�key ֵ" );
		   //commem_keyvalue = "91670051";
	    }
	    commenKeyValue = es::StringUtil::toInt( commem_keyvalue );	
	    ikey=commenKeyValue;*/
	    //////// key
	    //cout << "business.gjrate.rulevarvalue = " <<ikey<<endl;
		ishmid = shmget(ikey, 0, IPC_CREAT|0666);
		RealVarShm= (SRuleVar*)shmat(ishmid, NULL, 0);
		ShmVarPos = RealVarShm+pShm->iRuleCount;
		//�������ݿ�
		theJSLog << "connecting rule value.. " <<endi;
		ikey = ftok(m_szExactFile, EXACT_RULE_VALUE_INDEX); 

		//���Ĳ���key ��ȡ
	    //std::string commem_keyvalue;
	   /* if( !tpss::getKenelParam( "business.gjrate.rulevaluevalue", commem_keyvalue ) ) {
		    tpss::writelog( 0, "��ȡ�����ڴ�̶�key ֵ" );
		    //commem_keyvalue = "91670052";
	    }
	     commenKeyValue = es::StringUtil::toInt( commem_keyvalue );	
	     ikey=commenKeyValue;*/
	      //////// key
	
		ishmid = shmget(ikey, 0, IPC_CREAT|0666);
		RealValueShm = (SRuleValue*)shmat(ishmid, NULL, 0);
				
		ShmValuePos = RealValueShm+pShm->iRuleCount;
		
		//�Ǽǽ�����Ϣ
		int MemIndex = 0;
		for(int i = 0; i<MAX_PROC_NUM;i++)
		{
			if(-1 == kill(pShm->m_Process[i].iProcessID,0))
				{
					pShm->m_Process[i].iProcessID = getpid();
					pShm->m_Process[i].isLoadFlag = 0;
					MemIndex = i;
					break;
				}
		}
				
		theJSLog<<"unLock file"<<endi;
		filelock.UnLock();
		theJSLog<<"unLock success"<<endi;		
		if(MemIndex < MAX_PROC_NUM)		
		return MemIndex;
		else
			{
				sprintf(szMsg, "shmget error ! index=[%d], path=[%s]",SHARE_MEM_INDEX, m_szExactFile);	
		 jsexcp::CException e(RATE_ERR_CREATE_MEMORY, szMsg, __FILE__, __LINE__);
		 throw e;				
			} 
	}
	else if(errno == 0)
	{
		// ������ȫ������������ʼֵ
		theJSLog << "creating head shm..  " <<endi;
		pShm = (ShmStruct*)shmat(ishmid,NULL,0);
		
		//���Ӿ�ȷ�����������
		DEBUG_LOG << "creating rule shm..  " <<endd;
		ikey = ftok(m_szExactFile, EXACT_RULE_VAR_INDEX);

		/* if( !tpss::getKenelParam( "business.gjrate.rulevarvalue", commem_keyvalue ) ) {
		    tpss::writelog( 0, "��ȡ�����ڴ�̶�key ֵ" );
		    //commem_keyvalue = "91670052";
	    }
	     commenKeyValue = es::StringUtil::toInt( commem_keyvalue );	
	     ikey=commenKeyValue;*/
		
		DEBUG_LOG<<"*****"<<EXACT_RULE_VAR_INDEX<<"******"<<"key_t******"<<ikey<<endd;  
		ishmid = shmget(ikey, filelock.getFileSize() + 75*1024*1024, IPC_CREAT|0666);//��ʼ��СΪ�ļ���С+75M
		RealVarShm = (SRuleVar*)shmat(ishmid, NULL, 0);
		ShmVarPos = RealVarShm;
		pShm->iCurSize = filelock.getFileSize() + 75*1024*1024;		
		pShm->iMaxRule=pShm->iCurSize/sizeof(SRuleVar);
		
		//���Ӿ�ȷ�����������
		theJSLog << "creating rule shm..  " <<endi;
		ikey = ftok(m_szExactFile, EXACT_RULE_VALUE_INDEX);

		 /*if( !tpss::getKenelParam( "business.gjrate.rulevaluevalue", commem_keyvalue ) ) {
		    tpss::writelog( 0, "��ȡ�����ڴ�̶�key ֵ" );
		    //commem_keyvalue = "91670052";
	    }
	     commenKeyValue = es::StringUtil::toInt( commem_keyvalue );	
	     ikey=commenKeyValue;*/
	     
		DEBUG_LOG<<"*****"<<EXACT_RULE_VALUE_INDEX<<"******"<<"key_t******"<<ikey<<endd;   
		ishmid = shmget(ikey, filelock.getFileSize() + 75*1024*1024, IPC_CREAT|0666);//��ʼ��СΪ�ļ���С+75M
		RealValueShm = (SRuleValue*)shmat(ishmid, NULL, 0);
		ShmValuePos = RealValueShm;
		
		//�Ǽǽ�����Ϣ
		int MemIndex = 0;
		for(int i = 0; i<MAX_PROC_NUM;i++)
		{
			if(-1 == kill(pShm->m_Process[i].iProcessID,0))
				{
					pShm->m_Process[i].iProcessID = getpid();
					pShm->m_Process[i].isLoadFlag = 0;
					MemIndex = i;
					break;
				}
		}
				
		theJSLog<<"unLock file"<<endi;
		filelock.UnLock();
		theJSLog<<"unLock success"<<endi;
		if(MemIndex < MAX_PROC_NUM)		
		return MemIndex;
		else
			{
				sprintf(szMsg, "shmget error ! index=[%d], path=[%s]",SHARE_MEM_INDEX, m_szExactFile);	
		 jsexcp::CException e(RATE_ERR_CREATE_MEMORY, szMsg, __FILE__, __LINE__);
		 throw e;				
			}
	}
	else if(errno != 0)
	{
		perror("shmget error");
		sprintf(szMsg, "shmget error ! index=[%d], path=[%s]",SHARE_MEM_INDEX, m_szExactFile);
		theJSLog<<"unLock file"<<endi;
		filelock.UnLock();
		theJSLog<<"unLock success"<<endi;		
		 jsexcp::CException e(RATE_ERR_CREATE_MEMORY, szMsg, __FILE__, __LINE__);
		 throw e;
	}
}


void MemManager::Rebuild(int block)
{
	if (block<0) return;
  DEBUG_LOG<<"begin to apply new memory:"<< block <<"*50M"<<endd;
	key_t ikey =ftok(m_szExactFile, EXACT_RULE_VAR_INDEX); 

	//���Ĳ���key ��ȡ
	/*std::string commem_keyvalue;
	if( !tpss::getKenelParam( "business.gjrate.rulevarvalue", commem_keyvalue ) ) {
		tpss::writelog( 0, "��ȡ�����ڴ�̶�key ֵ" );
		//commem_keyvalue = "91670051";
	}
	int commenKeyValue = es::StringUtil::toInt( commem_keyvalue );	
	ikey=commenKeyValue;*/
	//////// key

	if(ikey == -1)
	{
		perror("ftok error");

		sprintf(szMsg, "get key_t error ! index=[%d], path=[%s]",EXACT_RULE_VAR_INDEX, m_szExactFile);
		jsexcp::CException e(RATE_ERR_CREATE_MEMORY, szMsg, __FILE__, __LINE__);
		throw e;		
	}
	errno = 0;
	
	ishmid = shmget(ikey, 0, IPC_CREAT|0666);
	shmctl(ishmid,IPC_RMID,0);
		
	ishmid = shmget(ikey, pShm->iCurSize+block*50*1024*1024, IPC_CREAT|IPC_EXCL|0666);
	if(errno == EEXIST)
	{
		//���Ӿ�ȷ��������
		ikey = ftok(m_szExactFile, EXACT_RULE_VAR_INDEX); 

		/*if( !tpss::getKenelParam( "business.gjrate.rulevarvalue", commem_keyvalue ) ) {
		    tpss::writelog( 0, "��ȡ�����ڴ�̶�key ֵ" );
		    //commem_keyvalue = "91670051";
	    }
	    commenKeyValue = es::StringUtil::toInt( commem_keyvalue );	
	    ikey=commenKeyValue;*/
	
		ishmid = shmget(ikey, 0, IPC_CREAT|0666);		
		RealVarShm = (SRuleVar*)shmat(ishmid, NULL, 0);
		ShmVarPos = RealVarShm;
		pShm->iRuleCount = 0;
		pShm->iCurSize = pShm->iCurSize+block*50*1024*1024;
		pShm->iMaxRule=pShm->iCurSize/sizeof(SRuleVar);
	}
	else if(errno != 0)
	{
		perror("shmget error");
		sprintf(szMsg, "shmget error ! index=[%d], path=[%s]",EXACT_RULE_VAR_INDEX, m_szExactFile);
		 jsexcp::CException e(RATE_ERR_CREATE_MEMORY, szMsg, __FILE__, __LINE__);
		 throw e;
	}
	else
	{
		ikey = ftok(m_szExactFile, EXACT_RULE_VAR_INDEX); 

		/*if( !tpss::getKenelParam( "business.gjrate.rulevarvalue", commem_keyvalue ) ) {
		     tpss::writelog( 0, "��ȡ�����ڴ�̶�key ֵ" );
		     //commem_keyvalue = "91670051";
	    }
	    commenKeyValue = es::StringUtil::toInt( commem_keyvalue );	
	    ikey=commenKeyValue;*/
	
		ishmid = shmget(ikey, 0, IPC_CREAT|0666);		
		RealVarShm = (SRuleVar*)shmat(ishmid, NULL, 0);
		ShmVarPos = RealVarShm;
		pShm->iRuleCount = 0;
		pShm->iCurSize = pShm->iCurSize+block*50*1024*1024;
		pShm->iMaxRule=pShm->iCurSize/sizeof(SRuleVar);
		
	}
	
	ikey = ftok(m_szExactFile, EXACT_RULE_VALUE_INDEX); 

	/*if( !tpss::getKenelParam( "business.gjrate.rulevaluevalue", commem_keyvalue ) ) {
		     tpss::writelog( 0, "��ȡ�����ڴ�̶�key ֵ" );
		     //commem_keyvalue = "91670052";
	    }
	    commenKeyValue = es::StringUtil::toInt( commem_keyvalue );	
	    ikey=commenKeyValue;*/
	    
	if(ikey == -1)
	{
		perror("ftok error");

		sprintf(szMsg, "get key_t error ! index=[%d], path=[%s]",EXACT_RULE_VALUE_INDEX, m_szExactFile);
		jsexcp::CException e(RATE_ERR_CREATE_MEMORY, szMsg, __FILE__, __LINE__);
		throw e;		
	}
	errno = 0;
	
	ishmid = shmget(ikey, 0, IPC_CREAT|0666);		
	shmctl(ishmid,IPC_RMID,0);
	ishmid = shmget(ikey, pShm->iCurSize+block*50*1024*1024, IPC_CREAT|IPC_EXCL|0666);
	if(errno == EEXIST)
	{
		//���Ӿ�ȷ��������
		ikey = ftok(m_szExactFile, EXACT_RULE_VALUE_INDEX); 

		/*if( !tpss::getKenelParam( "business.gjrate.rulevaluevalue", commem_keyvalue ) ) {
		     tpss::writelog( 0, "��ȡ�����ڴ�̶�key ֵ" );
		     //commem_keyvalue = "91670052";
	    }
	    commenKeyValue = es::StringUtil::toInt( commem_keyvalue );	
	    ikey=commenKeyValue;*/
	    
		ishmid = shmget(ikey, 0, IPC_CREAT|0666);						
		RealValueShm = (SRuleValue*)shmat(ishmid, NULL, 0);
		ShmValuePos = RealValueShm;
	}
	else if(errno != 0)
	{
		perror("shmget error");
		sprintf(szMsg, "shmget error ! index=[%d], path=[%s]",EXACT_RULE_VALUE_INDEX, m_szExactFile);
		 jsexcp::CException e(RATE_ERR_CREATE_MEMORY, szMsg, __FILE__, __LINE__);
		 throw e;
	}
	else
	{
		ikey = ftok(m_szExactFile, EXACT_RULE_VALUE_INDEX); 

		/*if( !tpss::getKenelParam( "business.gjrate.rulevaluevalue", commem_keyvalue ) ) {
		     tpss::writelog( 0, "��ȡ�����ڴ�̶�key ֵ" );
		     //commem_keyvalue = "91670052";
	    }
	    commenKeyValue = es::StringUtil::toInt( commem_keyvalue );	
	    ikey=commenKeyValue;*/
	    
		ishmid = shmget(ikey, 0, IPC_CREAT|0666);						
		RealValueShm = (SRuleValue*)shmat(ishmid, NULL, 0);
		ShmValuePos = RealValueShm;
	}
	
	pShm->MemVersion++;
		

}

int MemManager::IsUseMemory()
{

	int iTotalProcess=0;
	for(int i=0; i<MAX_PROC_NUM; i++)
	{
		if ( pShm->m_Process[i].iProcessID != 0 && pShm->m_Process[i].iSleepFlag != 0)
		{
			//����ý��̲�������
				//theJSLog<<pShm->m_Process[i].iProcessID<<" process is doing!"<<endi;
				//�鿴�ý����Ƿ��쳣�˳�,������쳣�˳�,����ոý����ڹ����ڴ��е���Ϣ
				if (-1 == kill(pShm->m_Process[i].iProcessID, 0))
				{
					pShm->m_Process[i].iProcessID=0;
				}
				else  iTotalProcess++;				        
		}
	}

	if ( iTotalProcess != 0)
		{
			return 1;
		}
		else 
			return 0;
}
