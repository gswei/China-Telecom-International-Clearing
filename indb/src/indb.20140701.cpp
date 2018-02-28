/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-06-19
File:			 C_Indb.cpp
Description:	 读写话单块处理
History:
<table>
revision	author            date                description
--------    ------            ----                -----------
v1.0		hed          2013-09-01

</table>
**************************************************************************/

#include<dirent.h>			//_chdir() _getcwd() 读取文件目录等，浏览文件夹信息
#include<sys/types.h>
#include<sys/stat.h>		//stat()函数，查询文件信息
#include<unistd.h>			//读取当前程序运行目录
#include<fstream>
#include <sstream>
#include "indb.h"

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

//模块初始化动作
bool C_Indb::init(int argc,char** argv)
{ 
    if(!PS_Process::init(argc,argv))
    {
      return false;
    }
	
	if(!(dbConnect(conn)))
	{
		cout<<"初始化数据库 connect error."<<endl;
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
			cout<<"请在tp_billing_line表中配置流水线["<<mConfParam.iflowID<<"]的数据源组"<<endl;
			return false ;
		}

		sql = "select ext_param from TP_PROCESS where billing_line_id = :1 and module_id = :2";
		stmt.setSQLString(sql);
		stmt<<mConfParam.iflowID<<mConfParam.iModuleId;
		stmt.execute();
		if(!(stmt>>mConfParam.szService))
		{
			cout<<"请在tp_process表字段ext_param中配置模块["<<mConfParam.iModuleId<<"]service"<<endl;
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
			theJSLog<<"C_SOURCE_GROUP_DEFINE,C_SERVICE_INTERFACE,C_SERVICE_INTERFACE关联查询失败:"<<sql<<endw;
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
				theJSLog<<"请在表c_process_env中配置加载模块的错误路径,INDB_FILE_ERR_DIR"<<endw;
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
					theJSLog<<"请在表c_process_env中配置加载模块的备份目录,INDB_FILE_BAK_DIR"<<endw;
					return false;
			}
			completeDir(mConfParam.szBakPath);
			theJSLog<<"szBakPath="<<mConfParam.szBakPath<<endi;
		}	

		stmt.close();
		
	   }catch(SQLException  e)
		{
			theJSLog<<"初始化时数据库查询异常:"<<e.what()<<endw;
			return false ;
		}
	
	char sParamName[256];
	CString sKeyVal;
	memset(sParamName,0,sizeof(sParamName));
	//2013-08-16新增可以配置每次扫描数据源目录下面指定个数文件后调到下个数据源
	sprintf(sParamName, "business.source.file.%d.num",getFlowID());
	if(param_cfg.bGetMem(sParamName, sKeyVal))
	{
		mConfParam.source_file_num = sKeyVal.toInteger();
		theJSLog<<sParamName<<"="<<mConfParam.source_file_num<<endi;
	}
	else
	{	
		theJSLog<<"请在配置文件中配置流水线["<<mConfParam.iflowID<<"]中数据源每次扫描文件的个数,参数名:"<<sParamName<<endw;
		return false ;
	}	

	bool bb = initializeLog(argc,argv,false);  //是否调试模式
	if(!bb)
	{
			return false;
	}
	
	bool flag = true;
	for(int i=1;i<argc;i++)
	{
		if(strcmp(argv[i],"-k") == 0)
		{
			theJSLog<<"模块["<<module_name<<"]不进行容灾..."<<endi;
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

	//获取petri网状态,当系统为只读态时,数据库更新操作语句写文件
	if(!(rtinfo.connect()))
	{
		theJSLog<<"连接运行时内存区失败"<<endw;
		return false;
	}
	rtinfo.getDBSysMode(petri_status);
	petri_status_tmp = petri_status;
	theJSLog<<"current petri_status="<<petri_status<<endi;

	//theJSLog.setLog(szLogPath,szLogLevel,m_szService , m_szSrcGrpID, 001);
	//theJSLog<<"  相对输入路径:"<<input_path<<"  错误路径:"<<erro_path<<"	日志路径:"<<szLogPath<<" 日志级别:"
			//<<szLogLevel<<"	每个数据源扫描文件个数:"<<source_file_num<<endi;
	
	//if(bak_flag == 'Y')
	//{
	//	theJSLog<<"文件备份路径:"<<bak_path<<endi;
	//}

    theJSLog<<"加载数据源配置信息LoadSourceCfg..."<<endi;

	if(LoadSourceCfg() == -1) return false ;  //加载数据源配置信息
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
					//sprintf(erro_msg,"数据源[%s]的输入文件路径[%s]不存在",iter->first,input_dir);
					//theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错

					//return false ;
					theJSLog<<"数据源【"<<iter->first<<"】的输入文件路径: "<<input_dir<<"不存在，自行创建"<<endw;
					rett = mkdir(input_dir,0755);
					if(rett == -1)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"数据源[%s]的话单文件输入路径[%s]不存在，自行创建失败",iter->first,input_dir);
						theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
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
						theJSLog<<"数据源【"<<iter->first<<"】的备份文件路径: "<<bak_dir<<"不存在，自行创建"<<endw;
						rett = mkdir(bak_dir,0755);
						if(rett == -1)
						{
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"数据源[%s]的备份文件文件路径[%s]不存在，自行创建失败",iter->first,bak_dir);
							theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错

							return false;
						}
					}else closedir(dirptr);
			}
			
			memset(erro_dir,0,sizeof(erro_dir));
			strcpy(erro_dir,iter->second.szSourcePath);
			strcat(erro_dir,mConfParam.szErroPath);
			if((dirptr=opendir(erro_dir)) == NULL)
			{
					theJSLog<<"数据源【"<<iter->first<<"】的错误文件路径: "<<erro_dir<<"不存在，自行创建"<<endw;
					rett = mkdir(erro_dir,0755);
					if(rett == -1)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"数据源[%s]的错误文件文件路径[%s]不存在，自行创建失败",iter->first,erro_dir);
						theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错

						return false;
					}
			}else closedir(dirptr);		

	}
	
   it = m_SourceCfg.begin();   //初始化第一个数据源
	
   theJSLog<<"初始化完毕..."<<endi;

   return true ;
}

