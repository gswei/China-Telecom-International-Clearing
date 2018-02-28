/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-07-20
File:			 ExceptionOut.cpp
Description:	 �쳣�ļ����ģ��
History:
<table>
revision	author            date                description
--------    ------            ----                -----------
</table>
**************************************************************************/
#include <dirent.h>
#include <string>
//#include <vector>
#include <sys/types.h>
#include <sys/stat.h>  //stat()��������ѯ�ļ���Ϣ
#include<unistd.h>     //��ȡ��ǰ��������Ŀ¼
//#include<iostream>
#include<fstream>
#include "ExceptionOut.h"

CLog theJSLog;
SGW_RTInfo	rtinfo;

ExceptionOut::ExceptionOut()
{  
	petri_status = DB_STATUS_ONLINE;
	vsql.clear();
	memset(m_szFileName,0,sizeof(m_szFileName));
	//memset(input_path,0,sizeof(input_path));
	//memset(out_path,0,sizeof(out_path));
	memset(m_szSourceID,0,sizeof(m_szSourceID));
	memset(outFileName,0,sizeof(outFileName));
	memset(currTime,0,sizeof(currTime));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(sql,0,sizeof(sql));
}


ExceptionOut::~ExceptionOut()
{
	mdrDeal.dr_ReleaseDR();

}

//ģ���ʼ������
bool ExceptionOut::init(int argc,char** argv)
{  
    if(!PS_Process::init(argc,argv))
    {
      return false;
    }

	//*********2013-07-15 ��ȡ���ݿ��������Ϣ��������Դ�飬����Ŀ¼ 2013-03-11 add by hed*********************************************/
	if(!(dbConnect(conn)))
	{
		cout<<"�������ݿ� connect error."<<endl;	//д��־
		return false ;
	}
		
	mConfParam.iflowID = getFlowID();
	mConfParam.iModuleId = getModuleID(); 

	try
	{
		//�������·�����ں�����Դ��û�й�ϵ������������C_GOLABAL_ENV���棬���������·��������Ǿ���·��
		string sql ;
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

		sql = "select varvalue from c_global_env where varname = 'EXCEPTION_INPUT'";
		stmt.setSQLString(sql);
		stmt.execute();
		if(!(stmt>>mConfParam.szInPath))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"�쳣�ļ����ģ���������·��û������,����c_global_env�����ñ���EXCEPTION_INPUT");		
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
			return false ;
		}		
		completeDir(mConfParam.szInPath);
		
		sql = "select varvalue from c_global_env where varname = 'EXCEPTION_OUTPUT'";
		stmt.setSQLString(sql);
		stmt.execute();
		if(!(stmt>>mConfParam.szOutPath))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"�쳣�ļ����ģ�����·��û������,����c_global_env�����ñ��� EXCEPTION_OUTPUT");		
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
			return false ;
		}		
		completeDir(mConfParam.szOutPath);
					
		stmt.close();
		
		theJSLog<<"szInPath="<<mConfParam.szInPath<<"  szOutPath="<<mConfParam.szOutPath<<endi;

	}catch(SQLException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init ���ݿ��ѯ�쳣 %s��%s",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);

		return false ;
	}


	//�ж��쳣˵���ļ����Ŀ¼�Ƿ����
	 DIR *dirptr = NULL; 
	 
	 if((dirptr=opendir(mConfParam.szOutPath)) == NULL)
	 {
		//theJSLog<<"���Ŀ¼"<<mConfParam.szOutPath<<"��ʧ��"<<endw;
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"���Ŀ¼[%s]��ʧ��",mConfParam.szOutPath);
		theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����

		return false ;

	 }else closedir(dirptr);


	//��ʼ���ڴ���־�ӿ�
	bool bb = initializeLog(argc,argv,false);  //�Ƿ����ģʽ
	if(!bb)
	{
		theJSLog.writeLog(0,"��ʼ���ڴ���־�ӿ�ʧ��");
		return false;
	}
	
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

	//��ȡpetri��״̬,��ϵͳΪֻ��̬ʱ,���ݿ���²������д�ļ�	
	if(!(rtinfo.connect()))
	{
		theJSLog.writeLog(0,"��������ʱ��Ϣʧ��");
		return false;
	}
	rtinfo.getDBSysMode(petri_status);
	theJSLog<<"petri status:"<<petri_status<<endw;

	//theJSLog.setLog(szLogPath, szLogLevel,"ERROUT" , "GJJS", 001);	//�ļ���־�ӿڣ��������ڴ���־�ӿ�	
	//theJSLog<<"�������·����"<<input_path<<"  ���·��:"<<out_path<<"	��־·��:"
	//		<<szLogPath<<" ��־����:"<<szLogLevel<<" sql���·��:"<<sql_path<<endi;

	if(LoadSourceCfg() == -1) 
	{		
		return false ;  
	}
	
	conn.close();

	char input_dir[JS_MAX_FILEFULLPATH_LEN];
	int rett = -1;
	for(map<string,SOURCECFG>::const_iterator iter = m_SourceCfg.begin();iter !=m_SourceCfg.end(); ++iter)
	{		   		
			memset(input_dir,0,sizeof(input_dir));
			strcpy(input_dir,iter->second.szSourcePath);
			strcat(input_dir,mConfParam.szInPath);
			if((dirptr=opendir(input_dir)) == NULL)
			{		
					//memset(erro_msg,0,sizeof(erro_msg));
					//sprintf(erro_msg,"����Դ[%s]�������ļ�·��[%s]������",iter->first,input_dir);
					//theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����
					
					theJSLog<<"����Դ��"<<iter->first<<"���������ļ�·��: "<<input_dir<<"�����ڣ����д���"<<endw;
					rett = mkdir(input_dir,0755);
					if(rett == -1)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"����Դ[%s]�Ĵ��󻰵��ļ�����·��[%s]�����ڣ����д���ʧ��",iter->first,input_dir);
						theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����
						return false;
					}
					
					//return false ;
			}else closedir(dirptr);  
	}
	

   theJSLog<<"��ʼ����ϣ�"<<endi;

   return true ;
}


