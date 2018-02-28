/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-07-22
File:			 FileInAduit.cpp
Description:	 �ļ��˶����ģ��
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
#include "FileInAduit.h"

#include "CF_Common.h"
#include "CF_CLogger.h"
CLog theJSLog;


FileInAduit::FileInAduit()
{  
	memset(m_szFileName,0,sizeof(m_szFileName));
	
	memset(receive_path,0,sizeof(receive_path));	
	memset(input_path,0,sizeof(input_path));		
	memset(output_path,0,sizeof(output_path));
	memset(bak_path1,0,sizeof(bak_path1));
	memset(bak_path2,0,sizeof(bak_path2));
	memset(month_input_path,0,sizeof(month_input_path));

	memset(currTime,0,sizeof(currTime));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(sql,0,sizeof(sql));

	m_enable = false;
}

FileInAduit::~FileInAduit()
{

}

//ģ���ʼ������
bool FileInAduit::init(int argc,char** argv)
{ 
    if(!PS_Process::init(argc,argv))
    {
      return false;
    }
	
	//PS_Process::setSignalPrc(); 

	//cout<<"��ˮ��ID:"<<getFlowID()<<"   ģ��ID:"<<getModuleID()<<endl;

	 // �Ӻ��Ĳ��������ȡ��־��·��������
	 char sParamName[256],bak_path[1024];
	 CString sKeyVal;
	 
	 sprintf(sParamName, "file.check.receive_path");			//Դ�ļ�����Ŀ¼
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(receive_path,0,sizeof(receive_path));
		strcpy(receive_path,(const char*)sKeyVal);
		completeDir(receive_path);

	 }
	 else
	 {	
		cout<<"���ں��Ĳ���������Դ�ļ�����·��[file.check.receive_path]"<<endl;
		return false ;
	 }	 
	 
	 sprintf(sParamName, "file.check.output_path");			  //Դ�ļ����Ŀ¼
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(output_path,0,sizeof(output_path));
		strcpy(output_path,(const char*)sKeyVal);
		completeDir(output_path);
	 }
	 else
	 {	
		cout<<"���ں��Ĳ���������Դ�ļ������·��[file.check.output_path]"<<endl;
		return false ;
	 }	


	 sprintf(sParamName, "file.check.input_path");			//�˶��ļ�����Ŀ¼
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(input_path,0,sizeof(input_path));
		strcpy(input_path,(const char*)sKeyVal);

	 }
	 else
	 {	
		cout<<"���ں��Ĳ��������ú˶��ļ�������·��[file.check.input_path]"<<endl;
		return false ;
	 }	 

	 sprintf(sParamName, "file.check.month_input_path");			//�»����ļ�����Ŀ¼
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(month_input_path,0,sizeof(month_input_path));
		strcpy(month_input_path,(const char*)sKeyVal);
	 }
	 else
	 {	
		cout<<"���ں��Ĳ����������»����ļ�������·��[file.check.month_input_path]"<<endl;
		return false ;
	 }	 
	
	 sprintf(sParamName, "file.check.bak_path");			
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(bak_path,0,sizeof(bak_path));
		strcpy(bak_path,(const char*)sKeyVal);

	 }
	 else
	 {	
		cout<<"���ں��Ĳ��������ú˶�ģ��ı���·��[file.check.bak_path]"<<endl;
		return false ;
	 }	 	

	//�ж�Ŀ¼�Ƿ����
	 DIR *dirptr = NULL; 
	
	 if((dirptr=opendir(input_path)) == NULL)
	 {
	 	cout<<"�˶��ļ�����Ŀ¼["<<input_path<<"]��ʧ��"<<endl;
	 	return false ;

	 }else closedir(dirptr);
	 completeDir(input_path);
	

	 if((dirptr=opendir(month_input_path)) == NULL)
	 {
	 	cout<<"�»����ļ�����Ŀ¼["<<month_input_path<<"]��ʧ��"<<endl;
	 	return false ;

	 }else closedir(dirptr);
	 completeDir(month_input_path);

	 if((dirptr=opendir(bak_path)) == NULL)
	 {
	 	cout<<"����Ŀ¼["<<bak_path<<"]��ʧ��"<<endl;
	 	return false ;

	 }else closedir(dirptr);
	  completeDir(bak_path);
	 
	 //����Ŀ¼����Ӧ�½�����Ŀ¼�����ֺ˶��ļ����»����ļ�
	 strcpy(bak_path1,bak_path);
	 strcat(bak_path1,"AUD");
	 if(chkDir(bak_path1))
	 {
			cout<<"����Ŀ¼["<<bak_path1<<"]��ʧ��"<<endl;
			return false;
	 }
	 completeDir(bak_path1);

	
	 strcpy(bak_path2,bak_path);
	 strcat(bak_path2,"SUM");
	 if(chkDir(bak_path2))
	 {
			cout<<"����Ŀ¼["<<bak_path2<<"]��ʧ��"<<endl;
			return false ;
	 }
	 completeDir(bak_path2);

	 if((dirptr=opendir(sql_path)) == NULL)
	 {		
		cout<<"SQLĿ¼:"<<sql_path<<"��ʧ��"<<endl;
		return false ;
	 }else  closedir(dirptr);


	//��ʼ���ڴ���־�ӿ�
	bool bb = initializeLog(argc,argv,false);  //�Ƿ����ģʽ
	if(!bb)
	{
			//cout<<"��ʼ���ڴ���־�ӿ�ʧ��"<<endl;
			return false;
	}
	
	//getCurTime(currTime);
	//char tmp[15];
	//memset(tmp,0,sizeof(tmp));
	//strncpy(tmp,currTime,6);
	theJSLog.setLog(szLogPath, szLogLevel,"CHECK_MONTH","CHECK", 001);	//�ļ���־�ӿڣ��������ڴ���־�ӿ�
	
	theJSLog<<"	��־·��:"<<szLogPath<<" ��־����:"<<"�˶��ļ�����Ŀ¼:"<<input_path<<"�»����ļ�����Ŀ¼:"<<month_input_path
		    <<" Դ�ļ�����Ŀ¼:"<<receive_path<<" Դ�ļ����Ŀ¼:"<<output_path <<"  ����Ŀ¼:"<<bak_path<<" sql�ļ�Ŀ¼:"<<sql_path<<endi;
   	
		
	//��ѯ�º˶��ļ���������Ϣ
	try
	{
		if(!(dbConnect(conn)))
		{
			sprintf(erro_msg,"init() �������ݿ�ʧ�� connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
			return  false;
		}
		
		if(LoadSourceCfg() == -1)	    //��������Դ������Ϣ 2013-08-07
		{
			return false;
		}

		//�ж����������·���Ƿ����
		char input_dir[1024],output_dir[1024];
		for(map<string,SOURCECFG>::const_iterator iter = m_SourceCfg.begin();iter !=m_SourceCfg.end(); ++iter)
		{		   		
			memset(input_dir,0,sizeof(input_dir));
			strcpy(input_dir,iter->second.szSourcePath);
			strcat(input_dir,receive_path);
			if((dirptr=opendir(input_dir)) == NULL)
			{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"����Դ[%s]�Ľ����ļ�·��[%s]������",iter->first,input_dir);
					theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����

					return false ;
			}else closedir(dirptr);

			memset(output_dir,0,sizeof(output_dir));
			strcpy(output_dir,iter->second.szSourcePath);
			strcat(output_dir,output_path);
			if((dirptr=opendir(output_dir)) == NULL)
			{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"����Դ[%s]������ļ�·��[%s]������",iter->first,output_dir);
					theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����

					return false ;
			}else closedir(dirptr);
		}


		Check_Sum_Conf fmt;
		char fee[256];
		memset(fee,0,sizeof(fee));
		
		theJSLog<<"�����»����ļ�������Ϣ..."<<endi;

		string sql = "select check_type,sum_table,cdr_count,cdr_duration,cdr_fee,rate_cycle from c_check_file_config ";
		Statement stmt = conn.createStatement();
		stmt.setSQLString(sql);
		stmt.execute();
		while(stmt>>fmt.check_type>>fmt.sum_table>>fmt.cdr_count>>fmt.cdr_duration>>fee>>fmt.rate_cycle)
		{
			//cout<<"�ļ����ͣ�"<<fmt.check_type<<endl;
			splitString(fee,",",fmt.cdr_fee,"true");
			monthSumMap.insert( map< string,Check_Sum_Conf >::value_type(fmt.check_type,fmt));

			memset(fee,0,sizeof(fee));
			fmt.cdr_fee.clear();
		}
		
		stmt.close();

		conn.close();
	}
	catch(SQLException  e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init ��ʼ��ʱ���ݿ��ѯ�쳣:%s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
		return false ;
	}

  if(!drInit()) return false;
   
   theJSLog<<"��ʼ�����"<<endi;

   return true ;
}

