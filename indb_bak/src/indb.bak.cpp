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


#include<dirent.h> //_chdir() _getcwd() 读取文件目录等，浏览文件夹信息
#include<string>
#include<vector>
#include<sys/types.h>
#include<sys/stat.h>  //stat()函数，查询文件信息
#include<unistd.h>     //读取当前程序运行目录
#include<iostream>
#include<fstream>
#include <sstream>
#include "indb.h"

#include "CF_Common.h"
#include "CF_CLogger.h"

CLog theJSLog;
SGW_RTInfo	rtinfo;

C_Indb::C_Indb()
{  	
	//file_num  = 0;
	source_file_num = 0;
	petri_status = 0;

	//split_num = 0;
	bak_flag = 'N';  //默认不备份
	record_num = 0;

    memset(m_szSrcGrpID,0,sizeof(m_szSrcGrpID));
	memset(m_szService,0,sizeof(m_szService));
	//memset(mServCatId,0,sizeof(mServCatId));
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

C_Indb::~C_Indb()
{
	if(m_enable) 
	{
		int ret = dr_ReleasePlatform();
		if(ret != 0)
		{
			char tmp[100] = {0};
			snprintf(tmp, sizeof(tmp), "释放容灾平台失败,返回值=%d", ret);
			theJSLog<<tmp<<endw;
		}
	}
	
}

//模块初始化动作
bool C_Indb::init(int argc,char** argv)
{
   
    if(!PS_Process::init(argc,argv))
    {
      return false;
    }
	
	if(!drInit())	return false;

	//获取petri网状态,当系统为只读态时,数据库更新操作语句写文件
	if(!(rtinfo.connect()))
	{
		return false;
	}
	rtinfo.getDBSysMode(petri_status);
	cout<<"petri status:"<<petri_status<<endl; 


	if(!(dbConnect(conn)))
	{
		cout<<"初始化数据库 connect error."<<endl;
		return false ;
	}

	int flow_id = getFlowID();
	int module_id = getModuleID(); 

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
			cout<<"请在tp_process表字段ext_param中配置service"<<endl;
			return false ;
		}

		sql = "select c.input_path from C_SOURCE_GROUP_DEFINE a,C_SERVICE_FLOW b,C_SERVICE_INTERFACE c where a.source_group=:1" 
                    "and a.workflow_id = b.workflow_id and b.service=:2 and b.input_id = c.interface_id ";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID<<m_szService;
		stmt.execute();
		if(!(stmt>>input_path))
		{
			cout<<"表中入库模块输入文件相对路径input_path没有配置"<<endl;
			return false ;
		}
		completeDir(input_path);
	
		sql = "select var_value from c_process_env where varname = 'INDB_FILE_BAK_FLAG' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID<<m_szService;
		stmt.execute();
		if(!(stmt>>bak_flag))
		{
			bak_flag = 'N';			 
		}
		if(bak_flag == 'Y')
		{
			sql = "select var_value from c_process_env where varname = 'INDB_FILE_BAK_DIR' and source_group=:1 and service=:2 ";
			stmt.setSQLString(sql);
			stmt<<m_szSrcGrpID<<m_szService;
			stmt.execute();
			if(!(stmt>>bak_path))
			{
					cout<<"请在表c_process_env中配置加载模块的备份目录,INDB_FILE_BAK_DIR"<<endl;
					return false;
			}
			completeDir(bak_path);
		}
		
		sql = "select var_value from c_process_env where varname = 'INDB_FILE_ERR_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID<<m_szService;
		stmt.execute();
		if(!(stmt>>erro_path))
		{
				cout<<"请在表c_process_env中配置加载模块的错误路径,INDB_FILE_ERR_DIR"<<endl;
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
			return false;
	}
	
	theJSLog.setLog(szLogPath,szLogLevel,m_szService , m_szSrcGrpID, 001);
	
	theJSLog<<"  相对输入路径:"<<input_path<<"  错误路径:"<<erro_path<<"	日志路径:"<<szLogPath<<" 日志级别:"
			<<szLogLevel<<"	每个数据源扫描文件个数:"<<source_file_num<<endi;
	
	if(bak_flag == 'Y')
	{
		theJSLog<<"文件备份路径:"<<bak_path<<endi;
	}

    theJSLog<<"加载数据源配置信息LoadSourceCfg..."<<endi;

	if(LoadSourceCfg() == -1) return false ;  //加载数据源配置信息
	
	conn.close();


	char input_dir[512],bak_dir[512],erro_dir[512];
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
			strcat(erro_dir,erro_path);
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

				if(getSourceFilter(SourceCfg.szSourceId,SourceCfg.filterRule,SourceCfg.file_begin,SourceCfg.file_length))
				{
							return -1;
				}
				
				m_SourceCfg[strSourceId]=SourceCfg;
		     }
		}

		
		//加载每个数据源对应的统计入库表
		string source_id;
		int config_id = 0;
		sql = "select source_id, var_value from c_source_env a  where a.varname = 'INS_TABLE_CONFIGID' and a.service =:1";
		stmt.setSQLString(sql);
		stmt<<m_szService;
		stmt.execute();
		while(stmt>>source_id>>config_id)
		{
				mapConfig.insert(map<string,int>::value_type(source_id,config_id));
		}
		stmt.close();
		
		for(map<string,SOURCECFG>::const_iterator iter = m_SourceCfg.begin();iter !=m_SourceCfg.end(); ++iter)
		{
			map<string,int>::const_iterator iter2 = mapConfig.find(iter->first);
			if(iter2 == mapConfig.end())
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"表source_env中数据源组[%s]配置的数据源[%s]没有配置入库的表项 INS_TABLE_CONFIGID",m_szSrcGrpID,iter->first);
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);

				return -1;
			}
			
			CF_CError_Table tab ;
			tab.Init(iter2->second);
			mapTabConf.insert(map< string,CF_CError_Table>::value_type(iter->first,tab));
			
			theJSLog<<"source_id = "<<iter->first<<" config_id = "<<iter2->second<<endi;
		}

		//判断是否和配置config_id个数相同，以免数据源找不到对应的表
		//if(m_SourceCfg.size() > mapConfig.size())
		//{
		//	memset(erro_msg,0,sizeof(erro_msg));
		//	sprintf(erro_msg,"表source_env中数据源组[%s]配置的INS_TABLE_CONFIGID的某些数据源没有配置入库的表项 config_id",m_szSrcGrpID);
		//	theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
		//	return -1;
		//}	
		//cout<<"iSourceCount = "<<iSourceCount<<" mapConfig.size()="<<mapConfig.size()<<endl;

		//for(map<string,int>::const_iterator iter = mapConfig.begin();iter != mapConfig.end();++iter)
		//{
		//	CF_CError_Table tab ;
		//	tab.Init(iter->second);
		//	mapTabConf.insert(map< string,CF_CError_Table>::value_type(iter->first,tab));
		//}

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

