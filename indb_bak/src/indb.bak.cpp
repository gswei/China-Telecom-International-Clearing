/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-06-19
File:			 C_Indb.cpp
Description:	 ��д�����鴦��
History:
<table>
revision	author            date                description
--------    ------            ----                -----------
v1.0		hed          2013-09-01

</table>
**************************************************************************/


#include<dirent.h> //_chdir() _getcwd() ��ȡ�ļ�Ŀ¼�ȣ�����ļ�����Ϣ
#include<string>
#include<vector>
#include<sys/types.h>
#include<sys/stat.h>  //stat()��������ѯ�ļ���Ϣ
#include<unistd.h>     //��ȡ��ǰ��������Ŀ¼
#include<iostream>
#include<fstream>
#include <sstream>
#include "indb.h"

#include "CF_Common.h"
#include "CF_CLogger.h"

CLog theJSLog;
SGW_RTInfo	rtinfo;

C_Indb::C_Indb()
{  	
	//file_num  = 0;
	source_file_num = 0;
	petri_status = 0;

	//split_num = 0;
	bak_flag = 'N';  //Ĭ�ϲ�����
	record_num = 0;

    memset(m_szSrcGrpID,0,sizeof(m_szSrcGrpID));
	memset(m_szService,0,sizeof(m_szService));
	//memset(mServCatId,0,sizeof(mServCatId));
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

C_Indb::~C_Indb()
{
	if(m_enable) 
	{
		int ret = dr_ReleasePlatform();
		if(ret != 0)
		{
			char tmp[100] = {0};
			snprintf(tmp, sizeof(tmp), "�ͷ�����ƽ̨ʧ��,����ֵ=%d", ret);
			theJSLog<<tmp<<endw;
		}
	}
	
}

//ģ���ʼ������
bool C_Indb::init(int argc,char** argv)
{
   
    if(!PS_Process::init(argc,argv))
    {
      return false;
    }
	
	if(!drInit())	return false;

	//��ȡpetri��״̬,��ϵͳΪֻ��̬ʱ,���ݿ���²������д�ļ�
	if(!(rtinfo.connect()))
	{
		return false;
	}
	rtinfo.getDBSysMode(petri_status);
	cout<<"petri status:"<<petri_status<<endl; 


	if(!(dbConnect(conn)))
	{
		cout<<"��ʼ�����ݿ� connect error."<<endl;
		return false ;
	}

	int flow_id = getFlowID();
	int module_id = getModuleID(); 

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
			cout<<"����tp_process���ֶ�ext_param������service"<<endl;
			return false ;
		}

		sql = "select c.input_path from C_SOURCE_GROUP_DEFINE a,C_SERVICE_FLOW b,C_SERVICE_INTERFACE c where a.source_group=:1" 
                    "and a.workflow_id = b.workflow_id and b.service=:2 and b.input_id = c.interface_id ";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID<<m_szService;
		stmt.execute();
		if(!(stmt>>input_path))
		{
			cout<<"�������ģ�������ļ����·��input_pathû������"<<endl;
			return false ;
		}
		completeDir(input_path);
	
		sql = "select var_value from c_process_env where varname = 'INDB_FILE_BAK_FLAG' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID<<m_szService;
		stmt.execute();
		if(!(stmt>>bak_flag))
		{
			bak_flag = 'N';			 
		}
		if(bak_flag == 'Y')
		{
			sql = "select var_value from c_process_env where varname = 'INDB_FILE_BAK_DIR' and source_group=:1 and service=:2 ";
			stmt.setSQLString(sql);
			stmt<<m_szSrcGrpID<<m_szService;
			stmt.execute();
			if(!(stmt>>bak_path))
			{
					cout<<"���ڱ�c_process_env�����ü���ģ��ı���Ŀ¼,INDB_FILE_BAK_DIR"<<endl;
					return false;
			}
			completeDir(bak_path);
		}
		
		sql = "select var_value from c_process_env where varname = 'INDB_FILE_ERR_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID<<m_szService;
		stmt.execute();
		if(!(stmt>>erro_path))
		{
				cout<<"���ڱ�c_process_env�����ü���ģ��Ĵ���·��,INDB_FILE_ERR_DIR"<<endl;
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
			return false;
	}
	
	theJSLog.setLog(szLogPath,szLogLevel,m_szService , m_szSrcGrpID, 001);
	
	theJSLog<<"  �������·��:"<<input_path<<"  ����·��:"<<erro_path<<"	��־·��:"<<szLogPath<<" ��־����:"
			<<szLogLevel<<"	ÿ������Դɨ���ļ�����:"<<source_file_num<<endi;
	
	if(bak_flag == 'Y')
	{
		theJSLog<<"�ļ�����·��:"<<bak_path<<endi;
	}

    theJSLog<<"��������Դ������ϢLoadSourceCfg..."<<endi;

	if(LoadSourceCfg() == -1) return false ;  //��������Դ������Ϣ
	
	conn.close();


	char input_dir[512],bak_dir[512],erro_dir[512];
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
						theJSLog<<"����Դ��"<<iter->first<<"���ı����ļ�·��: "<<bak_dir<<"�����ڣ����д���"<<endw;
						rett = mkdir(bak_dir,0755);
						if(rett == -1)
						{
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"����Դ[%s]�ı����ļ��ļ�·��[%s]�����ڣ����д���ʧ��",iter->first,bak_dir);
							theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����

							return false;
						}
					}else closedir(dirptr);
			}
			
			memset(erro_dir,0,sizeof(erro_dir));
			strcpy(erro_dir,iter->second.szSourcePath);
			strcat(erro_dir,erro_path);
			if((dirptr=opendir(erro_dir)) == NULL)
			{
					theJSLog<<"����Դ��"<<iter->first<<"���Ĵ����ļ�·��: "<<erro_dir<<"�����ڣ����д���"<<endw;
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
	
   it = m_SourceCfg.begin();   //��ʼ����һ������Դ
	
   theJSLog<<"��ʼ�����..."<<endi;

   return true ;
}



