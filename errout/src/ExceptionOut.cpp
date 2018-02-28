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
//#include <vector>
#include <sys/types.h>
#include <sys/stat.h>  //stat()函数，查询文件信息
#include<unistd.h>     //读取当前程序运行目录
//#include<iostream>
#include<fstream>
#include "ExceptionOut.h"

CLog theJSLog;
SGW_RTInfo	rtinfo;

ExceptionOut::ExceptionOut()
{  
	petri_status = DB_STATUS_ONLINE;
	vsql.clear();
	memset(m_szFileName,0,sizeof(m_szFileName));
	//memset(input_path,0,sizeof(input_path));
	//memset(out_path,0,sizeof(out_path));
	memset(m_szSourceID,0,sizeof(m_szSourceID));
	memset(outFileName,0,sizeof(outFileName));
	memset(currTime,0,sizeof(currTime));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(sql,0,sizeof(sql));
}


ExceptionOut::~ExceptionOut()
{
	mdrDeal.dr_ReleaseDR();

}

//模块初始化动作
bool ExceptionOut::init(int argc,char** argv)
{  
    if(!PS_Process::init(argc,argv))
    {
      return false;
    }

	//*********2013-07-15 读取数据库的配置信息，如数据源组，错误目录 2013-03-11 add by hed*********************************************/
	if(!(dbConnect(conn)))
	{
		cout<<"连接数据库 connect error."<<endl;	//写日志
		return false ;
	}
		
	mConfParam.iflowID = getFlowID();
	mConfParam.iModuleId = getModuleID(); 

	try
	{
		//输入输出路径由于和数据源组没有关系，所以配置在C_GOLABAL_ENV里面，输入是相对路径，输出是绝对路径
		string sql ;
		Statement stmt = conn.createStatement();
		
		sql = "select ext_param from TP_PROCESS where billing_line_id = :1 and module_id = :2";
		stmt.setSQLString(sql);
		stmt<<mConfParam.iflowID<<mConfParam.iModuleId;
		stmt.execute();
		if(!(stmt>>mConfParam.szService))
		{
			cout<<"请在tp_process表字段ext_param中配置模块["<<mConfParam.iModuleId<<"]service"<<endl;
			return false ;
		}
		
		theJSLog.setLog(szLogPath,szLogLevel,mConfParam.szService, "GJJS", 001);
		
		theJSLog<<"szLogPath="<<szLogPath<<"	szLogLevel="<<szLogLevel<<endi;
		theJSLog<<"flowID="<<mConfParam.iflowID<<"	ModuleId="<<mConfParam.iModuleId<<"	szService="<<mConfParam.szService<<endi;

		sql = "select varvalue from c_global_env where varname = 'EXCEPTION_INPUT'";
		stmt.setSQLString(sql);
		stmt.execute();
		if(!(stmt>>mConfParam.szInPath))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"异常文件输出模块输入相对路径没有配置,请在c_global_env中配置变量EXCEPTION_INPUT");		
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
			return false ;
		}		
		completeDir(mConfParam.szInPath);
		
		sql = "select varvalue from c_global_env where varname = 'EXCEPTION_OUTPUT'";
		stmt.setSQLString(sql);
		stmt.execute();
		if(!(stmt>>mConfParam.szOutPath))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"异常文件输出模块输出路径没有配置,请在c_global_env中配置变量 EXCEPTION_OUTPUT");		
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
			return false ;
		}		
		completeDir(mConfParam.szOutPath);
					
		stmt.close();
		
		theJSLog<<"szInPath="<<mConfParam.szInPath<<"  szOutPath="<<mConfParam.szOutPath<<endi;

	}catch(SQLException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init 数据库查询异常 %s，%s",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);

		return false ;
	}


	//判断异常说明文件输出目录是否存在
	 DIR *dirptr = NULL; 
	 
	 if((dirptr=opendir(mConfParam.szOutPath)) == NULL)
	 {
		//theJSLog<<"输出目录"<<mConfParam.szOutPath<<"打开失败"<<endw;
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"输出目录[%s]打开失败",mConfParam.szOutPath);
		theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错

		return false ;

	 }else closedir(dirptr);


	//初始化内存日志接口
	bool bb = initializeLog(argc,argv,false);  //是否调试模式
	if(!bb)
	{
		theJSLog.writeLog(0,"初始化内存日志接口失败");
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
		theJSLog.writeLog(0,"连接运行时信息失败");
		return false;
	}
	rtinfo.getDBSysMode(petri_status);
	theJSLog<<"petri status:"<<petri_status<<endw;

	//theJSLog.setLog(szLogPath, szLogLevel,"ERROUT" , "GJJS", 001);	//文件日志接口，调用了内存日志接口	
	//theJSLog<<"相对输入路径："<<input_path<<"  输出路径:"<<out_path<<"	日志路径:"
	//		<<szLogPath<<" 日志级别:"<<szLogLevel<<" sql存放路径:"<<sql_path<<endi;

	if(LoadSourceCfg() == -1) 
	{		
		return false ;  
	}
	
	conn.close();

	char input_dir[JS_MAX_FILEFULLPATH_LEN];
	int rett = -1;
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
					
					theJSLog<<"数据源【"<<iter->first<<"】的输入文件路径: "<<input_dir<<"不存在，自行创建"<<endw;
					rett = mkdir(input_dir,0755);
					if(rett == -1)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"数据源[%s]的错误话单文件输入路径[%s]不存在，自行创建失败",iter->first,input_dir);
						theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
						return false;
					}
					
					//return false ;
			}else closedir(dirptr);  
	}
	

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
					
					theJSLog<<"szSourceId="<<SourceCfg.szSourceId<<" szSourcePath="<<SourceCfg.szSourcePath<<endi;
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
		sprintf(erro_msg,"LoadSourceCfg数据库出错：%s (%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		//throw jsexcp::CException(0, "LoadSourceCfg加载数据库出错：", __FILE__, __LINE__);
		return -1;
	 }
	catch (jsexcp::CException &e) 
	{	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"LoadSourceCfg() %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);
		return -1;
	}

	return 0;
}


