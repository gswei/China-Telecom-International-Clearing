/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-08-16
File:			 CSql.cpp
Description:	 ʵʱSQL���ģ��
History:
<table>
revision	author            date                description
--------    ------            ----                -----------
</table>
**************************************************************************/
#include "CSql.h"
CLog theJSLog;
SGW_RTInfo	rtinfo;
struct stat fileInfo;


CSql::CSql()
{
	audit_file_num = 20;
	mini_flag = false;

	memset(filenames,0,sizeof(filenames));
	memset(m_Filename,0,sizeof(m_Filename));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(currTime,0,sizeof(currTime));
	
	mVfile.clear();
	vDealFlag.clear();
}

CSql::~CSql()
{
	mdrDeal.dr_ReleaseDR();
}

	//ģ���ʼ������
bool CSql::init(int argc,char** argv)
{

    if(!PS_Process::init(argc,argv))
    {
		return false;
    }
	
	mConfParam.iflowID = getFlowID();
	mConfParam.iModuleId = getModuleID();

	//theJSLog.setLog(szLogPath, szLogLevel,"CSql", "GJJS", 001);	
	//theJSLog<<"��־·����"<<szLogPath<<" ��־����"<<szLogLevel<<"  ÿ���ٲ��ļ�����:"<<audit_file_num<<endi;

	string sql;
  	try
	{
		if (dbConnect(conn))
	 	{
			Statement stmt = conn.createStatement();

			sql = "select ext_param from TP_PROCESS where billing_line_id = :1 and module_id = :2";
			stmt.setSQLString(sql);
			stmt<<mConfParam.iflowID<<mConfParam.iModuleId;
			stmt.execute();
			if(!(stmt>>mConfParam.szService))
			{
				cout<<"����tp_process���ֶ�ext_param������ģ��["<<mConfParam.iModuleId<<"]service"<<endl;
				return false ;
			}
		
			theJSLog.setLog(szLogPath,szLogLevel,mConfParam.szService, "GJJS", 001);
		
			theJSLog<<"szLogPath="<<szLogPath<<"	szLogLevel="<<szLogLevel<<endi;
			theJSLog<<"flowID="<<mConfParam.iflowID<<"	ModuleId="<<mConfParam.iModuleId<<"	szService="<<mConfParam.szService<<endi;

			//��ȡ�ļ���ȡĿ¼
			sql = "select VARVALUE from C_GLOBAL_ENV where VARNAME = 'SQL_PATH'";
			stmt.setSQLString(sql);
			stmt.execute();//ִ��sql���
			if(!(stmt >> mConfParam.szInPath))//��ȡ���
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"C_GLOBAL_ENV��û������:SQL_PATH");
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return false;
			}	
			theJSLog <<"szInPath="<<mConfParam.szInPath<<endi;
			completeDir(mConfParam.szInPath);

			//��ȡ�ļ�ִ�гɹ��ı���Ŀ¼
			sql = "select VARVALUE from C_GLOBAL_ENV where VARNAME = 'SQL_BAK_PATH'";
			stmt.setSQLString(sql);
			stmt.execute();//ִ��sql���
			if(!(stmt >> mConfParam.szBakPath))//��ȡ���
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"C_GLOBAL_ENV��û������:SQL_BAK_PATH");
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return false;
			}
			theJSLog <<"szBakPath="<<mConfParam.szBakPath<<endi;
			completeDir(mConfParam.szBakPath);

			//��ȡ�ļ�ִ��ʧ�ܵĴ���Ŀ¼
			sql = "select VARVALUE from C_GLOBAL_ENV where VARNAME = 'SQL_ERR_PATH'";
			stmt.setSQLString(sql);
			stmt.execute();//ִ��sql���
			if(!(stmt >> mConfParam.szErrPath))		//��ȡ���
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"C_GLOBAL_ENV��û������:SQL_ERR_PATH");
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return false;
			}
			theJSLog <<"szErrPath="<<mConfParam.szErrPath<<endi;
			completeDir(mConfParam.szErrPath);
			
			//��ȡ�ļ�ִ���ٲ�ʧ�ܵ�Ŀ¼
			sql = "select VARVALUE from C_GLOBAL_ENV where VARNAME = 'SQL_FAIL_PATH'";
			stmt.setSQLString(sql);
			stmt.execute();//ִ��sql���
			if(!(stmt >> mConfParam.szFailPath))		//��ȡ���
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"C_GLOBAL_ENV��û������:SQL_FAIL_PATH");
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return false;
			}
			theJSLog <<"szFailPath="<<mConfParam.szFailPath<<endi;
			completeDir(mConfParam.szFailPath);
			
			sql = "select VARVALUE from C_GLOBAL_ENV where VARNAME = 'SQL_AUDIT_NUM'";
			stmt.setSQLString(sql);
			stmt.execute();//ִ��sql���
			if(!(stmt >> audit_file_num))			//��ȡ���
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"C_GLOBAL_ENV��û������:SQL_AUDIT_NUM");
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return false;
			}
			theJSLog <<"SQL_AUDIT_NUM="<<audit_file_num<<endi;

			sql = "select VARVALUE from C_GLOBAL_ENV where VARNAME = 'SQL_FAIL_MINI_NUM'";
			stmt.setSQLString(sql);
			stmt.execute();//ִ��sql���
			if(!(stmt >> audit_fail_mini_num))		//��ȡ���
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"C_GLOBAL_ENV��û������:SQL_FAIL_MINI_NUM");
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return false;
			}
			theJSLog <<"SQL_FAIL_MINI_NUM="<<audit_fail_mini_num<<endi;

			stmt.close();
			
		 }else
		 {
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"init() �������ݿ�ʧ�� connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
			return  false;
		 }
	 	 conn.close();

  	}catch( SQLException e )
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"�����ļ�·��ʱʧ��:%s,sql���Ϊ:%s",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
		conn.close();
		return false;
  	}
   
   DIR *dirptr = NULL; 
   if((dirptr=opendir(mConfParam.szInPath)) == NULL)
   {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"�����ļ�·��[%s]��ʧ��",mConfParam.szInPath);
		theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����

		return false ;

	}else closedir(dirptr);

	if((dirptr=opendir(mConfParam.szBakPath)) == NULL)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"�����ļ�·��[%s]��ʧ��",mConfParam.szBakPath);
		theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����

		return false ;
			
	}else closedir(dirptr);

	if((dirptr=opendir(mConfParam.szErrPath)) == NULL)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"�����ļ�·��[%s]��ʧ��",mConfParam.szErrPath);
		theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����

		return false;

	}else closedir(dirptr);

	if((dirptr=opendir(mConfParam.szFailPath)) == NULL)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"�ٲ�ʧ���ļ�·��[%s]��ʧ��",mConfParam.szFailPath);
		theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����

		return false ;
			
	}else closedir(dirptr);

   if(!(rtinfo.connect()))
	{
		theJSLog.writeLog(0,"��������ʱ��Ϣʧ��");	 
		return false;
	}	
	rtinfo.getDBSysMode(petri_status);
	theJSLog<<"petri status:"<<petri_status<<endi;

	bool flag = true;
	for(int i=1;i<argc;i++)
	{
		if(strcmp(argv[i],"-k") == 0)
		{
			theJSLog<<"ģ��["<<module_name<<"]����������..."<<endi;
			flag = false;
			mdrDeal.mdrParam.m_enable = false;
			break;
		}	
	}
	
	if(flag)
	{
		char tmp[12];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%ld",getPrcID());

		if(!mdrDeal.drInit(module_name,tmp))  return false;
	}

