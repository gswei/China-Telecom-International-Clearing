/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-06-19
File:			 FileLoad.cpp
Description:	 读写话单块处理
History:
<table>
revision	author            date                description
--------    ------            ----                -----------
v1.0		hed          2013-06-19

</table>
**************************************************************************/

//#include <io.h> // _findfirst() _findnext()
//#include <string.h> //strcat()
//#include <stdio.h>//gets() puts()
//#include<direct.h>
#include <dirent.h> //_chdir() _getcwd() 读取文件目录等，浏览文件夹信息
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>  //stat()函数，查询文件信息
#include<unistd.h>     //读取当前程序运行目录
#include<iostream>
#include<fstream>
#include <sstream>
#include "FileLoad.h"

#include "CF_Common.h"
#include "CF_CLogger.h"

//MdrStatus syn_status;//容灾状态
//string auditkey;
//string sessionid;
//SGW_RTInfo	rtinfo;

FileLoad::FileLoad()
{  
	//初始化话单块信息
	//data_block.header.blockSize = 【记录数*每条记录长度+话单头】 块大小读配置文件 ，还可读取 共有多少块，每块有多少记录数，记录长度
	//每个任务区20块，每块500条记录，每条记录4KB

	file_num  = 0;
	source_file_num = 0;
	//petri_status = 0;
	m_enable = false;

	split_num = 0;
	bak_flag = 'N';  //默认不备份
	record_num = 0;

    memset(m_szSrcGrpID,0,sizeof(m_szSrcGrpID));
	memset(m_szService,0,sizeof(m_szService));
	memset(mServCatId,0,sizeof(mServCatId));
	memset(m_szSourceID,0,sizeof(m_szSourceID));
	memset(m_szOutTypeId,0,sizeof(m_szOutTypeId));

	memset(input_path,0,sizeof(input_path));
	memset(erro_path,0,sizeof(erro_path));
	memset(bak_path,0,sizeof(bak_path));

	memset(m_szFileName,0,sizeof(m_szFileName));

	memset(sql,0,sizeof(sql));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(currTime,0,sizeof(currTime));
  
}


