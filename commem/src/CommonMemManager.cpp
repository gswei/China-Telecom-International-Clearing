/*
 * =====================================================================================
 *
 *       Filename:  CommonMemManager.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2010��04��03�� 08ʱ59��13��
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *        Company:
 *
 * =====================================================================================
 */



#include <assert.h>
#include "CommonMemManager.h"

using namespace std;
using namespace tpss;

CommonMemManager::CommonMemManager()
{
	commonTable=NULL;
	m_ishmidcommon=-1;
	m_icommonkey=-1;
	conn = NULL;
}

CommonMemManager::~CommonMemManager()
{
	
	if ( commonTable != NULL )
	{
		m_RWLock.DestroyLock(commonTable->memlock);
		shmdt((char *)commonTable);
	}
	commonTable=NULL;
	m_ishmidcommon=-1;
	m_icommonkey=-1;
	conn = NULL;
}

void CommonMemManager::init(char* env_path)
{
	char envname[256];
	char temp[256];
	int ret;
	//���Ĳ�����ȡ
	// std::string server_id;
	 if( !tpss::getKenelParam( "business.commem.servid", serv_id ) ) {
		    tpss::writelog( 0, "��ȡ������ID SERVID" );
		    serv_id = "SERV1";
	   }
	 //add by sunhua at 2010-11-22
	cout << "init manager" <<endl;
	cout << env_path<<endl;
	mem_path=env_path;
	mem_path+="/commem/";
	cout << mem_path<<endl;
	// strcpy(serv_id,log_path.c_str());
    //cout << "init manager" <<endl;
	/*shm_path=env_path;
	shm_path+="zhjs.ini";
	sem_path=env_path;
	sem_path+="service.ini";	
	//add by sunhua at 2010-11-22
	mem_path=env_path;
	mem_path+="commem/";
	theJSLog<<"�����ڴ洴��·��="<<shm_path<<"=,"<<__FILE__<<","<<__LINE__<<endd;
	theJSLog<<"�źŵƴ���·��="<<sem_path<<"=,"<<__FILE__<<","<<__LINE__<<endd;*/

	
	//CReadIni v_ini;
	//v_ini.init(shm_path.c_str());

	//��ȡSERVER_ID��Ϣ
	/*if ( ! v_ini.GetValue("COMMON","SERVER_ID",temp,'Y' ) )
	{
		throw CException(10001,"��ȡ��������������ID[SERVER_ID]����",__FILE__,__LINE__);
	}
	serv_id=temp;*/
	theJSLog<<"������ID="<<serv_id<<"="<<endd;
	
	return ;
}
	