/*
	char sParamName[256];
	CString sKeyVal;
	sprintf(sParamName, "sql.file.audit.num");
	if(param_cfg.bGetMem(sParamName, sKeyVal))
	{
		audit_file_num = sKeyVal.toInteger();
		if((audit_file_num <= 0) || (audit_file_num) > 60)
		{
			audit_file_num = 20;
		}

		theJSLog<<sParamName<<"="<<audit_file_num<<endi;
	}
	else
	{	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"���ں��Ĳ���������ÿ��ɨ���ļ��ĸ���,������:%s",sParamName);
		theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
		return false ;
	}
*/

	bool bb = initializeLog(argc,argv,false);  //�Ƿ����ģʽ
	if(!bb)
	{
		theJSLog.writeLog(0,"��ʼ���ڴ���־�ӿ�ʧ��"); 
		return false;
	}

   theJSLog<<"��ʼ�����...\n"<<endi;

   return true ;
}

//����ʧ�ܵ��ļ�����
int CSql::getFileExist()
{
	int  count = 0;

	for(int i = 0;i<mVfile.size();i++)
	{
		memset(filenames,0,sizeof(filenames));
		strcpy(filenames,mConfParam.szInPath);
		strcat(filenames,mVfile[i].c_str());

		if(access(filenames,F_OK|R_OK))
		{
			theJSLog<<"�ļ�["<<mVfile[i].c_str()<<"]������!"<<endw;
			count++;
			continue;
		}
	}

	return count;
}