//��������Դ������Ϣ��ȡȫ������Դ�������Դ��Ϣ
int FileInAduit::LoadSourceCfg()
{
	char m_szSrcGrpID[8];
	int iSourceCount=0;
	string sql ;
	try
	{		
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

					if(getSourceFilter(SourceCfg.szSourceId,SourceCfg.filterRule,SourceCfg.file_begin,SourceCfg.file_length))
					{
							return -1;
					}
		
					m_SourceCfg[strSourceId]=SourceCfg;
				}
			}
			
			memset(m_szSrcGrpID,0,sizeof(m_szSrcGrpID));
		}
		
		stmt2.close();
		stmt.close();

	}catch (SQLException e)
	 {
		sprintf(erro_msg,"LoadSourceCfg���ݿ����%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

		return -1;
	 }

	return 0;
}

/******��������Դ��ȡ���˹��� 0û�в鵽����1�鵽������ ���ӻ�ȡ�ļ�����ʱ�����ʼλ��,�ͳ���*********************/
int FileInAduit::getSourceFilter(char* source,char* filter,int &index,int &length)
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
		sprintf(erro_msg,"getSourceFilter ���ݿ��ѯ�쳣: %s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);
		return -1 ;
	}
	catch(jsexcp::CException e)
	{
		sprintf(erro_msg,"getSourceFilter �ֶ�ת������%s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);
		return -1;
	}
	
	return 0;
}