//循环扫描各个数据源
void C_Indb::run()
{	
	int ret = 0;	
	char dir[512],inputFilePath[512],filter[256],szFiletypeIn[10];	
	
	if(gbExitSig)
	{
		theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "应用程序收到退出信号");
		PS_Process::prcExit();
		return;
	}

	//判断数据库状态
	rtinfo.getDBSysMode(petri_status);
	if(petri_status == DB_STATUS_OFFLINE)	return ;
	
	if(!(dbConnect(conn)))
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() 连接数据库失败 connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
		return  ;
	}

	if( drStatus == 1 )  //备系统
	{	
			//检查trigger触发文件是否存在
			if(!CheckTriggerFile())
			{
				sleep(1);
				return ;
			}

			//获取同步变量
			memset(m_SerialString,0,sizeof(m_SerialString));
			ret = drVarGetSet(m_SerialString);
			if(ret)
			{
				theJSLog<<"同步失败..."<<endw;
				return ;
			}
	
			//获取同步变量
			vector<string> data;		
			splitString(m_SerialString,";",data,false,false);  //发送的字符串数据源ID,文件名,sqlFile文件名
			
			it = m_SourceCfg.find(data[0]);
			if(it == m_SourceCfg.end())			//考虑是否仲裁
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"没有找到该数据源信息[%s]",data[0]);		//环境变量未设置
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				
				dr_AbortIDX();  
				return ;
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
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"主系统传过来的文件[%s]不存在",fileName);
					theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);
					
					dr_AbortIDX();  
					return ;
				}		
			}
			else if(iStatus == 2)		//跟随模式,备系统
			{
				while(1)
				{
					//设置中断
					if(gbExitSig)
					{
						dr_AbortIDX();

						theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "应用程序收到退出信号");
						PS_Process::prcExit();
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
			else
			{
				char tmp[50] = {0};
				snprintf(tmp, sizeof(tmp), "容灾平台dr_GetAuditMode函数配置错误，返回值[%d]", iStatus);
				theJSLog<<tmp<<endw;
				return ;
			}
			
			theJSLog<<"查找到文件："<<fileName<<endi;

			memset(m_szSourceID,0,sizeof(m_szSourceID));
			memset(m_szFileName,0,sizeof(m_szFileName));
			memset(mServCatId,0,sizeof(mServCatId));

			strcpy(m_szFileName,data[1].c_str());
			strcpy(m_szSourceID,it->first.c_str());
			strcpy(mServCatId,it->second.serverCatID);
			
			//setSQLFileName(data[2].c_str());

			ret = dealFile();
			if(ret)
			{
				dr_AbortIDX();
			}
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
		strcat(inputFilePath,input_path); 

		memset(filter,0,sizeof(filter));
		memset(szFiletypeIn,0,sizeof(szFiletypeIn));  
		strcpy(filter,it->second.filterRule);		  //过滤条件
		strcpy(szFiletypeIn,it->second.szInFileFmt);  //当前数据源的输入格式
		strcpy(m_szSourceID,it->first.c_str());
					
		//打开话单文件目录
		if(scan.openDir(inputFilePath))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"打开话单文件目录[%s]失败",inputFilePath);
			theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
			it++;
			return ;	
		}		
						
		//循环读取目录，扫描文件夹，获取文件  因为存在临时文件，所以会扫描10次
		int rett = -1 ;
		char tmp[512];
		file_num = 0;
		while(1)		
		{		
				if(file_num == source_file_num)
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
	
				theJSLog<<"扫描到文件："<<fileName<<endi;

				strcpy(m_szFileName,p+1);

				//发送同步信息
				memset(m_SerialString,0,sizeof(m_SerialString));
				sprintf(m_SerialString,"%s;%s",m_szSourceID,m_szFileName);
				ret = drVarGetSet(m_SerialString);
				if(ret)
				{
					theJSLog<<"同步失败...."<<endw;
					return ;
				}
				
				//setSQLFileName(m_szFileName);
				
				memset(mServCatId,0,sizeof(mServCatId));
				strcpy(mServCatId,it->second.serverCatID);

				ret = dealFile();
				if(ret)
				{
					dr_AbortIDX();
				}
		}

		scan.closeDir();
		it++;
	}

	conn.close();
}


