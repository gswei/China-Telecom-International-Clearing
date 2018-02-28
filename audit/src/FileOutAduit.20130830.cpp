/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-07-22
File:			 FileOutAduit.cpp
Description:	 �ļ�������ģ��
History:
<table>
revision	author            date                description
--------    ------            ----                -----------
</table>
**************************************************************************/
#include <dirent.h>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>  //stat()��������ѯ�ļ���Ϣ
#include<unistd.h>     //��ȡ��ǰ��������Ŀ¼
#include<iostream>
#include<fstream>
#include "FileOutAduit.h"

#include "CF_Common.h"
#include "CF_CLogger.h"
CLog theJSLog;


FileOutAduit::FileOutAduit()
{  
	memset(m_szFileName,0,sizeof(m_szFileName));
	memset(currTime,0,sizeof(currTime));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(sql,0,sizeof(sql));
}


FileOutAduit::~FileOutAduit()
{
	if(m_enable) 
	{
		int ret = dr_ReleasePlatform();
		if(ret != 0)
		{
			char tmp[100] = {0};
			snprintf(tmp, sizeof(tmp), "�ͷ�����ƽ̨ʧ��,����ֵ=%d", ret);
			theJSLog<<tmp<<endi;
		}
	}

}

//ģ���ʼ������
bool FileOutAduit::init(int argc,char** argv)
{
   
    if(!PS_Process::init(argc,argv))
    {
      return false;
    }
	
	//PS_Process::setSignalPrc(); 

	//��ʼ���ڴ���־�ӿ�
	bool bb = initializeLog(argc,argv,false);  //�Ƿ����ģʽ
	if(!bb)
	{
			//cout<<"��ʼ���ڴ���־�ӿ�ʧ��"<<endl;
			return false;
	}
	
	theJSLog.setLog(szLogPath,szLogLevel,"HED" , "ADUIT", 001);	//�ļ���־�ӿڣ��������ڴ���־�ӿ�
	
	theJSLog<<"	��־·��:"<<szLogPath<<szLogPath<<" ��־����:"<<szLogLevel<<endi;
  
	try
	{	
		if(!(dbConnect(conn)))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"init()  �������ݿ�ʧ�� connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
			return false ;
		}

		//��������Դ��Ϣ������map��ȥ
		string sql = "select aduit_flag,source_array,null_out_flag,out_path from c_source_audit_env ";
		Statement stmt = conn.createStatement();
		stmt.setSQLString(sql);
		stmt.execute();
		//***********************************************************************************/
		string str ;
		AduitEnv audit ;
		char source_array[256];
		memset(source_array,0,sizeof(source_array));

		while(stmt>>str>>source_array>>audit.null_out_flag>>audit.out_path)
		{
			 vector<string> array ,array2;
			 splitString(source_array,",",array,true);
			 audit.arrayFile = array2 ;
			 completeDir(audit.out_path);

			 sourceMap.insert(map< string,vector<string>  >::value_type(str,array));
			 fileNameMap.insert(map< string, AduitEnv>::value_type(str,audit));

			 memset(source_array,0,sizeof(source_array));
			 str = "";
		}

		stmt.close();

		conn.close();	
	}
	catch(util_1_0::db::SQLException e)
	{ 
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"��ʼ��ʱ���ݿ��ѯ�쳣��%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
		return false;
	}

	DIR *dirptr = NULL; 

	//�жϸ�����˵��ļ������Ŀ¼
	for(map< string,AduitEnv >::const_iterator it = fileNameMap.begin();it !=  fileNameMap.end();++it)
    {
		if((dirptr=opendir((it->second).out_path)) == NULL)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"�������Դ��־[%s]�ĵ�����ļ�·��[%s]������",it->first,(it->second).out_path);
			theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����

			return false ;
		}else closedir(dirptr);

	}
   
   if(!drInit)		return false;

   theJSLog<<"��ʼ�����"<<endi;

   return true ;
}

