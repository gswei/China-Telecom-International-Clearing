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
//SGW_RTInfo	rtinfo;

Write_File::Write_File()
{  
	file_status = 0;
	record_num = 0;
	file_id = 0;

	memset(mServCatId,0,sizeof(mServCatId));
	memset(m_szSourceID,0,sizeof(m_szSourceID));
	memset(m_szFileName,0,sizeof(m_szFileName));
	memset(sql,0,sizeof(sql));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(currTime,0,sizeof(currTime));
}


Write_File::~Write_File()
{
	
}


//ģ���ʼ������
bool Write_File::init(int argc,char** argv)
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
	//rtinfo.getDBSysMode(petri_status);
	//cout<<"petri status:"<<petri_status<<endl;

	//*********2013-07-15 ��ȡ���ݿ��������Ϣ��������Դ�飬����Ŀ¼ 2013-03-11 add by hed*********************************************/
	
	if(!(dbConnect(conn)))
	{
		cout<<"�������ݿ� connect error."<<endl;	//д��־
		return false ;
	}

	mConfParam.iflowID = getFlowID();
	mConfParam.iModuleId = getModuleID(); 

	try{

		string sql = "select source_group from TP_BILLING_LINE  where billing_line_id = :1";
		Statement stmt = conn.createStatement();
		stmt.setSQLString(sql);
		stmt<<mConfParam.iflowID;
		stmt.execute();
		if(!(stmt>>mConfParam.szSrcGrpID))
		{
			cout<<"����tp_billing_line����������ˮ��["<<mConfParam.iflowID<<"]������Դ��"<<endl;
			return false ;
		}

		sql = "select ext_param from TP_PROCESS where billing_line_id = :1 and module_id = :2";
		stmt.setSQLString(sql);
		stmt<<mConfParam.iflowID<<mConfParam.iModuleId;
		stmt.execute();
		if(!(stmt>>mConfParam.szService))
		{
			cout<<"����tp_process���ֶ�ext_param������ģ��["<<mConfParam.iModuleId<<"]service"<<endl;
			return false ;
		}
		
		theJSLog.setLog(szLogPath,szLogLevel,mConfParam.szService, mConfParam.szSrcGrpID, 001);
		
		theJSLog<<"szLogPath="<<szLogPath<<"	szLogLevel="<<szLogLevel<<endi;
		theJSLog<<"flowID="<<mConfParam.iflowID<<"	ModuleId="<<mConfParam.iModuleId<<endi;
		theJSLog<<"szSrcGrpID="<<mConfParam.szSrcGrpID<<"	szService="<<mConfParam.szService<<endi;


		sql = "select a.workflow_id,c.output_path,b.input_id,b.output_id,c.log_tabname from C_SOURCE_GROUP_DEFINE a,C_SERVICE_FLOW b,C_SERVICE_INTERFACE c where a.source_group=:1" 
                    "and a.workflow_id = b.workflow_id and b.service=:2 and b.input_id = c.interface_id ";
		stmt.setSQLString(sql);
		stmt<<mConfParam.szSrcGrpID<<mConfParam.szService;
		stmt.execute();
		if(!(stmt>>mConfParam.iWorkflowId>>mConfParam.szOutPath>>mConfParam.iInputId>>mConfParam.iOutputId>>mConfParam.szSchCtlTabname))
		{
			theJSLog<<"C_SOURCE_GROUP_DEFINE,C_SERVICE_INTERFACE,C_SERVICE_INTERFACE������ѯʧ��:"<<sql<<endw;		
			return false ;
		}
		completeDir(mConfParam.szOutPath);
		
		theJSLog<<"WorkflowId="<<mConfParam.iWorkflowId<<"	InputId="<<mConfParam.iInputId
				<<"	 OutputId="<<mConfParam.iOutputId<<"	sch_table="<<mConfParam.szSchCtlTabname<<endi;

		sql = "select var_value from c_process_env where varname = 'WR_FILE_ERR_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<mConfParam.szSrcGrpID<<mConfParam.szService;
		stmt.execute();
		if(!(stmt>>mConfParam.szErroPath))
		{
				theJSLog<<"���ڱ�c_process_env������д�ļ�ģ��Ĵ���·�� WR_FILE_ERR_DIR"<<endw; //д��־
				return false;
		}		
		completeDir(mConfParam.szErroPath);
	
		sql = "select var_value from c_process_env where varname = 'WR_FILE_BAK_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<mConfParam.szSrcGrpID<<mConfParam.szService;
		stmt.execute();
		if(!(stmt>>mConfParam.szSrcBakPath))
		{
			theJSLog<<"���ڱ�c_process_env������д�ļ�ģ��ı���Ŀ¼(Դ�ļ�Ŀ¼) WR_FILE_BAK_DIR"<<endw;		//д��־
			return false;
		}
		completeDir(mConfParam.szSrcBakPath);
		
		theJSLog<<"szOutPath="<<mConfParam.szOutPath<<"  szErroPath="<<mConfParam.szErroPath<<" szSrcBakPath="<<mConfParam.szSrcBakPath<<endi;

		stmt.close();
	   
	   }catch(SQLException e)
		{
			theJSLog<<"��ʼ��ʱ���ݿ��ѯ�쳣:"<<e.what()<<endw;
			return false ;
		} 

	//��ʼ���ڴ���־�ӿ�
	bool bb = initializeLog(argc,argv,false);  //�Ƿ����ģʽ
	if(!bb)
	{
			return false;
	}
	
	//theJSLog.setLog(szLogPath,szLogLevel,m_szService , m_szSrcGrpID, 001);	//�ļ���־�ӿڣ��������ڴ���־�ӿ�
	//theJSLog<<"����Դ�飺"<<m_szSrcGrpID<<"   service:"<<m_szService<<"  ������·��:"<<out_path<<"  ����·��:"<<erro_path
	//		<<"	Դ�ļ�����·��:"<<bak_path<<"	��־·��:"<<szLogPath<<" ��־����:"<<szLogLevel<<" sql���·��:"<<sql_path<<endi;

	if(LoadSourceCfg() == -1) 
	{		
		return false ;  
	}
	
	conn.close();

	char out_dir[JS_MAX_FILEFULLPATH_LEN],erro_dir[JS_MAX_FILEFULLPATH_LEN],bak_dir[JS_MAX_FILEFULLPATH_LEN],other_dir[JS_MAX_FILEFULLPATH_LEN];
	int rett = 0;
	DIR *dirptr = NULL; 
	for(map<string,SOURCECFG>::const_iterator iter = m_SourceCfg.begin();iter !=m_SourceCfg.end(); ++iter)
	{		   				
			memset(out_dir,0,sizeof(out_dir));
			strcpy(out_dir,iter->second.szSourcePath);
			strcat(out_dir,mConfParam.szOutPath);
			if((dirptr=opendir(out_dir)) == NULL)
			{
					theJSLog<<"����Դ��"<<iter->first<<"��������ļ�·��: "<<out_dir<<"�����ڣ����д���"<<endw;
					rett = mkdir(out_dir,0755);
					if(rett == -1)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"����Դ[%s]������ļ��ļ�·��[%s]�����ڣ����д���ʧ��",iter->first,out_dir);
						theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����
						return false;
					}
			}else closedir(dirptr);
			
			memset(erro_dir,0,sizeof(erro_dir));
			strcpy(erro_dir,iter->second.szSourcePath);
			strcat(erro_dir,mConfParam.szErroPath);
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
					
			memset(bak_dir,0,sizeof(bak_dir));
			strcpy(bak_dir,iter->second.szSourcePath);
			strcat(bak_dir,mConfParam.szSrcBakPath);
			if((dirptr=opendir(bak_dir)) == NULL)
			{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"����Դ[%s]��ԭʼ�����ļ�·��[%s]������",iter->first,bak_dir);
					rett = mkdir(bak_dir,0755);
					//theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����
					//return false ;

			}else  closedir(dirptr);
	  
	}

   theJSLog<<"��ʼ�����...\n"<<endi;

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
		stmt<<mConfParam.szSrcGrpID;
		if(stmt.execute())
		{
			stmt>>mConfParam.szOutputFiletypeId;
		}
		
		theJSLog<<"szOutputFiletypeId="<<mConfParam.szOutputFiletypeId<<endi;
		outrcd.Init(mConfParam.szOutputFiletypeId);

   		//expTrace(szDebugFlag, __FILE__, __LINE__,"%s;%s",m_szSrcGrpID,m_szOutTypeId);
		sql = "select count(*) from C_SOURCE_GROUP_CONFIG where SOURCE_GROUP=:1";
		stmt.setSQLString(sql);
		stmt<<mConfParam.szSrcGrpID;
		if(stmt.execute())
		{
			stmt>>iSourceCount;
		}
		
		theJSLog<<"iSourceCount="<<iSourceCount<<endi;
		//expTrace(szDebugFlag, __FILE__, __LINE__, "iSourceCount=%d;", iSourceCount);
		
		sql = "select a.source_id,b.source_path,b.SERV_CAT_ID from C_SOURCE_GROUP_CONFIG a,I_SOURCE_DEFINE b where SOURCE_GROUP=:1 and a.source_id=b.source_id";
		stmt.setSQLString(sql);
		stmt<<mConfParam.szSrcGrpID;
		if(stmt.execute())
		{
			for (int i=0; i<iSourceCount; i++)
			{
				SOURCECFG SourceCfg;
				string strSourceId;

				stmt>>SourceCfg.szSourceId>>SourceCfg.szSourcePath>>SourceCfg.serverCatID;      
				strSourceId=SourceCfg.szSourceId;
				
				completeDir(SourceCfg.szSourcePath);

				if(getSourceFilter(SourceCfg.szSourceId,SourceCfg.filterRule,SourceCfg.file_begin,SourceCfg.file_length))
				{
							return -1;
				}
				
				m_SourceCfg[strSourceId]=SourceCfg;

				theJSLog<<"szSourceId="<<SourceCfg.szSourceId<<" szSourcePath="<<SourceCfg.szSourcePath<<" serverCatID="<<SourceCfg.serverCatID
						<<" filterRule="<<SourceCfg.filterRule<<"  filetime_begin="<<SourceCfg.file_length<<"  filetime_length="<<SourceCfg.file_length<<endi;
		     }
		}
		
	}catch (SQLException e)
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"LoadSourceCfg() ���ݿ����%s (%s)",e.what(),sql);
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