//��������Դ������Ϣ��ȡȫ������Դ�������Դ��Ϣ
int ExceptionOut::LoadSourceCfg()
{
	char m_szSrcGrpID[8];
	int iSourceCount=0;
	string sql ;
	try
	{	
		//string sql = "select FILETYPE_ID from C_SOURCE_GROUP_DEFINE where SOURCE_GROUP=:1";
		//stmt.setSQLString(sql);
		//stmt<<m_szSrcGrpID;
		//if(stmt.execute())
		//{
		//	stmt>>m_szOutTypeId;
		//}

   		//expTrace(szDebugFlag, __FILE__, __LINE__,"%s;%s",m_szSrcGrpID,m_szOutTypeId);
		
		Statement stmt = conn.createStatement();
		Statement stmt2 = conn.createStatement();
		sql = "select source_group from c_source_group_define ";
		stmt2.setSQLString(sql);
		stmt2.execute();
		memset(m_szSrcGrpID,0,sizeof(m_szSrcGrpID));

		while(stmt2>>m_szSrcGrpID)
		{			
			sql = "select count(*) from C_SOURCE_GROUP_CONFIG  where SOURCE_GROUP =:1";
			stmt.setSQLString(sql);
			stmt<<m_szSrcGrpID;
			stmt.execute();
			if(!(stmt>>iSourceCount))
			{
				continue ;
			}

			theJSLog<<"����Դ�飺"<<m_szSrcGrpID<<"  iSourceCount="<<iSourceCount<<endi;
		
			sql = "select a.source_id,b.source_path from C_SOURCE_GROUP_CONFIG a,I_SOURCE_DEFINE b where SOURCE_GROUP=:1 and a.source_id=b.source_id";
			stmt.setSQLString(sql);
			stmt<<m_szSrcGrpID;
			if(stmt.execute())
			{
				for (int i=0; i<iSourceCount; i++)
				{
					SOURCECFG SourceCfg;
					string strSourceId;

					stmt>>SourceCfg.szSourceId>>SourceCfg.szSourcePath;      
					strSourceId=SourceCfg.szSourceId;
			    
					completeDir(SourceCfg.szSourcePath);

					//if(getSourceFilter(SourceCfg.szSourceId,SourceCfg.filterRule) == 0)
					//{
					//cout<<"����Դ��"<<strSourceId<<" û�����ù��˹���"<<endl;
					//	sprintf(erro_msg,"����Դ%sû�����ù��˹���",strSourceId);	//��������δ����
					//	theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
					//	return -1;
					//}
					
					theJSLog<<"szSourceId="<<SourceCfg.szSourceId<<" szSourcePath="<<SourceCfg.szSourcePath<<endi;
					m_SourceCfg[strSourceId]=SourceCfg;
				}
			}
			
			memset(m_szSrcGrpID,0,sizeof(m_szSrcGrpID));
		}

		stmt.close();
		stmt2.close();

	}catch (SQLException e)
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"LoadSourceCfg���ݿ����%s (%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

		//throw jsexcp::CException(0, "LoadSourceCfg�������ݿ����", __FILE__, __LINE__);
		return -1;
	 }
	catch (jsexcp::CException &e) 
	{	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"LoadSourceCfg() %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);
		return -1;
	}

	return 0;
}


