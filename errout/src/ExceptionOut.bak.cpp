/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-07-20
File:			 ExceptionOut.cpp
Description:	 异常文件输出模块
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
    memset(sqlFile,0,sizeof(sqlFile));
	memset(erro_msg,0,sizeof(erro_msg));

}


ExceptionOut::~ExceptionOut()
{

}


//模块初始化动作
bool ExceptionOut::init(int argc,char** argv)
{  
    if(!PS_Process::init(argc,argv))
    {
      return false;
    }
	
	PS_Process::setSignalPrc(); 

	//获取petri网状态,当系统为只读态时,数据库更新操作语句写文件
	
	if(!(rtinfo.connect()))
	{
		return false;
	}
	//short status;
	//rtinfo.getSysMode(petri_status);
	//cout<<"petri status:"<<status<<endl;


	//*********2013-07-15 读取数据库的配置信息，如数据源组，错误目录 2013-03-11 add by hed*********************************************/
	
	if(!(dbConnect(conn)))
	{
		cout<<"连接数据库 connect error."<<endl;	//写日志
		return false ;
	}

	//int flow_id = getFlowID();
	//int module_id = getModuleID();

	cout<<"流水线ID:"<<getFlowID()<<"   模块ID:"<<getModuleID()<<endl;
	try{

		//输入输出路径由于和数据源组没有关系，所以配置在C_GOLABAL_ENV里面，输入是相对路径，输出是绝对路径
		string sql ;
		Statement stmt = conn.createStatement();

		sql = "select varvalue from c_global_env where varname = 'EXCEPTION_INPUT'";
		stmt.setSQLString(sql);
		stmt.execute();
		if(!(stmt>>input_path))
		{
			cout<<"异常文件输出模块输入相对路径input_path没有配置"<<endl;		
			return false ;
		}		
		completeDir(input_path);
		
		sql = "select varvalue from c_global_env where varname = 'EXCEPTION_OUTPUT'";
		stmt.setSQLString(sql);
		stmt.execute();
		if(!(stmt>>out_path))
		{
			cout<<"异常文件输出模块输出相对路径output_path没有配置"<<endl;		
			return false ;
		}		
		completeDir(out_path);
					
		stmt.close();
	   
	   }catch(SQLException e)
		{
			cout<<"init 数据库查询异常:"<<e.what()<<endl;
			return false ;
		}
	
	 //2013-07-18 从核心参数里面读取日志的路径，级别，
	 char sParamName[256],szLogPath[PATH_NAME_LEN+1],szLogLevel[10],sql_path[1024];
	 CString sKeyVal;
	 sprintf(sParamName, "log.path");
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(szLogPath,0,sizeof(szLogPath));
		strcpy(szLogPath,(const char*)sKeyVal);
		completeDir(sql_path);

		sprintf(sqlFile,"%sjserrout.sql",sql_path);	
	 }
	 else
	 {	
		cout<<"请在核心参数里配置日志的路径"<<endl;
		return false ;
	 }	 
	 sprintf(sParamName, "log.level");
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(szLogLevel,0,sizeof(szLogLevel));
		strcpy(szLogLevel,(const char*)sKeyVal);

	 }
	 else
	 {	
		cout<<"请在核心参数里配置日志的级别"<<endl;
		return false ;
	 }
	
	 sprintf(sParamName, "sql.path");
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(sql_path,0,sizeof(sql_path));
		strcpy(sql_path,(const char*)sKeyVal);
	 }
	 else
	 {	
		cout<<"请在核心参数里配置sql文件所在的路径"<<endl;
		return false ;
	 }

	
	//判断日志目录和sql目录是否存在
	 DIR *dirptr = NULL; 
	 if((dirptr=opendir(szLogPath)) == NULL)
	 {
		cout<<"日志目录:"<<szLogPath<<"打开失败"<<endl;	
		return false ;
	 }else closedir(dirptr);

	 if((dirptr=opendir(sql_path)) == NULL)
	 {		
		cout<<"SQL目录:"<<sql_path<<"打开失败"<<endl;
		return false ;
	 }else  closedir(dirptr);	
	
	 if((dirptr=opendir(out_path)) == NULL)
	 {
		cout<<"输出目录"<<out_path<<"打开失败"<<endl;
		return false ;

	 }else closedir(dirptr);


	//初始化内存日志接口
	bool bb = initializeLog(argc,argv,false);  //是否调试模式
	if(!bb)
	{
			cout<<"初始化内存日志接口失败"<<endl;
			return false;
	}
	

	theJSLog.setLog(szLogPath, atoi(szLogLevel),"ZBJS" , "ERROUT", 001);	//文件日志接口，调用了内存日志接口
	
	theJSLog<<"相对输入路径："<<input_path<<"  输出路径:"<<out_path<<"	日志路径:"
			<<szLogPath<<" 日志级别:"<<szLogLevel<<" sql存放路径:"<<sql_path<<endi;

	if(LoadSourceCfg() == -1) 
	{		
		//cout<<"加载数据源配置信息失败"<<endl;
		return false ;  
	}
	
	char input_dir[1024];

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

   conn.close();
	
   theJSLog<<"初始化完毕！"<<endi;

   return true ;
}


