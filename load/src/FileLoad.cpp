/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-06-19
File:			 FileLoad.cpp
Description:	 �ļ����ش���
History:
<table>
revision	author            date                description
--------    ------            ----                -----------
v1.0		hed          2013-06-19

</table>
**************************************************************************/

#include<dirent.h> //_chdir() _getcwd() ��ȡ�ļ�Ŀ¼�ȣ�����ļ�����Ϣ
#include<sys/types.h>
#include<sys/stat.h>  //stat()��������ѯ�ļ���Ϣ
#include<unistd.h>     //��ȡ��ǰ��������Ŀ¼
#include<fstream>
#include <sstream>

#include "FileLoad.h"

SGW_RTInfo	rtinfo;
CF_MemFileI mInfile;

FileLoad::FileLoad()
{  
	file_id = 0;
	file_num  = 0;
	split_num = 0;	
	record_num = 0;
	petri_status = DB_STATUS_ONLINE;
	petri_status_tmp = DB_STATUS_ONLINE;
	curr_record_num = 0;

	memset(m_szSourceID,0,sizeof(m_szSourceID));
	memset(mServCatId,0,sizeof(mServCatId));
	memset(m_szFileName,0,sizeof(m_szFileName));
	memset(fileName,0,sizeof(fileName));
	memset(file_name,0,sizeof(file_name));

	memset(sql,0,sizeof(sql));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(currTimeA,0,sizeof(currTimeA));
	memset(currTimeB,0,sizeof(currTimeB));

	m_record = NULL;
}

FileLoad::~FileLoad()
{
	if(m_record)
	{
		delete[] m_record;
		m_record = NULL;
	}
	
	mdrDeal.dr_ReleaseDR();
}