//ɨ��˶���ϸ���г�����ļ����ٴκ˶�
int FileInAduit::check_before_file()
{
	int ret = 0;
	
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select check_file,content from d_check_file_detail where check_type  = 'AUD' and check_flag = 'N'");
	try
	{
		if(!(dbConnect(conn)))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"check_before_file() �������ݿ�ʧ�� connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
			return  ;
		}
		
		map< string,vector<string> > failFileMap;
		string f1,f2;

		Statement stmt = conn.createStatement();
		stmt.setSQLString(sql);
		stmt.execute();
		while(stmt>>f1>>f2)
		{
			map< string,vector<string> >::iterator	 it = failFileMap.find(f1);
			if(it == failFileMap.end())
			{
				vector<string> vv ;
				vv.push_back(f2);
			    failFileMap.insert(map< string,vector<string> >::value_type(f1,vv));
			}
			else
			{
				it->second.push_back(f2);
			}
		}
		

		stmt.close();
		conn.close();

		if(failFileMap.size() == 0)
		{
				return 0;
		}

		for(map< string,vector<string> >::iterator	it = failFileMap.begin();it != failFileMap.end();++it)
		{
			vector<string> vv = it->second;
			for(int i = 0;i<vv.size();i++)
			{
				ret = check_file_exist(vv[i].c_str());			
				if(ret)									//�˶Գɹ��͸���ԭ�ȵļ�¼
				{
					theJSLog<<"�ļ�["<<vv[i]<<"]���º˶Գɹ�"<<endi;

					getCurTime(currTime);				//��ȡ��ǰ�ļ�ʱ��
					memset(sql,0,sizeof(sql));
					sprintf(sql,"update d_check_file_detail set check_flag = 'Y',deal_time = '%s' where check_file = '%s' and content = '%s'",currTime,it->first,vv[i]);
					writeSQL(sql);
					memset(sql,0,sizeof(sql));
					sprintf(sql,"update d_check_file_result set suc_cnt = suc_cnt + 1 ,err_cnt = err_cnt -1 ,deal_time = '%s' where check_file = '%s' ",currTime,it->first);
					writeSQL(sql);
				}
			}

		}
		
		//��Ҫ�����ٲ���Ϣ��

		
	}
	catch (util_1_0::db::SQLException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"check_before_file() ���ݿ�����쳣%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
	}	

}

