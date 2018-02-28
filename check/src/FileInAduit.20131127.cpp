/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-07-22
File:			 FileInAduit.cpp
Description:	 文件核对输出模块
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
#include <sys/stat.h>  //stat()函数，查询文件信息
#include<unistd.h>     //读取当前程序运行目录
#include<iostream>
#include<fstream>
#include "FileInAduit.h"

#include "CF_Common.h"
#include "CF_CLogger.h"
CLog theJSLog;

SGW_RTInfo	rtinfo;

int day_or_month = 1;		//核对失败次数

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
	
	drStatus = 2;
	petri_status = 0;
	m_enable = false;
}

FileInAduit::~FileInAduit()
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
bool FileInAduit::init(int argc,char** argv)
{ 
    if(!PS_Process::init(argc,argv))
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
			m_enable = false;
			break;
		}	
	}
	
	if(flag)
	{
		if(!drInit())  return false;
	}
	
	if(!(rtinfo.connect()))
	{
		cout<<"连接运行时内存区失败"<<endl;
		return false;
	}
	rtinfo.getDBSysMode(petri_status);
	petri_status_tmp = petri_status;
	cout<<"petri status:"<<petri_status<<endl;

	 char sParamName[256],bak_path[512];
	 CString sKeyVal;
	 
	 sprintf(sParamName, "file.check.receive_path");			//源文件接收目录
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(receive_path,0,sizeof(receive_path));
		strcpy(receive_path,(const char*)sKeyVal);
		completeDir(receive_path);
	 }
	 else
	 {	
		cout<<"请在核心参数里配置源文件接收路径[file.check.receive_path]"<<endl;
		return false ;
	 }	 
	 
	 sprintf(sParamName, "file.check.output_path");			  //源文件输出目录
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(output_path,0,sizeof(output_path));
		strcpy(output_path,(const char*)sKeyVal);
		completeDir(output_path);
	 }
	 else
	 {	
		cout<<"请在核心参数里配置源文件的输出路径[file.check.output_path]"<<endl;
		return false ;
	 }	

	 sprintf(sParamName, "file.check.input_path");			//核对文件输入目录
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(input_path,0,sizeof(input_path));
		strcpy(input_path,(const char*)sKeyVal);

	 }
	 else
	 {	
		cout<<"请在核心参数里配置核对文件的输入路径[file.check.input_path]"<<endl;
		return false ;
	 }	 

	 sprintf(sParamName, "file.check.month_input_path");			//月汇总文件输入目录
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(month_input_path,0,sizeof(month_input_path));
		strcpy(month_input_path,(const char*)sKeyVal);
	 }
	 else
	 {	
		cout<<"请在核心参数里配置月核对文件的输入路径[file.check.month_input_path]"<<endl;
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
		cout<<"请在核心参数里配置核对模块的备份路径[file.check.bak_path]"<<endl;
		return false ;
	 }	 	
	
	 sprintf(sParamName, "file.check.fail_path");			//核对文件输入目录
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(fail_path,0,sizeof(fail_path));
		strcpy(fail_path,(const char*)sKeyVal);
	 }
	 else
	 {	
		cout<<"请在核心参数里配置核对文件的核对失败存放路径[file.check.fail_path]"<<endl;
		return false ;
	 }	 

	//判断目录是否存在
	 DIR *dirptr = NULL; 
	
	 if((dirptr=opendir(input_path)) == NULL)
	 {
	 	cout<<"核对文件输入目录["<<input_path<<"]打开失败"<<endl;
	 	return false ;

	 }else closedir(dirptr);
	 completeDir(input_path);
	

	 if((dirptr=opendir(month_input_path)) == NULL)
	 {
	 	cout<<"月汇总文件输入目录["<<month_input_path<<"]打开失败"<<endl;
	 	return false ;

	 }else closedir(dirptr);
	 completeDir(month_input_path);

	 if((dirptr=opendir(bak_path)) == NULL)
	 {
	 	cout<<"备份目录["<<bak_path<<"]打开失败"<<endl;
	 	return false ;

	 }else closedir(dirptr);
	  completeDir(bak_path);

	 if((dirptr=opendir(fail_path)) == NULL)
	 {
	 	cout<<"核对失败目录["<<fail_path<<"]打开失败"<<endl;
	 	return false ;

	 }else closedir(dirptr);
	  completeDir(fail_path);
	 
	 //备份目录下面应新建两个目录来区分核对文件和月汇总文件
	 strcpy(bak_path1,bak_path);
	 strcat(bak_path1,"AUD");
	 if(chkDir(bak_path1))
	 {
			cout<<"备份目录["<<bak_path1<<"]打开失败"<<endl;
			return false;
	 }
	 completeDir(bak_path1);

	 strcpy(bak_path2,bak_path);
	 strcat(bak_path2,"SUM");
	 if(chkDir(bak_path2))
	 {
			cout<<"备份目录["<<bak_path2<<"]打开失败"<<endl;
			return false ;
	 }
	 completeDir(bak_path2);


	//初始化内存日志接口
	bool bb = initializeLog(argc,argv,false);  //是否调试模式
	if(!bb)
	{
			return false;
	}
	
	theJSLog.setLog(szLogPath, szLogLevel,"CHECK","GJJS", 001);	//文件日志接口，调用了内存日志接口
	
	theJSLog<<"	日志路径:"<<szLogPath<<" 日志级别:"<<"核对文件输入目录:"<<input_path<<"月汇总文件输入目录:"<<month_input_path
		    <<" 源文件接收目录:"<<receive_path<<" 源文件输出目录:"<<output_path <<"  备份目录:"<<bak_path<<" 核对失败文件存放目录"<<fail_path<<" sql文件目录:"<<sql_path<<endi;
   	
		
	//查询月核对文件的配置信息
	try
	{
		if(!(dbConnect(conn)))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"init() 连接数据库失败 connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
			return  false;
		}
		
		if(LoadSourceCfg() == -1)	    //加载数据源配置信息 2013-08-07
		{
			return false;
		}

		//判断输入输出的路径是否存在
		char input_dir[512],output_dir[512];
		int rett = -1;
		for(map<string,SOURCECFG>::const_iterator iter = m_SourceCfg.begin();iter !=m_SourceCfg.end(); ++iter)
		{		   		
			memset(input_dir,0,sizeof(input_dir));
			strcpy(input_dir,iter->second.szSourcePath);
			strcat(input_dir,receive_path);
			if((dirptr=opendir(input_dir)) == NULL)
			{
					//memset(erro_msg,0,sizeof(erro_msg));
					//sprintf(erro_msg,"数据源[%s]的接收文件路径[%s]不存在",iter->first,input_dir);
					//theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错

					theJSLog<<"数据源【"<<iter->first<<"】的接收文件路径: "<<input_dir<<"不存在，自行创建"<<endw;
					rett = mkdir(input_dir,0755);
					if(rett == -1)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"数据源[%s]的接收文件文件路径[%s]不存在，自行创建失败",iter->first,input_dir);
						theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错

						return false;
					}

			}else closedir(dirptr);

			memset(output_dir,0,sizeof(output_dir));
			strcpy(output_dir,iter->second.szSourcePath);
			strcat(output_dir,output_path);
			if((dirptr=opendir(output_dir)) == NULL)
			{
					//memset(erro_msg,0,sizeof(erro_msg));
					//sprintf(erro_msg,"数据源[%s]的输出文件路径[%s]不存在",iter->first,output_dir);
					//theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
					
					theJSLog<<"数据源【"<<iter->first<<"】的输出文件路径: "<<output_dir<<"不存在，自行创建"<<endw;
					rett = mkdir(output_dir,0755);
					if(rett == -1)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"数据源[%s]的输出文件文件路径[%s]不存在，自行创建失败",iter->first,output_dir);
						theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错

						return false;
					}

			}else closedir(dirptr);
		}


		char fee[256];
		memset(fee,0,sizeof(fee));
		Check_Sum_Conf fmt;

		theJSLog<<"加载月汇总文件配置信息..."<<endi;
		string sql = "select check_type,sum_table,cdr_count,cdr_duration,cdr_fee,rate_cycle from c_check_file_config ";
		Statement stmt = conn.createStatement();
		stmt.setSQLString(sql);
		stmt.execute();
		while(stmt>>fmt.check_type>>fmt.sum_table>>fmt.cdr_count>>fmt.cdr_duration>>fee>>fmt.rate_cycle)
		{
			//cout<<"check_type="<<fmt.check_type<<"   cdr_duration="<<fmt.cdr_duration<<endl;
			
			splitString(fee,",",fmt.cdr_fee,"true");
			monthSumMap.insert( map< string,Check_Sum_Conf >::value_type(fmt.check_type,fmt));

			memset(fee,0,sizeof(fee));
			//fmt.cdr_fee.clear();
			fmt.clear();
		}
		
		stmt.close();

		conn.close();
	}
	catch(SQLException  e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init 初始化时数据库查询异常:%s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
		return false ;
	}
  
   theJSLog<<"初始化完毕..."<<endi;

   return true ;
}