//模块初始化动作
bool FileLoad::init(int argc,char** argv)
{
   
   if(!PS_BillProcess::init(argc,argv))
   {
      return false;
   }
		
	//获取petri网状态,当系统为只读态时,数据库更新操作语句写文件
	
	//if(!(rtinfo.connect()))
	//{
	//	return false;
	//}
	//short status;
	//rtinfo.getSysMode(petri_status);
	//cout<<"petri status:"<<status<<endl;

    //syn_status = syncInit(); 

	//*********2013-06-22 读取数据库的配置信息，如数据源组，错误目录 2013-03-11 add by hed*********************************************/
	
	if(!(dbConnect(conn)))
	{
		cout<<"初始化数据库 connect error."<<endl;
		return false ;
	}

	int flow_id = getFlowID();
	int module_id = getModuleID(); 
	//cout<<"流水线ID:"<<flow_id<<"   模块ID:"<<module_id<<endl;

	try{

		string sql = "select source_group from TP_BILLING_LINE  where billing_line_id = :1";
		Statement stmt = conn.createStatement();
		stmt.setSQLString(sql);
		stmt<<flow_id;
		stmt.execute();
		if(!(stmt>>m_szSrcGrpID))
		{
			cout<<"请在tp_billing_line表中配置数据源组"<<endl;
			return false ;
		}

		sql = "select ext_param from TP_PROCESS where billing_line_id = :1 and module_id = :2";
		stmt.setSQLString(sql);
		stmt<<flow_id<<module_id;
		stmt.execute();
		if(!(stmt>>m_szService))
		{
			cout<<"请在tp_process表中配置service"<<endl;
			return false ;
		}

		sql = "select c.input_path from C_SOURCE_GROUP_DEFINE a,C_SERVICE_FLOW b,C_SERVICE_INTERFACE c where a.source_group=:1" 
                    "and a.workflow_id = b.workflow_id and b.service=:2 and b.input_id = c.interface_id ";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID<<m_szService;
		stmt.execute();
		if(!(stmt>>input_path))
		{
			cout<<"表中加载模块输入文件相对路径input_path没有配置"<<endl;
			return false ;
		}
		completeDir(input_path);
	
		//2013-07-01 增加判断备份标志及路径
		sql = "select var_value from c_process_env where varname = 'LOAD_FILE_BAK_FLAG' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID<<m_szService;
		stmt.execute();
		if(!(stmt>>bak_flag))
		{
			bak_flag = 'N';			 
		}
		if(bak_flag == 'Y')
		{
			sql = "select var_value from c_process_env where varname = 'LOAD_FILE_BAK_DIR' and source_group=:1 and service=:2 ";
			stmt.setSQLString(sql);
			stmt<<m_szSrcGrpID<<m_szService;
			stmt.execute();
			if(!(stmt>>bak_path))
			{
					cout<<"请在表c_process_env中配置加载模块的备份目录,LOAD_FILE_BAK_DIR"<<endl;
					return false;
			}
			completeDir(bak_path);
		}
		
		sql = "select var_value from c_process_env where varname = 'LOAD_FILE_ERR_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID<<m_szService;
		stmt.execute();
		if(!(stmt>>erro_path))
		{
				cout<<"请在表c_process_env中配置加载模块的错误路径,LOAD_FILE_ERR_DIR"<<endl;
				return false;
		}	
		completeDir(erro_path);

		stmt.close();
		
	   }catch(SQLException  e)
		{
			cout<<"初始化时数据库查询异常:"<<e.what()<<endl;
			return false ;
		}
	
	
	char sParamName[256];
	CString sKeyVal;
	memset(sParamName,0,sizeof(sParamName));
	sprintf(sParamName, "billing_line.%d.record_num", getFlowID());		//获取话单块的最大记录数
	param_cfg.bGetMem(sParamName, sKeyVal) ;
	maxRecord_num=sKeyVal.toInteger();
	
	//2013-08-16新增可以配置每次扫描数据源目录下面指定个数文件后调到下个数据源
	sprintf(sParamName, "business.source.file.%d.num",getFlowID());
	if(param_cfg.bGetMem(sParamName, sKeyVal))
	{
		source_file_num = sKeyVal.toInteger();
	}
	else
	{	
		cout<<"请在配置文件中配置流水线["<<flow_id<<"]中数据源每次扫描文件的个数,参数名:"<<sParamName<<endl;
		return false ;
	}	


	bool bb = initializeLog(argc,argv,false);  //是否调试模式
	if(!bb)
	{
			//theJSLog<<"初始化内存日志接口失败"<<endi;
			return false;
	}

	//writelog(90155,"测试加载模块写日志--业务接口");  //类别:debug|级别:2|
	//PS_Process::writeLog(90156,"测试加载模块写日志--框架接口");  //类别:error|级别:2
	//exit(-1);	

	theJSLog.setLog(szLogPath,szLogLevel,m_szService , m_szSrcGrpID, 001);
	
	theJSLog<<"数据源组:"<<m_szSrcGrpID<<"   service:"<<m_szService<<"  相对输入路径:"<<input_path<<"  错误路径:"<<erro_path
			<<"	日志路径:"<<szLogPath<<" 日志级别:"<<szLogLevel<<" sql存放路径:"<<sql_path<<"	每个数据源扫描文件个数:"<<source_file_num<<endi;
	
	if(bak_flag == 'Y')
	{
		theJSLog<<"文件备份路径:"<<bak_path<<endi;
	}

    theJSLog<<"加载数据源配置信息LoadSourceCfg..."<<endi;

	if(LoadSourceCfg() == -1) return false ;  //加载数据源配置信息
	
	conn.close();


	char input_dir[1024],bak_dir[1024],erro_dir[1024];
	int rett = -1;
	
	DIR *dirptr = NULL; 
	for(map<string,SOURCECFG>::const_iterator iter = m_SourceCfg.begin();iter !=m_SourceCfg.end(); ++iter)
	{		   		
			memset(input_dir,0,sizeof(input_dir));
			strcpy(input_dir,iter->second.szSourcePath);
			strcat(input_dir,input_path);
			if((dirptr=opendir(input_dir)) == NULL)
			{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"数据源[%s]的输入文件路径[%s]不存在",iter->first,input_dir);
					theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错

					return false ;
			}else closedir(dirptr);

			if(bak_flag == 'Y')
			{
					memset(bak_dir,0,sizeof(bak_dir));
					strcpy(bak_dir,iter->second.szSourcePath);
					strcat(bak_dir,bak_path);
					if((dirptr=opendir(bak_dir)) == NULL)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"数据源[%s]的备份文件路径[%s]不存在",iter->first,bak_dir);
						theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
						return false ;
					}else closedir(dirptr);
			}
			
			memset(erro_dir,0,sizeof(erro_dir));
			strcpy(erro_dir,iter->second.szSourcePath);
			strcat(erro_dir,erro_path);
			if((dirptr=opendir(erro_dir)) == NULL)
			{
					theJSLog<<"数据源【"<<iter->first<<"】的错误文件路径: "<<erro_dir<<"不存在，自行创建"<<endi;
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

   //测试数据库操作
   //excuteSQL();   
   
   //核心参数获取
   //CString sKeyVal;
   //param_cfg.bGetMem("memory.MT_DSPCH.sem_key_value", sKeyVal);
   //cout<<"获取的核心参数值:"<<sKeyVal<<endl;

   //测试写日志
   //writeLog(0,"测试写日志");

   //exit (0);
	
   if(!drInit()) return false;
   //cout<<"m_triggerFile = "<<m_triggerFile<<endl;

/*
   char tmp[10];
   memset(tmp,0,sizeof(tmp));
   if(tmp[0] == '\0')
   {
			cout<<"tmp为空"<<endl;
   }
   char* p = NULL;
   if(strcmp(tmp,p) == 0)
   {
		cout<<"NULL = 0"<<endl;

   }
*/

   it = m_SourceCfg.begin();   //初始化第一个数据源
	
   theJSLog<<"初始化完毕！"<<endi;

   return true ;
}

FileLoad::~FileLoad()
{
	if(m_enable) 
	{
		int ret = dr_ReleasePlatform();
		if(ret != 0)
		{
			char tmp[100] = {0};
			snprintf(tmp, sizeof(tmp), "释放容灾平台失败,返回值=%d", ret);
			theJSLog<<tmp<<endi;
		}
	}
}