//�����ύsql,��֤һ������������
int ExceptionOut::updateDB()
{	
	int ret = 0;
	Statement stmt;
	char ssql[1024];
    try
    {	
		stmt = conn.createStatement();

		for(vector<string>::iterator iter = vsql.begin();iter != vsql.end();++iter)
		{	
			memset(ssql,0,sizeof(ssql));
			strcpy(ssql,(*iter).c_str());
			stmt.setSQLString(ssql);
			ret = stmt.execute();
		}

		stmt.close();
	}
	catch(util_1_0::db::SQLException e)
	{ 
		stmt.rollback();
		stmt.close();

		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"updateDB ���ݿ����%s (%s)",e.what(),ssql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
		ret =  -1;
	}

	vsql.clear();
	
	return ret ;

}


//ɨ������ļ���Ϣ�ǼǱ������쳣˵���ļ�(Դ�ļ���+.err),�������ļ����أ��ļ�����������һ��������Ϣ����¼���������ɶ���
void ExceptionOut::run()
{	
	short db_status = 0;
	int ret = 0,event_sn, event_type,maxAuditNum=200;
	long param1, param2, src_id;
	Statement stmt;

 while(1)
 {	
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
	
	theJSLog.reSetLog();

	try
	{	
		//�ж����ݿ�״̬	
		rtinfo.getDBSysMode(db_status);
		if(db_status != petri_status)
		{
			theJSLog<<"���ݿ�״̬�л�... "<<petri_status<<"->"<<db_status<<endw;
			int cmd_sn = 0;
			if( !putEvt(cmd_sn, EVT_RPT_DBSTATUS, 0, db_status, DSPCH_PRC_ID) )
			{
				theJSLog<<"�������ݿ����״̬ʧ�ܣ�\n"<<endw;
				continue ;
			}
			petri_status = db_status;
		}
		if(petri_status == DB_STATUS_OFFLINE)	continue ;
		
		if(mdrDeal.mdrParam.drStatus == 1)		//����ϵͳ
		{
			//���trigger�����ļ��Ƿ����
			if(!mdrDeal.CheckTriggerFile(m_triggerFile))
			{
				sleep(1);
				continue ;
			}
			
			memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));
			ret = mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
			if(ret)
			{
				theJSLog<<"��ϵͳͬ��ʧ��...."<<endw;
				continue ;
			}
			
			if(!(dbConnect(conn)))
			{
				//dr_AbortIDX();
				mdrDeal.dr_abort();

				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"run() �������ݿ�ʧ�� connect error");
				theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��		
				sleep(30);
				continue  ;
			}

			//��ȡͬ������
			vector<string> data;		
			splitString(mdrDeal.m_SerialString,"|",data,false,false);  //���͵��ַ���: file_id|file_id|
			
			theJSLog<<"######## start deal d_err_file_info ########"<<endi;

			bool flag  = false;
			int counter=0;			//2014-10-23
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select source_id,fileName,err_msg,err_code,err_col,err_line,err_seq from d_errfile_info where state = 'W' and err_seq = :1 order by err_msg,fileName,err_line");		
			
			try
			{
				stmt = conn.createStatement();			//��ϵͳ��Ҫ���Ƽ�¼������?
				stmt.setSQLString(sql);
				
				//int iStatus = dr_GetAuditMode(module_name);
				int iStatus = mdrDeal.mdrParam.aduitMode;
				if(iStatus == 1)		//ͬ��ģʽ,	��ϵͳ�ȴ�ָ��ʱ��
				{	
					int cnt = 0;
					while(cnt < 10)
					{	
						erroinfoMap.clear();
						cnt++;
						counter=0;

						for(int i = 0;i<data.size();i++)		//��ѯÿ���ļ�
						{
							flag = false ;

							stmt<<data[i];
							stmt.execute();

							ERRINFO errinfo;					//���ļ���Ϊ��λ,�����ж�����¼
							while(stmt>>errinfo.source_id>>errinfo.filename>>errinfo.err_msg>>errinfo.err_code>>errinfo.err_col>>errinfo.err_line>>errinfo.err_seq)
							{
								counter++;
								if(counter > maxAuditNum)
								{
									break;
								}

								map< string,vector<ERRINFO> >::iterator  iter= erroinfoMap.find(errinfo.filename);
								if(iter == erroinfoMap.end())
								{
									vector<ERRINFO> errinfoV; 
									errinfoV.push_back(errinfo);
									erroinfoMap.insert(map< string,vector<ERRINFO> >::value_type(errinfo.filename,errinfoV));
									flag = true;
								}
								else
								{
									(iter->second).push_back(errinfo);
								}	
							}
					
							if(!flag)	//���ļ�û���ҵ�,������
							{
								theJSLog<<"�ļ�file_id:"<<data[i]<<"û�ҵ�"<<cnt<<endw;
								sleep(30);
								break;			//�˳�for ���½���while
							}	
						}

						if(flag) 
						{
							theJSLog<<"��d_errfile_info���Ѿ��鵽ȫ���ļ���Ϣ"<<endi;
							break;
						}
					}

					//��������ж��Ƿ���Ҫ�ٲ� ���ļ�û����,������ٲ�Ҳ�ǰ���, ��Ҫ�˹���Ԥ
					if(!flag)  
					{
						//dr_AbortIDX();
						mdrDeal.dr_abort();

						stmt.close();
						conn.close();
						//theJSLog<<"�쳣�ļ��޷�����"<<endw;
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"�쳣�ļ��޷�����");
						theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);		
						continue ;
					}

				}	
				else if(iStatus == 2)
				{
					//�����ж�
					if(gbExitSig)
					{
						//dr_AbortIDX();
						mdrDeal.dr_abort();
						
						stmt.close();
						conn.close();

						theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "Ӧ�ó����յ��˳��ź�");
						prcExit();
						return;
					}

					while(1)
					{	
						erroinfoMap.clear();
						counter=0;

						for(int i = 0;i<data.size();i++)
						{
							flag = false ;

							stmt<<data[i];
							stmt.execute();

							ERRINFO errinfo;
							while(stmt>>errinfo.source_id>>errinfo.filename>>errinfo.err_msg>>errinfo.err_code>>errinfo.err_col>>errinfo.err_line>>errinfo.err_seq)
							{
								counter++;
								if(counter > maxAuditNum)
								{
									break;
								}

								map< string,vector<ERRINFO> >::iterator  iter= erroinfoMap.find(errinfo.filename);
								if(iter == erroinfoMap.end())
								{
									vector<ERRINFO> errinfoV; 
									errinfoV.push_back(errinfo);
									erroinfoMap.insert(map< string,vector<ERRINFO> >::value_type(errinfo.filename,errinfoV));
									flag = true;
								}
								else
								{
									(iter->second).push_back(errinfo);
								}	
							}

							if(!flag)			//���ļ�û���ҵ�,������
							{
								sleep(10);
								break;			//�˳�for ���½���while
							}		
						}
						if(flag) 
						{
							theJSLog<<"�����Ѿ��鵽ȫ���ļ���Ϣ"<<endi;
							break;
						}
					}
				}

				stmt.close();
			}
			catch (util_1_0::db::SQLException e)
			{
				//dr_AbortIDX();
				mdrDeal.dr_abort();
				stmt.close();
				conn.close();
				erroinfoMap.clear();
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"run() ���ݿ�����쳣%s (%s)",e.what(),sql);
				theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
				continue;
			}	
		}
		else
		{
			if(!(dbConnect(conn)))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"run() �������ݿ�ʧ�� connect error");
				theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
				sleep(30);
				continue  ;
			}

			memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));

			//ɨ�����ǼǱ�,�����ļ�������,���Ҽ�¼������,���Կ���һ�����Ĵ����ļ��ٲ�???
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select source_id,fileName,err_msg,err_code,err_col,err_line,err_seq from d_errfile_info where state = 'W' order by err_msg,fileName,err_line");		
			try
			{
				stmt = conn.createStatement();
				stmt.setSQLString(sql);
				stmt.execute();
		
				ERRINFO errinfo;
				int cnt = 0;
				while(stmt>>errinfo.source_id>>errinfo.filename>>errinfo.err_msg>>errinfo.err_code>>errinfo.err_col>>errinfo.err_line>>errinfo.err_seq)
				{
					cnt++;
					if(cnt > maxAuditNum)	break;

					map< string,vector<ERRINFO> >::iterator  iter= erroinfoMap.find(errinfo.filename);
					if(iter == erroinfoMap.end())
					{	
						if(erroinfoMap.size() >= 10)  break;  //���ҵ��ļ�,��ֹ�ٲ���Ϣ����4096

						vector<ERRINFO> errinfoV; 
						errinfoV.push_back(errinfo);
						erroinfoMap.insert(map< string,vector<ERRINFO> >::value_type(errinfo.filename,errinfoV));
						
						snprintf(mdrDeal.m_SerialString,sizeof(mdrDeal.m_SerialString),"%s%ld|",mdrDeal.m_SerialString,errinfo.err_seq); //ÿ�������һ���ļ�ʱ,��Ϊ��¼��������ܴ��ڶ�����¼				
					}
					else
					{
						(iter->second).push_back(errinfo);
					}
				
				}

				stmt.close();
			}
			catch (util_1_0::db::SQLException e)
			{
				stmt.close();
				conn.close();
				erroinfoMap.clear();
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"run() ���ݿ�����쳣%s (%s)",e.what(),sql);
				theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
				sleep(30);
				continue;
			}	

			if(erroinfoMap.size() == 0) 
			{
				conn.close();
				sleep(30);
				continue ;
			}
			
			theJSLog<<"######## start deal d_err_file_info ########"<<endi;

			ret = mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
			if(ret)
			{
				theJSLog<<"��ϵͳͬ��ʧ��...."<<endw;
				erroinfoMap.clear();
				conn.close();
				sleep(30);
				continue ;
			}
		}
		
		char errFile[512],errFiletmp[512],tmp[256];
		int pas_count = 1;			//�ļ����ݴ���
		int p1, p2;
				
		map<string,string> errFileMap;		//����ڴ���Ϣ
		//vsql.clear();
		memset(mdrDeal.m_AuditMsg,0,sizeof(mdrDeal.m_AuditMsg));
	
		for(map< string,vector<ERRINFO> >::const_iterator iter= erroinfoMap.begin();iter != erroinfoMap.end();++iter)
		{
				theJSLog<<"�����쳣�ļ�: "<<iter->first<<endi;
				
				memset(sql,0,sizeof(sql));
				sprintf(sql,"update d_errfile_info set state = 'H' where filename = '%s'",iter->first);	//����״̬H	
				vsql.push_back(sql);

				vector<ERRINFO>	vv = iter->second;		
				
				snprintf(mdrDeal.m_AuditMsg,sizeof(mdrDeal.m_AuditMsg),"%s%s,%s,%ld:",mdrDeal.m_AuditMsg,vv[0].source_id,vv[0].err_msg,vv[0].err_seq);//�ļ���,����Դ,�ļ���������

				it = m_SourceCfg.find(vv[0].source_id);					//�ҵ�����Դ���ڵ�·��
				if( it == m_SourceCfg.end())
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"û���ҵ��ļ�[%s]������Դ[%s]����Ϣ��",iter->first,vv[0].source_id);
					theJSLog.writeLog(LOG_CODE_SOURCE_NOT_FIND,erro_msg);	// ����������Ϣȱʧ
					
					snprintf(mdrDeal.m_AuditMsg,sizeof(mdrDeal.m_AuditMsg),"%s%s not find;",mdrDeal.m_AuditMsg,vv[0].source_id);
					continue;

				}

				errFileMap.insert(map<string,string>::value_type(iter->first,it->second.szSourcePath)); //��������Դ�ļ���

				memset(errFile,0,sizeof(errFile));
				memset(errFiletmp,0,sizeof(errFiletmp));

				strcpy(errFile,mConfParam.szOutPath);								//��д��ʱ�ļ��ڸ���
				strcat(errFile,iter->first.c_str());
				strcat(errFile,".err");
				strcpy(errFiletmp,errFile);
				strcat(errFiletmp,".tmp");

				ofstream out(errFiletmp);
				if(!out)
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"д�쳣�ļ�[%s]��Ϣʧ��:%s ",errFiletmp,strerror(errno));
					theJSLog.writeLog(LOG_CODE_FILE_WRITE_ERR,erro_msg);
					//vsql.clear();
					continue;
				}
				
				getCurTime(currTime);					
				
				if(iter->first.find(".ROL") == string::npos)			//�ж��ļ����ݴ���
				{
						pas_count = 1;
				}
				else
				{
					theJSLog<<"�ǻع��ļ�"<<endi;
					p1  = iter->first.find('.');
					if(p1)	iter->first.find('.',p1+1);
					if(p2)	
					{	
						memset(tmp,0,sizeof(tmp));
						p1++;
						strcpy(tmp,(iter->first.substr(p1,(p2-p1))).c_str());
						pas_count = atoi(tmp);
					}
				}
				
				theJSLog<<"д�쳣�ļ���Ϣ: "<<iter->first<<".err"<<endi;

				for(int i = 0;i<vv.size();i++)
				{
					sprintf(mdrDeal.m_AuditMsg,"%s%d,%d;",mdrDeal.m_AuditMsg,vv[i].err_line,vv[i].err_col); //�������,������,������

					if(strcmp(vv[i].err_msg,"F") == 0)            //��ʾ�ļ�������
					{
						out<<iter->first<<";"<<currTime<<";"<<pas_count<<";"<<vv[i].err_code<<";"<<""<<";"<<""<<endl;
					}
					else
					{
						out<<iter->first<<";"<<currTime<<";"<<pas_count<<";"<<vv[i].err_code<<";"<<vv[i].err_col<<";"<<vv[i].err_line<<endl;
					}

				}
				
				out.close();
				
				snprintf(mdrDeal.m_AuditMsg,sizeof(mdrDeal.m_AuditMsg),"%s|",mdrDeal.m_AuditMsg);

				memset(sql,0,sizeof(sql));
				sprintf(sql,"update d_errfile_info set state = 'Y' where filename = '%s'",iter->first);	//����״̬			
				vsql.push_back(sql);		

				//�Ǽ����ɵĴ������Ϣ��2013-07-22�����ԭʼ���ļ����õǼ�
				memset(sql,0,sizeof(sql));
				sprintf(sql,"insert into d_out_file_reg(filename,source_id,reg_time,file_type,state) values('%s','%s','%s','%c','%c')",iter->first,vv[0].source_id,currTime,'N','W');
				vsql.push_back(sql);
				
				memset(sql,0,sizeof(sql));
				sprintf(sql,"insert into d_out_file_reg(filename,source_id,reg_time,file_type,state) values('%s.err','%s','%s','%c','%c')",iter->first,vv[0].source_id,currTime,'E','W');
				vsql.push_back(sql);		
		}	

		erroinfoMap.clear();						//��մ�����Ϣ
		
		ret = mdrDeal.IsAuditSuccess(mdrDeal.m_AuditMsg);
		if(ret)				//�ٲ�ʧ��,�ع����ݿ�,ɾ����ʱ�ļ�
		{
			vsql.clear();
			
			try
			{
				//���鵽���ļ�״̬��E
				stmt = conn.createStatement();
				memset(sql,0,sizeof(sql));
				sprintf(sql,"update d_errfile_info set state = 'E' where filename = :1 ");	//����״̬E
				stmt.setSQLString(sql);
				
				//ɾ����ʱ�ļ�
				theJSLog<<"ɾ����ʱ�����쳣��Ϣ�ļ�"<<endi;
				//sleep(10);
				for(map< string,string >::const_iterator iter= errFileMap.begin();iter != errFileMap.end();++iter)
				{
					memset(errFiletmp,0,sizeof(errFiletmp));
					memset(errFile,0,sizeof(errFile));	
					strcpy(errFile,mConfParam.szOutPath);
					strcat(errFile,iter->first.c_str());
					strcat(errFile,".err");
					strcpy(errFiletmp,errFile);
					strcat(errFiletmp,".tmp");
					if(remove(errFiletmp))
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"��ʱ�ļ�[%s]ɾ��ʧ��: %s",errFiletmp,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
					}
					
					stmt<<iter->first;
					stmt.execute();
				}
				
				stmt.close();
			}
			catch (util_1_0::db::SQLException e)
			{
				stmt.rollback();
				stmt.close();
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"run() ���ݿ�����쳣%s (%s)",e.what(),sql);
				theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
			}	

			conn.close();
			errFileMap.clear();

			theJSLog<<"######## end deal  ########\n"<<endi;

			continue ;
		}

		theJSLog<<"�ύsql��䵽���ݿ�..."<<endi;	
		ret = updateDB();

		if(ret == -1)								//ִ��sqlʧ��,����ʱ�ļ�ɾ��
		{
			for(map< string,string >::const_iterator iter= errFileMap.begin();iter != errFileMap.end();++iter)
			{
				memset(errFiletmp,0,sizeof(errFiletmp));
				memset(errFile,0,sizeof(errFile));	
				strcpy(errFile,mConfParam.szOutPath);
				strcat(errFile,iter->first.c_str());
				strcat(errFile,".err");
				strcpy(errFiletmp,errFile);
				strcat(errFiletmp,".tmp");
				if(remove(errFiletmp))
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"��ʱ�ļ�[%s]ɾ��ʧ��: %s",errFiletmp,strerror(errno));
					theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
				}	
			}
			
			errFileMap.clear();
			conn.close();		
			theJSLog<<"######## end deal ########\n"<<endi;

			continue ;
		}

		conn.close();

		theJSLog<<"����ʱ�쳣˵���ļ���Ϊ��ʽ�ļ�,�����ļ����쳣����Ŀ¼�Ƶ��ϴ�Ŀ¼..."<<endi;
		for(map< string,string >::const_iterator iter= errFileMap.begin();iter != errFileMap.end();++iter)
		{
			memset(errFiletmp,0,sizeof(errFiletmp));
			memset(errFile,0,sizeof(errFile));	
			strcpy(errFile,mConfParam.szOutPath);
			strcat(errFile,iter->first.c_str());
			strcat(errFile,".err");
			strcpy(errFiletmp,errFile);
			strcat(errFiletmp,".tmp");
			if(rename(errFiletmp,errFile))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"��ʱ�ļ�[%s]������ʽʧ��: %s",errFiletmp,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_RENAME_ERR,erro_msg);
			}
			
			memset(tmp,0,sizeof(tmp));
			memset(outFileName,0,sizeof(outFileName));
			strcpy(tmp,iter->second.c_str());
			strcat(tmp,mConfParam.szInPath);
			strcat(tmp,iter->first.c_str());
			strcpy(outFileName,mConfParam.szOutPath);
			strcat(outFileName,iter->first.c_str());
			//cout<<"Դ�ļ�:"<<tmp<<"  ����ļ�:"<<outFileName<<endl;

			if(rename(tmp,outFileName))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"�ƶ��쳣Դ�ļ�[%s]ʧ��: %s",iter->first,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
			}

		}
		
		errFileMap.clear();

	}