//加载数据源配置信息，取全部数据源组的数据源信息
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

			theJSLog<<"数据源组："<<m_szSrcGrpID<<"  iSourceCount="<<iSourceCount<<endi;
		
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
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"LoadSourceCfg err：%s (%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		return -1;
	}
	catch (jsexcp::CException &e) 
	{	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"LoadSourceCfg() err %s ",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);
		return -1;
	}

	return 0;
}

/******根据数据源获取过滤规则 0没有查到规则，1查到规则了 增加获取文件名上时间的起始位置,和长度*********************/
int FileInAduit::getSourceFilter(char* source,char* filter,int &index,int &length)
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
				sprintf(erro_msg,"数据源[%s]没有配置过滤规则或者文件名时间截取规则",source);
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return -1;
		}
		
		char tmp[6];
		memset(tmp,0,sizeof(tmp));
		strcpy(tmp,file_time.c_str());
		
		vector<string> fileTime;
		//cout<<"tmp="<<tmp<<endl;
		splitString(tmp,",",fileTime,false);	//不能跳过重复的字符串: 8,8 这样就会有问题(只保留了8)
		if(fileTime.size() != 2) 
		{
			stmt.close();
			sprintf(erro_msg,"数据源[%s]文件名时间截取规则配置规则错误:%s  [如3,8]",source,file_time);
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
			return -1;
		}
		
		index = atoi(fileTime[0].c_str());
		length = atoi(fileTime[1].c_str());
		
		//cout<<"source = "<<source<<"  tmp = "<<tmp<<" index = "<<index<<"  length = "<<length<<" file_time = "<<file_time<<endl;
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
		sprintf(erro_msg,"getSourceFilter err: %s(%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);
		return -1 ;
	}
	catch(jsexcp::CException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"getSourceFilter err：%s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);
		return -1;
	}
	
	return 0;
}

//检查输入的文件名是否在文件接收路径下面找到,先通过文件名找到所属数据源 0不存在，1 存在
int FileInAduit::check_file_exist(char* file)
{			
	int ret = 0,flag = 0;
	CF_CFscan scan2;
	char tmp[512],tmp2[512], *p,receive_dir[512];  //out_path[1024],receive_dir[1024],filter[256];
	string source_id;

	memset(file_time,0,sizeof(file_time));

	//查询文件所属数据源ID,再到目录下面去查找原始文件
	for(map<string,SOURCECFG>::const_iterator it = m_SourceCfg.begin();it != m_SourceCfg.end();++it)
	{
			if(checkFormat(file,it->second.filterRule))		//HDC.2013---    HD*
			{		
					//theJSLog<<"文件["<<file<<"]所属数据源:"<<it->first<<endi;
					flag = 1;
					source_id = it->first;
					//memset(out_path,0,sizeof(out_path));
					//strcpy(out_path,it->second.szSourcePath);
					memset(m_szSourceID,0,sizeof(m_szSourceID));
					strcpy(m_szSourceID,source_id.c_str());

					memset(receive_dir,0,sizeof(receive_dir));				
					strcpy(receive_dir,it->second.szSourcePath);
					strncpy(file_time,file+it->second.file_begin,it->second.file_length);
					file_time[8] = '\0';
					break;
			}
	}
	
	if(flag == 0)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"找不到文件[%s]所属数据源",file);
		theJSLog.writeLog(LOG_CODE_SOURCE_NOT_FIND,erro_msg);
		return 0;
	}

	strcat(receive_dir,receive_path);
	flag = scan2.openDir(receive_dir);		//查询文件所在路径 flag = 0
	if(flag)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"打开文件接收目录[%s]失败",receive_dir);
		theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg);		//打开目录出错
		return 0;
	}
	//cout<<"扫描接收目录"<<receive_dir<<endl;
	flag = 1;  
	memset(tmp,0,sizeof(tmp));		
	while(true)
	{
		ret = scan2.getFile("*",tmp);			//找文件	
		if(ret == 0)
		{
			p = strrchr(tmp,'/');
			memset(tmp2,0,sizeof(tmp2));
			strcpy(tmp2,p+1);

			//cout<<"扫到文件["<<tmp2<<"]  对比文件["<<file<<"]  :"<<ret<<endl;

			if(strcmp(tmp2,file) == 0)
			{							
				mapFileSource.insert(map<string,string>::value_type(file,source_id));

				break ;
			}
		}
		else if(ret == 100)			//文件扫描完
		{	
			 flag = 0;
			 break ;	
		}
		else if(ret == -1) 			//获取文件信息失败
		{
			 flag = 0;
			 break;
		}
	}
				
	scan2.closeDir();	
	
	return flag;
}

void FileInAduit::execute()
{
	int ret = 0,event_sn, event_type;
	long param1, param2, src_id;

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

		run();		//日核对文件
		run2();

		sleep(30);
	}

}