//加载数据源配置信息
int FileLoad::LoadSourceCfg()
{
	char szSqlStr[400];
	int iSourceCount=0;
	string sql;
	try
	{
		Statement stmt = conn.createStatement();
		sql = "select FILETYPE_ID from C_SOURCE_GROUP_DEFINE where SOURCE_GROUP=:1";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID;
		if(stmt.execute())
		{
			stmt>>m_szOutTypeId;
		}
		
		outrcd.Init(m_szOutTypeId);								//2013-07-31

   		//expTrace(szDebugFlag, __FILE__, __LINE__,"%s;%s",m_szSrcGrpID,m_szOutTypeId);
		sql = "select count(*) from C_SOURCE_GROUP_CONFIG where SOURCE_GROUP=:1";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID;
		if(stmt.execute())
		{
			stmt>>iSourceCount;
		}

		//expTrace(szDebugFlag, __FILE__, __LINE__, "iSourceCount=%d;", iSourceCount);
		
		sql = "select a.source_id,b.file_fmt,b.source_path,b.SERV_CAT_ID from C_SOURCE_GROUP_CONFIG a,I_SOURCE_DEFINE b where SOURCE_GROUP=:1 and a.source_id=b.source_id";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID;
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
		     }
		}

	}catch (SQLException e)
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"LoadSourceCfg数据库出错：%s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		//throw jsexcp::CException(0, "LoadSourceCfg加载数据库出错：", __FILE__, __LINE__);
		return -1;
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
		sprintf(erro_msg,"getSourceFilter数据库出错：%s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
		return -1 ;
	}
	
	return 0;
}


//数据库的更新操作，便于捕获异常sql 便于重现
int FileLoad::updateDB(char* sql)
{
	int ret = 0;
	try
	{
		//cout<<"SQL:"<<sql<<endl;
		Statement stmt = conn.createStatement();
		stmt.setSQLString(sql);
		ret = stmt.execute();		//返回0表示更新失败
		stmt.close();

	}
	catch(util_1_0::db::SQLException e)
	{ 
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"updateDB 数据库出错：%s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		return -1;
	}

	return ret;
}


