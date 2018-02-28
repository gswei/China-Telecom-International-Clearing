/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-06-19
File:			 FileLoad.cpp
Description:	 文件加载处理
History:
<table>
revision	author            date                description
--------    ------            ----                -----------
v1.0		hed          2013-06-19

</table>
**************************************************************************/

#include<dirent.h> //_chdir() _getcwd() 读取文件目录等，浏览文件夹信息
#include<sys/types.h>
#include<sys/stat.h>  //stat()函数，查询文件信息
#include<unistd.h>     //读取当前程序运行目录
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

//模块初始化动作
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
	cin >> ch;												// 此时检查内存情况 占用54M     
	v.clear();    
	cin >> ch;												// 此时再次检查， 仍然占用54M     
	cout << "Vector 的 容量为" << v.capacity() << endl;    // 此时容量为 1048576
	cin >> ch;
	vector<string>(v).swap(v);     
	cout << "Vector 的 容量为" << v.capacity() << endl;    // 此时容量为0       // 检查内存，释放了 10M+ 即为数据内存    return 0;
	cin >> ch;
*/
	//*********2013-06-22 读取数据库的配置信息，如数据源组，错误目录 2013-03-11 add by hed*********************************************/
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
			
		sql = "select var_value from c_process_env where varname = 'LOAD_FILE_ERR_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<mConfParam.szSrcGrpID<<mConfParam.szService;
		stmt.execute();
		if(!(stmt>>mConfParam.szErroPath))
		{
				theJSLog<<"请在表c_process_env中配置加载模块的错误路径,LOAD_FILE_ERR_DIR"<<endw;
				return false;
		}	
		completeDir(mConfParam.szErroPath);
		
		theJSLog<<"szInPath="<<mConfParam.szInPath<<"  szErroPath="<<mConfParam.szErroPath<<endi;
		
		//2013-07-01 增加判断备份标志及路径
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
					theJSLog<<"请在表c_process_env中配置加载模块的备份目录,LOAD_FILE_BAK_DIR"<<endw;
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
	sprintf(sParamName, "billing_line.%d.record_num", getFlowID());		//获取话单块的最大记录数
	param_cfg.bGetMem(sParamName, sKeyVal) ;
	mConfParam.maxRecord_num=sKeyVal.toInteger();
	theJSLog<<"sParamName="<<mConfParam.maxRecord_num<<endi;

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
	//theJSLog<<"数据源组:"<<m_szSrcGrpID<<"   service:"<<m_szService<<"  相对输入路径:"<<input_path<<"  错误路径:"<<erro_path
	//		<<"	日志路径:"<<szLogPath<<" 日志级别:"<<szLogLevel<<" sql存放路径:"<<sql_path<<"	每个数据源扫描文件个数:"<<source_file_num<<endi;
	
	//if(mConfParam.bak_flag == 'Y')
	//{
	//	theJSLog<<"文件备份路径:"<<mConfParam.szBakPath<<endi;
	//}

    theJSLog<<"加载数据源配置信息LoadSourceCfg..."<<endi;
	if(LoadSourceCfg() == -1)							return false ;  //加载数据源配置信息
	
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
							sprintf(erro_msg,"数据源[%s]的备份文件路径[%s]不存在，自行创建失败",iter->first,bak_dir);
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
	
   theJSLog<<"初始化完毕...\n"<<endi;

   return true ;
}

//加载数据源配置信息
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
		sprintf(erro_msg,"LoadSourceCfg数据库出错：%s (%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		//throw jsexcp::CException(0, "LoadSourceCfg加载数据库出错：", __FILE__, __LINE__);
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

/******根据数据源获取过滤规则 0没有查到规则，1查到规则了*******************考虑放在加载数据源**/
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
			sprintf(erro_msg,"数据源[%s]没有配置过滤规则",source);	//环境变量未设置
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
			stmt.close();
			return -1;
		}
		stmt.close();
	}catch(SQLException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"getSourceFilter数据库出错：%s (%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
		return -1 ;
	}
	
	return 0;
}