//�ж��Ƿ�Ҫ���뻰����
int Write_File::onBeforeTask()
{
	//cout<<"���뻰����"<<endl;
	theJSLog.reSetLog();
	sleep(1);

	return 1;		//Ĭ��Ҫ���뻰����
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

//��ȡ��ʽ������ʱ��Ŀ¼,����30�����ڵ�Ŀ¼�Ƿ����Դ�ļ�
int Write_File::getBackTimeDir(char* date_dir,char* orgFileName)  
{
	int ret = -1;
	char date[8+1],tmp_date[8+1],tmp[JS_MAX_FILEFULLPATH_LEN];
	memset(tmp,0,sizeof(tmp));
	memset(date,0,sizeof(date));
	memset(tmp_date,0,sizeof(tmp_date));
	getCurTime(currTime);
	
	strncpy(date,currTime,8);		//20130916
	date[8] = '\0';
	strcpy(tmp_date,date);

	for(int i = 0;i<31;i++)
	{
		memset(tmp,0,sizeof(tmp));
		strcpy(tmp,it->second.szSourcePath);
		strcat(tmp,mConfParam.szSrcBakPath);

		strncat(tmp,tmp_date,6);
		strcat(tmp,"/");
		strncat(tmp,tmp_date+6,2);
		strcat(tmp,"/");
		strcat(tmp,orgFileName);

		if(access(tmp,F_OK) == 0) 
		{
			theJSLog<<"�Ӹ�ʽ������Ŀ¼�ҵ����󻰵��ļ�["<<tmp<<"]"<<endi;
			ret = 0;
			strcpy(date_dir,tmp_date);
			return ret;
		}
		
		addDays(-1,date,tmp_date);
		strcpy(date,tmp_date);
	}
	
	return ret;
}


//����ɹ����ػ�������(>=0)
int Write_File::onTask(void *task_addr, int offset, int ticket_num)
{

    //cout<<"�����ַ"<<task_addr<<endl;		 	
	PkgBlock pkg((char*)task_addr);
	pkg.init(getTicketLength(),getBlockSize());
	
	pkg.setModuleId(getModuleID());

	if(pkg.getRecordNum() == 0)
	{
		theJSLog<<"�������¼����Ϊ0,����..."<<endw;
		return ticket_num;
	}

	int ret = 0;
	char tmp[JS_MAX_FILEFULLPATH_LEN],path[JS_MAX_FILEFULLPATH_LEN],orgFileName[JS_MAX_FILENAME_LEN],full_name[JS_MAX_FILEFULLPATH_LEN],block_pos[2];

	memset(tmp,0,sizeof(tmp));
	memset(path,0,sizeof(path));
	memset(orgFileName,0,sizeof(orgFileName));
	memset(full_name,0,sizeof(full_name));
	memset(block_pos,0,sizeof(block_pos));

	strcpy(block_pos,pkg.getBlkPos());  //��ȡ������λ��

	record_num += pkg.getRecordNum();
	
	strcpy(m_szSourceID,pkg.getSourceId());
	it  = m_SourceCfg.find(string(m_szSourceID));
	if(it == m_SourceCfg.end())
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"�����в���������Դ[%s]����Ϣ",m_szSourceID);
		theJSLog.writeLog(LOG_CODE_SOURCE_NOT_FIND,erro_msg);
		return -1;
	}
	
	strcpy(m_szFileName,pkg.getFileName());
	strcpy(path,it->second.szSourcePath);
	strcat(path,mConfParam.szOutPath);
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

 try
 {	
	//��ʾ�����ļ���ʼ��,��ʵǰ�����Ϣ�������ڻ����鿪ʼʱȡ��,����ÿ�������鶼ȥȡ
	if((strcmp(block_pos,"S") == 0) || (strcmp(block_pos,"D") == 0))
	{
		fileOverFlag = false;

		theJSLog<<"######## start deal file " <<orgFileName<<" ########"<<endi;

		char fileid[10];
		memset(fileid,0,sizeof(fileid));
		outrcd.Set_record(pkg.readPkgRecord(1).record);
		outrcd.Get_Field(FILE_ID,fileid);
		file_id = atol(fileid);
		setSQLFileName(orgFileName);
		memset(file_time,0,sizeof(file_time));			//���ļ��������ȡʱ��
		strncpy(file_time,orgFileName+it->second.file_begin,it->second.file_length);
		file_time[8] = '\0';
		strcpy(mServCatId,it->second.serverCatID);

		theJSLog<<"source_id:"<<m_szSourceID<<" file_time:"<<file_time <<" file_id:"<<file_id<<endi;
	}
	theJSLog<<"��������,split_filename:"<<m_szFileName<<" status:"<<pkg.getStatus()<<" record_num:"<<pkg.getRecordNum()<<" pos:"<<pkg.getBlkPos()<<endi;

	strcat(full_name,orgFileName);
	strcpy(tmp,full_name);	 //��ʽ�ļ�
	strcat(tmp,".tmp");		//��ʱ�ļ�
	
	//�жϻ������Ƿ���ȷ������ȷ����д��ʱ�ļ���
	//������ȷ������ɾ����ǰ�ĸû������ļ���дsql�쳣��¼��Ϣ��Ȼ��ѭ���������ÿ����¼��������Ϣ�Ǽ�	
	//2013-07-17 �жϻ���������ļ���λ��ͨ���������ͷ������ж�S��ʾ��ʼ E���� M�м� D����
	//���쳣���˵����ݿ�Ǽ�ERRFILE_INFO���ļ���������Դ��ʱ�䡢����ԭ��F/R���������롢�����С���������š�
	//����״̬---�ɳ���ĳ������Ǽ�	�ڲ���ʽд�������ֶΣ�LINE_NUM,ERR_CODE,ERR_LINE,FILE_ID
	
		if(pkg.getStatus() == 2)					 //����������
		{
			if((strcmp(block_pos,"S") == 0) || (strcmp(block_pos,"M") == 0))
			{
				if(strcmp(block_pos,"S") == 0)				//��һ��ģ��
				{
					ofstream out(tmp);						//��ֹ�����쳣���ļ�����ͬ�����,����ļ�
					if(!out)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"Ontask() �ļ�[%s]�򿪳���",tmp);
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
				else
				{
					if(file_status == 0)
					{
						ofstream out(tmp,ios::app);
						if(!out)
						{
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"Ontask() �ļ�[%s]�򿪳���",tmp);
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
				}
				
			}
			else if(strcmp(block_pos,"E") == 0)					//����������Ѿ������һ����
			{	
				if(file_status == -1)
				{
					//theJSLog<<"ɾ����ʱ����Ļ������ļ�"<<endi; �п�����ʱ�ļ���û����,���ж��ļ��Ƿ����
					if(access(tmp,F_OK) == 0)
					{
						if(remove(tmp))
						{
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"��ʱ�ļ�[%s]ɾ��ʧ��: %s",tmp,strerror(errno));
							theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
						}
					}

					//�Ӹ�ʽ��ԭʼ�ļ�Ŀ¼���ó��ŵ�����Ŀ¼
					char bak_time[8+1],erro_dir[JS_MAX_FILEFULLPATH_LEN],bak_dir[JS_MAX_FILEFULLPATH_LEN];
					memset(bak_time,0,sizeof(bak_time));
					memset(erro_dir,0,sizeof(erro_dir));
					memset(bak_dir,0,sizeof(bak_dir));				
					//2013-09-16
					if(getBackTimeDir(bak_time,orgFileName) == 0)
					{	
						strcpy(erro_dir,it->second.szSourcePath);
						strcat(erro_dir,mConfParam.szErroPath);
						strcat(erro_dir,orgFileName);
						
						strcpy(bak_dir,it->second.szSourcePath);
						strcat(bak_dir,mConfParam.szSrcBakPath);
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
						sprintf(erro_msg,"�޷��ӱ���Ŀ¼[%s]���ҵ�Դ�ļ�[%s]",mConfParam.szSrcBakPath,orgFileName);
						theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);
					}

					//��ʹ�Ǵ��󻰵���ҲҪ���Ǽǵ��ȱ� 2013-09-02
					getCurTime(currTime);
					memset(sql,0,sizeof(sql));							
					sprintf(sql,"insert into %s (source_id,serv_cat_id,filename,deal_flag,dealtime,input_count,file_id,file_time) values('%s','%s','%s','E','%s',%d,%ld,'%s')",mConfParam.szSchCtlTabname,m_szSourceID,mServCatId,orgFileName,currTime,record_num,file_id,file_time);
					writeSQL(sql);

					theJSLog<<"�ύsql�ļ�..."<<endi;
					commitSQLFile();
				/*	
					//2013-10-24
					rtinfo.getDBSysMode(petri_status);
					if(petri_status == DB_STATUS_OFFLINE)
					{
						theJSLog<<"���ݿ�״̬Ϊֻ��̬,дsql�ļ�"<<endi;
						commitSQLFile();
					}
					else
					{
						if(!(dbConnect(conn)))
						{
							commitSQLFile();
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"onTask() �������ݿ�ʧ��,дsql�ļ�");
							theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
						}
						else
						{
							theJSLog<<"�������ݿ�ǼǱ�..."<<endi;
							vsql = getvSQL();
							updateDB();
							conn.close();
						}	
 					}	
				*/
					file_status = 0;
				}
				else					//����
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
				
					if(rename(tmp,full_name))				//����ʱ�ļ������ʽ�ļ�
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"��ʱ�ļ�[%s]����ʧ��: %s",tmp,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_RENAME_ERR,erro_msg); // �ļ���������
					}
					else
					{
						theJSLog<<"д�ļ�"<<orgFileName<<"�ɹ�,��鿴�ļ���·��:"<<path<<endi;
					}		
	
					//�����¼ֵ�����ȱ�
					getCurTime(currTime);
					memset(sql,0,sizeof(sql));							
					sprintf(sql,"insert into %s (source_id,serv_cat_id,filename,deal_flag,dealtime,input_count,file_id,file_time) values('%s','%s','%s','Y','%s',%d,%ld,'%s')",mConfParam.szSchCtlTabname,m_szSourceID,mServCatId,orgFileName,currTime,record_num,file_id,file_time);
					writeSQL(sql);
					
					theJSLog<<"�ύsql�ļ�..."<<endi;
					commitSQLFile();
				   /*
					//2013-10-24
					rtinfo.getDBSysMode(petri_status);
					if(petri_status == DB_STATUS_OFFLINE)
					{
						theJSLog<<"���ݿ�״̬Ϊֻ��̬,дsql�ļ�"<<endi;
						commitSQLFile();
					}
					else
					{
						if(!(dbConnect(conn)))
						{
							theJSLog<<"�������ݿ�ʧ��,дsql�ļ�"<<endw;
							commitSQLFile();
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"onTask() �������ݿ�ʧ��,дsql�ļ�");
							theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
						}
						else
						{
							theJSLog<<"�������ݿ�ǼǱ�..."<<endi;
							vsql = getvSQL();
							updateDB();
							conn.close();
						}	
 					}	
					*/
				}	
				record_num = 0;  //�ļ���¼����0
				theJSLog<<"######## end deal file ########\n"<<endi;
			}
			else if(strcmp(block_pos,"D") == 0)	//��ʾ����һ���ļ�һ�������� ������
			{	
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
				
				if(rename(tmp,full_name))				//����ʱ�ļ������ʽ�ļ�
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"��ʱ�ļ�[%s]����ʧ��: %s",tmp,strerror(errno));
					theJSLog.writeLog(LOG_CODE_FILE_RENAME_ERR,erro_msg); // �ļ���������
				}
				else
				{
					theJSLog<<"д�ļ�"<<orgFileName<<"�ɹ�,��鿴�ļ���·��:"<<path<<endi;
				}

				//�����¼ֵ�����ȱ�
				getCurTime(currTime);
				memset(sql,0,sizeof(sql));		
				sprintf(sql,"insert into %s (source_id,serv_cat_id,filename,deal_flag,dealtime,input_count,file_id,file_time) values('%s','%s','%s','Y','%s',%d,%ld,'%s')",mConfParam.szSchCtlTabname,m_szSourceID,mServCatId,orgFileName,currTime,record_num,file_id,file_time);
				writeSQL(sql);
				
				theJSLog<<"�ύsql�ļ�..."<<endi;
				commitSQLFile();
			    /*
				//2013-10-24
				rtinfo.getDBSysMode(petri_status);
				if(petri_status == DB_STATUS_OFFLINE)
				{
					theJSLog<<"���ݿ�״̬Ϊֻ��̬,дsql�ļ�"<<endi;
					commitSQLFile();
				}
				else
				{
					if(!(dbConnect(conn)))
					{
						theJSLog<<"�������ݿ�ʧ��,дsql�ļ�"<<endw;
						commitSQLFile();
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"onTask() �������ݿ�ʧ��,дsql�ļ�");
						theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
					}
					else
					{
						theJSLog<<"�������ݿ�ǼǱ�..."<<endi;
						vsql = getvSQL();
						updateDB();
						conn.close();
					}	
 				}		
				*/
				record_num = 0;				//�ļ���¼����0
				theJSLog<<"######## end deal file ########"<<endi;
			}
			else													
			{
				theJSLog<<"�Ƿ��Ļ�����״̬:"<<pkg.getBlkPos()<<endw;
			}		

		}
		else if(pkg.getStatus() == 3)		//���λ������쳣
		{
			theJSLog<<"�������ļ�["<<m_szFileName<<"]״̬�쳣"<<endw;

			char errocode[10],erro_col[10],errline[10],errseq[10],source_id[5+1];		
			getCurTime(currTime);
			theJSLog<<"�Ǽǻ���������¼������SQL�ļ�"<<endi;
			for(int k = 1;k<=pkg.getRecordNum();k++)
			{
				PkgFmt fmt = pkg.readPkgRecord(k);
				if(strcmp(fmt.type , "E") == 0)
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

						//cout<<"��¼�����¼�е���Ϣ"<<"����ԴID: "<<source_id<<" �����룺"<<errocode<<"  ������"<<erro_col<<"  �����к�"<<errline<<" �����ļ����к�"<<errseq<<endl;
						sprintf(sql,"insert into D_ERRFILE_INFO(filename,source_id,deal_time,err_msg,err_code,err_col,err_line,err_seq,state)"
							  "values('%s','%s','%s','%c','%s',%d,%d,%d,'%c')",orgFileName,m_szSourceID,currTime,'R',errocode,atoi(erro_col),atoi(errline),atoi(errseq),'W');
						writeSQL(sql);
				}
				
			}

			//��ʾ����һ���ļ�һ��������	//�����������Ѿ������һ��
			if((strcmp(block_pos,"D") == 0) || (strcmp(block_pos,"E") == 0))		
			{
					if(strcmp(block_pos,"E") == 0)
					{
						//theJSLog<<"ɾ����ʱ����Ļ������ļ�"<<endi; �п�����ʱ�ļ���û����,���ж��ļ��Ƿ����
						if(access(tmp,F_OK) == 0)
						{			
							if(remove(tmp))
							{
								memset(erro_msg,0,sizeof(erro_msg));
								sprintf(erro_msg,"��ʱ�ļ�[%s]ɾ��ʧ��: %s",tmp,strerror(errno));
								theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
							}
						}

						file_status = 0;
					}

					//��ʹ�Ǵ��󻰵���ҲҪ���Ǽǵ��ȱ� 2013-09-02
					getCurTime(currTime);
					memset(sql,0,sizeof(sql));							
					sprintf(sql,"insert into %s (source_id,serv_cat_id,filename,deal_flag,dealtime,input_count,file_id,file_time) values('%s','%s','%s','E','%s',%d,%ld,'%s')",mConfParam.szSchCtlTabname,m_szSourceID,mServCatId,orgFileName,currTime,record_num,file_id,file_time);
					writeSQL(sql);
					
					record_num = 0;				//�ļ���¼����0

					char bak_time[8+1],erro_dir[JS_MAX_FILEFULLPATH_LEN],bak_dir[JS_MAX_FILEFULLPATH_LEN];
					memset(bak_time,0,sizeof(bak_time));
					memset(erro_dir,0,sizeof(erro_dir));
					memset(bak_dir,0,sizeof(bak_dir));	
					
					if(getBackTimeDir(bak_time,orgFileName) == 0)
					{		
							strcpy(erro_dir,it->second.szSourcePath);
							strcat(erro_dir,mConfParam.szErroPath);
							strcat(erro_dir,orgFileName);
						
							strcpy(bak_dir,it->second.szSourcePath);
							strcat(bak_dir,mConfParam.szSrcBakPath);
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
							sprintf(erro_msg,"�޷��ӱ���Ŀ¼[%s]���ҵ�Դ�ļ�[%s]",mConfParam.szSrcBakPath,orgFileName);
							theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);
					}
					
					theJSLog<<"�ύsql�ļ�..."<<endi;
					commitSQLFile();
				/*
					//2013-10-24
					rtinfo.getDBSysMode(petri_status);
					if(petri_status == DB_STATUS_OFFLINE)
					{
						theJSLog<<"���ݿ�״̬Ϊֻ��̬,дsql�ļ�"<<endi;
						commitSQLFile();
					}
					else
					{
						if(!(dbConnect(conn)))
						{
							theJSLog<<"�������ݿ�ʧ��,дsql�ļ�"<<endw;
							commitSQLFile();
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"onTask() �������ݿ�ʧ��,дsql�ļ�");
							theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
						}
						else
						{
							theJSLog<<"�������ݿ�ǼǱ�..."<<endi;
							vsql = getvSQL();
							updateDB();
							conn.close();
						}	
 					}			
				*/
					theJSLog<<"######## end deal file ########\n"<<endi;
			}
			else										//������������м���߿�ʼ S,M
			{
					file_status = -1;
			}		
	
		}
		else if(pkg.getStatus() == -1)					//��ʾ�ٲ�ʧ��,һ�������һ��D,E
		{
			theJSLog<<"�ļ�["<<orgFileName<<"]�ٲ�ʧ��,����"<<endi;
			record_num = 0;								//�ļ���¼����0
			if(strcmp(block_pos,"E") == 0)
			{
				//theJSLog<<"ɾ����ʱ����Ļ������ļ�"<<endi; �п�����ʱ�ļ���û����,���ж��ļ��Ƿ����
				if(access(tmp,F_OK) == 0)
				{			
					if(remove(tmp))
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"��ʱ�ļ�[%s]ɾ��ʧ��: %s",tmp,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
					}
				}

				file_status = 0;
			}

			theJSLog<<"######## end deal file ########"<<endi;
		}

	}
	catch (jsexcp::CException &e) 
	{	
		rollBackSQL();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"[%s]OnTask() : %s",orgFileName,e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		//�ֶ�ת������
		//return -1;
		ticket_num = -1;
	}
	
	if((strcmp(block_pos,"D") == 0) || (strcmp(block_pos,"E") == 0))
	{
		fileOverFlag = true;
	}

	pkg.clear();

   return ticket_num;
}

