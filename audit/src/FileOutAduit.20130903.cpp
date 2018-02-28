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

SGW_RTInfo	rtinfo;

FileOutAduit::FileOutAduit()
{  

	memset(m_szFileName,0,sizeof(m_szFileName));
	memset(currTime,0,sizeof(currTime));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(sql,0,sizeof(sql));
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
	
	//PS_Process::setSignalPrc(); 

	if(!(rtinfo.connect()))
	{
		return false;
	}
	
	rtinfo.getDBSysMode(petri_status);
	cout<<"petri status:"<<petri_status<<endl;


	//初始化内存日志接口
	bool bb = initializeLog(argc,argv,false);  //是否调试模式
	if(!bb)
	{
			//cout<<"初始化内存日志接口失败"<<endl;
			return false;
	}
	
	theJSLog.setLog(szLogPath,szLogLevel,"HED" , "ADUIT", 001);	//文件日志接口，调用了内存日志接口
	
	theJSLog<<"日志路径:"<<szLogPath<<szLogPath<<" 日志级别:"<<szLogLevel<<endi;
  
	try
	{	
		if(!(dbConnect(conn)))
		{
			memset(erro_msg,0,sizeof(erro_msg));
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
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"初始化时数据库查询异常：%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
		return false;
	}

	DIR *dirptr = NULL; 

	//判断各个审核的文件的输出目录
	for(map< string,AduitEnv >::const_iterator it = fileNameMap.begin();it !=  fileNameMap.end();++it)
    {
		if((dirptr=opendir((it->second).out_path)) == NULL)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"审核数据源标志[%s]的的输出文件路径[%s]不存在",it->first,(it->second).out_path);
			theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错

			return false ;
		}else closedir(dirptr);

	}

   theJSLog<<"初始化完毕"<<endi;

   return true ;
}



//批量提交sql,保证一个事物完整性
int FileOutAduit::updateDB()
{	
	int ret = 0;
	Statement stmt;
    try
    {	
		stmt = conn.createStatement();
		for(int i =0;i<vsql.size();i++)
		{		
			stmt.setSQLString(vsql[i]);
			ret = stmt.execute();
		}
		stmt.close();

	}
	catch(util_1_0::db::SQLException e)
	{ 
		stmt.close();
		vsql.clear();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"updateDB 数据库出错：%s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
		return -1;
	}

	vsql.clear();
	
	return ret ;
}

//扫描输出文件登记表 列出当日传送的ERR文件的清单，作为核对依据,怎样保证每天只执行一次,当天核对的昨天的文件吗,怎么判断?
void FileOutAduit::run()
{
	int ret = 0;

	if(gbExitSig)
	{
		if(gbExitSig) theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "应用程序收到退出信号");
		PS_Process::prcExit();
		return;
	}

	try
	{
		//判断数据库状态
		rtinfo.getDBSysMode(petri_status);
		//cout<<"状态值:"<<petri_status<<endl;

		if(petri_status == DB_STATUS_OFFLINE)	return ;
		
		if(!(dbConnect(conn)))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"Ontask() 连接数据库失败 connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
			return  ;
		}
		
		int cnt = 0;
		Statement stmt = conn.createStatement();
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(1) from d_out_file_reg where file_type = 'E' and state = 'W' ");
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>cnt;
		
		if(cnt == 0)	return ;
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select fileName,source_id from d_out_file_reg where file_type = 'E' and state = 'W' ");		
		stmt.setSQLString(sql);
		stmt.execute();
		string filename ,source;
		while(stmt>>filename>>source)
		{	
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
								vsql.push_back(sql);

								break ;
							}
					}
			}
			
			filename = "";
			source = "";
		}

		stmt.close();
		
		char tmpTime[8+1],outFile[1024],tmp[1024];
		vector<string> fileList;	

		//将对应的数据源生成文件 上游数据源标志,文件记录数目,文件大小
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
			sprintf(m_szFileName,"ACCT_%s_D_%s_AUD",tmpTime,it3->first);	 //写核对文件

			memset(outFile,0,sizeof(outFile));
			strcpy(outFile,(it3->second).out_path);
			
			strcat(outFile,m_szFileName);

			fileList.push_back(outFile);		//保存正常文件名+路径
			
			strcat(outFile,".tmp");

			theJSLog<<"生成核对文件:"<<m_szFileName<<endi;
			ofstream out(outFile);
			if(!out)
			{
				vsql.clear();
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"run() 文件%s打开出错",m_szFileName);
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

		getCurTime(currTime);
		sprintf(sql,"update d_out_file_reg set state = 'Y' , deal_time = '%s' where state = 'H'",currTime);
		vsql.push_back(sql);
		
		ret = updateDB();

		conn.close();	

		if(ret == -1)
		{
			for(int i = 0;i<fileList.size();i++)
			{
					memset(tmp,0,sizeof(tmp));
					strcpy(tmp,fileList[i].c_str());
					strcat(tmp,".tmp");
					if(remove(tmp))
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"删除临时文件[%s]失败: %s",tmp,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
					}
			}
			return ;
		}

		theJSLog<<"将临时文件改为正式文件..."<<endi;

		for(int i = 0;i<fileList.size();i++)
		{
				strcpy(tmp,fileList[i].c_str());
				strcat(tmp,".tmp");
				if(rename(tmp,fileList[i].c_str()))
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"临时文件[%s]重命名为正式文件失败: %s",tmp,strerror(errno));
					theJSLog.writeLog(LOG_CODE_FILE_RENAME_ERR,erro_msg);		//移动文件失败
				}
		}
		
	}
	catch (util_1_0::db::SQLException e)
	{
		rollBackSQL();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() 数据库操作异常%s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
	}	
	catch (jsexcp::CException &e) 
	{	
		rollBackSQL();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		
	}

	return ;
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
			memset(erro_msg,0,sizeof(erro_msg));
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
	cout<<"*     created time :  2013-07-20 by  hed	    * "<<endl;
	cout<<"*     last updaye time :  2013-08-30 by  hed	* "<<endl;
	cout<<"********************************************** "<<endl;


	FileOutAduit fm ;

	if( !fm.init( argc, argv ) )
	{
		 return -1;
	}
        
	while(1)
	{
		fm.run();
		sleep(600);
	}

   return 0;
}