//判断是否要申请话单块
int FileLoad::onBeforeTask()
{
	theJSLog.reSetLog();

	if(curr_record_num > 0) return 1 ;  //表示上次文件的记录数一个话单块没处理完

	/**************************扫描数据源，读取话单文件*************************************/
	int ret = 0;
	char dir[JS_MAX_FILEPATH_LEN],inputFilePath[JS_MAX_FILEFULLPATH_LEN],filter[50]; 
	
	short db_status = 0;
	rtinfo.getDBSysMode(db_status);

	if(mdrDeal.mdrParam.drStatus == 1)  //备系统
	{	
			if(db_status != petri_status_tmp)
			{
				theJSLog<<"数据库状态切换... "<<petri_status_tmp<<"->"<<db_status<<endw;
				int cmd_sn = 0;
				if( !putEvt(cmd_sn, EVT_RPT_DBSTATUS, 0, db_status, DSPCH_PRC_ID) )
				{
					theJSLog<<"报告数据库更换状态失败！\n"<<endw;
					return 0;
				}
				petri_status_tmp = db_status;
			}

			//检查trigger触发文件是否存在
			if(!mdrDeal.CheckTriggerFile(m_triggerFile))
			{
				sleep(1);
				return 0;
			}

			//获取同步变量
			memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));
			ret = mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
			if(ret)
			{
				theJSLog<<"同步失败..."<<endw;
				return 0;
			}
			//获取同步变量
			vector<string> data;		
			splitString(mdrDeal.m_SerialString,";",data,false,false);  //发送的字符串数据源ID,文件名,
			
			it = m_SourceCfg.find(data[0]);
			if(it == m_SourceCfg.end())			//考虑是否仲裁
			{
				//dr_AbortIDX();
				mdrDeal.dr_abort();

				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"没有找到该数据源信息[%s],dr_AbortIDX() ",data[0]);		//环境变量未设置
				theJSLog.writeLog(LOG_CODE_SOURCE_NOT_FIND,erro_msg);			
				return 0;
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
					sprintf(erro_msg,"主系统传过来的文件[%s]不存在,dr_AbortIDX()",fileName);
					theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);			
					return 0;
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
	else								//主系统,非容灾系统
	{
		if(db_status != petri_status)
		{
			theJSLog<<"数据库状态切换... "<<petri_status<<"->"<<db_status<<endw;
			int cmd_sn = 0;
			if( !putEvt(cmd_sn, EVT_RPT_DBSTATUS, 0, db_status, DSPCH_PRC_ID) )
			{
				theJSLog<<"报告数据库更换状态失败！\n"<<endw;
				return 0;
			}
			petri_status = db_status;
			petri_status_tmp = db_status;
		}

		if(file_num >= mConfParam.source_file_num)
		{
			file_num = 0;				//在同一个数据源下面扫描到N个文件后,跳到下个数据源
			sleep(5);
			++it ;
		}
		
		if(it == m_SourceCfg.end())
		{
			it = m_SourceCfg.begin();
		}

		memset(dir,0,sizeof(dir));
		strcpy(dir,it->second.szSourcePath);		 //数据源主路径		

		memset(inputFilePath,0,sizeof(inputFilePath));
		strcpy(inputFilePath,dir);
		strcat(inputFilePath,mConfParam.szInPath); 

		memset(filter,0,sizeof(filter));
		strcpy(filter,it->second.filterRule);		  //过滤条件	 					
			
		//打开话单文件目录
		if(scan.openDir(inputFilePath))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"打开话单文件目录[%s]失败",inputFilePath);
			theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错

			return -11;		//程序退出
		}		
						
		//循环读取目录，扫描文件夹，获取文件  因为存在临时文件，所以会扫描10次
		int rett = -1 ;
		char tmp[512];

		while(1)		
		{
				memset(fileName,0,sizeof(fileName));
				rett = scan.getFile(filter,fileName);  				
				if(rett == 100)
				{		
						//cout<<dir<<": "<<it->first<<"此时文件目录下面没有文件，扫描下个数据源"<<endl;
						scan.closeDir();		//2013-07-19
						file_num = 0;			//当前数据源文件个数清零
						sleep(5);
						++it ;
						return 0;
				}
				if(rett == -1)
				{	
					scan.closeDir();	//2013-07-19
					return 0 ;			//表示获取文件信息失败
				}

				file_num++;				//扫描一次文件计数器+1							

				/*过滤文件*.tmp,*TMP,~* */			
				char* p = strrchr(fileName,'/');
				memset(tmp,0,sizeof(tmp));
				strcpy(tmp,p+1);

				if(tmp[0] == '~' )	continue ;//return 0;
				if(strlen(tmp) > 2)
				{		
						int pos = strlen(tmp)-3;
						//cout<<"后缀名为："<<tmp+pos<<endl;
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
				strcpy(m_szSourceID,it->first.c_str());		  //当前数据源的source_id
				strcpy(mServCatId,it->second.serverCatID);	
				
				memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));
				sprintf(mdrDeal.m_SerialString,"%s;%s;%d",m_szSourceID,m_szFileName,petri_status);
				ret = mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
				if(ret)
				{
					theJSLog<<"同步失败...."<<endw;
					scan.closeDir();
					return 0;
				}
				
				//setSQLFileName(m_szFileName);

				break;								//找到文件退出循环，可能一个文件有占用多个话单块
		}
		
		scan.closeDir();
	}
	
	ret = dealFile();       //处理文件
	
	return ret; 

}