/*
//2013-10-24 �����ύsql,��֤һ������������
int Write_File::updateDB()
{	
	int ret = 0;
	Statement stmt;
	string ssql;
    try
    {	
		stmt = conn.createStatement();
		for(int i =0;i<vsql.size();i++)
		{	
			//cout<<"sql = "<<vsql[i]<<endl;
			ssql = vsql[i];
			stmt.setSQLString(ssql);
			ret = stmt.execute();
		}
		stmt.close();

		rollBackSQL();			//sql���ɹ�ִ��,�ع�sql�ļ�
	}
	catch(util_1_0::db::SQLException e)
	{ 
		stmt.rollback();
		commitSQLFile();		//���쳣ʱдsql�ļ������������

		vsql.clear();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"updateDB ���ݿ����%s (%s),дsql�ļ�",e.what(),ssql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

		return -1;
	}

	vsql.clear();
	
	return ret ;
}
*/

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

int main(int argc,char** argv)
{
    cout<<"********************************************* "<<endl;
	cout<<"*    China Telecom. Telephone Network         "<<endl;
	cout<<"*    InterNational Account Settle System      "<<endl;
	cout<<"*                                             "<<endl;
	cout<<"*           jswritefile                       "<<endl;
	cout<<"*           sys.GJZW.Version 1.0	             "<<endl;
	cout<<"*    created time :      2013-07-01 by  hed   "<<endl;
	cout<<"*    last update time :  2013-12-01 by  hed	 "<<endl;
	cout<<"********************************************* "<<endl;


	Write_File fm ;
 
	if( !fm.init( argc, argv ) )
	{
		return -1;
	}

	fm.run();

	return 0;
    

}


