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
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>  //stat()��������ѯ�ļ���Ϣ
#include<unistd.h>     //��ȡ��ǰ��������Ŀ¼
#include<iostream>
#include<fstream>
#include "ExceptionOut.h"

#include "CF_Common.h"
#include "CF_CLogger.h"
CLog theJSLog;
//SGW_RTInfo	rtinfo;

ExceptionOut::ExceptionOut()
{  
	//petri_status = 0;
	
	m_enable  = false;

	memset(m_szFileName,0,sizeof(m_szFileName));
	memset(input_path,0,sizeof(input_path));
	memset(out_path,0,sizeof(out_path));
	memset(m_szSourceID,0,sizeof(m_szSourceID));
	memset(outFileName,0,sizeof(outFileName));
	memset(currTime,0,sizeof(currTime));
	memset(erro_msg,0,sizeof(erro_msg));

}


ExceptionOut::~ExceptionOut()
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
bool ExceptionOut::init(int argc,char** argv)
{  
    if(!PS_Process::init(argc,argv))
    {
      return false;
    }
	
	//PS_Process::setSignalPrc(); 

	//��ȡpetri��״̬,��ϵͳΪֻ��̬ʱ,���ݿ���²������д�ļ�
	
	//if(!(rtinfo.connect()))
	//{
	//	return false;
	//}
	//short status;
	//rtinfo.getSysMode(petri_status);
	//cout<<"petri status:"<<status<<endl;


	//*********2013-07-15 ��ȡ���ݿ��������Ϣ��������Դ�飬����Ŀ¼ 2013-03-11 add by hed*********************************************/
	
	if(!(dbConnect(conn)))
	{
		cout<<"�������ݿ� connect error."<<endl;	//д��־
		return false ;
	}

	cout<<"��ˮ��ID:"<<getFlowID()<<"   ģ��ID:"<<getModuleID()<<endl;
	try{

		//�������·�����ں�����Դ��û�й�ϵ������������C_GOLABAL_ENV���棬���������·��������Ǿ���·��
		string sql ;
		Statement stmt = conn.createStatement();

		sql = "select varvalue from c_global_env where varname = 'EXCEPTION_INPUT'";
		stmt.setSQLString(sql);
		stmt.execute();
		if(!(stmt>>input_path))
		{
			cout<<"�쳣�ļ����ģ���������·��û������,����c_global_env�����ñ���EXCEPTION_INPUT"<<endl;		
			return false ;
		}		
		completeDir(input_path);
		
		sql = "select varvalue from c_global_env where varname = 'EXCEPTION_OUTPUT'";
		stmt.setSQLString(sql);
		stmt.execute();
		if(!(stmt>>out_path))
		{
			cout<<"�쳣�ļ����ģ��������·��û������,����c_global_env�����ñ���EXCEPTION_OUTPUT"<<endl;		
			return false ;
		}		
		completeDir(out_path);
					
		stmt.close();
	   
	   }catch(SQLException e)
		{
			cout<<"init ���ݿ��ѯ�쳣:"<<e.what()<<endl;
			return false ;
		}


	//�ж���־Ŀ¼��sqlĿ¼�Ƿ����
	 DIR *dirptr = NULL; 
	 
	 if((dirptr=opendir(out_path)) == NULL)
	 {
		cout<<"���Ŀ¼"<<out_path<<"��ʧ��"<<endl;
		return false ;

	 }else closedir(dirptr);


	//��ʼ���ڴ���־�ӿ�
	bool bb = initializeLog(argc,argv,false);  //�Ƿ����ģʽ
	if(!bb)
	{
			//cout<<"��ʼ���ڴ���־�ӿ�ʧ��"<<endl;
			return false;
	}
	
	theJSLog.setLog(szLogPath, szLogLevel,"HED" , "ERROUT", 001);	//�ļ���־�ӿڣ��������ڴ���־�ӿ�
	
	theJSLog<<"�������·����"<<input_path<<"  ���·��:"<<out_path<<"	��־·��:"
			<<szLogPath<<" ��־����:"<<szLogLevel<<" sql���·��:"<<sql_path<<endi;

	if(LoadSourceCfg() == -1) 
	{		
		return false ;  
	}
	
	conn.close();

	char input_dir[1024];

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
	  
	}
	
   if(!drInit)		return false;

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
		sprintf(erro_msg,"LoadSourceCfg���ݿ����%s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

		//throw jsexcp::CException(0, "LoadSourceCfg�������ݿ����", __FILE__, __LINE__);
		return -1;
	 }

	return 0;
}