void CSql::run()//ɨ��Ŀ¼����ȡ�ļ���
{
	int ret = 0,event_sn, event_type;
	long param1, param2, src_id;
	unsigned long filesize = 0;

	if(gbExitSig)
	{
		theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "Ӧ�ó����յ��˳��ź�");
		prcExit();
		return;
	}
	
	ret=getCmd(event_sn, event_type, param1, param2, src_id);
	if(ret == 1)
	{
		if(event_type == EVT_CMD_STOP)
		{
			theJSLog<<"***********���յ��˳�����**********************\n"<<endw;
			prcExit();
		}
	}

	//�ж����ݿ�״̬
	short db_status = 0;
	rtinfo.getDBSysMode(db_status);
	if(db_status != petri_status)
	{
		theJSLog<<"���ݿ�״̬�л�... "<<petri_status<<"->"<<db_status<<endw;
		int cmd_sn = 0;
		if( !putEvt(cmd_sn, EVT_RPT_DBSTATUS, 0, db_status, DSPCH_PRC_ID) )
		{
			theJSLog<<"�������ݿ����״̬ʧ�ܣ�\n"<<endw;
			return ;
		}
		petri_status = db_status;
	}
	if(petri_status == DB_STATUS_OFFLINE)	return ;
	
	try
	{
		if(mdrDeal.mdrParam.drStatus==1)  //��ϵͳ
		{
				//���trigger�����ļ��Ƿ����
				if(!mdrDeal.CheckTriggerFile(m_triggerFile))
				{
					sleep(1);
					return ;
				}

				//��ȡͬ������
				memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));
				ret=mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
				if(ret)
				{
					theJSLog<<"ͬ��ʧ��..."<<endw;
					return ;
				}
				
				theJSLog<<"######## start deal file ###################"<<endi;
		
				splitString(mdrDeal.m_SerialString,";",mVfile,false,false);

				//int dr_GetAuditMode()1��ʾͬ����2��ʾ����, ����Ϊʧ�ܣ�-1�����ô���-2�����ļ���ȡ��������
				//int iStatus = dr_GetAuditMode(module_name);
				int iStatus = mdrDeal.mdrParam.aduitMode;
				
				if(iStatus == 1)		//ͬ��ģʽ,	��ϵͳ�ȴ�ָ��ʱ�� 
				{
					bool flag=false;
					int times=1,count = 0;
					while(times<31)
					{
						count = getFileExist();
						if(count)
						//if(access(filenames,F_OK|R_OK))
						{
							theJSLog<<"������"<<times<<"��,����ʧ���ļ�����:"<<count<<endw;
							times++;
							sleep(10);
						}
						else
						{
							flag=true;
							break;
						}
					}
					if(!flag)
					{
						//dr_AbortIDX();
						mdrDeal.dr_abort();
						mVfile.clear();

						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"��ϵͳ���������ļ�����");
						theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);			
						return ;
					}
				}
				else if(iStatus==2) //����ģʽ����ϵͳ
				{
					int count = 0;
					while(1)
					{
						//�����ж�
						if(gbExitSig)
						{
							//dr_AbortIDX();
							mdrDeal.dr_abort();
							
							theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "Ӧ�ó����յ��˳��ź�");
							prcExit();
							return ;
						}
						
						count = getFileExist();
						if(count)
						//if(access(filenames,F_OK|R_OK))
						{
							sleep(10);
						}
						else
						{
							break;
						}
					}
				}
				
				if(!(dbConnect(conn)))
				{
					//dr_AbortIDX();
					mdrDeal.dr_abort();
					mVfile.clear();

					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"run() �������ݿ�ʧ�� connect error");
					theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
					sleep(30);
					return  ;
				}

				//�����ļ�
				ret = doAllSQL();
				conn.close();

				theJSLog<<"######## end deal file ########\n"<<endi;
		}
		else      //��ϵͳ,������ϵͳ    
		{
			if(mMiniVfile.size())						//��ʾ�ϴ��ٲ�ʧ��,���ļ���̫����Ҫ��С��Χ
			{
				vector<string>::iterator  iter1 = mMiniVfile.begin();
				vector<string>::iterator  iter2 = mMiniVfile.begin();

				int szSize = audit_fail_mini_num;
				if(mMiniVfile.size() <= audit_fail_mini_num)
				{
					mini_flag = false;					//���һ��
					szSize = mMiniVfile.size();
				}

				for(int i=0;i<szSize;i++)
				{
					mVfile.push_back(mMiniVfile[i]);
					++iter2;
				}
				
				mMiniVfile.erase(iter1,iter2);

			}else
			{

			if(scan.openDir(mConfParam.szInPath))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"���ļ�Ŀ¼[%s]ʧ��",mConfParam.szInPath);
				theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����
				return ;	
			}	
			
			filesize = 0;
			int rett = 0,counter = 0;
			while(1)		
			{
				memset(filenames,0,sizeof(filenames));	    
				rett = scan.getFile("*.sql",filenames); 
				if(rett == 100)
				{		
						break ;
				}
				if(rett == -1)
				{	
						break ;							//��ʾ��ȡ�ļ���Ϣʧ��
				}

				counter++;								//2013-11-05ÿɨ��50���ļ�����
				if(counter > audit_file_num)
				{
					break;
				}
				
				/*���ļ�����·��ȥ��*/
				char* p = strrchr(filenames,'/');
				strcpy(m_Filename,p+1);
						
				mVfile.push_back(m_Filename);
				
				stat(filenames,&fileInfo);				//��ȡ�ļ���С,����ָ����С1M,���2K����������ɨ�������ļ�			
				filesize += (unsigned long)fileInfo.st_size;
				if(filesize > 1048576)
				{
					theJSLog<<"�ļ���С�ܺ��Ѿ�ָ����С,ֹͣɨ���ļ�;filesize="<<filesize<<endi;
					break;
				}
			}

			scan.closeDir();
			
			}	//else

			if(mVfile.size() != 0)
			{
				if(!(dbConnect(conn)))
				{
					mVfile.clear();
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"run() �������ݿ�ʧ�� connect error");
					theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
					sleep(30);

					return  ;
				}

				memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));
				for(int i = 0;i<mVfile.size();i++)
				{
					sprintf(mdrDeal.m_SerialString,"%s%s;",mdrDeal.m_SerialString,mVfile[i]);
				}

				theJSLog<<"######## start deal file #####################"<<endi;
				
				ret = mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
				if(ret)
				{
					theJSLog<<"ͬ��ʧ��...."<<endw;
					mVfile.clear();
					conn.close();
					sleep(30);
					return ;
				}
			    
				//�����ļ�
				ret = doAllSQL();
				
				conn.close();

				theJSLog<<"######## end deal file ########\n"<<endi;
			}

		}
	}catch (jsexcp::CException &e) 
	{	
		mVfile.clear();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		
	}

}

