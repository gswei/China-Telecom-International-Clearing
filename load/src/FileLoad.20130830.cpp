/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-06-19
File:			 FileLoad.cpp
Description:	 ��д�����鴦��
History:
<table>
revision	author            date                description
--------    ------            ----                -----------
v1.0		hed          2013-06-19

</table>
**************************************************************************/

//#include <io.h> // _findfirst() _findnext()
//#include <string.h> //strcat()
//#include <stdio.h>//gets() puts()
//#include<direct.h>
#include <dirent.h> //_chdir() _getcwd() ��ȡ�ļ�Ŀ¼�ȣ�����ļ�����Ϣ
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>  //stat()��������ѯ�ļ���Ϣ
#include<unistd.h>     //��ȡ��ǰ��������Ŀ¼
#include<iostream>
#include<fstream>
#include <sstream>
#include "FileLoad.h"

#include "CF_Common.h"
#include "CF_CLogger.h"

//MdrStatus syn_status;//����״̬
//string auditkey;
//string sessionid;
//SGW_RTInfo	rtinfo;

FileLoad::FileLoad()
{  
	//��ʼ����������Ϣ
	//data_block.header.blockSize = ����¼��*ÿ����¼����+����ͷ�� ���С�������ļ� �����ɶ�ȡ ���ж��ٿ飬ÿ���ж��ټ�¼������¼����
	//ÿ��������20�飬ÿ��500����¼��ÿ����¼4KB

	file_num  = 0;
	source_file_num = 0;
	//petri_status = 0;
	m_enable = false;

	split_num = 0;
	bak_flag = 'N';  //Ĭ�ϲ�����
	record_num = 0;

    memset(m_szSrcGrpID,0,sizeof(m_szSrcGrpID));
	memset(m_szService,0,sizeof(m_szService));
	memset(mServCatId,0,sizeof(mServCatId));
	memset(m_szSourceID,0,sizeof(m_szSourceID));
	memset(m_szOutTypeId,0,sizeof(m_szOutTypeId));

	memset(input_path,0,sizeof(input_path));
	memset(erro_path,0,sizeof(erro_path));
	memset(bak_path,0,sizeof(bak_path));

	memset(m_szFileName,0,sizeof(m_szFileName));

	memset(sql,0,sizeof(sql));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(currTime,0,sizeof(currTime));
  
}