//扫描输入文件路径，查找上游发过来的核对文件，查询其中的文件名是否都在调度表中有登记，并记录核对信息
void FileInAduit::run(int flag)
{
	int ret = -1,day_or_month = 1;

	try
	{	
		short db_status = 0;
		rtinfo.getDBSysMode(db_status);

		if( drStatus == 1 )  //备系统
		{
			if(db_status != petri_status_tmp)
			{
				theJSLog<<"数据库状态切换... "<<petri_status_tmp<<"->"<<db_status<<endw;
				int cmd_sn = 0;
				if( !putEvt(cmd_sn, EVT_RPT_DBSTATUS, 0, db_status, DSPCH_PRC_ID) )
				{
					theJSLog<<"报告数据库更换状态失败！\n"<<endw;
					return;
				}
				petri_status_tmp = db_status;
			}

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
			splitString(m_SerialString,";",data,false,false);  //发送的字符串文件名,sqlFile文件名
			
			memset(fileName,0,sizeof(fileName));
			memset(m_szFileName,0,sizeof(m_szFileName));
			strcpy(fileName,input_path);
			strcat(fileName,data[0].c_str());
			strcpy(m_szFileName,data[0].c_str());

			setSQLFileName(data[0].c_str());		//设置sqlFile文件名
			
			if(petri_status != atoi(data[1].c_str()))
			{
				theJSLog<<"主系统的数据库状态发生了切换..."<<endw;
			}
			petri_status = atoi(data[1].c_str());		//备系统的状态根据主系统来定

			//int dr_GetAuditMode()1表示同步，2表示跟随, 其它为失败，-1是配置错误，-2配置文件读取出现问题
			int iStatus = dr_GetAuditMode(module_name);

			if(iStatus == 1)		//同步模式,等待，主系统要么成功,要么失败,要么超时
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
					dr_AbortIDX();

					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"日核对文件[%s]不存在,dr_AbortIDX()",fileName);
					theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);
					return ;
				}	
				
			}
			else if(iStatus == 2)		//跟随模式,不等，主系统直接仲裁成功,单边模式必须为该模式
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
				}
			}
			else
			{
				char tmp[50] = {0};
				snprintf(tmp, sizeof(tmp), "容灾平台dr_GetAuditMode函数配置错误，返回值[%d]", iStatus);
				theJSLog<<tmp<<endi;
				return ;
			}
			
			theJSLog<<"######## start deal file "<<fileName<<" ########"<<endi;
			//theJSLog<<"查找到日核对文件："<<fileName<<endi;	

			if(iStatus == 1)		//同步模式,等待，主系统要么成功,要么失败,要么超时
			{	
				bool flag = false;
				int times = 1;
				while(times < 10)
				{
					if(checkFile())
					{
						//rollBackSQL();	//回滚数据库
						//fileList.clear();						
						//theJSLog<<"查找了"<<times<<"次文件"<<endi;
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
					dr_AbortIDX();

					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"核对文件[%s]中文件名不完全存在,本次核对失败,dr_AbortIDX() ",fileName);
					theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);
					
					//theJSLog<<"文件超时或者文件格式错误,将文件挪到失败目录"<<endi;
					char tmp[512];
					memset(tmp,0,sizeof(tmp));
					strcpy(tmp,fail_path);
					strcat(tmp,m_szFileName);
					//strcat(fail_path,m_szFileName);
					
					if(rename(fileName,tmp))
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"移动文件[%s]到错误目录失败: %s",m_szFileName,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
					}

					theJSLog<<"######## end deal file ########\n"<<endi;

					return ;
				}	
				
			}
			else if(iStatus == 2)		//跟随模式,不等，主系统直接仲裁成功
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

					ret = checkFile();
					if(ret == 0)  break;

					//rollBackSQL();	//回滚数据库
					//fileList.clear();
				}			
			}
			else
			{
				char tmp[50] = {0};
				snprintf(tmp, sizeof(tmp), "容灾平台dr_GetAuditMode函数配置错误，返回值[%d]", iStatus);
				theJSLog<<tmp<<endw;
				return ;
			}
	}
	else
	{
			if(db_status != petri_status)
			{
				theJSLog<<"数据库状态切换... "<<petri_status<<"->"<<db_status<<endw;
				int cmd_sn = 0;
				if( !putEvt(cmd_sn, EVT_RPT_DBSTATUS, 0, db_status, DSPCH_PRC_ID) )
				{
					theJSLog<<"报告数据库更换状态失败！\n"<<endw;
					return;
				}
				petri_status = db_status;
				petri_status_tmp = db_status;
			}

			//主系统,非容灾系统
			if(scan.openDir(input_path))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"打开文件目录[%s]失败",input_path);
				theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
				return ;		
			}
			
			char tmp[512];
			int rett = 0;
			while(1)		
			{
				memset(fileName,0,sizeof(fileName));
			
				rett = scan.getFile("*.AUD",fileName); 
			
				if(rett == 100)
				{		
						scan.closeDir();
						return ;
				}
				else if(rett == -1)
				{	
						scan.closeDir();
						return ;			//表示获取文件信息失败
				}
				
				/*过滤文件*.tmp,*.TMP,~* */			
				char* p = strrchr(fileName,'/');
				memset(tmp,0,sizeof(tmp));
				strcpy(tmp,p+1);
			
				if(tmp[0] == '~' )	continue ;
				//if(strlen(tmp) <= 3) continue;

				if(strlen(tmp) > 3)						//条件可考虑舍弃，前面扫描文件已过滤
				{		
						int pos = strlen(tmp)-4;
						//cout<<"后缀名为："<<tmp+pos<<endl;
						if((strcmp(tmp+pos,".tmp") && strcmp(tmp+pos,".TMP")) == 0) 
						{
							//cout<<"扫描到临时文件，舍弃"<<fileName<<endl;
							continue;
						}
				}
				
				theJSLog<<"######## start deal file "<<fileName<<" ########"<<endi;
				//theJSLog<<"扫描到日核对文件："<<fileName<<endi;	

				strcpy(m_szFileName,p+1);		
				setSQLFileName(m_szFileName);

				break;
			}
		
		 scan.closeDir();

		 int cnt = 0;
		//先将文件中文件查找出来,当全部查找成功过后,才发同步信息给备系统
		while(cnt < 10)
		{	
				if(gbExitSig)
				{
						theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "应用程序收到退出信号");
						prcExit();
						return;
				}

				cnt++;
				ret = checkFile();
				if(ret == 0)
				{
					break;
				}
				else if(ret == -1)
				{
					break;
				}
								
				theJSLog<<"核对文件失败个数:"<<ret<<" 核对次数:"<<cnt<<endw;

				//rollBackSQL();	//函数里清空
				//fileList.clear();
				sleep(60);
		}

		if(cnt == 10 ||ret == -1)		//文件超时或者文件格式错误等
		{
			//theJSLog<<"文件超时或者文件格式错误,将文件挪到失败目录:"<<ret<<endw;
			if(ret > 0)
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"本次核对文件核对失败,失败个数:(%d)",ret);
				theJSLog.writeLog(LOG_CODE_FILE_ERR_CHECK,erro_msg);
			}

			char tmp[512];
			memset(tmp,0,sizeof(tmp));
			strcpy(tmp,fail_path);
			strcat(tmp,m_szFileName);
			
			theJSLog<<"将文件移到错误目录:"<<fail_path<<endi;
			if(rename(fileName,tmp))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"移动文件[%s]到错误目录失败: %s",m_szFileName,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
			}
			
			theJSLog<<"######## end deal file ########\n"<<endi;

			return;  
		}

		memset(m_SerialString,0,sizeof(m_SerialString));		//文件全部找齐了
		sprintf(m_SerialString,"%s;%d",m_szFileName,petri_status);
		ret = drVarGetSet(m_SerialString);
		if(ret)
		{
			theJSLog<<"同步失败...."<<endw;

			rollBackSQL();	
			fileList.clear();
			mapFileSource.clear();

			return ;
		}
	}
				
		//仲裁......................................
		memset(m_AuditMsg,0,sizeof(m_AuditMsg));
		//for(int i = 0;i<fileList.size();i++)
		//{
		//	sprintf(m_AuditMsg,"%s%s|",m_AuditMsg,fileList[i].fileName);
		//}
		sprintf(m_AuditMsg,"%d",fileList.size());	//2013-10-20

		fileList.clear();		//清空文件列表
		
		theJSLog<<"wait dr_audit() ..."<<endi;
		ret = IsAuditSuccess(m_AuditMsg);
		if(ret)										//回滚数据库,将文件移到错误目录
		{
			rollBackSQL();	//回滚数据库
			
			if(ret != 3)						//2013-11-07 仲裁超时不移动文件
			{
				theJSLog<<"将文件挪到失败目录:"<<fail_path<<endi;
				char tmp[512];
				memset(tmp,0,sizeof(tmp));
				strcpy(tmp,fail_path);
				strcat(tmp,m_szFileName);
				if(rename(fileName,tmp))
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"移动文件[%s]失败: %s",m_szFileName,strerror(errno));
					theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
				}	
			}
			
			mapFileSource.clear();
			
			theJSLog<<"######## end deal file ########\n"<<endi;

			return;
		}
				
		//仲裁成功,提交数据库,将成功的文件移到输出目录,格式化目录
		theJSLog<<"提交sql..."<<endi;	
		if(petri_status == DB_STATUS_OFFLINE)
		{
			theJSLog<<"数据库状态为只读态,写sql文件"<<endi;
			commitSQLFile();
		}
		else
		{
			if(!(dbConnect(conn)))
			{
				commitSQLFile();

				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"run() 连接数据库失败,暂时写sql文件");
				//theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
				theJSLog<<erro_msg<<endw;
			}
			else
			{
				m_vsql = getvSQL();
				updateDB();
				conn.close();
			}	
 		}		

		char out_dir[512],receive_dir[512];
		map<string,SOURCECFG>::const_iterator it;

		theJSLog<<"将接收文件从接收目录["<<receive_path<<"]移到格式化目录"<<output_path<<endi;
		for(map<string,string>::const_iterator iter = mapFileSource.begin();iter != mapFileSource.end();++iter)
		{
				memset(out_dir,0,sizeof(out_dir));
				memset(receive_dir,0,sizeof(receive_dir));

				it = m_SourceCfg.find(iter->second);
				
				strcpy(out_dir,it->second.szSourcePath);
				strcpy(receive_dir,out_dir);

				strcat(out_dir,output_path);
				strcat(receive_dir,receive_path);
				
				strcat(receive_dir,iter->first.c_str());
				strcat(out_dir,iter->first.c_str());
				if(rename(receive_dir,out_dir))
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"移动文件[%s]失败: %s",iter->first,strerror(errno));
					theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
				}
		}
		
		//清空核对成功的文件
		mapFileSource.clear();
		
		//将文件备份 按照日期来备份
		char bak_dir[512];
		memset(bak_dir,0,sizeof(bak_dir));
		strcpy(bak_dir,bak_path1);

		strncat(bak_dir,currTime,6);
		completeDir(bak_dir);
		strncat(bak_dir,currTime+6,2);
		completeDir(bak_dir);

		theJSLog<<"日核对文件"<<m_szFileName<<"备份到"<<bak_dir<<endi;

		if(chkAllDir(bak_dir) == 0)			//备份失败,将程序写改为临时文件,放在当前目录,需要手工干预
		{
			strcat(bak_dir,m_szFileName);
			if(rename(fileName,bak_dir))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"文件[%s]移动[%s]失败: %s",m_szFileName,bak_dir,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);		//移动文件失败

				theJSLog<<"无法移动到备份目录,暂时改为临时文件"<<endw;
				char tmp[512];
				memset(tmp,0,sizeof(tmp));
				strcpy(tmp,fileName);
				strcat(tmp,".tmp");
				rename(fileName,tmp);		
			}
		}
		else
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"核对文件备份路径[%s]不存在，且无法创建",bak_dir);
			theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg);		//打开目录出错

			theJSLog<<"无法移动到备份目录,暂时改为临时文件"<<endw;
			char tmp[512];
			memset(tmp,0,sizeof(tmp));
			strcpy(tmp,fileName);
			strcat(tmp,".tmp");
			rename(fileName,tmp);
			//rename(fileName,strcat(fileName,".tmp"));
		}
		
	}catch (jsexcp::CException &e) 
	{	
		rollBackSQL();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"[%s]run() err %s",m_szFileName,e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		
	}
	
	theJSLog<<"######## end deal file ########\n"<<endi;
}