void CommonMemManager::createCommonTable()
{
	char eMessage[512];
	if (commonTable == NULL)
	{
		key_t commonkey;
		errno=0;
		//���㹲����Ϣ����IPC��
		/*string common_path = mem_path;
		common_path +="0";
		cout << "createtable" <<endl;
		theJSLog<<"common_path="<<common_path.c_str()<<"="<<endi;
		char fname[255];
		sprintf(fname,"%s",common_path.c_str());
		FILE *tmpfp;
		if ( ( tmpfp = fopen( fname,"r" ) ) != NULL )
		{
			fclose( tmpfp );
		} else {
			tmpfp = fopen( fname,"w" ) ;
			char buf[255];
			strcpy(buf,"COMMEM\n");
			fputs( buf,tmpfp );
			fclose( tmpfp );
		}*/
		//commonkey=ftok(common_path.c_str(),255);
		//���Ĳ�����ȡ
	    std::string commem_keyvalue;
	    if( !tpss::getKenelParam( "business.commem.commemkeyvalue", commem_keyvalue ) ) {
		    tpss::writelog( 0, "��ȡ�����ڴ�̶�key ֵ" );
		    commem_keyvalue = "91671000";
	      }
	    int commenKeyValue = es::StringUtil::toInt( commem_keyvalue );
	    theJSLog << "commenKeyValue = " << commenKeyValue <<endi;
	
		commonkey = commenKeyValue;
		if (commonkey==-1)
		{
			sprintf(eMessage,"�������������ڴ�������ftok���������쳣,commonkey=%d,errno=%d",commonkey,errno);
			throw CException(10003,eMessage,__FILE__,__LINE__);	
		}
		theJSLog<<"���������ڴ��� commonkey="<<commonkey<<"="<<endi;
		
   	errno = 0;
		
		theJSLog<<"�����ڴ��ռ俪ʼ."<<endi;
		m_ishmidcommon = shmget(commonkey, sizeof(S_MemManager), IPC_CREAT|0666);		
		if (m_ishmidcommon==-1)
	  {
			sprintf(eMessage,"�������������ڴ�������shmget���������쳣,m_ishmidcommon=%d,errno=%d",m_ishmidcommon,errno);
			throw CException(10004,eMessage,__FILE__,__LINE__);	
	 	}
	 	theJSLog<<"�����ڴ��ռ�ɹ������������ڴ��� shmget��ishmidcommon="<<m_ishmidcommon<<"="<<endi;
       		
		errno = 0;
		//���Ӹս��������Ĺ�����Ϣ�����ڴ�,�����׵�ַ
		commonTable = (S_MemManager *)shmat(m_ishmidcommon, NULL, 0);
		if (commonTable==(S_MemManager *)-1)
		{
			if ( m_ishmidcommon != -1 )
				shmctl(m_ishmidcommon,IPC_RMID,NULL);
				
			sprintf(eMessage,"�������������ڴ�������shmat���������쳣��commonTable=%d,errno=%d",commonTable,errno);
			throw CException(10004,eMessage,__FILE__,__LINE__);	
		}
	    cout << "m_ishmidcommon = " <<   m_ishmidcommon <<endl;  
		//��ʼ��������Ϣ��������
		InitCommonMem();
		theJSLog<<"�������������ڴ����ɹ�"<<endi;
	}
	
	return ;
}

void CommonMemManager::AttachCommonTable()
{
	char eMessage[512];
	if (commonTable == NULL)
	{
		key_t commonkey;
		errno=0;
		//���㹲����Ϣ����IPC��
		//string common_path = mem_path;
		//common_path +="0";
		//theJSLog<<"common_path="<<common_path.c_str()<<"="<<endi;
		//commonkey=ftok(common_path.c_str(),255);
		
		//���Ĳ�����ȡ
	    std::string commem_keyvalue;
	    if( !tpss::getKenelParam( "business.commem.commemkeyvalue", commem_keyvalue ) ) {
		   tpss::writelog( 0, "��ȡ�����ڴ�̶�key ֵ" );
		   commem_keyvalue = "91671000";
	     }
	    int commenKeyValue = es::StringUtil::toInt( commem_keyvalue );
	
		commonkey=commenKeyValue;
		if (commonkey==-1)
		{
			sprintf(eMessage,"�������������ڴ�������ftok���������쳣,commonkey=%d,errno=%d",commonkey,errno);
			throw CException(10003,eMessage,__FILE__,__LINE__);	
		}
		char tempStr[255];
		sprintf(tempStr,"%x",commonkey);		
		theJSLog<<"���������ڴ��� commonkey=0x00"<<tempStr<<"="<<endd;
		
   	errno = 0;
		
		m_ishmidcommon = shmget(commonkey, sizeof(S_MemManager), IPC_CREAT|0666);		
		if (m_ishmidcommon==-1)
	  {
			sprintf(eMessage,"�������������ڴ�������shmget���������쳣,m_ishmidcommon=%d,errno=%d",m_ishmidcommon,errno);
			throw CException(10004,eMessage,__FILE__,__LINE__);	
	 	}
	 	theJSLog<<"���������ڴ��� shmget��ishmidcommon="<<m_ishmidcommon<<"="<<endd;
       		
		errno = 0;
		//���Ӹս��������Ĺ�����Ϣ�����ڴ�,�����׵�ַ
		commonTable = (S_MemManager *)shmat(m_ishmidcommon, NULL, 0);
		if (commonTable == (S_MemManager *) -1)
		{
			if ( m_ishmidcommon != -1 )
				shmctl(m_ishmidcommon,IPC_RMID,NULL);
				
			sprintf(eMessage,"�������������ڴ�������commonTable=%d,errno=%d",commonTable,errno);
			throw CException(10004,eMessage,__FILE__,__LINE__);	
		}
		
		theJSLog<<"���ӹ��������ڴ����ɹ�"<<endi;	
		theJSLog<<"commonkey=0x00"<<tempStr<<",m_ishmidcommon="<<m_ishmidcommon<<endi;
		m_RWLock.InitLock(commonTable->memlock,NULL);
	}
}

