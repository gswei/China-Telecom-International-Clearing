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

#include<dirent.h>			//_chdir() _getcwd() ��ȡ�ļ�Ŀ¼�ȣ�����ļ�����Ϣ
#include<sys/types.h>
#include<sys/stat.h>		//stat()��������ѯ�ļ���Ϣ
#include<unistd.h>			//��ȡ��ǰ��������Ŀ¼
#include<fstream>
#include <sstream>
#include "indb.h"

#define MAX_CHLD_PRC_NUM  10
typedef struct
{
	pid_t pid;
	char  state;
}CHLD_PRC_INFO;
CHLD_PRC_INFO g_stChldPrcInfo[MAX_CHLD_PRC_NUM];
int  g_iChldPrcNum;
bool gbNormal=true;

CLog theJSLog;
SGW_RTInfo	rtinfo;
CF_MemFileI mInfile;

C_Indb::C_Indb()
{  	
	petri_status = DB_STATUS_ONLINE ;
	petri_status_tmp = DB_STATUS_ONLINE;
	record_num = 0;
	file_id = 0;
	file_num = 0;

	memset(file_time,0,sizeof(file_time));
	memset(mServCatId,0,sizeof(mServCatId));
	memset(m_szSourceID,0,sizeof(m_szSourceID));
	memset(m_szFileName,0,sizeof(m_szFileName));
	memset(fileName,0,sizeof(fileName));

	memset(sql,0,sizeof(sql));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(currTime,0,sizeof(currTime));
}

C_Indb::~C_Indb()
{
	mdrDeal.dr_ReleaseDR();
	
	if(TabConf != NULL)
	{
		delete[] TabConf;
	}
}

void ChldExitPrc(int sig_type)
{
	pid_t pid;
	while(1)
	{
		pid=waitpid(-1,0,WNOHANG);
		if(pid<=0) break;
		for(int i=0; i<MAX_CHLD_PRC_NUM; i++)
		{
			if(pid==g_stChldPrcInfo[i].pid)
			{
				if(g_stChldPrcInfo[i].state==PRCST_STOPPING)
					cerr<<"�ӽ���"<<i+1<<"�˳�"<<endl;
				else if(g_stChldPrcInfo[i].state==PRCST_STARTING)
				{
					g_stChldPrcInfo[i].state=PRCST_INACTV;
					break;
				}
				else
				{
					cerr<<"�ӽ���"<<i+1<<"�쳣�˳�"<<endl;
					gbNormal=false;
				}
				g_stChldPrcInfo[i].pid=0;
				g_stChldPrcInfo[i].state=PRCST_INACTV;
				g_iChldPrcNum--;
				break;
			}
		}
	}
}