//检查核对文件名上面的格式是否都能找到 0表示都能找到,>0表示未找到的记录条数,-1表示核对内容记录格式有误
int FileInAduit::checkFile()
{	
		char szBuff[1024];
		ifstream in(fileName,ios::in) ;
		if(!in)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"checkFile 文件[%s]打开出错",fileName);
			theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败
			return -1;
		}
		
		memset(szBuff,0,sizeof(szBuff));
		vector<string> vf ;
		int count = 0 ,total = 0,month = 0,cnt = 0,err_cnt = 0,suc_cnt = 0;
		while(in.getline(szBuff,sizeof(szBuff)))		//考虑实现记录数的校验???
		{	
			if(count == 0)
			{
				theJSLog<<"文件头:"<<szBuff<<endi;					//可以对文件头进行解析，判断里面文件名的个数
				vf.clear();
				splitString(szBuff,"|",vf,false,false);
				total = atoi(vf[3].c_str());					//解析文件头
				if(strcmp("10",vf[0].c_str()))
				{
					theJSLog.writeLog(LOG_CODE_FILE_HEAD_TAIL_VALID,"日核对文件 头记录格式不正确");
					return -1;
				}

				total++;
				count++;
				continue;
			}
			
			if(count == total)		
			{
				theJSLog<<"文件尾："<<szBuff<<endi;
				if(strcmp("90",szBuff))
				{
					in.close();
					fileList.clear();
					theJSLog.writeLog(LOG_CODE_FILE_HEAD_TAIL_VALID,"日核对文件 尾记录格式不正确");

					return -1;
				}
				break ;
			}

			vf.clear();
			splitString(szBuff,"|",vf,false,false);
			if(vf.size() != 3)
			{
				in.close();
				fileList.clear();

				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"日核对文件记录格式不正确,记录行号(%d)",count);
				theJSLog.writeLog(LOG_CODE_FILE_ERR_RCD,erro_msg);

				return -1;
			}

			Check_Rec_Fmt rc ;					//解析文件记录行，文件名，账期标志，账期
			rc.fileName = vf[0];
			rc.rate_flag = vf[1];
			rc.month = vf[2];
			fileList.push_back(rc);
			
			count++;
			memset(szBuff,0,sizeof(szBuff));
			
		}
		in.close();
		
		total--;
		theJSLog<<"记录条数："<<total<<endi;
		char check_flag = 'Y';
		for(int i = 0;i<fileList.size();i++)
		{		
				cnt = 1;	
				check_flag = 'Y';
				cnt = check_file_exist(fileList[i].fileName.c_str());
			
				if(cnt == 0)
				{
						theJSLog<<"文件["<<fileList[i].fileName<<"]核对失败"<<endw;
						check_flag = 'N';
						err_cnt++;
				}
				else
				{		theJSLog<<"文件["<<fileList[i].fileName<<"]核对成功"<<endi;
						suc_cnt++;
				}
				
				//每查询一个文件都要登记
				getCurTime(currTime);    //获取当前文件时间
				memset(sql,0,sizeof(sql));
				sprintf(sql,"insert into d_check_file_detail(check_file,content,deal_time,check_flag,check_type,rate_cycle,file_time,source_id,cycle_flag) values('%s','%s','%s','%c','AUD','%s','%s','%s','%s')",m_szFileName,fileList[i].fileName,currTime,check_flag,fileList[i].month,file_time,m_szSourceID,fileList[i].rate_flag);		
				writeSQL(sql);
				
				//2013-10-24 新增 C_RATE_CYCLE
				if(atoi(fileList[i].rate_flag.c_str()) == 1)
				{
					theJSLog<<"文件"<<fileList[i].fileName<<"为封帐标志文件,封帐日期:"<<fileList[i].month<<endi;
					memset(sql,0,sizeof(sql));
					sprintf(sql,"update C_RATE_CYCLE set CYCLE_FLAG='Y' where source_id = '%s' and rate_cycle = '%s' ",m_szSourceID,fileList[i].month);
					writeSQL(sql);

					//自动增加下个账期
					char tmp_month[6+1];
					int month,cnt;
					
					memset(tmp_month,0,sizeof(tmp_month));
					strcpy(tmp_month,fileList[i].month.c_str());
					if(strncmp("12",tmp_month+4,2) == 0)
					{
						month = (atoi(tmp_month)/100)+1;
						month = month*100+1;
					}

					month = (atoi(tmp_month)+1);
					
					if(dbConnect(conn))
					{
						memset(sql,0,sizeof(sql));
						sprintf(sql,"select count(*) from C_RATE_CYCLE where source_id = '%s' and rate_cycle = '%d' ",m_szSourceID,month);
						Statement stmt = conn.createStatement();
						stmt.setSQLString(sql);
						stmt.execute();
						stmt>>cnt;
						if(cnt == 0)	//表示要自动增加下个该数据源账期记录
						{
							char tmp_date[8+1];
							memset(tmp_date,0,sizeof(tmp_date));
							strncpy(tmp_date,currTime,8);
							currTime[8] = '\0';

							memset(sql,0,sizeof(sql));
							sprintf(sql,"insert into C_RATE_CYCLE (source_id,rate_cycle,update_time) values('%s','%d','%s')",m_szSourceID,month,tmp_date);
							writeSQL(sql);
						}
						
						conn.close();
					}
					else
					{
						theJSLog<<"连接数据库失败"<<endw;
					}
					
				}		
		}
	
	 //最后登记总的情况
	 memset(sql,0,sizeof(sql));
	 sprintf(sql,"insert into d_check_file_result(check_file,total_cnt,suc_cnt,err_cnt,deal_time,check_type) values('%s',%d,%d,%d,'%s','AUD')",m_szFileName,total,suc_cnt,err_cnt,currTime);
	 writeSQL(sql);
		
	if(err_cnt)			//失败了数据清空,避免下次查询缓存了上次
	{
		rollBackSQL();	
		fileList.clear();
		mapFileSource.clear();
	}

	return err_cnt;  //大于0表示返回失败的文件个数
}