//将文件入库 ret = 0表示入库成功 ret= -1表示入库失败
int C_Indb::dealFile()
{	
	int ret = 0;
	char szBuff[1024];
	
	//测试***********************：
	//strcpy(m_szSourceID,"HD");
	//strcpy(m_szOutTypeId,"SJSTD");
	//strcpy(m_szFileName,"HDC.2013.201307181150");
	//file = "/mboss/jtcbs/zbjs1_a/data/service/HED/HD/end_path/HDC.2013.201307181150";

	//if(!(dbConnect(conn)))
	//{
	//	cout<<"连接数据库失败 connect error."<<endl;
	//	return -1 ;
	//}
	
	memset(m_AuditMsg,0,sizeof(m_AuditMsg));
	
	theJSLog<<"文件["<<m_szFileName<<"]入库"<<endi;

	map< string,CF_CError_Table>::const_iterator iter =  mapTabConf.find(m_szSourceID);
	if( iter == mapTabConf.end())
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"indb() 数据库中没有数据源%s的配置信息",m_szSourceID);
		theJSLog.writeLog(LOG_CODE_SOURCE_NOT_FIND,erro_msg);	
		return -1;
	}

	record_num = 0;
	Statement stmt;
	try
	{
		ifstream in(fileName,ios::nocreate);
		if(!in)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"dealFile() 文件[%s]打开出错",fileName);
			theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败
			return -1;
		}

		stmt = conn.createStatement();

		CF_CError_Table tab = iter->second;
		tab.setFileName(m_szFileName,m_szSourceID,stmt);  
		
		memset(szBuff,0,sizeof(szBuff));
	
		while(in.getline(szBuff,sizeof(szBuff)))   
		{	
			theJSLog<<"record["<<szBuff<<"]"<<endi;
			outrcd.Set_record(szBuff);
			tab.dealInsertRec(outrcd,NULL,NULL,NULL);
			record_num++;
			memset(szBuff,0,sizeof(szBuff));		
		}	

		tab.commit();			//防止没达到记录条数插入条件，强制插入		
		in.close();
		
		//2013-10-01  新增获取file_id 和file_time
		strncpy(file_time,m_szFileName+it->second.file_begin,it->second.file_length);
		file_time[8] = '\0';
	
		char fileid[10];
		memset(fileid,0,sizeof(fileid));
		outrcd.Get_Field(FILE_ID,fileid);
		file_id = atol(fileid);

		//**************做仲裁,数据源,文件名,单个话单块能存放的记录条数,记录条数
		char state = 'Y',tmp[512];
		sprintf(m_AuditMsg,"%s;%s;%d",m_szSourceID,m_szFileName,record_num);
		if(!IsAuditSuccess(m_AuditMsg))
		{
			stmt.rollback();
			//rollBackSQL();
			//state = 'E';
			memset(tmp,0,sizeof(tmp));								//文件移到错误目录
			strcpy(tmp,it->second.szSourcePath);
			strcat(tmp,erro_path);
			strcat(tmp,m_szFileName);
			if(rename(fileName,tmp))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"移动文件[%s]到错误目录失败: %s",fileName,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
			}
			
			return ;
		 }
		
		//在这儿需要判断数据库状态吗,有可能在这儿数据库切换为只读了
		//获取数据库状态	
		while(1)
		{
			rtinfo.getDBSysMode(petri_status);
			if(petri_status == DB_STATUS_OFFLINE)
			{
				theJSLog<<"数据库状态为只读态,等待状态改变..."<<endw;
				sleep(600);
				continue;
			}

			break;
		}
		
		//theJSLog<<"更新调度表d_sch_indb,文件处理完成"<<endi;

		//更新注册调度表
		getCurTime(currTime); 
		memset(sql,0,sizeof(sql));	
		sprintf(sql,"insert into d_sch_indb(source_id,serv_cat_id,filename,deal_flag,dealtime,mainflow_count,file_id,file_time) values('%s','%s','%s','%c','%s',%d,%ld,'%s')",m_szSourceID,mServCatId,m_szFileName,state,currTime,record_num,file_id,file_time);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt.close();

		//if(state == 'E') return -1;		//文件已经挪到失败目录去了

		//将文件备份
		if(bak_flag == 'Y')
		{		
			//判断是否需要备份										
			char bak_dir[512];
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
	}
	catch (jsexcp::CException e)
	{
		stmt.rollback();
		
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"%s indb() error line=(%d) :%s ",m_szFileName,record_num,e.GetErrMessage());
		theJSLog.writeLog(0,erro_msg);
		
		char tmp[512];
		memset(tmp,0,sizeof(tmp));								//文件移到错误目录
		strcpy(tmp,it->second.szSourcePath);
		strcat(tmp,erro_path);
		strcat(tmp,m_szFileName);
		rename(fileName,tmp);

		return -11 ;
	}
	catch(SQLException e)
	{
		stmt.rollback();

		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"%s indb() sql error line=(%d) :%s (%s)",m_szFileName,record_num,e.what(),szBuff);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);
    	
		char tmp[512];
		memset(tmp,0,sizeof(tmp));								//文件移到错误目录
		strcpy(tmp,it->second.szSourcePath);
		strcat(tmp,erro_path);
		strcat(tmp,m_szFileName);
		rename(fileName,tmp);

    	return -11 ;
    }

	return ret;
}

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
			theJSLog << "容灾平台初始化失败,返回值=" << ret<<endw;
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
			theJSLog<<"获取容灾平台状态出错: 返回值="<<drStatus<<endw;
			return false;
		}
		
		if(drStatus == 0)		theJSLog<<"当前系统配置为主系统"<<endi;
		else if(drStatus == 1)	theJSLog<<"当前系统配置为备系统"<<endi;
		else if(drStatus == 2)	theJSLog<<"当前系统配置非容灾系统"<<endi;

		return true;
}

