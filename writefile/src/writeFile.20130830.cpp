/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2012-11-4
File:			 Write_File.cpp
Description:	 ��д�����鴦��
History:
<table>
revision	author            date                description
--------    ------            ----                -----------
v1.0		hd          2012-11-4

2013-07-15 ��һ���ļ�д���������ʱĳ��������ļ�¼��Ϣ��������˸��ļ��Ѿ�����Ļ����飬����ɾ��������¼������Ϣ��sql�ļ���
		  �������¸��ļ�ʱ��д����ǼǱ�����Դ�ļ�copy������Ŀ���ļ�
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
#include "writeFile.h"

#include "CF_Common.h"
#include "CF_CLogger.h"
CLog theJSLog;
SGW_RTInfo	rtinfo;

Write_File::Write_File()
{  
	file_status = 0;
	record_num = 0;
	petri_status = 0;
	m_enable = false;

	memset(mServCatId,0,sizeof(mServCatId));
    memset(m_szSrcGrpID,0,sizeof(m_szSrcGrpID));
	memset(m_szService,0,sizeof(m_szService));
	memset(m_szSourceID,0,sizeof(m_szSourceID));
	memset(m_szOutTypeId,0,sizeof(m_szOutTypeId));

	memset(other_path,0,sizeof(other_path));
	memset(out_path,0,sizeof(out_path));
	memset(erro_path,0,sizeof(erro_path));
	memset(bak_path,0,sizeof(bak_path));

	memset(m_szFileName,0,sizeof(m_szFileName));
	memset(outFileName,0,sizeof(outFileName));

	memset(sql,0,sizeof(sql));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(erro_sql,0,sizeof(erro_sql));
	memset(currTime,0,sizeof(currTime));
}


Write_File::~Write_File()
{
	//cout<<"xxxx"<<endl;
	if(m_enable) 
	{
		//cout<<"�ͷ�����ƽ̨��Դ"<<endl;

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
bool Write_File::init(int argc,char** argv)
{
   
   if(!PS_BillProcess::init(argc,argv))
   {
      return false;
   }
	
   //��ȡpetri��״̬,��ϵͳΪֻ��̬ʱ,���ݿ���²������д�ļ�
	
	if(!(rtinfo.connect()))
	{
		return false;
	}
	
	rtinfo.getSysMode(petri_status);
	cout<<"petri status:"<<petri_status<<endl;

	//*********2013-07-15 ��ȡ���ݿ��������Ϣ��������Դ�飬����Ŀ¼ 2013-03-11 add by hed*********************************************/
	
	if(!(dbConnect(conn)))
	{
		cout<<"�������ݿ� connect error."<<endl;	//д��־
		return false ;
	}

	int flow_id = getFlowID();
	int module_id = getModuleID();
	//char sourceGroup[8],service[8];  
	//cout<<"��ˮ��ID:"<<flow_id<<"   ģ��ID:"<<module_id<<endl;
	try{

		string sql = "select source_group from TP_BILLING_LINE  where billing_line_id = :1";
		Statement stmt = conn.createStatement();
		stmt.setSQLString(sql);
		stmt<<flow_id;
		stmt.execute();
		if(!(stmt>>m_szSrcGrpID))
		{
			cout<<"����tp_billing_line������������Դ��"<<endl;			//д��־
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

		sql = "select c.output_path from C_SOURCE_GROUP_DEFINE a,C_SERVICE_FLOW b,C_SERVICE_INTERFACE c where a.source_group=:1" 
                    "and a.workflow_id = b.workflow_id and b.service=:2 and b.input_id = c.interface_id ";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID<<m_szService;
		stmt.execute();
		if(!(stmt>>out_path))
		{
			cout<<"д�ļ�ģ������ļ����·��out_pathû������"<<endl;		
			return false ;
		}

		completeDir(out_path);
				
		sql = "select var_value from c_process_env where varname = 'WR_FILE_ERR_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID<<m_szService;
		stmt.execute();
		if(!(stmt>>erro_path))
		{
				cout<<"���ڱ�c_process_env������д�ļ�ģ��Ĵ���·�� WR_FILE_ERR_DIR"<<endl;		//д��־
				return false;
		}		
		completeDir(erro_path);
	
		
		sql = "select var_value from c_process_env where varname = 'WR_FILE_BAK_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID<<m_szService;
		stmt.execute();
		if(!(stmt>>bak_path))
		{
			cout<<"���ڱ�c_process_env������д�ļ�ģ��ı���Ŀ¼(Դ�ļ�Ŀ¼) WR_FILE_BAK_DIR"<<endl;		//д��־
			return false;
		}
		completeDir(bak_path);

		sql = "select var_value from c_process_env where varname = 'WR_FILE_TMP_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID<<m_szService;
		stmt.execute();
		if(!(stmt>>other_path))
		{
			cout<<"���ڱ�c_process_env������д�ļ�ģ�����ʱĿ¼ WR_FILE_TMP_DIR"<<endl;		//д��־
			return false;
		}
		completeDir(other_path);

		stmt.close();
	   
	   }catch(SQLException e)
		{
			cout<<"��ʼ��ʱ���ݿ��ѯ�쳣:"<<e.what()<<endl;
			return false ;
		}
	
	
	 DIR *dirptr = NULL; 
	
	sprintf(erro_sql,"%sjswritefileErro.%d.sql",sql_path,getFlowID());	//����sql�ļ�ȫ·��

	//��ʼ���ڴ���־�ӿ�
	bool bb = initializeLog(argc,argv,false);  //�Ƿ����ģʽ
	if(!bb)
	{
			//theJSLog<<"��ʼ���ڴ���־�ӿ�ʧ��"<<endi;
			return false;
	}
	
	theJSLog.setLog(szLogPath,szLogLevel,m_szService , m_szSrcGrpID, 001);	//�ļ���־�ӿڣ��������ڴ���־�ӿ�
	
	theJSLog<<"����Դ�飺"<<m_szSrcGrpID<<"   service:"<<m_szService<<"  ������·��:"<<out_path<<"  ����·��:"<<erro_path
			<<"	Դ�ļ�����·��:"<<bak_path<<"	��־·��:"<<szLogPath<<" ��־����:"<<szLogLevel<<" sql���·��:"<<sql_path<<endi;

	if(LoadSourceCfg() == -1) 
	{		
		return false ;  
	}
	
	conn.close();

	char out_dir[1024],erro_dir[1024],bak_dir[1024],other_dir[1024];
	int rett = 0;

	for(map<string,SOURCECFG>::const_iterator iter = m_SourceCfg.begin();iter !=m_SourceCfg.end(); ++iter)
	{		   				
			memset(out_dir,0,sizeof(out_dir));
			strcpy(out_dir,iter->second.szSourcePath);
			strcat(out_dir,out_path);
			if((dirptr=opendir(out_dir)) == NULL)
			{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"����Դ[%s]������ļ�·��[%s]������",iter->first,out_dir);
					theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����
					return false ;
			}else closedir(dirptr);
			
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
					
			memset(bak_dir,0,sizeof(bak_dir));
			strcpy(bak_dir,iter->second.szSourcePath);
			strcat(bak_dir,bak_path);
			if((dirptr=opendir(bak_dir)) == NULL)
			{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"����Դ[%s]�ı����ļ�·��[%s]������",iter->first,bak_dir);
					theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����
					return false ;

			}else  closedir(dirptr);

			memset(other_dir,0,sizeof(other_dir));
			strcpy(other_dir,iter->second.szSourcePath);
			strcat(other_dir,other_path);
			if((dirptr=opendir(other_dir)) == NULL)
			{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"����Դ[%s]����ʱ�ļ�·��[%s]������",iter->first,other_dir);
					theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����
					return false ;

			}else  closedir(dirptr);
	  
	}

	
   if(!drInit())  return false;

   theJSLog<<"��ʼ����ϣ�"<<endi;

   return true ;
}