void CommonMemManager::DetachCommonTable()
{
	char eMessage[512];
	int ret;
	if ( commonTable != NULL ) 
	{
		errno=0;
		ret=shmdt(commonTable);
		if (ret==-1) 
		{
			sprintf(eMessage,"CommonMemManager shmdt���������ڴ���ʱ����errno=%d=",errno);
			throw CException(10004,eMessage,__FILE__,__LINE__);	
		}		
		commonTable=NULL;
	}


}

int CommonMemManager::InitCommonMem()
{
	if ( commonTable->iTotalTable !=0 )
	{
		return 0;
	}
	
	memset( commonTable, 0, sizeof(S_MemManager));
	commonTable->isUsed = 1;
	commonTable->iTotalTable = 0;
	
	m_RWLock.InitLock(commonTable->memlock,NULL);

	return 0;
	
}

void CommonMemManager::deleteCommonTable()
{
	char eMessage[512];
	int ret;
	if ( commonTable != NULL )
	{
		errno =0;
		ret=shmdt((char *)commonTable);
		if ( ret == -1)
		{
			sprintf(eMessage,"ɾ�����������ڴ���shmdt��������commonTable=%d,errno=%d",commonTable,errno);
			throw CException(10004,eMessage,__FILE__,__LINE__);	
		}
		commonTable=NULL;
	}
	
	if ( m_ishmidcommon != -1 )
	{
		errno =0;
		ret=shmctl(m_ishmidcommon,IPC_RMID,NULL);
		if ( ret == -1)
		{
			sprintf(eMessage,"ɾ�����������ڴ���shmctl����IPC_RMID����m_ishmidcommon=%d,errno=%d",m_ishmidcommon,errno);
			throw CException(10004,eMessage,__FILE__,__LINE__);	
		}
		killSingleMem(m_ishmidcommon);
		m_ishmidcommon=-1;
	}
	
	theJSLog<<"ɾ�����������ڴ����ɹ�"<<endi;
	return ;
}			