//加载数据源配置信息
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
				
				if(getmapAdjCycleBusi(strSourceId) == -1)  return -1;

				m_SourceCfg[strSourceId]=SourceCfg;
				theJSLog<<"szSourceId="<<SourceCfg.szSourceId<<" szSourcePath="<<SourceCfg.szSourcePath<<" serverCatID="<<SourceCfg.serverCatID
						<<" filterRule="<<SourceCfg.filterRule<<"  filetime_begin="<<SourceCfg.file_length<<"  filetime_length="<<SourceCfg.file_length<<endi;
		     }
		}

		
		//加载每个数据源对应的统计入库表
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
				sprintf(erro_msg,"表source_env中数据源组[%s]配置的数据源[%s]没有配置入库的表项 INS_TABLE_CONFIGID",mConfParam.szSrcGrpID,iter->first);
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
		sprintf(erro_msg,"LoadSourceCfg数据库出错：%s (%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		//throw jsexcp::CException(0, "LoadSourceCfg加载数据库出错：", __FILE__, __LINE__);
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

/******根据数据源获取过滤规则 0没有查到规则，1查到规则了*******************考虑放在加载数据源**/
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
				sprintf(erro_msg,"数据源[%s]没有配置过滤规则或者文件名时间截取规则",source);
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
			sprintf(erro_msg,"数据源[%s]文件名时间截取规则配置规则错误:%s  [如3,8]",source,file_time);
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
			sprintf(erro_msg,"数据源[%s]文件名时间截取规则配置规则错误:%s  [如3,8]",source,file_time);
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
			return -1;
		}

		index--;

		stmt.close();

	}
	catch(SQLException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"getSourceFilter 数据库查询异常: %s (%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);
		return -1 ;
	}
	catch(jsexcp::CException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"getSourceFilter 字段转化出错：%s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);
		return -1;
	}
	
	return 0;
}

//2014-07-01 获取该是否可调整文件的账期再次录入的业务
int C_Indb::getmapAdjCycleBusi(string source)
{
	string sql;
	try
	{	
		Statement stmt = conn.createStatement();
	    sql = "select MONTH_ADJ from C_CYLCE_ADJ_DEFINE where source_id = :1 and vail_flag='Y' ";		
		stmt.setSQLString(sql);
		stmt << source;
		if(!stmt.execute())
		{
			stmt.close();
			return 0;	
		}
		
		C_ADJ_CYCEL_DEF vfmt;
		stmt>>vfmt.monthnum;
		
		mapAdjCycleBusi.insert(map< string,C_ADJ_CYCEL_DEF>::value_type(source,vfmt));

		stmt.close();

	}catch(SQLException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"getmapAdjCycleBusi 数据库查询异常: %s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);

		return -1 ;
	}
	
	return 0; 
}