//主系统发送同步变量,备系统获取同步变量
int C_Indb::drVarGetSet(char* serialString)
{
		int ret ;
		char tmpVar[5000] = {0};

		//检查容灾平台的切换锁
		ret = dr_CheckSwitchLock();   
		if(ret != 0)  
		{  
			theJSLog<<"检查容灾切换锁失败,返回值="<<ret<<endw;
			return -1;  
		} 
		//初始化index  
		ret = dr_InitIDX();  
		if(ret != 0)  
		{  
			theJSLog<<"初始化index失败,返回值=" <<ret<<endw;
			dr_AbortIDX();
			return -1;  
		}
/*		
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

*/
		snprintf(tmpVar, sizeof(tmpVar), "%s", serialString);
		//主系统把要同步的index “键值对”写入容灾平台维护的index 文件中
		//备系统调用该函数的结果是，var获得和主系统一样的随机变量的值。	SYNC_SINGLE表示注册单一的随机变量
		ret = dr_SyncIdxVar("serialString", tmpVar, SYNC_SINGLE);		
		if (ret != 0)
		{
			theJSLog<<"传序列串时失败,序列名:["<<serialString<<"]"<<endw;
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
			theJSLog<<"传输实例名失败："<<tmpVar<<endw;
			dr_AbortIDX();  
			return -1;
		}
		
		
		// <6> 预提交index  此关键字用于将平台当前内存中的随机变量写入磁盘
		ret = dr_SyncIdxVar("@@FLUSH","SUCCESS",SYNC_SINGLE);  
		if (ret != 0 )
		{
			theJSLog<<"预提交index失败"<<endw;
			dr_AbortIDX();
			return -1;
		}
		
		
		// <7> 提交index  	提交Index。在index文件中设置完成标志
		ret = dr_CommitIDX();  
		if(ret != 0)  
		{  
			theJSLog<<"提交index失败,返回值="<<ret<<endw;
			dr_AbortIDX();  
			return -1;  
		}

		//备系统搜索目录
		//if(!m_syncDr.isMaster())thelog<<"备系统SerialString："<<m_SerialString<<endi;

		theJSLog<<"本次的同步串serialString:"<<serialString<<endi;//for test

		return ret;

}