//��������Դ������Ϣ
int C_Indb::LoadSourceCfg()
{
	int iSourceCount=0;
	string sql ;
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

				if(getSourceFilter(SourceCfg.szSourceId,SourceCfg.filterRule,SourceCfg.file_begin,SourceCfg.file_length))
				{
							return -1;
				}
				
				m_SourceCfg[strSourceId]=SourceCfg;
		     }
		}

		
		//����ÿ������Դ��Ӧ��ͳ������
		string source_id;
		int config_id = 0;
		sql = "select source_id, var_value from c_source_env a  where a.varname = 'INS_TABLE_CONFIGID' and a.service =:1";
		stmt.setSQLString(sql);
		stmt<<m_szService;
		stmt.execute();
		while(stmt>>source_id>>config_id)
		{
				mapConfig.insert(map<string,int>::value_type(source_id,config_id));
		}
		stmt.close();
		
		for(map<string,SOURCECFG>::const_iterator iter = m_SourceCfg.begin();iter !=m_SourceCfg.end(); ++iter)
		{
			map<string,int>::const_iterator iter2 = mapConfig.find(iter->first);
			if(iter2 == mapConfig.end())
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"��source_env������Դ��[%s]���õ�����Դ[%s]û���������ı��� INS_TABLE_CONFIGID",m_szSrcGrpID,iter->first);
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);

				return -1;
			}
			
			CF_CError_Table tab ;
			tab.Init(iter2->second);
			mapTabConf.insert(map< string,CF_CError_Table>::value_type(iter->first,tab));
			
			theJSLog<<"source_id = "<<iter->first<<" config_id = "<<iter2->second<<endi;
		}

		//�ж��Ƿ������config_id������ͬ����������Դ�Ҳ�����Ӧ�ı�
		//if(m_SourceCfg.size() > mapConfig.size())
		//{
		//	memset(erro_msg,0,sizeof(erro_msg));
		//	sprintf(erro_msg,"��source_env������Դ��[%s]���õ�INS_TABLE_CONFIGID��ĳЩ����Դû���������ı��� config_id",m_szSrcGrpID);
		//	theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
		//	return -1;
		//}	
		//cout<<"iSourceCount = "<<iSourceCount<<" mapConfig.size()="<<mapConfig.size()<<endl;

		//for(map<string,int>::const_iterator iter = mapConfig.begin();iter != mapConfig.end();++iter)
		//{
		//	CF_CError_Table tab ;
		//	tab.Init(iter->second);
		//	mapTabConf.insert(map< string,CF_CError_Table>::value_type(iter->first,tab));
		//}

	}catch (SQLException e)
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"LoadSourceCfg���ݿ����%s (%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

		//throw jsexcp::CException(0, "LoadSourceCfg�������ݿ����", __FILE__, __LINE__);
		return -1;
	 }
	catch (jsexcp::CException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"LoadSourceCfg() error: %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);
		return -1 ;
	}
	return 0;
}

