//用于构造日汇总条件,将格式化的文件记录添加到核对列表中去

#include<iostream>
#include <vector>

#include "CF_Common.h"
#include "CF_CLogger.h"

#include "CfgParam.h"
#include<dirent.h>

using namespace tpss;  //和psutil.h对应
using namespace std;

CLog theJSLog;

char mdate[8+1],msource[6],last_date[8+1],date_path[512];
bool gbExitSig=false;

void sig_prc(int sig_type)
{
	cerr<<"  接收到退出信号!!!\n";
	signal(sig_type, sig_prc);
	gbExitSig=true;
}

void setSignalPrc()
{
	for(int i=1; i<256; i++)
	{
		switch(i)
		{
		case SIGINT:
		case SIGQUIT:
		case SIGTERM:
			signal(i, sig_prc);
			break;
		//case SIGUSR1:
		//	signal(i, usersig_prc);
		//	break;
		//case SIGCHLD:
		//	break;
		default:
			signal(i, SIG_IGN);
		}
	}
}

void printV()
{
	cout<<"命令参数不对,自动生成汇总条件程序\n"<<endl;

	cout<<"jsSummaryCondition -all "<<endl;
	cout<<"jsSummaryCondition -t 汇总日期 -s  汇总数据源 \n"<<endl;
}


//输入参数-s 数据源   -t 日期
int checkArg(int argc,char** argv)
{
	//printf("\nxxxxx\n");
	if(argc < 2)
	{
		printV();
		return -1;
	}

	int ret = 0;
	bool one_flag = false,all_flag = false;

	memset(msource,0,sizeof(msource));
	memset(mdate,0,sizeof(mdate));
	
	for(int i = 1;i<argc;i++)
	{
		if(strcmp(argv[i],"-s") == 0)
		{	
			if(argc < (i+2))
			{
				cout<<"-s 后面请跟数据源"<<endl;
				ret = -1;
			}

			strcpy(msource,argv[i+1]);

			one_flag = true;
		}
		else if(strcmp(argv[i],"-t") == 0)
		{
			if(argc < (i+2))
			{
				cout<<"-t 后面请跟时间"<<endl;
				ret =  -1;
			}

			strcpy(mdate,argv[i+1]);
			one_flag = true;
		}
		else if(strcmp(argv[i],"-all") == 0)
		{
			all_flag = true;
		}
	}
	
	if((ret == -1) || ((one_flag && all_flag) ||(!one_flag && !all_flag)))
	{
		printV();

		return -1;
	}
	
	return ret;
}

/*
//先查询内存,再查询文件
bool checkAuditBefore(char* date)
{
	if(strcmp(date,last_date) == 0)  return true;
	
	char szBuff[1024];
	ifstream in(date_path,ios::in) ;			//供读，文件不存在则创建
	if(!in)
	{
		theJSLog<<"文件["<<date_path<<"]打开失败"<<endw;
		return false;
	}

	memset(szBuff,0,sizeof(szBuff));
	while(in.getline(szBuff,sizeof(szBuff)))   
	{	
		if(strcmp(szBuff,date) == 0) 
		{
			in.close();
			return true;
		}
		memset(szBuff,0,sizeof(szBuff));
	}

	in.close();

	return false;
}
*/