//���������ļ����Ƿ����ļ�����·�������ҵ�,��ͨ���ļ����ҵ���������Դ 0�����ڣ�1 ����
int FileInAduit::check_file_exist(char* file)
{			
	int ret = 0,flag = 0;
	CF_CFscan scan2;
	char tmp[1024],tmp2[1024], *p,out_path[1024],receive_dir[1024],filter[256];
	string source_id;

	memset(file_time,0,sizeof(file_time));
	//��ѯ�ļ���������ԴID,�ٵ�Ŀ¼����ȥ����ԭʼ�ļ�
	for(map<string,SOURCECFG>::const_iterator it = m_SourceCfg.begin();it != m_SourceCfg.end();++it)
	{
			if(checkFormat(file,it->second.filterRule))		//HDC.2013---    HD*
			{		
					theJSLog<<"�ļ�["<<file<<"]��������Դ:"<<it->first<<endi;

					flag = 1;
					source_id = it->first;
					memset(out_path,0,sizeof(out_path));
					memset(receive_dir,0,sizeof(receive_dir));
					strcpy(out_path,it->second.szSourcePath);
					strcpy(receive_dir,it->second.szSourcePath);

					strncpy(file_time,file+it->second.file_begin,it->second.file_length);
			}
	}
	
	if(flag == 0)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"�Ҳ����ļ�[%s]��������Դ",file);
		theJSLog.writeLog(LOG_CODE_PARAM_INFO_LACK,erro_msg);
		return 0;
	}

	strcat(receive_dir,receive_path);
	flag = scan2.openDir(receive_dir);		//��ѯ�ļ�����·��
	if(flag)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"���ļ�����Ŀ¼[%s]ʧ��",receive_dir);
		theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg);		//��Ŀ¼����
		return 0;
	}

	//cout<<"ɨ�����Ŀ¼"<<receive_dir<<endl;
	flag = 1;  
	memset(tmp,0,sizeof(tmp));		
	while(true)
	{
		ret = scan2.getFile("*",tmp);			//���ļ�
		
		if(ret == 0)
		{
			p = strrchr(tmp,'/');
			memset(tmp2,0,sizeof(tmp2));
			strcpy(tmp2,p+1);

			//cout<<"ɨ���ļ�["<<tmp2<<"]  �Ա��ļ�["<<file<<"]  :"<<ret<<endl;

			if(strcmp(tmp2,file) == 0)
			{							
				//ͨ���ļ����ҵ�����Դ
				//getSourFromFileName(fileList[i].fileName,source_id);
			/*				
				//ͨ������ԴID���Ҵ�����Դ����·��+������·�������ý����ļ��ŵ����·������ʽ��ʹ��
				map<string,SOURCECFG>::const_iterator it  = m_SourceCfg.find(source_id);
				if(it == m_SourceCfg.end())
				{
					sprintf(erro_msg,"�����в���������Դ%s����Ϣ",source_id);
					theJSLog.writeLog(LOG_CODE_PARAM_INFO_LACK,erro_msg);
					flag = 0;
					break;
				}
							
				memset(out_path,0,sizeof(out_path));
				strcpy(out_path,it->second.szSourcePath);
			*/
				
				//cout<<"----------ok"<<endl;
				
				theJSLog<<"�ļ�["<<file<<"]��Ŀ¼["<<receive_path<<"]�Ƶ����Ŀ¼"<<endi;

				strcat(out_path,output_path);
				strcat(out_path,tmp2);
							
				if(rename(tmp,out_path) )  //�ƶ��ļ�
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"�ƶ��ļ�ʧ��: %s",strerror(errno));
					theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
					flag = 0;
					break;
				}
		
				break ;
			}
		}
		else if(ret == 100)			//�ļ�ɨ����
		{	
			flag = 0;
			break ;	
		}
		else if(ret == -1) 			//��ȡ�ļ���Ϣʧ��
		{
			 flag = 0;
			 break;
		}
	}
				
	scan2.closeDir();	
	
	return flag;
}