//��������Դ������Ϣ
int Write_File::LoadSourceCfg()
{
	char szSqlStr[400];
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
		
		outrcd.Init(m_szOutTypeId);

   		//expTrace(szDebugFlag, __FILE__, __LINE__,"%s;%s",m_szSrcGrpID,m_szOutTypeId);
		sql = "select count(*) from C_SOURCE_GROUP_CONFIG where SOURCE_GROUP=:1";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID;
		if(stmt.execute())
		{
			stmt>>iSourceCount;
		}

		//expTrace(szDebugFlag, __FILE__, __LINE__, "iSourceCount=%d;", iSourceCount);
		
		sql = "select a.source_id,b.file_fmt,b.source_path,b.TOLLCODE,b.SERV_CAT_ID from C_SOURCE_GROUP_CONFIG a,I_SOURCE_DEFINE b where SOURCE_GROUP=:1 and a.source_id=b.source_id";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID;
		if(stmt.execute())
		{
			for (int i=0; i<iSourceCount; i++)
			{
				SOURCECFG SourceCfg;
				string strSourceId;

				stmt>>SourceCfg.szSourceId>>SourceCfg.szInFileFmt>>SourceCfg.szSourcePath>>SourceCfg.szTollCode>>SourceCfg.serverCatID;      
				strSourceId=SourceCfg.szSourceId;
				
				completeDir(SourceCfg.szSourcePath);

				if(getSourceFilter(SourceCfg.szSourceId,SourceCfg.filterRule,SourceCfg.file_begin,SourceCfg.file_length))
				{
							return -1;
				}
				
				m_SourceCfg[strSourceId]=SourceCfg;
		     }
		}
		
		//2013-07-29
		string source_id;
		int config_id = 0;
		sql = "select source_id, var_value from c_source_env a  where a.varname = 'INS_TABLE_CONFIGID' and a.service = 'WRTF'";
		stmt.setSQLString(sql);
		stmt.execute();
		while(stmt>>source_id>>config_id)
		{
			mapConfig.insert(map<string,int>::value_type(source_id,config_id));
		}
		stmt.close();

		//�ж��Ƿ������config_id������ͬ����������Դ�Ҳ�����Ӧ�ı�
		if(iSourceCount > mapConfig.size())
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"source_env�����õ�INS_TABLE_CONFIGID������Դû��������ȫ��%s");
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
			return -1;
		}	
		//cout<<"iSourceCount = "<<iSourceCount<<" mapConfig.size()="<<mapConfig.size()<<endl;

		for(map<string,int>::const_iterator iter = mapConfig.begin();iter != mapConfig.end();++iter)
		{
			CF_CError_Table tab ;
			tab.Init(iter->second);
			mapTabConf.insert(map< string,CF_CError_Table>::value_type(iter->first,tab));
		}

		
	}catch (SQLException e)
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"LoadSourceCfg() ���ݿ����%s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
		//throw jsexcp::CException(0, "LoadSourceCfg�������ݿ����", __FILE__, __LINE__);
		return -1;
	 }
	 catch (jsexcp::CException &e) 
	 {	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"LoadSourceCfg() : %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		//�ֶ�ת������
		return -1;
	 }

	return 0;
}