//ģ���ʼ������
bool C_Indb::init(int argc,char** argv)
{ 
	m_iMainPid=getpid();

    if(!PS_Process::init(argc,argv))
    {
      return false;
    }
	
	char sParamName[256];
	CString sKeyVal;
	memset(sParamName,0,sizeof(sParamName));

	int chldprc_evtchnl_key;
	if( param_cfg.bGetMem("memory.MT_DSPCH_CHILD.msgq_key_value", sKeyVal) && sKeyVal.isNumber() )
	{
		chldprc_evtchnl_key=sKeyVal.toInteger();
	}
	else
	{
		cerr<<"δ���ø��ӽ��̼���Ϣͨ����ֵ\n";
		return false;
	}

	m_iChldPrcMsgID=msgget(chldprc_evtchnl_key, 0600);
	if(m_iChldPrcMsgID<0)
	{
		cerr<<"���Ӹ��ӽ���ͨ��ͨ��ʧ�ܣ�\n";
		return false;
	}

	cerr<<"���ڽ��н��̳�ʼ��...\n";

	if( !onInit() )
	{
		prcExit();
		return false;
	}
	
	int ret=1;
	int event_sn, event_type;
	long param1, param2, src_id;
	while(ret)	//����ӽ�����Ϣ����
	{
		ret=getChldEvt(event_sn, event_type, param1, param2, src_id, false);
		if(ret<0)
		{
			cerr<<"����ӽ�����Ϣ����ʧ��!\n";
			return false;
		}
	}
	
	//PS_Process::setSignalPrc();   //add by hed 2013-02-01 ��������Ƶ�����Process�е�init��ȥ��

	g_iChldPrcNum=0;
	memset(&g_stChldPrcInfo, 0, sizeof(g_stChldPrcInfo));

	struct sigaction siginfo;
	siginfo.sa_handler=ChldExitPrc;
	sigemptyset(&siginfo.sa_mask);
	siginfo.sa_flags=0;
	sigaction(SIGCHLD,&siginfo,0);

	///////////////////////////////////////////////////////////////////////////////////////////
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
		
		sql = "select var_value from c_process_env where varname = 'INDB_FILE_ERR_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<mConfParam.szSrcGrpID<<mConfParam.szService;
		stmt.execute();
		if(!(stmt>>mConfParam.szErroPath))
		{
				theJSLog<<"���ڱ�c_process_env�����ü���ģ��Ĵ���·��,INDB_FILE_ERR_DIR"<<endw;
				return false;
		}	
		completeDir(mConfParam.szErroPath);

		sql = "select var_value from c_process_env where varname = 'INDB_FILE_BAK_FLAG' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<mConfParam.szSrcGrpID<<mConfParam.szService;
		stmt.execute();
		if(!(stmt>>mConfParam.bak_flag))
		{
			mConfParam.bak_flag = 'N';			 
		}
		if(mConfParam.bak_flag == 'Y')
		{
			sql = "select var_value from c_process_env where varname = 'INDB_FILE_BAK_DIR' and source_group=:1 and service=:2 ";
			stmt.setSQLString(sql);
			stmt<<mConfParam.szSrcGrpID<<mConfParam.szService;
			stmt.execute();
			if(!(stmt>>mConfParam.szBakPath))
			{
					theJSLog<<"���ڱ�c_process_env�����ü���ģ��ı���Ŀ¼,INDB_FILE_BAK_DIR"<<endw;
					return false;
			}
			completeDir(mConfParam.szBakPath);
			theJSLog<<"szBakPath="<<mConfParam.szBakPath<<endi;
		}	
		
		//2014-07-07 �����õ��ӽ��̸���
		sql = "select var_value from c_process_env where varname = 'INDB_CHILD_NUM' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<mConfParam.szSrcGrpID<<mConfParam.szService;
		stmt.execute();
		if(!(stmt>>mConfParam.child_num))
		{
				mConfParam.child_num=1;
		}	
		
		stmt.close();
		
	   }catch(SQLException  e)
		{
			theJSLog<<"��ʼ��ʱ���ݿ��ѯ�쳣:"<<e.what()<<endw;
			return false ;
		}
	
	
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
	
	if(mConfParam.child_num > mConfParam.source_file_num)
	{
		mConfParam.child_num = mConfParam.source_file_num;
	}
	theJSLog<<"mConfParam.child_num="<<mConfParam.child_num<<endi;


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
	//theJSLog<<"  �������·��:"<<input_path<<"  ����·��:"<<erro_path<<"	��־·��:"<<szLogPath<<" ��־����:"
			//<<szLogLevel<<"	ÿ������Դɨ���ļ�����:"<<source_file_num<<endi;
	
	//if(bak_flag == 'Y')
	//{
	//	theJSLog<<"�ļ�����·��:"<<bak_path<<endi;
	//}

    theJSLog<<"��������Դ������ϢLoadSourceCfg..."<<endi;

	if(LoadSourceCfg() == -1) return false ;  //��������Դ������Ϣ
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
							sprintf(erro_msg,"����Դ[%s]�ı����ļ��ļ�·��[%s]�����ڣ����д���ʧ��",iter->first,bak_dir);
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

				if(getSourceFilter(SourceCfg.szSourceId,SourceCfg.filterRule,SourceCfg.file_begin,SourceCfg.file_length))
				{
							return -1;
				}
				
				m_SourceCfg[strSourceId]=SourceCfg;
				theJSLog<<"szSourceId="<<SourceCfg.szSourceId<<" szSourcePath="<<SourceCfg.szSourcePath<<" serverCatID="<<SourceCfg.serverCatID
						<<" filterRule="<<SourceCfg.filterRule<<"  filetime_begin="<<SourceCfg.file_length<<"  filetime_length="<<SourceCfg.file_length<<endi;
		     }
		}

		
		//����ÿ������Դ��Ӧ��ͳ������
		string source_id;
		int config_id = 0;
		sql = "select source_id, var_value from c_source_env a  where a.varname = 'INS_TABLE_CONFIGID' and a.service =:1";
		stmt.setSQLString(sql);
		stmt<<mConfParam.szService;
		stmt.execute();
		while(stmt>>source_id>>config_id)
		{
				mapConfig.insert(map<string,int>::value_type(source_id,config_id));
		}
		stmt.close();
		
		TabConf = new CF_CNewError_Table[m_SourceCfg.size()];
		int index = 0;
		for(map<string,SOURCECFG>::const_iterator iter = m_SourceCfg.begin();iter !=m_SourceCfg.end(); ++iter)
		{
			map<string,int>::const_iterator iter2 = mapConfig.find(iter->first);
			if(iter2 == mapConfig.end())
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"��source_env������Դ��[%s]���õ�����Դ[%s]û���������ı��� INS_TABLE_CONFIGID",mConfParam.szSrcGrpID,iter->first);
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);

				return -1;
			}
			
			theJSLog<<"source_id = "<<iter->first<<" config_id = "<<iter2->second<<endi;
			TabConf[index].init(iter2->second,mConfParam.szOutputFiletypeId);
			mapTabConf.insert(map< string,int >::value_type(iter->first,index));

			index++;
				
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

//����ʽѡ��  ������or�����
void C_Indb::dealType()
{
	if(mConfParam.child_num <= 1)
	{
		theJSLog<<"������ģʽ..."<<endi;
		run();
	}
	else
	{
		theJSLog<<"�����ģʽ..."<<endi;
		run2();
	}
}