void CommonMemManager::loadAllTable()
{
	char mem_name[256];
	char eMessage[512];
	int version;
	int shmid_key;
	if(!dbConnect(conn)){
		
		return;
	}
	//CBindSQL ds( DBConn );
	string sqlStr;
	sqlStr="select MEM_NAME,VERSION_ID,SHMKEY_ID from C_COMMEM_CONTROL where valid_flag='Y' and server_id='";
	sqlStr=sqlStr+serv_id+"' ";
	theJSLog<<"sqlStr="<<sqlStr<<__FILE__<<","<<__LINE__<<endi;
	
	Statement stmt = conn.createStatement();
	stmt.setSQLString(sqlStr);
	stmt.execute();
	//ds.Open(sqlStr, SELECT_QUERY);
	//while(ds>>mem_name>>version>>shmid_key)
	while(stmt>>mem_name>>version>>shmid_key)
	{
		try
		{
			int i;
			m_RWLock.GetReadLock(commonTable->memlock);
			for ( i=0;i<commonTable->iTotalTable; i++ )
			{
				
				if ( !strcmp(commonTable->m_TableInfo[i].MemName,mem_name) && commonTable->m_TableInfo[i].version == version )
				{
					//vector<CommonMemTableDefine>::iterator it;
					//for ( it=vtable.begin(); it!= vtable.end(); it ++)
					//if ( it->tableName == mem_name && it->vesion == version )
					//{
						//it->reloadTable( server_id,mem_name,version,shmid_key,i,commonTable,&m_RWLock );
					//}
					//if (it== vtable.end())
					//{
					m_RWLock.UnLock(commonTable->memlock);
					theJSLog<<"�����ڴ�����["<<mem_name<<","<<version<<"]�Ѵ��ڣ����µ���"<<endi;
					//modi by sunhua at 2010-11-22
					CommonMemTableDefine* newtable = new CommonMemTableDefine(serv_id,mem_name,version,mem_path.c_str(),i,commonTable,&m_RWLock );
					vtable.push_back(newtable);
					theJSLog<<"�����ڴ�����["<<mem_name<<","<<version<<"]����ɹ�"<<endi;
					//}
					break;
				}
			}
			if ( i==commonTable->iTotalTable )
			{
				m_RWLock.UnLock(commonTable->memlock);
				theJSLog<<"�����ڴ�����["<<mem_name<<","<<version<<"]�����ڣ��´���"<<endi;
				//modi by sunhua at 2010-11-22
				CommonMemTableDefine* newtable = new CommonMemTableDefine(serv_id,mem_name,version,mem_path.c_str(),i,commonTable,&m_RWLock );
				vtable.push_back(newtable);
				theJSLog<<"�����ڴ�����["<<mem_name<<","<<version<<"]�����ɹ�"<<endi;
			}
		}
		catch ( CException e )
		{
			//modi by ww at 2015-7-8
			//�ͷ��ڴ���
			m_RWLock.UnLock(commonTable->memlock);
			//���±�־λΪ'E'
			string m_name=mem_name;
			sqlStr="update C_COMMEM_CONTROL set status='E' where updatetype=0 and valid_flag='Y' and mem_name='"+m_name+"'and server_id='";
			sqlStr=sqlStr+serv_id+"'";
			theJSLog<<"sqlStr="<<sqlStr<<__FILE__<<","<<__LINE__<<endd;
			stmt.setSQLString(sqlStr);    
  			stmt.execute();
 		 	conn.commit();
 			 conn.close();
			 
			sprintf(eMessage,"���빲���ڴ�����[%s,%d]����",mem_name,version);
			e.PushStack(10005,eMessage,__FILE__, __LINE__);
			throw e;
		}
	}
	//ds.Close();
	sqlStr="update C_COMMEM_CONTROL set status='N' where updatetype=0 and valid_flag='Y' and server_id='";
	sqlStr=sqlStr+serv_id+"'";
	theJSLog<<"sqlStr="<<sqlStr<<__FILE__<<","<<__LINE__<<endd;
	stmt.setSQLString(sqlStr);
	//ds.Open(sqlStr,NONSELECT_DML);       
  stmt.execute();
  conn.commit();
  conn.close();
	//ds.Execute();
	//DBConn.Commit();

	theJSLog<<"All Tables is OK!"<<endi;
	
	return ;
}