//判断是否要申请话单块
int FileLoad::onBeforeTask()
{
	if(m_record.size() > 0) return 1 ;  //表示上次文件的记录数一个话单块没处理完

	/**************************扫描数据源，读取话单文件*************************************/
		
	int ret = 0;	
	char szBuff[1012],dir[1024],filter[256],inputFilePath[1024]; 
		
	if( drStatus == 1 )  //备系统
	{
			isWriteSQLFileByTime();		

			//检查trigger触发文件是否存在
			if(!CheckTriggerFile())
			{
				sleep(1);
				return 0;
			}

			//获取同步变量
			memset(m_SerialString,0,sizeof(m_SerialString));
			ret = drVarGetSet(m_SerialString);
			if(ret)
			{
				theJSLog<<"同步失败..."<<endl;
				return 0;
			}
	
			//获取同步变量
			vector<string> data;		
			splitString(m_SerialString,";",data,true,true);  //发送的字符串数据源ID,文件名,sqlFile文件名
			
			isWriteSQLFileByMain(data[2].c_str());	//备系统的sqlFile是通过主系统传过来的

			it = m_SourceCfg.find(data[0]);
			if(it == m_SourceCfg.end())			//考虑是否仲裁???
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"没有找到该数据源信息[%s]",data[0]);		//环境变量未设置
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);

				return 0;
			}
			
			//读文件加载文件到私有内存,
			memset(fileName,0,sizeof(fileName));
			memset(m_szFileName,0,sizeof(m_szFileName));
			memset(dir,0,sizeof(dir));
			
			strcpy(dir,it->second.szSourcePath);  //数据源主路径
			strcpy(inputFilePath,dir);
			strcat(inputFilePath,input_path);

			strcpy(fileName,inputFilePath);
			strcat(fileName,data[1].c_str());

			//int dr_GetAuditMode()1表示同步，2表示跟随, 其它为失败，-1是配置错误，-2配置文件读取出现问题
			int iStatus = dr_GetAuditMode(module_name);
			if(iStatus == 1)		//同步模式,只能是单边情况
			{	
				while(1)
				{
					//设置中断
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
			else if(iStatus == 2)		//跟随模式,默认300s
			{
				bool flag = false;
				int times = 1;
				while(times < 31)
				{
					if(access(fileName,F_OK|R_OK))
					{
						theJSLog<<"查找了"<<times<<"次文件"<<endi;
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
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"主系统传过来的文件[%s]不存在",fileName);
					theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);

					//需要写调度表吗,只有先写到这儿
					memset(m_AuditMsg,0,sizeof(m_AuditMsg));
					sprintf(m_AuditMsg,"%s not find",data[1]);
					if(!IsAuditSuccess(m_AuditMsg))
					{
						memset(sql,0,sizeof(sql));
						getCurTime(currTime); 
						sprintf(sql,"insert into D_SCH_LOAD(source_id,serv_cat_id,filename,deal_flag,dealstarttime,record_num,split_num) values('%s','%s','%s','E','%s',0,0)",data[0],it->second.serverCatID,data[1].c_str(),currTime);
						writeSQL(sql);
						commitSQLFile();	
					}

					return 0;
				}
			}
			else
			{
				char tmp[50] = {0};
				snprintf(tmp, sizeof(tmp), "容灾平台dr_GetAuditMode函数配置错误，返回值[%d]", iStatus);
				theJSLog<<tmp<<endi;
				return 0;
			}

			memset(m_szSourceID,0,sizeof(m_szSourceID));
			memset(m_szFileName,0,sizeof(m_szFileName));
			memset(mServCatId,0,sizeof(mServCatId));

			strcpy(m_szFileName,data[1].c_str());
			strcpy(m_szSourceID,it->first.c_str());
			strcpy(mServCatId,it->second.serverCatID);
				
	}	
	else								//主系统,非容灾系统
	{
		
		isWriteSQLFile();				//是否提交sql文件

		if(file_num >= source_file_num)
		{
			file_num = 0;		//cout<<"在同一个数据源下面扫描到N个文件后，跳到下个数据源"<<endl;
			++it ;
		}
		
		if(it == m_SourceCfg.end())
		{
			it = m_SourceCfg.begin();
		}

		memset(dir,0,sizeof(dir));
		strcpy(dir,it->second.szSourcePath);  //数据源主路径		

		memset(inputFilePath,0,sizeof(inputFilePath));
		strcpy(inputFilePath,dir);
		strcat(inputFilePath,input_path); 

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
							//cout<<"扫描到临时文件，舍弃"<<fileName<<endl;
							continue;
						}
				}
				
				theJSLog<<"扫描到文件:"<<fileName<<endi;
				
				memset(m_szSourceID,0,sizeof(m_szSourceID));
				memset(m_szFileName,0,sizeof(m_szFileName));
				memset(mServCatId,0,sizeof(mServCatId));

				strcpy(m_szFileName,p+1);
				strcpy(m_szSourceID,it->first.c_str());		  //当前数据源的source_id
				strcpy(mServCatId,it->second.serverCatID);	
				
				//p = strrchr(sqlFile,'/');
				//memset(tmp,0,sizeof(tmp));
				//if(p)
				//{
				//	strcpy(tmp,p+1);
				//}
				//else strcpy(tmp,sqlFile);

				memset(m_SerialString,0,sizeof(m_SerialString));
				sprintf(m_SerialString,"%s;%s;%s",m_szSourceID,m_szFileName,sqlFile);
				ret = drVarGetSet(m_SerialString);
				if(ret)
				{
					theJSLog<<"同步失败...."<<endi;
					return 0;
				}

				break;			//找到文件退出循环，可能一个文件有占用多个话单块
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
	char szBuff[1024],tmp[1024],state;
	try
	{
					//获取petri网状态
					//rtinfo.getSysMode(petri_status);
			/*		
					int cnt = 0;
					//string sql = "select count(*) from D_SCH_LOAD where source_id = :1 and filename = :2 and deal_flag = :3";
					memset(sql,0,sizeof(sql));
					sprintf(sql,"select count(*) from D_SCH_LOAD where source_id = '%s' and filename = '%s' and deal_flag = 'Y'",m_szSourceID,m_szFileName);					
					Statement stmt = conn.createStatement();
					stmt.setSQLString(sql);
					//stmt<<m_szSourceID<<m_szFileName<<'Y';
					stmt.execute();
					stmt>>cnt;
					if(cnt > 0)  
					{
							theJSLog<<"调度表已经存在该文件,且已经被成功处理,移到错误目录"<<endi;
							memset(tmp,0,sizeof(tmp));
							strcpy(tmp,it->second.szSourcePath);
							strcat(tmp,erro_path);
							strcat(tmp,m_szFileName);
							rename(fileName,tmp);
							return 0;
					}
			*/																
					//读文件加载话单 ,默认以文件不存在时产生错误，常和in或app联合使用
					ifstream in(fileName,ios::nocreate);
					if(!in)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"dealFile() 打开文件%s失败！！！ ",fileName);
						theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败

						return 0;
					}
					
					split_num = 0;	  //初始化分割数和文件记录条数
					record_num = 0;

					//扫描到一个文件，将其注册到调度表
					getCurTime(currTime);    //获取当前文件时间
					memset(sql,0,sizeof(sql));
					//sql = "insert into D_SCH_LOAD(source_id,serv_cat_id,filename,deal_flag,dealstarttime,record_num,split_num) values(:1,:2,:3,:4,:5,:6,:7)";
					sprintf(sql,"insert into D_SCH_LOAD(source_id,serv_cat_id,filename,deal_flag,dealstarttime,record_num,split_num) values('%s','%s','%s','W','%s',0,0)",m_szSourceID,mServCatId,m_szFileName,currTime);			
					writeSQL(sql);
					
					
					theJSLog<<"将文件"<<m_szFileName<<"记录加载到私有内存"<<endi;
					
					memset(szBuff,0,sizeof(szBuff));
					//一次性加载一个文件全部数据，对数据进行若超过话单块记录数，则进行分配申请
					while(in.getline(szBuff,sizeof(szBuff)))   
					{					
						//cout<<"读取记录"<<szBuff<<endl;
						PkgFmt fmt ;
						strcpy(fmt.status,"0");
						strcpy(fmt.type,"H");
						strcpy(fmt.code,"load");
						strcpy(fmt.record,szBuff);
						m_record.push_back(fmt);

						memset(szBuff,0,sizeof(szBuff));
					}
					
					
					record_num = m_record.size();

					//获取文件ID,以第一条记录为准
					outrcd.Set_record(m_record[0].record);
					char file_id[10];
					memset(file_id,0,sizeof(file_id));
					outrcd.Get_Field(FILE_ID,file_id);


					//**************做仲裁,数据源,文件名,单个话单块能存放的记录条数,记录条数
					memset(sql,0,sizeof(sql));

					memset(m_AuditMsg,0,sizeof(m_AuditMsg));
					sprintf(m_AuditMsg,"%s;%s;%d;%d",m_szSourceID,m_szFileName,record_num,maxRecord_num);
					if(!IsAuditSuccess(m_AuditMsg))
					{
						//调度表置E文件移到错误目录
						state = 'E';
						getCurTime(currTime);  
						sprintf(sql,"update D_SCH_LOAD set deal_flag = '%c',dealendtime = '%s',file_id = %ld,record_num =%d,split_num =%d where source_id = '%s' and fileName = '%s' ",state,currTime,file_id,record_num,split_num,m_szSourceID,m_szFileName);
						writeSQL(sql);
						commitSQLFile();		

						memset(tmp,0,sizeof(tmp));								//文件移到错误目录
						strcpy(tmp,it->second.szSourcePath);
						strcat(tmp,erro_path);
						strcat(tmp,m_szFileName);
						rename(fileName,tmp);

						return 0;
					}
									
					strcpy(file_name,m_szFileName);  //分割前默认与原始文件名相同					
					
					//更新注册调度表				
					//sql = "update D_SCH_LOAD set deal_flag = :1 where source_id = :2 and filename = :3";
					getCurTime(currTime); 
					sprintf(sql,"update D_SCH_LOAD set deal_flag = 'H',dealendtime = '%s',file_id = %ld where source_id = '%s' and filename = '%s'",currTime,atol(file_id),m_szSourceID,m_szFileName);								
					writeSQL(sql);	
					
					ret = 1;

	}
	catch(jsexcp::CException e)
	{
			rollBackSQL();

			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"dealFile() error %s",e.GetErrMessage());
			theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//读文件异常

			memset(tmp,0,sizeof(tmp));								//文件移到错误目录
			strcpy(tmp,it->second.szSourcePath);
			strcat(tmp,erro_path);
			strcat(tmp,m_szFileName);
			rename(fileName,tmp);
			
			return 0;
	}

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
    theJSLog<<"任务地址:"<<task_addr<<endi;
    char tmp[1024];

    memset(task_addr,0,getBlockSize());   //初始化话单内存块,清空内存	

	PkgBlock pkg((char*)task_addr);       //话单块类初始化
	pkg.init(getTicketLength(),getBlockSize());

	pkg.setModuleId(getModuleID());			//设置模块ID
	pkg.setStatus(0);						//话单状态，0未处理，1处理
	pkg.setSourceId(it->first.c_str());		//设置数据源
	pkg.setFileHeadFlag(0);					//设置文件头标志
	pkg.setFileTailFlag(0);					//设置文件尾标志
	pkg.setFileType(it->second.szInFileFmt);//设置文件类型

	theJSLog<<"处理文件:"<<m_szFileName<<"	记录条数:"<<m_record.size()<<endi;

	if(m_record.size() > maxRecord_num)             //此时文件记录需要分割为多个话单块
	{
		 //record_num += maxRecord_num;
		 split_num++;
		 sprintf(file_name,"%s#%04d",m_szFileName,split_num);

		 //theJSLog<<"文件记录数太多需要分割,文件名："<<file_name<<endi;

		 pkg.setFileName(file_name);  //设置文件名  

		 vector<PkgFmt>::iterator  iter1 = m_record.begin();
		 vector<PkgFmt>::iterator  iter2 = m_record.begin();
		 //int position = maxRecord_num*(split_num-1);
		
		 for(int i = 0;i<maxRecord_num;i++)
		 {
			 //cout<<"要写的话单记录："<<m_record[i].record<<" 记录长度："<<strlen(m_record[i].record)<<endl;
			 pkg.writePkgRecord(m_record[i]);
			 ++iter2;
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
		 pkg.setNamalRcdNum(maxRecord_num);
		 pkg.setRecordNum(maxRecord_num); 
		
		 //删除前前面N条话单块的内容
		//cout<<"准备删除记录"<<endl;
		m_record.erase(iter1,iter2);
			
	}
	else
	{
		if(split_num)					//表示文件记录需要拆分为多个话单块
		{
			split_num++;
			sprintf(file_name,"%s#%04d",m_szFileName,split_num);
			pkg.setFileName(file_name);
			pkg.setBlkPos("E");			//表示话单块在文件中是最后一个
		}
		else
		{
			pkg.setFileName(file_name);  //设置文件名
			pkg.setBlkPos("D");			 //表示话单块单独代表一个文件
		}

		//record_num  += m_record.size();			//记录数可以考虑在加载完文件开始时就保存下来，不用每次相加
		for(int i = 0;i<m_record.size();i++)
		{
			//cout<<"要写的话单记录："<<m_record[i].record<<" 记录长度："<<strlen(m_record[i].record)<<endl;
			pkg.writePkgRecord(m_record[i]);
		}

		pkg.setStatus(0);
		pkg.setNamalRcdNum(m_record.size());
		pkg.setRecordNum(m_record.size());   
		
		m_record.clear();  //清空私有内存	
		
		
		getCurTime(currTime);  
		memset(sql,0,sizeof(sql));
		//char state = 'Y';	
		//更新话单块文件信息，写调度表完成标志
		//string sql = "update D_SCH_LOAD set deal_flag = :1,dealendtime = :2,record_num = :3,split_num = :4 where source_id = :5 and fileName = :6 ";
		sprintf(sql,"update D_SCH_LOAD set deal_flag = 'Y',dealendtime = '%s',record_num =%d,split_num =%d where source_id = '%s' and fileName = '%s' ",currTime,record_num,split_num,m_szSourceID,m_szFileName);			
		writeSQL(sql);
		commitSQLFile();			
		
	    //将文件备份
		if(bak_flag == 'Y')
		{		
				//判断是否需要备份,2013-07-16目录根据YYYYMM/DD										
				char bak_dir[1024];
				memset(bak_dir,0,sizeof(bak_dir));
				strcpy(bak_dir,it->second.szSourcePath);
				strcat(bak_dir,bak_path);
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
		
	/*	
	   char tmp[1024];   //容灾内容，文件名，数据源，文件记录数
	   memset(tmp,0,sizeof(tmp));
	   sprintf(tmp,"%s;%s;%d",m_szSourceID,m_szFileName,record_num);
		
	   MdrAuditInfo audit_info_;
       if ( syn_status == 1 )			   //处理备系统内容
		{		
			if( slaveAudit(audit_info_,tmp)) 
			{
                 cmtResult(audit_info_ );
				 cout<<"仲裁提交成功"<<endl;
			}
			else
			{
				cout<<"仲裁提交失败"<<endl;
			}
		
		}
		else                               //主系统处理完一个文件，则发送信息给备份系统进行对比
		{
			if( masterAudit( audit_info_, tmp ) )
			{
				cmtResult( audit_info_ );
				cout<<"仲裁提交成功"<<endl;	
			} 
			else 
			{
				 cout<<"仲裁提交失败"<<endl;
			}

		}
		
	*/

	}

   //sleep(30);   //暂停30秒，便于观察

   return -1;
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

//数据库操作
int FileLoad::excuteSQL()
{	
    return 1;

}


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
			theJSLog << "容灾平台初始化失败,返回值=" << ret<<endi;
			return false;
		}
		else
		{
			theJSLog<<"dr_InitPlatform ok."<<endi;
		}

		m_enable = true;

		drStatus = _dr_GetSystemState();	//获取主备系统状态
		if(drStatus < 0)
		{
			theJSLog<<"获取容灾平台状态出错: 返回值="<<drStatus<<endi;
			return false;
		}
		
		if(drStatus == 0)		theJSLog<<"当前系统配置为主系统"<<endi;
		else if(drStatus == 1)	theJSLog<<"当前系统配置为备系统"<<endi;
		else if(drStatus == 2)	theJSLog<<"当前系统配置非容灾系统"<<endi;

		return true;
}