/******��������Դ��ȡ���˹��� 0û�в鵽����1�鵽������*******************���Ƿ��ڼ�������Դ**/
int C_Indb::getSourceFilter(char* source,char* filter,int &index,int &length)
{	
	try
	{	
		string file_time;
		Statement stmt = conn.createStatement();
		string sql = "select file_filter,file_time_index_len from C_FILE_RECEIVE_ENV where source_id = :1 ";		
		stmt.setSQLString(sql);
		stmt << source;
		stmt.execute();
		if(!(stmt>>filter>>file_time))
		{
				stmt.close();
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"����Դ[%s]û�����ù��˹�������ļ���ʱ���ȡ����",source);
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return -1;
		}
		
		//cout<<"file_time = "<<file_time<<endl;

		char tmp[6];
		memset(tmp,0,sizeof(tmp));	
		strcpy(tmp,file_time.c_str());	
		vector<string> fileTime;		
		splitString(tmp,",",fileTime,false);
		if(fileTime.size() != 2)
		{
			stmt.close();
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"����Դ[%s]�ļ���ʱ���ȡ�������ù������:%s  [��3,8]",source,file_time);
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
			return -1;
		}
		
		index = atoi(fileTime[0].c_str());
		length = atoi(fileTime[1].c_str());
		
		//cout<<"index = "<<index<<"  length = "<<length<<" file_time = "<<file_time<<endl;
		if(index < 1 || length == 0)
		{
			stmt.close();
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"����Դ[%s]�ļ���ʱ���ȡ�������ù������:%s  [��3,8]",source,file_time);
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
			return -1;
		}

		index--;

		stmt.close();

	}
	catch(SQLException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"getSourceFilter ���ݿ��ѯ�쳣: %s (%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);
		return -1 ;
	}
	catch(jsexcp::CException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"getSourceFilter �ֶ�ת������%s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);
		return -1;
	}
	
	return 0;
}