//ɨ�������ļ�·�����������η������ĺ˶��ļ�����ѯ���е��ļ����Ƿ��ڵ��ȱ����еǼǣ�����¼�˶���Ϣ
void FileInAduit::run(int flag)
{
	cout<<"�������У�����������"<<flag<<endl;
	
	int ret = -1 ;

	if(gbExitSig)
	{
		if(gbExitSig) theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "Ӧ�ó����յ��˳��ź�");
		PS_Process::prcExit();
		return;
	}

	try
	{
		if(flag)
		{	
			//2013-08-07 �Ȳ�ѯ�����Ƿ��к˶�ʧ�ܵ��ļ������У����ٺ˶ԣ�����״̬
			check_before_file();

			if(scan.openDir(input_path))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"���ļ�Ŀ¼[%s]ʧ��",input_path);
				theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����
				return ;	
			}	
		}
		else
		{
			if(scan.openDir(month_input_path))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"���ļ�Ŀ¼[%s]ʧ��",month_input_path);
				theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����

				return ;	
			}	

		}

		char tmp[1024];
		int rett = 0;
		while(1)		
		{
				memset(fileName,0,sizeof(fileName));
				if(flag)
				{
					rett = scan.getFile("*.AUD",fileName); 
				}
				else
				{
					rett = scan.getFile("*.SUM",fileName);
				}

				if(rett == 100)
				{		
						break ;
				}
				else if(rett == -1)
				{	
						break ;			//��ʾ��ȡ�ļ���Ϣʧ��
				}
				
				/*�����ļ�*.tmp,*.TMP,~* */			
				char* p = strrchr(fileName,'/');
				memset(tmp,0,sizeof(tmp));
				strcpy(tmp,p+1);
			
				if(tmp[0] == '~' )	continue ;
				//if(strlen(tmp) <= 3) continue;

				if(strlen(tmp) > 3)						//�����ɿ���������ǰ��ɨ���ļ��ѹ���
				{		
						int pos = strlen(tmp)-4;
						//cout<<"��׺��Ϊ��"<<tmp+pos<<endl;
						if((strcmp(tmp+pos,".tmp") && strcmp(tmp+pos,".TMP")) == 0) 
						{
							cout<<"ɨ�赽��ʱ�ļ�������"<<fileName<<endl;
							continue;
						}
				}

				theJSLog<<"ɨ�赽�ļ���"<<fileName<<endi;
				
				strcpy(m_szFileName,p+1);

				//����ͬ����....................

				//�ж��Ǻ˶��ļ������»����ļ���ͨ���ļ������ж�
				if(flag) ret = dealFile();
				else	 ret = dealMonthFile();

				//�ٲ�......................................
		}
		
		scan.closeDir();

	}catch (jsexcp::CException &e) 
	{	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		
	}

}

//����˶��ļ����˶Թ���ͨ���ļ���ȥ���ҽ���Ŀ¼Ŀ¼�����Ƿ��и��ļ�
int FileInAduit::dealFile()
{
	int ret = 0 ;
	try
	{	
		char szBuff[1024];
		ifstream in(fileName,ios::in) ;
		if(!in)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"dealFile �ļ�%s�򿪳���",fileName);
			theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��
			return -1;
		}

		memset(szBuff,0,sizeof(szBuff));
		vector<string> vf ;
		int count = 0 ,total = 0,month = 0,cnt = 0,err_cnt = 0,suc_cnt = 0;
		while(in.getline(szBuff,sizeof(szBuff)))   
		{	
			if(count == 0)
			{
				cout<<"�ļ�ͷ:"<<szBuff<<endl;  //���Զ��ļ�ͷ���н������ж������ļ����ĸ���
				vf.clear();
				splitString(szBuff,",",vf,true);
				total = atoi(vf[3].c_str());					//�����ļ�ͷ

				total++;
				count++;
				continue;
			}
			
			if(count == total)		
			{
				cout<<"�ļ�β��"<<szBuff<<endl;
				break ;
			}

			vf.clear();
			splitString(szBuff,";",vf,true);
			if(vf.size() != 3)
			{
				theJSLog<<"�˶��ļ� ��¼�У�"<<szBuff<<"��ʽ����!"<<" �кţ�"<<count<<endi;
				err_cnt++;
				continue ;
			}

			Check_Rec_Fmt rc ;					//�����ļ���¼�У��ļ��������ڱ�־������
			rc.fileName = vf[0];
			rc.rate_flag = vf[1];
			rc.month = vf[2];
			fileList.push_back(rc);
			
			count++;
			memset(szBuff,0,sizeof(szBuff));
			
		}
		in.close();
		
		total--;
		theJSLog<<"��¼������"<<total<<endi;

		char check_flag ;
		
		getCurTime(currTime);    //��ȡ��ǰ�ļ�ʱ��
		
		for(int i = 0;i<fileList.size();i++)
		{		
				cnt = 1;
				check_flag = 'Y';
			
				cnt = check_file_exist(fileList[i].fileName.c_str());
			
				if(cnt == 0)
				{
						theJSLog<<"�ļ�["<<fileList[i].fileName<<"]�˶�ʧ��"<<endi;
						err_cnt++;
						check_flag = 'N';
				}
				else
				{		theJSLog<<"�ļ�["<<fileList[i].fileName<<"]�˶Գɹ�"<<endi;
						suc_cnt++;
				}
		
				//ÿ��ѯһ���ļ���Ҫ�Ǽ�
				getCurTime(currTime);    //��ȡ��ǰ�ļ�ʱ��
				sprintf(sql,"insert into d_check_file_detail(check_file,content,deal_time,check_flag,check_type,rate_cycle,file_time) values('%s','%s','%s','%c','AUD','%s','%s')",m_szFileName,fileList[i].fileName,currTime,check_flag,fileList[i].month,file_time);		
				writeSQL(sql);
		}

		theJSLog<<"�Ǽ��ļ��˶��ܵ����"<<endi;
		//���Ǽ��ܵ����
		sprintf(sql,"insert into d_check_file_result(check_file,total_cnt,suc_cnt,err_cnt,deal_time,check_type) values('%s',%d,%d,%d,'%s','AUD')",m_szFileName,total,suc_cnt,err_cnt,currTime);
		writeSQL(sql);
		commitSQLFile();

		fileList.clear();  //����ļ��б�

		//���ļ����� ��������������
		char bak_dir[1024];
		theJSLog<<"�ļ�"<<m_szFileName<<"���ݵ�"<<bak_path1<<endi;
		memset(bak_dir,0,sizeof(bak_dir));
		strcpy(bak_dir,bak_path1);

		strncat(bak_dir,currTime,6);
		completeDir(bak_dir);
		strncat(bak_dir,currTime+6,2);
		completeDir(bak_dir);
		
		if(chkAllDir(bak_dir) == 0)			//����ʧ��,������д��Ϊ��ʱ�ļ�,���ڵ�ǰĿ¼,��Ҫ�ֹ���Ԥ
		{
			strcat(bak_dir,m_szFileName);
			if(rename(fileName,bak_dir))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"�ļ�[%s]�ƶ�[%s]ʧ��: %s",m_szFileName,bak_dir,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);		//�ƶ��ļ�ʧ��
				rename(fileName,strcat(fileName,".tmp"));
			}
		}
		else
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"�˶��ļ�����·��[%s]�����ڣ����޷�����",bak_dir);
			theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg);		//��Ŀ¼����
			rename(fileName,strcat(fileName,".tmp"));
		}

	}
	catch (jsexcp::CException &e) 
	{	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"dealFile() %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);	
		
		ret = -1;
	}

	return ret ;
}

