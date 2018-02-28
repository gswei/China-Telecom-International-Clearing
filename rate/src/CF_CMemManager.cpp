/*******************************************************************
*MemManager.cpp
*集中申请管理大块内存供批价规则使用，以避免各自申请内存造成的内存碎片
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
	//加全局锁
	theJSLog<<"Lock file"<<endi;
	filelock.Lock();
	theJSLog<<"Lock success"<<endi;
	
	
	key_t ikey = ftok(m_szExactFile, SHARE_MEM_INDEX);

	//核心参数key 获取
	/*std::string commem_keyvalue;
	if( !tpss::getKenelParam( "business.gjrate.sharememvalue", commem_keyvalue ) ) {
		tpss::writelog( 0, "获取共享内存固定key 值" );
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
		// 存在
		theJSLog << "Connecting head shm..  " <<endi;
		ishmid = shmget(ikey, 0, IPC_CREAT|0666);
		pShm = (ShmStruct*)shmat(ishmid,NULL,0);
		
		//连接文件头
		theJSLog << "connecting rule var.. " <<endi;
		ikey = ftok(m_szExactFile, EXACT_RULE_VAR_INDEX); 

			//核心参数key 获取
	    //std::string commem_keyvalue;
	   /* if( !tpss::getKenelParam( "business.gjrate.rulevarvalue", commem_keyvalue ) ) {
		   tpss::writelog( 0, "获取共享内存固定key 值" );
		   //commem_keyvalue = "91670051";
	    }
	    commenKeyValue = es::StringUtil::toInt( commem_keyvalue );	
	    ikey=commenKeyValue;*/
	    //////// key
	    //cout << "business.gjrate.rulevarvalue = " <<ikey<<endl;
		ishmid = shmget(ikey, 0, IPC_CREAT|0666);
		RealVarShm= (SRuleVar*)shmat(ishmid, NULL, 0);
		ShmVarPos = RealVarShm+pShm->iRuleCount;
		//连接数据块
		theJSLog << "connecting rule value.. " <<endi;
		ikey = ftok(m_szExactFile, EXACT_RULE_VALUE_INDEX); 

		//核心参数key 获取
	    //std::string commem_keyvalue;
	   /* if( !tpss::getKenelParam( "business.gjrate.rulevaluevalue", commem_keyvalue ) ) {
		    tpss::writelog( 0, "获取共享内存固定key 值" );
		    //commem_keyvalue = "91670052";
	    }
	     commenKeyValue = es::StringUtil::toInt( commem_keyvalue );	
	     ikey=commenKeyValue;*/
	      //////// key
	
		ishmid = shmget(ikey, 0, IPC_CREAT|0666);
		RealValueShm = (SRuleValue*)shmat(ishmid, NULL, 0);
				
		ShmValuePos = RealValueShm+pShm->iRuleCount;
		
		//登记进程信息
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
		// 不存在全部创建并赋初始值
		theJSLog << "creating head shm..  " <<endi;
		pShm = (ShmStruct*)shmat(ishmid,NULL,0);
		
		//连接精确规则变量区域
		DEBUG_LOG << "creating rule shm..  " <<endd;
		ikey = ftok(m_szExactFile, EXACT_RULE_VAR_INDEX);

		/* if( !tpss::getKenelParam( "business.gjrate.rulevarvalue", commem_keyvalue ) ) {
		    tpss::writelog( 0, "获取共享内存固定key 值" );
		    //commem_keyvalue = "91670052";
	    }
	     commenKeyValue = es::StringUtil::toInt( commem_keyvalue );	
	     ikey=commenKeyValue;*/
		
		DEBUG_LOG<<"*****"<<EXACT_RULE_VAR_INDEX<<"******"<<"key_t******"<<ikey<<endd;  
		ishmid = shmget(ikey, filelock.getFileSize() + 75*1024*1024, IPC_CREAT|0666);//初始大小为文件大小+75M
		RealVarShm = (SRuleVar*)shmat(ishmid, NULL, 0);
		ShmVarPos = RealVarShm;
		pShm->iCurSize = filelock.getFileSize() + 75*1024*1024;		
		pShm->iMaxRule=pShm->iCurSize/sizeof(SRuleVar);
		
		//连接精确规则变量区域
		theJSLog << "creating rule shm..  " <<endi;
		ikey = ftok(m_szExactFile, EXACT_RULE_VALUE_INDEX);

		 /*if( !tpss::getKenelParam( "business.gjrate.rulevaluevalue", commem_keyvalue ) ) {
		    tpss::writelog( 0, "获取共享内存固定key 值" );
		    //commem_keyvalue = "91670052";
	    }
	     commenKeyValue = es::StringUtil::toInt( commem_keyvalue );	
	     ikey=commenKeyValue;*/
	     
		DEBUG_LOG<<"*****"<<EXACT_RULE_VALUE_INDEX<<"******"<<"key_t******"<<ikey<<endd;   
		ishmid = shmget(ikey, filelock.getFileSize() + 75*1024*1024, IPC_CREAT|0666);//初始大小为文件大小+75M
		RealValueShm = (SRuleValue*)shmat(ishmid, NULL, 0);
		ShmValuePos = RealValueShm;
		
		//登记进程信息
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

	//核心参数key 获取
	/*std::string commem_keyvalue;
	if( !tpss::getKenelParam( "business.gjrate.rulevarvalue", commem_keyvalue ) ) {
		tpss::writelog( 0, "获取共享内存固定key 值" );
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
		//连接精确规则区域
		ikey = ftok(m_szExactFile, EXACT_RULE_VAR_INDEX); 

		/*if( !tpss::getKenelParam( "business.gjrate.rulevarvalue", commem_keyvalue ) ) {
		    tpss::writelog( 0, "获取共享内存固定key 值" );
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
		     tpss::writelog( 0, "获取共享内存固定key 值" );
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
		     tpss::writelog( 0, "获取共享内存固定key 值" );
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
		//连接精确规则区域
		ikey = ftok(m_szExactFile, EXACT_RULE_VALUE_INDEX); 

		/*if( !tpss::getKenelParam( "business.gjrate.rulevaluevalue", commem_keyvalue ) ) {
		     tpss::writelog( 0, "获取共享内存固定key 值" );
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
		     tpss::writelog( 0, "获取共享内存固定key 值" );
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
			//如果该进程不存在了
				//theJSLog<<pShm->m_Process[i].iProcessID<<" process is doing!"<<endi;
				//查看该进程是否异常退出,如果是异常退出,则清空该进程在共享内存中的信息
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