//主系统发送同步变量,备系统获取同步变量
int FileLoad::drVarGetSet(char* serialString)
{
		int ret ;
		char tmpVar[5000] = {0};

		//检查容灾平台的切换锁
		ret = dr_CheckSwitchLock();   
		if(ret != 0)  
		{  
			theJSLog<<"检查容灾切换锁失败,返回值="<<ret<<endi;
			return -1;  
		} 
		//初始化index  
		ret = dr_InitIDX();  
		if(ret != 0)  
		{  
			theJSLog<<"初始化index失败,返回值=" <<ret<<endi;
			dr_AbortIDX();
			return -1;  
		}

		snprintf(tmpVar, sizeof(tmpVar), "%s", serialString);
		//主系统把要同步的index “键值对”写入容灾平台维护的index 文件中
		//备系统调用该函数的结果是，var获得和主系统一样的随机变量的值。	SYNC_SINGLE表示注册单一的随机变量
		ret = dr_SyncIdxVar("serialString", tmpVar, SYNC_SINGLE);		
		if (ret != 0)
		{
			theJSLog<<"传序列串时失败，序列名：["<<serialString<<"]"<<endi;
			dr_AbortIDX();
			return -1;
		}
		//serialString = tmpVar;			//同步索引字符串,主系统是赋值，备系统是取值
		strcpy(serialString,tmpVar);
		//m_AuditMsg = tmpVar;			//要仲裁的字符串

		// <5> 传输实例名  用于主系统注册此IDX的模块调用参数。
		//备系统的index manager检查IDX条件满足后，把使用该函数注册的随机变量作为模块的调用参数trigger相应的进程
		snprintf(tmpVar, sizeof(tmpVar), "%d", getPrcID());
		ret = dr_SyncIdxVar("@@ARG", tmpVar,SYNC_SINGLE);  
		if(ret !=0)
		{
			theJSLog<<"传输实例名失败："<<tmpVar<<endi;
			dr_AbortIDX();  
			return -1;
		}
		
		
		// <6> 预提交index  此关键字用于将平台当前内存中的随机变量写入磁盘
		ret = dr_SyncIdxVar("@@FLUSH","SUCCESS",SYNC_SINGLE);  
		if (ret != 0 )
		{
			theJSLog<<"预提交index失败"<<endi;
			dr_AbortIDX();
			return -1;
		}
		
		
		// <7> 提交index  	提交Index。在index文件中设置完成标志
		ret = dr_CommitIDX();  
		if(ret != 0)  
		{  
			theJSLog<<"提交index失败,返回值="<<ret<<endi;
			dr_AbortIDX();  
			return -1;  
		}

		//备系统搜索目录
		//if(!m_syncDr.isMaster())thelog<<"备系统SerialString："<<m_SerialString<<endi;

		theJSLog<<"本次的同步串serialString:"<<serialString<<endi;//for test

		return ret;

}