/******��������Դ��ȡ���˹��� 0û�в鵽����1�鵽������*******************���Ƿ��ڼ�������Դ**/
//int Write_File::getSourceFilter(char* source,char* filter)
int Write_File::getSourceFilter(char* source,char* filter,int &index,int &length)
{	
	try
	{	
		string file_time;
		char tmp[5];
		memset(tmp,0,sizeof(tmp));

		Statement stmt = conn.createStatement();
		string sql = "select file_filter,file_time_index_len from C_FILE_RECEIVE_ENV where source_id = :1 ";		
		stmt.setSQLString(sql);
		stmt << source;
		stmt.execute();
		if(!(stmt>>filter>>file_time))
		{
				stmt.close();
				sprintf(erro_msg,"����Դ[%s]û�����ù��˹�������ļ���ʱ���ȡ����",source);
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return -1;
		}
		
		//cout<<"file_time = "<<file_time<<endl;
		strcpy(tmp,file_time.c_str());

		vector<string> fileTime;		
		splitString(tmp,",",fileTime,true);
		if(fileTime.size() != 2)
		{
			stmt.close();
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
		sprintf(erro_msg,"getSourceFilter ���ݿ��ѯ�쳣: %s [%s]",e.what(),sql);
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

//���ݿ�ĸ��²��������ڲ����쳣sql ��������
int Write_File::updateDB(char* sql)
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
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"updateDB ���ݿ����%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
		return -1;
	}

	return 0;
}


//�鿴sql�ļ����Ƿ������ݣ�������ִ��
int Write_File::scanSQLFile()
{
	char szBuff[1024];
	ifstream in(sqlFile,ios::in) ;
	if(!in)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"scanSQLFile �ļ�%s�򿪳���",sqlFile);
		theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��
		return -1;
	}

	Statement stmt = conn.createStatement();
	memset(szBuff,0,sizeof(szBuff));
	while(in.getline(szBuff,sizeof(szBuff)))   
	{		
			stmt.setSQLString(szBuff);
			stmt.execute();
			memset(szBuff,0,sizeof(szBuff));
			
	}

	stmt.close();

	in.close();

	in.open(sqlFile,ios::trunc);
	in.close();

	return 0;

}

//�ж��Ƿ�Ҫ���뻰����
int Write_File::onBeforeTask()
{
	//cout<<"���뻰����"<<endl;
	return 1;
  
}

//�����̿�ʼ���仰��ǰ�Ĵ���
int Write_File::onTaskBegin(void *task_addr)
{
      return 1;
}

//�ӽ��̳�ʼ��
bool Write_File::onChildInit()
{
   //cout<<"ģ��3�ӽ��̳�ʼ��"<<endl;
   return true;
   
}

