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
	//memset(input_path,0,sizeof(input_path));
	//memset(out_path,0,sizeof(out_path));
	//memset(m_szSourceID,0,sizeof(m_szSourceID));
	//memset(outFileName,0,sizeof(outFileName));
	memset(currTime,0,sizeof(currTime));
    memset(sqlFile,0,sizeof(sqlFile));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(sql,0,sizeof(sql));
	//memset(currTime,0,sizeof(currTime));
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
	
	PS_Process::setSignalPrc(); 

	//cout<<"��ˮ��ID:"<<getFlowID()<<"   ģ��ID:"<<getModuleID()<<endl;

	 // �Ӻ��Ĳ��������ȡ��־��·��������
	 char sParamName[256],szLogPath[PATH_NAME_LEN+1],szLogLevel[10],sql_path[1024];
	 CString sKeyVal;
	 sprintf(sParamName, "log.path");
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(szLogPath,0,sizeof(szLogPath));
		strcpy(szLogPath,(const char*)sKeyVal);

	 }
	 else
	 {	
		cout<<"���ں��Ĳ�����������־��·��"<<endl;
		return false ;
	 }	 
	 sprintf(sParamName, "log.level");
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(szLogLevel,0,sizeof(szLogLevel));
		strcpy(szLogLevel,(const char*)sKeyVal);

	 }
	 else
	 {	
		cout<<"���ں��Ĳ�����������־�ļ���"<<endl;
		return false ;
	 }
	
	 //sprintf(sParamName, "file.aduit.outpath");			//����ļ����Ŀ¼
	 //if(param_cfg.bGetMem(sParamName, sKeyVal))
	 //{
		//memset(szLogPath,0,sizeof(szLogPath));
		//strcpy(out_path,(const char*)sKeyVal);

	 //}
	 //else
	 //{	
	//	cout<<"���ں��Ĳ������������ģ������·��"<<endl;
	//	return false ;
	 //}	 
	 	
	//�ж���־Ŀ¼��sqlĿ¼�Ƿ����
	 DIR *dirptr = NULL; 
	 if((dirptr=opendir(szLogPath)) == NULL)
	 {
		cout<<"��־Ŀ¼:"<<szLogPath<<"��ʧ��"<<endl;	
		return false ;
	 }else closedir(dirptr);

	 //if((dirptr=opendir(out_path)) == NULL)
	 //{
	 //	cout<<"���Ŀ¼"<<out_path<<"��ʧ��"<<endl;
	 //	return false ;

	 //}else closedir(dirptr);

	 //completeDir(out_path);

	//��ʼ���ڴ���־�ӿ�
	bool bb = initializeLog(argc,argv,false);  //�Ƿ����ģʽ
	if(!bb)
	{
			//cout<<"��ʼ���ڴ���־�ӿ�ʧ��"<<endl;
			return false;
	}
	
	theJSLog.setLog(szLogPath, atoi(szLogLevel),"2013" , "ADUIT", 001);	//�ļ���־�ӿڣ��������ڴ���־�ӿ�
	
	theJSLog<<"	��־·����"<<szLogPath<<" ��־����"<<endi;
   
	
	try
	{	
		if(!(dbConnect(conn)))
		{
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
		sprintf(erro_msg,"��ʼ��ʱ���ݿ��ѯ�쳣��%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
		return false;
	}

	//�жϸ�����˵��ļ������Ŀ¼
	for(map< string,AduitEnv >::const_iterator it = fileNameMap.begin();it !=  fileNameMap.end();++it)
    {
		if((dirptr=opendir((it->second).out_path)) == NULL)
		{
			//cout<<"�������Դ��־:"<<it->first<<"������ļ�Ŀ¼["<<(it->second).out_path<<"]��ʧ��"<<endl;
			sprintf(erro_msg,"�������Դ��־[%s]�ĵ�����ļ�·��[%s]������",it->first,(it->second).out_path);
			theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����

			return false ;
		}else closedir(dirptr);

	}


   //���Ĳ�����ȡ
   //CString sKeyVal;
   //param_cfg.bGetMem("memory.MT_DSPCH.sem_key_value", sKeyVal);
   //cout<<"��ȡ�ĺ��Ĳ���ֵ:"<<sKeyVal<<endl;

   //����д��־
   //writeLog(0,"����д��־");

   //conn.close();

   //cerr<<"��ʼ����ϣ�\n";
   
   theJSLog<<"��ʼ�����"<<endi;

   return true ;
}



//���ݿ�ĸ��²��������ڲ����쳣sql ��������
int FileOutAduit::updateDB(char* sql)
{
	try
	{
		//cout<<"SQL:"<<sql<<endl;
		Statement stmt = conn.createStatement();
		stmt.setSQLString(sql);
		stmt.execute();
		stmt.close();

	}
	catch(util_1_0::db::SQLException e)
	{ 
		sprintf(erro_msg,"updateDB ���ݿ����%s",e.what());
		//memset(erro_msg,sizeof(erro_msg));
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
		return -1;
	}

	return 0;
}


//�鿴sql�ļ����Ƿ������ݣ�������ִ��
int FileOutAduit::scanSQLFile()
{
	char szBuff[1024];
	ifstream in(sqlFile,ios::in) ;
	if(!in)
	{
		//cout<<"�ļ�: "<<sqlFile<<"�򿪴���"<<endl;
		sprintf(erro_msg,"scanSQLFile �ļ�%s�򿪳���",sqlFile);
		theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��
		return -1;
	}

	memset(szBuff,0,sizeof(szBuff));
	while(in.getline(szBuff,sizeof(szBuff)))   
	{		
			//cout<<"������SQL�ļ�"<<szBuff<<endl;
			updateDB(szBuff);   //ִ���ļ��е�sql���
			memset(szBuff,0,sizeof(szBuff));
			
	}
	
	in.close();

	in.open(sqlFile,ios::trunc);
	in.close();

	return 0;

}

//ɨ������ļ��ǼǱ� �г����մ��͵�ERR�ļ����嵥����Ϊ�˶�����
void FileOutAduit::run()
{
	cout<<"�������У�����������"<<endl;

	if(gbExitSig)
	{
		if(gbExitSig) theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "Ӧ�ó����յ��˳��ź�");
		PS_Process::prcExit();
		return;
	}

	try
	{
		if(!(dbConnect(conn)))
		{
			//cout<<"�������ݿ�ʧ�� connect error."<<endl;
			sprintf(erro_msg,"Ontask() �������ݿ�ʧ�� connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
			return  ;
		}

		sprintf(sql,"select fileName,source_id from d_out_file_reg where file_type = 'E' and state = 'W' ");				
		Statement stmt = conn.createStatement();
		stmt.setSQLString(sql);
		stmt.execute();
		string filename ,source;
		while(stmt>>filename>>source)
		{	
			//fileList.push_back(str);
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
								updateDB(sql);

								break ;
							}

					}
			}
			
			filename = "";
			source = "";
		}

		stmt.close();
		char tmpTime[8+1],outFile[1024];
		//����Ӧ������Դ�����ļ�
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
			sprintf(m_szFileName,"ACC_%s_D_%s_AUD",tmpTime,it3->first);	 //д�˶��ļ�
			
			memset(outFile,0,sizeof(outFile));
			strcpy(outFile,(it3->second).out_path);
			
			strcat(outFile,m_szFileName);

			theJSLog<<"���ɺ˶��ļ�"<<m_szFileName<<endi;
			ofstream out(outFile);
			if(!out)
			{
				sprintf(erro_msg,"writeFile �ļ�%s�򿪳���",m_szFileName);
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

		sprintf(sql,"update d_out_file_reg set state = 'Y' , deal_time = '%s' where state = 'H'",currTime);
		updateDB(sql);

		conn.close();

	}
	catch (util_1_0::db::SQLException e)
	{
		sprintf(erro_msg,"run() ���ݿ�����쳣%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
	}	
	catch (jsexcp::CException &e) 
	{	
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
	cout<<"*     created time :  2013-07-22 by  hed	    * "<<endl;
	cout<<"********************************************** "<<endl;


	FileOutAduit fm ;


	if( !fm.init( argc, argv ) )
	{
		 return -1;
	}
        
	while(1)
	{
		fm.run();
		sleep(60);
	}

   return 0;
}


