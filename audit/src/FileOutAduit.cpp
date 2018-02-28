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
//#include <string>
//#include <vector>
#include <sys/types.h>
#include <sys/stat.h>  //stat()函数，查询文件信息
#include<unistd.h>     //读取当前程序运行目录
//#include<iostream>
#include<fstream>
#include "FileOutAduit.h"

//#include "CF_Common.h"
//#include "CF_CLogger.h"
CLog theJSLog;

SGW_RTInfo	rtinfo;
char last_date[9],date_path[512];

FileOutAduit::FileOutAduit()
{  
	memset(m_szFileName,0,sizeof(m_szFileName));
	memset(currTime,0,sizeof(currTime));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(sql,0,sizeof(sql));
}

FileOutAduit::~FileOutAduit()
{
	mdrDeal.dr_ReleaseDR();
}

//模块初始化动作
bool FileOutAduit::init(int argc,char** argv)
{
   
    if(!PS_Process::init(argc,argv))
    {
      return false;
    }
	
	mConfParam.iflowID = getFlowID();
	mConfParam.iModuleId = getModuleID();

	try
	{	
		if(!(dbConnect(conn)))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"init()  连接数据库失败 connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
			return false ;
		}
		
		string sql;
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

		//加载数据源信息，放入map中去
		sql = "select aduit_flag,source_array,null_out_flag,out_path from c_source_audit_env ";	
		stmt.setSQLString(sql);
		stmt.execute();
		//***********************************************************************************/
		string str ;
		AduitEnv audit ;
		char source_array[256];
		memset(source_array,0,sizeof(source_array));

		while(stmt>>str>>source_array>>audit.null_out_flag>>audit.out_path)
		{
			 theJSLog<<"audit_flag="<<str<<" source_array="<<source_array<<" null_out_flag="<<audit.null_out_flag<<" out_path="<<audit.out_path<<endi;

			 vector<string> array ,array2;
			 splitString(source_array,";",array,true);
			 audit.arrayFile = array2 ;										//可考虑用map代替
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
		sprintf(erro_msg,"初始化时数据库查询异常：%s (%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
		return false;
	}
	
	memset(date_path,0,sizeof(date_path));
	strcpy(date_path,getenv("SETTLEDATA"));
	completeDir(date_path);
	sprintf(date_path,"%s.%s.tmp",date_path,module_name);
	theJSLog<<"日期存放全路径:"<<date_path<<endi;

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

	if(!(rtinfo.connect()))
	{
		theJSLog.writeLog(0,"连接运行时信息失败");
		return false;
	}
	rtinfo.getDBSysMode(petri_status);
	theJSLog<<"petri status:"<<petri_status<<endi;

	//theJSLog.setLog(szLogPath,szLogLevel,"AUDIT" , "GJJS", 001);	//文件日志接口，调用了内存日志接口
	//theJSLog<<"日志路径:"<<szLogPath<<szLogPath<<" 日志级别:"<<szLogLevel<<endi;

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


//扫描输出文件登记表 列出当日传送的ERR文件的清单，作为核对依据,怎样保证每天只执行一次,当天核对的昨天的文件吗,怎么判断?
void FileOutAduit::run()
{
	//int ret = 0;
	int ret = 0,event_sn, event_type;
	long param1, param2, src_id;
	short db_status = 0;
	char before_date[8+1];

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
		if(petri_status == DB_STATUS_OFFLINE)	
		{
			sleep(60);
			continue ;
		}
		
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
				theJSLog<<"备系统同步失败...."<<endi;
				continue ;
			}
			
			//获取同步变量
			//vector<string> data;		
			//splitString(mdrParam.m_SerialString,";",data,true,true);  //发送的字符串: 审核话单的日期
			
			//bool flag  = false;
			memset(before_date,0,sizeof(before_date));
			strcpy(before_date,mdrDeal.m_SerialString);	
			theJSLog<<"审核话单日期"<<before_date<<endi;
		}
		else			//主系统
		{
			//时间判断 先判断前天的有没有做过每天4点钟以后执行一次,执行后存放到内存,然后写文件
			char tmpDate[8+1];
			memset(tmpDate,0,sizeof(tmpDate));
			memset(before_date,0,sizeof(before_date));
			getCurTime(currTime);
		    strncpy(tmpDate,currTime,8);
			addDays(-1,tmpDate,before_date);
			if(checkAuditBefore(before_date))		//昨天已经核对则不用核对了
			{
				sleep(30);
				continue ;   
			}
			
			char tmpTime[2+1];
			memset(tmpTime,0,sizeof(tmpTime));
			strncpy(tmpTime,currTime+8,2);
			if(strcmp(tmpTime,"06") < 0)			//6点钟以后执行
			{
				sleep(30);
				continue ;
			}
			
			theJSLog<<"审核话单日期:"<<before_date<<endi;
		}
		
		if(!(dbConnect(conn)))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"run() 连接数据库失败 connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
			sleep(30);

			continue  ;
		}

		theJSLog<<"######## start deal d_out_reg ########"<<endi;

		int cnt = 0;
		Statement stmt;
		
		try
		{	
			stmt = conn.createStatement();
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from d_out_file_reg where file_type = 'E' and state = 'W' and reg_time < '%s9999' ",before_date);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>cnt;
				
			theJSLog<<"错误清单文件个数:"<<cnt<<endi;

			memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select fileName,source_id from d_out_file_reg where file_type = 'E' and state = 'W' and reg_time < '%s9999' ",before_date);		
			stmt.setSQLString(sql);
			stmt.execute();
			string filename ,source;
			while(stmt>>filename>>source)			//通过数据源找到文件名所属的上游数据源标志
			{	
				for(map< string,vector<string> >::const_iterator it=sourceMap.begin();it!=sourceMap.end();++it)
				{
					vector<string> array = it->second;
					for(vector<string>::iterator iter = array.begin();iter != array.end();++iter)
					{
						if((*iter) == source)		//找到数据源所属的结算种类
						{
							map< string,AduitEnv >::iterator it2 = fileNameMap.find(it->first);						
							(it2->second).arrayFile.push_back(filename);
									
							//sprintf(mdrParam.m_SerialString,"%s%s|",mdrParam.m_SerialString,filename);
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
		}
		catch (util_1_0::db::SQLException e)
		{
			vsql.clear();
			clearMap();
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"run() 数据库操作异常%s (%s)",e.what(),sql);
			theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
		}
	
		if(mdrDeal.mdrParam.drStatus != 1)
		{
			sprintf(mdrDeal.m_SerialString,"%s%s",mdrDeal.m_SerialString,before_date);
			ret = mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
			if(ret)
			{
				theJSLog<<"主系统同步失败...."<<endw;
				//清空fileNameMap
				clearMap();
				conn.close();
				sleep(30);
				continue ;
			}
		}

		/*************************************************************/
		char outFile[512],tmp[512];
		vector<string> fileList;

		memset(mdrDeal.m_AuditMsg,0,sizeof(mdrDeal.m_AuditMsg));
		struct stat fileInfo;
		
		getCurTime(currTime);
		//将对应的数据源生成文件 上游数据源标志,文件记录数目,文件大小
		for(map< string,AduitEnv >::const_iterator it3=fileNameMap.begin() ;it3!=fileNameMap.end();++it3)
		{
			vector<string> arrayFile = (it3->second).arrayFile;	
			
			sprintf(m_szFileName,"ACCT_%s_D_%s_AUD",before_date,it3->first);	 //写核对文件
			sprintf(mdrDeal.m_AuditMsg,"%s%s,%d,%c,",mdrDeal.m_AuditMsg,m_szFileName,arrayFile.size(),(it3->second).null_out_flag);

			if(arrayFile.size() == 0)
			{		
				 if((it3->second).null_out_flag == 'N')	
				 {
					 sprintf(mdrDeal.m_AuditMsg,"%s|",mdrDeal.m_AuditMsg);
					 continue ;	
				 }
			}
				
			//sprintf(m_szFileName,"ACCT_%s_D_%s_AUD",tmpTime,it3->first);	 //写核对文件

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
				clearMap();
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"run() 文件%s打开出错",m_szFileName);
				theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败
				continue ;
			}
		
			//写文件头
			out<<10<<","<<currTime<<","<<before_date<<","<<arrayFile.size()<<endl;
			for(int k = 0;k<arrayFile.size();k++)
			{		
				out<<arrayFile[k]<<"\n";					
			}		
		
			out<<"90"<<endl;
			out.close();	

			stat(outFile,&fileInfo); //获取文件大小
			sprintf(mdrDeal.m_AuditMsg,"%s%ld|",mdrDeal.m_AuditMsg,fileInfo.st_size);	
		}
	
		clearMap();

		char state = 'Y';
		ret = mdrDeal.IsAuditSuccess(mdrDeal.m_AuditMsg);
		if(ret)											//仲裁失败,回滚数据库,删除临时文件
		{		
			//getCurTime(currTime);
			//memset(sql,0,sizeof(sql));
			//sprintf(sql,"update d_out_file_reg set state = 'E' , deal_time = '%s' where state = 'H'",currTime);
			//vsql.push_back(sql);	
			//ret = updateDB();

			theJSLog<<"删除临时审核文件..."<<endi;
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
			state = 'E';
			//fileList.clear();
			//conn.close();
			//return ;
		}
		else
		{
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
		}
		
		fileList.clear();

		theJSLog<<"更改数据库表记录状态..."<<endi;
		getCurTime(currTime);
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update d_out_file_reg set state = '%c' , deal_time = '%s' where state = 'H'",state,currTime);
		vsql.push_back(sql);

		ret = updateDB();							//暂时不考虑失败情况	
		conn.close();

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
			out.close();
		}	
	}