//����ɹ����ػ�������(>=0)
int Write_File::onTask(void *task_addr, int offset, int ticket_num)
{
    //cout<<"\nģ��3������ඣ���������������������"<<endl;

    cout<<"�����ַ"<<task_addr<<endl;		 
	
	PkgBlock pkg((char*)task_addr);
	pkg.init(getTicketLength(),getBlockSize());

	//cout<<"---------------------����д��ȥ�ļ�¼------------------------"<<endl;	
    cout<<"������¼��: "<<pkg.getRecordNum()<<endl;
	cout<<"ģ��ID: "<<pkg.getModuleId()<<endl;
	cout<<"����Դ�� "<<pkg.getSourceId()<<endl;
	cout<<"�����ļ�����"<<pkg.getFileName()<<endl;
	cout<<"����״̬��"<<pkg.getStatus()<<endl;	
	cout<<"���������ļ�������λ�ã�"<<pkg.getBlkPos()<<endl;
	
	int ret = 0;
	char deal_flag = 'Y';   //����״̬

	char tmp[1024],path[1024],orgFileName[1024],sql[1024],full_name[1024],block_pos[2];

	memset(tmp,0,sizeof(tmp));
	memset(path,0,sizeof(path));
	memset(orgFileName,0,sizeof(orgFileName));
	memset(full_name,0,sizeof(full_name));
	memset(block_pos,0,sizeof(block_pos));

	strcpy(block_pos,pkg.getBlkPos());  //��ȡ������λ��

	//��ʱ��Ҫ��ȡpetri��״̬
	if((strcmp(block_pos,"S") == 0) || (strcmp(block_pos,"D") == 0))
	{
		rtinfo.getDBSysMode(petri_status);
		theJSLog<<"�������ļ�["<<orgFileName<<"] petri״̬:"<<petri_status<<endi;
	}

	//if(record_num  == 0)  
	//{
		record_num += pkg.getRecordNum();
	//	return 0;
	//}
	
	strcpy(m_szSourceID,pkg.getSourceId());
	it  = m_SourceCfg.find(string(m_szSourceID));
	if(it == m_SourceCfg.end())
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"�����в���������Դ%s����Ϣ",m_szSourceID);
		theJSLog.writeLog(LOG_CODE_PARAM_INFO_LACK,erro_msg);
		return -1;
	}
	strcpy(mServCatId,it->second.serverCatID);

	strcpy(m_szFileName,pkg.getFileName());
	strcpy(path,it->second.szSourcePath);
	strcat(path,out_path);
	strcpy(full_name,path);

	char* p = strrchr(m_szFileName,'#');			//���ԭ�ļ���
	if(p)
	{
		strncpy(orgFileName,m_szFileName,(p-m_szFileName));
	}
	else
	{
		strcpy(orgFileName,m_szFileName);
	}
	//cout<<"ԭʼ�ļ�����"<<orgFileName<<endl;
	
	memset(file_time,0,sizeof(file_time));			//���ļ��������ȡʱ��
	strncpy(file_time,orgFileName+it->second.file_begin,it->second.file_length);
	
	strcat(full_name,orgFileName);
	strcpy(tmp,full_name);	 //��ʽ�ļ�
	strcat(tmp,".tmp");		//��ʱ�ļ�

	//�жϻ������Ƿ���ȷ������ȷ����д��ʱ�ļ���
	//������ȷ������ɾ����ǰ�ĸû������ļ���дsql�쳣��¼��Ϣ��Ȼ��ѭ���������ÿ����¼��������Ϣ�Ǽ�	
	try
	{	
		if(!(dbConnect(conn)))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"Ontask() �������ݿ�ʧ�� connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
			return -1 ;
		}
		
		//2013-07-17 �жϻ���������ļ���λ��ͨ���������ͷ������ж�S��ʾ��ʼ E���� M�м� D����

		//���쳣���˵����ݿ�Ǽ�ERRFILE_INFO���ļ���������Դ��ʱ�䡢����ԭ��F/R���������롢�����С���������š�
		//����״̬---�ɳ���ĳ������Ǽ�	�ڲ���ʽд�������ֶΣ�LINE_NUM,ERR_CODE,ERR_LINE,FILE_ID
	
		if(pkg.getStatus() == 2)					 //����������
		{
			if((strcmp(block_pos,"S") == 0) || (strcmp(block_pos,"M") == 0))
			{
				if(strcmp(block_pos,"S") == 0)				//��һ��ģ��
				{
					//��ȡpetri��״̬
					//rtinfo.getSysMode(petri_status);

					// ���ļ�ע�ᵽ���ȱ��ļ���ȡ��ǰ�ļ�ʱ��,file_id,�Ե�һ��Ϊ׼
					getCurTime(currTime);							
					outrcd.Set_record(pkg.readPkgRecord(1).record);
					char file_id[10];
					memset(file_id,0,sizeof(file_id));
					memset(sql,0,sizeof(sql));
					outrcd.Get_Field(FILE_ID,file_id);
					sprintf(sql,"insert into d_sch_end(source_id,serv_cat_id,filename,deal_flag,dealstarttime,file_id,file_time) values('%s','%s','%s','W','%s',%ld,'%s')",m_szSourceID,mServCatId,orgFileName,currTime,atol(file_id),file_time);
					writeSQL(sql);

					ofstream out(tmp);									//��ֹ�����쳣���ļ�����ͬ�����,����ļ�
					if(!out)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"Ontask() �ļ�%s�򿪳���",tmp);
						theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��

						return -1;
					}
					out.close();
				}
						
				if(file_status == 0)							//�ϸ��ļ�������״̬����
				{
					ofstream out(tmp,ios::app);
					if(!out)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"Ontask() �ļ�%s�򿪳���",tmp);
						theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��

						return -1;
					}
					for(int k = 1;k<=pkg.getRecordNum();k++)
					{
						PkgFmt fmt = pkg.readPkgRecord(k);
						out<<fmt.record<<"\n";
						
					}					
					out.close();		
				}

				//ÿ����һ�������飬���������ļ���¼����
				getCurTime(currTime);	
				memset(sql,0,sizeof(sql));
				sprintf(sql,"update d_sch_end set deal_flag = 'H',dealendtime = '%s',input_count = '%d' where filename = '%s' and source_id = '%s' ",currTime,record_num,orgFileName,m_szSourceID);
				writeSQL(sql);

			}
			else if(strcmp(block_pos,"E") == 0)		//����������Ѿ������һ����
			{	
				if(file_status == -1)
				{
					commitErrMsg();
					//theJSLog<<"ɾ����ʱ����Ļ������ļ�"<<endi;
					if(remove(tmp))
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"��ʱ�ļ�[%s]ɾ��ʧ��: %s",tmp,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
					}

					//�Ӹ�ʽ��ԭʼ�ļ�Ŀ¼���ó��ŵ�����Ŀ¼
					char bak_time[15],erro_dir[1024],bak_dir[1024];

					memset(bak_time,0,sizeof(bak_time));
					memset(erro_dir,0,sizeof(erro_dir));
					memset(bak_dir,0,sizeof(bak_dir));
					memset(sql,0,sizeof(sql));

					//�Ӹ�ʽ��ԭʼ�ļ�Ŀ¼���ó��ŵ�����Ŀ¼
					sprintf(sql,"select deal_time from D_SCH_FORMAT where source_id='%s' and filename='%s' and deal_flag='Y'",m_szSourceID,orgFileName);
					Statement stmt = conn.createStatement();
					stmt.setSQLString(sql);
					if((stmt.execute()))
					{
						stmt>>bak_time;
						strcpy(erro_dir,it->second.szSourcePath);
						strcat(erro_dir,erro_path);
						strcat(erro_dir,orgFileName);
						
						strcpy(bak_dir,it->second.szSourcePath);
						strcat(bak_dir,bak_path);
						strncat(bak_dir,bak_time,6);
						strcat(bak_dir,"/");
						strncat(bak_dir,bak_time+6,2);
						strcat(bak_dir,"/");
						strcat(bak_dir,orgFileName);
						
						theJSLog<<"��ԭʼ�ļ��ӱ���Ŀ¼"<<bak_dir<<"�ϴ�������Ŀ¼"<<erro_dir<<endi;
						if(link(bak_dir,erro_dir))					 //����һ���ļ�������Ŀ¼
						{
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"��ԭʼ�ļ��ӱ���Ŀ¼�ϴ�������Ŀ¼ʧ��: %s",strerror(errno));
							theJSLog.writeLog(LOG_CODE_FILE_RENAME_ERR,erro_msg);
						}
					}
					else
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"û�дӸ�ʽ���ǼǱ����ҵ��ļ�[%s]",orgFileName);
						theJSLog.writeLog(LOG_CODE_TABLE_QUERY_ERR_CODE,erro_msg);
					}
					

					file_status = 0;
				}
				else												//����
				{
					//��д��ʱ�ļ����ٸ���
					ofstream out(tmp,ios::app);
					if(!out)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"Ontask() д�ļ�,�ļ�%s�򿪳���",tmp);
						theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��

						return -1;
					}
					for(int k = 1;k<=pkg.getRecordNum();k++)
					{
						PkgFmt fmt = pkg.readPkgRecord(k);
						out<<fmt.record<<"\n";
					}					
					out.close();	
					
					//�ж����ݿ�״̬�Ƿ�����⻹��д�ļ�
					
					if (petri_status == DB_STATUS_ONLINE)   //�ж��Ƿ���ά��̬
					{
						ret = indb(tmp,orgFileName) ;
						if(ret)								//���ʧ�ܻ��߲�ȫ�����ļ�Ų������Ŀ¼
						{
							deal_flag = 'E';
							char other_dir[1024];
							memset(other_dir,0,sizeof(other_dir));
							strcpy(other_dir,it->second.szSourcePath);
							strcat(other_dir,other_path);
							theJSLog<<"���ļ��Ƶ�����Ŀ¼"<<other_dir<<endi;
							strcat(other_dir,orgFileName);
							rename(tmp,other_dir);
						}
						else
						{
							ret = record_num;

							rename(tmp,full_name) ;				//����ʱ�ļ������ʽ�ļ�
							theJSLog<<"д�ļ�["<<orgFileName<<"]�ɹ�,��鿴�ļ���·����"<<path<<endi;
						}

					}
					else
					{
						//���ݿ�ֻ�� ���ļ�����·����������vector,�ȴ��´����ݿ�״̬����ʱ�ٽ�Ŀ¼������ļ����
						deal_flag  = 'D';
					}
	
					//�������ݿ�״̬��Ϣ
					memset(sql,0,sizeof(sql));
					sprintf(sql,"update d_sch_end set deal_flag = '%c',input_count='%d',mainflow_count='%d' where filename = '%s' and source_id = '%s'",deal_flag,record_num,ret,orgFileName,m_szSourceID);
					writeSQL(sql);			
				}
				
				record_num = 0;  //�ļ���¼����0
			}
			else if(strcmp(block_pos,"D") == 0)	//��ʾ����һ���ļ�һ�������� ������
			{
				//��ȡpetri��״̬
				//rtinfo.getSysMode(petri_status);

				// ���ļ�ע�ᵽ���ȱ��ļ���ȡ��ǰ�ļ�ʱ��,file_id,�Ե�һ��Ϊ׼
				getCurTime(currTime);							
				outrcd.Set_record(pkg.readPkgRecord(1).record);
				char file_id[10];
				memset(file_id,0,sizeof(file_id));
				memset(sql,0,sizeof(sql));
				outrcd.Get_Field(FILE_ID,file_id);
				sprintf(sql,"insert into d_sch_end(source_id,serv_cat_id,filename,deal_flag,dealstarttime,file_id,file_time) values('%s','%s','%s','W','%s',%ld,'%s')",m_szSourceID,mServCatId,orgFileName,currTime,atol(file_id),file_time);
				writeSQL(sql);
				
				//��д��ʱ�ļ����ٸ���
				ofstream out(tmp);
				if(!out)
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"Ontask() д�ļ�,�ļ�%s�򿪳���",tmp);
					theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��

					return -1;
				}
				for(int k = 1;k<=pkg.getRecordNum();k++)
				{
					PkgFmt fmt = pkg.readPkgRecord(k);
					out<<fmt.record<<"\n";
					//cout<<"��"<<k<<"����¼״̬��"<<fmt.status<<"  ��¼���룺"<<fmt.code<<"  ��¼ֵ��"<<fmt.record<<endl;
				}		
			
				out.close();
				
				//�ж����ݿ�״̬
				ret = indb(tmp,orgFileName) ;
				if(ret)									//���ʧ�ܻ��߲�ȫ�����ļ�Ų������Ŀ¼
				{
					deal_flag = 'E';
					char other_dir[1024];
					memset(other_dir,0,sizeof(other_dir));
					strcpy(other_dir,it->second.szSourcePath);
					strcat(other_dir,other_path);
					theJSLog<<"���ļ��Ƶ�����Ŀ¼"<<other_dir<<endi;
					strcat(other_dir,orgFileName);
					rename(tmp,other_dir);
				}
				else
				{
					ret = record_num;

					rename(tmp,full_name) ;				//����ʱ�ļ������ʽ�ļ�
					theJSLog<<"д�ļ�"<<orgFileName<<"�ɹ�,��鿴�ļ���·����"<<path<<endi;
				}
		
				//�������ݿ�״̬��Ϣ
				getCurTime(currTime);
				memset(sql,0,sizeof(sql));
				sprintf(sql,"update d_sch_end set deal_flag='%c',input_count='%d',mainflow_count='%d',dealendtime='%s' where filename='%s' and source_id='%s'",deal_flag,record_num,ret,currTime,orgFileName,m_szSourceID);
				writeSQL(sql);

				record_num = 0;				//�ļ���¼����0
				
			}
			else													
			{
				theJSLog<<"�Ƿ��Ļ�����״̬:"<<pkg.getBlkPos()<<endi;
			}		

		}
		else		//���λ������쳣
		{
			theJSLog<<"�������ļ�["<<m_szFileName<<"]״̬�쳣"<<endi;

			getCurTime(currTime);
			char erro_sql_tmp[1024];
			memset(erro_sql_tmp,0,sizeof(erro_sql_tmp));
			strcpy(erro_sql_tmp,erro_sql);
			strcat(erro_sql_tmp,".tmp");

			ofstream erroFile(erro_sql_tmp,ios::app);	//׷�ӷ�ʽд����Ǽ���Ϣ�ļ�,��д��ʱ��
			if(!erroFile)
			{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"Ontask() ���ļ�%sʧ�ܣ����� ",erro_sql_tmp);
					theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��
					return -1;
			}
			
			//CF_MemFileI _infile;
			//_infile.Init(m_szOutTypeId);
			//CFmt_Change fmts;			
			//fmts.Init(m_szOutTypeId);

			char errocode[10],erro_col[10],errline[10],errseq[10],source_id[5];

			theJSLog<<"�Ǽǻ���������¼������SQL�ļ�"<<endi;
			for(int k = 1;k<=pkg.getRecordNum();k++)
			{
				PkgFmt fmt = pkg.readPkgRecord(k);
				if(strcmp(fmt.status , "E") == 0)
				{		
						//cout<<"��¼ֵ��"<<fmt.record<<endl;
					    outrcd.Set_record(fmt.record);				
						//memset(source_id,0,sizeof(source_id));
						memset(errocode,0,sizeof(errocode));
						memset(erro_col,0,sizeof(erro_col));
						memset(errline,0,sizeof(errline));
						memset(errseq,0,sizeof(errseq));				
						memset(sql,0,sizeof(sql));

						//outrcd.Get_Field("SourceID",source_id);
						outrcd.Get_Field(ERR_CODE,errocode);
						outrcd.Get_Field(ERR_COLINDEX,erro_col);
						outrcd.Get_Field(LINE_NUM,errline);
						outrcd.Get_Field(FILE_ID,errseq);

						cout<<"дsql�ļ�����¼�����¼�е���Ϣ"<<"����ԴID: "<<source_id<<" �����룺"<<errocode<<"  ������"<<erro_col<<"  �����к�"<<errline<<" �����ļ����к�"<<errseq<<endl;
						sprintf(sql,"insert into D_ERRFILE_INFO(filename,source_id,deal_time,err_msg,err_code,err_col,err_line,err_seq,state)"
							  "values('%s','%s','%s','%c',%s,%d,%d,%d,'%c')",orgFileName,m_szSourceID,currTime,'R',errocode,atoi(erro_col),atoi(errline),atoi(errseq),'W');
						erroFile<<sql<<endl;
				}
				
			}

			erroFile.close();
			
			//����ʱ�ļ�����
			if(rename(erro_sql_tmp,erro_sql))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"����sql��ʱ�ļ�[%s]������ʽʧ��: %s",erro_sql_tmp,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_RENAME_ERR,erro_msg);
			}

			//��ʾ����һ���ļ�һ��������	//�����������Ѿ������һ��
			if((strcmp(block_pos,"D") == 0) || (strcmp(block_pos,"E") == 0))		
			{
					commitErrMsg();
					
					record_num = 0;				//�ļ���¼����0

					char bak_time[15],erro_dir[1024],bak_dir[1024];

					memset(bak_time,0,sizeof(bak_time));
					memset(erro_dir,0,sizeof(erro_dir));
					memset(bak_dir,0,sizeof(bak_dir));	
					memset(sql,0,sizeof(sql));

					//�Ӹ�ʽ��ԭʼ�ļ�Ŀ¼���ó��ŵ�����Ŀ¼,ͨ������ʱ�����
					sprintf(sql,"select deal_time from D_SCH_FORMAT where source_id='%s' and filename='%s' and deal_flag='Y'",m_szSourceID,orgFileName);
					Statement stmt = conn.createStatement();
					stmt.setSQLString(sql);
					if((stmt.execute()))
					{
							stmt>>bak_time;
							strcpy(erro_dir,it->second.szSourcePath);
							strcat(erro_dir,erro_path);
							strcat(erro_dir,orgFileName);
						
							strcpy(bak_dir,it->second.szSourcePath);
							strcat(bak_dir,bak_path);
							strncat(bak_dir,bak_time,6);
							strcat(bak_dir,"/");
							strncat(bak_dir,bak_time+6,2);
							strcat(bak_dir,"/");
							strcat(bak_dir,orgFileName);
							
							theJSLog<<"��ԭʼ�ļ��ӱ���Ŀ¼"<<bak_dir<<"�ϴ�������Ŀ¼"<<erro_dir<<endi;
							link(bak_dir,erro_dir);  //����һ���ļ�������Ŀ¼
					}
					else
					{
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"û�дӸ�ʽ���ǼǱ����ҵ��ļ�[%s]",orgFileName);
							theJSLog.writeLog(LOG_CODE_TABLE_QUERY_ERR_CODE,erro_msg);
							return -1;
					}
					
					if(strcmp(block_pos,"E") == 0)
					{
						theJSLog<<"ɾ����ʱ����Ļ������ļ�"<<endi;
						if(remove(tmp))
						{
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"��ʱ�ļ�[%s]ɾ��ʧ��: %s",tmp,strerror(errno));
							theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
						}

						file_status = 0;
					}
			}
			else						//������������м���߿�ʼ S,M
			{
					file_status = -1;
			}		
	
		}
		
		conn.close();
	}
	catch (SQLException e )
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"OnTask() ���ݿ�����쳣: %s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
		return -1;
	}
	catch (jsexcp::CException &e) 
	{	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"OnTask() : %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		//�ֶ�ת������
		return -1;
	}

   //sleep(3);

   return ret;
}