/*
	catch (util_1_0::db::SQLException e)
	{
		stmt.rollback();
		stmt.close();
		conn.close();
		
		vsql.clear();
		erroinfoMap.clear();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() ���ݿ�����쳣%s (%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
	}
*/
	catch (jsexcp::CException &e) 
	{	
		vsql.clear();
		erroinfoMap.clear();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		
	}
	
	theJSLog<<"######## end deal ########\n "<<endi;
	
	sleep(30);
  }//while(1)

}

//2013-10-11 �����˳�����
void ExceptionOut::prcExit()
{
	mdrDeal.dr_ReleaseDR();

	PS_Process::prcExit();
}

/*
//���ֳ�ʼ��
bool ExceptionOut::drInit()
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
int ExceptionOut::drVarGetSet(char* serialString)
{
		int ret = 0;
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
 int ExceptionOut::IsAuditSuccess(const char* dealresult)
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
			//theJSLog << "�����ٲ�ʧ��,���:" << ret <<"���ˣ�"<<dealresult<< endw;
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"�����ٲ�ʧ��,���:%d,����:%s",ret,dealresult);
			theJSLog.writeLog(LOG_CODE_DR_AUDIT_ERR,erro_msg);

			dr_AbortIDX();
		}
		else if (3 == ret)
		{
			theJSLog<<"�����ٲó�ʱ..."<<endw;
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

bool ExceptionOut::CheckTriggerFile()
{
	int ret = 0;
	if(access(m_triggerFile.c_str(),F_OK) != 0)	return false;

	theJSLog<< "��鵽trigger�ļ�,��ɾ��"<< m_triggerFile <<endi;

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
	cout<<"********************************************* "<<endl;
	cout<<"*    China Telecom. Telephone Network         "<<endl;
	cout<<"*    InterNational Account Settle System      "<<endl;
	cout<<"*                                             "<<endl;
	cout<<"*           jserrout                          "<<endl;
	cout<<"*           sys.GJZW.Version 1.0	             "<<endl;
	cout<<"*    last update time :  2013-12-16 by  hed	 "<<endl;
	cout<<"********************************************* "<<endl;

	ExceptionOut fm ;

	if( !fm.init( argc, argv ) )
	{
		 return -1;
	}
        
	//while(1)
	//{
		//theJSLog.reSetLog();
		fm.run();
		//sleep(30);
	//}

   return 0;
}