//ѭ��ɨ���������Դ
void C_Indb::run()
{	
	int ret = 0;	
	char dir[512],inputFilePath[512],filter[256],szFiletypeIn[10];	
	
	if(gbExitSig)
	{
		theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "Ӧ�ó����յ��˳��ź�");
		PS_Process::prcExit();
		return;
	}

	//�ж����ݿ�״̬
	rtinfo.getDBSysMode(petri_status);
	if(petri_status == DB_STATUS_OFFLINE)	return ;
	
	if(!(dbConnect(conn)))
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() �������ݿ�ʧ�� connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
		return  ;
	}

	if( drStatus == 1 )  //��ϵͳ
	{	
			//���trigger�����ļ��Ƿ����
			if(!CheckTriggerFile())
			{
				sleep(1);
				return ;
			}

			//��ȡͬ������
			memset(m_SerialString,0,sizeof(m_SerialString));
			ret = drVarGetSet(m_SerialString);
			if(ret)
			{
				theJSLog<<"ͬ��ʧ��..."<<endw;
				return ;
			}
	
			//��ȡͬ������
			vector<string> data;		
			splitString(m_SerialString,";",data,false,false);  //���͵��ַ�������ԴID,�ļ���,sqlFile�ļ���
			
			it = m_SourceCfg.find(data[0]);
			if(it == m_SourceCfg.end())			//�����Ƿ��ٲ�
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"û���ҵ�������Դ��Ϣ[%s]",data[0]);		//��������δ����
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				
				dr_AbortIDX();  
				return ;
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
			if(iStatus == 1)		//ͬ��ģʽ,	��ϵͳ�ȴ�ָ��ʱ��
			{	
				bool flag = false;
				int times = 1;
				while(times < 31)
				{
					if(access(fileName,F_OK|R_OK))
					{
						theJSLog<<"������"<<times<<"���ļ�"<<endi;
						times++;
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
					
					dr_AbortIDX();  
					return ;
				}		
			}
			else if(iStatus == 2)		//����ģʽ,��ϵͳ
			{
				while(1)
				{
					//�����ж�
					if(gbExitSig)
					{
						dr_AbortIDX();

						theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "Ӧ�ó����յ��˳��ź�");
						PS_Process::prcExit();
						return;
					}

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
			else
			{
				char tmp[50] = {0};
				snprintf(tmp, sizeof(tmp), "����ƽ̨dr_GetAuditMode�������ô��󣬷���ֵ[%d]", iStatus);
				theJSLog<<tmp<<endw;
				return ;
			}
			
			theJSLog<<"���ҵ��ļ���"<<fileName<<endi;

			memset(m_szSourceID,0,sizeof(m_szSourceID));
			memset(m_szFileName,0,sizeof(m_szFileName));
			memset(mServCatId,0,sizeof(mServCatId));

			strcpy(m_szFileName,data[1].c_str());
			strcpy(m_szSourceID,it->first.c_str());
			strcpy(mServCatId,it->second.serverCatID);
			
			//setSQLFileName(data[2].c_str());

			ret = dealFile();
			if(ret)
			{
				dr_AbortIDX();
			}
	}
	else
	{
		if(it == m_SourceCfg.end())
		{
			it = m_SourceCfg.begin();
		}
	
		memset(dir,0,sizeof(dir));
		strcpy(dir,it->second.szSourcePath);  //����Դ��·��		

		strcpy(inputFilePath,dir);
		strcat(inputFilePath,input_path); 

		memset(filter,0,sizeof(filter));
		memset(szFiletypeIn,0,sizeof(szFiletypeIn));  
		strcpy(filter,it->second.filterRule);		  //��������
		strcpy(szFiletypeIn,it->second.szInFileFmt);  //��ǰ����Դ�������ʽ
		strcpy(m_szSourceID,it->first.c_str());
					
		//�򿪻����ļ�Ŀ¼
		if(scan.openDir(inputFilePath))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"�򿪻����ļ�Ŀ¼[%s]ʧ��",inputFilePath);
			theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����
			it++;
			return ;	
		}		
						
		//ѭ����ȡĿ¼��ɨ���ļ��У���ȡ�ļ�  ��Ϊ������ʱ�ļ������Ի�ɨ��10��
		int rett = -1 ;
		char tmp[512];
		file_num = 0;
		while(1)		
		{		
				if(file_num == source_file_num)
				{
					file_num = 0;
					break;
				}

				memset(fileName,0,sizeof(fileName));
				rett = scan.getFile(filter,fileName);  				
				if(rett == 100)
				{		
						break;
				}
				if(rett == -1)
				{	
						break;		//��ʾ��ȡ�ļ���Ϣʧ��
				}

				file_num++;				//ɨ��һ���ļ�������+1				

				/*�����ļ�*.tmp,*TMP,~* */			
				char* p = strrchr(fileName,'/');
				memset(tmp,0,sizeof(tmp));
				strcpy(tmp,p+1);
				
				if(tmp[0] == '~' )	continue ;
				if(strlen(tmp) > 2)
				{		
						int pos = strlen(tmp)-3;
						if((strcmp(tmp+pos,"tmp") && strcmp(tmp+pos,"TMP")) == 0) 
						{
							continue;
						}
				}
	
				theJSLog<<"ɨ�赽�ļ���"<<fileName<<endi;

				strcpy(m_szFileName,p+1);

				//����ͬ����Ϣ
				memset(m_SerialString,0,sizeof(m_SerialString));
				sprintf(m_SerialString,"%s;%s",m_szSourceID,m_szFileName);
				ret = drVarGetSet(m_SerialString);
				if(ret)
				{
					theJSLog<<"ͬ��ʧ��...."<<endw;
					return ;
				}
				
				//setSQLFileName(m_szFileName);
				
				memset(mServCatId,0,sizeof(mServCatId));
				strcpy(mServCatId,it->second.serverCatID);

				ret = dealFile();
				if(ret)
				{
					dr_AbortIDX();
				}
		}

		scan.closeDir();
		it++;
	}

	conn.close();
}