bool CSql::doAllSQL()			
{	
	Statement stmt;
	try
	{
		stmt = conn.createStatement();
	}
	catch ( SQLException e)
	{
		//dr_AbortIDX();
		mdrDeal.dr_abort();
		mVfile.clear();

		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"doAllSQL() err %s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
		
		return false;
	}
	
	int count = 0,ret = 0;	
	char szBuff[JS_MAX_RECORD_LEN],flag;
	memset(mdrDeal.m_AuditMsg,0,sizeof(mdrDeal.m_AuditMsg));
	
	for(int i = 0;i<mVfile.size();i++)
	{
		memset(filenames,0,sizeof(filenames));
		strcpy(filenames,mConfParam.szInPath);
		strcat(filenames,mVfile[i].c_str());
		memset(m_Filename,0,sizeof(m_Filename));
		strcpy(m_Filename,mVfile[i].c_str());

		count = 0;
		flag = 'Y';

		try{			
		 		
				ifstream in(filenames,ios::in) ;
				if(!in)
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"�ļ�[%s]�򿪳���",filenames);
					theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��

					sprintf(mdrDeal.m_AuditMsg,"%s%s|",mdrDeal.m_AuditMsg,strerror(errno));
					vDealFlag.push_back('E');

					continue;
				}	
			
				memset(szBuff,0,sizeof(szBuff));
				while(in.getline(szBuff,sizeof(szBuff)))   
				{
					count++;
					stmt.setSQLString(szBuff);	
					stmt.execute();  //ִ��sql���

					//if((count%SQL_COMMIT_COUNT) == 0)			//ÿ��500���ύ
					//{
					//	stmt.commit();
					//}

					memset(szBuff,0,sizeof(szBuff));
				}
				in.close();
				theJSLog<<"������sql�ļ�:"<<m_Filename<<endi;
				
				vDealFlag.push_back(flag);
					
				sprintf(mdrDeal.m_AuditMsg,"%s%c;%d|",mdrDeal.m_AuditMsg,flag,count);
		  }
		  catch( SQLException e ) 
		  {
				stmt.rollback();
				flag = 'E';
				vDealFlag.push_back(flag);

				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"�ļ�[%s]��sql���ʧ��:%s,sql:(%s)",filenames,e.what(),szBuff);
				theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

				sprintf(mdrDeal.m_AuditMsg,"%s%c;%d|",mdrDeal.m_AuditMsg,flag,count);
		  }

	}

		ret = mdrDeal.IsAuditSuccess(mdrDeal.m_AuditMsg);
		if(ret)							//�ٲ�ʧ��
		{
			stmt.rollback();
			stmt.close();
			
			if(mVfile.size() > audit_fail_mini_num)
			{
				if(!mini_flag)								//��һ���ٲ�ʧ����Ҫ
				{
					mini_flag = true;
					theJSLog<<"����sql�ļ�����:"<<mVfile.size()<<">"<<audit_fail_mini_num<<",������С��Χ��ѯ"<<endi;		
					for(int i = 0;i<mVfile.size();i++)
					{	
						mMiniVfile.push_back(mVfile[i]);
					}
				}

				mVfile.clear();
				vDealFlag.clear();

				return true;
			}
			
			if(ret != 3)				//2013-11-07 �ٲó�ʱ���ƶ��ļ�
			{			
				moveFiles(1);			
			}
		}			
		else
		{
			stmt.close();	 
			saveLog();					//ÿ����һ���ļ�����¼��D_SQL_FILEREG����

			moveFiles(0);				
		}
		
		mVfile.clear();
		vDealFlag.clear();

		return true;
}