//ģ���ʼ������
bool FileLoad::init(int argc,char** argv)
{
   
    if(!PS_BillProcess::init(argc,argv))
    {
      return false;
    }
/*	
	vector <string> v;
	char ch;
	for(int i=0; i<1000000; i++)        
	{
		v.push_back("abcdefghijklmn");    
	}
	cin >> ch;												// ��ʱ����ڴ���� ռ��54M     
	v.clear();    
	cin >> ch;												// ��ʱ�ٴμ�飬 ��Ȼռ��54M     
	cout << "Vector �� ����Ϊ" << v.capacity() << endl;    // ��ʱ����Ϊ 1048576
	cin >> ch;
	vector<string>(v).swap(v);     
	cout << "Vector �� ����Ϊ" << v.capacity() << endl;    // ��ʱ����Ϊ0       // ����ڴ棬�ͷ��� 10M+ ��Ϊ�����ڴ�    return 0;
	cin >> ch;
*/
	//*********2013-06-22 ��ȡ���ݿ��������Ϣ��������Դ�飬����Ŀ¼ 2013-03-11 add by hed*********************************************/
	if(!(dbConnect(conn)))
	{
		cout<<"��ʼ�����ݿ� connect error."<<endl;
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
		

		sql = "select a.workflow_id,c.input_path,b.input_id,b.output_id,c.log_tabname from C_SOURCE_GROUP_DEFINE a,C_SERVICE_FLOW b,C_SERVICE_INTERFACE c where a.source_group=:1" 
                    "and a.workflow_id = b.workflow_id and b.service=:2 and b.input_id = c.interface_id ";
		stmt.setSQLString(sql);
		stmt<<mConfParam.szSrcGrpID<<mConfParam.szService;
		stmt.execute();
		if(!(stmt>>mConfParam.iWorkflowId>>mConfParam.szInPath>>mConfParam.iInputId>>mConfParam.iOutputId>>mConfParam.szSchCtlTabname))
		{
			theJSLog<<"C_SOURCE_GROUP_DEFINE,C_SERVICE_INTERFACE,C_SERVICE_INTERFACE������ѯʧ��:"<<sql<<endw;
			return false ;
		}
		completeDir(mConfParam.szInPath);
		
		theJSLog<<"WorkflowId="<<mConfParam.iWorkflowId<<"	InputId="<<mConfParam.iInputId
				<<"	 OutputId="<<mConfParam.iOutputId<<"	sch_table="<<mConfParam.szSchCtlTabname<<endi;
			
		sql = "select var_value from c_process_env where varname = 'LOAD_FILE_ERR_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<mConfParam.szSrcGrpID<<mConfParam.szService;
		stmt.execute();
		if(!(stmt>>mConfParam.szErroPath))
		{
				theJSLog<<"���ڱ�c_process_env�����ü���ģ��Ĵ���·��,LOAD_FILE_ERR_DIR"<<endw;
				return false;
		}	
		completeDir(mConfParam.szErroPath);
		
		theJSLog<<"szInPath="<<mConfParam.szInPath<<"  szErroPath="<<mConfParam.szErroPath<<endi;
		
		//2013-07-01 �����жϱ��ݱ�־��·��
		sql = "select var_value from c_process_env where varname = 'LOAD_FILE_BAK_FLAG' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<mConfParam.szSrcGrpID<<mConfParam.szService;
		stmt.execute();
		if(!(stmt>>mConfParam.bak_flag))
		{
			mConfParam.bak_flag = 'N';			 
		}
		if(mConfParam.bak_flag == 'Y')
		{
			sql = "select var_value from c_process_env where varname = 'LOAD_FILE_BAK_DIR' and source_group=:1 and service=:2 ";
			stmt.setSQLString(sql);
			stmt<<mConfParam.szSrcGrpID<<mConfParam.szService;
			stmt.execute();
			if(!(stmt>>mConfParam.szBakPath))
			{
					theJSLog<<"���ڱ�c_process_env�����ü���ģ��ı���Ŀ¼,LOAD_FILE_BAK_DIR"<<endw;
					return false;
			}
			completeDir(mConfParam.szBakPath);
			theJSLog<<"szBakPath="<<mConfParam.szBakPath<<endi;
		}

		stmt.close();
		
	   }catch(SQLException  e)
		{
			theJSLog<<"��ʼ��ʱ���ݿ��ѯ�쳣:"<<e.what()<<endw;
			return false ;
		}
	
	
	char sParamName[256];
	CString sKeyVal;
	memset(sParamName,0,sizeof(sParamName));
	sprintf(sParamName, "billing_line.%d.record_num", getFlowID());		//��ȡ�����������¼��
	param_cfg.bGetMem(sParamName, sKeyVal) ;
	mConfParam.maxRecord_num=sKeyVal.toInteger();
	theJSLog<<"sParamName="<<mConfParam.maxRecord_num<<endi;

	//2013-08-16������������ÿ��ɨ������ԴĿ¼����ָ�������ļ�������¸�����Դ
	sprintf(sParamName, "business.source.file.%d.num",getFlowID());
	if(param_cfg.bGetMem(sParamName, sKeyVal))
	{
		mConfParam.source_file_num = sKeyVal.toInteger();
		theJSLog<<sParamName<<"="<<mConfParam.source_file_num<<endi;
	}
	else
	{	
		theJSLog<<"���������ļ���������ˮ��["<<mConfParam.iflowID<<"]������Դÿ��ɨ���ļ��ĸ���,������:"<<sParamName<<endw;
		return false ;
	}	
	

	bool bb = initializeLog(argc,argv,false);  //�Ƿ����ģʽ
	if(!bb)
	{
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
		theJSLog<<"��������ʱ�ڴ���ʧ��"<<endw;
		return false;
	}	
	rtinfo.getDBSysMode(petri_status);
	petri_status_tmp = petri_status;
	theJSLog<<"current petri_status="<<petri_status<<endi;

	//theJSLog.setLog(szLogPath,szLogLevel,m_szService , m_szSrcGrpID, 001);	
	//theJSLog<<"����Դ��:"<<m_szSrcGrpID<<"   service:"<<m_szService<<"  �������·��:"<<input_path<<"  ����·��:"<<erro_path
	//		<<"	��־·��:"<<szLogPath<<" ��־����:"<<szLogLevel<<" sql���·��:"<<sql_path<<"	ÿ������Դɨ���ļ�����:"<<source_file_num<<endi;
	
	//if(mConfParam.bak_flag == 'Y')
	//{
	//	theJSLog<<"�ļ�����·��:"<<mConfParam.szBakPath<<endi;
	//}

    theJSLog<<"��������Դ������ϢLoadSourceCfg..."<<endi;
	if(LoadSourceCfg() == -1)							return false ;  //��������Դ������Ϣ
	
	conn.close();

	char input_dir[JS_MAX_FILEFULLPATH_LEN],bak_dir[JS_MAX_FILEFULLPATH_LEN],erro_dir[JS_MAX_FILEFULLPATH_LEN];
	int rett = -1;
	
	DIR *dirptr = NULL; 
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

					//return false ;
					
					theJSLog<<"����Դ��"<<iter->first<<"���������ļ�·��: "<<input_dir<<"�����ڣ����д���"<<endw;
					rett = mkdir(input_dir,0755);
					if(rett == -1)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"����Դ[%s]�Ļ����ļ�����·��[%s]�����ڣ����д���ʧ��",iter->first,input_dir);
						theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����
						return false;
					}
					
			}else closedir(dirptr);

			if(mConfParam.bak_flag == 'Y')
			{
					memset(bak_dir,0,sizeof(bak_dir));
					strcpy(bak_dir,iter->second.szSourcePath);
					strcat(bak_dir,mConfParam.szBakPath);
					if((dirptr=opendir(bak_dir)) == NULL)
					{
						theJSLog<<"����Դ��"<<iter->first<<"���ı����ļ�·��: "<<bak_dir<<"�����ڣ����д���"<<endw;
						rett = mkdir(bak_dir,0755);
						if(rett == -1)
						{
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"����Դ[%s]�ı����ļ�·��[%s]�����ڣ����д���ʧ��",iter->first,bak_dir);
							theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����

							return false;
						}
					}else closedir(dirptr);
			}
			
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

	}	

   it = m_SourceCfg.begin();   //��ʼ����һ������Դ
	
   theJSLog<<"��ʼ�����...\n"<<endi;

   return true ;
}