/*
	catch (util_1_0::db::SQLException e)
	{
		vsql.clear();
		clearMap();
		fileList.clear();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() 数据库操作异常%s (%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
	}
*/
	catch (jsexcp::CException &e) 
	{	
		vsql.clear();
		clearMap();
		fileList.clear();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		
	}
		
	theJSLog<<"######## end deal ########\n"<<endi;
	
	sleep(30);
 } //while(1)

	//return ;
}

//先查询内存,再查询文件
bool FileOutAduit::checkAuditBefore(char* date)
{
	if(strcmp(date,last_date) == 0)  return true;
	
	char szBuff[1024];
	ifstream in(date_path,ios::in) ;			//供读，文件不存在则创建
	if(!in)
	{
			//memset(erro_msg,0,sizeof(erro_msg));
			//sprintf(erro_msg,"checkAuditBefore() 文件[%s]打开出错",date_path);
			//theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败
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

//批量提交sql,保证一个事物完整性
int FileOutAduit::updateDB()
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

//清空Map
void FileOutAduit::clearMap()
{
	//for(map< string,AduitEnv >::const_iterator it3=fileNameMap.begin() ;it3!=fileNameMap.end();++it3)
	//{
	//	vector<string> arrayFile = (it3->second).arrayFile;		
	//	arrayFile.clear();
	//}
	//2013-12-02修改
	for(map< string,AduitEnv >::iterator it3=fileNameMap.begin();it3!=fileNameMap.end();++it3)
	{
		((it3->second).arrayFile).clear();		
	}
}

//2013-10-11 新增退出函数
void FileOutAduit::prcExit()
{
	mdrDeal.dr_ReleaseDR();

	PS_Process::prcExit();
}

/*
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
int FileOutAduit::drVarGetSet(char* serialString)
{
		int ret  = 0;
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
 int FileOutAduit::IsAuditSuccess(const char* dealresult)
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

bool FileOutAduit::CheckTriggerFile()
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
	cout<<"*            jsfileAduit                      "<<endl;
	cout<<"*            sys.GJZW.Version 1.0	         "<<endl;
	cout<<"*     created time :      2013-07-20 by  hed	 "<<endl;
	cout<<"*     last update time :  2013-12-16 by  hed	 "<<endl;
	cout<<"********************************************* "<<endl;


	FileOutAduit fm ;

	if( !fm.init( argc, argv ) )
	{
		 return -1;
	}
        
	//while(1)
	//{
	//	theJSLog.reSetLog();
		fm.run();
	//	sleep(30);
	//}

   return 0;
}