//���ļ���� ret = 0��ʾ���ɹ� ret= -1��ʾ���ʧ��
int C_Indb::dealFile()
{	
	int ret = 0;
	char szBuff[1024];
	
	//����***********************��
	//strcpy(m_szSourceID,"HD");
	//strcpy(m_szOutTypeId,"SJSTD");
	//strcpy(m_szFileName,"HDC.2013.201307181150");
	//file = "/mboss/jtcbs/zbjs1_a/data/service/HED/HD/end_path/HDC.2013.201307181150";

	//if(!(dbConnect(conn)))
	//{
	//	cout<<"�������ݿ�ʧ�� connect error."<<endl;
	//	return -1 ;
	//}
	
	memset(m_AuditMsg,0,sizeof(m_AuditMsg));
	
	theJSLog<<"�ļ�["<<m_szFileName<<"]���"<<endi;

	map< string,CF_CError_Table>::const_iterator iter =  mapTabConf.find(m_szSourceID);
	if( iter == mapTabConf.end())
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"indb() ���ݿ���û������Դ%s��������Ϣ",m_szSourceID);
		theJSLog.writeLog(LOG_CODE_SOURCE_NOT_FIND,erro_msg);	
		return -1;
	}

	record_num = 0;
	Statement stmt;
	try
	{
		ifstream in(fileName,ios::nocreate);
		if(!in)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"dealFile() �ļ�[%s]�򿪳���",fileName);
			theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��
			return -1;
		}

		stmt = conn.createStatement();

		CF_CError_Table tab = iter->second;
		tab.setFileName(m_szFileName,m_szSourceID,stmt);  
		
		memset(szBuff,0,sizeof(szBuff));
	
		while(in.getline(szBuff,sizeof(szBuff)))   
		{	
			theJSLog<<"record["<<szBuff<<"]"<<endi;
			outrcd.Set_record(szBuff);
			tab.dealInsertRec(outrcd,NULL,NULL,NULL);
			record_num++;
			memset(szBuff,0,sizeof(szBuff));		
		}	

		tab.commit();			//��ֹû�ﵽ��¼��������������ǿ�Ʋ���		
		in.close();
		
		//2013-10-01  ������ȡfile_id ��file_time
		strncpy(file_time,m_szFileName+it->second.file_begin,it->second.file_length);
		file_time[8] = '\0';
	
		char fileid[10];
		memset(fileid,0,sizeof(fileid));
		outrcd.Get_Field(FILE_ID,fileid);
		file_id = atol(fileid);

		//**************���ٲ�,����Դ,�ļ���,�����������ܴ�ŵļ�¼����,��¼����
		char state = 'Y',tmp[512];
		sprintf(m_AuditMsg,"%s;%s;%d",m_szSourceID,m_szFileName,record_num);
		if(!IsAuditSuccess(m_AuditMsg))
		{
			stmt.rollback();
			//rollBackSQL();
			//state = 'E';
			memset(tmp,0,sizeof(tmp));								//�ļ��Ƶ�����Ŀ¼
			strcpy(tmp,it->second.szSourcePath);
			strcat(tmp,erro_path);
			strcat(tmp,m_szFileName);
			if(rename(fileName,tmp))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"�ƶ��ļ�[%s]������Ŀ¼ʧ��: %s",fileName,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
			}
			
			return ;
		 }
		
		//�������Ҫ�ж����ݿ�״̬��,�п�����������ݿ��л�Ϊֻ����
		//��ȡ���ݿ�״̬	
		while(1)
		{
			rtinfo.getDBSysMode(petri_status);
			if(petri_status == DB_STATUS_OFFLINE)
			{
				theJSLog<<"���ݿ�״̬Ϊֻ��̬,�ȴ�״̬�ı�..."<<endw;
				sleep(600);
				continue;
			}

			break;
		}
		
		//theJSLog<<"���µ��ȱ�d_sch_indb,�ļ��������"<<endi;

		//����ע����ȱ�
		getCurTime(currTime); 
		memset(sql,0,sizeof(sql));	
		sprintf(sql,"insert into d_sch_indb(source_id,serv_cat_id,filename,deal_flag,dealtime,mainflow_count,file_id,file_time) values('%s','%s','%s','%c','%s',%d,%ld,'%s')",m_szSourceID,mServCatId,m_szFileName,state,currTime,record_num,file_id,file_time);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt.close();

		//if(state == 'E') return -1;		//�ļ��Ѿ�Ų��ʧ��Ŀ¼ȥ��

		//���ļ�����
		if(bak_flag == 'Y')
		{		
			//�ж��Ƿ���Ҫ����										
			char bak_dir[512];
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
	}
	catch (jsexcp::CException e)
	{
		stmt.rollback();
		
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"%s indb() error line=(%d) :%s ",m_szFileName,record_num,e.GetErrMessage());
		theJSLog.writeLog(0,erro_msg);
		
		char tmp[512];
		memset(tmp,0,sizeof(tmp));								//�ļ��Ƶ�����Ŀ¼
		strcpy(tmp,it->second.szSourcePath);
		strcat(tmp,erro_path);
		strcat(tmp,m_szFileName);
		rename(fileName,tmp);

		return -11 ;
	}
	catch(SQLException e)
	{
		stmt.rollback();

		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"%s indb() sql error line=(%d) :%s (%s)",m_szFileName,record_num,e.what(),szBuff);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);
    	
		char tmp[512];
		memset(tmp,0,sizeof(tmp));								//�ļ��Ƶ�����Ŀ¼
		strcpy(tmp,it->second.szSourcePath);
		strcat(tmp,erro_path);
		strcat(tmp,m_szFileName);
		rename(fileName,tmp);

    	return -11 ;
    }

	return ret;
}