//ѭ��ɨ���������Դ
void C_Indb::run()
{	
	//int ret = 0;
	int ret = 0,event_sn, event_type;
	long param1, param2, src_id;

	short db_status = 0;
	char dir[512],inputFilePath[512],filter[50],szFiletypeIn[10];	
	
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
	
	if(mdrDeal.mdrParam.drStatus == 1 )  //��ϵͳ
	{	
			rtinfo.getDBSysMode(db_status);
			if(db_status != petri_status_tmp)
			{
				theJSLog<<"���ݿ�״̬�л�... "<<petri_status_tmp<<"->"<<db_status<<endw;
				int cmd_sn = 0;
				if( !putEvt(cmd_sn, EVT_RPT_DBSTATUS, 0, db_status, DSPCH_PRC_ID) )
				{
					theJSLog<<"�������ݿ����״̬ʧ�ܣ�\n"<<endw;
					continue ;
				}
				petri_status_tmp = db_status;
			}

			//���trigger�����ļ��Ƿ����
			if(!mdrDeal.CheckTriggerFile(m_triggerFile))
			{
				sleep(1);
				continue ;
			}

			//��ȡͬ������
			memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));
			ret = mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
			if(ret)
			{
				theJSLog<<"ͬ��ʧ��..."<<endw;
				continue ;
			}
	
			//��ȡͬ������
			vector<string> data;		
			splitString(mdrDeal.m_SerialString,";",data,false,false);  //���͵��ַ�������ԴID,�ļ���,sqlFile�ļ���
			
			it = m_SourceCfg.find(data[0]);
			if(it == m_SourceCfg.end())			//�����Ƿ��ٲ�
			{
				//dr_AbortIDX();
				mdrDeal.dr_abort();

				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"û���ҵ�������Դ��Ϣ[%s]",data[0]);		//��������δ����
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);		  
				continue ;
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
					sprintf(erro_msg,"��ϵͳ���������ļ�[%s]������",fileName);
					theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);		
					continue ;
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

			ret = dealFile();
			//if(petri_status == DB_STATUS_ONLINE)
			//{
			//	conn.close();
			//}

			theJSLog<<"######## end deal file ########\n"<<endi;
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
		strcat(inputFilePath,mConfParam.szInPath); 

		memset(filter,0,sizeof(filter));
		memset(szFiletypeIn,0,sizeof(szFiletypeIn));  
		strcpy(filter,it->second.filterRule);		  //��������
		strcpy(szFiletypeIn,it->second.szInFileFmt);  //��ǰ����Դ�������ʽ
		strcpy(m_szSourceID,it->first.c_str());
		
		memset(mServCatId,0,sizeof(mServCatId));
		strcpy(mServCatId,it->second.serverCatID);

		//�򿪻����ļ�Ŀ¼
		if(scan.openDir(inputFilePath))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"�򿪻����ļ�Ŀ¼[%s]ʧ��",inputFilePath);
			theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����
			it++;
			continue ;	
		}		
						
		//ѭ����ȡĿ¼��ɨ���ļ��У���ȡ�ļ�  ��Ϊ������ʱ�ļ������Ի�ɨ��10��
		int rett = -1 ;
		char tmp[512];
		file_num = 0;
		while(1)		
		{		
				ret=getCmd(event_sn, event_type, param1, param2, src_id);
				if(ret == 1)
				{
					if(event_type == EVT_CMD_STOP)
					{
						scan.closeDir();
						theJSLog<<"***********���յ��˳�����**********************\n"<<endw;
						prcExit();
					}
				}
				
				rtinfo.getDBSysMode(db_status);
				if(db_status != petri_status)
				{
					theJSLog<<"���ݿ�״̬�л�... "<<petri_status<<"->"<<db_status<<endw;
					int cmd_sn = 0;
					if( !putEvt(cmd_sn, EVT_RPT_DBSTATUS, 0, db_status, DSPCH_PRC_ID) )
					{
						theJSLog<<"�������ݿ����״̬ʧ�ܣ�\n"<<endw;
						break;
					}
					petri_status = db_status;
					petri_status_tmp = db_status;
				}

				if(file_num == mConfParam.source_file_num)
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
				
				theJSLog<<"######## start deal file "<<fileName<<" ########"<<endi;
				strcpy(m_szFileName,p+1);

				//����ͬ����Ϣ
				memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));
				sprintf(mdrDeal.m_SerialString,"%s;%s;%d",m_szSourceID,m_szFileName,petri_status);
				ret = mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
				if(ret)
				{
					theJSLog<<"ͬ��ʧ��...."<<endw;
					continue ;
				}
				
				ret = dealFile();
				//if(petri_status == DB_STATUS_ONLINE)
				//{
				//	conn.close();
				//}

				theJSLog<<"######## end deal file ########\n"<<endi;
		}

		scan.closeDir();
		++it;
	}

	//conn.close();
	
	sleep(5);
  }//while(1)

}