//循环扫描各个数据源
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
		theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "应用程序收到退出信号");
		prcExit();
		return;
	}
	
	ret=getCmd(event_sn, event_type, param1, param2, src_id);
	if(ret == 1)
	{
		if(event_type == EVT_CMD_STOP)
		{
			theJSLog<<"***********接收到退出命令**********************\n"<<endw;
			prcExit();
		}
	}

	theJSLog.reSetLog();
	
	if(mdrDeal.mdrParam.drStatus == 1 )  //备系统
	{	
			rtinfo.getDBSysMode(db_status);
			if(db_status != petri_status_tmp)
			{
				theJSLog<<"数据库状态切换... "<<petri_status_tmp<<"->"<<db_status<<endw;
				int cmd_sn = 0;
				if( !putEvt(cmd_sn, EVT_RPT_DBSTATUS, 0, db_status, DSPCH_PRC_ID) )
				{
					theJSLog<<"报告数据库更换状态失败！\n"<<endw;
					continue ;
				}
				petri_status_tmp = db_status;
			}

			//检查trigger触发文件是否存在
			if(!mdrDeal.CheckTriggerFile(m_triggerFile))
			{
				sleep(1);
				continue ;
			}

			//获取同步变量
			memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));
			ret = mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
			if(ret)
			{
				theJSLog<<"同步失败..."<<endw;
				continue ;
			}
	
			//获取同步变量
			vector<string> data;		
			splitString(mdrDeal.m_SerialString,";",data,false,false);  //发送的字符串数据源ID,文件名,sqlFile文件名
			
			it = m_SourceCfg.find(data[0]);
			if(it == m_SourceCfg.end())			//考虑是否仲裁
			{
				//dr_AbortIDX();
				mdrDeal.dr_abort();

				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"没有找到该数据源信息[%s]",data[0]);		//环境变量未设置
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);		  
				continue ;
			}
			
			if(petri_status != atoi(data[2].c_str()))
			{
				theJSLog<<"主系统的数据库状态发生了切换..."<<endw;
			}
			petri_status = atoi(data[2].c_str());		//备系统的状态根据主系统来定

			//读文件加载文件到私有内存,
			memset(fileName,0,sizeof(fileName));
			memset(m_szFileName,0,sizeof(m_szFileName));
			memset(dir,0,sizeof(dir));
			
			strcpy(dir,it->second.szSourcePath);  //数据源主路径
			strcpy(inputFilePath,dir);
			strcat(inputFilePath,mConfParam.szInPath);

			strcpy(fileName,inputFilePath);
			strcat(fileName,data[1].c_str());

			//int dr_GetAuditMode()1表示同步，2表示跟随, 其它为失败，-1是配置错误，-2配置文件读取出现问题
			//int iStatus = dr_GetAuditMode(module_name);
			int iStatus = mdrDeal.mdrParam.aduitMode;

			if(iStatus == 1)		//同步模式,	主系统等待指定时间
			{	
				bool flag = false;
				int times = 1;
				while(times < 31)
				{
					if(access(fileName,F_OK|R_OK))
					{
						theJSLog<<"查找了"<<times<<"次文件"<<endi;
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
					sprintf(erro_msg,"主系统传过来的文件[%s]不存在",fileName);
					theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);		
					continue ;
				}		
			}
			else if(iStatus == 2)		//跟随模式,备系统
			{
				while(1)
				{
					//设置中断
					if(gbExitSig)
					{
						//dr_AbortIDX();
						mdrDeal.dr_abort();

						theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "应用程序收到退出信号");
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
		strcpy(dir,it->second.szSourcePath);  //数据源主路径		
		strcpy(inputFilePath,dir);
		strcat(inputFilePath,mConfParam.szInPath); 

		memset(filter,0,sizeof(filter));
		memset(szFiletypeIn,0,sizeof(szFiletypeIn));  
		strcpy(filter,it->second.filterRule);		  //过滤条件
		strcpy(szFiletypeIn,it->second.szInFileFmt);  //当前数据源的输入格式
		strcpy(m_szSourceID,it->first.c_str());
		
		memset(mServCatId,0,sizeof(mServCatId));
		strcpy(mServCatId,it->second.serverCatID);

		//打开话单文件目录
		if(scan.openDir(inputFilePath))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"打开话单文件目录[%s]失败",inputFilePath);
			theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
			it++;
			continue ;	
		}		
						
		//循环读取目录，扫描文件夹，获取文件  因为存在临时文件，所以会扫描10次
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
						theJSLog<<"***********接收到退出命令**********************\n"<<endw;
						prcExit();
					}
				}
				
				rtinfo.getDBSysMode(db_status);
				if(db_status != petri_status)
				{
					theJSLog<<"数据库状态切换... "<<petri_status<<"->"<<db_status<<endw;
					int cmd_sn = 0;
					if( !putEvt(cmd_sn, EVT_RPT_DBSTATUS, 0, db_status, DSPCH_PRC_ID) )
					{
						theJSLog<<"报告数据库更换状态失败！\n"<<endw;
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
						break;		//表示获取文件信息失败
				}

				file_num++;				//扫描一次文件计数器+1				

				/*过滤文件*.tmp,*TMP,~* */			
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

				//发送同步信息
				memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));
				sprintf(mdrDeal.m_SerialString,"%s;%s;%d",m_szSourceID,m_szFileName,petri_status);
				ret = mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
				if(ret)
				{
					theJSLog<<"同步失败...."<<endw;
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

//将文件入库 ret = 0表示入库成功 ret= -1表示入库失败
int C_Indb::dealFile()
{	
	int ret = 0;
	char szBuff[1024];
	
	memset(mdrDeal.m_AuditMsg,0,sizeof(mdrDeal.m_AuditMsg));	
	theJSLog<<"文件["<<m_szFileName<<"]入库"<<endi;

	map< string,int>::const_iterator iter =  mapTabConf.find(m_szSourceID);
	if( iter == mapTabConf.end())
	{
		//dr_AbortIDX();
		mdrDeal.dr_abort();

		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"indb() 数据库中没有数据源%s的配置信息",m_szSourceID);
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
			sprintf(erro_msg,"dealFile() 文件[%s]打开出错",fileName);
			theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败
			
			return -1;
		}

		in.getline(szBuff,sizeof(szBuff));
		//2013-10-01  新增获取file_id 和file_time
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
		
		//2014-07-01   判断该业务是否可调整,且是否需要改账期
		if(mapAdjCycleBusi.find(m_szSourceID) != mapAdjCycleBusi.end())
		{
			if(!(dbConnect(conn)))
			{
				//dr_AbortIDX();
				mdrDeal.dr_abort();

				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"run() 连接数据库失败 connect error");
				theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
				
				tab = NULL;
				return -1;
			}
			
			stmt = conn.createStatement();
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select min(rate_cycle) from c_rate_cycle a where a.source_id='%s' and cycle_flag='N'",m_szSourceID);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>rate_cycle;
			rate_cycle[6] = '\0';

			stmt.close();
			conn.close();	
		}

		tab = &TabConf[iter->second];
		
		if(petri_status == DB_STATUS_ONLINE)
		{
			if(!(dbConnect(conn)))
			{
				//dr_AbortIDX();
				mdrDeal.dr_abort();

				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"run() 连接数据库失败 connect error");
				theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
				
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
		
		//**************做仲裁,数据源,文件名,单个话单块能存放的记录条数,记录条数
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

			if(ret != 3)								//2013-11-07 仲裁超时不移动文件
			{
				memset(tmp,0,sizeof(tmp));				//文件移到错误目录
				strcpy(tmp,it->second.szSourcePath);
				strcat(tmp,mConfParam.szErroPath);
				theJSLog<<"移动文件到失败目录:"<<tmp<<endi;
				strcat(tmp,m_szFileName);
				if(rename(fileName,tmp))
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"移动文件[%s]到错误目录失败: %s",fileName,strerror(errno));
					theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
				}
			}

			return -1;
		 }

   try
   {   
		theJSLog<<"更新入库调度表..."<<endi;
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

		if(state == 'E')		//入库失败,文件移到错误目录
		{
			theJSLog<<"入库异常文件"<<m_szFileName<<"移到失败目录"<<endi;

			memset(tmp,0,sizeof(tmp));								
			strcpy(tmp,it->second.szSourcePath);
			strcat(tmp,mConfParam.szErroPath);
			strcat(tmp,m_szFileName);
			if(rename(fileName,tmp))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"移动文件[%s]到错误目录失败: %s",fileName,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
			}
			
			return -1;
		}
		
		//将文件备份
		if(mConfParam.bak_flag == 'Y')
		{		
			//判断是否需要备份										
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
				theJSLog<<"备份文件["<<m_szFileName<<"]到目录"<<bak_dir<<endi;
				strcat(bak_dir,m_szFileName);
				if(rename(fileName,bak_dir))
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"文件[%s]移动[%s]失败: %s",fileName,bak_dir,strerror(errno));
					theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);		//移动文件失败
				}
			}
			else
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"备份路径[%s]不存在，且无法创建",bak_dir);
				theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg);		//打开目录出错
			}
		}	
		else 
		{
			if(remove(fileName))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"文件[%s]删除失败: %s",fileName,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);		//删除文件失败
			}
		}

	theJSLog<<"文件入库成功..."<<endi;
	
	return ret;
}