void CommonMemManager::freeAllTable()
{
	char eMessage[512];
	int ret;
	key_t commonkey;
	errno=0;
	//���㹲����Ϣ����IPC��
	string common_path = mem_path;
	common_path +="0";
	theJSLog<<"common_path="<<common_path.c_str()<<"="<<endi;
	//commonkey=ftok(common_path.c_str(),255);

	//���Ĳ�����ȡ
	std::string commem_keyvalue;
	if( !tpss::getKenelParam( "business.commem.commemkeyvalue", commem_keyvalue ) ) {
		tpss::writelog( 0, "��ȡ�����ڴ�̶�key ֵ" );
		commem_keyvalue = "91671000";
	}
	int commenKeyValue = es::StringUtil::toInt( commem_keyvalue );

	
	commonkey = commenKeyValue;
	if (commonkey==-1)
	{
		sprintf(eMessage,"��ȡ���������ڴ���ftok��������commonkey=%d,errno=%d",commonkey,errno);
		throw CException(10003,eMessage,__FILE__,__LINE__);	
	}

  errno = 0;
	
	m_ishmidcommon = shmget(commonkey, sizeof(S_MemManager), IPC_CREAT|0666);
	
	if (m_ishmidcommon == -1)
	{
		sprintf(eMessage,"��ȡ���������ڴ���shmget��������ishmid=%d,errno=%d",m_ishmidcommon,errno);
		throw CException(10004,eMessage,__FILE__,__LINE__);	
	}
     		
	errno = 0;
	//���ӹ�����Ϣ�����ڴ�,�����׵�ַ
	commonTable = (S_MemManager *)shmat(m_ishmidcommon, NULL, 0);
	      
	if (commonTable == (S_MemManager *) -1)
	{
		//�澯�˳�
		sprintf(eMessage,"��ȡ���������ڴ���shmat��������commonTable=%d,errno=%d",commonTable,errno);
		throw CException(10004,eMessage,__FILE__,__LINE__);	
	}
	theJSLog<<"��ȡ���������ڴ����ɹ�����ѯɾ�������ڴ�������ʼ"<<endi;
	
	m_RWLock.InitLock(commonTable->memlock,NULL);
	m_RWLock.GetWriteLock(commonTable->memlock);
	theJSLog<<"�����ڴ���������="<<commonTable->iTotalTable<<endi;
	for ( int i=0;i<commonTable->iTotalTable;i++ )
	{
		if ( commonTable->m_TableInfo[i].ishmid != 0)
		{
			theJSLog<<"ɾ����������["<<commonTable->m_TableInfo[i].MemName<<"]����"<<endi;
			errno =0;
			ret=shmctl(commonTable->m_TableInfo[i].ishmid,IPC_RMID,NULL);
			if ( ret == -1 && errno != 22 )
			{
				sprintf(eMessage,"ɾ����������[%s]����shmctl IPC_RMID����,errno=%d",commonTable->m_TableInfo[i].MemName,errno);
				throw CException(10004,eMessage,__FILE__,__LINE__);	
			}
			killSingleMem(commonTable->m_TableInfo[i].ishmid);
			commonTable->m_TableInfo[i].ishmid=0;
		}
		for ( int j=0;j<MEM_MAX_INDEX;j++ )
		{
			if ( commonTable->m_TableInfo[i].iIdxshmid[j] != 0)
			{
				theJSLog<<"ɾ����������["<<commonTable->m_TableInfo[i].MemName<<"]��������"<<j<<endi;
				errno =0;
				ret=shmctl(commonTable->m_TableInfo[i].iIdxshmid[j],IPC_RMID,NULL);
				if ( ret == -1 && errno != 22 )
				{
					sprintf(eMessage,"ɾ����������[%s]��������[%d] shmctl IPC_RMID����,errno=%d",commonTable->m_TableInfo[i].MemName,j,errno);
					throw CException(10004,eMessage,__FILE__,__LINE__);	
				}
				killSingleMem(commonTable->m_TableInfo[i].iIdxshmid[j]);
				commonTable->m_TableInfo[i].iIdxshmid[j] = 0;
			}
		}
		theJSLog<<"ɾ����������["<<commonTable->m_TableInfo[i].MemName<<"]�ɹ�"<<endi;	
	}
	theJSLog<<"ȫ��ɾ�����������ɹ�"<<endi;
	//m_RWLock.UnLock(commonTable->memlock);
	//���ù����ڴ治����
	//m_RWLock.GetWriteLock(commonTable->memlock);
	commonTable->isUsed = 0;
	m_RWLock.UnLock(commonTable->memlock);
	m_RWLock.DestroyLock(commonTable->memlock);
	
	return ;
}
	