//仲裁字符串
 bool FileLoad::IsAuditSuccess(const char* dealresult)
 {
		int auitStatus = 0, ret = 0;
		ret = dr_Audit(dealresult);
		if(2 == ret )
		{
			theJSLog << "容灾仲裁失败,结果:" << ret <<"本端："<<dealresult<< endi;
			dr_AbortIDX();
			return false;
		}
		else if (3 == ret)
		{
			theJSLog<<"容灾仲裁超时..."<<endi;
			dr_AbortIDX();
			return false;
		}
		else if(1 == ret)
		{
			ret = dr_CommitSuccess();
			if(ret != 0)
			{
				theJSLog << "业务全部提交失败(容灾平台)" << endi;
				dr_AbortIDX();
				return false;
			}
			theJSLog<<"ret = "<<ret<<"仲裁成功...\n仲裁内容："<<dealresult<<endi;
			return true;
		}
		else
		{
			theJSLog<<"未知ret="<<ret<<"	仲裁内容："<<dealresult<<endi;
			dr_AbortIDX();
			return false;
		}
	
	return true;
 }

bool FileLoad::CheckTriggerFile()
{
	int ret = 0;
	if(access(m_triggerFile.c_str(),F_OK) != 0)	return false;

	theJSLog<< "检查到trigger文件,并删除"<< m_triggerFile <<endl;

	ret = remove(m_triggerFile.c_str());	
	if(ret) 
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"删除trigger文件[%s]失败: %s",m_triggerFile,strerror(errno));
		theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
	}
}