//���ļ���� ret = 0��ʾ���ɹ� ret= -1��ʾ���ʧ��
int C_Indb::dealFile()
{	
	int ret = 0;
	char szBuff[1024];
	
	memset(mdrDeal.m_AuditMsg,0,sizeof(mdrDeal.m_AuditMsg));	
	theJSLog<<"�ļ�["<<m_szFileName<<"]���"<<endi;

	map< string,int>::const_iterator iter =  mapTabConf.find(m_szSourceID);
	if( iter == mapTabConf.end())
	{
		//dr_AbortIDX();
		mdrDeal.dr_abort();

		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"indb() ���ݿ���û������Դ%s��������Ϣ",m_szSourceID);
		theJSLog.writeLog(LOG_CODE_SOURCE_NOT_FIND,erro_msg);
		return -1;
	}

	record_num = 0;
	Statement stmt;
	char state = 'Y';
	CF_CNewError_Table* tab;
	
	try
	{	
		ifstream in(fileName,ios::nocreate);
		if(!in)
		{
			//dr_AbortIDX();
			mdrDeal.dr_abort();

			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"dealFile() �ļ�[%s]�򿪳���",fileName);
			theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��
			
			return -1;
		}

		in.getline(szBuff,sizeof(szBuff));
		//2013-10-01  ������ȡfile_id ��file_time
		memset(file_time,0,sizeof(file_time));
		strncpy(file_time,m_szFileName+it->second.file_begin,it->second.file_length);
		file_time[8] = '\0';
		char fileid[10],rate_cycle[6+1];
		memset(fileid,0,sizeof(fileid));
		outrcd.Set_record(szBuff);
		outrcd.Get_Field(FILE_ID,fileid);
		file_id = atol(fileid);
		
		memset(rate_cycle,0,sizeof(rate_cycle));
		outrcd.Get_Field(RATE_CYCLE,rate_cycle);
		rate_cycle[6] = '\0';

		tab = &TabConf[iter->second];
		
		if(petri_status == DB_STATUS_ONLINE)
		{
			if(!(dbConnect(conn)))
			{
				//dr_AbortIDX();
				mdrDeal.dr_abort();

				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"run() �������ݿ�ʧ�� connect error");
				theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
				
				tab = NULL;
				return -1;
			}
			
			tab->setBegin(conn);
			tab->setConfig(file_id,rate_cycle);
			
			do 			  
			{
				tab->insertData(szBuff);
				record_num++;
				memset(szBuff,0,sizeof(szBuff));		
			}while(in.getline(szBuff,sizeof(szBuff))); 	
			
		}
		else
		{
			setSQLFileName(m_szFileName);
			tab->setRWFlag(false);
			tab->setConfig(file_id,rate_cycle);
			
			mInfile.Open(fileName);
			tab->setSQLconf(getSQLFileName(),mInfile.Get_recCount());
			mInfile.Close();

			do	   
			{
				tab->writeSQL(szBuff);
				record_num++;
				memset(szBuff,0,sizeof(szBuff));		
			}while(in.getline(szBuff,sizeof(szBuff)));	
		}
							
		in.close();	
	
	}catch (jsexcp::CException e)
	{
		tab->RollBack();
		state = 'E';
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"[%s]indb() error line=(%d) :%s ",m_szFileName,record_num,e.GetErrMessage());
		theJSLog.writeLog(0,erro_msg);
	}
		
		//**************���ٲ�,����Դ,�ļ���,�����������ܴ�ŵļ�¼����,��¼����
		char tmp[512];
		sprintf(mdrDeal.m_AuditMsg,"%s;%s;%c;%d",m_szSourceID,m_szFileName,state,record_num);
	
		ret = mdrDeal.IsAuditSuccess(mdrDeal.m_AuditMsg);
		if(ret)
		{
			tab->RollBack();
			tab = NULL;

			if(petri_status == DB_STATUS_ONLINE)
			{
				conn.close();
			}

			if(ret != 3)								//2013-11-07 �ٲó�ʱ���ƶ��ļ�
			{
				memset(tmp,0,sizeof(tmp));				//�ļ��Ƶ�����Ŀ¼
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

			return -1;
		 }

   try
   {   
		theJSLog<<"���������ȱ�..."<<endi;
		getCurTime(currTime); 
		memset(sql,0,sizeof(sql));	
		sprintf(sql,"insert into %s (source_id,serv_cat_id,filename,deal_flag,dealtime,mainflow_count,file_id,file_time) values('%s','%s','%s','%c','%s',%d,%ld,'%s')",mConfParam.szSchCtlTabname,m_szSourceID,mServCatId,m_szFileName,state,currTime,record_num,file_id,file_time);
		if(petri_status == DB_STATUS_ONLINE)
		{			
			tab->Commit();
			stmt = conn.createStatement();
			stmt.setSQLString(sql);
			stmt.execute();
			stmt.close();
			conn.close();		
		}
		else
		{
			//char ssql[2048] ;
			//vector<string> vv = tab->getvSQL();
			//for(vector<string>::iterator iter = vv.begin();iter != vv.end();++iter)
			//{
			//	memset(ssql,0,sizeof(ssql));
			//	strcpy(ssql,(*iter).c_str());
			//	writeSQL(ssql);
			//}	

			tab->Commit();

			writeSQL(sql);
			commitSQLFile();

			//tab->RollBack();
		}	

	}catch(SQLException e)
	{
		stmt.rollback();
		conn.close();
		state = 'E';
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"[%s]indb() sql error %s (%s)",m_szFileName,e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);
    }
		
		tab = NULL;

		if(state == 'E')		//���ʧ��,�ļ��Ƶ�����Ŀ¼
		{
			theJSLog<<"����쳣�ļ�"<<m_szFileName<<"�Ƶ�ʧ��Ŀ¼"<<endi;

			memset(tmp,0,sizeof(tmp));								
			strcpy(tmp,it->second.szSourcePath);
			strcat(tmp,mConfParam.szErroPath);
			strcat(tmp,m_szFileName);
			if(rename(fileName,tmp))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"�ƶ��ļ�[%s]������Ŀ¼ʧ��: %s",fileName,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
			}
			
			return -1;
		}
		
		//���ļ�����
		if(mConfParam.bak_flag == 'Y')
		{		
			//�ж��Ƿ���Ҫ����										
			char bak_dir[512];
			memset(bak_dir,0,sizeof(bak_dir));
			strcpy(bak_dir,it->second.szSourcePath);
			strcat(bak_dir,mConfParam.szBakPath);

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

	theJSLog<<"�ļ����ɹ�..."<<endi;
	
	return ret;
}

//����ʧ�ܵ��ļ�����
int C_Indb::getFileExist()
{
	int  count = 0;

	for(int i = 0;i<mVfile.size();i++)
	{
		memset(fileName,0,sizeof(fileName));

		strcpy(fileName,it->second.szSourcePath);
		strcat(fileName,mConfParam.szInPath);
		strcat(fileName,mVfile[i].c_str());

		if(access(fileName,F_OK|R_OK))
		{
			theJSLog<<"�ļ�["<<mVfile[i].c_str()<<"]������!"<<endw;
			count++;
			continue;
		}
	}

	return count;
}


