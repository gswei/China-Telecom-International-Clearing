/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-07-22
File:			 FileOutAduit.cpp
Description:	 文件审核输出模块
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
#include "FileOutAduit.h"

#include "CF_Common.h"
#include "CF_CLogger.h"
CLog theJSLog;


FileOutAduit::FileOutAduit()
{  
	memset(m_szFileName,0,sizeof(m_szFileName));
	//memset(input_path,0,sizeof(input_path));
	//memset(out_path,0,sizeof(out_path));
	//memset(m_szSourceID,0,sizeof(m_szSourceID));
	//memset(outFileName,0,sizeof(outFileName));
	memset(currTime,0,sizeof(currTime));
    memset(sqlFile,0,sizeof(sqlFile));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(sql,0,sizeof(sql));
	//memset(currTime,0,sizeof(currTime));
}


FileOutAduit::~FileOutAduit()
{

}


//模块初始化动作
bool FileOutAduit::init(int argc,char** argv)
{
   
    if(!PS_Process::init(argc,argv))
    {
      return false;
    }
	
	PS_Process::setSignalPrc(); 

	//cout<<"流水线ID:"<<getFlowID()<<"   模块ID:"<<getModuleID()<<endl;

	 // 从核心参数里面读取日志的路径，级别，
	 char sParamName[256],szLogPath[PATH_NAME_LEN+1],szLogLevel[10],sql_path[1024];
	 CString sKeyVal;
	 sprintf(sParamName, "log.path");
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(szLogPath,0,sizeof(szLogPath));
		strcpy(szLogPath,(const char*)sKeyVal);

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
	
	 //sprintf(sParamName, "file.aduit.outpath");			//审核文件输出目录
	 //if(param_cfg.bGetMem(sParamName, sKeyVal))
	 //{
		//memset(szLogPath,0,sizeof(szLogPath));
		//strcpy(out_path,(const char*)sKeyVal);

	 //}
	 //else
	 //{	
	//	cout<<"请在核心参数里配置审核模块的输出路径"<<endl;
	//	return false ;
	 //}	 
	 	
	//判断日志目录和sql目录是否存在
	 DIR *dirptr = NULL; 
	 if((dirptr=opendir(szLogPath)) == NULL)
	 {
		cout<<"日志目录:"<<szLogPath<<"打开失败"<<endl;	
		return false ;
	 }else closedir(dirptr);

	 //if((dirptr=opendir(out_path)) == NULL)
	 //{
	 //	cout<<"输出目录"<<out_path<<"打开失败"<<endl;
	 //	return false ;

	 //}else closedir(dirptr);

	 //completeDir(out_path);

	//初始化内存日志接口
	bool bb = initializeLog(argc,argv,false);  //是否调试模式
	if(!bb)
	{
			//cout<<"初始化内存日志接口失败"<<endl;
			return false;
	}
	
	theJSLog.setLog(szLogPath, atoi(szLogLevel),"2013" , "ADUIT", 001);	//文件日志接口，调用了内存日志接口
	
	theJSLog<<"	日志路径："<<szLogPath<<" 日志级别："<<endi;
   
	
	try
	{	
		if(!(dbConnect(conn)))
		{
			sprintf(erro_msg,"init()  连接数据库失败 connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
			return false ;
		}

		//加载数据源信息，放入map中去
		string sql = "select aduit_flag,source_array,null_out_flag,out_path from c_source_audit_env ";
		Statement stmt = conn.createStatement();
		stmt.setSQLString(sql);
		stmt.execute();
		//***********************************************************************************/
		string str ;
		AduitEnv audit ;
		char source_array[256];
		memset(source_array,0,sizeof(source_array));

		while(stmt>>str>>source_array>>audit.null_out_flag>>audit.out_path)
		{
			 vector<string> array ,array2;
			 splitString(source_array,",",array,true);
			 audit.arrayFile = array2 ;
			 completeDir(audit.out_path);

			 sourceMap.insert(map< string,vector<string>  >::value_type(str,array));
			 fileNameMap.insert(map< string, AduitEnv>::value_type(str,audit));

			 memset(source_array,0,sizeof(source_array));
			 str = "";
		}

		stmt.close();

		conn.close();	
	}
	catch(util_1_0::db::SQLException e)
	{ 
		sprintf(erro_msg,"初始化时数据库查询异常：%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
		return false;
	}

	//判断各个审核的文件的输出目录
	for(map< string,AduitEnv >::const_iterator it = fileNameMap.begin();it !=  fileNameMap.end();++it)
    {
		if((dirptr=opendir((it->second).out_path)) == NULL)
		{
			//cout<<"审核数据源标志:"<<it->first<<"的输出文件目录["<<(it->second).out_path<<"]打开失败"<<endl;
			sprintf(erro_msg,"审核数据源标志[%s]的的输出文件路径[%s]不存在",it->first,(it->second).out_path);
			theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错

			return false ;
		}else closedir(dirptr);

	}


   //核心参数获取
   //CString sKeyVal;
   //param_cfg.bGetMem("memory.MT_DSPCH.sem_key_value", sKeyVal);
   //cout<<"获取的核心参数值:"<<sKeyVal<<endl;

   //测试写日志
   //writeLog(0,"测试写日志");

   //conn.close();

   //cerr<<"初始化完毕！\n";
   
   theJSLog<<"初始化完毕"<<endi;

   return true ;
}



//数据库的更新操作，便于捕获异常sql 便于重现
int FileOutAduit::updateDB(char* sql)
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
		sprintf(erro_msg,"updateDB 数据库出错：%s",e.what());
		//memset(erro_msg,sizeof(erro_msg));
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
		return -1;
	}

	return 0;
}