//月汇总
void FileInAduit::run2()
{
	int ret = -1 ;

	if(gbExitSig)
	{
		theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "应用程序收到退出信号");
		prcExit();
		return;
	}

	day_or_month = 0;

	try
	{	
		short db_status = 0;
		rtinfo.getDBSysMode(db_status);

		//如果是备系统
		if( drStatus == 1 ) 
		{
			if(db_status != petri_status_tmp)
			{
				theJSLog<<"数据库状态切换... "<<petri_status_tmp<<"->"<<db_status<<endw;
				int cmd_sn = 0;
				if( !putEvt(cmd_sn, EVT_RPT_DBSTATUS, 0, db_status, DSPCH_PRC_ID) )
				{
					theJSLog<<"报告数据库更换状态失败！\n"<<endw;
					return;
				}
				petri_status_tmp = db_status;
			}

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
			splitString(m_SerialString,";",data,false,false);  //发送的字符串:文件名
			
			memset(fileName,0,sizeof(fileName));
			
			strcpy(fileName,month_input_path);		
			strcat(fileName,data[0].c_str());
			
			setSQLFileName(data[0].c_str());		//设置sqlFile文件名
			
			if(petri_status != atoi(data[1].c_str()))
			{
				theJSLog<<"主系统的数据库状态发生了切换..."<<endw;
			}
			petri_status = atoi(data[1].c_str());		//备系统的状态根据主系统来定	

			//check_file check_path 其实可以保证程序进入到这一步已经表示可以找到月核对文件

			//int dr_GetAuditMode()1表示同步，2表示跟随, 其它为失败，-1是配置错误，-2配置文件读取出现问题
			int iStatus = dr_GetAuditMode(module_name);
			if(iStatus == 1)		//同步模式,等待，主系统要么成功,要么失败,要么超时
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
					dr_AbortIDX();

					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"月核对文件[%s]中文件名不存在",fileName);
					theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);
					return ;
				}	
				
			}
			else if(iStatus == 2)		//跟随模式,不等，主系统直接仲裁成功
			{
				while(1)
				{
					//设置中断

					if(gbExitSig)
					{
						dr_AbortIDX();

						theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "应用程序收到退出信号");
						prcExit();
						return;
					}

					if(access(fileName,F_OK|R_OK))
					{
						//theJSLog<<"查找了"<<times<<"次文件"<<endi;
						sleep(10);
					}
				}
			}
			else
			{
				char tmp[50] = {0};
				snprintf(tmp, sizeof(tmp), "容灾平台dr_GetAuditMode函数配置错误，返回值[%d]", iStatus);
				theJSLog<<tmp<<endi;
				return ;
			}
			
			theJSLog<<"######## start deal file "<<fileName<<" ########"<<endi;
			//theJSLog<<"查找到月核对文件:"<<fileName<<endi;
		
	}
	else
	{		
			if(db_status != petri_status)
			{
				theJSLog<<"数据库状态切换... "<<petri_status<<"->"<<db_status<<endw;
				int cmd_sn = 0;
				if( !putEvt(cmd_sn, EVT_RPT_DBSTATUS, 0, db_status, DSPCH_PRC_ID) )
				{
					theJSLog<<"报告数据库更换状态失败！\n"<<endw;
					return;
				}
				petri_status = db_status;
				petri_status_tmp = db_status;
			}

			//主系统,非容灾系统	
			if(scan.openDir(month_input_path))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"打开文件目录[%s]失败",month_input_path);
				theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
				return ;		
			}
			
			char tmp[512];
			int rett = 0;
			while(1)		
			{
				memset(fileName,0,sizeof(fileName));		
				rett = scan.getFile("*.SUM",fileName); 		
				if(rett == 100)
				{		
						scan.closeDir();
						return ;
				}
				else if(rett == -1)
				{	
						scan.closeDir();
						return ;			//表示获取文件信息失败
				}
				
				/*过滤文件*.tmp,*.TMP,~* */			
				char* p = strrchr(fileName,'/');
				memset(tmp,0,sizeof(tmp));
				strcpy(tmp,p+1);
			
				if(tmp[0] == '~' )	continue ;
				//if(strlen(tmp) <= 3) continue;
				if(strlen(tmp) > 3)						//条件可考虑舍弃，前面扫描文件已过滤
				{		
						int pos = strlen(tmp)-4;
						//cout<<"后缀名为："<<tmp+pos<<endl;
						if((strcmp(tmp+pos,".tmp") && strcmp(tmp+pos,".TMP")) == 0) 
						{
							//cout<<"扫描到临时文件，舍弃"<<fileName<<endl;
							continue;
						}
				}
				
				theJSLog<<"######## start deal file "<<fileName<<" ########"<<endi;
				//theJSLog<<"扫描到文件："<<fileName<<endi;	

				strcpy(m_szFileName,p+1);		
				setSQLFileName(m_szFileName);

				break;
			}
		
		 scan.closeDir();	 
	}
	
	if(!(dbConnect(conn)))
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run2() 连接数据库失败");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);
		
		theJSLog<<"######## end deal file ########\n"<<endi;
		return ;
	}

	//先进行月核对,主备系统
	ret = checkMonthFile();
	conn.close();

	if(ret)
	{
		if(ret > 0 )
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"月核对失败");
			theJSLog.writeLog(LOG_CODE_FILE_ERR_CHECK,erro_msg);
		}
		
		rollBackSQL();	
		fileList2.clear();

		if(drStatus  == 1) //备系统
		{
			dr_AbortIDX();
		}
		
		if(ret == -2)		//月核对条件不满足
		{	
			return ;
		}

		char tmp[512];
		memset(tmp,0,sizeof(tmp));
		strcpy(tmp,fail_path);
		strcat(tmp,m_szFileName);
		if(rename(fileName,tmp))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"移动文件[%s]失败: %s",m_szFileName,strerror(errno));
			theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
		}
		
		theJSLog<<"######## end deal file ########\n"<<endi;

		return ;
	}

	if(drStatus != 1)		//主系统
	{
		//发送同步信息
		memset(m_SerialString,0,sizeof(m_SerialString));
		sprintf(m_SerialString,"%s;%d",m_szFileName,petri_status);
		ret = drVarGetSet(m_SerialString);
		if(ret)
		{
			rollBackSQL();	
			fileList2.clear();

			theJSLog<<"同步失败...."<<endw;
			return ;
		}
	}
	
	//拼接仲裁字符串
	memset(m_AuditMsg,0,sizeof(m_AuditMsg));
	for(int i = 0;i<fileList2.size();i++)
	{
		sprintf(m_AuditMsg,"%s%s,%ld,%ld,%.2f|",m_AuditMsg,fileList2[i].file_type,fileList2[i].cdr_count,fileList2[i].cdr_duration,fileList2[i].cdr_fee);
	}
	
	fileList2.clear();
	
	char tmp[512];

	//仲裁......................................
	theJSLog<<"wait dr_audit() ..."<<endi;
	ret = IsAuditSuccess(m_AuditMsg);
	if(ret)									//回滚数据库,将文件移到错误目录
	{
		rollBackSQL();	
		if(ret != 3)						//2013-11-07 仲裁超时不移动文件
		{
			theJSLog<<"将文件挪到失败目录:"<<fail_path<<endi;
			memset(tmp,0,sizeof(tmp));
			strcpy(tmp,fail_path);
			strcpy(tmp,m_szFileName);
			strcat(fail_path,m_szFileName);
			if(rename(fileName,tmp))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"移动文件[%s]失败: %s",fileName,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
			}
		}

		theJSLog<<"######## end deal file ########\n"<<endi;
		return ;
	}

	//仲裁成功,写数据库,并将文件备份
	theJSLog<<"提交月核对信息到数据库..."<<endi;
	if(petri_status == DB_STATUS_OFFLINE)
	{
		theJSLog<<"数据库状态为只读态,写sql文件"<<endi;
		commitSQLFile();
	}
	else
	{
		if(!(dbConnect(conn)))
		{
			commitSQLFile();

			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"run2() 连接数据库失败,暂时写sql文件");
			//theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		
			theJSLog<<erro_msg<<endw;
		}
		else
		{
			m_vsql = getvSQL();
			updateDB();
			conn.close();
		}	
 	}		
	
	theJSLog<<"月核对文件"<<m_szFileName<<"备份到"<<bak_path2<<endi;
	memset(tmp,0,sizeof(tmp));
	strcpy(tmp,bak_path2);
	strcat(tmp,m_szFileName);
	if(rename(fileName,tmp))
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"文件[%s]移动[%s]失败: %s",fileName,bak_path2,strerror(errno));
		theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);		//移动文件失败
	}

	}catch (jsexcp::CException &e) 
	{	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"[%s]run() err %s",m_szFileName,e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		
	}

	theJSLog<<"######## end deal file ########\n"<<endi;
}

