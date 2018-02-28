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
	memset(currTime,0,sizeof(currTime));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(sql,0,sizeof(sql));
}


FileOutAduit::~FileOutAduit()
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
bool FileOutAduit::init(int argc,char** argv)
{
   
    if(!PS_Process::init(argc,argv))
    {
      return false;
    }
	
	//PS_Process::setSignalPrc(); 

	//初始化内存日志接口
	bool bb = initializeLog(argc,argv,false);  //是否调试模式
	if(!bb)
	{
			//cout<<"初始化内存日志接口失败"<<endl;
			return false;
	}
	
	theJSLog.setLog(szLogPath,szLogLevel,"HED" , "ADUIT", 001);	//文件日志接口，调用了内存日志接口
	
	theJSLog<<"	日志路径:"<<szLogPath<<szLogPath<<" 日志级别:"<<szLogLevel<<endi;
  
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
   
   if(!drInit)		return false;

   theJSLog<<"初始化完毕"<<endi;

   return true ;
}

//扫描输出文件登记表 列出当日传送的ERR文件的清单，作为核对依据
void FileOutAduit::run()
{
	//cout<<"程序运行！！！！！！"<<endl;
	
	int ret = 0;
	if(gbExitSig)
	{
		if(gbExitSig) theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "应用程序收到退出信号");
		PS_Process::prcExit();
		return;
	}

	try
	{
		if(drStatus == 1)
		{
			isWriteSQLFileByTime();		
			//检查trigger触发文件是否存在
			if(!CheckTriggerFile())
			{
				sleep(1);
				return ;
			}
		}
		else
		{
			isWriteSQLFile();				//是否提交sql文件

		}

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
		
		//if(cnt == 0)	return 0;
		
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
								writeSQL(sql);

								break ;
							}
					}
			}
			
			filename = "";
			source = "";
		}

		stmt.close();
		conn.close();	

		char tmpTime[8+1],outFile[1024];
		

		memset(m_SerialString,0,sizeof(m_SerialString));
		memset(tmpTime,0,sizeof(tmpTime));

		//同步变量: 记录条数,传输时间,sqlFile
		if(drStatus ==1)
		{	
			ret = drVarGetSet(m_SerialString);
			if(ret)
			{
				theJSLog<<"备系统同步失败...."<<endi;
				return ;
			}
			
			//获取同步变量
			vector<string> data;		
			splitString(m_SerialString,";",data,true,true);  //发送的字符串数据源ID,文件名,sqlFile文件名
			
			isWriteSQLFileByMain(data[2].c_str());	//备系统的sqlFile是通过主系统传过来的
			
			strcpy(tmpTime,data[1].c_str());
		}
		else
		{
			getCurTime(currTime);	
			memset(tmpTime,0,sizeof(tmpTime));
			strncpy(tmpTime,currTime,8);

			sprintf(m_SerialString,"%d;%s;%s",cnt,currTime,sqlFile);
			ret = drVarGetSet(m_SerialString);
			if(ret)
			{
				theJSLog<<"主系统同步失败...."<<endi;
				return ;
			}
		}
		
		memset(m_AuditMsg,0,sizeof(m_AuditMsg));
		
		vector<string> fileList;	

		//将对应的数据源生成文件 上游数据源标志,文件记录数目,文件大小
		for(map< string,AduitEnv >::const_iterator it3=fileNameMap.begin() ;it3!=fileNameMap.end();++it3)
		{
			vector<string> arrayFile = (it3->second).arrayFile;
			sprintf(m_szFileName,"ACCT_%s_D_%s_AUD",tmpTime,it3->first);	 //写核对文件


			sprintf(m_AuditMsg,"%s%s,%d,%c|",m_AuditMsg,m_szFileName,arrayFile.size(),(it3->second).null_out_flag);

			if(arrayFile.size() == 0)
			{		
				 if((it3->second).null_out_flag == 'N')	continue ;		
			}
					
			memset(outFile,0,sizeof(outFile));
			strcpy(outFile,(it3->second).out_path);
			
			strcat(outFile,m_szFileName);

			fileList.push_back(outFile);		//保存正常文件名+路径
			
			strcat(outFile,".tmp");

			theJSLog<<"生成核对文件"<<m_szFileName<<endi;
			ofstream out(outFile);
			if(!out)
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"writeFile 文件%s打开出错",m_szFileName);
				theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败
				//return ;
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
		writeSQL(sql);
		
		//核对信息,
		char tmp[1024];
		if(!IsAuditSuccess(m_AuditMsg))				//仲裁失败,回滚数据库,删除临时文件
		{
			rollBackSQL();
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
		
		theJSLog<<"提交sql语句到文件..."<<endi;
		commitSQLFile();

		//临时文件写正式文件
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
		
		PS_Process::prcExit();
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


//容灾初始化
bool FileOutAduit::drInit()
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
int FileOutAduit::drVarGetSet(char* serialString)
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
 bool FileOutAduit::IsAuditSuccess(const char* dealresult)
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

bool FileOutAduit::CheckTriggerFile()
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
	cout<<"*            fileOutAduit                    * "<<endl;
	cout<<"*              Version 1.0	                * "<<endl;
	cout<<"*     created time :  2013-07-20 by  hed	    * "<<endl;
	cout<<"*     last updaye time :  2013-08-27 by  hed	* "<<endl;
	cout<<"********************************************** "<<endl;


	FileOutAduit fm ;


	if( !fm.init( argc, argv ) )
	{
		 return -1;
	}
        
	//while(1)
	//{
		fm.run();
		//sleep(60);
	//}

   return 0;
}