int main(int argc,char** argv)
{
	cout<<"********************************************* "<<endl;
	cout<<"*    China Telecom. Telephone Network         "<<endl;
	cout<<"*    InterNational Account Settle System      "<<endl;
	cout<<"*                                             "<<endl;
	cout<<"*           jsSummaryCondition               "<<endl;
	cout<<"*           sys.GJZW.Version 1.0	             "<<endl;
	cout<<"*     created time :      2013-12-19 by  hed	 "<<endl;
	cout<<"*     last updaye time :  2013-12-21 by  hed	 "<<endl;
	cout<<"********************************************* "<<endl;
	
	if(checkArg(argc,argv))
	{
		return -1;
	}
	
	DBConnection conn;
	char erro_msg[1024],before_date[8+1],currTime[14+1];
/*
	memset(date_path,0,sizeof(date_path));
	strcpy(date_path,getenv("SETTLEDATA"));
	completeDir(date_path);
	sprintf(date_path,"%s.%s.tmp",date_path,"daySummaryCondition");
	theJSLog<<"日期存放全路径:"<<date_path<<endi;
*/	
	// 从核心参数里面读取日志的路径，级别
	IBC_ParamCfgMng param_cfg;
	
	if( !param_cfg.bOnInit() )		//核心参数需要自己初始化
	{
		string sErr;
		int nCodeId;
		param_cfg.getError(sErr,nCodeId);
		cerr<<"参数配置接口初始化失败！错误码="<<nCodeId<<", 错误信息="<<sErr<<endl;
		return false;
	}

	 char sParamName[256],szLogPath[PATH_NAME_LEN+1],szLogLevel[10];
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

	 //判断目录是否存在
	 DIR *dirptr = NULL; 
	 if((dirptr=opendir(szLogPath)) == NULL)
	 {
		cout<<"日志目录["<<szLogPath<<"]打开失败"<<endl;	
		return false ;
	 }else closedir(dirptr);

	theJSLog.setLog(szLogPath, atoi(szLogLevel),"CONDITION","SUMMARY", 001);
	
	//setSignalPrc();

	if(!(dbConnect(conn)))
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"连接数据库失败 connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
		return false ;
	}
	
	int ret = 0,iSourceCount = 0;
	char m_szSrcGrpID[8],strSourceId[8];
	string sql ;
	Statement stmt;
	vector<string> vsource;

	try
	{	
		stmt = conn.createStatement();
		if(msource[0] == '\0')
		{
			sql = "select count(1) from c_source_group_define a ,C_SOURCE_GROUP_CONFIG b where a.SOURCE_GROUP = b.SOURCE_GROUP ";
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>iSourceCount;
			if(iSourceCount == 0)
			{
				sprintf(erro_msg,"init()  各个数据源组都没有配置数据源");
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return false ;
			}
			
			//查询有效的数据源个数
			sql = "select count(1) from c_source_group_define a,C_SOURCE_GROUP_CONFIG b,C_SUMTABLE_DEFINE c where a.SOURCE_GROUP = b.SOURCE_GROUP and b.source_id = c.sourceid and VALIDFLAG = 'Y' " ;
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>iSourceCount;
			if(iSourceCount == 0)
			{
				theJSLog<<"各个数据源组"<<"ID汇总都无效"<<endi;
				return false ;
			}
		
			theJSLog<<"有效数据源个数："<<iSourceCount<<endi;

			sql = "select a.source_group,b.SOURCE_ID from c_source_group_define a ,C_SOURCE_GROUP_CONFIG b ,C_SUMTABLE_DEFINE c where a.SOURCE_GROUP = b.SOURCE_GROUP and b.source_id = c.sourceid and VALIDFLAG = 'Y' ";
			stmt.setSQLString(sql);
			stmt.execute();
			memset(m_szSrcGrpID,0,sizeof(m_szSrcGrpID));
			memset(strSourceId,0,sizeof(strSourceId));
			int i = 0;
			while(stmt>>m_szSrcGrpID>>strSourceId)
			{	
				vsource.push_back(strSourceId);
				theJSLog<<"["<<i+1<<"]数据源:"<<strSourceId<<endi;

				memset(m_szSrcGrpID,0,sizeof(m_szSrcGrpID));
				memset(strSourceId,0,sizeof(strSourceId));
				i++;
			}
		}
		else
		{
			char sql[1024];
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from C_SOURCE_GROUP_CONFIG where source_id = '%s'",msource);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>iSourceCount;
			if(iSourceCount == 0)
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"init() 数据源[%s]没有配置",msource);
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return false;
			}

			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from C_SUMTABLE_DEFINE where sourceid = '%s' and VALIDFLAG = 'Y' ",msource);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>iSourceCount;
			if(iSourceCount == 0)
			{
				theJSLog<<"数据源ID"<<msource<<"无效"<<endi;
				return false ;
			}

			char group[10];
			memset(group,0,sizeof(group));
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select source_group from c_source_group_config where source_id = '%s'",msource);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>group;

			vsource.push_back(msource);
			theJSLog<<"数据源:"<<msource<<endi;
		}

		stmt.close();

	}catch (SQLException e)
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"数据库出错：%s(%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		return false;
	 }

	conn.close();

	//while(1)
	//{
		//if(gbExitSig)
		//{
			//theJSLog<<"进程退出"<<endw;
		//	break;
		//}
		
		if(mdate[0] == '\0')
		{
			getCurTime(currTime);
		
			//时间判断 先判断前天的有没有做过每天4点钟以后执行一次,执行后存放到内存,然后写文件
			char tmpDate[8+1];
			memset(tmpDate,0,sizeof(tmpDate));
			memset(before_date,0,sizeof(before_date));
			getCurTime(currTime);
			strncpy(tmpDate,currTime,8);

			memset(before_date,0,sizeof(before_date));
			addDays(-1,tmpDate,before_date);
			//if(checkAuditBefore(before_date))		//昨天已经核对则不用核对了
			//{
			//	sleep(3600);
				//continue ;   
			//}
		
			//char tmpTime[2+1];
			//memset(tmpTime,0,sizeof(tmpTime));
			//strncpy(tmpTime,currTime+8,2);
			//if(strcmp(tmpTime,"04") < 0)			//4点钟以后执行
			//{
			//	sleep(600);
				//continue ;
			//}
		}
		else
		{
			strcpy(before_date,mdate);
		}

		if(!(dbConnect(conn)))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"连接数据库失败 connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
			//sleep(60);
			//continue;
		}

		theJSLog<<"#############当前汇总条件日期["<<before_date<<"]########################"<<endi;
		
		try
		{
			stmt = conn.createStatement();
			char sql[1024];
			for(int i = 0;i<vsource.size();i++)
			{
				theJSLog<<"处理数据源:"<<vsource[i]<<endi;

				//先删除当天的 D_CHECK_FILE_DETAIL 表的数据,然后将D_SCH_FORMAT表当天数据源的数据插入到该表中
				memset(sql,0,sizeof(sql));
				sprintf(sql,"delete from D_CHECK_FILE_DETAIL a where a.source_id = '%s' and a.deal_time like '%s%s'",vsource[i],before_date,"%");
				theJSLog<<"先清空当天的文件信息:"<<sql<<endi;
				stmt.setSQLString(sql);
				stmt.execute();
				
				memset(sql,0,sizeof(sql));
				sprintf(sql,"insert into D_CHECK_FILE_DETAIL(CHECK_FILE,CONTENT,CHECK_FLAG,DEAL_TIME,RATE_CYCLE,CHECK_TYPE,FILE_TIME,SOURCE_ID) "
							"select '%s.AUD',a.filename,'Y',a.recieve_time,substr(a.filename,-6,6),'AUD',a.file_time ,a.source_id from D_SCH_FORMAT a "
							"where a.source_id = '%s' and a.file_time = '%s'",before_date,vsource[i],before_date);
				theJSLog<<"将调度表中的数据插入到核对表:"<<sql<<endi;
				stmt.setSQLString(sql);
				stmt.execute();
			}
			
			stmt.close();		
		}
		catch (SQLException e)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"数据库出错：%s(%s)",e.what(),sql);
			theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
			//continue;
		}
		
		conn.close();
		
		/*
		if(mdate[0] == '\0')
		{
			memset(last_date,0,sizeof(last_date));
			strcpy(last_date,before_date);
			ofstream out(date_path,ios::app);
			if(!out)
			{
				theJSLog.writeLog(LOG_CODE_FILE_WRITE_ERR,"记录审核日期文件信息失败");
			}
			else
			{
				out<<last_date<<"\n";
			}	
			
			out.close();
			theJSLog<<"#############处理完成########################"<<endi;
		}
		else
		{
			theJSLog<<"#############处理完成########################"<<endi;
			//break;
		}

		sleep(600);
		*/
	//}


	return 0;
}