void CommonMemManager::searchUpdate()
{
	char eMessage[512];
	char mem_name[256];
	int version;
	int shmid_key;
	//CBindSQL ds( DBConn );
	if (!dbConnect(conn)){
		return;
	}
	string sqlStr;
	int cnt=0;
	Statement stmt = conn.createStatement();
	sqlStr="select count(*) from C_COMMEM_CONTROL where updatetype=0 and status='W' and valid_flag='Y' and server_id='";
	sqlStr=sqlStr+serv_id+"'";
	theJSLog<<"sqlStr="<<sqlStr<<"=,"<<__FILE__<<","<<__LINE__<<endd;
	//ds.Open(sqlStr);
	//ds>>cnt;
	//ds.Close();
		stmt.setSQLString(sqlStr);
		stmt.execute();
    stmt>>cnt;
	while ( cnt > 0 )
	{
		Statement stmt1 = conn.createStatement();
		sqlStr="select MEM_NAME,VERSION_ID,SHMKEY_ID from C_COMMEM_CONTROL where updatetype=0 and status='W' and valid_flag='Y' and server_id='";
		sqlStr=sqlStr+serv_id+"' for update";
		theJSLog<<"sqlStr="<<sqlStr<<"=,"<<__FILE__<<","<<__LINE__<<endd;
		try
		{
			//ds.Open(sqlStr, SELECT_QUERY);
			//ds>>mem_name>>version>>shmid_key;
			stmt1.setSQLString(sqlStr);
		  stmt1.execute();
      stmt1>>mem_name>>version>>shmid_key;
			int i;
			int isReload=0;
			m_RWLock.GetReadLock(commonTable->memlock);
			for ( i=0;i<commonTable->iTotalTable; i++ )
			{
				if ( !strcmp(commonTable->m_TableInfo[i].MemName,mem_name) && commonTable->m_TableInfo[i].version == version )
				{
					//vector<CommonMemTableDefine*>::iterator it;
					//for ( it=vtable.begin(); it!= vtable.end(); it ++)
					int j;
					for ( j=0;j<vtable.size();j++)
					{
						//if ( it->getMemName() == mem_name && it->getVersion() == version )
						#ifndef _LINUX_ACC
						if ( vtable.at(j)->getMemName() == mem_name && vtable.at(j)->getVersion() == version )
						#else
						if ( vtable[j]->getMemName() == mem_name && vtable[j]->getVersion() == version )
						#endif
						{
							m_RWLock.UnLock(commonTable->memlock);
							theJSLog<<"�����ڴ�����["<<mem_name<<","<<version<<"]�Ѵ��ڣ����µ���"<<endi;
							//it->reloadTable( serv_id,mem_name,version,shmid_key,i,commonTable,&m_RWLock );
							#ifndef _LINUX_ACC
							vtable.at(j)->reloadTable( serv_id,mem_name,version,shmid_key,i,commonTable,&m_RWLock );
							#else
							vtable[j]->reloadTable( serv_id,mem_name,version,shmid_key,i,commonTable,&m_RWLock );
							#endif
							theJSLog<<"�����ڴ�����["<<mem_name<<","<<version<<"]����ɹ�"<<endi;
							isReload=1;
							break;
						}
					}
					//if (it== vtable.end())
					if ( j==vtable.size())
					{
							m_RWLock.UnLock(commonTable->memlock);
							theJSLog<<"�����ڴ�����["<<mem_name<<","<<version<<"]�����ڣ��´���"<<endi;
							CommonMemTableDefine* newtable = new CommonMemTableDefine(serv_id,mem_name,version,shm_path.c_str(),i,commonTable,&m_RWLock );
							vtable.push_back(newtable);
							theJSLog<<"�����ڴ�����["<<mem_name<<","<<version<<"]����ɹ�"<<endi;
							isReload=1;
					}
				}
				if ( isReload ) break;
			}

			if ( i==commonTable->iTotalTable && isReload==0 )
			{
				m_RWLock.UnLock(commonTable->memlock);
				theJSLog<<"�����ڴ�����["<<mem_name<<","<<version<<"]�����ڣ��´���"<<endi;
				CommonMemTableDefine* newtable = new CommonMemTableDefine(serv_id,mem_name,version,shm_path.c_str(),i,commonTable,&m_RWLock );
				vtable.push_back(newtable);
				theJSLog<<"�����ڴ�����["<<mem_name<<","<<version<<"]�����ɹ�"<<endi;
			}
			//ds.Close();
			
			sqlStr="update C_COMMEM_CONTROL set status='N' where updatetype=0 and valid_flag='Y' and MEM_NAME='";
			sqlStr=sqlStr+mem_name+"' and version_id=";
			char s_ver[100];
			sprintf(s_ver,"%d",version);
			sqlStr=sqlStr+s_ver+" and server_id='";
			sqlStr=sqlStr+serv_id+"'";
			theJSLog<<"sqlStr="<<sqlStr<<"=,"<<__FILE__<<","<<__LINE__<<endd;
			//ds.Open(sqlStr,NONSELECT_DML);       
  		
			//ds.Execute();
			stmt1.setSQLString(sqlStr);
		  stmt1.execute();
		  conn.commit();
			//DBConn.Commit();
			
			sqlStr="select count(*) from C_COMMEM_CONTROL where updatetype=0 and status='W' and valid_flag='Y' and server_id='";
			sqlStr=sqlStr+serv_id+"'";
			theJSLog<<"sqlStr="<<sqlStr<<"=,"<<__FILE__<<","<<__LINE__<<endd;
			stmt1.setSQLString(sqlStr);
		  stmt1.execute();
		  stmt1>>cnt;
		//	ds.Open(sqlStr);
		//	ds>>cnt;
		//	ds.Close();
		  stmt1.close();
		}
		catch ( CException e )
		{
			//modi by ww at 2015-7-8
			//�ͷ��ڴ���
			m_RWLock.UnLock(commonTable->memlock);
			//���±�־λΪ'E'
			string m_name=mem_name;
			sqlStr="update C_COMMEM_CONTROL set status='E' where updatetype=0 and valid_flag='Y' and mem_name='"+m_name+"'and server_id='";
			sqlStr=sqlStr+serv_id+"'";
			theJSLog<<"sqlStr="<<sqlStr<<__FILE__<<","<<__LINE__<<endd;
			stmt.setSQLString(sqlStr);    
  			stmt.execute();
 		 	conn.commit();
 			 conn.close();
			 
			sprintf(eMessage,"���µ��빲���ڴ�����[%s,%d]����",mem_name,version);
			e.PushStack(11001,eMessage,__FILE__, __LINE__);
			throw e;
		}
	}			
	
	theJSLog<<"Tables update finish!"<<endi;
	conn.close();
	return ;
	
}