//��������Դ������Ϣ
int FileLoad::LoadSourceCfg()
{
	int iSourceCount=0;
	string sql;
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
		outrcd.Init(mConfParam.szOutputFiletypeId);								//2013-07-31
		
		mInfile.Init(mConfParam.szOutputFiletypeId);

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
		
		sql = "select a.source_id,b.file_fmt,b.source_path,b.SERV_CAT_ID from C_SOURCE_GROUP_CONFIG a,I_SOURCE_DEFINE b where SOURCE_GROUP=:1 and a.source_id=b.source_id";
		stmt.setSQLString(sql);
		stmt<<mConfParam.szSrcGrpID;
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

				theJSLog<<"szSourceId="<<SourceCfg.szSourceId<<"  szInFileFmt="<<SourceCfg.szInFileFmt<<" szSourcePath="<<SourceCfg.szSourcePath<<" serverCatID="<<SourceCfg.serverCatID<<" filterRule="<<SourceCfg.filterRule<<endi;
		     }
		}

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
		theJSLog.writeLog(0,erro_msg);
		return -1 ;
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
		sprintf(erro_msg,"getSourceFilter���ݿ����%s (%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
		return -1 ;
	}
	
	return 0;
}


//�ж��Ƿ�Ҫ���뻰����
int FileLoad::onBeforeTask()
{
	theJSLog.reSetLog();

	if(curr_record_num > 0) return 1 ;  //��ʾ�ϴ��ļ��ļ�¼��һ��������û������

	/**************************ɨ������Դ����ȡ�����ļ�*************************************/
	int ret = 0;
	char dir[JS_MAX_FILEPATH_LEN],inputFilePath[JS_MAX_FILEFULLPATH_LEN],filter[50]; 
	
	short db_status = 0;
	rtinfo.getDBSysMode(db_status);

	if(mdrDeal.mdrParam.drStatus == 1)  //��ϵͳ
	{	
			if(db_status != petri_status_tmp)
			{
				theJSLog<<"���ݿ�״̬�л�... "<<petri_status_tmp<<"->"<<db_status<<endw;
				int cmd_sn = 0;
				if( !putEvt(cmd_sn, EVT_RPT_DBSTATUS, 0, db_status, DSPCH_PRC_ID) )
				{
					theJSLog<<"�������ݿ����״̬ʧ�ܣ�\n"<<endw;
					return 0;
				}
				petri_status_tmp = db_status;
			}

			//���trigger�����ļ��Ƿ����
			if(!mdrDeal.CheckTriggerFile(m_triggerFile))
			{
				sleep(1);
				return 0;
			}

			//��ȡͬ������
			memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));
			ret = mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
			if(ret)
			{
				theJSLog<<"ͬ��ʧ��..."<<endw;
				return 0;
			}
			//��ȡͬ������
			vector<string> data;		
			splitString(mdrDeal.m_SerialString,";",data,false,false);  //���͵��ַ�������ԴID,�ļ���,
			
			it = m_SourceCfg.find(data[0]);
			if(it == m_SourceCfg.end())			//�����Ƿ��ٲ�
			{
				//dr_AbortIDX();
				mdrDeal.dr_abort();

				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"û���ҵ�������Դ��Ϣ[%s],dr_AbortIDX() ",data[0]);		//��������δ����
				theJSLog.writeLog(LOG_CODE_SOURCE_NOT_FIND,erro_msg);			
				return 0;
			}
			
			if(petri_status != atoi(data[2].c_str()))
			{
				theJSLog<<"��ϵͳ�����ݿ�״̬�������л�..."<<endw;
			}
			petri_status = atoi(data[2].c_str());		//��ϵͳ��״̬������ϵͳ����

			//���ļ������ļ���˽���ڴ�,
			memset(fileName,0,sizeof(fileName));
			memset(m_szFileName,0,sizeof(m_szFileName));
			memset(dir,0,sizeof(dir));
			
			strcpy(dir,it->second.szSourcePath);  //����Դ��·��
			strcpy(inputFilePath,dir);
			strcat(inputFilePath,mConfParam.szInPath);

			strcpy(fileName,inputFilePath);
			strcat(fileName,data[1].c_str());

			//int dr_GetAuditMode()1��ʾͬ����2��ʾ����, ����Ϊʧ�ܣ�-1�����ô���-2�����ļ���ȡ��������
			//int iStatus = dr_GetAuditMode(module_name);
			int iStatus = mdrDeal.mdrParam.aduitMode;

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
					//dr_AbortIDX();
					mdrDeal.dr_abort();

					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"��ϵͳ���������ļ�[%s]������,dr_AbortIDX()",fileName);
					theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);			
					return 0;
				}		
			}
			else if(iStatus == 2)		//����ģʽ,��ϵͳ
			{
				while(1)
				{
					//�����ж�
					if(gbExitSig)
					{
						//dr_AbortIDX();
						mdrDeal.dr_abort();

						theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "Ӧ�ó����յ��˳��ź�");
						//PS_Process::prcExit();
						prcExit();
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
		
			theJSLog<<"######## start deal file "<<fileName<<" ########"<<endi;

			memset(m_szSourceID,0,sizeof(m_szSourceID));
			memset(m_szFileName,0,sizeof(m_szFileName));
			memset(mServCatId,0,sizeof(mServCatId));
			strcpy(m_szFileName,data[1].c_str());
			strcpy(m_szSourceID,it->first.c_str());
			strcpy(mServCatId,it->second.serverCatID);
			
			//setSQLFileName(data[1].c_str());
	}	
	else								//��ϵͳ,������ϵͳ
	{
		if(db_status != petri_status)
		{
			theJSLog<<"���ݿ�״̬�л�... "<<petri_status<<"->"<<db_status<<endw;
			int cmd_sn = 0;
			if( !putEvt(cmd_sn, EVT_RPT_DBSTATUS, 0, db_status, DSPCH_PRC_ID) )
			{
				theJSLog<<"�������ݿ����״̬ʧ�ܣ�\n"<<endw;
				return 0;
			}
			petri_status = db_status;
			petri_status_tmp = db_status;
		}

		if(file_num >= mConfParam.source_file_num)
		{
			file_num = 0;				//��ͬһ������Դ����ɨ�赽N���ļ���,�����¸�����Դ
			sleep(5);
			++it ;
		}
		
		if(it == m_SourceCfg.end())
		{
			it = m_SourceCfg.begin();
		}

		memset(dir,0,sizeof(dir));
		strcpy(dir,it->second.szSourcePath);		 //����Դ��·��		

		memset(inputFilePath,0,sizeof(inputFilePath));
		strcpy(inputFilePath,dir);
		strcat(inputFilePath,mConfParam.szInPath); 

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
						sleep(5);
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
							continue;
						}
				}
				
				theJSLog<<"######## start deal file "<<fileName<<" ########"<<endi;
				
				memset(m_szSourceID,0,sizeof(m_szSourceID));
				memset(m_szFileName,0,sizeof(m_szFileName));
				memset(mServCatId,0,sizeof(mServCatId));

				strcpy(m_szFileName,p+1);
				strcpy(m_szSourceID,it->first.c_str());		  //��ǰ����Դ��source_id
				strcpy(mServCatId,it->second.serverCatID);	
				
				memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));
				sprintf(mdrDeal.m_SerialString,"%s;%s;%d",m_szSourceID,m_szFileName,petri_status);
				ret = mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
				if(ret)
				{
					theJSLog<<"ͬ��ʧ��...."<<endw;
					scan.closeDir();
					return 0;
				}
				
				//setSQLFileName(m_szFileName);

				break;								//�ҵ��ļ��˳�ѭ��������һ���ļ���ռ�ö��������
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
	char szBuff[JS_MAX_RECORD_LEN],tmp[JS_MAX_FILEFULLPATH_LEN],state;
	try
	{	
/*
			//���ļ����ػ��� ,Ĭ�����ļ�������ʱ�������󣬳���in��app����ʹ��
			ifstream in(fileName,ios::nocreate);
			if(!in)
			{
				dr_AbortIDX();

				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"dealFile() ���ļ�[%s]ʧ��",fileName);
				theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��

				return 0;
			}
*/			
			if(petri_status == DB_STATUS_ONLINE)
			{
				if(!(dbConnect(conn)))
				{
					//dr_AbortIDX();
					mdrDeal.dr_abort();

					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"dealFile() �������ݿ�ʧ��,dr_AbortIDX");
					theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��

					return 0;
				}
				//else
				//{
				//	stmt = conn.createStatement();
				//}			
			}