//ģ���ʼ������
bool FileLoad::init(int argc,char** argv)
{
   
   if(!PS_BillProcess::init(argc,argv))
   {
      return false;
   }
		
	//��ȡpetri��״̬,��ϵͳΪֻ��̬ʱ,���ݿ���²������д�ļ�
	
	//if(!(rtinfo.connect()))
	//{
	//	return false;
	//}
	//short status;
	//rtinfo.getSysMode(petri_status);
	//cout<<"petri status:"<<status<<endl;

    //syn_status = syncInit(); 

	//*********2013-06-22 ��ȡ���ݿ��������Ϣ��������Դ�飬����Ŀ¼ 2013-03-11 add by hed*********************************************/
	
	if(!(dbConnect(conn)))
	{
		cout<<"��ʼ�����ݿ� connect error."<<endl;
		return false ;
	}

	int flow_id = getFlowID();
	int module_id = getModuleID(); 
	//cout<<"��ˮ��ID:"<<flow_id<<"   ģ��ID:"<<module_id<<endl;

	try{

		string sql = "select source_group from TP_BILLING_LINE  where billing_line_id = :1";
		Statement stmt = conn.createStatement();
		stmt.setSQLString(sql);
		stmt<<flow_id;
		stmt.execute();
		if(!(stmt>>m_szSrcGrpID))
		{
			cout<<"����tp_billing_line������������Դ��"<<endl;
			return false ;
		}

		sql = "select ext_param from TP_PROCESS where billing_line_id = :1 and module_id = :2";
		stmt.setSQLString(sql);
		stmt<<flow_id<<module_id;
		stmt.execute();
		if(!(stmt>>m_szService))
		{
			cout<<"����tp_process��������service"<<endl;
			return false ;
		}

		sql = "select c.input_path from C_SOURCE_GROUP_DEFINE a,C_SERVICE_FLOW b,C_SERVICE_INTERFACE c where a.source_group=:1" 
                    "and a.workflow_id = b.workflow_id and b.service=:2 and b.input_id = c.interface_id ";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID<<m_szService;
		stmt.execute();
		if(!(stmt>>input_path))
		{
			cout<<"���м���ģ�������ļ����·��input_pathû������"<<endl;
			return false ;
		}
		completeDir(input_path);
	
		//2013-07-01 �����жϱ��ݱ�־��·��
		sql = "select var_value from c_process_env where varname = 'LOAD_FILE_BAK_FLAG' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID<<m_szService;
		stmt.execute();
		if(!(stmt>>bak_flag))
		{
			bak_flag = 'N';			 
		}
		if(bak_flag == 'Y')
		{
			sql = "select var_value from c_process_env where varname = 'LOAD_FILE_BAK_DIR' and source_group=:1 and service=:2 ";
			stmt.setSQLString(sql);
			stmt<<m_szSrcGrpID<<m_szService;
			stmt.execute();
			if(!(stmt>>bak_path))
			{
					cout<<"���ڱ�c_process_env�����ü���ģ��ı���Ŀ¼,LOAD_FILE_BAK_DIR"<<endl;
					return false;
			}
			completeDir(bak_path);
		}
		
		sql = "select var_value from c_process_env where varname = 'LOAD_FILE_ERR_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID<<m_szService;
		stmt.execute();
		if(!(stmt>>erro_path))
		{
				cout<<"���ڱ�c_process_env�����ü���ģ��Ĵ���·��,LOAD_FILE_ERR_DIR"<<endl;
				return false;
		}	
		completeDir(erro_path);

		stmt.close();
		
	   }catch(SQLException  e)
		{
			cout<<"��ʼ��ʱ���ݿ��ѯ�쳣:"<<e.what()<<endl;
			return false ;
		}
	
	
	char sParamName[256];
	CString sKeyVal;
	memset(sParamName,0,sizeof(sParamName));
	sprintf(sParamName, "billing_line.%d.record_num", getFlowID());		//��ȡ�����������¼��
	param_cfg.bGetMem(sParamName, sKeyVal) ;
	maxRecord_num=sKeyVal.toInteger();
	
	//2013-08-16������������ÿ��ɨ������ԴĿ¼����ָ�������ļ�������¸�����Դ
	sprintf(sParamName, "business.source.file.%d.num",getFlowID());
	if(param_cfg.bGetMem(sParamName, sKeyVal))
	{
		source_file_num = sKeyVal.toInteger();
	}
	else
	{	
		cout<<"���������ļ���������ˮ��["<<flow_id<<"]������Դÿ��ɨ���ļ��ĸ���,������:"<<sParamName<<endl;
		return false ;
	}	


	bool bb = initializeLog(argc,argv,false);  //�Ƿ����ģʽ
	if(!bb)
	{
			//theJSLog<<"��ʼ���ڴ���־�ӿ�ʧ��"<<endi;
			return false;
	}

	//writelog(90155,"���Լ���ģ��д��־--ҵ��ӿ�");  //���:debug|����:2|
	//PS_Process::writeLog(90156,"���Լ���ģ��д��־--��ܽӿ�");  //���:error|����:2
	//exit(-1);	

	theJSLog.setLog(szLogPath,szLogLevel,m_szService , m_szSrcGrpID, 001);
	
	theJSLog<<"����Դ��:"<<m_szSrcGrpID<<"   service:"<<m_szService<<"  �������·��:"<<input_path<<"  ����·��:"<<erro_path
			<<"	��־·��:"<<szLogPath<<" ��־����:"<<szLogLevel<<" sql���·��:"<<sql_path<<"	ÿ������Դɨ���ļ�����:"<<source_file_num<<endi;
	
	if(bak_flag == 'Y')
	{
		theJSLog<<"�ļ�����·��:"<<bak_path<<endi;
	}

    theJSLog<<"��������Դ������ϢLoadSourceCfg..."<<endi;

	if(LoadSourceCfg() == -1) return false ;  //��������Դ������Ϣ
	
	conn.close();


	char input_dir[1024],bak_dir[1024],erro_dir[1024];
	int rett = -1;
	
	DIR *dirptr = NULL; 
	for(map<string,SOURCECFG>::const_iterator iter = m_SourceCfg.begin();iter !=m_SourceCfg.end(); ++iter)
	{		   		
			memset(input_dir,0,sizeof(input_dir));
			strcpy(input_dir,iter->second.szSourcePath);
			strcat(input_dir,input_path);
			if((dirptr=opendir(input_dir)) == NULL)
			{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"����Դ[%s]�������ļ�·��[%s]������",iter->first,input_dir);
					theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����

					return false ;
			}else closedir(dirptr);

			if(bak_flag == 'Y')
			{
					memset(bak_dir,0,sizeof(bak_dir));
					strcpy(bak_dir,iter->second.szSourcePath);
					strcat(bak_dir,bak_path);
					if((dirptr=opendir(bak_dir)) == NULL)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"����Դ[%s]�ı����ļ�·��[%s]������",iter->first,bak_dir);
						theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����
						return false ;
					}else closedir(dirptr);
			}
			
			memset(erro_dir,0,sizeof(erro_dir));
			strcpy(erro_dir,iter->second.szSourcePath);
			strcat(erro_dir,erro_path);
			if((dirptr=opendir(erro_dir)) == NULL)
			{
					theJSLog<<"����Դ��"<<iter->first<<"���Ĵ����ļ�·��: "<<erro_dir<<"�����ڣ����д���"<<endi;
					rett = mkdir(erro_dir,0755);
					if(rett == -1)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"����Դ[%s]�Ĵ����ļ��ļ�·��[%s]�����ڣ����д���ʧ��",iter->first,erro_dir);
						theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����

						return false;
					}
			}else closedir(dirptr);		

	}

   //�������ݿ����
   //excuteSQL();   
   
   //���Ĳ�����ȡ
   //CString sKeyVal;
   //param_cfg.bGetMem("memory.MT_DSPCH.sem_key_value", sKeyVal);
   //cout<<"��ȡ�ĺ��Ĳ���ֵ:"<<sKeyVal<<endl;

   //����д��־
   //writeLog(0,"����д��־");

   //exit (0);
	
   if(!drInit()) return false;
   //cout<<"m_triggerFile = "<<m_triggerFile<<endl;

/*
   char tmp[10];
   memset(tmp,0,sizeof(tmp));
   if(tmp[0] == '\0')
   {
			cout<<"tmpΪ��"<<endl;
   }
   char* p = NULL;
   if(strcmp(tmp,p) == 0)
   {
		cout<<"NULL = 0"<<endl;

   }
*/

   it = m_SourceCfg.begin();   //��ʼ����һ������Դ
	
   theJSLog<<"��ʼ����ϣ�"<<endi;

   return true ;
}

FileLoad::~FileLoad()
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