void CommonMemManager::ReadCommonInfo(int readLine)
{
	if ( commonTable == NULL )
	{
		theJSLog<<"��û�������������ڴ���"<<__FILE__<<","<<__LINE__<<endi;
		return;
	}
	m_RWLock.GetReadLock(commonTable->memlock);
	theJSLog<<"������="<<commonTable->iTotalTable<<"=�����ݱ�洢����"<<__FILE__<<","<<__LINE__<<endi;
	for ( int i=0;i<commonTable->iTotalTable; i++ )
	{
		if ( readLine > -1 && readLine != i+1 ) continue;
		char tempStr[255];
		sprintf(tempStr,"%x",commonTable->m_TableInfo[i].iTableKey);		
		theJSLog<<"��"<<i+1<<"�����ݱ�["<<commonTable->m_TableInfo[i].MemName<<","<<commonTable->m_TableInfo[i].version<<"],����["<<commonTable->m_TableInfo[i].chTableName<<"]"<<endi;
		theJSLog<<"    �ڴ������С="<<commonTable->m_TableInfo[i].iMemSize<<",����¼����="<<commonTable->m_TableInfo[i].iMemRows<<",ʵ��ʹ�ü�¼����="<<commonTable->m_TableInfo[i].iRealRows<<endi;
		theJSLog<<"    ��TableKey=0x"<<tempStr<<",��ishmid="<<commonTable->m_TableInfo[i].ishmid<<endi;
		for ( int j=0;j<MEM_MAX_INDEX;j++)
		{
			if ( commonTable->m_TableInfo[i].iIdxshmid[j] > 0 )
			{
				sprintf(tempStr,"%x",commonTable->m_TableInfo[i].iIdxKey[j]);		
				theJSLog<<"    ����["<<j<<"]ishKey=0x"<<tempStr<<",ishmid="<<commonTable->m_TableInfo[i].iIdxshmid[j]<<endi;
			}
		}
		//theJSLog<<" "<<endi;
	}
	m_RWLock.UnLock(commonTable->memlock);
}