//处理月汇总文件  文件的头尾都没有校验，可以考虑！！！！！！！！！！！
int FileInAduit::checkMonthFile()
{
	int ret = 0 ;
	try
	{	
		ifstream in(fileName,ios::in) ;
		if(!in)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"dealMonthFile 文件%s打开出错",fileName);
			theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败
			return -1;
		}

		int count = 0 ,total = 0,err_cnt = 0,suc_cnt = 0;
		long cdr_cnt = 0,cdr_duration = 0;
		long totalFee = 0,feeTmp = 0;
		char check_flag ,month[6+1];
		
		char szBuff[1024];
		memset(szBuff,0,sizeof(szBuff));
		vector<string> vf ;
		while(in.getline(szBuff,sizeof(szBuff)))   
		{	
			if(count == 0)
			{
				theJSLog<<"文件头:"<<szBuff<<endi;  
				vf.clear();
				splitString(szBuff,"|",vf,false,false);
				total = atoi(vf[3].c_str());				//解析文件头，记录类型，文件产生日期，核对账期，记录个数
				
				memset(month,0,sizeof(month));
				strncpy(month,vf[2].c_str(),6);
				month[6] = '\0';
				//month = atoi(vf[2].c_str())/100;

				if(strcmp("10",vf[0].c_str()))
				{
					in.close();
					memset(erro_msg,0,sizeof(erro_msg));
					theJSLog.writeLog(LOG_CODE_FILE_HEAD_TAIL_VALID,"月核对文件 头记录格式不正确");
					return -1;
				}

				total++;
				count++;
				continue;
			}
			
			if(count == total)		
			{
				theJSLog<<"文件尾："<<szBuff<<endi;
				if(strcmp("90",szBuff))
				{
					in.close();
					memset(erro_msg,0,sizeof(erro_msg));
					theJSLog.writeLog(LOG_CODE_FILE_HEAD_TAIL_VALID,"月核对文件 尾记录格式不正确");
					return -1;
				}
				break ;

			}

			vf.clear();
			splitString(szBuff,"|",vf,false,false);
			if(vf.size() != 4)
			{
				in.close();
				//theJSLog<<"月汇总文件 记录行："<<szBuff<<"格式不对!"<<" 行号："<<count<<endw;
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"checkMonthFile 月核对文件记录格式不正确,行号(%d)",count);
				theJSLog.writeLog(LOG_CODE_FILE_ERR_RCD,erro_msg);

				//err_cnt++;					//格式不正确 暂时不登记到数据库
				//continue ;
				return -1;
			}

			Check_Sum_Rec_Fmt rc ;			//解析文件记录行,文件类型，话单条数，通话时长，总费用
			rc.file_type = vf[0];
			rc.cdr_count = atol(vf[1].c_str());
			rc.cdr_duration = atol(vf[2].c_str());
			rc.cdr_fee = atof(vf[3].c_str());

			fileList2.push_back(rc);
			
			count++;
			memset(szBuff,0,sizeof(szBuff));
			
		}
		in.close();
		
		total--; 
		theJSLog<<"月汇总文件记录条数："<<total<<endi;

		getCurTime(currTime);    //获取当前文件时间

		//由于可能存在文件格式不正确，导致总的记录数不一定=匹配到的记录数
		for(int i = 0;i<fileList2.size();i++)
		{		
				check_flag = 'Y';

				Check_Sum_Rec_Fmt ff = fileList2[i];
				map< string,Check_Sum_Conf >::const_iterator iter = monthSumMap.find(ff.file_type);
				if(iter == monthSumMap.end())
				{					
						theJSLog<<"checkMonthFile 配置表中没有找到月汇总文件文件类型为["<<ff.file_type<<"]的信息"<<endw;
						check_flag = 'N';
						err_cnt++;
						memset(sql,0,sizeof(sql));
						sprintf(sql,"insert into d_check_file_detail(check_file,content,deal_time,check_flag,check_type,rate_cycle) values('%s','%s','%s','%c','SUM','%s')",m_szFileName,ff.file_type,currTime,check_flag,month);
						writeSQL(sql);
						continue;
				}
				
				//2013-11-12,月核对条件和月汇总条件一直,看当月日汇总是否完成

				//查询当月是否封帐
				Statement stmt = conn.createStatement();
				memset(sql,0,sizeof(sql));
				sprintf(sql,"select count(*) from D_CHECK_FILE_DETAIL where source_id like '%s%s' and check_type = 'AUD' and rate_cycle = '%s' and cycle_flag = '1'",ff.file_type,"%",month);
				stmt.setSQLString(sql);
				stmt.execute();
				stmt>>ret;
				if(ret == 0)
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"checkMonthFile 数据源[%s]当月账期[%s]还没封帐...",ff.file_type,month);
					theJSLog<<erro_msg<<endi;
					return -2;	
				}
	
				//将当月的封帐后的时间存,和日汇总后的时间比较
				vector<string> vft;
				string ft;
				memset(sql,0,sizeof(sql));
				sprintf(sql,"select distinct(file_time) from D_CHECK_FILE_DETAIL where source_id like '%s%s' and check_type = 'AUD' and rate_cycle = '%s'  order by  file_time desc ",ff.file_type,"%",month);
				stmt.setSQLString(sql);
				stmt.execute();
				while(stmt>>ft)
				{
					vft.push_back(ft);
				}
	
				memset(sql,0,sizeof(sql));
				sprintf(sql,"select count(*) from D_SUMMARY_RESULT where sourceid = '%s' and sumtype in('D','RD') and sumdate = :1 ",ff.file_type);
				for(int  i = 0;i<vft.size();i++)
				{
					stmt.setSQLString(sql);
					stmt<<vft[i];
					stmt.execute();
					stmt>>ret;
					if(ret == 0)
					{		
						theJSLog<<"数据源["<<ff.file_type<<"]在日期["<<vft[i]<<"]还没有完成日汇总..."<<endi;
						vft.clear();
						return -2;
					}
				}
	
				vft.clear();


				//查询表，求出对应表中的数据与核对记录进行合计
				memset(sql,0,sizeof(sql));
				//cout<<"cdr_duration="<<iter->second.cdr_duration<<endl;

				sprintf(sql,"select sum(%s),sum(%s)",iter->second.cdr_count,iter->second.cdr_duration);
				vector<string> vfee = iter->second.cdr_fee;
				for(int i = 0;i<vfee.size();i++)
				{
						sprintf(sql,"%s,sum(%s)",sql,vfee[i]);
				}
				//memset(sql,0,sizeof(sql));
				sprintf(sql,"%s from %s_%s ",sql,iter->second.sum_table,month);

				cout<<"费用 sql = "<<sql<<endl;

				//Statement stmt = conn.createStatement();
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
					theJSLog<<"文件类型：["<<ff.file_type<<"] 通话次数核对失败 "<<cdr_cnt<<" != "<<fileList2[i].cdr_count<<endw;
					check_flag = 'N';
				}
				if(cdr_duration != fileList2[i].cdr_duration)
				{
					theJSLog<<"文件类型：["<<ff.file_type<<"] 通话时长核对失败 "<<cdr_duration<<" != "<<fileList2[i].cdr_duration<<endw;
					check_flag = 'N';
				}
				
				if(totalFee != fileList2[i].cdr_fee)
				{
					theJSLog<<"文件类型：["<<ff.file_type<<"] 费用核对失败 "<<totalFee<<" != "<<fileList2[i].cdr_fee<<endw;
					check_flag = 'N';
				}


				if(check_flag == 'Y')
				{
						theJSLog<<"文件类型：["<<ff.file_type<<"]核对成功"<<endi;
						suc_cnt++;
				}
				else
				{		
						//theJSLog<<"文件类型：["<<ff.file_type<<"]核对失败"<<endi;
						err_cnt++;
				}

				//每查询一个文件类型都要登记
				memset(sql,0,sizeof(sql));
				sprintf(sql,"insert into d_check_file_detail(check_file,content,deal_time,check_flag,check_type,rate_cycle) values('%s','%s','%s','%c','SUM','%s')",m_szFileName,ff.file_type,currTime,check_flag,month);
				writeSQL(sql);
				
				//2013-09-19 写核对结果数据到表给前台显示
				memset(sql,0,sizeof(sql));
				sprintf(sql,"insert into D_MONTH_CHECK_RESULT(CHECK_ACCOUNT,FILE_TYPE,CALL_COUNTS_I,DURATION_I,CDR_FEE_I,CALL_COUNTS_O,DURATION_O,CDR_FEE_O) values('%s','%s',%ld,%ld,%ld,%ld,%ld,%ld)",month,ff.file_type,fileList2[i].cdr_count,fileList2[i].cdr_duration,fileList2[i].cdr_fee,cdr_cnt,cdr_duration,totalFee);
				writeSQL(sql);
				cout<<"月核对结果:"<<sql<<endl;
		}

		theJSLog<<"登记月汇总文件总的核对情况"<<endi;
		//最后登记总的情况
		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into d_check_file_result(check_file,total_cnt,suc_cnt,err_cnt,deal_time,check_type) values('%s',%d,%d,%d,'%s','SUM')",m_szFileName,total,suc_cnt,err_cnt,currTime);
		writeSQL(sql);

		ret = err_cnt;

	}catch (util_1_0::db::SQLException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"checkMonthFile() 数据库操作异常%s(%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
		return -1;
	}	
	catch (jsexcp::CException &e) 
	{	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"checkMonthFile() %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);	
		return -1;
	}
		
	return ret;
}