//��������Դ������Ϣ
int FileLoad::LoadSourceCfg()
{
	char szSqlStr[400];
	int iSourceCount=0;
	string sql;
	try
	{
		Statement stmt = conn.createStatement();
		sql = "select FILETYPE_ID from C_SOURCE_GROUP_DEFINE where SOURCE_GROUP=:1";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID;
		if(stmt.execute())
		{
			stmt>>m_szOutTypeId;
		}
		
		outrcd.Init(m_szOutTypeId);								//2013-07-31

   		//expTrace(szDebugFlag, __FILE__, __LINE__,"%s;%s",m_szSrcGrpID,m_szOutTypeId);
		sql = "select count(*) from C_SOURCE_GROUP_CONFIG where SOURCE_GROUP=:1";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID;
		if(stmt.execute())
		{
			stmt>>iSourceCount;
		}

		//expTrace(szDebugFlag, __FILE__, __LINE__, "iSourceCount=%d;", iSourceCount);
		
		sql = "select a.source_id,b.file_fmt,b.source_path,b.SERV_CAT_ID from C_SOURCE_GROUP_CONFIG a,I_SOURCE_DEFINE b where SOURCE_GROUP=:1 and a.source_id=b.source_id";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID;
		if(stmt.execute())
		{
			for (int i=0; i<iSourceCount; i++)
			{
				SOURCECFG SourceCfg;
				string strSourceId;

				stmt>>SourceCfg.szSourceId>>SourceCfg.szInFileFmt>>SourceCfg.szSourcePath>>SourceCfg.serverCatID;      
				strSourceId=SourceCfg.szSourceId;
			    
				completeDir(SourceCfg.szSourcePath);

				if(getSourceFilter(SourceCfg.szSourceId,SourceCfg.filterRule))
				{
					return -1;
				}
				
				m_SourceCfg[strSourceId]=SourceCfg;
		     }
		}

	}catch (SQLException e)
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"LoadSourceCfg���ݿ����%s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

		//throw jsexcp::CException(0, "LoadSourceCfg�������ݿ����", __FILE__, __LINE__);
		return -1;
	 }

	return 0;
}

/******��������Դ��ȡ���˹��� 0û�в鵽����1�鵽������*******************���Ƿ��ڼ�������Դ**/
int FileLoad::getSourceFilter(char* source,char* filter)
{	
	//CBindSQL ds( *m_DBConn );
	string sql;
	try
	{	Statement stmt = conn.createStatement();
		sql = "select file_filter from C_FILE_RECEIVE_ENV where source_id = :1 ";		
		stmt.setSQLString(sql);
		stmt << source;
		stmt.execute();
		if(!(stmt>>filter))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"����Դ[%s]û�����ù��˹���",source);	//��������δ����
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
			stmt.close();
			return -1;
		}
		stmt.close();
	}catch(SQLException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"getSourceFilter���ݿ����%s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
		return -1 ;
	}
	
	return 0;
}


//���ݿ�ĸ��²��������ڲ����쳣sql ��������
int FileLoad::updateDB(char* sql)
{
	int ret = 0;
	try
	{
		//cout<<"SQL:"<<sql<<endl;
		Statement stmt = conn.createStatement();
		stmt.setSQLString(sql);
		ret = stmt.execute();		//����0��ʾ����ʧ��
		stmt.close();

	}
	catch(util_1_0::db::SQLException e)
	{ 
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"updateDB ���ݿ����%s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

		return -1;
	}

	return ret;
}


