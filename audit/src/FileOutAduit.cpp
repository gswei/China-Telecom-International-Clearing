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
//#include <string>
//#include <vector>
#include <sys/types.h>
#include <sys/stat.h>  //stat()��������ѯ�ļ���Ϣ
#include<unistd.h>     //��ȡ��ǰ��������Ŀ¼
//#include<iostream>
#include<fstream>
#include "FileOutAduit.h"

//#include "CF_Common.h"
//#include "CF_CLogger.h"
CLog theJSLog;

SGW_RTInfo	rtinfo;
char last_date[9],date_path[512];

FileOutAduit::FileOutAduit()
{  
	memset(m_szFileName,0,sizeof(m_szFileName));
	memset(currTime,0,sizeof(currTime));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(sql,0,sizeof(sql));
}

FileOutAduit::~FileOutAduit()
{
	mdrDeal.dr_ReleaseDR();
}

//ģ���ʼ������
bool FileOutAduit::init(int argc,char** argv)
{
   
    if(!PS_Process::init(argc,argv))
    {
      return false;
    }
	
	mConfParam.iflowID = getFlowID();
	mConfParam.iModuleId = getModuleID();

	try
	{	
		if(!(dbConnect(conn)))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"init()  �������ݿ�ʧ�� connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
			return false ;
		}
		
		string sql;
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

		//��������Դ��Ϣ������map��ȥ
		sql = "select aduit_flag,source_array,null_out_flag,out_path from c_source_audit_env ";	
		stmt.setSQLString(sql);
		stmt.execute();
		//***********************************************************************************/
		string str ;
		AduitEnv audit ;
		char source_array[256];
		memset(source_array,0,sizeof(source_array));

		while(stmt>>str>>source_array>>audit.null_out_flag>>audit.out_path)
		{
			 theJSLog<<"audit_flag="<<str<<" source_array="<<source_array<<" null_out_flag="<<audit.null_out_flag<<" out_path="<<audit.out_path<<endi;

			 vector<string> array ,array2;
			 splitString(source_array,";",array,true);
			 audit.arrayFile = array2 ;										//�ɿ�����map����
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
		sprintf(erro_msg,"��ʼ��ʱ���ݿ��ѯ�쳣��%s (%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
		return false;
	}
	
	memset(date_path,0,sizeof(date_path));
	strcpy(date_path,getenv("SETTLEDATA"));
	completeDir(date_path);
	sprintf(date_path,"%s.%s.tmp",date_path,module_name);
	theJSLog<<"���ڴ��ȫ·��:"<<date_path<<endi;

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

	if(!(rtinfo.connect()))
	{
		theJSLog.writeLog(0,"��������ʱ��Ϣʧ��");
		return false;
	}
	rtinfo.getDBSysMode(petri_status);
	theJSLog<<"petri status:"<<petri_status<<endi;

	//theJSLog.setLog(szLogPath,szLogLevel,"AUDIT" , "GJJS", 001);	//�ļ���־�ӿڣ��������ڴ���־�ӿ�
	//theJSLog<<"��־·��:"<<szLogPath<<szLogPath<<" ��־����:"<<szLogLevel<<endi;

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


//ɨ������ļ��ǼǱ� �г����մ��͵�ERR�ļ����嵥����Ϊ�˶�����,������֤ÿ��ִֻ��һ��,����˶Ե�������ļ���,��ô�ж�?
void FileOutAduit::run()
{
	//int ret = 0;
	int ret = 0,event_sn, event_type;
	long param1, param2, src_id;
	short db_status = 0;
	char before_date[8+1];

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
		if(petri_status == DB_STATUS_OFFLINE)	
		{
			sleep(60);
			continue ;
		}
		
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
				theJSLog<<"��ϵͳͬ��ʧ��...."<<endi;
				continue ;
			}
			
			//��ȡͬ������
			//vector<string> data;		
			//splitString(mdrParam.m_SerialString,";",data,true,true);  //���͵��ַ���: ��˻���������
			
			//bool flag  = false;
			memset(before_date,0,sizeof(before_date));
			strcpy(before_date,mdrDeal.m_SerialString);	
			theJSLog<<"��˻�������"<<before_date<<endi;
		}
		else			//��ϵͳ
		{
			//ʱ���ж� ���ж�ǰ�����û������ÿ��4�����Ժ�ִ��һ��,ִ�к��ŵ��ڴ�,Ȼ��д�ļ�
			char tmpDate[8+1];
			memset(tmpDate,0,sizeof(tmpDate));
			memset(before_date,0,sizeof(before_date));
			getCurTime(currTime);
		    strncpy(tmpDate,currTime,8);
			addDays(-1,tmpDate,before_date);
			if(checkAuditBefore(before_date))		//�����Ѿ��˶����ú˶���
			{
				sleep(30);
				continue ;   
			}
			
			char tmpTime[2+1];
			memset(tmpTime,0,sizeof(tmpTime));
			strncpy(tmpTime,currTime+8,2);
			if(strcmp(tmpTime,"06") < 0)			//6�����Ժ�ִ��
			{
				sleep(30);
				continue ;
			}
			
			theJSLog<<"��˻�������:"<<before_date<<endi;
		}
		
		if(!(dbConnect(conn)))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"run() �������ݿ�ʧ�� connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
			sleep(30);

			continue  ;
		}

		theJSLog<<"######## start deal d_out_reg ########"<<endi;

		int cnt = 0;
		Statement stmt;
		
		try
		{	
			stmt = conn.createStatement();
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from d_out_file_reg where file_type = 'E' and state = 'W' and reg_time < '%s9999' ",before_date);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>cnt;
				
			theJSLog<<"�����嵥�ļ�����:"<<cnt<<endi;

			memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select fileName,source_id from d_out_file_reg where file_type = 'E' and state = 'W' and reg_time < '%s9999' ",before_date);		
			stmt.setSQLString(sql);
			stmt.execute();
			string filename ,source;
			while(stmt>>filename>>source)			//ͨ������Դ�ҵ��ļ�����������������Դ��־
			{	
				for(map< string,vector<string> >::const_iterator it=sourceMap.begin();it!=sourceMap.end();++it)
				{
					vector<string> array = it->second;
					for(vector<string>::iterator iter = array.begin();iter != array.end();++iter)
					{
						if((*iter) == source)		//�ҵ�����Դ�����Ľ�������
						{
							map< string,AduitEnv >::iterator it2 = fileNameMap.find(it->first);						
							(it2->second).arrayFile.push_back(filename);
									
							//sprintf(mdrParam.m_SerialString,"%s%s|",mdrParam.m_SerialString,filename);
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
		}
		catch (util_1_0::db::SQLException e)
		{
			vsql.clear();
			clearMap();
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"run() ���ݿ�����쳣%s (%s)",e.what(),sql);
			theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
		}
	
		if(mdrDeal.mdrParam.drStatus != 1)
		{
			sprintf(mdrDeal.m_SerialString,"%s%s",mdrDeal.m_SerialString,before_date);
			ret = mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
			if(ret)
			{
				theJSLog<<"��ϵͳͬ��ʧ��...."<<endw;
				//���fileNameMap
				clearMap();
				conn.close();
				sleep(30);
				continue ;
			}
		}

		/*************************************************************/
		char outFile[512],tmp[512];
		vector<string> fileList;

		memset(mdrDeal.m_AuditMsg,0,sizeof(mdrDeal.m_AuditMsg));
		struct stat fileInfo;
		
		getCurTime(currTime);
		//����Ӧ������Դ�����ļ� ��������Դ��־,�ļ���¼��Ŀ,�ļ���С
		for(map< string,AduitEnv >::const_iterator it3=fileNameMap.begin() ;it3!=fileNameMap.end();++it3)
		{
			vector<string> arrayFile = (it3->second).arrayFile;	
			
			sprintf(m_szFileName,"ACCT_%s_D_%s_AUD",before_date,it3->first);	 //д�˶��ļ�
			sprintf(mdrDeal.m_AuditMsg,"%s%s,%d,%c,",mdrDeal.m_AuditMsg,m_szFileName,arrayFile.size(),(it3->second).null_out_flag);

			if(arrayFile.size() == 0)
			{		
				 if((it3->second).null_out_flag == 'N')	
				 {
					 sprintf(mdrDeal.m_AuditMsg,"%s|",mdrDeal.m_AuditMsg);
					 continue ;	
				 }
			}
				
			//sprintf(m_szFileName,"ACCT_%s_D_%s_AUD",tmpTime,it3->first);	 //д�˶��ļ�

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
				clearMap();
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"run() �ļ�%s�򿪳���",m_szFileName);
				theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��
				continue ;
			}
		
			//д�ļ�ͷ
			out<<10<<","<<currTime<<","<<before_date<<","<<arrayFile.size()<<endl;
			for(int k = 0;k<arrayFile.size();k++)
			{		
				out<<arrayFile[k]<<"\n";					
			}		
		
			out<<"90"<<endl;
			out.close();	

			stat(outFile,&fileInfo); //��ȡ�ļ���С
			sprintf(mdrDeal.m_AuditMsg,"%s%ld|",mdrDeal.m_AuditMsg,fileInfo.st_size);	
		}
	
		clearMap();

		char state = 'Y';
		ret = mdrDeal.IsAuditSuccess(mdrDeal.m_AuditMsg);
		if(ret)											//�ٲ�ʧ��,�ع����ݿ�,ɾ����ʱ�ļ�
		{		
			//getCurTime(currTime);
			//memset(sql,0,sizeof(sql));
			//sprintf(sql,"update d_out_file_reg set state = 'E' , deal_time = '%s' where state = 'H'",currTime);
			//vsql.push_back(sql);	
			//ret = updateDB();

			theJSLog<<"ɾ����ʱ����ļ�..."<<endi;
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
			state = 'E';
			//fileList.clear();
			//conn.close();
			//return ;
		}
		else
		{
			//��ʱ�ļ�д��ʽ�ļ�
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
		
		fileList.clear();

		theJSLog<<"�������ݿ���¼״̬..."<<endi;
		getCurTime(currTime);
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update d_out_file_reg set state = '%c' , deal_time = '%s' where state = 'H'",state,currTime);
		vsql.push_back(sql);

		ret = updateDB();							//��ʱ������ʧ�����	
		conn.close();

		memset(last_date,0,sizeof(last_date));
		strcpy(last_date,before_date);
		ofstream out(date_path,ios::app);
		if(!out)
		{
			theJSLog.writeLog(LOG_CODE_FILE_WRITE_ERR,"��¼��������ļ���Ϣʧ��");
		}
		else
		{
			out<<last_date<<"\n";
			out.close();
		}	
	}
/*
	catch (util_1_0::db::SQLException e)
	{
		vsql.clear();
		clearMap();
		fileList.clear();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() ���ݿ�����쳣%s (%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
	}
*/
	catch (jsexcp::CException &e) 
	{	
		vsql.clear();
		clearMap();
		fileList.clear();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		
	}
		
	theJSLog<<"######## end deal ########\n"<<endi;
	
	sleep(30);
 } //while(1)

	//return ;
}