//ѭ��ɨ���������Դ ����� #############################################
void C_Indb::run2()
{	
	//int ret = 0;
	int ret = 0,event_sn, event_type;
	long param1, param2, src_id;

	short db_status = 0;
	char dir[512],inputFilePath[512],filter[50],szFiletypeIn[10];	
	
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
	
	if(mdrDeal.mdrParam.drStatus == 1 )  //��ϵͳ
	{	
			rtinfo.getDBSysMode(db_status);
			if(db_status != petri_status_tmp)
			{
				theJSLog<<"���ݿ�״̬�л�... "<<petri_status_tmp<<"->"<<db_status<<endw;
				int cmd_sn = 0;
				if( !putEvt(cmd_sn, EVT_RPT_DBSTATUS, 0, db_status, DSPCH_PRC_ID) )
				{
					theJSLog<<"�������ݿ����״̬ʧ�ܣ�\n"<<endw;
					continue ;
				}
				petri_status_tmp = db_status;
			}

			//���trigger�����ļ��Ƿ����
			if(!mdrDeal.CheckTriggerFile(m_triggerFile))
			{
				sleep(1);
				continue ;
			}

			//��ȡͬ������
			memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));
			ret = mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
			if(ret)
			{
				theJSLog<<"ͬ��ʧ��..."<<endw;
				continue ;
			}
	
			//��ȡͬ������
			vector<string> data;	
			splitString(mdrDeal.m_SerialString,";",data,false,false);  //���͵��ַ�������ԴID,�ļ���,sqlFile�ļ���
			
			it = m_SourceCfg.find(data[0]);
			if(it == m_SourceCfg.end())			//�����Ƿ��ٲ�
			{
				//dr_AbortIDX();
				mdrDeal.dr_abort();

				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"û���ҵ�������Դ��Ϣ[%s]",data[0]);		//��������δ����
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);		  
				continue ;
			}
			
			if(petri_status != atoi(data[1].c_str()))
			{
				theJSLog<<"��ϵͳ�����ݿ�״̬�������л�..."<<endw;
			}
			petri_status = atoi(data[1].c_str());		//��ϵͳ��״̬������ϵͳ����
			
			//mVfile.clear();
			for(int i=2;i<data.size();i++)				//�ļ����ӵ�����Ԫ�ؿ�ʼ
			{
				mVfile.push_back(data[i]);
			}

			//���ļ������ļ���˽���ڴ�,
			//memset(fileName,0,sizeof(fileName));
			//memset(m_szFileName,0,sizeof(m_szFileName));
			memset(dir,0,sizeof(dir));
			
			strcpy(dir,it->second.szSourcePath);  //����Դ��·��
			strcpy(inputFilePath,dir);
			strcat(inputFilePath,mConfParam.szInPath);

			//strcpy(fileName,inputFilePath);
			//strcat(fileName,data[2].c_str());	//�ļ��� data[2...N]

			//int dr_GetAuditMode()1��ʾͬ����2��ʾ����, ����Ϊʧ�ܣ�-1�����ô���-2�����ļ���ȡ��������
			//int iStatus = dr_GetAuditMode(module_name);

			int iStatus = mdrDeal.mdrParam.aduitMode;
			if(iStatus == 1)		//ͬ��ģʽ,	��ϵͳ�ȴ�ָ��ʱ�� 
			{
					bool flag=false;
					int times=1,count = 0;
					while(times<31)
					{
						count = getFileExist();
						if(count)
						//if(access(filenames,F_OK|R_OK))
						{
							theJSLog<<"������"<<times<<"��,����ʧ���ļ�����:"<<count<<endw;
							times++;
							sleep(10);
						}
						else
						{
							flag=true;
							break;
						}
					}
					if(!flag)
					{
						//dr_AbortIDX();
						mdrDeal.dr_abort();
						mVfile.clear();

						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"��ϵͳ���������ļ�����");
						theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);			
						continue ;
					}
				}
				else if(iStatus==2) //����ģʽ����ϵͳ
				{
					int count = 0;
					while(1)
					{
						//�����ж�
						if(gbExitSig)
						{
							//dr_AbortIDX();
							mdrDeal.dr_abort();
							
							theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "Ӧ�ó����յ��˳��ź�");
							prcExit();
							return ;
						}
						
						count = getFileExist();
						if(count)
						//if(access(filenames,F_OK|R_OK))
						{
							sleep(10);
						}
						else
						{
							break;
						}
					}
				}
			
			
			//theJSLog<<"######## start deal file "<<fileName<<" ########"<<endi;

			memset(m_szSourceID,0,sizeof(m_szSourceID));
			//memset(m_szFileName,0,sizeof(m_szFileName));
			memset(mServCatId,0,sizeof(mServCatId));

			//strcpy(m_szFileName,mVfile[1].c_str());
			strcpy(m_szSourceID,it->first.c_str());
			strcpy(mServCatId,it->second.serverCatID);

			//ret = dealFile();
			//if(petri_status == DB_STATUS_ONLINE)
			//{
			//	conn.close();
			//}

			//theJSLog<<"######## end deal file ########\n"<<endi;
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
		strcat(inputFilePath,mConfParam.szInPath); 

		memset(filter,0,sizeof(filter));
		memset(szFiletypeIn,0,sizeof(szFiletypeIn));  
		strcpy(filter,it->second.filterRule);		  //��������
		strcpy(szFiletypeIn,it->second.szInFileFmt);  //��ǰ����Դ�������ʽ
		strcpy(m_szSourceID,it->first.c_str());
		
		memset(mServCatId,0,sizeof(mServCatId));
		strcpy(mServCatId,it->second.serverCatID);

		//�򿪻����ļ�Ŀ¼
		if(scan.openDir(inputFilePath))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"�򿪻����ļ�Ŀ¼[%s]ʧ��",inputFilePath);
			theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����
			it++;
			continue ;	
		}		
						
		//ѭ����ȡĿ¼��ɨ���ļ��У���ȡ�ļ�  ��Ϊ������ʱ�ļ������Ի�ɨ��N��
		int rett = -1 ;
		char tmp[512];
		file_num = 0;
		while(1)		
		{		
				ret=getCmd(event_sn, event_type, param1, param2, src_id);
				if(ret == 1)
				{
					if(event_type == EVT_CMD_STOP)
					{
						scan.closeDir();
						theJSLog<<"***********���յ��˳�����**********************\n"<<endw;
						prcExit();
					}
				}
				
				rtinfo.getDBSysMode(db_status);
				if(db_status != petri_status)
				{
					theJSLog<<"���ݿ�״̬�л�... "<<petri_status<<"->"<<db_status<<endw;
					int cmd_sn = 0;
					if( !putEvt(cmd_sn, EVT_RPT_DBSTATUS, 0, db_status, DSPCH_PRC_ID) )
					{
						theJSLog<<"�������ݿ����״̬ʧ�ܣ�\n"<<endw;
						break;
					}
					petri_status = db_status;
					petri_status_tmp = db_status;
				}

				if(file_num == mConfParam.source_file_num)
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
				
				strcpy(m_szFileName,p+1);
/*
				if(mConfParam.child_num <= 1)	//�����̴���
				{
					theJSLog<<"######## start deal file "<<fileName<<" ########"<<endi;
					
					//����ͬ����Ϣ
					memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));
					sprintf(mdrDeal.m_SerialString,"%s;%s;%d",m_szSourceID,m_szFileName,petri_status);
					ret = mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
					if(ret)
					{
						theJSLog<<"ͬ��ʧ��...."<<endw;
						continue ;
					}
				
					ret = dealFile();
					//if(petri_status == DB_STATUS_ONLINE)
					//{
					//	conn.close();
					//}

					theJSLog<<"######## end deal file ########\n"<<endi;
				} */
				//else						//����̴��� �����̱����ļ�
				//{
					mVfile.push_back(m_szFileName);

					if(mVfile.size() >= mConfParam.child_num) break;
					
				//}
		}

		scan.closeDir();

		//++it;		// 2014-07-28 �����Ƶ�������ɺ�,����������
	}
	
	//�����˶���̱�־ �����ļ�������
	//if((mVfile.size()> 0) && (mConfParam.child_num > 1))		//2014-07-07 /////////////////////////////////////////////
	if(mVfile.size() > 0)
	{
		theJSLog<<"the bactch file begin..."<<"file num:"<<mVfile.size()<<endi;
		
		if(mdrDeal.mdrParam.drStatus == 0)
		{
			memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));
			sprintf(mdrDeal.m_SerialString,"%s;%d;",m_szSourceID,petri_status);
		}

		int pid = 0,chld_prc_idx=0;		
		for(chld_prc_idx=0;chld_prc_idx<mVfile.size();chld_prc_idx++)
		{	
			if(mdrDeal.mdrParam.drStatus == 0)
			{
				sprintf(mdrDeal.m_SerialString,"%s%s;",mdrDeal.m_SerialString,mVfile[chld_prc_idx]);
			}

			strcpy(m_szFileName,mVfile[chld_prc_idx].c_str());
			sprintf(fileName,"%s%s",inputFilePath,m_szFileName);

			pid = fork();	//fork �ӽ���ȥ�����ļ�,������ɺ��˳�
			if(pid == 0)
			{
				dealFile2(chld_prc_idx);
				prcExit();
			}
			else if(pid == -1)
			{
				theJSLog << "�����ӽ���ʧ��!" << endw;
			}
			else
			{
				g_stChldPrcInfo[chld_prc_idx].pid=pid;
				g_stChldPrcInfo[chld_prc_idx].state=PRCST_STARTING;
				theJSLog<<"�����ӽ���(�ӽ������="<<chld_prc_idx+1<<")..."<<endi;
				
				while(true)
				{
					ret=getChldEvt(event_sn, event_type, param1, param2, src_id);
					if(ret<=0)
					{
						if( ret==0 && g_stChldPrcInfo[chld_prc_idx].state==PRCST_INACTV )
						{
							g_stChldPrcInfo[chld_prc_idx].pid=0;
							break;
						}
						else
						{
							prcExit();
							return;
						}
					}
					if( param1==m_lPrc_ID+chld_prc_idx+1 && pid==param2 && event_type==EVT_RESP_START )
					{
						if(param2>0)
						{
							cerr<<"�ɹ���\n";
							g_stChldPrcInfo[chld_prc_idx].pid=pid;
							g_stChldPrcInfo[chld_prc_idx].state=PRCST_IDLE;
							g_iChldPrcNum++;
						}
						else
						{
							g_stChldPrcInfo[chld_prc_idx].pid=0;
							cerr<<"ʧ�ܣ�\n";
						}
						break;
					}
				}
			}
		}

		if(mdrDeal.mdrParam.drStatus == 0)
		{
			ret = mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
			if(ret)
			{
				theJSLog<<"ͬ��ʧ��...."<<endw;
				mVfile.clear();

				continue ;
			}
		}

		//�����̵ȴ��ӽ��̴������
		//int event_sn, event_type;
		//long param1, param2, src_id;
		int result,over_num=0;		
		vector<int> mVstatus(mVfile.size());
		vector<int> mVrecordNum(mVfile.size());

		while(1)
		{
			result=getChldEvt(event_sn, event_type, param1, param2, src_id);  //param1 �ӽ���ʵ��ID param2 ������
			//if(result<=0) return -11;			//����(ret<0)���ӽ����˳�(ret==0) �� �յ�kill�ź�(ret==0)

			if(event_type==EVT_RPT_TASKEND)
			{
				theJSLog<<"�������յ��ӽ���"<<src_id-m_lPrc_ID<<"���������Ϣ:"<<src_id<<" "<<m_lPrc_ID<<endi;

				mVstatus[src_id-m_lPrc_ID-1]=param2;		//����״̬
				mVrecordNum[src_id-m_lPrc_ID-1]=param1;	//�����¼����

				g_stChldPrcInfo[src_id-m_lPrc_ID-1].state=PRCST_IDLE;
				over_num++;

				//theJSLog<<"g_iChldPrcNum="<<g_iChldPrcNum<<" over_num="<<over_num<<endi;
			}

			if(over_num >= g_iChldPrcNum)	//��ʾȫ���ӽ��̴������
			{
				break ;
			}
		}
				
		memset(mdrDeal.m_AuditMsg,0,sizeof(mdrDeal.m_AuditMsg));
		strcpy(mdrDeal.m_AuditMsg,m_szSourceID);
		for(int i=0;i<mVfile.size();i++)
		{
			sprintf(mdrDeal.m_AuditMsg,"%s:%s|%d|%d",mdrDeal.m_AuditMsg,mVfile[i],mVstatus[i],mVrecordNum[i]);
		}
		
		if((mdrDeal.mdrParam.drStatus == 0) || (mdrDeal.mdrParam.drStatus == 1))
		{
			ret = mdrDeal.IsAuditSuccess(mdrDeal.m_AuditMsg);
		}
		else
		{
			ret = 0;
		}
		
		for(int i=0;i<mVfile.size();i++)
		{
			if(!putChldEvt(0, EVT_REQ_STOP, m_lPrc_ID, ret, m_lPrc_ID+i+1))
			{
				theJSLog<<"�����̷� EVT_REQ_STOP "<<"���ӽ���"<<m_lPrc_ID+i+1<<"ʧ��"<<endi;
			}

			g_stChldPrcInfo[i].state=PRCST_STOPPING;
		}

		while(g_iChldPrcNum)			//�ȴ��ӽ���ȫ���˳�
		{
			sleep(1);
			//for(int i=0;i<mVfile.size();i++)
			//{
			//	if( g_stChldPrcInfo[i].state==PRCST_INACTV ) break;
			//}
		}

		mVfile.clear();

		theJSLog<<"the bactch file end ..."<<endi;
	}

	//conn.close();
	
	if(mdrDeal.mdrParam.drStatus != 1)		//��ϵͳ���߷�����ϵͳ
	{
		++it;
	}

	sleep(5);
  }//while(1)

}