void CSql::saveLog()			//ÿ����һ���ļ������浽D_SQL_FILEREG����
{
	string sql;
	//char flag = 'Y';
	Statement stmt = conn.createStatement();
	sql = "insert into D_SQL_FILEREG(FILE_NAME,DEAL_DATE,DEAL_FLAG) values(:v1,sysdate,:v2)";
	stmt.setSQLString(sql);	

	for(int i = 0;i<mVfile.size();i++)
	{
		try
		{
			stmt<<mVfile[i]<<vDealFlag[i];
			stmt.execute();						//ִ��sql���
			stmt.commit();
		
		}catch( SQLException e ) 
		{
			stmt.rollback();

			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"�����ļ�[%s]ʱ���浽D_SQL_FILEREG��ʱsql���ʧ��:%s,sql:(%s)",mVfile[i],e.what(),sql);
			theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
		} 
		
	}
	
	stmt.close();
}

//flag:0Ϊ��ʾ�ٲóɹ�,1�ٲ�ʧ��Ŀ¼

bool CSql::moveFiles(int flag)						//���Ѿ��������ļ��ƶ���ָ������Ŀ¼	
{
	char bak_dir[512];
	char tmp[1024];

	if(flag == 0)									//�ٲóɹ������浽ָ��Ŀ¼				
	{	
		getCurTime(currTime);
		memset(bak_dir,0,sizeof(bak_dir));
		strcpy(bak_dir,mConfParam.szBakPath);
		strncat(bak_dir,currTime,6);
		completeDir(bak_dir);
		strncat(bak_dir,currTime+6,2);
		completeDir(bak_dir);
		
		if(chkAllDir(bak_dir) == 0)
		{	
			for(int i = 0;i<mVfile.size();i++)
			{	
				memset(filenames,0,sizeof(filenames));
				strcpy(filenames,mConfParam.szInPath);
				strcat(filenames,mVfile[i].c_str());
				
				if(vDealFlag[i] == 'Y')
				{
					memset(tmp,0,sizeof(tmp));
					strcpy(tmp,bak_dir);
					strcat(tmp,mVfile[i].c_str());
					theJSLog<<"�����ļ�["<<mVfile[i]<<"]��Ŀ¼:"<<bak_dir<<endi;

					if(rename(filenames,tmp))		
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"�ƶ��ļ�[%s]������Ŀ¼ʧ��: %s",filenames,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
						//return false;
					}	
				}
				else
				{
					theJSLog<<"���ļ�["<<mVfile[i]<<"]������Ŀ¼ "<<mConfParam.szErrPath<<endi;
					memset(tmp,0,sizeof(tmp));
					strcpy(tmp,mConfParam.szErrPath);
					strcat(tmp,mVfile[i].c_str());

					if(rename(filenames,tmp))		
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"�ƶ��ļ�[%s]������Ŀ¼ʧ��: %s",filenames,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
						//return false;
					}	

				}
			}

		}
		else
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"����·��[%s]�����ڣ����޷�����",bak_dir);
			theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg);		//��Ŀ¼����
			prcExit();
		}		
	}
	else										//�ٲ�ʧ�ܣ�����ִ���ļ��е�sql���ʧ�ܣ����浽ʧ��Ŀ¼				
	{ 
		theJSLog<<"���ļ��Ƶ��ٲ�ʧ��Ŀ¼ "<<mConfParam.szFailPath<<endi;

		for(int i = 0;i<mVfile.size();i++)
		{	
			memset(filenames,0,sizeof(filenames));
			strcpy(filenames,mConfParam.szInPath);
			strcat(filenames,mVfile[i].c_str());

			memset(tmp,0,sizeof(tmp));
			strcpy(tmp,mConfParam.szFailPath);
			strcat(tmp,mVfile[i].c_str());
			if(rename(filenames,tmp))		
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"�ƶ��ļ�[%s]��ʧ��Ŀ¼ʧ��: %s",filenames,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
				//return false;
			}
			
		}
	}

	return true;
}