/*
//容灾初始化
bool C_Indb::drInit()
{
		//传入模块名和实例ID
		char tmp[32];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%ld",getPrcID());

		theJSLog << "初始化容灾平台,模块名:"<< module_name<<" 实例名:"<<tmp<<endi;

		int ret = dr_InitPlatform(module_name,tmp);
		if(ret != 0)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"容灾平台初始化失败,返回值=%d",ret);
			theJSLog.writeLog(LOG_CODE_DR_INIT_ERR,erro_msg);

			return false;
		}
		else
		{
			theJSLog<<"dr_InitPlatform ok."<<endi;
		}

		mdrParam.m_enable = true;

		mdrParam.drStatus = _dr_GetSystemState();	//获取主备系统状态
		if(mdrParam.drStatus < 0)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"获取容灾平台状态出错,返回值=%d",mdrParam.drStatus);
			theJSLog.writeLog(LOG_CODE_DR_GETSTATE_ERR,erro_msg);

			return false;
		}
		
		if(mdrParam.drStatus == 0)		theJSLog<<"当前系统配置为主系统"<<endi;
		else if(mdrParam.drStatus == 1)	theJSLog<<"当前系统配置为备系统"<<endi;
		else if(mdrParam.drStatus == 2)	theJSLog<<"当前系统配置非容灾系统"<<endi;

		return true;
}

//主系统发送同步变量,备系统获取同步变量
int C_Indb::drVarGetSet(char* serialString)
{
		int ret  = 0;
		char tmpVar[5000] = {0};
		
		if(!mdrParam.m_enable)
		{
			return ret;
		}

		//检查容灾平台的切换锁
		ret = dr_CheckSwitchLock();   
		if(ret != 0)  
		{  
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"检查容灾切换锁失败,返回值=%d",ret);
			theJSLog.writeLog(LOG_CODE_DR_CHECK_LOCK_ERR,erro_msg);

			return -1;  
		} 
		//初始化index  
		ret = dr_InitIDX();  
		if(ret != 0)  
		{  
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"初始化index失败,返回值=%d",ret);
			theJSLog.writeLog(LOG_CODE_DR_INIT_IDX_ERR,erro_msg);

			//dr_AbortIDX();
			return -1;  
		}
		
		//主系统传递文件所在路径和文件名 只有容灾平台可以感知,备系统无法识别
		if(drStatus != 1)
		{
			snprintf(tmpVar, sizeof(tmpVar), "%s%s", it->second.szSourcePath,input_path);
			ret = dr_SyncIdxVar("@@CHECKPATH", tmpVar,SYNC_SINGLE);  
			if(ret != 0)
			{
				theJSLog<<"传输文件所在路径失败,文件路径["<<input_path<<"]"<<endw;
				dr_AbortIDX();
				return -1;
			}
			
			snprintf(tmpVar, sizeof(tmpVar), "%s", m_szFileName);
			ret = dr_SyncIdxVar("@@CHECKFILE", tmpVar,SYNC_SINGLE);  
			if(ret != 0)
			{
				theJSLog<<"传输文件失败,文件名["<<m_szFileName<<"]"<<endw;
				dr_AbortIDX();
				return -1;
			}

			cout<<"传输文件路径:"<<input_path<<" 文件名:"<<m_szFileName<<endl;
		}


		snprintf(tmpVar, sizeof(tmpVar), "%s", serialString);
		//主系统把要同步的index “键值对”写入容灾平台维护的index 文件中
		//备系统调用该函数的结果是，var获得和主系统一样的随机变量的值。	SYNC_SINGLE表示注册单一的随机变量
		ret = dr_SyncIdxVar("serialString", tmpVar, SYNC_SINGLE);		
		if (ret != 0)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"传序列串时失败，序列名:%s",serialString);
			theJSLog.writeLog(LOG_CODE_DR_SYSC_IDXVAR_ERR,erro_msg);

			dr_AbortIDX();
			return -1;
		}
		if(mdrParam.drStatus == 1)
		{
			//serialString = tmpVar;			//同步索引字符串,主系统是赋值，备系统是取值
			strcpy(serialString,tmpVar);
			//m_AuditMsg = tmpVar;			//要仲裁的字符串
		}

		theJSLog<<"本次的同步串serialString:"<<serialString<<endi;//for test

		// <5> 传输实例名  用于主系统注册此IDX的模块调用参数。
		//备系统的index manager检查IDX条件满足后，把使用该函数注册的随机变量作为模块的调用参数trigger相应的进程
		snprintf(tmpVar, sizeof(tmpVar), "%d", getPrcID());
		ret = dr_SyncIdxVar("@@ARG", tmpVar,SYNC_SINGLE);  
		if(ret !=0)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"传输实例名失败:%s",tmpVar);
			theJSLog.writeLog(LOG_CODE_DR_SYSC_IDXVAR_ERR,erro_msg);

			dr_AbortIDX();  
			return -1;
		}
		
		
		// <6> 预提交index  此关键字用于将平台当前内存中的随机变量写入磁盘
		ret = dr_SyncIdxVar("@@FLUSH","SUCCESS",SYNC_SINGLE);  
		if (ret != 0 )
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"预提交index失败");
			theJSLog.writeLog(LOG_CODE_DR_SYSC_IDXVAR_ERR,erro_msg);

			dr_AbortIDX();
			return -1;
		}
		
		
		// <7> 提交index  	提交Index。在index文件中设置完成标志
		ret = dr_CommitIDX();  
		if(ret != 0)  
		{  
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"提交index失败,返回值=%d",ret);
			theJSLog.writeLog(LOG_CODE_DR_COMMIT_IDX_ERR,erro_msg);

			dr_AbortIDX();  
			return -1;  
		}

		//备系统搜索目录
		//if(!m_syncDr.isMaster())thelog<<"备系统SerialString："<<m_SerialString<<endi;

		return ret;

}

//仲裁字符串
 int C_Indb::IsAuditSuccess(const char* dealresult)
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
			//theJSLog << "容灾仲裁失败,结果:" << ret <<"本端："<<dealresult<< endw;
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"容灾仲裁失败,结果:%d,本端:%s",ret,dealresult);
			theJSLog.writeLog(LOG_CODE_DR_AUDIT_ERR,erro_msg);

			dr_AbortIDX();
		}
		else if (3 == ret)
		{
			theJSLog<<"容灾仲裁超时..."<<endw;
			dr_AbortIDX();
		}
		else if(4 == ret)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"对端idx异常终止");
			theJSLog.writeLog(LOG_CODE_DR_IDX_STOP_ERR,erro_msg);

			dr_AbortIDX();
		}
		else if(1 == ret)
		{
			ret = dr_CommitSuccess();
			if(ret != 0)
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"业务全部提交失败(容灾平台),返回值=%d",ret);
				theJSLog.writeLog(LOG_CODE_DR_COMMIT_SUCCESS_ERR,erro_msg);

				//dr_AbortIDX();
			}
			theJSLog<<"ret = "<<ret<<"仲裁成功...\n仲裁内容："<<dealresult<<endi;
		}
		else
		{
			theJSLog<<"未知ret="<<ret<<"	仲裁内容："<<dealresult<<endw;
			dr_AbortIDX();
		}
	
	return ret;
 }

bool C_Indb::CheckTriggerFile()
{
	int ret = 0;
	if(access(m_triggerFile.c_str(),F_OK) != 0)	return false;

	theJSLog<< "检查到trigger文件，并删除"<< m_triggerFile <<endi;

	ret = remove(m_triggerFile.c_str());	
	if(ret) 
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"删除trigger文件[%s]失败: %s",m_triggerFile,strerror(errno));
		theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
		return false;
	}
	
	return true;
}
*/

//2013-11-02 新增退出函数
void C_Indb::prcExit()
{
	int ret = 0;

	mdrDeal.dr_ReleaseDR();
	
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
	cout<<"*     last updaye time :  2014-07-01 by  hed	  "<<endl;
	cout<<"********************************************** "<<endl;


	C_Indb fm ;

	if(!fm.init(argc,argv)) return false;
	
	//while(1)
	//{
		//theJSLog.reSetLog();
		fm.run();
		//sleep(3);
	//}

   return 0;
}