//2014-07-07 �������� 
int C_Indb::dealFile2(int chld_prc_idx)
{	
	int ret = 0;
	char szBuff[1024];
	
	int event_sn, event_type;
	long param1, param2, src_id;

	m_lPrc_ID=m_lPrc_ID+chld_prc_idx+1;
	m_bReg=false;
	
	do	//����ӽ�����Ϣ����
	{
		ret=getChldEvt(event_sn, event_type, param1, param2, src_id, false);
		if(ret<0)
		{
			theJSLog<<"�ӽ���"<<chld_prc_idx+1<<"����ӽ�����Ϣ����ʧ��!\n"<<endw;
			return -1;
		}
	}while(ret);

	if( !putChldEvt(0, EVT_RESP_START, m_lPrc_ID, getpid(), m_lPrc_ID-chld_prc_idx-1) ) 
	{
		theJSLog<<"�ӽ���"<<chld_prc_idx+1<<" ���� EVT_RESP_START ��Ϣʧ��"<<endw;
		return -1;
	}
	
	theJSLog<<chld_prc_idx+1<<":�ļ�["<<m_szFileName<<"]���"<<endi;

	map< string,int>::const_iterator iter =  mapTabConf.find(m_szSourceID);
	if( iter == mapTabConf.end())
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"indb() ���ݿ���û������Դ%s��������Ϣ",m_szSourceID);
		theJSLog.writeLog(LOG_CODE_SOURCE_NOT_FIND,erro_msg);
		
		return -1;
	}
	
	record_num = 0;
	Statement stmt;
	char state = 'Y';
	CF_CNewError_Table* tab;
	
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

		in.getline(szBuff,sizeof(szBuff));
		//2013-10-01  ������ȡfile_id ��file_time
		memset(file_time,0,sizeof(file_time));
		strncpy(file_time,m_szFileName+it->second.file_begin,it->second.file_length);
		file_time[8] = '\0';
		char fileid[10],rate_cycle[6+1];
		memset(fileid,0,sizeof(fileid));
		outrcd.Set_record(szBuff);
		outrcd.Get_Field(FILE_ID,fileid);
		file_id = atol(fileid);
		
		//cout<<"m_szFileName="<<m_szFileName<<"  file_begin="<<it->second.file_begin<<" file_length="<<it->second.file_length
		//	<<" file_time="<<file_time<<endl;

		memset(rate_cycle,0,sizeof(rate_cycle));
		outrcd.Get_Field(RATE_CYCLE,rate_cycle);
		rate_cycle[6] = '\0';

		tab = &TabConf[iter->second];
		
		if(petri_status == DB_STATUS_ONLINE)
		{
			if(!(dbConnect(conn)))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"run() �������ݿ�ʧ�� connect error");
				theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
				
				tab = NULL;
				return -1;
			}
			
			tab->setBegin(conn);
			tab->setConfig(file_id,rate_cycle);
			
			do 			  
			{
				tab->insertData(szBuff);
				record_num++;
				memset(szBuff,0,sizeof(szBuff));		
			}while(in.getline(szBuff,sizeof(szBuff))); 	
			
		}
		else
		{
			setSQLFileName(m_szFileName);
			tab->setRWFlag(false);
			tab->setConfig(file_id,rate_cycle);
			
			mInfile.Open(fileName);
			tab->setSQLconf(getSQLFileName(),mInfile.Get_recCount());
			mInfile.Close();

			do	   
			{
				tab->writeSQL(szBuff);
				record_num++;
				memset(szBuff,0,sizeof(szBuff));		
			}while(in.getline(szBuff,sizeof(szBuff)));	
		}
							
		in.close();	
	
	}catch (jsexcp::CException e)
	{
		tab->RollBack();
		state = 'E';
		ret = -1 ;
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"[%s]indb() error line=(%d) :%s ",m_szFileName,record_num,e.GetErrMessage());
		theJSLog.writeLog(0,erro_msg);
	}
	
	char tmp[512];

	//����Ϣ��������  ����״̬,��¼��
	theJSLog<<"�ӽ���"<<chld_prc_idx+1<<"�������,������Ϣ EVT_RPT_TASKEND "<<endi;

	if( !putChldEvt(0, EVT_RPT_TASKEND, record_num, ret, m_lPrc_ID-chld_prc_idx-1) )
	{
		theJSLog<<"�ӽ���"<<chld_prc_idx+1<<" ���� EVT_RPT_TASKEND ��Ϣʧ��"<<endw;
		return -1 ;
	}
	
	//ret = 0 ;				//�ȴ��������ٲý�� �ٲ�ʧ��Ҫ�ع�
	while(1)
	{
		int ret=getChldEvt(event_sn, event_type, param1, param2, src_id);
		if(ret<0)
		{
			cerr<<"�ӽ���"<<chld_prc_idx+1<<"��ȡ�������¼�ʧ�ܣ�"<<endl;
			perror("ʧ��ԭ��");
			break;
		}
		else if(event_type==EVT_REQ_STOP)  break;   //������Ҫ���ӽ����˳�
	}
	
	ret = param2;
	////////////////////////////////////////////////////////////////////////////////////
	if(ret)
	{
		tab->RollBack();
		tab = NULL;

		if(petri_status == DB_STATUS_ONLINE)
		{
			conn.close();
		}

		if(ret != 3)								//2013-11-07 �ٲó�ʱ���ƶ��ļ�
		{
			memset(tmp,0,sizeof(tmp));				//�ļ��Ƶ�����Ŀ¼
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

		return -1;
	 }

   try
   {   
		theJSLog<<chld_prc_idx+1<<":���������ȱ�..."<<endi;
		getCurTime(currTime); 
		memset(sql,0,sizeof(sql));	
		sprintf(sql,"insert into %s (source_id,serv_cat_id,filename,deal_flag,dealtime,mainflow_count,file_id,file_time) values('%s','%s','%s','%c','%s',%d,%ld,'%s')",mConfParam.szSchCtlTabname,m_szSourceID,mServCatId,m_szFileName,state,currTime,record_num,file_id,file_time);
		cout<<"sql ="<<sql<<endl;

		if(petri_status == DB_STATUS_ONLINE)
		{			
			tab->Commit();
			stmt = conn.createStatement();
			stmt.setSQLString(sql);
			stmt.execute();
			stmt.close();
			conn.close();		
		}
		else
		{
			tab->Commit();

			writeSQL(sql);
			commitSQLFile();

			//tab->RollBack();
		}	

	}catch(SQLException e)
	{
		stmt.rollback();
		conn.close();
		state = 'E';
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"[%s]indb() sql error %s (%s)",m_szFileName,e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);
    }
		
		tab = NULL;

		if(state == 'E')		//���ʧ��,�ļ��Ƶ�����Ŀ¼
		{
			theJSLog<<"����쳣�ļ�"<<m_szFileName<<"�Ƶ�ʧ��Ŀ¼"<<endi;

			memset(tmp,0,sizeof(tmp));								
			strcpy(tmp,it->second.szSourcePath);
			strcat(tmp,mConfParam.szErroPath);
			strcat(tmp,m_szFileName);
			if(rename(fileName,tmp))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"�ƶ��ļ�[%s]������Ŀ¼ʧ��: %s",fileName,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
			}
			
			return -1;
		}
		
		//���ļ�����
		if(mConfParam.bak_flag == 'Y')
		{		
			//�ж��Ƿ���Ҫ����										
			char bak_dir[512];
			memset(bak_dir,0,sizeof(bak_dir));
			strcpy(bak_dir,it->second.szSourcePath);
			strcat(bak_dir,mConfParam.szBakPath);

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
	
		theJSLog<<chld_prc_idx+1<<":�ļ����ɹ�..."<<endi;

	return ret ;

}