/*
			split_num = 0;	  //��ʼ���ָ������ļ���¼����
			record_num = 0;
					
			theJSLog<<"���ļ�["<<m_szFileName<<"]��¼���ص�˽���ڴ�"<<endi;	
			memset(szBuff,0,sizeof(szBuff));	
			while(in.getline(szBuff,sizeof(szBuff)))   
			{					
				PkgFmt fmt ;
				strcpy(fmt.status,"0");
				strcpy(fmt.type,"H");
				strcpy(fmt.code,"jsload");
				strcpy(fmt.record,szBuff);
				m_record.push_back(fmt);

				memset(szBuff,0,sizeof(szBuff));
			}
			in.close();

			record_num = m_record.size();
*/
			split_num = 0;	  //��ʼ���ָ������ļ���¼����
			record_num = 0;

			mInfile.Open(fileName);
			record_num = mInfile.Get_recCount();
			curr_record_num = record_num;
			m_record = new PkgFmt[record_num];
			
			int cnt = 0;
			memset(szBuff,0,sizeof(szBuff));
			while(1)
			{
				if( mInfile.readRec(szBuff, MAX_LINE_LENGTH) == READ_AT_END )
				{	
					break;									
				}

				strcpy(m_record[cnt].status,"0");
				strcpy(m_record[cnt].type,"H");
				strcpy(m_record[cnt].code,"jsload");
				strcpy(m_record[cnt].record,szBuff);
				
				cnt++;
				memset(szBuff,0,sizeof(szBuff));
			}
			mInfile.Close();

			//��ȡ�ļ�ID,�Ե�һ����¼Ϊ׼
			outrcd.Set_record(m_record[0].record);
			char fileId[10];
			memset(fileId,0,sizeof(fileId));
			outrcd.Get_Field(FILE_ID,fileId);
			file_id = atol(fileId);

			//**************���ٲ�,����Դ,�ļ���,�����������ܴ�ŵļ�¼����,��¼����
			memset(sql,0,sizeof(sql));
			memset(mdrDeal.m_AuditMsg,0,sizeof(mdrDeal.m_AuditMsg));
			sprintf(mdrDeal.m_AuditMsg,"%s;%s;%d;%d",m_szSourceID,m_szFileName,record_num,mConfParam.maxRecord_num);
	/*
			ret = IsAuditSuccess(mdrParam.m_AuditMsg);
			if(ret)
			{
				if(petri_status == DB_STATUS_ONLINE)
				{
					conn.close();
				}

				m_record.clear();		//2013-10-20
				record_num = 0;
				
				if(ret != 3)		   //2013-11-07 �ٲó�ʱ���ƶ��ļ�
				{	
					memset(tmp,0,sizeof(tmp));								//�ļ��Ƶ�����Ŀ¼
					strcpy(tmp,it->second.szSourcePath);
					strcat(tmp,mConfParam.szErroPath);
					theJSLog<<"�ƶ��ļ���ʧ��Ŀ¼:"<<tmp<<endi;
					strcat(tmp,m_szFileName);
					if(rename(fileName,tmp))
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"�ƶ��ļ�[%s]������Ŀ¼ʧ��: %s",fileName,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
					}
				}

				theJSLog<<"######## end deal file ########\n"<<endi;

				return 0;
			 }
	*/
			strcpy(file_name,m_szFileName);  //�ָ�ǰĬ����ԭʼ�ļ�����ͬ

			memset(currTimeA,0,sizeof(currTimeA)); 	
			getCurTime(currTimeA);
			//memset(sql,0,sizeof(sql));
			//����ע����ȱ�				
			//sprintf(sql,"insert into %s(source_id,serv_cat_id,filename,deal_flag,dealstarttime,record_num,split_num,file_id) values('%s','%s','%s','W','%s',%d,0,%ld)",mConfParam.szSchCtlTabname,m_szSourceID,mServCatId,m_szFileName,currTime,record_num,file_id);			
			if(petri_status == DB_STATUS_ONLINE)
			{
				stmt = conn.createStatement();
				//stmt.setSQLString(sql);
				//stmt.execute();		
			}
			else
			{
				setSQLFileName(m_szFileName);
				//writeSQL(sql);
			}

			ret = 1;
	}
	catch(jsexcp::CException e)
	{
			//dr_AbortIDX();
			mdrDeal.dr_abort();
			//m_record.clear();
			curr_record_num = 0;
			if(m_record) 
			{
				delete[] m_record;
				m_record = NULL;
			}

			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"[%s]dealFile() error %s",m_szFileName,e.GetErrMessage());
			theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ��쳣

			memset(tmp,0,sizeof(tmp));								//�ļ��Ƶ�����Ŀ¼
			strcpy(tmp,it->second.szSourcePath);
			strcat(tmp,mConfParam.szErroPath);
			strcat(tmp,m_szFileName);
			if(rename(fileName,tmp))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"�ƶ��ļ�[%s]������Ŀ¼[%s]ʧ��: %s",fileName,tmp,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
			}
			
			ret =  0;
	}