//批量提交sql,保证一个事物完整性
int ExceptionOut::updateDB()
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
		sprintf(erro_msg,"updateDB 数据库出错：%s (%s)",e.what(),ssql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
		ret =  -1;
	}

	vsql.clear();
	
	return ret ;

}


//扫描错误文件信息登记表，生成异常说明文件(源文件名+.err),并将错单文件返回，文件级错误生成一条错误信息，记录级错误生成多条
void ExceptionOut::run()
{	
	short db_status = 0;
	int ret = 0,event_sn, event_type,maxAuditNum=200;
	long param1, param2, src_id;
	Statement stmt;

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

	try
	{	
		//判断数据库状态	
		rtinfo.getDBSysMode(db_status);
		if(db_status != petri_status)
		{
			theJSLog<<"数据库状态切换... "<<petri_status<<"->"<<db_status<<endw;
			int cmd_sn = 0;
			if( !putEvt(cmd_sn, EVT_RPT_DBSTATUS, 0, db_status, DSPCH_PRC_ID) )
			{
				theJSLog<<"报告数据库更换状态失败！\n"<<endw;
				continue ;
			}
			petri_status = db_status;
		}
		if(petri_status == DB_STATUS_OFFLINE)	continue ;
		
		if(mdrDeal.mdrParam.drStatus == 1)		//主备系统
		{
			//检查trigger触发文件是否存在
			if(!mdrDeal.CheckTriggerFile(m_triggerFile))
			{
				sleep(1);
				continue ;
			}
			
			memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));
			ret = mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
			if(ret)
			{
				theJSLog<<"备系统同步失败...."<<endw;
				continue ;
			}
			
			if(!(dbConnect(conn)))
			{
				//dr_AbortIDX();
				mdrDeal.dr_abort();

				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"run() 连接数据库失败 connect error");
				theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败		
				sleep(30);
				continue  ;
			}

			//获取同步变量
			vector<string> data;		
			splitString(mdrDeal.m_SerialString,"|",data,false,false);  //发送的字符串: file_id|file_id|
			
			theJSLog<<"######## start deal d_err_file_info ########"<<endi;

			bool flag  = false;
			int counter=0;			//2014-10-23
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select source_id,fileName,err_msg,err_code,err_col,err_line,err_seq from d_errfile_info where state = 'W' and err_seq = :1 order by err_msg,fileName,err_line");		
			
			try
			{
				stmt = conn.createStatement();			//备系统需要限制记录条数吗?
				stmt.setSQLString(sql);
				
				//int iStatus = dr_GetAuditMode(module_name);
				int iStatus = mdrDeal.mdrParam.aduitMode;
				if(iStatus == 1)		//同步模式,	主系统等待指定时间
				{	
					int cnt = 0;
					while(cnt < 10)
					{	
						erroinfoMap.clear();
						cnt++;
						counter=0;

						for(int i = 0;i<data.size();i++)		//查询每个文件
						{
							flag = false ;

							stmt<<data[i];
							stmt.execute();

							ERRINFO errinfo;					//以文件名为单位,可能有多条记录
							while(stmt>>errinfo.source_id>>errinfo.filename>>errinfo.err_msg>>errinfo.err_code>>errinfo.err_col>>errinfo.err_line>>errinfo.err_seq)
							{
								counter++;
								if(counter > maxAuditNum)
								{
									break;
								}

								map< string,vector<ERRINFO> >::iterator  iter= erroinfoMap.find(errinfo.filename);
								if(iter == erroinfoMap.end())
								{
									vector<ERRINFO> errinfoV; 
									errinfoV.push_back(errinfo);
									erroinfoMap.insert(map< string,vector<ERRINFO> >::value_type(errinfo.filename,errinfoV));
									flag = true;
								}
								else
								{
									(iter->second).push_back(errinfo);
								}	
							}
					
							if(!flag)	//有文件没有找到,重新找
							{
								theJSLog<<"文件file_id:"<<data[i]<<"没找到"<<cnt<<endw;
								sleep(30);
								break;			//退出for 重新进入while
							}	
						}

						if(flag) 
						{
							theJSLog<<"表d_errfile_info中已经查到全部文件信息"<<endi;
							break;
						}
					}

					//这儿可以判断是否不需要仲裁 若文件没找齐,后面的仲裁也是白做, 需要人工干预
					if(!flag)  
					{
						//dr_AbortIDX();
						mdrDeal.dr_abort();

						stmt.close();
						conn.close();
						//theJSLog<<"异常文件无法找齐"<<endw;
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"异常文件无法找齐");
						theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);		
						continue ;
					}

				}	
				else if(iStatus == 2)
				{
					//设置中断
					if(gbExitSig)
					{
						//dr_AbortIDX();
						mdrDeal.dr_abort();
						
						stmt.close();
						conn.close();

						theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "应用程序收到退出信号");
						prcExit();
						return;
					}

					while(1)
					{	
						erroinfoMap.clear();
						counter=0;

						for(int i = 0;i<data.size();i++)
						{
							flag = false ;

							stmt<<data[i];
							stmt.execute();

							ERRINFO errinfo;
							while(stmt>>errinfo.source_id>>errinfo.filename>>errinfo.err_msg>>errinfo.err_code>>errinfo.err_col>>errinfo.err_line>>errinfo.err_seq)
							{
								counter++;
								if(counter > maxAuditNum)
								{
									break;
								}

								map< string,vector<ERRINFO> >::iterator  iter= erroinfoMap.find(errinfo.filename);
								if(iter == erroinfoMap.end())
								{
									vector<ERRINFO> errinfoV; 
									errinfoV.push_back(errinfo);
									erroinfoMap.insert(map< string,vector<ERRINFO> >::value_type(errinfo.filename,errinfoV));
									flag = true;
								}
								else
								{
									(iter->second).push_back(errinfo);
								}	
							}

							if(!flag)			//有文件没有找到,重新找
							{
								sleep(10);
								break;			//退出for 重新进入while
							}		
						}
						if(flag) 
						{
							theJSLog<<"表中已经查到全部文件信息"<<endi;
							break;
						}
					}
				}

				stmt.close();
			}
			catch (util_1_0::db::SQLException e)
			{
				//dr_AbortIDX();
				mdrDeal.dr_abort();
				stmt.close();
				conn.close();
				erroinfoMap.clear();
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"run() 数据库操作异常%s (%s)",e.what(),sql);
				theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
				continue;
			}	
		}
		else
		{
			if(!(dbConnect(conn)))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"run() 连接数据库失败 connect error");
				theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
				sleep(30);
				continue  ;
			}

			memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));

			//扫描错误登记表,先找文件级错误,再找记录集错误,可以考虑一个个的错误文件仲裁???
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select source_id,fileName,err_msg,err_code,err_col,err_line,err_seq from d_errfile_info where state = 'W' order by err_msg,fileName,err_line");		
			try
			{
				stmt = conn.createStatement();
				stmt.setSQLString(sql);
				stmt.execute();
		
				ERRINFO errinfo;
				int cnt = 0;
				while(stmt>>errinfo.source_id>>errinfo.filename>>errinfo.err_msg>>errinfo.err_code>>errinfo.err_col>>errinfo.err_line>>errinfo.err_seq)
				{
					cnt++;
					if(cnt > maxAuditNum)	break;

					map< string,vector<ERRINFO> >::iterator  iter= erroinfoMap.find(errinfo.filename);
					if(iter == erroinfoMap.end())
					{	
						if(erroinfoMap.size() >= 10)  break;  //新找到文件,防止仲裁信息超出4096

						vector<ERRINFO> errinfoV; 
						errinfoV.push_back(errinfo);
						erroinfoMap.insert(map< string,vector<ERRINFO> >::value_type(errinfo.filename,errinfoV));
						
						snprintf(mdrDeal.m_SerialString,sizeof(mdrDeal.m_SerialString),"%s%ld|",mdrDeal.m_SerialString,errinfo.err_seq); //每次新添加一个文件时,因为记录级错误可能存在多条记录				
					}
					else
					{
						(iter->second).push_back(errinfo);
					}
				
				}

				stmt.close();
			}
			catch (util_1_0::db::SQLException e)
			{
				stmt.close();
				conn.close();
				erroinfoMap.clear();
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"run() 数据库操作异常%s (%s)",e.what(),sql);
				theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
				sleep(30);
				continue;
			}	

			if(erroinfoMap.size() == 0) 
			{
				conn.close();
				sleep(30);
				continue ;
			}
			
			theJSLog<<"######## start deal d_err_file_info ########"<<endi;

			ret = mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
			if(ret)
			{
				theJSLog<<"主系统同步失败...."<<endw;
				erroinfoMap.clear();
				conn.close();
				sleep(30);
				continue ;
			}
		}
		
		char errFile[512],errFiletmp[512],tmp[256];
		int pas_count = 1;			//文件传递次数
		int p1, p2;
				
		map<string,string> errFileMap;		//清空内存信息
		//vsql.clear();
		memset(mdrDeal.m_AuditMsg,0,sizeof(mdrDeal.m_AuditMsg));
	
		for(map< string,vector<ERRINFO> >::const_iterator iter= erroinfoMap.begin();iter != erroinfoMap.end();++iter)
		{
				theJSLog<<"处理异常文件: "<<iter->first<<endi;
				
				memset(sql,0,sizeof(sql));
				sprintf(sql,"update d_errfile_info set state = 'H' where filename = '%s'",iter->first);	//更新状态H	
				vsql.push_back(sql);

				vector<ERRINFO>	vv = iter->second;		
				
				snprintf(mdrDeal.m_AuditMsg,sizeof(mdrDeal.m_AuditMsg),"%s%s,%s,%ld:",mdrDeal.m_AuditMsg,vv[0].source_id,vv[0].err_msg,vv[0].err_seq);//文件名,数据源,文件错误类型

				it = m_SourceCfg.find(vv[0].source_id);					//找到数据源所在的路径
				if( it == m_SourceCfg.end())
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"没有找到文件[%s]的数据源[%s]的信息！",iter->first,vv[0].source_id);
					theJSLog.writeLog(LOG_CODE_SOURCE_NOT_FIND,erro_msg);	// 参数配置信息缺失
					
					snprintf(mdrDeal.m_AuditMsg,sizeof(mdrDeal.m_AuditMsg),"%s%s not find;",mdrDeal.m_AuditMsg,vv[0].source_id);
					continue;

				}

				errFileMap.insert(map<string,string>::value_type(iter->first,it->second.szSourcePath)); //保留查找源文件用

				memset(errFile,0,sizeof(errFile));
				memset(errFiletmp,0,sizeof(errFiletmp));

				strcpy(errFile,mConfParam.szOutPath);								//先写临时文件在改名
				strcat(errFile,iter->first.c_str());
				strcat(errFile,".err");
				strcpy(errFiletmp,errFile);
				strcat(errFiletmp,".tmp");

				ofstream out(errFiletmp);
				if(!out)
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"写异常文件[%s]信息失败:%s ",errFiletmp,strerror(errno));
					theJSLog.writeLog(LOG_CODE_FILE_WRITE_ERR,erro_msg);
					//vsql.clear();
					continue;
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
					sprintf(mdrDeal.m_AuditMsg,"%s%d,%d;",mdrDeal.m_AuditMsg,vv[i].err_line,vv[i].err_col); //错误代码,错误行,错误列

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
				
				snprintf(mdrDeal.m_AuditMsg,sizeof(mdrDeal.m_AuditMsg),"%s|",mdrDeal.m_AuditMsg);

				memset(sql,0,sizeof(sql));
				sprintf(sql,"update d_errfile_info set state = 'Y' where filename = '%s'",iter->first);	//更新状态			
				vsql.push_back(sql);		

				//登记生成的错误表信息，2013-07-22错误的原始的文件都得登记
				memset(sql,0,sizeof(sql));
				sprintf(sql,"insert into d_out_file_reg(filename,source_id,reg_time,file_type,state) values('%s','%s','%s','%c','%c')",iter->first,vv[0].source_id,currTime,'N','W');
				vsql.push_back(sql);
				
				memset(sql,0,sizeof(sql));
				sprintf(sql,"insert into d_out_file_reg(filename,source_id,reg_time,file_type,state) values('%s.err','%s','%s','%c','%c')",iter->first,vv[0].source_id,currTime,'E','W');
				vsql.push_back(sql);		
		}	

		erroinfoMap.clear();						//清空错误信息
		
		ret = mdrDeal.IsAuditSuccess(mdrDeal.m_AuditMsg);
		if(ret)				//仲裁失败,回滚数据库,删除临时文件
		{
			vsql.clear();
			
			try
			{
				//将查到的文件状态置E
				stmt = conn.createStatement();
				memset(sql,0,sizeof(sql));
				sprintf(sql,"update d_errfile_info set state = 'E' where filename = :1 ");	//更新状态E
				stmt.setSQLString(sql);
				
				//删除临时文件
				theJSLog<<"删除临时错误异常信息文件"<<endi;
				//sleep(10);
				for(map< string,string >::const_iterator iter= errFileMap.begin();iter != errFileMap.end();++iter)
				{
					memset(errFiletmp,0,sizeof(errFiletmp));
					memset(errFile,0,sizeof(errFile));	
					strcpy(errFile,mConfParam.szOutPath);
					strcat(errFile,iter->first.c_str());
					strcat(errFile,".err");
					strcpy(errFiletmp,errFile);
					strcat(errFiletmp,".tmp");
					if(remove(errFiletmp))
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"临时文件[%s]删除失败: %s",errFiletmp,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
					}
					
					stmt<<iter->first;
					stmt.execute();
				}
				
				stmt.close();
			}
			catch (util_1_0::db::SQLException e)
			{
				stmt.rollback();
				stmt.close();
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"run() 数据库操作异常%s (%s)",e.what(),sql);
				theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
			}	

			conn.close();
			errFileMap.clear();

			theJSLog<<"######## end deal  ########\n"<<endi;

			continue ;
		}

		theJSLog<<"提交sql语句到数据库..."<<endi;	
		ret = updateDB();

		if(ret == -1)								//执行sql失败,将临时文件删除
		{
			for(map< string,string >::const_iterator iter= errFileMap.begin();iter != errFileMap.end();++iter)
			{
				memset(errFiletmp,0,sizeof(errFiletmp));
				memset(errFile,0,sizeof(errFile));	
				strcpy(errFile,mConfParam.szOutPath);
				strcat(errFile,iter->first.c_str());
				strcat(errFile,".err");
				strcpy(errFiletmp,errFile);
				strcat(errFiletmp,".tmp");
				if(remove(errFiletmp))
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"临时文件[%s]删除失败: %s",errFiletmp,strerror(errno));
					theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
				}	
			}
			
			errFileMap.clear();
			conn.close();		
			theJSLog<<"######## end deal ########\n"<<endi;

			continue ;
		}

		conn.close();

		theJSLog<<"将临时异常说明文件改为正式文件,并将文件从异常错误目录移到上传目录..."<<endi;
		for(map< string,string >::const_iterator iter= errFileMap.begin();iter != errFileMap.end();++iter)
		{
			memset(errFiletmp,0,sizeof(errFiletmp));
			memset(errFile,0,sizeof(errFile));	
			strcpy(errFile,mConfParam.szOutPath);
			strcat(errFile,iter->first.c_str());
			strcat(errFile,".err");
			strcpy(errFiletmp,errFile);
			strcat(errFiletmp,".tmp");
			if(rename(errFiletmp,errFile))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"临时文件[%s]改名正式失败: %s",errFiletmp,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_RENAME_ERR,erro_msg);
			}
			
			memset(tmp,0,sizeof(tmp));
			memset(outFileName,0,sizeof(outFileName));
			strcpy(tmp,iter->second.c_str());
			strcat(tmp,mConfParam.szInPath);
			strcat(tmp,iter->first.c_str());
			strcpy(outFileName,mConfParam.szOutPath);
			strcat(outFileName,iter->first.c_str());
			//cout<<"源文件:"<<tmp<<"  输出文件:"<<outFileName<<endl;

			if(rename(tmp,outFileName))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"移动异常源文件[%s]失败: %s",iter->first,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
			}

		}
		
		errFileMap.clear();

	}
