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
//SGW_RTInfo	rtinfo;

ExceptionOut::ExceptionOut()
{  
	//petri_status = 0;
	
	m_enable  = false;

	memset(m_szFileName,0,sizeof(m_szFileName));
	memset(input_path,0,sizeof(input_path));
	memset(out_path,0,sizeof(out_path));
	memset(m_szSourceID,0,sizeof(m_szSourceID));
	memset(outFileName,0,sizeof(outFileName));
	memset(currTime,0,sizeof(currTime));
	memset(erro_msg,0,sizeof(erro_msg));

}


ExceptionOut::~ExceptionOut()
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


//模块初始化动作
bool ExceptionOut::init(int argc,char** argv)
{  
    if(!PS_Process::init(argc,argv))
    {
      return false;
    }
	
	//PS_Process::setSignalPrc(); 

	//获取petri网状态,当系统为只读态时,数据库更新操作语句写文件
	
	//if(!(rtinfo.connect()))
	//{
	//	return false;
	//}
	//short status;
	//rtinfo.getSysMode(petri_status);
	//cout<<"petri status:"<<status<<endl;


	//*********2013-07-15 读取数据库的配置信息，如数据源组，错误目录 2013-03-11 add by hed*********************************************/
	
	if(!(dbConnect(conn)))
	{
		cout<<"连接数据库 connect error."<<endl;	//写日志
		return false ;
	}

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
			cout<<"异常文件输出模块输入相对路径没有配置,请在c_global_env中配置变量EXCEPTION_INPUT"<<endl;		
			return false ;
		}		
		completeDir(input_path);
		
		sql = "select varvalue from c_global_env where varname = 'EXCEPTION_OUTPUT'";
		stmt.setSQLString(sql);
		stmt.execute();
		if(!(stmt>>out_path))
		{
			cout<<"异常文件输出模块输出相对路径没有配置,请在c_global_env中配置变量EXCEPTION_OUTPUT"<<endl;		
			return false ;
		}		
		completeDir(out_path);
					
		stmt.close();
	   
	   }catch(SQLException e)
		{
			cout<<"init 数据库查询异常:"<<e.what()<<endl;
			return false ;
		}


	//判断日志目录和sql目录是否存在
	 DIR *dirptr = NULL; 
	 
	 if((dirptr=opendir(out_path)) == NULL)
	 {
		cout<<"输出目录"<<out_path<<"打开失败"<<endl;
		return false ;

	 }else closedir(dirptr);


	//初始化内存日志接口
	bool bb = initializeLog(argc,argv,false);  //是否调试模式
	if(!bb)
	{
			//cout<<"初始化内存日志接口失败"<<endl;
			return false;
	}
	
	theJSLog.setLog(szLogPath, szLogLevel,"HED" , "ERROUT", 001);	//文件日志接口，调用了内存日志接口
	
	theJSLog<<"相对输入路径："<<input_path<<"  输出路径:"<<out_path<<"	日志路径:"
			<<szLogPath<<" 日志级别:"<<szLogLevel<<" sql存放路径:"<<sql_path<<endi;

	if(LoadSourceCfg() == -1) 
	{		
		return false ;  
	}
	
	conn.close();

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
	
   if(!drInit)		return false;

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