/*
	catch(util_1_0::db::SQLException e)			
	{		
			dr_AbortIDX();
			//m_record.clear();
			curr_record_num = 0;
			if(m_record) 
			{
				delete[] m_record;
				m_record = NULL;
			}

			stmt.rollback();
			stmt.close();
			conn.close();
			
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"[%s]dealFile ���ݿ����%s (%s)",m_szFileName,e.what(),sql);
			theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);	//����sqlִ���쳣
			
			memset(tmp,0,sizeof(tmp));								//�ļ��Ƶ�����Ŀ¼
			strcpy(tmp,it->second.szSourcePath);
			strcat(tmp,mConfParam.szErroPath);
			strcat(tmp,m_szFileName);
			if(rename(fileName,tmp))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"�ƶ��ļ�[%s]������Ŀ¼[%s]ʧ��: %s",fileName,tmp,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
			}

			ret = 0;
	}
*/	
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
    //theJSLog<<"�������ַ:"<<task_addr<<endi;

    memset(task_addr,0,getBlockSize());   //��ʼ�������ڴ��,����ڴ�	
	PkgBlock pkg((char*)task_addr);       //���������ʼ��
	pkg.init(getTicketLength(),getBlockSize());

	pkg.setModuleId(getModuleID());			//����ģ��ID
	pkg.setStatus(0);						//����״̬��0δ����1����
	pkg.setSourceId(it->first.c_str());		//��������Դ
	pkg.setFileHeadFlag(0);					//�����ļ�ͷ��־
	pkg.setFileTailFlag(0);					//�����ļ�β��־
	pkg.setFileType(it->second.szInFileFmt);//�����ļ�����

	theJSLog<<"��ȡ������,�����ļ�:"<<m_szFileName<<" ��ǰ���м�¼����:"<<curr_record_num<<endi;

	if(curr_record_num > mConfParam.maxRecord_num)             //��ʱ�ļ���¼��Ҫ�ָ�Ϊ���������
	{
		 curr_record_num -= mConfParam.maxRecord_num;

		 split_num++;
		 sprintf(file_name,"%s#%04d",m_szFileName,split_num);
		
		 theJSLog<<"��¼�����ڻ������¼����ָ�,�ָ����["<<split_num<<"]"<<endi;

		 pkg.setFileName(file_name);  //�����ļ���  

		 //vector<PkgFmt>::iterator  iter1 = m_record.begin();
		 //vector<PkgFmt>::iterator  iter2 = m_record.begin();
		 
		 int begin = (split_num-1)*mConfParam.maxRecord_num;
		 int end = begin + mConfParam.maxRecord_num;

		 for(int i = begin;i<end;i++)
		 {
			 pkg.writePkgRecord(m_record[i]);
			 //++iter2;
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
		 pkg.setNamalRcdNum(mConfParam.maxRecord_num);
		 pkg.setRecordNum(mConfParam.maxRecord_num); 
		
		 //ɾ��ǰǰ��N������������� 
		 //m_record.erase(iter1,iter2);
		
		 fileOverFlag = false;

		 theJSLog<<"��ǰ�����鴦�����..."<<endi;
	}
	else
	{
		int begin = 0;
		int end = record_num;

		if(split_num)					//��ʾ�ļ���¼��Ҫ���Ϊ���������
		{
			split_num++;
			sprintf(file_name,"%s#%04d",m_szFileName,split_num);
			pkg.setFileName(file_name);
			pkg.setBlkPos("E");			//��ʾ���������ļ��������һ��

			begin = (split_num-1)*mConfParam.maxRecord_num;
		}
		else
		{
			pkg.setFileName(file_name);  //�����ļ���
			pkg.setBlkPos("D");			 //��ʾ�����鵥������һ���ļ�
		}
		
		for(int i = begin;i<end;i++)
		{
			pkg.writePkgRecord(m_record[i]);
		}

		pkg.setStatus(0);
		pkg.setNamalRcdNum(curr_record_num);
		pkg.setRecordNum(curr_record_num);   
		
		//m_record.clear();  //���˽���ڴ�
		//vector<PkgFmt>().swap(m_record);	//2013-12-05 vector<int>().swap(nums)����nums.swap(vector<int>())
		curr_record_num = 0;
		delete[] m_record;
		m_record = NULL;

		fileOverFlag = true;
		
		theJSLog<<"��ǰ�����鼰�ļ��������..."<<endi;		
		
		//2013-12-09 Ԥ�ύsql���
		int ret = 0;
		memset(currTimeB,0,sizeof(currTimeB));
		getCurTime(currTimeB);
		memset(sql,0,sizeof(sql));	
		sprintf(sql,"insert into %s(source_id,serv_cat_id,filename,deal_flag,dealstarttime,dealendtime,record_num,split_num,file_id) values('%s','%s','%s','Y','%s','%s',%d,%d,%ld)",mConfParam.szSchCtlTabname,m_szSourceID,mServCatId,m_szFileName,currTimeA,currTimeB,record_num,split_num,file_id);			
		
		if(petri_status == DB_STATUS_ONLINE)
		{
			try
			{				
				stmt.setSQLString(sql);
				stmt.execute();
			}
			catch(util_1_0::db::SQLException e)	
			{ 	
				//dr_AbortIDX();
				mdrDeal.dr_abort();

				stmt.rollback();
				stmt.close();
				conn.close();
				
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"[%s]onTask() ���ݿ����%s (%s)",m_szFileName,e.what(),sql);
				theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);	//����sqlִ���쳣
				
				char tmp[JS_MAX_FILEFULLPATH_LEN];
				memset(tmp,0,sizeof(tmp));								//�ļ��Ƶ�����Ŀ¼
				strcpy(tmp,it->second.szSourcePath);
				strcat(tmp,mConfParam.szErroPath);
				strcat(tmp,m_szFileName);
				if(rename(fileName,tmp))
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"�ƶ��ļ�[%s]������Ŀ¼[%s]ʧ��: %s",fileName,tmp,strerror(errno));
					theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
				}

				pkg.setStatus(-1);		//���ļ���ʱ�ù���

				return ticket_num;
			}
		}

		ret = mdrDeal.IsAuditSuccess(mdrDeal.m_AuditMsg);					//�ٲ�Ū�����뻰������ɺ�
		if(ret)
		{
			if(petri_status == DB_STATUS_ONLINE)
			{
				stmt.rollback();
				stmt.close();
				conn.close();
			}
			//else
			//{
			//	rollBackSQL();
			//}

			pkg.setStatus(-1);		//��-1��ʾ�ٲ�ʧ��,Ԥ����,д�ļ�ģ����˸�ģʽ
			//m_record.clear();	    //2013-10-20
			//record_num = 0;
				
			if(ret != 3)		   //2013-11-07 �ٲó�ʱ���ƶ��ļ�
			{	
					char tmp[1024];
					memset(tmp,0,sizeof(tmp));								//�ļ��Ƶ�����Ŀ¼
					strcpy(tmp,it->second.szSourcePath);
					strcat(tmp,mConfParam.szErroPath);
					theJSLog<<"�ƶ��ļ���ʧ��Ŀ¼:"<<tmp<<endi;
					strcat(tmp,m_szFileName);
					if(rename(fileName,tmp))
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"�ƶ��ļ�[%s]������Ŀ¼ʧ��: %s",fileName,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
					}
			}

			theJSLog<<"######## end deal file ########\n"<<endi;

			return ticket_num;
		}

		//���»������ļ���Ϣ��д���ȱ���ɱ�־,�ٲóɹ��ı�ʶ
		//getCurTime(currTime);
		//memset(sql,0,sizeof(sql));	
		//sprintf(sql,"update %s set deal_flag = 'Y',dealendtime = '%s',split_num =%d where source_id = '%s' and fileName = '%s' and deal_flag = 'W'",mConfParam.szSchCtlTabname,currTime,split_num,m_szSourceID,m_szFileName);
		//sprintf(sql,"insert into %s(source_id,serv_cat_id,filename,deal_flag,dealstarttime,dealendtime,record_num,split_num,file_id) values('%s','%s','%s','Y','%s','%s',%d,%d,%ld)",mConfParam.szSchCtlTabname,m_szSourceID,mServCatId,m_szFileName,currTime,tmp_time,record_num,split_num,file_id);			

		if(petri_status == DB_STATUS_ONLINE)
		{
			theJSLog<<"�ύ�ļ���Ϣ�����ݿ�..."<<endi;

			stmt.commit();
			stmt.close();
			conn.close();
		}
		else
		{
			writeSQL(sql);
			theJSLog<<"���ݿ�״̬Ϊֻ��̬,дsql�ļ�..."<<endi;
			commitSQLFile();
 		}			
		
	    //���ļ�����
		if(mConfParam.bak_flag == 'Y')
		{		
			//�ж��Ƿ���Ҫ����,2013-07-16Ŀ¼����YYYYMM/DD										
			char bak_dir[JS_MAX_FILEFULLPATH_LEN];
			memset(bak_dir,0,sizeof(bak_dir));
			strcpy(bak_dir,it->second.szSourcePath);
			strcat(bak_dir,mConfParam.szBakPath);

			strncat(bak_dir,currTimeB,6);
			completeDir(bak_dir);
			strncat(bak_dir,currTimeB+6,2);
			completeDir(bak_dir);

			if(chkAllDir(bak_dir) == 0)
			{
				theJSLog<<"�����ļ�["<<m_szFileName<<"]��Ŀ¼"<<bak_dir<<endi;
				strcat(bak_dir,m_szFileName);
				if(rename(fileName,bak_dir))
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"�ļ�[%s]�ƶ���[%s]ʧ��: %s",fileName,bak_dir,strerror(errno));
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
		
		theJSLog<<"######## end deal file ########\n"<<endi;
	}
	
   //sleep(5);
   return ticket_num;
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

//2013-10-11 �����˳�����
void FileLoad::prcExit()
{
	int ret = 0;

	if(m_record)
	{
		delete[] m_record;
		m_record = NULL;
	}

	mdrDeal.dr_ReleaseDR();

	PS_Process::prcExit();
}


/*
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
int FileLoad::drVarGetSet(char* serialString)
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
			//m_AuditMsg = tmpVar;			//Ҫ�ٲõ��ַ���
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
		//if(!m_syncDr.isMaster())thelog<<"��ϵͳSerialString��"<<m_SerialString<<endi;

		return ret;

}

//�ٲ��ַ���
 int FileLoad::IsAuditSuccess(const char* dealresult)
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

bool FileLoad::CheckTriggerFile()
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

*/