//ɨ������ļ��ǼǱ� �г����մ��͵�ERR�ļ����嵥����Ϊ�˶�����
void FileOutAduit::run()
{
	//cout<<"�������У�����������"<<endl;
	
	int ret = 0;
	if(gbExitSig)
	{
		if(gbExitSig) theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "Ӧ�ó����յ��˳��ź�");
		PS_Process::prcExit();
		return;
	}

	try
	{
		if(drStatus == 1)
		{
			isWriteSQLFileByTime();		
			//���trigger�����ļ��Ƿ����
			if(!CheckTriggerFile())
			{
				sleep(1);
				return ;
			}
		}
		else
		{
			isWriteSQLFile();				//�Ƿ��ύsql�ļ�

		}

		if(!(dbConnect(conn)))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"Ontask() �������ݿ�ʧ�� connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
			return  ;
		}
		
		int cnt = 0;
		Statement stmt = conn.createStatement();
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(1) from d_out_file_reg where file_type = 'E' and state = 'W' ");
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>cnt;
		
		//if(cnt == 0)	return 0;
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select fileName,source_id from d_out_file_reg where file_type = 'E' and state = 'W' ");		
		stmt.setSQLString(sql);
		stmt.execute();
		string filename ,source;
		while(stmt>>filename>>source)
		{	
			for(map< string,vector<string> >::const_iterator it=sourceMap.begin();it!=sourceMap.end();++it)
			{
					vector<string> array = it->second;
					for(int i = 0;i<array.size();i++)
					{
							if(array[i] == source)		//�ҵ�����Դ�����Ľ�������
							{
								map< string,AduitEnv >::iterator it2 = fileNameMap.find(it->first);						
							    (it2->second).arrayFile.push_back(filename);
								
								sprintf(sql,"update d_out_file_reg set state = 'H' where file_type = 'E' and source_id = '%s' and filename = '%s'",source,filename);
								writeSQL(sql);

								break ;
							}
					}
			}
			
			filename = "";
			source = "";
		}

		stmt.close();
		conn.close();	

		char tmpTime[8+1],outFile[1024];
		

		memset(m_SerialString,0,sizeof(m_SerialString));
		memset(tmpTime,0,sizeof(tmpTime));

		//ͬ������: ��¼����,����ʱ��,sqlFile
		if(drStatus ==1)
		{	
			ret = drVarGetSet(m_SerialString);
			if(ret)
			{
				theJSLog<<"��ϵͳͬ��ʧ��...."<<endi;
				return ;
			}
			
			//��ȡͬ������
			vector<string> data;		
			splitString(m_SerialString,";",data,true,true);  //���͵��ַ�������ԴID,�ļ���,sqlFile�ļ���
			
			isWriteSQLFileByMain(data[2].c_str());	//��ϵͳ��sqlFile��ͨ����ϵͳ��������
			
			strcpy(tmpTime,data[1].c_str());
		}
		else
		{
			getCurTime(currTime);	
			memset(tmpTime,0,sizeof(tmpTime));
			strncpy(tmpTime,currTime,8);

			sprintf(m_SerialString,"%d;%s;%s",cnt,currTime,sqlFile);
			ret = drVarGetSet(m_SerialString);
			if(ret)
			{
				theJSLog<<"��ϵͳͬ��ʧ��...."<<endi;
				return ;
			}
		}
		
		memset(m_AuditMsg,0,sizeof(m_AuditMsg));
		
		vector<string> fileList;	

		//����Ӧ������Դ�����ļ� ��������Դ��־,�ļ���¼��Ŀ,�ļ���С
		for(map< string,AduitEnv >::const_iterator it3=fileNameMap.begin() ;it3!=fileNameMap.end();++it3)
		{
			vector<string> arrayFile = (it3->second).arrayFile;
			sprintf(m_szFileName,"ACCT_%s_D_%s_AUD",tmpTime,it3->first);	 //д�˶��ļ�


			sprintf(m_AuditMsg,"%s%s,%d,%c|",m_AuditMsg,m_szFileName,arrayFile.size(),(it3->second).null_out_flag);

			if(arrayFile.size() == 0)
			{		
				 if((it3->second).null_out_flag == 'N')	continue ;		
			}
					
			memset(outFile,0,sizeof(outFile));
			strcpy(outFile,(it3->second).out_path);
			
			strcat(outFile,m_szFileName);

			fileList.push_back(outFile);		//���������ļ���+·��
			
			strcat(outFile,".tmp");

			theJSLog<<"���ɺ˶��ļ�"<<m_szFileName<<endi;
			ofstream out(outFile);
			if(!out)
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"writeFile �ļ�%s�򿪳���",m_szFileName);
				theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��
				//return ;
			}
		
			//д�ļ�ͷ
			out<<10<<","<<currTime<<","<<tmpTime<<","<<arrayFile.size()<<endl;
			for(int k = 0;k<arrayFile.size();k++)
			{		
				out<<arrayFile[k]<<"\n";					
			}		
		
			out<<"90"<<endl;

			out.close();
			
		}

		getCurTime(currTime);
		sprintf(sql,"update d_out_file_reg set state = 'Y' , deal_time = '%s' where state = 'H'",currTime);
		writeSQL(sql);
		
		//�˶���Ϣ,
		char tmp[1024];
		if(!IsAuditSuccess(m_AuditMsg))				//�ٲ�ʧ��,�ع����ݿ�,ɾ����ʱ�ļ�
		{
			rollBackSQL();
			for(int i = 0;i<fileList.size();i++)
			{
					memset(tmp,0,sizeof(tmp));
					strcpy(tmp,fileList[i].c_str());
					strcat(tmp,".tmp");
					if(remove(tmp))
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"ɾ����ʱ�ļ�[%s]ʧ��: %s",tmp,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
					}
			}
			return ;
		}
		
		theJSLog<<"�ύsql��䵽�ļ�..."<<endi;
		commitSQLFile();

		//��ʱ�ļ�д��ʽ�ļ�
		theJSLog<<"����ʱ�ļ���Ϊ��ʽ�ļ�..."<<endi;

		for(int i = 0;i<fileList.size();i++)
		{
				strcpy(tmp,fileList[i].c_str());
				strcat(tmp,".tmp");
				if(rename(tmp,fileList[i].c_str()))
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"��ʱ�ļ�[%s]������Ϊ��ʽ�ļ�ʧ��: %s",tmp,strerror(errno));
					theJSLog.writeLog(LOG_CODE_FILE_RENAME_ERR,erro_msg);		//�ƶ��ļ�ʧ��
				}
		}
		
		PS_Process::prcExit();
	}
	catch (util_1_0::db::SQLException e)
	{
		rollBackSQL();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() ���ݿ�����쳣%s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
	}	
	catch (jsexcp::CException &e) 
	{	
		rollBackSQL();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		
	}
}