//2013-11-02 �����˳�����
void CSql::prcExit()
{
	//int ret = 0;

	mdrDeal.dr_ReleaseDR();
	
	PS_Process::prcExit();
}

/*
//���ֳ�ʼ��
bool CSql::drInit()
{
		//����ģ������ʵ��ID
		char tmp[32];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%ld",getPrcID());

		theJSLog << "��ʼ������ƽ̨,ģ����:"<< module_name<<" ʵ����:"<<tmp<<endi;

		int ret = dr_InitPlatform(module_name,tmp);
		if(ret != 0)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"����ƽ̨��ʼ��ʧ��,����ֵ=%d",ret);
			theJSLog.writeLog(LOG_CODE_DR_INIT_ERR,erro_msg);

			return false;
		}
		else
		{
			theJSLog<<"dr_InitPlatform ok."<<endi;
		}

		mdrParam.m_enable = true;

		mdrParam.drStatus = _dr_GetSystemState();	//��ȡ����ϵͳ״̬
		if(mdrParam.drStatus < 0)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"��ȡ����ƽ̨״̬����,����ֵ=%d",mdrParam.drStatus);
			theJSLog.writeLog(LOG_CODE_DR_GETSTATE_ERR,erro_msg);

			return false;
		}
		
		if(mdrParam.drStatus == 0)		theJSLog<<"��ǰϵͳ����Ϊ��ϵͳ"<<endi;
		else if(mdrParam.drStatus == 1)	theJSLog<<"��ǰϵͳ����Ϊ��ϵͳ"<<endi;
		else if(mdrParam.drStatus == 2)	theJSLog<<"��ǰϵͳ���÷�����ϵͳ"<<endi;

		return true;
}

//��ϵͳ����ͬ������,��ϵͳ��ȡͬ������
int CSql::drVarGetSet(char* serialString)
{
		int ret  = 0;
		char tmpVar[5000] = {0};
		
		if(!mdrParam.m_enable)
		{
			return ret;
		}

		//�������ƽ̨���л���
		ret = dr_CheckSwitchLock();   
		if(ret != 0)  
		{  
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"��������л���ʧ��,����ֵ=%d",ret);
			theJSLog.writeLog(LOG_CODE_DR_CHECK_LOCK_ERR,erro_msg);

			return -1;  
		} 
		//��ʼ��index  
		ret = dr_InitIDX();  
		if(ret != 0)  
		{  
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"��ʼ��indexʧ��,����ֵ=%d",ret);
			theJSLog.writeLog(LOG_CODE_DR_INIT_IDX_ERR,erro_msg);

			//dr_AbortIDX();
			return -1;  
		}
		
		//��ϵͳ�����ļ�����·�����ļ��� ֻ������ƽ̨���Ը�֪,��ϵͳ�޷�ʶ��
		if(mdrParam.drStatus != 1)
		{
			snprintf(tmpVar, sizeof(tmpVar), "%s",input_path);
			ret = dr_SyncIdxVar("@@CHECKPATH", tmpVar,SYNC_SINGLE);  
			if(ret != 0)
			{
				theJSLog<<"�����ļ�����·��ʧ��,�ļ�·��["<<input_path<<"]"<<endi;
				dr_AbortIDX();
				return -1;
			}
			
			snprintf(tmpVar, sizeof(tmpVar), "%s", m_Filename);
			ret = dr_SyncIdxVar("@@CHECKFILE", tmpVar,SYNC_SINGLE);  
			if(ret != 0)
			{
				theJSLog<<"�����ļ�ʧ��,�ļ���["<<m_Filename<<"]"<<endi;
				dr_AbortIDX();
				return -1;
			}

			cout<<"�����ļ�·��:"<<input_path<<" �ļ���:"<<m_Filename<<endl;
		}


		snprintf(tmpVar, sizeof(tmpVar), "%s", serialString);
		//��ϵͳ��Ҫͬ����index ����ֵ�ԡ�д������ƽ̨ά����index �ļ���
		//��ϵͳ���øú����Ľ���ǣ�var��ú���ϵͳһ�������������ֵ��	SYNC_SINGLE��ʾע�ᵥһ���������
		ret = dr_SyncIdxVar("serialString", tmpVar, SYNC_SINGLE);		
		if (ret != 0)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"�����д�ʱʧ�ܣ�������:%s",serialString);
			theJSLog.writeLog(LOG_CODE_DR_SYSC_IDXVAR_ERR,erro_msg);

			dr_AbortIDX();
			return -1;
		}
		if(mdrParam.drStatus == 1)
		{
			//serialString = tmpVar;			//ͬ�������ַ���,��ϵͳ�Ǹ�ֵ����ϵͳ��ȡֵ
			strcpy(serialString,tmpVar);
			//mdrParam.m_AuditMsg = tmpVar;			//Ҫ�ٲõ��ַ���
		}

		theJSLog<<"���ε�ͬ����serialString:"<<serialString<<endi;//for test

		// <5> ����ʵ����  ������ϵͳע���IDX��ģ����ò�����
		//��ϵͳ��index manager���IDX��������󣬰�ʹ�øú���ע������������Ϊģ��ĵ��ò���trigger��Ӧ�Ľ���
		snprintf(tmpVar, sizeof(tmpVar), "%d", getPrcID());
		ret = dr_SyncIdxVar("@@ARG", tmpVar,SYNC_SINGLE);  
		if(ret !=0)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"����ʵ����ʧ��:%s",tmpVar);
			theJSLog.writeLog(LOG_CODE_DR_SYSC_IDXVAR_ERR,erro_msg);

			dr_AbortIDX();  
			return -1;
		}
		
		
		// <6> Ԥ�ύindex  �˹ؼ������ڽ�ƽ̨��ǰ�ڴ��е��������д�����
		ret = dr_SyncIdxVar("@@FLUSH","SUCCESS",SYNC_SINGLE);  
		if (ret != 0 )
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"Ԥ�ύindexʧ��");
			theJSLog.writeLog(LOG_CODE_DR_SYSC_IDXVAR_ERR,erro_msg);

			dr_AbortIDX();
			return -1;
		}
		
		
		// <7> �ύindex  	�ύIndex����index�ļ���������ɱ�־
		ret = dr_CommitIDX();  
		if(ret != 0)  
		{  
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"�ύindexʧ��,����ֵ=%d",ret);
			theJSLog.writeLog(LOG_CODE_DR_COMMIT_IDX_ERR,erro_msg);

			dr_AbortIDX();  
			return -1;  
		}

		//��ϵͳ����Ŀ¼
		//if(!m_syncDr.isMaster())thelog<<"��ϵͳSerialString��"<<mdrParam.m_SerialString<<endi;

		return ret;

}

//�ٲ��ַ���
 int CSql::IsAuditSuccess(const char* dealresult)
 {
		int auitStatus = 0, ret = 0;
		
		if(!mdrParam.m_enable)
		{
			return ret;
		}
		
		theJSLog<<"wait dr_audit() ..."<<endi;
		ret = dr_Audit(dealresult);
		if(2 == ret )
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"�����ٲ�ʧ��,���:%d,����:%s",ret,dealresult);
			theJSLog.writeLog(LOG_CODE_DR_AUDIT_ERR,erro_msg);

			dr_AbortIDX();
		
		}
		else if (3 == ret)
		{
			theJSLog<<"�����ٲó�ʱ..."<<endi;
			dr_AbortIDX();
		
		}
		else if(4 == ret)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"�Զ�idx�쳣��ֹ");
			theJSLog.writeLog(LOG_CODE_DR_IDX_STOP_ERR,erro_msg);

			dr_AbortIDX();

		}
		else if(1 == ret)
		{
			ret = dr_CommitSuccess();
			if(ret != 0)
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"ҵ��ȫ���ύʧ��(����ƽ̨),����ֵ=%d",ret);
				theJSLog.writeLog(LOG_CODE_DR_COMMIT_SUCCESS_ERR,erro_msg);

				//dr_AbortIDX();
			}
			theJSLog<<"ret = "<<ret<<"�ٲóɹ�...\n�ٲ����ݣ�"<<dealresult<<endi;
		}
		else
		{
			theJSLog<<"δ֪ret="<<ret<<"	�ٲ����ݣ�"<<dealresult<<endw;
			dr_AbortIDX();
		}
	
	return ret;
 }

bool CSql::CheckTriggerFile()
{
	int ret = 0;
	if(access(m_triggerFile.c_str(),F_OK) != 0)	return false;

	theJSLog<< "��鵽trigger�ļ�����ɾ��"<< m_triggerFile <<endi;

	ret = remove(m_triggerFile.c_str());	
	if(ret) 
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"ɾ��trigger�ļ�[%s]ʧ��: %s",m_triggerFile,strerror(errno));
		theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
		return false;
	}

	return true;
}
*/

int main(int argc,char** argv)
{
	cout<<"********************************************** "<<endl;
	cout<<"*    China Telecom. Telephone Network          "<<endl;
	cout<<"*    InterNational Account Settle System       "<<endl;
	cout<<"*                                              "<<endl;
	cout<<"*           jsextSql							  "<<endl;
	cout<<"*           sys.GJZW.Version 1.0	              "<<endl;
	cout<<"*     last update time : 2013-12-16 by  hed	  "<<endl;
	cout<<"********************************************** "<<endl;


	CSql fm ;
   	if( !fm.init( argc, argv ) )
	{
		 return -1;
	}
	while(1)
	{
		theJSLog.reSetLog();
		fm.run();
		sleep(5);
	}

   return 0;
}