//处理文件，将记录加载到内存
int FileLoad::dealFile()
{	
	int ret = -1;
	char szBuff[JS_MAX_RECORD_LEN],tmp[JS_MAX_FILEFULLPATH_LEN],state;
	try
	{	
/*
			//读文件加载话单 ,默认以文件不存在时产生错误，常和in或app联合使用
			ifstream in(fileName,ios::nocreate);
			if(!in)
			{
				dr_AbortIDX();

				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"dealFile() 打开文件[%s]失败",fileName);
				theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败

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
					sprintf(erro_msg,"dealFile() 连接数据库失败,dr_AbortIDX");
					theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败

					return 0;
				}
				//else
				//{
				//	stmt = conn.createStatement();
				//}			
			}

/*
			split_num = 0;	  //初始化分割数和文件记录条数
			record_num = 0;
					
			theJSLog<<"将文件["<<m_szFileName<<"]记录加载到私有内存"<<endi;	
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
			split_num = 0;	  //初始化分割数和文件记录条数
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

			//获取文件ID,以第一条记录为准
			outrcd.Set_record(m_record[0].record);
			char fileId[10];
			memset(fileId,0,sizeof(fileId));
			outrcd.Get_Field(FILE_ID,fileId);
			file_id = atol(fileId);

			//**************做仲裁,数据源,文件名,单个话单块能存放的记录条数,记录条数
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
				
				if(ret != 3)		   //2013-11-07 仲裁超时不移动文件
				{	
					memset(tmp,0,sizeof(tmp));								//文件移到错误目录
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

				theJSLog<<"######## end deal file ########\n"<<endi;

				return 0;
			 }
	*/
			strcpy(file_name,m_szFileName);  //分割前默认与原始文件名相同

			memset(currTimeA,0,sizeof(currTimeA)); 	
			getCurTime(currTimeA);
			//memset(sql,0,sizeof(sql));
			//更新注册调度表				
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
			theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//读文件异常

			memset(tmp,0,sizeof(tmp));								//文件移到错误目录
			strcpy(tmp,it->second.szSourcePath);
			strcat(tmp,mConfParam.szErroPath);
			strcat(tmp,m_szFileName);
			if(rename(fileName,tmp))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"移动文件[%s]到错误目录[%s]失败: %s",fileName,tmp,strerror(errno));
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
			sprintf(erro_msg,"[%s]dealFile 数据库出错%s (%s)",m_szFileName,e.what(),sql);
			theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);	//发生sql执行异常
			
			memset(tmp,0,sizeof(tmp));								//文件移到错误目录
			strcpy(tmp,it->second.szSourcePath);
			strcat(tmp,mConfParam.szErroPath);
			strcat(tmp,m_szFileName);
			if(rename(fileName,tmp))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"移动文件[%s]到错误目录[%s]失败: %s",fileName,tmp,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
			}

			ret = 0;
	}