//д�ļ�
int FileOutAduit::writeFile()
{
		int ret = 0;
		char tmp[1024],source_flag[10];
		memset(tmp,0,sizeof(tmp));
		memset(source_flag,0,sizeof(source_flag));
		
		getCurTime(currTime);

		//�ļ���
		strncpy(tmp,currTime,8);
		sprintf(m_szFileName,"ACC_%s_D_%s_AUD",tmp,source_flag);
		getCurTime(currTime);

		ofstream out(m_szFileName);
		if(!out)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"writeFile �ļ�%s�򿪳���",m_szFileName);
			theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��
			return -1;
		}
		
		//д�ļ�ͷ
		out<<10<<","<<currTime<<","<<tmp<<","<<fileList.size()<<endl;
		for(int k = 0;k<fileList.size();k++)
		{		
			out<<fileList[k]<<"\n";					
		}		
		
		out<<"90"<<endl;

		out.close();		
		return ret ;
}


//���ֳ�ʼ��
bool FileOutAduit::drInit()
{
		//����ģ������ʵ��ID
		char tmp[32];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%ld",getPrcID());

		theJSLog << "��ʼ������ƽ̨,ģ����:"<< module_name<<" ʵ����:"<<tmp<<endi;

		int ret = dr_InitPlatform(module_name,tmp);
		if(ret != 0)
		{
			theJSLog << "����ƽ̨��ʼ��ʧ��,����ֵ=" << ret<<endi;
			return false;
		}
		else
		{
			theJSLog<<"dr_InitPlatform ok."<<endi;
		}

		m_enable = true;

		drStatus = _dr_GetSystemState();	//��ȡ����ϵͳ״̬
		if(drStatus < 0)
		{
			theJSLog<<"��ȡ����ƽ̨״̬����: ����ֵ="<<drStatus<<endi;
			return false;
		}
		
		if(drStatus == 0)		theJSLog<<"��ǰϵͳ����Ϊ��ϵͳ"<<endi;
		else if(drStatus == 1)	theJSLog<<"��ǰϵͳ����Ϊ��ϵͳ"<<endi;
		else if(drStatus == 2)	theJSLog<<"��ǰϵͳ���÷�����ϵͳ"<<endi;

		return true;
}