//�Ȳ�ѯ�ڴ�,�ٲ�ѯ�ļ�
bool FileOutAduit::checkAuditBefore(char* date)
{
	if(strcmp(date,last_date) == 0)  return true;
	
	char szBuff[1024];
	ifstream in(date_path,ios::in) ;			//�������ļ��������򴴽�
	if(!in)
	{
			//memset(erro_msg,0,sizeof(erro_msg));
			//sprintf(erro_msg,"checkAuditBefore() �ļ�[%s]�򿪳���",date_path);
			//theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��
			theJSLog<<"�ļ�["<<date_path<<"]��ʧ��"<<endw;
			return false;
	}

	memset(szBuff,0,sizeof(szBuff));
	while(in.getline(szBuff,sizeof(szBuff)))   
	{	
		if(strcmp(szBuff,date) == 0) 
		{
			in.close();
			return true;
		}
		memset(szBuff,0,sizeof(szBuff));
	}

	in.close();

	return false;
}

//�����ύsql,��֤һ������������
int FileOutAduit::updateDB()
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

//���Map
void FileOutAduit::clearMap()
{
	//for(map< string,AduitEnv >::const_iterator it3=fileNameMap.begin() ;it3!=fileNameMap.end();++it3)
	//{
	//	vector<string> arrayFile = (it3->second).arrayFile;		
	//	arrayFile.clear();
	//}
	//2013-12-02�޸�
	for(map< string,AduitEnv >::iterator it3=fileNameMap.begin();it3!=fileNameMap.end();++it3)
	{
		((it3->second).arrayFile).clear();		
	}
}

//2013-10-11 �����˳�����
void FileOutAduit::prcExit()
{
	mdrDeal.dr_ReleaseDR();

	PS_Process::prcExit();
}

/*
//���ֳ�ʼ��
bool FileOutAduit::drInit()
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
int FileOutAduit::drVarGetSet(char* serialString)
{
		int ret  = 0;
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
 int FileOutAduit::IsAuditSuccess(const char* dealresult)
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

bool FileOutAduit::CheckTriggerFile()
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
	cout<<"*            jsfileAduit                      "<<endl;
	cout<<"*            sys.GJZW.Version 1.0	         "<<endl;
	cout<<"*     created time :      2013-07-20 by  hed	 "<<endl;
	cout<<"*     last update time :  2013-12-16 by  hed	 "<<endl;
	cout<<"********************************************* "<<endl;


	FileOutAduit fm ;

	if( !fm.init( argc, argv ) )
	{
		 return -1;
	}
        
	//while(1)
	//{
	//	theJSLog.reSetLog();
		fm.run();
	//	sleep(30);
	//}

   return 0;
}