//仲裁字符串
 bool C_Indb::IsAuditSuccess(const char* dealresult)
 {
		int auitStatus = 0, ret = 0;
		ret = dr_Audit(dealresult);
		if(2 == ret )
		{
			theJSLog << "容灾仲裁失败,结果:" << ret <<"本端："<<dealresult<< endw;
			dr_AbortIDX();
			return false;
		}
		else if (3 == ret)
		{
			theJSLog<<"容灾仲裁超时..."<<endw;
			dr_AbortIDX();
			return false;
		}
		else if(4 == ret)
		{
			theJSLog<<"对端idx异常终止..."<<endw;
			dr_AbortIDX();
			return false;
		}
		else if(1 == ret)
		{
			ret = dr_CommitSuccess();
			if(ret != 0)
			{
				theJSLog << "业务全部提交失败(容灾平台)" << endw;
				dr_AbortIDX();
				return false;
			}
			theJSLog<<"ret = "<<ret<<"仲裁成功...\n仲裁内容："<<dealresult<<endi;
			return true;
		}
		else
		{
			theJSLog<<"未知ret="<<ret<<"	仲裁内容："<<dealresult<<endw;
			dr_AbortIDX();
			return false;
		}
	
	return true;
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

int main(int argc,char** argv)
{
	cout<<"********************************************** "<<endl;
	cout<<"*    GuangDong Telecom. Telephone Network      "<<endl;
	cout<<"*       Centralized Settlement System          "<<endl;
	cout<<"*                                              "<<endl;
	cout<<"*            jsindb		                      "<<endl;
	cout<<"*              Version 1.0	                  "<<endl;
	cout<<"*     created time :      2013-09-01 by  hed	  "<<endl;
	cout<<"*     last updaye time :  2013-09-26 by  hed	  "<<endl;
	cout<<"********************************************** "<<endl;


	C_Indb fm ;

	if(!fm.init(argc,argv)) return false;
	
	while(1)
	{
		theJSLog.reSetLog();
		fm.run();
		sleep(5);
	}

   return 0;
}








