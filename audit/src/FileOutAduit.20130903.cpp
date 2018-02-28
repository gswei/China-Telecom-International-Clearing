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

SGW_RTInfo	rtinfo;

FileOutAduit::FileOutAduit()
{  

	memset(m_szFileName,0,sizeof(m_szFileName));
	memset(currTime,0,sizeof(currTime));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(sql,0,sizeof(sql));
}


FileOutAduit::~FileOutAduit()
{
	

}

//ģ���ʼ������
bool FileOutAduit::init(int argc,char** argv)
{
   
    if(!PS_Process::init(argc,argv))
    {
      return false;
    }
	
	//PS_Process::setSignalPrc(); 

	if(!(rtinfo.connect()))
	{
		return false;
	}
	
	rtinfo.getDBSysMode(petri_status);
	cout<<"petri status:"<<petri_status<<endl;


	//��ʼ���ڴ���־�ӿ�
	bool bb = initializeLog(argc,argv,false);  //�Ƿ����ģʽ
	if(!bb)
	{
			//cout<<"��ʼ���ڴ���־�ӿ�ʧ��"<<endl;
			return false;
	}
	
	theJSLog.setLog(szLogPath,szLogLevel,"HED" , "ADUIT", 001);	//�ļ���־�ӿڣ��������ڴ���־�ӿ�
	
	theJSLog<<"��־·��:"<<szLogPath<<szLogPath<<" ��־����:"<<szLogLevel<<endi;
  
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

   theJSLog<<"��ʼ�����"<<endi;

   return true ;
}



//�����ύsql,��֤һ������������
int FileOutAduit::updateDB()
{	
	int ret = 0;
	Statement stmt;
    try
    {	
		stmt = conn.createStatement();
		for(int i =0;i<vsql.size();i++)
		{		
			stmt.setSQLString(vsql[i]);
			ret = stmt.execute();
		}
		stmt.close();

	}
	catch(util_1_0::db::SQLException e)
	{ 
		stmt.close();
		vsql.clear();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"updateDB ���ݿ����%s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
		return -1;
	}

	vsql.clear();
	
	return ret ;
}

//ɨ������ļ��ǼǱ� �г����մ��͵�ERR�ļ����嵥����Ϊ�˶�����,������֤ÿ��ִֻ��һ��,����˶Ե�������ļ���,��ô�ж�?
void FileOutAduit::run()
{
	int ret = 0;

	if(gbExitSig)
	{
		if(gbExitSig) theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "Ӧ�ó����յ��˳��ź�");
		PS_Process::prcExit();
		return;
	}

	try
	{
		//�ж����ݿ�״̬
		rtinfo.getDBSysMode(petri_status);
		//cout<<"״ֵ̬:"<<petri_status<<endl;

		if(petri_status == DB_STATUS_OFFLINE)	return ;
		
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
		
		if(cnt == 0)	return ;
		
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
								vsql.push_back(sql);

								break ;
							}
					}
			}
			
			filename = "";
			source = "";
		}

		stmt.close();
		
		char tmpTime[8+1],outFile[1024],tmp[1024];
		vector<string> fileList;	

		//����Ӧ������Դ�����ļ� ��������Դ��־,�ļ���¼��Ŀ,�ļ���С
		for(map< string,AduitEnv >::const_iterator it3=fileNameMap.begin() ;it3!=fileNameMap.end();++it3)
		{
			vector<string> arrayFile = (it3->second).arrayFile;	

			if(arrayFile.size() == 0)
			{		
				 if((it3->second).null_out_flag == 'N')	continue ;		
			}
			
			getCurTime(currTime);
			memset(tmpTime,0,sizeof(tmpTime));
			strncpy(tmpTime,currTime,8);
			sprintf(m_szFileName,"ACCT_%s_D_%s_AUD",tmpTime,it3->first);	 //д�˶��ļ�

			memset(outFile,0,sizeof(outFile));
			strcpy(outFile,(it3->second).out_path);
			
			strcat(outFile,m_szFileName);

			fileList.push_back(outFile);		//���������ļ���+·��
			
			strcat(outFile,".tmp");

			theJSLog<<"���ɺ˶��ļ�:"<<m_szFileName<<endi;
			ofstream out(outFile);
			if(!out)
			{
				vsql.clear();
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"run() �ļ�%s�򿪳���",m_szFileName);
				theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��
				return ;
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
		vsql.push_back(sql);
		
		ret = updateDB();

		conn.close();	

		if(ret == -1)
		{
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

	return ;
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


int main(int argc,char** argv)
{
	cout<<"********************************************** "<<endl;
	cout<<"*    GuangDong Telecom. Telephone Network    * "<<endl;
	cout<<"*       Centralized Settlement System        * "<<endl;
	cout<<"*                                            * "<<endl;
	cout<<"*            fileOutAduit                    * "<<endl;
	cout<<"*              Version 1.0	                * "<<endl;
	cout<<"*     created time :  2013-07-20 by  hed	    * "<<endl;
	cout<<"*     last updaye time :  2013-08-30 by  hed	* "<<endl;
	cout<<"********************************************** "<<endl;


	FileOutAduit fm ;

	if( !fm.init( argc, argv ) )
	{
		 return -1;
	}
        
	while(1)
	{
		fm.run();
		sleep(600);
	}

   return 0;
}