//�ж��Ƿ�Ҫ���뻰����
int FileLoad::onBeforeTask()
{
	if(m_record.size() > 0) return 1 ;  //��ʾ�ϴ��ļ��ļ�¼��һ��������û������

	/**************************ɨ������Դ����ȡ�����ļ�*************************************/
		
	int ret = 0;	
	char szBuff[1012],dir[1024],filter[256],inputFilePath[1024]; 
		
	if( drStatus == 1 )  //��ϵͳ
	{
			isWriteSQLFileByTime();		

			//���trigger�����ļ��Ƿ����
			if(!CheckTriggerFile())
			{
				sleep(1);
				return 0;
			}

			//��ȡͬ������
			memset(m_SerialString,0,sizeof(m_SerialString));
			ret = drVarGetSet(m_SerialString);
			if(ret)
			{
				theJSLog<<"ͬ��ʧ��..."<<endl;
				return 0;
			}
	
			//��ȡͬ������
			vector<string> data;		
			splitString(m_SerialString,";",data,true,true);  //���͵��ַ�������ԴID,�ļ���,sqlFile�ļ���
			
			isWriteSQLFileByMain(data[2].c_str());	//��ϵͳ��sqlFile��ͨ����ϵͳ��������

			it = m_SourceCfg.find(data[0]);
			if(it == m_SourceCfg.end())			//�����Ƿ��ٲ�???
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"û���ҵ�������Դ��Ϣ[%s]",data[0]);		//��������δ����
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);

				return 0;
			}
			
			//���ļ������ļ���˽���ڴ�,
			memset(fileName,0,sizeof(fileName));
			memset(m_szFileName,0,sizeof(m_szFileName));
			memset(dir,0,sizeof(dir));
			
			strcpy(dir,it->second.szSourcePath);  //����Դ��·��
			strcpy(inputFilePath,dir);
			strcat(inputFilePath,input_path);

			strcpy(fileName,inputFilePath);
			strcat(fileName,data[1].c_str());

			//int dr_GetAuditMode()1��ʾͬ����2��ʾ����, ����Ϊʧ�ܣ�-1�����ô���-2�����ļ���ȡ��������
			int iStatus = dr_GetAuditMode(module_name);
			if(iStatus == 1)		//ͬ��ģʽ,ֻ���ǵ������
			{	
				while(1)
				{
					//�����ж�
					if(access(fileName,F_OK|R_OK))
					{
						sleep(10);
					}
					else
					{
						break;
					}
				}
			}
			else if(iStatus == 2)		//����ģʽ,Ĭ��300s
			{
				bool flag = false;
				int times = 1;
				while(times < 31)
				{
					if(access(fileName,F_OK|R_OK))
					{
						theJSLog<<"������"<<times<<"���ļ�"<<endi;
						sleep(10);
					}
					else
					{
						flag = true;
						break;
					}
				}

				if(!flag)
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"��ϵͳ���������ļ�[%s]������",fileName);
					theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);

					//��Ҫд���ȱ���,ֻ����д�����
					memset(m_AuditMsg,0,sizeof(m_AuditMsg));
					sprintf(m_AuditMsg,"%s not find",data[1]);
					if(!IsAuditSuccess(m_AuditMsg))
					{
						memset(sql,0,sizeof(sql));
						getCurTime(currTime); 
						sprintf(sql,"insert into D_SCH_LOAD(source_id,serv_cat_id,filename,deal_flag,dealstarttime,record_num,split_num) values('%s','%s','%s','E','%s',0,0)",data[0],it->second.serverCatID,data[1].c_str(),currTime);
						writeSQL(sql);
						commitSQLFile();	
					}

					return 0;
				}
			}
			else
			{
				char tmp[50] = {0};
				snprintf(tmp, sizeof(tmp), "����ƽ̨dr_GetAuditMode�������ô��󣬷���ֵ[%d]", iStatus);
				theJSLog<<tmp<<endi;
				return 0;
			}

			memset(m_szSourceID,0,sizeof(m_szSourceID));
			memset(m_szFileName,0,sizeof(m_szFileName));
			memset(mServCatId,0,sizeof(mServCatId));

			strcpy(m_szFileName,data[1].c_str());
			strcpy(m_szSourceID,it->first.c_str());
			strcpy(mServCatId,it->second.serverCatID);
				
	}	
	else								//��ϵͳ,������ϵͳ
	{
		
		isWriteSQLFile();				//�Ƿ��ύsql�ļ�

		if(file_num >= source_file_num)
		{
			file_num = 0;		//cout<<"��ͬһ������Դ����ɨ�赽N���ļ��������¸�����Դ"<<endl;
			++it ;
		}
		
		if(it == m_SourceCfg.end())
		{
			it = m_SourceCfg.begin();
		}

		memset(dir,0,sizeof(dir));
		strcpy(dir,it->second.szSourcePath);  //����Դ��·��		

		memset(inputFilePath,0,sizeof(inputFilePath));
		strcpy(inputFilePath,dir);
		strcat(inputFilePath,input_path); 

		memset(filter,0,sizeof(filter));
		strcpy(filter,it->second.filterRule);		  //��������
		 					
			
		//�򿪻����ļ�Ŀ¼
		if(scan.openDir(inputFilePath))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"�򿪻����ļ�Ŀ¼[%s]ʧ��",inputFilePath);
			theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����

			return -11;		//�����˳�
		}		
						
		//ѭ����ȡĿ¼��ɨ���ļ��У���ȡ�ļ�  ��Ϊ������ʱ�ļ������Ի�ɨ��10��
		int rett = -1 ;
		char tmp[512];

		while(1)		
		{
				memset(fileName,0,sizeof(fileName));
				rett = scan.getFile(filter,fileName);  				
				if(rett == 100)
				{		
						//cout<<dir<<": "<<it->first<<"��ʱ�ļ�Ŀ¼����û���ļ���ɨ���¸�����Դ"<<endl;
						scan.closeDir();		//2013-07-19
						file_num = 0;			//��ǰ����Դ�ļ���������
						++it ;
						return 0;
				}
				if(rett == -1)
				{	
					scan.closeDir();	//2013-07-19
					return 0 ;			//��ʾ��ȡ�ļ���Ϣʧ��
				}

				file_num++;				//ɨ��һ���ļ�������+1				
				

				/*�����ļ�*.tmp,*TMP,~* */			
				char* p = strrchr(fileName,'/');
				memset(tmp,0,sizeof(tmp));
				strcpy(tmp,p+1);

				if(tmp[0] == '~' )	continue ;//return 0;
				if(strlen(tmp) > 2)
				{		
						int pos = strlen(tmp)-3;
						//cout<<"��׺��Ϊ��"<<tmp+pos<<endl;
						if((strcmp(tmp+pos,"tmp") && strcmp(tmp+pos,"TMP")) == 0) 
						{
							//cout<<"ɨ�赽��ʱ�ļ�������"<<fileName<<endl;
							continue;
						}
				}
				
				theJSLog<<"ɨ�赽�ļ�:"<<fileName<<endi;
				
				memset(m_szSourceID,0,sizeof(m_szSourceID));
				memset(m_szFileName,0,sizeof(m_szFileName));
				memset(mServCatId,0,sizeof(mServCatId));

				strcpy(m_szFileName,p+1);
				strcpy(m_szSourceID,it->first.c_str());		  //��ǰ����Դ��source_id
				strcpy(mServCatId,it->second.serverCatID);	
				
				//p = strrchr(sqlFile,'/');
				//memset(tmp,0,sizeof(tmp));
				//if(p)
				//{
				//	strcpy(tmp,p+1);
				//}
				//else strcpy(tmp,sqlFile);

				memset(m_SerialString,0,sizeof(m_SerialString));
				sprintf(m_SerialString,"%s;%s;%s",m_szSourceID,m_szFileName,sqlFile);
				ret = drVarGetSet(m_SerialString);
				if(ret)
				{
					theJSLog<<"ͬ��ʧ��...."<<endi;
					return 0;
				}

				break;			//�ҵ��ļ��˳�ѭ��������һ���ļ���ռ�ö��������
		}
		
		scan.closeDir();
	}
	
	ret = dealFile();       //�����ļ�

	return ret; 

}