//���ֳ�ʼ��
bool C_Indb::drInit()
{
		//����ģ������ʵ��ID
		char tmp[32];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%ld",getPrcID());

		theJSLog << "��ʼ������ƽ̨,ģ����:"<< module_name<<" ʵ����:"<<tmp<<endi;

		int ret = dr_InitPlatform(module_name,tmp);
		if(ret != 0)
		{
			theJSLog << "����ƽ̨��ʼ��ʧ��,����ֵ=" << ret<<endw;
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
			theJSLog<<"��ȡ����ƽ̨״̬����: ����ֵ="<<drStatus<<endw;
			return false;
		}
		
		if(drStatus == 0)		theJSLog<<"��ǰϵͳ����Ϊ��ϵͳ"<<endi;
		else if(drStatus == 1)	theJSLog<<"��ǰϵͳ����Ϊ��ϵͳ"<<endi;
		else if(drStatus == 2)	theJSLog<<"��ǰϵͳ���÷�����ϵͳ"<<endi;

		return true;
}

//��ϵͳ����ͬ������,��ϵͳ��ȡͬ������
int C_Indb::drVarGetSet(char* serialString)
{
		int ret ;
		char tmpVar[5000] = {0};

		//�������ƽ̨���л���
		ret = dr_CheckSwitchLock();   
		if(ret != 0)  
		{  
			theJSLog<<"��������л���ʧ��,����ֵ="<<ret<<endw;
			return -1;  
		} 
		//��ʼ��index  
		ret = dr_InitIDX();  
		if(ret != 0)  
		{  
			theJSLog<<"��ʼ��indexʧ��,����ֵ=" <<ret<<endw;
			dr_AbortIDX();
			return -1;  
		}
/*		
		//��ϵͳ�����ļ�����·�����ļ��� ֻ������ƽ̨���Ը�֪,��ϵͳ�޷�ʶ��
		if(drStatus != 1)
		{
			snprintf(tmpVar, sizeof(tmpVar), "%s%s", it->second.szSourcePath,input_path);
			ret = dr_SyncIdxVar("@@CHECKPATH", tmpVar,SYNC_SINGLE);  
			if(ret != 0)
			{
				theJSLog<<"�����ļ�����·��ʧ��,�ļ�·��["<<input_path<<"]"<<endw;
				dr_AbortIDX();
				return -1;
			}
			
			snprintf(tmpVar, sizeof(tmpVar), "%s", m_szFileName);
			ret = dr_SyncIdxVar("@@CHECKFILE", tmpVar,SYNC_SINGLE);  
			if(ret != 0)
			{
				theJSLog<<"�����ļ�ʧ��,�ļ���["<<m_szFileName<<"]"<<endw;
				dr_AbortIDX();
				return -1;
			}

			cout<<"�����ļ�·��:"<<input_path<<" �ļ���:"<<m_szFileName<<endl;
		}

*/
		snprintf(tmpVar, sizeof(tmpVar), "%s", serialString);
		//��ϵͳ��Ҫͬ����index ����ֵ�ԡ�д������ƽ̨ά����index �ļ���
		//��ϵͳ���øú����Ľ���ǣ�var��ú���ϵͳһ�������������ֵ��	SYNC_SINGLE��ʾע�ᵥһ���������
		ret = dr_SyncIdxVar("serialString", tmpVar, SYNC_SINGLE);		
		if (ret != 0)
		{
			theJSLog<<"�����д�ʱʧ��,������:["<<serialString<<"]"<<endw;
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
			theJSLog<<"����ʵ����ʧ�ܣ�"<<tmpVar<<endw;
			dr_AbortIDX();  
			return -1;
		}
		
		
		// <6> Ԥ�ύindex  �˹ؼ������ڽ�ƽ̨��ǰ�ڴ��е��������д�����
		ret = dr_SyncIdxVar("@@FLUSH","SUCCESS",SYNC_SINGLE);  
		if (ret != 0 )
		{
			theJSLog<<"Ԥ�ύindexʧ��"<<endw;
			dr_AbortIDX();
			return -1;
		}
		
		
		// <7> �ύindex  	�ύIndex����index�ļ���������ɱ�־
		ret = dr_CommitIDX();  
		if(ret != 0)  
		{  
			theJSLog<<"�ύindexʧ��,����ֵ="<<ret<<endw;
			dr_AbortIDX();  
			return -1;  
		}

		//��ϵͳ����Ŀ¼
		//if(!m_syncDr.isMaster())thelog<<"��ϵͳSerialString��"<<m_SerialString<<endi;

		theJSLog<<"���ε�ͬ����serialString:"<<serialString<<endi;//for test

		return ret;

}