//��ϵͳ����ͬ������,��ϵͳ��ȡͬ������
int FileOutAduit::drVarGetSet(char* serialString)
{
		int ret ;
		char tmpVar[5000] = {0};

		//�������ƽ̨���л���
		ret = dr_CheckSwitchLock();   
		if(ret != 0)  
		{  
			theJSLog<<"��������л���ʧ��,����ֵ="<<ret<<endi;
			return -1;  
		} 
		//��ʼ��index  
		ret = dr_InitIDX();  
		if(ret != 0)  
		{  
			theJSLog<<"��ʼ��indexʧ��,����ֵ=" <<ret<<endi;
			dr_AbortIDX();
			return -1;  
		}

		snprintf(tmpVar, sizeof(tmpVar), "%s", serialString);
		//��ϵͳ��Ҫͬ����index ����ֵ�ԡ�д������ƽ̨ά����index �ļ���
		//��ϵͳ���øú����Ľ���ǣ�var��ú���ϵͳһ�������������ֵ��	SYNC_SINGLE��ʾע�ᵥһ���������
		ret = dr_SyncIdxVar("serialString", tmpVar, SYNC_SINGLE);		
		if (ret != 0)
		{
			theJSLog<<"�����д�ʱʧ�ܣ���������["<<serialString<<"]"<<endi;
			dr_AbortIDX();
			return -1;
		}
		//serialString = tmpVar;			//ͬ�������ַ���,��ϵͳ�Ǹ�ֵ����ϵͳ��ȡֵ
		strcpy(serialString,tmpVar);
		//m_AuditMsg = tmpVar;			//Ҫ�ٲõ��ַ���

		// <5> ����ʵ����  ������ϵͳע���IDX��ģ����ò�����
		//��ϵͳ��index manager���IDX��������󣬰�ʹ�øú���ע������������Ϊģ��ĵ��ò���trigger��Ӧ�Ľ���
		snprintf(tmpVar, sizeof(tmpVar), "%d", getPrcID());
		ret = dr_SyncIdxVar("@@ARG", tmpVar,SYNC_SINGLE);  
		if(ret !=0)
		{
			theJSLog<<"����ʵ����ʧ�ܣ�"<<tmpVar<<endi;
			dr_AbortIDX();  
			return -1;
		}
		
		
		// <6> Ԥ�ύindex  �˹ؼ������ڽ�ƽ̨��ǰ�ڴ��е��������д�����
		ret = dr_SyncIdxVar("@@FLUSH","SUCCESS",SYNC_SINGLE);  
		if (ret != 0 )
		{
			theJSLog<<"Ԥ�ύindexʧ��"<<endi;
			dr_AbortIDX();
			return -1;
		}
		
		
		// <7> �ύindex  	�ύIndex����index�ļ���������ɱ�־
		ret = dr_CommitIDX();  
		if(ret != 0)  
		{  
			theJSLog<<"�ύindexʧ��,����ֵ="<<ret<<endi;
			dr_AbortIDX();  
			return -1;  
		}

		//��ϵͳ����Ŀ¼
		//if(!m_syncDr.isMaster())thelog<<"��ϵͳSerialString��"<<m_SerialString<<endi;

		theJSLog<<"���ε�ͬ����serialString:"<<serialString<<endi;//for test

		return ret;

}

//�ٲ��ַ���
 bool FileOutAduit::IsAuditSuccess(const char* dealresult)
 {
		int auitStatus = 0, ret = 0;
		ret = dr_Audit(dealresult);
		if(2 == ret )
		{
			theJSLog << "�����ٲ�ʧ��,���:" << ret <<"���ˣ�"<<dealresult<< endi;
			dr_AbortIDX();
			return false;
		}
		else if (3 == ret)
		{
			theJSLog<<"�����ٲó�ʱ..."<<endi;
			dr_AbortIDX();
			return false;
		}
		else if(1 == ret)
		{
			ret = dr_CommitSuccess();
			if(ret != 0)
			{
				theJSLog << "ҵ��ȫ���ύʧ��(����ƽ̨)" << endi;
				dr_AbortIDX();
				return false;
			}
			theJSLog<<"ret = "<<ret<<"�ٲóɹ�...\n�ٲ����ݣ�"<<dealresult<<endi;
			return true;
		}
		else
		{
			theJSLog<<"δ֪ret="<<ret<<"	�ٲ����ݣ�"<<dealresult<<endi;
			dr_AbortIDX();
			return false;
		}
	
	return true;
 }

bool FileOutAduit::CheckTriggerFile()
{
	int ret = 0;
	if(access(m_triggerFile.c_str(),F_OK) != 0)	return false;

	theJSLog<< "��鵽trigger�ļ�,��ɾ��"<< m_triggerFile <<endl;

	ret = remove(m_triggerFile.c_str());	
	if(ret) 
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"ɾ��trigger�ļ�[%s]ʧ��: %s",m_triggerFile,strerror(errno));
		theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
	}
}

int main(int argc,char** argv)
{
	cout<<"********************************************** "<<endl;
	cout<<"*    GuangDong Telecom. Telephone Network    * "<<endl;
	cout<<"*       Centralized Settlement System        * "<<endl;
	cout<<"*                                            * "<<endl;
	cout<<"*            fileOutAduit                    * "<<endl;
	cout<<"*              Version 1.0	                * "<<endl;
	cout<<"*     created time :  2013-07-20 by  hed	    * "<<endl;
	cout<<"*     last updaye time :  2013-08-27 by  hed	* "<<endl;
	cout<<"********************************************** "<<endl;


	FileOutAduit fm ;


	if( !fm.init( argc, argv ) )
	{
		 return -1;
	}
        
	//while(1)
	//{
		fm.run();
		//sleep(60);
	//}

   return 0;
}