//加载数据源配置信息，取全部数据源组的数据源信息
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

					//if(getSourceFilter(SourceCfg.szSourceId,SourceCfg.filterRule) == 0)
					//{
					//cout<<"数据源："<<strSourceId<<" 没有配置过滤规则！"<<endl;
					//	sprintf(erro_msg,"数据源%s没有配置过滤规则",strSourceId);	//环境变量未设置
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
		sprintf(erro_msg,"LoadSourceCfg数据库出错：%s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		//throw jsexcp::CException(0, "LoadSourceCfg加载数据库出错：", __FILE__, __LINE__);
		return -1;
	 }

	return 0;
}


//数据库的更新操作，便于捕获异常sql 便于重现
int ExceptionOut::updateDB(char* sql)
{
	try
	{
		//cout<<"SQL:"<<sql<<endl;
		Statement stmt = conn.createStatement();
		stmt.setSQLString(sql);
		stmt.execute();
		stmt.close();

	}
	catch(util_1_0::db::SQLException e)
	{ 
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"updateDB 数据库出错：%s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
		return -1;
	}

	return 0;
}


//查看sql文件中是否有数据，若有则执行
int ExceptionOut::scanSQLFile()
{
	char szBuff[1024];
	ifstream in(sqlFile,ios::in) ;
	if(!in)
	{
		//cout<<"文件: "<<sqlFile<<"打开错误"<<endl;
		sprintf(erro_msg,"scanSQLFile 文件%s打开出错",sqlFile);
		theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败
		return -1;
	}
	
	Statement stmt = conn.createStatement();
	memset(szBuff,0,sizeof(szBuff));
	while(in.getline(szBuff,sizeof(szBuff)))   
	{		
			stmt.setSQLString(szBuff);
			stmt.execute();
			memset(szBuff,0,sizeof(szBuff));		
	}
	stmt.close();

	in.close();

	in.open(sqlFile,ios::trunc);	//清空文件
	in.close();

	return 0;

}

//写sql语句到文件
int ExceptionOut::writeSQL(char* sql)
{
	int ret = 0;

	ofstream out(sqlFile,ios::app);
	if(!out)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"writeSQL 文件[%s]打开出错",sqlFile);
		theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败
		return -1;
	}

	out<<sql<<"\n"<<endl;
	out.close();
	
	return ret;
}