//�����ļ�������¼���ص��ڴ�
int FileLoad::dealFile()
{	
	int ret = -1;
	char szBuff[1024],tmp[1024],state;
	try
	{
					//��ȡpetri��״̬
					//rtinfo.getSysMode(petri_status);
			/*		
					int cnt = 0;
					//string sql = "select count(*) from D_SCH_LOAD where source_id = :1 and filename = :2 and deal_flag = :3";
					memset(sql,0,sizeof(sql));
					sprintf(sql,"select count(*) from D_SCH_LOAD where source_id = '%s' and filename = '%s' and deal_flag = 'Y'",m_szSourceID,m_szFileName);					
					Statement stmt = conn.createStatement();
					stmt.setSQLString(sql);
					//stmt<<m_szSourceID<<m_szFileName<<'Y';
					stmt.execute();
					stmt>>cnt;
					if(cnt > 0)  
					{
							theJSLog<<"���ȱ��Ѿ����ڸ��ļ�,���Ѿ����ɹ�����,�Ƶ�����Ŀ¼"<<endi;
							memset(tmp,0,sizeof(tmp));
							strcpy(tmp,it->second.szSourcePath);
							strcat(tmp,erro_path);
							strcat(tmp,m_szFileName);
							rename(fileName,tmp);
							return 0;
					}
			*/																
					//���ļ����ػ��� ,Ĭ�����ļ�������ʱ�������󣬳���in��app����ʹ��
					ifstream in(fileName,ios::nocreate);
					if(!in)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"dealFile() ���ļ�%sʧ�ܣ����� ",fileName);
						theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��

						return 0;
					}
					
					split_num = 0;	  //��ʼ���ָ������ļ���¼����
					record_num = 0;

					//ɨ�赽һ���ļ�������ע�ᵽ���ȱ�
					getCurTime(currTime);    //��ȡ��ǰ�ļ�ʱ��
					memset(sql,0,sizeof(sql));
					//sql = "insert into D_SCH_LOAD(source_id,serv_cat_id,filename,deal_flag,dealstarttime,record_num,split_num) values(:1,:2,:3,:4,:5,:6,:7)";
					sprintf(sql,"insert into D_SCH_LOAD(source_id,serv_cat_id,filename,deal_flag,dealstarttime,record_num,split_num) values('%s','%s','%s','W','%s',0,0)",m_szSourceID,mServCatId,m_szFileName,currTime);			
					writeSQL(sql);
					
					
					theJSLog<<"���ļ�"<<m_szFileName<<"��¼���ص�˽���ڴ�"<<endi;
					
					memset(szBuff,0,sizeof(szBuff));
					//һ���Լ���һ���ļ�ȫ�����ݣ������ݽ����������������¼��������з�������
					while(in.getline(szBuff,sizeof(szBuff)))   
					{					
						//cout<<"��ȡ��¼"<<szBuff<<endl;
						PkgFmt fmt ;
						strcpy(fmt.status,"0");
						strcpy(fmt.type,"H");
						strcpy(fmt.code,"load");
						strcpy(fmt.record,szBuff);
						m_record.push_back(fmt);

						memset(szBuff,0,sizeof(szBuff));
					}
					
					
					record_num = m_record.size();

					//��ȡ�ļ�ID,�Ե�һ����¼Ϊ׼
					outrcd.Set_record(m_record[0].record);
					char file_id[10];
					memset(file_id,0,sizeof(file_id));
					outrcd.Get_Field(FILE_ID,file_id);


					//**************���ٲ�,����Դ,�ļ���,�����������ܴ�ŵļ�¼����,��¼����
					memset(sql,0,sizeof(sql));

					memset(m_AuditMsg,0,sizeof(m_AuditMsg));
					sprintf(m_AuditMsg,"%s;%s;%d;%d",m_szSourceID,m_szFileName,record_num,maxRecord_num);
					if(!IsAuditSuccess(m_AuditMsg))
					{
						//���ȱ���E�ļ��Ƶ�����Ŀ¼
						state = 'E';
						getCurTime(currTime);  
						sprintf(sql,"update D_SCH_LOAD set deal_flag = '%c',dealendtime = '%s',file_id = %ld,record_num =%d,split_num =%d where source_id = '%s' and fileName = '%s' ",state,currTime,file_id,record_num,split_num,m_szSourceID,m_szFileName);
						writeSQL(sql);
						commitSQLFile();		

						memset(tmp,0,sizeof(tmp));								//�ļ��Ƶ�����Ŀ¼
						strcpy(tmp,it->second.szSourcePath);
						strcat(tmp,erro_path);
						strcat(tmp,m_szFileName);
						rename(fileName,tmp);

						return 0;
					}
									
					strcpy(file_name,m_szFileName);  //�ָ�ǰĬ����ԭʼ�ļ�����ͬ					
					
					//����ע����ȱ�				
					//sql = "update D_SCH_LOAD set deal_flag = :1 where source_id = :2 and filename = :3";
					getCurTime(currTime); 
					sprintf(sql,"update D_SCH_LOAD set deal_flag = 'H',dealendtime = '%s',file_id = %ld where source_id = '%s' and filename = '%s'",currTime,atol(file_id),m_szSourceID,m_szFileName);								
					writeSQL(sql);	
					
					ret = 1;

	}
	catch(jsexcp::CException e)
	{
			rollBackSQL();

			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"dealFile() error %s",e.GetErrMessage());
			theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ��쳣

			memset(tmp,0,sizeof(tmp));								//�ļ��Ƶ�����Ŀ¼
			strcpy(tmp,it->second.szSourcePath);
			strcat(tmp,erro_path);
			strcat(tmp,m_szFileName);
			rename(fileName,tmp);
			
			return 0;
	}

	return ret;
}