/*
	catch (util_1_0::db::SQLException e)
	{
		stmt.rollback();
		stmt.close();
		conn.close();
		
		vsql.clear();
		erroinfoMap.clear();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() 数据库操作异常%s (%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
	}
*/
	catch (jsexcp::CException &e) 
	{	
		vsql.clear();
		erroinfoMap.clear();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		
	}
	
	theJSLog<<"######## end deal ########\n "<<endi;
	
	sleep(30);
  }//while(1)

}

//2013-10-11 新增退出函数
void ExceptionOut::prcExit()
{
	mdrDeal.dr_ReleaseDR();

	PS_Process::prcExit();
}

/*
//容灾初始化
bool ExceptionOut::drInit()
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
int ExceptionOut::drVarGetSet(char* serialString)
{
		int ret = 0;
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
			//mdrParam.m_AuditMsg = tmpVar;			//要仲裁的字符串
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
		//if(!m_syncDr.isMaster())thelog<<"备系统SerialString："<<mdrParam.m_SerialString<<endi;

		return ret;

}

//仲裁字符串
 int ExceptionOut::IsAuditSuccess(const char* dealresult)
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

bool ExceptionOut::CheckTriggerFile()
{
	int ret = 0;
	if(access(m_triggerFile.c_str(),F_OK) != 0)	return false;

	theJSLog<< "检查到trigger文件,并删除"<< m_triggerFile <<endi;

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

int main(int argc,char** argv)
{
	cout<<"********************************************* "<<endl;
	cout<<"*    China Telecom. Telephone Network         "<<endl;
	cout<<"*    InterNational Account Settle System      "<<endl;
	cout<<"*                                             "<<endl;
	cout<<"*           jserrout                          "<<endl;
	cout<<"*           sys.GJZW.Version 1.0	             "<<endl;
	cout<<"*    last update time :  2013-12-16 by  hed	 "<<endl;
	cout<<"********************************************* "<<endl;

	ExceptionOut fm ;

	if( !fm.init( argc, argv ) )
	{
		 return -1;
	}
        
	//while(1)
	//{
		//theJSLog.reSetLog();
		fm.run();
		//sleep(30);
	//}

   return 0;
}