//�ٲ��ַ���
 bool C_Indb::IsAuditSuccess(const char* dealresult)
 {
		int auitStatus = 0, ret = 0;
		ret = dr_Audit(dealresult);
		if(2 == ret )
		{
			theJSLog << "�����ٲ�ʧ��,���:" << ret <<"���ˣ�"<<dealresult<< endw;
			dr_AbortIDX();
			return false;
		}
		else if (3 == ret)
		{
			theJSLog<<"�����ٲó�ʱ..."<<endw;
			dr_AbortIDX();
			return false;
		}
		else if(4 == ret)
		{
			theJSLog<<"�Զ�idx�쳣��ֹ..."<<endw;
			dr_AbortIDX();
			return false;
		}
		else if(1 == ret)
		{
			ret = dr_CommitSuccess();
			if(ret != 0)
			{
				theJSLog << "ҵ��ȫ���ύʧ��(����ƽ̨)" << endw;
				dr_AbortIDX();
				return false;
			}
			theJSLog<<"ret = "<<ret<<"�ٲóɹ�...\n�ٲ����ݣ�"<<dealresult<<endi;
			return true;
		}
		else
		{
			theJSLog<<"δ֪ret="<<ret<<"	�ٲ����ݣ�"<<dealresult<<endw;
			dr_AbortIDX();
			return false;
		}
	
	return true;
 }

bool C_Indb::CheckTriggerFile()
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

int main(int argc,char** argv)
{
	cout<<"********************************************** "<<endl;
	cout<<"*    GuangDong Telecom. Telephone Network      "<<endl;
	cout<<"*       Centralized Settlement System          "<<endl;
	cout<<"*                                              "<<endl;
	cout<<"*            jsindb		                      "<<endl;
	cout<<"*              Version 1.0	                  "<<endl;
	cout<<"*     created time :      2013-09-01 by  hed	  "<<endl;
	cout<<"*     last updaye time :  2013-09-26 by  hed	  "<<endl;
	cout<<"********************************************** "<<endl;


	C_Indb fm ;

	if(!fm.init(argc,argv)) return false;
	
	while(1)
	{
		theJSLog.reSetLog();
		fm.run();
		sleep(5);
	}

   return 0;
}








