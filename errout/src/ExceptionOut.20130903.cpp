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
SGW_RTInfo	rtinfo;

ExceptionOut::ExceptionOut()
{  
	petri_status = 0;
	
	memset(m_szFileName,0,sizeof(m_szFileName));
	memset(input_path,0,sizeof(input_path));
	memset(out_path,0,sizeof(out_path));
	memset(m_szSourceID,0,sizeof(m_szSourceID));
	memset(outFileName,0,sizeof(outFileName));
	memset(currTime,0,sizeof(currTime));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(sql,0,sizeof(sql));
}


ExceptionOut::~ExceptionOut()
{
	
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
	
	if(!(rtinfo.connect()))
	{
		return false;
	}
	
	rtinfo.getDBSysMode(petri_status);
	cout<<"petri status:"<<petri_status<<endl;


	//*********2013-07-15 ��ȡ���ݿ��������Ϣ��������Դ�飬����Ŀ¼ 2013-03-11 add by hed*********************************************/
	
	if(!(dbConnect(conn)))
	{
		cout<<"�������ݿ� connect error."<<endl;	//д��־
		return false ;
	}

	//cout<<"��ˮ��ID:"<<getFlowID()<<"   ģ��ID:"<<getModuleID()<<endl;
	try{

		//�������·�����ں�����Դ��û�й�ϵ������������C_GOLABAL_ENV���棬���������·��������Ǿ���·��
		string sql ;
		Statement stmt = conn.createStatement();

		sql = "select varvalue from c_global_env where varname = 'EXCEPTION_INPUT'";
		stmt.setSQLString(sql);
		stmt.execute();
		if(!(stmt>>input_path))
		{
			cout<<"�쳣�ļ����ģ���������·��û������,����c_global_env�����ñ��� EXCEPTION_INPUT"<<endl;		
			return false ;
		}		
		completeDir(input_path);
		
		sql = "select varvalue from c_global_env where varname = 'EXCEPTION_OUTPUT'";
		stmt.setSQLString(sql);
		stmt.execute();
		if(!(stmt>>out_path))
		{
			cout<<"�쳣�ļ����ģ��������·��û������,����c_global_env�����ñ��� EXCEPTION_OUTPUT"<<endl;		
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


//�����ύsql,��֤һ������������
int ExceptionOut::updateDB()
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


//ɨ������ļ���Ϣ�ǼǱ������쳣˵���ļ�(Դ�ļ���+.err),�������ļ����أ��ļ�����������һ��������Ϣ����¼���������ɶ���
void ExceptionOut::run()
{
	
	if(gbExitSig)
	{
		if(gbExitSig) theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "Ӧ�ó����յ��˳��ź�");
		PS_Process::prcExit();
		return;
	}
	
	int ret = 0;

	try
	{	
		//�ж����ݿ�״̬
		rtinfo.getDBSysMode(petri_status);
		if(petri_status == DB_STATUS_OFFLINE)	return ;
		
		//cout<<"...."<<DB_STATUS_OFFLINE<<":"<<petri_status<<endl;

		if(!(dbConnect(conn)))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"run() �������ݿ�ʧ�� connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
			return  ;
		}


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

			}
			else
			{
					(iter->second).push_back(errinfo);
			}
			
		}

		stmt.close();

		if(erroinfoMap.size() == 0) return ;

		char errFile[1024],errFiletmp[1024],tmp[256];
		int pas_count = 1;			//�ļ����ݴ���
		int p1, p2;
		
		//map<string,string> errFileMap;	
	
		for(map< string,vector<ERRINFO> >::const_iterator iter= erroinfoMap.begin();iter != erroinfoMap.end();++iter)
		{
				theJSLog<<"�����쳣�ļ�: "<<iter->first<<endi;
				
				memset(sql,0,sizeof(sql));
				sprintf(sql,"update d_errfile_info set state = 'H' where filename = '%s'",iter->first);	//����״̬H	
				vsql.push_back(sql);
				
				vector<ERRINFO>	vv = iter->second;		

				it = m_SourceCfg.find(vv[0].source_id);					//�ҵ�����Դ���ڵ�·��
				if( it == m_SourceCfg.end())
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"û���ҵ��ļ�[%s]������Դ[%s]����Ϣ��",iter->first,vv[0].source_id);
					theJSLog.writeLog(LOG_CODE_PARAM_INFO_LACK,erro_msg);	// ����������Ϣȱʧ

					continue;

				}

				//errFileMap.insert(map<string,string>::value_type(iter->first,it->second.szSourcePath));

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
					vsql.clear();
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
				
				memset(sql,0,sizeof(sql));
				sprintf(sql,"update d_errfile_info set state = 'Y' where filename = '%s'",iter->first);	//����״̬			
				vsql.push_back(sql);
				

				//�Ǽ����ɵĴ������Ϣ��2013-07-22�����ԭʼ���ļ����õǼ�
				memset(sql,0,sizeof(sql));
				sprintf(sql,"insert into d_out_file_reg(filename,source_id,reg_time,file_type,state) values('%s','%s','%s','%c','%c')",iter->first,vv[0].source_id,currTime,'E','W');
				vsql.push_back(sql);
				
				memset(sql,0,sizeof(sql));
				sprintf(sql,"insert into d_out_file_reg(filename,source_id,reg_time,file_type,state) values('%s.err','%s','%s','%c','%c')",iter->first,vv[0].source_id,currTime,'N','W');
				vsql.push_back(sql);

				ret = updateDB();

				if(ret == -1)		//ִ��sqlʧ��,����ʱ�ļ�ɾ��
				{
					if(remove(errFiletmp))
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"��ʱ�ļ�[%s]ɾ��ʧ��: %s",errFiletmp,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
					}
				}
				else			   //��ʱ�ļ�����ʽ�ļ�,��������ԭʼ�ļ��Ƶ����Ŀ¼
				{
					if(rename(errFiletmp,errFile))				//�ļ�����
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"��ʱ�ļ�[%s]������ʽʧ��: %s",errFiletmp,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_RENAME_ERR,erro_msg);
					}

					memset(tmp,0,sizeof(tmp));
					memset(outFileName,0,sizeof(outFileName));
					strcpy(tmp,it->second.szSourcePath);
					strcat(tmp,input_path);
					strcpy(outFileName,out_path);
					strcat(outFileName,iter->first.c_str());

					theJSLog<<"�ƶ��쳣ԭʼ�ļ�"<<"��Ŀ¼["<<input_path<<"]�����Ŀ¼"<<endi;
					strcat(tmp,iter->first.c_str());			
					if(rename(tmp,outFileName))
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"�ƶ��ļ�[%s]ʧ��: %s",iter->first,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
					}
				}
						
		}
		
		conn.close();

		erroinfoMap.clear();		//��մ�����Ϣ

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

int main(int argc,char** argv)
{
	cout<<"********************************************** "<<endl;
	cout<<"*    GuangDong Telecom. Telephone Network    * "<<endl;
	cout<<"*       Centralized Settlement System        * "<<endl;
	cout<<"*                                            * "<<endl;
	cout<<"*            errout                          * "<<endl;
	cout<<"*              Version 1.0	                * "<<endl;
	cout<<"*    last update time :  2013-08-30 by  hed	* "<<endl;
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