void CommonMemManager::AttachTable(string mem_name1)
{
	char eMessage[512];
	//CBindSQL ds( DBConn );
	string sqlStr;
	int version;
	int shmid_key;
	char mem_name[256];
	if (!dbConnect(conn)){
		sprintf(eMessage,"�������ݿ�ʧ��");
				throw CException(10003,eMessage,__FILE__,__LINE__);	
	}
	sqlStr="select MEM_NAME,VERSION_ID,SHMKEY_ID from C_COMMEM_CONTROL where valid_flag='Y' and server_id='";
	sqlStr=sqlStr+serv_id+"' and MEM_NAME='";
	sqlStr=sqlStr+mem_name1+"' ";
	theJSLog<<"sqlStr="<<sqlStr<<__FILE__<<","<<__LINE__<<endd;
	
	Statement stmt = conn.createStatement();
	stmt.setSQLString(sqlStr);
	stmt.execute();	
			
	//ds.Open(sqlStr, SELECT_QUERY);
	//while(ds>>mem_name>>version>>shmid_key)
	while(stmt>>mem_name>>version>>shmid_key)
	{
		try
		{
			int i;
			m_RWLock.GetReadLock(commonTable->memlock);
			for ( i=0;i<commonTable->iTotalTable; i++ )
			{
				if ( !strcmp(commonTable->m_TableInfo[i].MemName,mem_name) && commonTable->m_TableInfo[i].version == version )
				{
					CommonMemTableDefine* newtable = new CommonMemTableDefine();
					newtable->AttachTable( serv_id,mem_name,version,shm_path.c_str(),i,commonTable,&m_RWLock );
					vtable.push_back(newtable);
					break;
				}
			}

			if ( i==commonTable->iTotalTable )
			{
				sprintf(eMessage,"�Ҳ��������ڴ�����[%s]",mem_name);
				throw CException(10003,eMessage,__FILE__,__LINE__);	
			}
			m_RWLock.UnLock(commonTable->memlock);
		}
		catch ( CException e )
		{
			sprintf(eMessage,"���ӹ����ڴ�����[%s,%d]����",mem_name);
			e.PushStack(10003,eMessage,__FILE__, __LINE__);
			throw e;
		}
	}
	//ds.Close();
  conn.close();
}

void CommonMemManager::detach()
{
	for ( int i=0 ; i < vtable.size() ; i++ )
	{
		vtable[i]->detach();
	}
	DetachCommonTable();
}

void CommonMemManager::queryTableRecord(string mem_name,const char *queryCondition,int id)
{
	for ( int i=0;i<vtable.size() ; i++ )
	{
		#ifndef _LINUX_ACC
		if ( mem_name == vtable.at(i)->getMemName() )
		#else
		if ( mem_name == vtable[i]->getMemName() )
		#endif
		{
			#ifndef _LINUX_ACC
			vtable.at(i)->queryTableRecord(queryCondition,id);
			#else
			vtable[i]->queryTableRecord(queryCondition,id);
			#endif
		}
	}
}

void CommonMemManager::killSingleMem(int ismidkey)
{
	char cmdline[512];
	char chScanBuff[512];
	FILE *fp = 0;
	
	sprintf(
		chScanBuff,
		"ipcs -m | grep %d",ismidkey);
		
	if((fp = popen(chScanBuff , "r")) == 0)
	{
		return;
	}
	
	chScanBuff[0]=0;
	if (fgets(chScanBuff, sizeof(chScanBuff), fp) != 0)
	{
		if ( chScanBuff[0] !=0 )
		{
			sprintf(cmdline,"ipcrm -m %d",ismidkey);
			system(cmdline);
		}
	}
	pclose(fp);

	return;
}