//�ӽ����˳�ǰ�Ĵ���
void Write_File::onChildExit()
{
    cout<<"ģ��3�ӽ����˳�"<<endl;
}

//�����ӽ����������������̵Ĵ���
int Write_File::onTaskOver(int child_ret)
{
   cout<<"ģ��3�ӽ����Ѵ���������,���ӽ�������ļ�¼��"<<child_ret<<endl;
   return  child_ret ;

}


//д�ļ�
int Write_File::writeFile(char* fileName,PkgBlock pkg)
{
		int ret = 0;
		char tmp[1024];
		memset(tmp,0,sizeof(tmp));

		ofstream out(fileName);
		if(!out)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"writeFile �ļ�%s�򿪳���",fileName);
			theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��
			return -1;
		}

		for(int k = 1;k<=pkg.getRecordNum();k++)
		{
			PkgFmt fmt = pkg.readPkgRecord(k);
			out<<fmt.record<<"\n";
						
		}		
			
		out.close();		
		return ret ;
}

//�ļ���⣬ͨ������Դ�ҵ���Ӧ�ı� 0��ʾ����,�����ʾ�쳣
int  Write_File::indb(char* file,char* name)
{
	int ret = -1;
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

	theJSLog<<"�ļ�["<<name<<"]���"<<endi;

	map< string,CF_CError_Table>::const_iterator iter =  mapTabConf.find(m_szSourceID);
	if( iter == mapTabConf.end())
	{
		sprintf(erro_msg,"indb() ���ݿ���û������Դ%s��������Ϣ",m_szSourceID);
		theJSLog.writeLog(LOG_CODE_PARAM_INFO_LACK,erro_msg);	
		return -1;
	}
	
	Statement stmt = conn.createStatement();

	try
	{
	
	ifstream in(file,ios::in) ;
	if(!in)
	{
		sprintf(erro_msg,"indb() �ļ�%s�򿪳���",file);
		theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��

		return -1;
	}
	
	CF_CError_Table tab = iter->second;
	tab.setFileName(name,m_szSourceID,stmt);  
		
	memset(szBuff,0,sizeof(szBuff));
	//CFmt_Change rc;
	//rc.Init(m_szOutTypeId);
	
	while(in.getline(szBuff,sizeof(szBuff)))   
	{		
			outrcd.Set_record(szBuff);
			tab.dealInsertRec(outrcd,NULL,NULL,NULL);
			ret++;
			//if(ret == 8)  
			//{
			//	cout<<"�ļ����ʧ�ܣ�"<<endl;
			//	return ret ;
			//}
			memset(szBuff,0,sizeof(szBuff));		
	}	

	tab.commit();			//��ֹû�ﵽ��¼��������������ǿ�Ʋ���	
	stmt.close();

	in.close();

	}
	catch (jsexcp::CException e)
	{
		sprintf(erro_msg,"indb() error: %s",e.GetErrMessage());
		theJSLog.writeLog(0,erro_msg);
		stmt.close();
		//in.close();
		return ret ;
	}
	catch(SQLException e)
	{
		sprintf(erro_msg,"indb() error: %s [%s]",e.what(),szBuff);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);
    	stmt.rollback();
		//in.close();
    	return ret ;
    }
	

	return 0;
}