//�����̿�ʼ���仰��ǰ�Ĵ���
int FileLoad::onTaskBegin(void *task_addr)
{
      return 1;
}

//�ӽ��̳�ʼ��
bool FileLoad::onChildInit()
{
   theJSLog<<"�ӽ��̳�ʼ��"<<endi;
   return true;
   
}

//����ɹ����ػ�������(>=0)
int FileLoad::onTask(void *task_addr, int offset, int ticket_num)
{  
    theJSLog<<"�����ַ:"<<task_addr<<endi;
    char tmp[1024];

    memset(task_addr,0,getBlockSize());   //��ʼ�������ڴ��,����ڴ�	

	PkgBlock pkg((char*)task_addr);       //���������ʼ��
	pkg.init(getTicketLength(),getBlockSize());

	pkg.setModuleId(getModuleID());			//����ģ��ID
	pkg.setStatus(0);						//����״̬��0δ����1����
	pkg.setSourceId(it->first.c_str());		//��������Դ
	pkg.setFileHeadFlag(0);					//�����ļ�ͷ��־
	pkg.setFileTailFlag(0);					//�����ļ�β��־
	pkg.setFileType(it->second.szInFileFmt);//�����ļ�����

	theJSLog<<"�����ļ�:"<<m_szFileName<<"	��¼����:"<<m_record.size()<<endi;

	if(m_record.size() > maxRecord_num)             //��ʱ�ļ���¼��Ҫ�ָ�Ϊ���������
	{
		 //record_num += maxRecord_num;
		 split_num++;
		 sprintf(file_name,"%s#%04d",m_szFileName,split_num);

		 //theJSLog<<"�ļ���¼��̫����Ҫ�ָ�,�ļ�����"<<file_name<<endi;

		 pkg.setFileName(file_name);  //�����ļ���  

		 vector<PkgFmt>::iterator  iter1 = m_record.begin();
		 vector<PkgFmt>::iterator  iter2 = m_record.begin();
		 //int position = maxRecord_num*(split_num-1);
		
		 for(int i = 0;i<maxRecord_num;i++)
		 {
			 //cout<<"Ҫд�Ļ�����¼��"<<m_record[i].record<<" ��¼���ȣ�"<<strlen(m_record[i].record)<<endl;
			 pkg.writePkgRecord(m_record[i]);
			 ++iter2;
		 }
		
		 if(split_num == 1)
		 {
			pkg.setBlkPos("S");		//��ʾΪ������Ϊ�ļ��ĵ�һ��

		 }
		 else
		 {
			pkg.setBlkPos("M");		//��ʾ������Ϊ�ļ����м䲿��
		 }

		 pkg.setStatus(0);
		 pkg.setNamalRcdNum(maxRecord_num);
		 pkg.setRecordNum(maxRecord_num); 
		
		 //ɾ��ǰǰ��N�������������
		//cout<<"׼��ɾ����¼"<<endl;
		m_record.erase(iter1,iter2);
			
	}
	else
	{
		if(split_num)					//��ʾ�ļ���¼��Ҫ���Ϊ���������
		{
			split_num++;
			sprintf(file_name,"%s#%04d",m_szFileName,split_num);
			pkg.setFileName(file_name);
			pkg.setBlkPos("E");			//��ʾ���������ļ��������һ��
		}
		else
		{
			pkg.setFileName(file_name);  //�����ļ���
			pkg.setBlkPos("D");			 //��ʾ�����鵥������һ���ļ�
		}

		//record_num  += m_record.size();			//��¼�����Կ����ڼ������ļ���ʼʱ�ͱ�������������ÿ�����
		for(int i = 0;i<m_record.size();i++)
		{
			//cout<<"Ҫд�Ļ�����¼��"<<m_record[i].record<<" ��¼���ȣ�"<<strlen(m_record[i].record)<<endl;
			pkg.writePkgRecord(m_record[i]);
		}

		pkg.setStatus(0);
		pkg.setNamalRcdNum(m_record.size());
		pkg.setRecordNum(m_record.size());   
		
		m_record.clear();  //���˽���ڴ�	
		
		
		getCurTime(currTime);  
		memset(sql,0,sizeof(sql));
		//char state = 'Y';	
		//���»������ļ���Ϣ��д���ȱ���ɱ�־
		//string sql = "update D_SCH_LOAD set deal_flag = :1,dealendtime = :2,record_num = :3,split_num = :4 where source_id = :5 and fileName = :6 ";
		sprintf(sql,"update D_SCH_LOAD set deal_flag = 'Y',dealendtime = '%s',record_num =%d,split_num =%d where source_id = '%s' and fileName = '%s' ",currTime,record_num,split_num,m_szSourceID,m_szFileName);			
		writeSQL(sql);
		commitSQLFile();			
		
	    //���ļ�����
		if(bak_flag == 'Y')
		{		
				//�ж��Ƿ���Ҫ����,2013-07-16Ŀ¼����YYYYMM/DD										
				char bak_dir[1024];
				memset(bak_dir,0,sizeof(bak_dir));
				strcpy(bak_dir,it->second.szSourcePath);
				strcat(bak_dir,bak_path);
				strncat(bak_dir,currTime,6);
				completeDir(bak_dir);
				strncat(bak_dir,currTime+6,2);
				completeDir(bak_dir);

				if(chkAllDir(bak_dir) == 0)
				{
					theJSLog<<"�����ļ�["<<m_szFileName<<"]��Ŀ¼"<<bak_dir<<endi;
					strcat(bak_dir,m_szFileName);
					if(rename(fileName,bak_dir))
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"�ļ�[%s]�ƶ�[%s]ʧ��: %s",fileName,bak_dir,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);		//�ƶ��ļ�ʧ��
					}
				}
				else
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"����·��[%s]�����ڣ����޷�����",bak_dir);
					theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg);		//��Ŀ¼����
				}
		}	
		else 
		{
				if(remove(fileName))
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"�ļ�[%s]ɾ��ʧ��: %s",fileName,strerror(errno));
					theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);		//ɾ���ļ�ʧ��
				}
		}
		
	/*	
	   char tmp[1024];   //�������ݣ��ļ���������Դ���ļ���¼��
	   memset(tmp,0,sizeof(tmp));
	   sprintf(tmp,"%s;%s;%d",m_szSourceID,m_szFileName,record_num);
		
	   MdrAuditInfo audit_info_;
       if ( syn_status == 1 )			   //����ϵͳ����
		{		
			if( slaveAudit(audit_info_,tmp)) 
			{
                 cmtResult(audit_info_ );
				 cout<<"�ٲ��ύ�ɹ�"<<endl;
			}
			else
			{
				cout<<"�ٲ��ύʧ��"<<endl;
			}
		
		}
		else                               //��ϵͳ������һ���ļ���������Ϣ������ϵͳ���жԱ�
		{
			if( masterAudit( audit_info_, tmp ) )
			{
				cmtResult( audit_info_ );
				cout<<"�ٲ��ύ�ɹ�"<<endl;	
			} 
			else 
			{
				 cout<<"�ٲ��ύʧ��"<<endl;
			}

		}
		
	*/

	}

   //sleep(30);   //��ͣ30�룬���ڹ۲�

   return -1;
}