*/	
	return ret;
}

//主进程开始分配话单前的处理
int FileLoad::onTaskBegin(void *task_addr)
{
      return 1;
}

//子进程初始化
bool FileLoad::onChildInit()
{
   theJSLog<<"子进程初始化"<<endi;
   return true;
   
}

//处理成功返回话单条数(>=0)
int FileLoad::onTask(void *task_addr, int offset, int ticket_num)
{  
    //theJSLog<<"话单块地址:"<<task_addr<<endi;

    memset(task_addr,0,getBlockSize());   //初始化话单内存块,清空内存	
	PkgBlock pkg((char*)task_addr);       //话单块类初始化
	pkg.init(getTicketLength(),getBlockSize());

	pkg.setModuleId(getModuleID());			//设置模块ID
	pkg.setStatus(0);						//话单状态，0未处理，1处理
	pkg.setSourceId(it->first.c_str());		//设置数据源
	pkg.setFileHeadFlag(0);					//设置文件头标志
	pkg.setFileTailFlag(0);					//设置文件尾标志
	pkg.setFileType(it->second.szInFileFmt);//设置文件类型

	theJSLog<<"获取话单块,处理文件:"<<m_szFileName<<" 当前还有记录条数:"<<curr_record_num<<endi;

	if(curr_record_num > mConfParam.maxRecord_num)             //此时文件记录需要分割为多个话单块
	{
		 curr_record_num -= mConfParam.maxRecord_num;

		 split_num++;
		 sprintf(file_name,"%s#%04d",m_szFileName,split_num);
		
		 theJSLog<<"记录数多于话单块记录数需分割,分割序号["<<split_num<<"]"<<endi;

		 pkg.setFileName(file_name);  //设置文件名  

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
			pkg.setBlkPos("S");		//表示为话单块为文件的第一个

		 }
		 else
		 {
			pkg.setBlkPos("M");		//表示话单块为文件的中间部分
		 }

		 pkg.setStatus(0);
		 pkg.setNamalRcdNum(mConfParam.maxRecord_num);
		 pkg.setRecordNum(mConfParam.maxRecord_num); 
		
		 //删除前前面N条话单块的内容 
		 //m_record.erase(iter1,iter2);
		
		 fileOverFlag = false;

		 theJSLog<<"当前话单块处理完毕..."<<endi;
	}
	else
	{
		int begin = 0;
		int end = record_num;

		if(split_num)					//表示文件记录需要拆分为多个话单块
		{
			split_num++;
			sprintf(file_name,"%s#%04d",m_szFileName,split_num);
			pkg.setFileName(file_name);
			pkg.setBlkPos("E");			//表示话单块在文件中是最后一个

			begin = (split_num-1)*mConfParam.maxRecord_num;
		}
		else
		{
			pkg.setFileName(file_name);  //设置文件名
			pkg.setBlkPos("D");			 //表示话单块单独代表一个文件
		}
		
		for(int i = begin;i<end;i++)
		{
			pkg.writePkgRecord(m_record[i]);
		}

		pkg.setStatus(0);
		pkg.setNamalRcdNum(curr_record_num);
		pkg.setRecordNum(curr_record_num);   
		
		//m_record.clear();  //清空私有内存
		//vector<PkgFmt>().swap(m_record);	//2013-12-05 vector<int>().swap(nums)或者nums.swap(vector<int>())
		curr_record_num = 0;
		delete[] m_record;
		m_record = NULL;

		fileOverFlag = true;
		
		theJSLog<<"当前话单块及文件处理完毕..."<<endi;		
		
		//2013-12-09 预提交sql语句
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
				sprintf(erro_msg,"[%s]onTask() 数据库出错%s (%s)",m_szFileName,e.what(),sql);
				theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);	//发生sql执行异常
				
				char tmp[JS_MAX_FILEFULLPATH_LEN];
				memset(tmp,0,sizeof(tmp));								//文件移到错误目录
				strcpy(tmp,it->second.szSourcePath);
				strcat(tmp,mConfParam.szErroPath);
				strcat(tmp,m_szFileName);
				if(rename(fileName,tmp))
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"移动文件[%s]到错误目录[%s]失败: %s",fileName,tmp,strerror(errno));
					theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
				}

				pkg.setStatus(-1);		//该文件此时得过滤

				return ticket_num;
			}
		}

		ret = mdrDeal.IsAuditSuccess(mdrDeal.m_AuditMsg);					//仲裁弄到申请话单块完成后
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

			pkg.setStatus(-1);		//置-1表示仲裁失败,预处理,写文件模块过滤该模式
			//m_record.clear();	    //2013-10-20
			//record_num = 0;
				
			if(ret != 3)		   //2013-11-07 仲裁超时不移动文件
			{	
					char tmp[1024];
					memset(tmp,0,sizeof(tmp));								//文件移到错误目录
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

			theJSLog<<"######## end deal file ########\n"<<endi;

			return ticket_num;
		}

		//更新话单块文件信息，写调度表完成标志,仲裁成功的标识
		//getCurTime(currTime);
		//memset(sql,0,sizeof(sql));	
		//sprintf(sql,"update %s set deal_flag = 'Y',dealendtime = '%s',split_num =%d where source_id = '%s' and fileName = '%s' and deal_flag = 'W'",mConfParam.szSchCtlTabname,currTime,split_num,m_szSourceID,m_szFileName);
		//sprintf(sql,"insert into %s(source_id,serv_cat_id,filename,deal_flag,dealstarttime,dealendtime,record_num,split_num,file_id) values('%s','%s','%s','Y','%s','%s',%d,%d,%ld)",mConfParam.szSchCtlTabname,m_szSourceID,mServCatId,m_szFileName,currTime,tmp_time,record_num,split_num,file_id);			

		if(petri_status == DB_STATUS_ONLINE)
		{
			theJSLog<<"提交文件信息到数据库..."<<endi;

			stmt.commit();
			stmt.close();
			conn.close();
		}
		else
		{
			writeSQL(sql);
			theJSLog<<"数据库状态为只读态,写sql文件..."<<endi;
			commitSQLFile();
 		}			
		
	    //将文件备份
		if(mConfParam.bak_flag == 'Y')
		{		
			//判断是否需要备份,2013-07-16目录根据YYYYMM/DD										
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
				theJSLog<<"备份文件["<<m_szFileName<<"]到目录"<<bak_dir<<endi;
				strcat(bak_dir,m_szFileName);
				if(rename(fileName,bak_dir))
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"文件[%s]移动到[%s]失败: %s",fileName,bak_dir,strerror(errno));
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
		
		theJSLog<<"######## end deal file ########\n"<<endi;
	}
	
   //sleep(5);
   return ticket_num;
}



//子进程退出前的处理
void FileLoad::onChildExit()
{
    cout<<"子进程退出"<<endl;
}

//所有子进程完成任务后主进程的处理
int FileLoad::onTaskOver(int child_ret)
{
   return child_ret ;

}

//2013-10-11 新增退出函数
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
//容灾初始化
bool FileLoad::drInit()
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
int FileLoad::drVarGetSet(char* serialString)
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

bool FileLoad::CheckTriggerFile()
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