//2013-10-23 批量提交sql,保证一个事物完整性
int FileInAduit::updateDB()
{	
	int ret = 0;
	Statement stmt;
	string ssql;
    try
    {	
		stmt = conn.createStatement();
		for(int i =0;i<m_vsql.size();i++)
		{	
			//cout<<"sql = "<<vsql[i]<<endl;
			ssql = m_vsql[i];
			stmt.setSQLString(ssql);
			ret = stmt.execute();
		}
		stmt.close();

		rollBackSQL();			//sql语句成功执行,回滚sql文件
	}
	catch(util_1_0::db::SQLException e)
	{ 
		stmt.rollback();
		commitSQLFile();		//抛异常时写sql文件编译查找问题

		m_vsql.clear();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"updateDB 数据库出错%s (%s),暂时写sql文件",e.what(),ssql);
		//theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
		theJSLog<<erro_msg<<endw;
		return -1;
	}

	m_vsql.clear();
	
	return ret ;
}

//2013-11-02 新增退出函数
void FileInAduit::prcExit()
{
	int ret = 0;
	if(m_enable) 
	{
		ret = dr_ReleasePlatform();
		if(ret != 0)
		{
			char tmp[100] = {0};
			snprintf(tmp, sizeof(tmp), "释放容灾平台失败,返回值=%d", ret);
			theJSLog<<tmp<<endw;
		}
		
		m_enable = false;
	}
	
	PS_Process::prcExit();
}