//扫描错误文件信息登记表，生成异常说明文件(源文件名+.err),并将错单文件返回，文件级错误生成一条错误信息，记录级错误生成多条
void ExceptionOut::run()
{
	//cout<<"程序运行！！！！！！"<<endl;
	
	if(gbExitSig)
	{
		if(gbExitSig) theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "应用程序收到退出信号");
		PS_Process::prcExit();
		return;
	}
	
	int ret = 0;
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


		if(drStatus == 1)		//主备系统
		{
			isWriteSQLFileByTime();		
			//检查trigger触发文件是否存在
			if(!CheckTriggerFile())
			{
				sleep(1);
				return ;
			}

			ret = drVarGetSet(m_SerialString);
			if(ret)
			{
				theJSLog<<"备系统同步失败...."<<endi;
				return ;
			}
			
			//获取同步变量
			vector<string> data;		
			splitString(m_SerialString,";",data,true,true);  //发送的字符串: 文件名|文件名|sqlFile文件名

			isWriteSQLFileByMain(data[data.size()-1].c_str());	//备系统的sqlFile是通过主系统传过来的
			
			bool flag  = false;

			memset(m_AuditMsg,0,sizeof(m_AuditMsg));

			memset(sql,0,sizeof(sql));
			sprintf(sql,"select source_id,fileName,err_msg,err_code,err_col,err_line from d_errfile_info where state = 'W' and fileName = :1");		
			Statement stmt = conn.createStatement();
			stmt.setSQLString(sql);
			for(int i = 0;i<data.size()-1;i++)
			{
				flag = false ;

				stmt<<data[i];
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
					
				  flag = true;
				}
				
				if(!flag)  
				{
					theJSLog<<"文件名:"<<data[i]<<"无法找到"<<endi;
					sprintf(m_AuditMsg,"%s not find|",m_AuditMsg);
				}
 			}

		}
		else
		{
			isWriteSQLFile();				//是否提交sql文件
			
			memset(m_SerialString,0,sizeof(m_SerialString));

			memset(sql,0,sizeof(sql));
			sprintf(sql,"select source_id,fileName,err_msg,err_code,err_col,err_line from d_errfile_info where state = 'W' and rownum < 51");		
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

					sprintf(m_SerialString,"%s%s|",m_SerialString,errinfo.filename);
				}
				else
				{
					(iter->second).push_back(errinfo);
				}
			
			}

			stmt.close();

			if(erroinfoMap.size() == 0) return ;
			
			sprintf(m_SerialString,"%s%s|",m_SerialString,sqlFile);

			ret = drVarGetSet(m_SerialString);
			if(ret)
			{
				theJSLog<<"主系统同步失败...."<<endi;
				return ;
			}
			
			memset(m_AuditMsg,0,sizeof(m_AuditMsg));
		}
		
		conn.close();
		
		
		char errFile[1024],errFiletmp[1024],tmp[256];
		int pas_count = 1;			//文件传递次数
		int p1, p2;
		
		map<string,string> errFileMap;
		
		getCurTime(currTime);	

		//设置仲裁信息 文件名,记录条数,错误级别
		for(map< string,vector<ERRINFO> >::const_iterator iter= erroinfoMap.begin();iter != erroinfoMap.end();++iter)
		{
				theJSLog<<"处理异常文件: "<<iter->first<<endi;
				
				memset(sql,0,sizeof(sql));
				sprintf(sql,"update d_errfile_info set state = 'H' where filename = '%s'",iter->first);	//更新状态H	
				writeSQL(sql);
				
				vector<ERRINFO>	vv = iter->second;	
				
				sprintf(m_AuditMsg,"%s%s,%d,%s|",m_AuditMsg,iter->first,vv.size(),vv[0].err_msg);

				it = m_SourceCfg.find(vv[0].source_id);					//找到数据源所在的路径
				if( it == m_SourceCfg.end())
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"没有找到文件[%s]的数据源[%s]的信息！",iter->first,vv[0].source_id);
					theJSLog.writeLog(LOG_CODE_PARAM_INFO_LACK,erro_msg);	// 参数配置信息缺失

					sprintf(m_AuditMsg,"%s%s not find;",m_AuditMsg,vv[0].source_id);
					continue;

				}

				errFileMap.insert(map<string,string>::value_type(iter->first,it->second.szSourcePath));

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
					sprintf(m_AuditMsg,"%s%s not write[%s]|",m_AuditMsg,errFiletmp,strerror(errno));
					continue;
				}
				
				//getCurTime(currTime);					
				
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
				
				//rename(errFiletmp,errFile);				//文件改名
				
				memset(sql,0,sizeof(sql));
				sprintf(sql,"update d_errfile_info set state = 'Y' where filename = '%s'",iter->first);	//更新状态	
				
				writeSQL(sql);
				

				//登记生成的错误表信息，2013-07-22错误的原始的文件都得登记
				memset(sql,0,sizeof(sql));
				sprintf(sql,"insert into d_out_file_reg(filename,source_id,reg_time,file_type,state) values('%s','%s','%s','%c','%c')",iter->first,vv[0].source_id,currTime,'E','W');
				writeSQL(sql);
				
				memset(sql,0,sizeof(sql));
				sprintf(sql,"insert into d_out_file_reg(filename,source_id,reg_time,file_type,state) values('%s.err','%s','%s','%c','%c')",iter->first,vv[0].source_id,currTime,'N','W');
				writeSQL(sql);
			
			/*
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
			*/				
				
		}
		
		erroinfoMap.clear();		//清空错误信息
		
		//仲裁.....
		if(!IsAuditSuccess(m_AuditMsg))				//仲裁失败,回滚数据库,删除临时文件
		{
			rollBackSQL();

			//删除临时文件
			for(map< string,string >::const_iterator iter= errFileMap.begin();iter != errFileMap.end();++iter)
			{
				memset(errFiletmp,0,sizeof(errFiletmp));
				memset(errFile,0,sizeof(errFile));	
				strcpy(errFile,out_path);
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

			return ;
		}

		theJSLog<<"提交sql语句到文件..."<<endi;
		commitSQLFile();

		//临时文件写正式文件 并将原始文件移到输出目录
		theJSLog<<"将临时文件改为正式文件,并将文件备份目录移到上传目录..."<<endi;
		for(map< string,string >::const_iterator iter= errFileMap.begin();iter != errFileMap.end();++iter)
		{
			memset(errFiletmp,0,sizeof(errFiletmp));
			memset(errFile,0,sizeof(errFile));	
			strcpy(errFile,out_path);
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
			strcat(tmp,input_path);
			strcat(tmp,iter->first.c_str());
			strcpy(outFileName,out_path);
			strcat(outFileName,iter->first.c_str());

			//cout<<"源文件:"<<tmp<<"  输出文件:"<<outFileName<<endl;

			if(rename(tmp,outFileName))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"移动异常源文件[%s]失败: %s",iter->first,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
			}

		}


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
int ExceptionOut::drVarGetSet(char* serialString)
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
 bool ExceptionOut::IsAuditSuccess(const char* dealresult)
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

bool ExceptionOut::CheckTriggerFile()
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

int main(int argc,char** argv)
{
	cout<<"********************************************** "<<endl;
	cout<<"*    GuangDong Telecom. Telephone Network    * "<<endl;
	cout<<"*       Centralized Settlement System        * "<<endl;
	cout<<"*                                            * "<<endl;
	cout<<"*            errout                          * "<<endl;
	cout<<"*              Version 1.0	                * "<<endl;
	cout<<"*    last update time :  2013-08-29 by  hed	* "<<endl;
	cout<<"********************************************** "<<endl;


	ExceptionOut fm ;


	if( !fm.init( argc, argv ) )
	{
		 return -1;
	}
        
	while(1)
	{
		fm.run();
		sleep(60);
	}

   return 0;
}