//扫描错误文件信息登记表，生成异常说明文件(源文件名+.err),并将错单文件返回，文件级错误生成一条错误信息，记录级错误生成多条
void ExceptionOut::run()
{
	cout<<"程序运行！！！！！！"<<endl;

	if(gbExitSig)
	{
		if(gbExitSig) theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "应用程序收到退出信号");
		PS_Process::prcExit();
		return;
	}
	
	char sql[1024];
	try
	{
		if(!(dbConnect(conn)))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"run() 连接数据库失败 connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
			return  ;
		}

		memset(sql,0,sizeof(sql));
		sprintf(sql,"select source_id,fileName,err_msg,err_code,err_col,err_line from d_errfile_info where state = 'W'");		
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
		
		char errFile[1024],errFiletmp[1024],tmp[256];
		int pas_count = 1;			//文件传递次数
		int p1, p2;
		
		rtinfo.getSysMode(petri_status);

		for(map< string,vector<ERRINFO> >::const_iterator iter= erroinfoMap.begin();iter != erroinfoMap.end();++iter)
		{
				theJSLog<<"处理异常文件: "<<iter->first<<endi;
				
				memset(sql,0,sizeof(sql));
				sprintf(sql,"update errfile_info set state = 'H' where filename = '%s'",iter->first);	//更新状态H	
				if (petri_status == MAINTENANCE_OFFNET)   //判断是否处于维护态
				{
						writeSQL(sql);
				}
				else
				{
						updateDB(sql);
				}
				
				vector<ERRINFO>	vv = iter->second;	

				it = m_SourceCfg.find(vv[0].source_id);					//找到数据源所在的路径
				if( it == m_SourceCfg.end())
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"没有找到文件[%s]的数据源[%s]的信息！",iter->first,vv[0].source_id);
					theJSLog.writeLog(LOG_CODE_PARAM_INFO_LACK,erro_msg);	// 参数配置信息缺失
					continue;

				}

				memset(errFile,0,sizeof(errFile));
				memset(errFiletmp,0,sizeof(errFiletmp));

				strcpy(errFile,out_path);								//先写临时文件在改名
				strcat(errFile,iter->first.c_str());
				strcat(errFile,".err");
				strcpy(errFiletmp,errFile);
				strcat(errFiletmp,".tmp");

				ofstream out(errFiletmp);
				if(!out)
				{
					theJSLog.writeLog(LOG_CODE_FILE_WRITE_ERR,"写异常文件信息失败");
				}
				
				getCurTime(currTime);					
				
				if(iter->first.find(".ROL") == string::npos)			//判断文件传递次数
				{
						pas_count = 1;
				}
				else
				{
					theJSLog<<"是回滚文件"<<endi;
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
				
				theJSLog<<"写异常文件信息: "<<iter->first<<".err"<<endi;

				for(int i = 0;i<vv.size();i++)
				{
					if(strcmp(vv[i].err_msg,"F") == 0)            //表示文件级错误
					{
						out<<iter->first<<";"<<currTime<<";"<<pas_count<<";"<<vv[i].err_code<<";"<<""<<";"<<""<<endl;
					}
					else
					{
						out<<iter->first<<";"<<currTime<<";"<<pas_count<<";"<<vv[i].err_code<<";"<<vv[i].err_col<<";"<<vv[i].err_line<<endl;
					}

				}
				
				out.close();
				
				rename(errFiletmp,errFile);				//文件改名
				
				memset(sql,0,sizeof(sql));
				sprintf(sql,"update errfile_info set state = 'Y' where filename = '%s'",iter->first);	//更新状态	
				if (petri_status == MAINTENANCE_OFFNET)   //判断是否处于维护态
				{
					writeSQL(sql);
				}
				else
				{
					updateDB(sql);
				}

				//登记生成的错误表信息，2013-07-22错误的原始的文件都得登记
				memset(sql,0,sizeof(sql));
				sprintf(sql,"insert into d_out_file_reg(filename,source_id,reg_time,file_type,state) values('%s','%s','%s','%c','%c')",iter->first,vv[0].source_id,currTime,'E','W');
				if (petri_status == MAINTENANCE_OFFNET)   //判断是否处于维护态
				{
					writeSQL(sql);
				}
				else
				{
					updateDB(sql);
				}
				
				memset(sql,0,sizeof(sql));
				sprintf(sql,"insert into d_out_file_reg(filename,source_id,reg_time,file_type,state) values('%s.err','%s','%s','%c','%c')",iter->first,vv[0].source_id,currTime,'N','W');
				if (petri_status == MAINTENANCE_OFFNET)   //判断是否处于维护态
				{
					writeSQL(sql);
				}
				else
				{
					updateDB(sql);
				}
				
				//将错误文件移到输出目录
				//it = m_SourceCfg.find(vv[0].source_id);	//找到数据源所在的路径
				//if( it == m_SourceCfg.end())
				//{
				//	cout<<"没有找到文件"<<iter->first<<"数据源"<<vv[0].source_id<<"的信息"<<endl;

				//}
				//else
				//{	
						memset(tmp,0,sizeof(tmp));
						memset(outFileName,0,sizeof(outFileName));
						strcpy(tmp,it->second.szSourcePath);
						strcat(tmp,input_path);
						strcpy(outFileName,out_path);
						strcat(outFileName,iter->first.c_str());

						theJSLog<<"移到异常文件"<<"从目录["<<tmp<<"]到输出目录"<<endi;
						strcat(tmp,iter->first.c_str());			
						if(rename(tmp,outFileName))
						{
							//perror("移动文件失败:");
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"移动文件失败: %s",strerror(errno));
							theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
						}
				//}				
				
		}
		
		erroinfoMap.clear();		//清空错误信息

		conn.close();
	}
	catch (util_1_0::db::SQLException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() 数据库操作异常%s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
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
	cout<<"*    last update time :  2013-08-17 by  hed	* "<<endl;
	cout<<"********************************************** "<<endl;


	ExceptionOut fm ;


	if( !fm.init( argc, argv ) )
	{
		 return -1;
	}
        
	while(1)
	{
		fm.run();
		sleep(10);
	}

   return 0;
}