//�����»����ļ�  �ļ���ͷβ��û��У�飬���Կ��ǣ���������������������
int FileInAduit::dealMonthFile()
{
	int ret = -1 ;
	try
	{
		if(!(dbConnect(conn)))
		{
			sprintf(erro_msg,"dealMonthFile() �������ݿ�ʧ�� connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
			return  ;
		}
		
		
		ifstream in(fileName,ios::in) ;
		if(!in)
		{

			sprintf(erro_msg,"dealMonthFile �ļ�%s�򿪳���",sqlFile);
			theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��
			return -1;
		}

		
		int count = 0 ,total = 0,month = 0, err_cnt = 0,suc_cnt = 0;
		long cdr_cnt = 0,cdr_duration = 0;
		double totalFee = 0,feeTmp = 0;
		char check_flag ;
		
		char szBuff[1024];
		memset(szBuff,0,sizeof(szBuff));
		vector<string> vf ;
		while(in.getline(szBuff,sizeof(szBuff)))   
		{	
			if(count == 0)
			{
				cout<<"�ļ�ͷ:"<<szBuff<<endl;  
				vf.clear();
				splitString(szBuff,",",vf,true);
				total = atoi(vf[3].c_str());	//�����ļ�ͷ����¼���ͣ��ļ��������ڣ��˶����ڣ���¼����
				month = atoi(vf[2].c_str());

				total++;
				count++;
				continue;
			}
			
			if(count == total)		
			{
				cout<<"�ļ�β��"<<szBuff<<endl;
				break ;

			}

			vf.clear();
			splitString(szBuff,";",vf,false,false);
			if(vf.size() != 4)
			{
				theJSLog<<"�»����ļ� ��¼�У�"<<szBuff<<"��ʽ����!"<<" �кţ�"<<count<<endi;
				err_cnt++;					//��ʽ����ȷ ��ʱ���Ǽǵ����ݿ�
				continue ;
			}

			Check_Sum_Rec_Fmt rc ;			//�����ļ���¼��,�ļ����ͣ�����������ͨ��ʱ�����ܷ���
			rc.file_type = vf[0];
			rc.cdr_count = atol(vf[1].c_str());
			rc.cdr_duration = atol(vf[2].c_str());
			rc.cdr_fee = atof(vf[3].c_str());

			fileList2.push_back(rc);
			
			count++;
			memset(szBuff,0,sizeof(szBuff));
			
		}
		in.close();
		
		total--; //�»����ļ��ܵļ�¼��
		theJSLog<<"��¼������"<<total<<endi;

		getCurTime(currTime);    //��ȡ��ǰ�ļ�ʱ��

		//���ڿ��ܴ����ļ���ʽ����ȷ�������ܵļ�¼����һ��=ƥ�䵽�ļ�¼��
		for(int i = 0;i<fileList2.size();i++)
		{		
				check_flag = 'Y';

				Check_Sum_Rec_Fmt ff = fileList2[i];

				map< string,Check_Sum_Conf >::const_iterator iter = monthSumMap.find(ff.file_type);
				if(iter == monthSumMap.end())
				{					
						theJSLog<<"���ñ���û���ҵ��»����ļ��ļ�����Ϊ["<<ff.file_type<<"]����Ϣ"<<endi;
						check_flag = 'N';
						err_cnt++;
						sprintf(sql,"insert into d_check_file_detail(check_file,content,deal_time,check_flag,check_type,rate_cycle) values('%s','%s','%s','%c','SUM','%d')",m_szFileName,ff.file_type,currTime,check_flag,month);
						writeSQL(sql);
						continue;
				}
				
				//��ѯ�������Ӧ���е�������˶Լ�¼���кϼ�
				sprintf(sql,"select sum(%s),sum(%s)",iter->second.cdr_count,iter->second.cdr_duration);
				vector<string> vfee = iter->second.cdr_fee;
				for(int i = 0;i<vfee.size();i++)
				{
						sprintf(sql,"%s,sum(%s)",sql,vfee[i]);
				}
				sprintf(sql,"%s from %s where %s = %d",sql,iter->second.sum_table,iter->second.rate_cycle,month);

				//cout<<"sql = "<<sql<<endl;

				Statement stmt = conn.createStatement();
				stmt.setSQLString(sql);
				stmt.execute();

				stmt>>cdr_cnt>>cdr_duration;
				totalFee = 0;
				for(int i = 0;i<vfee.size();i++)
				{	
					stmt>>feeTmp;
					totalFee += feeTmp;
					
				}
				
				if(cdr_cnt != fileList2[i].cdr_count)
				{
					theJSLog<<"�ļ����ͣ�["<<ff.file_type<<"] ͨ�������˶�ʧ�� "<<cdr_cnt<<" != "<<fileList2[i].cdr_count<<endi;
					check_flag = 'N';
				}
				if(cdr_duration != fileList2[i].cdr_duration)
				{
					theJSLog<<"�ļ����ͣ�["<<ff.file_type<<"] ͨ��ʱ���˶�ʧ�� "<<cdr_duration<<" != "<<fileList2[i].cdr_duration<<endi;
					check_flag = 'N';
				}
				
				if(totalFee != fileList2[i].cdr_fee)
				{
					theJSLog<<"�ļ����ͣ�["<<ff.file_type<<"] ���ú˶�ʧ�� "<<totalFee<<" != "<<fileList2[i].cdr_fee<<endi;
					check_flag = 'N';
				}


				if(check_flag == 'Y')
				{
						theJSLog<<"�ļ����ͣ�["<<ff.file_type<<"]�˶Գɹ�"<<endi;
						suc_cnt++;
				}
				else
				{		
						//theJSLog<<"�ļ����ͣ�["<<ff.file_type<<"]�˶�ʧ��"<<endi;
						err_cnt++;
				}

				//ÿ��ѯһ���ļ����Ͷ�Ҫ�Ǽ�
				sprintf(sql,"insert into d_check_file_detail(check_file,content,deal_time,check_flag,check_type,rate_cycle) values('%s','%s','%s','%c','SUM','%d')",m_szFileName,ff.file_type,currTime,check_flag,month);
				writeSQL(sql);
			 
		}

		theJSLog<<"�Ǽ��»����ļ��ܵĺ˶����"<<endi;
		//���Ǽ��ܵ����
		sprintf(sql,"insert into d_check_file_result(check_file,total_cnt,suc_cnt,err_cnt,deal_time,check_type) values('%s',%d,%d,%d,'%s','SUM')",m_szFileName,total,suc_cnt,err_cnt,currTime);
		writeSQL(sql);
		commitSQLFile();
		
		conn.close();

		fileList2.clear();  //����ļ��б�

		//���ļ�����
		theJSLog<<"�ļ�"<<m_szFileName<<"���ݵ�"<<bak_path2<<endi;
		memset(szBuff,0,sizeof(szBuff));
		strcpy(szBuff,bak_path2);
		strcat(szBuff,m_szFileName);
		rename(fileName,szBuff);



	}catch (util_1_0::db::SQLException e)
	{
		sprintf(erro_msg,"dealFile() ���ݿ�����쳣%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
	}	
	catch (jsexcp::CException &e) 
	{	
		sprintf(erro_msg,"dealFile() %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		
	}
		
	

	return ret;
}


//���ֳ�ʼ��
bool FileInAduit::drInit()
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
int FileInAduit::drVarGetSet(char* serialString)
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
 bool FileInAduit::IsAuditSuccess(const char* dealresult)
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

bool FileInAduit::CheckTriggerFile()
{
	int ret = 0;
	if(access(m_triggerFile.c_str(),F_OK) != 0)	return false;

	theJSLog<< "��鵽trigger�ļ�����ɾ��"<< m_triggerFile <<endl;

	ret = remove(m_triggerFile.c_str());	
	if(ret) theJSLog<<"ɾ��triggerʧ��"<<endi;

}


/**************************************************
*	Function Name:	checkFormat
*	Description:	�Ƚ������ַ����Ƿ�ƥ�䣨��ȣ�
*	Input Param:
*		cmpString -------> ���Ƚϵ��ַ���
*		format	   -------> ƥ����ַ�����֧��*,?,[]��ͨ���
*	Returns:
*		��������ַ���ƥ�䣬����SUC
*		��������ַ�����ƥ�䣬����FAIL
*	complete:	2001/12/13
******************************************************/
bool FileInAduit::checkFormat(const char *cmpString,const char *format)
{
	while(1)
	{
		switch(*format)
	  	{
	  		case '\0':
					if (*cmpString == '\0')
					{
						return true;
					}
					else
					{
						return false;
					}
			case '!':
					if (checkFormat(cmpString,format + 1) == true)
					{
						return false;
					}
					else
					{
						return true;
					}
			case '?' :
					if(*cmpString == '\0')
					{
						return false;
					}
					return checkFormat(cmpString + 1,format + 1);
			case '*' :
					if(*(format+1) == '\0')
					{
						return true;
					}
					do
					{
						if(checkFormat(cmpString,format+1)==true)
						{
							return true;
						}
					}while(*(cmpString++));
					return false;
			case '[' :
					format++;
					do
					{
						
						if(*format == *cmpString)
						{
							while(*format != '\0' && *format != ']')
							{
								format++;
							}
							if(*format == ']')
							{
								format++;
							}
							return checkFormat(cmpString+1,format);			
						}
						format++;
						if((*format == ':') && (*(format+1) != ']'))
						{
							if((*cmpString >= *(format - 1)) && (*cmpString <= *(format + 1)))
							{
								while(*format != '\0' && *format != ']')
								{
									format++;
								}
								if(*format == ']')
								{
									format++;
								}
								return checkFormat(cmpString+1,format);
							}
							format++;
							format++;

						}
					}while(*format != '\0' && *format != ']');

					return false;
			default  :
					if(*cmpString == *format)
					{
						return checkFormat(cmpString+1,format+1);
					}
					else
					{
						return false;
					}
		}//switch
	}//while(1)
}

int main(int argc,char** argv)
{
	cout<<"********************************************** "<<endl;
	cout<<"*    GuangDong Telecom. Telephone Network    * "<<endl;
	cout<<"*       Centralized Settlement System        * "<<endl;
	cout<<"*                                            * "<<endl;
	cout<<"*            fileInAduit                    * "<<endl;
	cout<<"*              Version 1.0	                * "<<endl;
	cout<<"*     last update time : 2013-08-26 by  hed	   * "<<endl;
	cout<<"********************************************** "<<endl;


	FileInAduit fm ;


	if( !fm.init( argc, argv ) )
	{
		 return -1;
	}
        
	while(1)
	{
		fm.run();		//�˶��ļ�
		fm.run(0);		//�»����ļ�
		sleep(10);
	}

   return 0;
}