//查看sql文件中是否有数据，若有则执行
int FileOutAduit::scanSQLFile()
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

	memset(szBuff,0,sizeof(szBuff));
	while(in.getline(szBuff,sizeof(szBuff)))   
	{		
			//cout<<"残留的SQL文件"<<szBuff<<endl;
			updateDB(szBuff);   //执行文件中的sql语句
			memset(szBuff,0,sizeof(szBuff));
			
	}
	
	in.close();

	in.open(sqlFile,ios::trunc);
	in.close();

	return 0;

}

//扫描输出文件登记表 列出当日传送的ERR文件的清单，作为核对依据
void FileOutAduit::run()
{
	cout<<"程序运行！！！！！！"<<endl;

	if(gbExitSig)
	{
		if(gbExitSig) theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "应用程序收到退出信号");
		PS_Process::prcExit();
		return;
	}

	try
	{
		if(!(dbConnect(conn)))
		{
			//cout<<"连接数据库失败 connect error."<<endl;
			sprintf(erro_msg,"Ontask() 连接数据库失败 connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
			return  ;
		}

		sprintf(sql,"select fileName,source_id from d_out_file_reg where file_type = 'E' and state = 'W' ");				
		Statement stmt = conn.createStatement();
		stmt.setSQLString(sql);
		stmt.execute();
		string filename ,source;
		while(stmt>>filename>>source)
		{	
			//fileList.push_back(str);
			for(map< string,vector<string> >::const_iterator it=sourceMap.begin();it!=sourceMap.end();++it)
			{
					vector<string> array = it->second;
					for(int i = 0;i<array.size();i++)
					{
							if(array[i] == source)		//找到数据源所属的结算种类
							{
								map< string,AduitEnv >::iterator it2 = fileNameMap.find(it->first);						
							    (it2->second).arrayFile.push_back(filename);
								
								sprintf(sql,"update d_out_file_reg set state = 'H' where file_type = 'E' and source_id = '%s' and filename = '%s'",source,filename);
								updateDB(sql);

								break ;
							}

					}
			}
			
			filename = "";
			source = "";
		}

		stmt.close();
		char tmpTime[8+1],outFile[1024];
		//将对应的数据源生成文件
		for(map< string,AduitEnv >::const_iterator it3=fileNameMap.begin() ;it3!=fileNameMap.end();++it3)
		{
			vector<string> arrayFile = (it3->second).arrayFile;
			if(arrayFile.size() == 0)
			{		
				    if((it3->second).null_out_flag == 'N')	continue ;		
			}
			
			getCurTime(currTime);
			memset(tmpTime,0,sizeof(tmpTime));
			strncpy(tmpTime,currTime,8);
			sprintf(m_szFileName,"ACC_%s_D_%s_AUD",tmpTime,it3->first);	 //写核对文件
			
			memset(outFile,0,sizeof(outFile));
			strcpy(outFile,(it3->second).out_path);
			
			strcat(outFile,m_szFileName);

			theJSLog<<"生成核对文件"<<m_szFileName<<endi;
			ofstream out(outFile);
			if(!out)
			{
				sprintf(erro_msg,"writeFile 文件%s打开出错",m_szFileName);
				theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败
				return ;
			}
		
			//写文件头
			out<<10<<","<<currTime<<","<<tmpTime<<","<<arrayFile.size()<<endl;
			for(int k = 0;k<arrayFile.size();k++)
			{		
				out<<arrayFile[k]<<"\n";					
			}		
		
			out<<"90"<<endl;

			out.close();
			

		}

		sprintf(sql,"update d_out_file_reg set state = 'Y' , deal_time = '%s' where state = 'H'",currTime);
		updateDB(sql);

		conn.close();

	}
	catch (util_1_0::db::SQLException e)
	{
		sprintf(erro_msg,"run() 数据库操作异常%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
	}	
	catch (jsexcp::CException &e) 
	{	
		sprintf(erro_msg,"run() %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		
	}

}

//写文件
int FileOutAduit::writeFile()
{
		int ret = 0;
		char tmp[1024],source_flag[10];
		memset(tmp,0,sizeof(tmp));
		memset(source_flag,0,sizeof(source_flag));
		
		getCurTime(currTime);

		//文件名
		strncpy(tmp,currTime,8);
		sprintf(m_szFileName,"ACC_%s_D_%s_AUD",tmp,source_flag);
		getCurTime(currTime);

		ofstream out(m_szFileName);
		if(!out)
		{
			sprintf(erro_msg,"writeFile 文件%s打开出错",m_szFileName);
			theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败
			return -1;
		}
		
		//写文件头
		out<<10<<","<<currTime<<","<<tmp<<","<<fileList.size()<<endl;
		for(int k = 0;k<fileList.size();k++)
		{		
			out<<fileList[k]<<"\n";					
		}		
		
		out<<"90"<<endl;

		out.close();		
		return ret ;
}


int main(int argc,char** argv)
{
	cout<<"********************************************** "<<endl;
	cout<<"*    GuangDong Telecom. Telephone Network    * "<<endl;
	cout<<"*       Centralized Settlement System        * "<<endl;
	cout<<"*                                            * "<<endl;
	cout<<"*            fileOutAduit                    * "<<endl;
	cout<<"*              Version 1.0	                * "<<endl;
	cout<<"*     created time :  2013-07-22 by  hed	    * "<<endl;
	cout<<"********************************************** "<<endl;


	FileOutAduit fm ;


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