//容灾初始化
bool FileInAduit::drInit()
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
int FileInAduit::drVarGetSet(char* serialString)
{
		int ret  = 0;
		char tmpVar[5000] = {0};
		
		if(!m_enable)
		{
			return ret;
		}

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
			if(day_or_month)
			{
				snprintf(tmpVar, sizeof(tmpVar), "%s", input_path);
			}
			else
			{
				snprintf(tmpVar, sizeof(tmpVar), "%s", month_input_path);
			}

			ret = dr_SyncIdxVar("@@CHECKPATH", tmpVar,SYNC_SINGLE);  
			if(ret != 0)
			{
				theJSLog<<"传输文件所在路径失败,文件路径["<<tmpVar<<"]"<<endi;
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

			cout<<"传输文件路径:"<<tmpVar<<" 文件名:"<<m_szFileName<<endl;
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
		if(drStatus == 1)
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

		return ret;

}

//仲裁字符串
 int FileInAduit::IsAuditSuccess(const char* dealresult)
 {
		int auitStatus = 0, ret = 0;

		if(!m_enable)
		{
			return ret;
		}

		ret = dr_Audit(dealresult);
		if(2 == ret )
		{
			theJSLog << "容灾仲裁失败,结果:" << ret <<"本端："<<dealresult<< endw;
			dr_AbortIDX();
			//return false;
		}
		else if (3 == ret)
		{
			theJSLog<<"容灾仲裁超时..."<<endw;
			dr_AbortIDX();
			//return false;
		}
		else if(4 == ret)
		{
			theJSLog<<"对端idx异常终止..."<<endw;
			dr_AbortIDX();
			//return false;
		}
		else if(1 == ret)
		{
			ret = dr_CommitSuccess();
			if(ret != 0)
			{
				theJSLog << "业务全部提交失败(容灾平台)" << endw;
				dr_AbortIDX();
				//return false;
			}
			theJSLog<<"ret = "<<ret<<"仲裁成功...\n仲裁内容："<<dealresult<<endi;
			//return true;
		}
		else
		{
			theJSLog<<"未知ret="<<ret<<"	仲裁内容："<<dealresult<<endw;
			dr_AbortIDX();
			//return false;
		}
	
	return ret;
 }

bool FileInAduit::CheckTriggerFile()
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


/**************************************************
*	Function Name:	checkFormat
*	Description:	比较两个字符串是否匹配（相等）
*	Input Param:
*		cmpString -------> 被比较的字符串
*		format	   -------> 匹配的字符串，支持*,?,[]等通配符
*	Returns:
*		如果两个字符串匹配，返回SUC
*		如果两个字符串不匹配，返回FAIL
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
	cout<<"*    GuangDong Telecom. Telephone Network      "<<endl;
	cout<<"*       Centralized Settlement System          "<<endl;
	cout<<"*                                              "<<endl;
	cout<<"*            fileInAduit                       "<<endl;
	cout<<"*              Version 1.0	                  "<<endl;
	cout<<"*     created time :     2013-07-01 by  hed 	  "<<endl;
	cout<<"*     last update time : 2013-11-14 by  hed	  "<<endl;
	cout<<"********************************************** "<<endl;

	FileInAduit fm ;

	if( !fm.init( argc, argv ) )
	{
		 return -1;
	}
    	
	//while(1)
	//{		
	//		theJSLog.reSetLog();

	//		fm.run();		//日核对文件
	//		fm.run2();
	//		sleep(30);
	//}
	
	fm.execute();

   return 0;
}