//ɨ������ļ���Ϣ�ǼǱ������쳣˵���ļ�(Դ�ļ���+.err),�������ļ����أ��ļ�����������һ��������Ϣ����¼���������ɶ���
void ExceptionOut::run()
{
	//cout<<"�������У�����������"<<endl;
	
	if(gbExitSig)
	{
		if(gbExitSig) theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "Ӧ�ó����յ��˳��ź�");
		PS_Process::prcExit();
		return;
	}
	
	int ret = 0;
	char sql[1024];

	try
	{
		if(!(dbConnect(conn)))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"run() �������ݿ�ʧ�� connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
			return  ;
		}


		if(drStatus == 1)		//����ϵͳ
		{
			isWriteSQLFileByTime();		
			//���trigger�����ļ��Ƿ����
			if(!CheckTriggerFile())
			{
				sleep(1);
				return ;
			}

			ret = drVarGetSet(m_SerialString);
			if(ret)
			{
				theJSLog<<"��ϵͳͬ��ʧ��...."<<endi;
				return ;
			}
			
			//��ȡͬ������
			vector<string> data;		
			splitString(m_SerialString,";",data,true,true);  //���͵��ַ���: �ļ���|�ļ���|sqlFile�ļ���

			isWriteSQLFileByMain(data[data.size()-1].c_str());	//��ϵͳ��sqlFile��ͨ����ϵͳ��������
			
			bool flag  = false;

			memset(m_AuditMsg,0,sizeof(m_AuditMsg));

			memset(sql,0,sizeof(sql));
			sprintf(sql,"select source_id,fileName,err_msg,err_code,err_col,err_line from d_errfile_info where state = 'W' and fileName = :1");		
			Statement stmt = conn.createStatement();
			stmt.setSQLString(sql);
			for(int i = 0;i<data.size()-1;i++)
			{
				flag = false ;

				stmt<<data[i];
				stmt.execute();

				ERRINFO errinfo;
				while(stmt>>errinfo.source_id>>errinfo.filename>>errinfo.err_msg>>errinfo.err_code>>errinfo.err_col>>errinfo.err_line)
				{
					map< string,vector<ERRINFO> >::iterator  iter= erroinfoMap.find(errinfo.filename);
					if(iter == erroinfoMap.end())
					{
						vector<ERRINFO> errinfoV; 
						errinfoV.push_back(errinfo);
						erroinfoMap.insert(map< string,vector<ERRINFO> >::value_type(errinfo.filename,errinfoV));
					}
					else
					{
						(iter->second).push_back(errinfo);
					}
					
				  flag = true;
				}
				
				if(!flag)  
				{
					theJSLog<<"�ļ���:"<<data[i]<<"�޷��ҵ�"<<endi;
					sprintf(m_AuditMsg,"%s not find|",m_AuditMsg);
				}
 			}

		}
		else
		{
			isWriteSQLFile();				//�Ƿ��ύsql�ļ�
			
			memset(m_SerialString,0,sizeof(m_SerialString));

			memset(sql,0,sizeof(sql));
			sprintf(sql,"select source_id,fileName,err_msg,err_code,err_col,err_line from d_errfile_info where state = 'W' and rownum < 51");		
			Statement stmt = conn.createStatement();
			stmt.setSQLString(sql);
			stmt.execute();
	
			ERRINFO errinfo;	
			while(stmt>>errinfo.source_id>>errinfo.filename>>errinfo.err_msg>>errinfo.err_code>>errinfo.err_col>>errinfo.err_line)
			{
				map< string,vector<ERRINFO> >::iterator  iter= erroinfoMap.find(errinfo.filename);
				if(iter == erroinfoMap.end())
				{
					vector<ERRINFO> errinfoV; 
					errinfoV.push_back(errinfo);
					erroinfoMap.insert(map< string,vector<ERRINFO> >::value_type(errinfo.filename,errinfoV));

					sprintf(m_SerialString,"%s%s|",m_SerialString,errinfo.filename);
				}
				else
				{
					(iter->second).push_back(errinfo);
				}
			
			}

			stmt.close();

			if(erroinfoMap.size() == 0) return ;
			
			sprintf(m_SerialString,"%s%s|",m_SerialString,sqlFile);

			ret = drVarGetSet(m_SerialString);
			if(ret)
			{
				theJSLog<<"��ϵͳͬ��ʧ��...."<<endi;
				return ;
			}
			
			memset(m_AuditMsg,0,sizeof(m_AuditMsg));
		}
		
		conn.close();
		
		
		char errFile[1024],errFiletmp[1024],tmp[256];
		int pas_count = 1;			//�ļ����ݴ���
		int p1, p2;
		
		map<string,string> errFileMap;
		
		getCurTime(currTime);	

		//�����ٲ���Ϣ �ļ���,��¼����,���󼶱�
		for(map< string,vector<ERRINFO> >::const_iterator iter= erroinfoMap.begin();iter != erroinfoMap.end();++iter)
		{
				theJSLog<<"�����쳣�ļ�: "<<iter->first<<endi;
				
				memset(sql,0,sizeof(sql));
				sprintf(sql,"update d_errfile_info set state = 'H' where filename = '%s'",iter->first);	//����״̬H	
				writeSQL(sql);
				
				vector<ERRINFO>	vv = iter->second;	
				
				sprintf(m_AuditMsg,"%s%s,%d,%s|",m_AuditMsg,iter->first,vv.size(),vv[0].err_msg);

				it = m_SourceCfg.find(vv[0].source_id);					//�ҵ�����Դ���ڵ�·��
				if( it == m_SourceCfg.end())
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"û���ҵ��ļ�[%s]������Դ[%s]����Ϣ��",iter->first,vv[0].source_id);
					theJSLog.writeLog(LOG_CODE_PARAM_INFO_LACK,erro_msg);	// ����������Ϣȱʧ

					sprintf(m_AuditMsg,"%s%s not find;",m_AuditMsg,vv[0].source_id);
					continue;

				}

				errFileMap.insert(map<string,string>::value_type(iter->first,it->second.szSourcePath));

				memset(errFile,0,sizeof(errFile));
				memset(errFiletmp,0,sizeof(errFiletmp));

				strcpy(errFile,out_path);								//��д��ʱ�ļ��ڸ���
				strcat(errFile,iter->first.c_str());
				strcat(errFile,".err");
				strcpy(errFiletmp,errFile);
				strcat(errFiletmp,".tmp");

				ofstream out(errFiletmp);
				if(!out)
				{
					theJSLog.writeLog(LOG_CODE_FILE_WRITE_ERR,"д�쳣�ļ���Ϣʧ��");
					sprintf(m_AuditMsg,"%s%s not write[%s]|",m_AuditMsg,errFiletmp,strerror(errno));
					continue;
				}
				
				//getCurTime(currTime);					
				
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
				
				//rename(errFiletmp,errFile);				//�ļ�����
				
				memset(sql,0,sizeof(sql));
				sprintf(sql,"update d_errfile_info set state = 'Y' where filename = '%s'",iter->first);	//����״̬	
				
				writeSQL(sql);
				

				//�Ǽ����ɵĴ������Ϣ��2013-07-22�����ԭʼ���ļ����õǼ�
				memset(sql,0,sizeof(sql));
				sprintf(sql,"insert into d_out_file_reg(filename,source_id,reg_time,file_type,state) values('%s','%s','%s','%c','%c')",iter->first,vv[0].source_id,currTime,'E','W');
				writeSQL(sql);
				
				memset(sql,0,sizeof(sql));
				sprintf(sql,"insert into d_out_file_reg(filename,source_id,reg_time,file_type,state) values('%s.err','%s','%s','%c','%c')",iter->first,vv[0].source_id,currTime,'N','W');
				writeSQL(sql);
			
			/*
				memset(tmp,0,sizeof(tmp));
				memset(outFileName,0,sizeof(outFileName));
				strcpy(tmp,it->second.szSourcePath);
				strcat(tmp,input_path);
				strcpy(outFileName,out_path);
				strcat(outFileName,iter->first.c_str());

				theJSLog<<"�Ƶ��쳣�ļ�"<<"��Ŀ¼["<<tmp<<"]�����Ŀ¼"<<endi;
				strcat(tmp,iter->first.c_str());			
				if(rename(tmp,outFileName))
				{
					//perror("�ƶ��ļ�ʧ��:");
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"�ƶ��ļ�ʧ��: %s",strerror(errno));
					theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
				}
			*/				
				
		}
		
		erroinfoMap.clear();		//��մ�����Ϣ
		
		//�ٲ�.....
		if(!IsAuditSuccess(m_AuditMsg))				//�ٲ�ʧ��,�ع����ݿ�,ɾ����ʱ�ļ�
		{
			rollBackSQL();

			//ɾ����ʱ�ļ�
			for(map< string,string >::const_iterator iter= errFileMap.begin();iter != errFileMap.end();++iter)
			{
				memset(errFiletmp,0,sizeof(errFiletmp));
				memset(errFile,0,sizeof(errFile));	
				strcpy(errFile,out_path);
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

			return ;
		}

		theJSLog<<"�ύsql��䵽�ļ�..."<<endi;
		commitSQLFile();

		//��ʱ�ļ�д��ʽ�ļ� ����ԭʼ�ļ��Ƶ����Ŀ¼
		theJSLog<<"����ʱ�ļ���Ϊ��ʽ�ļ�,�����ļ�����Ŀ¼�Ƶ��ϴ�Ŀ¼..."<<endi;
		for(map< string,string >::const_iterator iter= errFileMap.begin();iter != errFileMap.end();++iter)
		{
			memset(errFiletmp,0,sizeof(errFiletmp));
			memset(errFile,0,sizeof(errFile));	
			strcpy(errFile,out_path);
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
			strcat(tmp,input_path);
			strcat(tmp,iter->first.c_str());
			strcpy(outFileName,out_path);
			strcat(outFileName,iter->first.c_str());

			//cout<<"Դ�ļ�:"<<tmp<<"  ����ļ�:"<<outFileName<<endl;

			if(rename(tmp,outFileName))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"�ƶ��쳣Դ�ļ�[%s]ʧ��: %s",iter->first,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
			}

		}


	}
	catch (util_1_0::db::SQLException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() ���ݿ�����쳣%s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
	}	
	catch (jsexcp::CException &e) 
	{	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		
	}

}

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
int ExceptionOut::drVarGetSet(char* serialString)
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
 bool ExceptionOut::IsAuditSuccess(const char* dealresult)
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

bool ExceptionOut::CheckTriggerFile()
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
	cout<<"*            errout                          * "<<endl;
	cout<<"*              Version 1.0	                * "<<endl;
	cout<<"*    last update time :  2013-08-29 by  hed	* "<<endl;
	cout<<"********************************************** "<<endl;


	ExceptionOut fm ;


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