//���ӽ���ͨ����Ϣ��������ȡ��Ϣ�����ӽ��̾��ɵ���
int C_Indb::getChldEvt(int &event_sn, int &event_type, long &param1, long &param2, long &src_id, bool sync)
{
	if(m_iChldPrcMsgID==-1) return -1;
	EVENT_MSG event_msg;
	while(true)
	{
		if( msgrcv(m_iChldPrcMsgID, &event_msg, sizeof(EVENT), m_lPrc_ID, sync?0:IPC_NOWAIT)<0 )
		{
			if(errno==ENOMSG) return 0;
			else if(errno==EINTR)
			{
				if( getpid()==m_iMainPid ) return 0;
				else continue;
			}
			else return -1;
		}
		else break;
	}
	event_sn=event_msg.event.event_sn;
	event_type=event_msg.event.event_type;
	param1=event_msg.event.param1;
	param2=event_msg.event.param2;
	src_id=event_msg.event.src_id;
	return 1;
}

//���ӽ���ͨ����Ϣ�����﷢����Ϣ�����ӽ��̾��ɵ���
bool C_Indb::putChldEvt(int event_sn, int event_type, long param1, long param2, long dest_id)
{
	if(m_iChldPrcMsgID==-1) return false;
	EVENT_MSG event_msg;
	event_msg.msgtype=dest_id;
	event_msg.event.event_sn=event_sn;
	event_msg.event.event_type=event_type;
	event_msg.event.param1=param1;
	event_msg.event.param2=param2;
	event_msg.event.src_id=m_lPrc_ID;
	time(&(event_msg.event.event_time));
	if( msgsnd(m_iChldPrcMsgID, &event_msg, sizeof(EVENT), IPC_NOWAIT)<0 ) return false;
	return true;
}

//2013-11-02 �����˳�����
void C_Indb::prcExit()
{
	int ret = 0;
	
	if(getpid() == m_iMainPid)
	{
		mdrDeal.dr_ReleaseDR();   //�ӽ���ʱ�����˳�
	}
	
	if(TabConf != NULL)
	{
		delete[] TabConf;
	}

	PS_Process::prcExit();
}



int main(int argc,char** argv)
{
	cout<<"********************************************** "<<endl;
	cout<<"*    China Telecom. Telephone Network          "<<endl;
	cout<<"*    InterNational Account Settle System       "<<endl;
	cout<<"*                                              "<<endl;
	cout<<"*           jsindb		                      "<<endl;
	cout<<"*           sys.GJZW.Version 1.0	              "<<endl;
	cout<<"*     created time :      2013-09-01 by  hed	  "<<endl;
	cout<<"*     last updaye time :  2014-07-09 by  hed	  "<<endl;
	cout<<"********************************************** "<<endl;


	C_Indb fm ;

	if(!fm.init(argc,argv)) return false;
	
	//while(1)
	//{
		//theJSLog.reSetLog();
		fm.dealType();
		//sleep(3);
	//}

   return 0;
}