//�ύ�����¼��Ϣ,ɨ�����sql�ļ������뵽�������
int Write_File::commitErrMsg()
{
	int rett = 0;
	char szBuff[1024];
	ifstream in(erro_sql,ios::in) ;
	if(!in)
	{
		sprintf(erro_msg,"commitErrMsg �ļ�%s�򿪳���",erro_sql);
		theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��
		return -1;
	}
	
	theJSLog<<"�ύ�����¼��Ϣ���ǼǱ�"<<endi;
	
	memset(szBuff,0,sizeof(szBuff));
	while(in.getline(szBuff,sizeof(szBuff)))   
	{	
		writeSQL(szBuff);
		memset(szBuff,0,sizeof(szBuff));		
	}

	in.close();

	in.open(erro_sql,ios::trunc);	//����ļ�
	in.close();

	return rett;

}

//��ָ��·��ɾ��ָ����ʽ���ļ�
int Write_File::deleteFile(char* path,char* filter)
{
	char tmp[512],fileName[1024];
	int rett = 0;
	memset(tmp,0,sizeof(tmp));
	
	strcpy(tmp,filter);
	strcat(tmp,"*");
	scan.openDir(path);
	while(1)
	{
		memset(fileName,0,sizeof(fileName));
		rett = scan.getFile(filter,fileName);  				
		if(rett == 100)
		{	
			cout<<"������ָ����ʽ"<<filter<<"���ļ�"<<endl;
			return 0;
		}
		if(rett == -1)
		{
			return -1 ;			//��ʾ��ȡ�ļ���Ϣʧ��
		}
		
		theJSLog<<"ɾ��ɨ�赽���ļ�"<<fileName<<endi;
		remove(fileName);
		
	}
	
	return rett;
}

//���ֳ�ʼ��
bool Write_File::drInit()
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
int Write_File::drVarGetSet(char* serialString)
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
 bool Write_File::IsAuditSuccess(const char* dealresult)
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

bool Write_File::CheckTriggerFile()
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
	cout<<"*            writeFile                       * "<<endl;
	cout<<"*              Version 1.0.0	                * "<<endl;
	cout<<"*    last update time :  2013-08-29 by  hed	* "<<endl;
	cout<<"********************************************** "<<endl;


	Write_File fm ;
 
	if( !fm.init( argc, argv ) )
	{
     return -1;
	}
	
	//fm.indb("/mboss/jtcbs/zbjs1_a/data/service/HED/HD/end_path/HDC.2013.201307181148");
	//fm.indb("/mboss/jtcbs/zbjs1_a/data/service/HED/HD/end_path/HDC.2013.201307301230","HDC.2013.201307301230");

	fm.run();

	return 0;
    

}