/*

//容灾系统初始化
MdrStatus FileLoad::syncInit()
{
	MdrRetCode rc_;
    rc_ = mdr_InitPlatform();  //初始化容灾平台
    printf( "%s,%d-->mdr_InitPlatform, rc_=%d, (rc=>0:SUCCESS,1:FAILURE,-1:INVALID)\n", __FILE__, __LINE__, rc_ );
    //writelog( 0, "mdr_InitPlatform" );

    if( rc_ != MDR_SUCCESS ) {
        //printf( "%s,%d-->FATAL! mdr_InitPlatform failed\n", __FILE__, __LINE__ );
        //writelog( 0, "初始化容灾平台出错!" );
        cerr<<"初始化容灾平台出错!"<<endi;
    }

    MdrStatus stat_;
    rc_ = mdr_GetDRStatus( stat_ );
    printf( "%s,%d-->mdr_GetDRStatus, rc_=%d, stat_=%d, (stat=>0:MASTER,1:SLAVE,2:NODR)\n", __FILE__, __LINE__, rc_, stat_ );

    if( rc_ != MDR_SUCCESS ) {
        printf( "%s,%d-->FATAL! mdr_GetDRStatus failed\n", __FILE__, __LINE__ );
        //writelog( 0, "获取容灾平台状态失败!" );
        cerr<<"获取容灾平台状态失败!"<<endi;
    }

    MdrNodeType node_type_;
    rc_ = mdr_GetNodeType( node_type_ );
    printf( "%s,%d-->mdr_GetNodeType, rc_=%d, node_type_=%d (node_type=>0:DUPLEX,1:SINGLE)\n", __FILE__, __LINE__, rc_, node_type_ );

    if( rc_ != MDR_SUCCESS ) {
        //printf( "%s,%d-->FATAL! mdr_GetNodeType failed\n", __FILE__, __LINE__ );
        //writelog( 0, "初始化容灾平台出错!" );
        cerr<<"初始化容灾平台出错!"<<endi;
    }

    rc_ = mdr_GetNodeTypeSR( node_type_ );
    printf( "%s,%d-->mdr_GetNodeTypeSR, rc_=%d, sr_node_type_=%d\n", __FILE__, __LINE__, rc_, node_type_ );

    if( rc_ != MDR_SUCCESS ) {
        //printf( "%s,%d-->FATAL! mdr_GetNodeTypeSR failed\n", __FILE__, __LINE__ );
        //writelog( 0, "初始化容灾平台出错!" );
        cerr<<"初始化容灾平台出错!"<<endi;
    }

	if(rc_ != MDR_SUCCESS )
	{
			exit(-1);
	}

    return stat_;

}

bool FileLoad::runmdr( char* value )
{
    if( syn_status == 1 ) {
        std::vector<MdrVarInfo> var_list_;
        var_list_.clear();
        return true;
    } else {
        MdrAuditInfo audit_info_;
        cout << "进入主系统" << endl;
        if( masterAudit( audit_info_, value ) ) {
            cmtResult( audit_info_ );
            return true;
        } else {
            return false;
        }

    }
}

//填充主系统内容
void FileLoad::fillMasterAuditInfo( MdrAuditInfo& audit_info, char* value  )
{
    int pid_ = getpid();
    char buf_[1024];
    audit_info.node = "TPSS1";
    audit_info.srvContext = "srvContext";

    struct timeval tv_;
    ::gettimeofday( &tv_, NULL );
    tv_.tv_sec --;
    struct tm tm_;
    ::localtime_r( &tv_.tv_sec, &tm_ );
    char tmp_[64];
    strftime( tmp_, sizeof( tmp_ ), "%Y%m%d%H%M%S", &tm_ );
    sprintf( buf_, "%s.%06d", tmp_, ( int )( tv_.tv_usec % 1000000 ) );
    audit_info.ccrRcvTime = buf_;

    sprintf( buf_, "auditKey_%d_%s", pid_, audit_info.ccrRcvTime.c_str() );
    audit_info.auditKey = buf_;
    sprintf( buf_, "sessionId_%d_%s", pid_, audit_info.ccrRcvTime.c_str() );
    audit_info.sessionId = buf_;

    audit_info.rflag = 2;			// must fill 2

    audit_info.syncVar = string( value );
    audit_info.auditVal = audit_info.syncVar;
    tv_.tv_sec --;
    ::localtime_r( &tv_.tv_sec, &tm_ );
    strftime( tmp_, sizeof( tmp_ ), "%Y%m%d%H%M%S", &tm_ );
    sprintf( buf_, "%s.%06d", tmp_, ( int )( tv_.tv_usec % 1000000 ) );
    audit_info.ccrEvtTime = buf_;
    audit_info.result = 0;    
    //cout << "auditvalue = " << audit_info.syncVar << endl;
    //cout << "auditkey = " << auditkey << endl;
    //cout << "sessionid = " << sessionid << endl;
}

//主系统仲裁
bool FileLoad::masterAudit( MdrAuditInfo& audit_info,char* value )
{
    fillMasterAuditInfo( audit_info, value );
    MdrRetCode rc_ = mdr_Audit( audit_info );    //audit_info.result字段作为传出的参数会被API修改
    std::string audit_info_str_;
    //audit_info.toStr( audit_info_str_ );
    //audit_info_str_ = audit_info.toStr();
    cout << "auditKey " << audit_info.auditKey << endl;
    cout << "auditVal " << audit_info.auditVal << endl;
    cout << "sessionId " << audit_info.sessionId << endl;
    cout << "rflag " << audit_info.rflag << endl;
    
    printf( "%s,%d-->mdr_Audit, rc_=%d, %s\n", __FILE__, __LINE__, rc_, audit_info_str_.data() );
    //writelog( 0, "主系统仲裁内容为 " + audit_info_str_ );

    if( rc_ != MDR_SUCCESS ) {
        printf( "%s,%d-->FATAL! mdr_Audit failed\n", __FILE__, __LINE__ );
        //writelog( 0, "主系统仲裁失败 " );
        return false;
    }

    return true;
}


bool FileLoad::slaveAudit( MdrAuditInfo& audit_info ,char *allval)
{
    audit_info.node = "TPSS2";
    audit_info.srvContext = "srvContext";
    audit_info.rflag = 2;					// must fill 2
    audit_info.auditKey = auditkey;			//值是通过前面主系统传过来的
    audit_info.sessionId = sessionid;
    audit_info.auditVal = allval;
    audit_info.ccrEvtTime = "20000101000000";		// SLAVE side: dummy value to pass fmt validation
    audit_info.ccrRcvTime = "20000101000000.000000";	// SLAVE side: dummy value to pass fmt validation

    MdrRetCode rc_ = mdr_Audit( audit_info );
    cout << "auditKey " << audit_info.auditKey << endl;
    cout << "auditVal " << audit_info.auditVal << endl;
    cout << "sessionId " << audit_info.sessionId << endl;
    cout << "rflag " << audit_info.rflag << endl;
    
    //printf( "%s,%d-->mdr_Audit, rc_=%d, %s\n", __FILE__, __LINE__, rc_, audit_info.data() );
    printf( "%s,%d-->mdr_Audit, rc_=%d\n", __FILE__, __LINE__, rc_ );
   // writelog( 0, "备系统仲裁内容为" + audit_info.toStr() );

    if( rc_ != MDR_SUCCESS ) {
        printf( "%s,%d-->FATAL! mdr_Audit failed\n", __FILE__, __LINE__ );
        writelog( 0, "备系统仲裁失败 " );
        return false ;
    }

    return true ;

}

void FileLoad::cmtResult( const MdrAuditInfo& audit_info )
{
    MdrRetCode rc_ = mdr_CmtResult( audit_info );
    std::string audit_info_str_;
    //audit_info.toStr( audit_info_str_ );
    printf( "%s,%d-->mdr_CmtResult, rc_=%d, %s\n", __FILE__, __LINE__, rc_, audit_info_str_.data() );
    writelog( 0, "mdr_CmtResult 结果为" + audit_info_str_ );

    if( rc_ != MDR_SUCCESS ) {
        //printf( "%s,%d-->FATAL! mdr_CmtResult failed\n", __FILE__, __LINE__ );
        writelog( 0, "处理失败" );
    }
}

*/