//�ӽ����˳�ǰ�Ĵ���
void FileLoad::onChildExit()
{
    cout<<"�ӽ����˳�"<<endl;
}

//�����ӽ����������������̵Ĵ���
int FileLoad::onTaskOver(int child_ret)
{
   return child_ret ;

}

//���ݿ����
int FileLoad::excuteSQL()
{	
    return 1;

}


//���ֳ�ʼ��
bool FileLoad::drInit()
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
int FileLoad::drVarGetSet(char* serialString)
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
 bool FileLoad::IsAuditSuccess(const char* dealresult)
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

bool FileLoad::CheckTriggerFile()
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

/*

//����ϵͳ��ʼ��
MdrStatus FileLoad::syncInit()
{
	MdrRetCode rc_;
    rc_ = mdr_InitPlatform();  //��ʼ������ƽ̨
    printf( "%s,%d-->mdr_InitPlatform, rc_=%d, (rc=>0:SUCCESS,1:FAILURE,-1:INVALID)\n", __FILE__, __LINE__, rc_ );
    //writelog( 0, "mdr_InitPlatform" );

    if( rc_ != MDR_SUCCESS ) {
        //printf( "%s,%d-->FATAL! mdr_InitPlatform failed\n", __FILE__, __LINE__ );
        //writelog( 0, "��ʼ������ƽ̨����!" );
        cerr<<"��ʼ������ƽ̨����!"<<endi;
    }

    MdrStatus stat_;
    rc_ = mdr_GetDRStatus( stat_ );
    printf( "%s,%d-->mdr_GetDRStatus, rc_=%d, stat_=%d, (stat=>0:MASTER,1:SLAVE,2:NODR)\n", __FILE__, __LINE__, rc_, stat_ );

    if( rc_ != MDR_SUCCESS ) {
        printf( "%s,%d-->FATAL! mdr_GetDRStatus failed\n", __FILE__, __LINE__ );
        //writelog( 0, "��ȡ����ƽ̨״̬ʧ��!" );
        cerr<<"��ȡ����ƽ̨״̬ʧ��!"<<endi;
    }

    MdrNodeType node_type_;
    rc_ = mdr_GetNodeType( node_type_ );
    printf( "%s,%d-->mdr_GetNodeType, rc_=%d, node_type_=%d (node_type=>0:DUPLEX,1:SINGLE)\n", __FILE__, __LINE__, rc_, node_type_ );

    if( rc_ != MDR_SUCCESS ) {
        //printf( "%s,%d-->FATAL! mdr_GetNodeType failed\n", __FILE__, __LINE__ );
        //writelog( 0, "��ʼ������ƽ̨����!" );
        cerr<<"��ʼ������ƽ̨����!"<<endi;
    }

    rc_ = mdr_GetNodeTypeSR( node_type_ );
    printf( "%s,%d-->mdr_GetNodeTypeSR, rc_=%d, sr_node_type_=%d\n", __FILE__, __LINE__, rc_, node_type_ );

    if( rc_ != MDR_SUCCESS ) {
        //printf( "%s,%d-->FATAL! mdr_GetNodeTypeSR failed\n", __FILE__, __LINE__ );
        //writelog( 0, "��ʼ������ƽ̨����!" );
        cerr<<"��ʼ������ƽ̨����!"<<endi;
    }

	if(rc_ != MDR_SUCCESS )
	{
			exit(-1);
	}

    return stat_;

}

bool FileLoad::runmdr( char* value )
{
    if( syn_status == 1 ) {
        std::vector<MdrVarInfo> var_list_;
        var_list_.clear();
        return true;
    } else {
        MdrAuditInfo audit_info_;
        cout << "������ϵͳ" << endl;
        if( masterAudit( audit_info_, value ) ) {
            cmtResult( audit_info_ );
            return true;
        } else {
            return false;
        }

    }
}

//�����ϵͳ����
void FileLoad::fillMasterAuditInfo( MdrAuditInfo& audit_info, char* value  )
{
    int pid_ = getpid();
    char buf_[1024];
    audit_info.node = "TPSS1";
    audit_info.srvContext = "srvContext";

    struct timeval tv_;
    ::gettimeofday( &tv_, NULL );
    tv_.tv_sec --;
    struct tm tm_;
    ::localtime_r( &tv_.tv_sec, &tm_ );
    char tmp_[64];
    strftime( tmp_, sizeof( tmp_ ), "%Y%m%d%H%M%S", &tm_ );
    sprintf( buf_, "%s.%06d", tmp_, ( int )( tv_.tv_usec % 1000000 ) );
    audit_info.ccrRcvTime = buf_;

    sprintf( buf_, "auditKey_%d_%s", pid_, audit_info.ccrRcvTime.c_str() );
    audit_info.auditKey = buf_;
    sprintf( buf_, "sessionId_%d_%s", pid_, audit_info.ccrRcvTime.c_str() );
    audit_info.sessionId = buf_;

    audit_info.rflag = 2;			// must fill 2

    audit_info.syncVar = string( value );
    audit_info.auditVal = audit_info.syncVar;
    tv_.tv_sec --;
    ::localtime_r( &tv_.tv_sec, &tm_ );
    strftime( tmp_, sizeof( tmp_ ), "%Y%m%d%H%M%S", &tm_ );
    sprintf( buf_, "%s.%06d", tmp_, ( int )( tv_.tv_usec % 1000000 ) );
    audit_info.ccrEvtTime = buf_;
    audit_info.result = 0;    
    //cout << "auditvalue = " << audit_info.syncVar << endl;
    //cout << "auditkey = " << auditkey << endl;
    //cout << "sessionid = " << sessionid << endl;
}

//��ϵͳ�ٲ�
bool FileLoad::masterAudit( MdrAuditInfo& audit_info,char* value )
{
    fillMasterAuditInfo( audit_info, value );
    MdrRetCode rc_ = mdr_Audit( audit_info );    //audit_info.result�ֶ���Ϊ�����Ĳ����ᱻAPI�޸�
    std::string audit_info_str_;
    //audit_info.toStr( audit_info_str_ );
    //audit_info_str_ = audit_info.toStr();
    cout << "auditKey " << audit_info.auditKey << endl;
    cout << "auditVal " << audit_info.auditVal << endl;
    cout << "sessionId " << audit_info.sessionId << endl;
    cout << "rflag " << audit_info.rflag << endl;
    
    printf( "%s,%d-->mdr_Audit, rc_=%d, %s\n", __FILE__, __LINE__, rc_, audit_info_str_.data() );
    //writelog( 0, "��ϵͳ�ٲ�����Ϊ " + audit_info_str_ );

    if( rc_ != MDR_SUCCESS ) {
        printf( "%s,%d-->FATAL! mdr_Audit failed\n", __FILE__, __LINE__ );
        //writelog( 0, "��ϵͳ�ٲ�ʧ�� " );
        return false;
    }

    return true;
}


bool FileLoad::slaveAudit( MdrAuditInfo& audit_info ,char *allval)
{
    audit_info.node = "TPSS2";
    audit_info.srvContext = "srvContext";
    audit_info.rflag = 2;					// must fill 2
    audit_info.auditKey = auditkey;			//ֵ��ͨ��ǰ����ϵͳ��������
    audit_info.sessionId = sessionid;
    audit_info.auditVal = allval;
    audit_info.ccrEvtTime = "20000101000000";		// SLAVE side: dummy value to pass fmt validation
    audit_info.ccrRcvTime = "20000101000000.000000";	// SLAVE side: dummy value to pass fmt validation

    MdrRetCode rc_ = mdr_Audit( audit_info );
    cout << "auditKey " << audit_info.auditKey << endl;
    cout << "auditVal " << audit_info.auditVal << endl;
    cout << "sessionId " << audit_info.sessionId << endl;
    cout << "rflag " << audit_info.rflag << endl;
    
    //printf( "%s,%d-->mdr_Audit, rc_=%d, %s\n", __FILE__, __LINE__, rc_, audit_info.data() );
    printf( "%s,%d-->mdr_Audit, rc_=%d\n", __FILE__, __LINE__, rc_ );
   // writelog( 0, "��ϵͳ�ٲ�����Ϊ" + audit_info.toStr() );

    if( rc_ != MDR_SUCCESS ) {
        printf( "%s,%d-->FATAL! mdr_Audit failed\n", __FILE__, __LINE__ );
        writelog( 0, "��ϵͳ�ٲ�ʧ�� " );
        return false ;
    }

    return true ;

}

void FileLoad::cmtResult( const MdrAuditInfo& audit_info )
{
    MdrRetCode rc_ = mdr_CmtResult( audit_info );
    std::string audit_info_str_;
    //audit_info.toStr( audit_info_str_ );
    printf( "%s,%d-->mdr_CmtResult, rc_=%d, %s\n", __FILE__, __LINE__, rc_, audit_info_str_.data() );
    writelog( 0, "mdr_CmtResult ���Ϊ" + audit_info_str_ );

    if( rc_ != MDR_SUCCESS ) {
        //printf( "%s,%d-->FATAL! mdr_CmtResult failed\n", __FILE__, __LINE__ );
        writelog( 0, "����ʧ��" );
    }
}

*/