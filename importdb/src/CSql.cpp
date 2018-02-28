/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-08-16
File:			 CSql.cpp
Description:	 实时SQL入库模块
History:
<table>
revision	author            date                description
--------    ------            ----                -----------
</table>
**************************************************************************/
#include "CSql.h"
CLog theJSLog;
SGW_RTInfo	rtinfo;
struct stat fileInfo;


CSql::CSql()
{
	audit_file_num = 20;
	mini_flag = false;

	memset(filenames,0,sizeof(filenames));
	memset(m_Filename,0,sizeof(m_Filename));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(currTime,0,sizeof(currTime));
	
	mVfile.clear();
	vDealFlag.clear();
}

CSql::~CSql()
{
	mdrDeal.dr_ReleaseDR();
}

	//模块初始化动作
bool CSql::init(int argc,char** argv)
{

    if(!PS_Process::init(argc,argv))
    {
		return false;
    }
	
	mConfParam.iflowID = getFlowID();
	mConfParam.iModuleId = getModuleID();

	//theJSLog.setLog(szLogPath, szLogLevel,"CSql", "GJJS", 001);	
	//theJSLog<<"日志路径："<<szLogPath<<" 日志级别："<<szLogLevel<<"  每次仲裁文件个数:"<<audit_file_num<<endi;

	string sql;
  	try
	{
		if (dbConnect(conn))
	 	{
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

			//获取文件读取目录
			sql = "select VARVALUE from C_GLOBAL_ENV where VARNAME = 'SQL_PATH'";
			stmt.setSQLString(sql);
			stmt.execute();//执行sql语句
			if(!(stmt >> mConfParam.szInPath))//获取结果
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"C_GLOBAL_ENV表没有配置:SQL_PATH");
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return false;
			}	
			theJSLog <<"szInPath="<<mConfParam.szInPath<<endi;
			completeDir(mConfParam.szInPath);

			//获取文件执行成功的备份目录
			sql = "select VARVALUE from C_GLOBAL_ENV where VARNAME = 'SQL_BAK_PATH'";
			stmt.setSQLString(sql);
			stmt.execute();//执行sql语句
			if(!(stmt >> mConfParam.szBakPath))//获取结果
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"C_GLOBAL_ENV表没有配置:SQL_BAK_PATH");
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return false;
			}
			theJSLog <<"szBakPath="<<mConfParam.szBakPath<<endi;
			completeDir(mConfParam.szBakPath);

			//获取文件执行失败的错误目录
			sql = "select VARVALUE from C_GLOBAL_ENV where VARNAME = 'SQL_ERR_PATH'";
			stmt.setSQLString(sql);
			stmt.execute();//执行sql语句
			if(!(stmt >> mConfParam.szErrPath))		//获取结果
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"C_GLOBAL_ENV表没有配置:SQL_ERR_PATH");
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return false;
			}
			theJSLog <<"szErrPath="<<mConfParam.szErrPath<<endi;
			completeDir(mConfParam.szErrPath);
			
			//获取文件执行仲裁失败的目录
			sql = "select VARVALUE from C_GLOBAL_ENV where VARNAME = 'SQL_FAIL_PATH'";
			stmt.setSQLString(sql);
			stmt.execute();//执行sql语句
			if(!(stmt >> mConfParam.szFailPath))		//获取结果
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"C_GLOBAL_ENV表没有配置:SQL_FAIL_PATH");
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return false;
			}
			theJSLog <<"szFailPath="<<mConfParam.szFailPath<<endi;
			completeDir(mConfParam.szFailPath);
			
			sql = "select VARVALUE from C_GLOBAL_ENV where VARNAME = 'SQL_AUDIT_NUM'";
			stmt.setSQLString(sql);
			stmt.execute();//执行sql语句
			if(!(stmt >> audit_file_num))			//获取结果
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"C_GLOBAL_ENV表没有配置:SQL_AUDIT_NUM");
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return false;
			}
			theJSLog <<"SQL_AUDIT_NUM="<<audit_file_num<<endi;

			sql = "select VARVALUE from C_GLOBAL_ENV where VARNAME = 'SQL_FAIL_MINI_NUM'";
			stmt.setSQLString(sql);
			stmt.execute();//执行sql语句
			if(!(stmt >> audit_fail_mini_num))		//获取结果
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"C_GLOBAL_ENV表没有配置:SQL_FAIL_MINI_NUM");
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return false;
			}
			theJSLog <<"SQL_FAIL_MINI_NUM="<<audit_fail_mini_num<<endi;

			stmt.close();
			
		 }else
		 {
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"init() 连接数据库失败 connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
			return  false;
		 }
	 	 conn.close();

  	}catch( SQLException e )
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"查找文件路径时失败:%s,sql语句为:%s",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
		conn.close();
		return false;
  	}
   
   DIR *dirptr = NULL; 
   if((dirptr=opendir(mConfParam.szInPath)) == NULL)
   {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"输入文件路径[%s]打开失败",mConfParam.szInPath);
		theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错

		return false ;

	}else closedir(dirptr);

	if((dirptr=opendir(mConfParam.szBakPath)) == NULL)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"备份文件路径[%s]打开失败",mConfParam.szBakPath);
		theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错

		return false ;
			
	}else closedir(dirptr);

	if((dirptr=opendir(mConfParam.szErrPath)) == NULL)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"错误文件路径[%s]打开失败",mConfParam.szErrPath);
		theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错

		return false;

	}else closedir(dirptr);

	if((dirptr=opendir(mConfParam.szFailPath)) == NULL)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"仲裁失败文件路径[%s]打开失败",mConfParam.szFailPath);
		theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错

		return false ;
			
	}else closedir(dirptr);

   if(!(rtinfo.connect()))
	{
		theJSLog.writeLog(0,"连接运行时信息失败");	 
		return false;
	}	
	rtinfo.getDBSysMode(petri_status);
	theJSLog<<"petri status:"<<petri_status<<endi;

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

/*
	char sParamName[256];
	CString sKeyVal;
	sprintf(sParamName, "sql.file.audit.num");
	if(param_cfg.bGetMem(sParamName, sKeyVal))
	{
		audit_file_num = sKeyVal.toInteger();
		if((audit_file_num <= 0) || (audit_file_num) > 60)
		{
			audit_file_num = 20;
		}

		theJSLog<<sParamName<<"="<<audit_file_num<<endi;
	}
	else
	{	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"请在核心参数中配置每次扫描文件的个数,参数名:%s",sParamName);
		theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
		return false ;
	}
*/

	bool bb = initializeLog(argc,argv,false);  //是否调试模式
	if(!bb)
	{
		theJSLog.writeLog(0,"初始化内存日志接口失败"); 
		return false;
	}

   theJSLog<<"初始化完毕...\n"<<endi;

   return true ;
}

//返回失败的文件个数
int CSql::getFileExist()
{
	int  count = 0;

	for(int i = 0;i<mVfile.size();i++)
	{
		memset(filenames,0,sizeof(filenames));
		strcpy(filenames,mConfParam.szInPath);
		strcat(filenames,mVfile[i].c_str());

		if(access(filenames,F_OK|R_OK))
		{
			theJSLog<<"文件["<<mVfile[i].c_str()<<"]不存在!"<<endw;
			count++;
			continue;
		}
	}

	return count;
}

void CSql::run()//扫描目录，获取文件名
{
	int ret = 0,event_sn, event_type;
	long param1, param2, src_id;
	unsigned long filesize = 0;

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

	//判断数据库状态
	short db_status = 0;
	rtinfo.getDBSysMode(db_status);
	if(db_status != petri_status)
	{
		theJSLog<<"数据库状态切换... "<<petri_status<<"->"<<db_status<<endw;
		int cmd_sn = 0;
		if( !putEvt(cmd_sn, EVT_RPT_DBSTATUS, 0, db_status, DSPCH_PRC_ID) )
		{
			theJSLog<<"报告数据库更换状态失败！\n"<<endw;
			return ;
		}
		petri_status = db_status;
	}
	if(petri_status == DB_STATUS_OFFLINE)	return ;
	
	try
	{
		if(mdrDeal.mdrParam.drStatus==1)  //备系统
		{
				//检查trigger触发文件是否存在
				if(!mdrDeal.CheckTriggerFile(m_triggerFile))
				{
					sleep(1);
					return ;
				}

				//获取同步变量
				memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));
				ret=mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
				if(ret)
				{
					theJSLog<<"同步失败..."<<endw;
					return ;
				}
				
				theJSLog<<"######## start deal file ###################"<<endi;
		
				splitString(mdrDeal.m_SerialString,";",mVfile,false,false);

				//int dr_GetAuditMode()1表示同步，2表示跟随, 其它为失败，-1是配置错误，-2配置文件读取出现问题
				//int iStatus = dr_GetAuditMode(module_name);
				int iStatus = mdrDeal.mdrParam.aduitMode;
				
				if(iStatus == 1)		//同步模式,	主系统等待指定时间 
				{
					bool flag=false;
					int times=1,count = 0;
					while(times<31)
					{
						count = getFileExist();
						if(count)
						//if(access(filenames,F_OK|R_OK))
						{
							theJSLog<<"查找了"<<times<<"次,查找失败文件个数:"<<count<<endw;
							times++;
							sleep(10);
						}
						else
						{
							flag=true;
							break;
						}
					}
					if(!flag)
					{
						//dr_AbortIDX();
						mdrDeal.dr_abort();
						mVfile.clear();

						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"主系统传过来的文件不齐");
						theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);			
						return ;
					}
				}
				else if(iStatus==2) //跟随模式，备系统
				{
					int count = 0;
					while(1)
					{
						//设置中断
						if(gbExitSig)
						{
							//dr_AbortIDX();
							mdrDeal.dr_abort();
							
							theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "应用程序收到退出信号");
							prcExit();
							return ;
						}
						
						count = getFileExist();
						if(count)
						//if(access(filenames,F_OK|R_OK))
						{
							sleep(10);
						}
						else
						{
							break;
						}
					}
				}
				
				if(!(dbConnect(conn)))
				{
					//dr_AbortIDX();
					mdrDeal.dr_abort();
					mVfile.clear();

					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"run() 连接数据库失败 connect error");
					theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
					sleep(30);
					return  ;
				}

				//处理文件
				ret = doAllSQL();
				conn.close();

				theJSLog<<"######## end deal file ########\n"<<endi;
		}
		else      //主系统,非容灾系统    
		{
			if(mMiniVfile.size())						//表示上次仲裁失败,且文件数太多需要缩小范围
			{
				vector<string>::iterator  iter1 = mMiniVfile.begin();
				vector<string>::iterator  iter2 = mMiniVfile.begin();

				int szSize = audit_fail_mini_num;
				if(mMiniVfile.size() <= audit_fail_mini_num)
				{
					mini_flag = false;					//最后一次
					szSize = mMiniVfile.size();
				}

				for(int i=0;i<szSize;i++)
				{
					mVfile.push_back(mMiniVfile[i]);
					++iter2;
				}
				
				mMiniVfile.erase(iter1,iter2);

			}else
			{

			if(scan.openDir(mConfParam.szInPath))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"打开文件目录[%s]失败",mConfParam.szInPath);
				theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
				return ;	
			}	
			
			filesize = 0;
			int rett = 0,counter = 0;
			while(1)		
			{
				memset(filenames,0,sizeof(filenames));	    
				rett = scan.getFile("*.sql",filenames); 
				if(rett == 100)
				{		
						break ;
				}
				if(rett == -1)
				{	
						break ;							//表示获取文件信息失败
				}

				counter++;								//2013-11-05每扫描50个文件返回
				if(counter > audit_file_num)
				{
					break;
				}
				
				/*把文件名的路径去掉*/
				char* p = strrchr(filenames,'/');
				strcpy(m_Filename,p+1);
						
				mVfile.push_back(m_Filename);
				
				stat(filenames,&fileInfo);				//获取文件大小,超过指定大小1M,大概2K条数据则不再扫描其它文件			
				filesize += (unsigned long)fileInfo.st_size;
				if(filesize > 1048576)
				{
					theJSLog<<"文件大小总和已经指定大小,停止扫描文件;filesize="<<filesize<<endi;
					break;
				}
			}

			scan.closeDir();
			
			}	//else

			if(mVfile.size() != 0)
			{
				if(!(dbConnect(conn)))
				{
					mVfile.clear();
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"run() 连接数据库失败 connect error");
					theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
					sleep(30);

					return  ;
				}

				memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));
				for(int i = 0;i<mVfile.size();i++)
				{
					sprintf(mdrDeal.m_SerialString,"%s%s;",mdrDeal.m_SerialString,mVfile[i]);
				}

				theJSLog<<"######## start deal file #####################"<<endi;
				
				ret = mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
				if(ret)
				{
					theJSLog<<"同步失败...."<<endw;
					mVfile.clear();
					conn.close();
					sleep(30);
					return ;
				}
			    
				//处理文件
				ret = doAllSQL();
				
				conn.close();

				theJSLog<<"######## end deal file ########\n"<<endi;
			}

		}
	}catch (jsexcp::CException &e) 
	{	
		mVfile.clear();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		
	}

}

bool CSql::doAllSQL()			
{	
	Statement stmt;
	try
	{
		stmt = conn.createStatement();
	}
	catch ( SQLException e)
	{
		//dr_AbortIDX();
		mdrDeal.dr_abort();
		mVfile.clear();

		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"doAllSQL() err %s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
		
		return false;
	}
	
	int count = 0,ret = 0;	
	char szBuff[JS_MAX_RECORD_LEN],flag;
	memset(mdrDeal.m_AuditMsg,0,sizeof(mdrDeal.m_AuditMsg));
	
	for(int i = 0;i<mVfile.size();i++)
	{
		memset(filenames,0,sizeof(filenames));
		strcpy(filenames,mConfParam.szInPath);
		strcat(filenames,mVfile[i].c_str());
		memset(m_Filename,0,sizeof(m_Filename));
		strcpy(m_Filename,mVfile[i].c_str());

		count = 0;
		flag = 'Y';

		try{			
		 		
				ifstream in(filenames,ios::in) ;
				if(!in)
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"文件[%s]打开出错",filenames);
					theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败

					sprintf(mdrDeal.m_AuditMsg,"%s%s|",mdrDeal.m_AuditMsg,strerror(errno));
					vDealFlag.push_back('E');

					continue;
				}	
			
				memset(szBuff,0,sizeof(szBuff));
				while(in.getline(szBuff,sizeof(szBuff)))   
				{
					count++;
					stmt.setSQLString(szBuff);	
					stmt.execute();  //执行sql语句

					//if((count%SQL_COMMIT_COUNT) == 0)			//每隔500条提交
					//{
					//	stmt.commit();
					//}

					memset(szBuff,0,sizeof(szBuff));
				}
				in.close();
				theJSLog<<"处理完sql文件:"<<m_Filename<<endi;
				
				vDealFlag.push_back(flag);
					
				sprintf(mdrDeal.m_AuditMsg,"%s%c;%d|",mdrDeal.m_AuditMsg,flag,count);
		  }
		  catch( SQLException e ) 
		  {
				stmt.rollback();
				flag = 'E';
				vDealFlag.push_back(flag);

				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"文件[%s]的sql语句失败:%s,sql:(%s)",filenames,e.what(),szBuff);
				theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

				sprintf(mdrDeal.m_AuditMsg,"%s%c;%d|",mdrDeal.m_AuditMsg,flag,count);
		  }

	}

		ret = mdrDeal.IsAuditSuccess(mdrDeal.m_AuditMsg);
		if(ret)							//仲裁失败
		{
			stmt.rollback();
			stmt.close();
			
			if(mVfile.size() > audit_fail_mini_num)
			{
				if(!mini_flag)								//第一次仲裁失败需要
				{
					mini_flag = true;
					theJSLog<<"本次sql文件个数:"<<mVfile.size()<<">"<<audit_fail_mini_num<<",则需缩小范围查询"<<endi;		
					for(int i = 0;i<mVfile.size();i++)
					{	
						mMiniVfile.push_back(mVfile[i]);
					}
				}

				mVfile.clear();
				vDealFlag.clear();

				return true;
			}
			
			if(ret != 3)				//2013-11-07 仲裁超时不移动文件
			{			
				moveFiles(1);			
			}
		}			
		else
		{
			stmt.close();	 
			saveLog();					//每处理一个文件都记录到D_SQL_FILEREG表中

			moveFiles(0);				
		}
		
		mVfile.clear();
		vDealFlag.clear();

		return true;
}

void CSql::saveLog()			//每处理一个文件都保存到D_SQL_FILEREG表中
{
	string sql;
	//char flag = 'Y';
	Statement stmt = conn.createStatement();
	sql = "insert into D_SQL_FILEREG(FILE_NAME,DEAL_DATE,DEAL_FLAG) values(:v1,sysdate,:v2)";
	stmt.setSQLString(sql);	

	for(int i = 0;i<mVfile.size();i++)
	{
		try
		{
			stmt<<mVfile[i]<<vDealFlag[i];
			stmt.execute();						//执行sql语句
			stmt.commit();
		
		}catch( SQLException e ) 
		{
			stmt.rollback();

			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"处理文件[%s]时保存到D_SQL_FILEREG表时sql语句失败:%s,sql:(%s)",mVfile[i],e.what(),sql);
			theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
		} 
		
	}
	
	stmt.close();
}

//flag:0为表示仲裁成功,1仲裁失败目录

bool CSql::moveFiles(int flag)						//将已经处理后的文件移动到指定备份目录	
{
	char bak_dir[512];
	char tmp[1024];

	if(flag == 0)									//仲裁成功，保存到指定目录				
	{	
		getCurTime(currTime);
		memset(bak_dir,0,sizeof(bak_dir));
		strcpy(bak_dir,mConfParam.szBakPath);
		strncat(bak_dir,currTime,6);
		completeDir(bak_dir);
		strncat(bak_dir,currTime+6,2);
		completeDir(bak_dir);
		
		if(chkAllDir(bak_dir) == 0)
		{	
			for(int i = 0;i<mVfile.size();i++)
			{	
				memset(filenames,0,sizeof(filenames));
				strcpy(filenames,mConfParam.szInPath);
				strcat(filenames,mVfile[i].c_str());
				
				if(vDealFlag[i] == 'Y')
				{
					memset(tmp,0,sizeof(tmp));
					strcpy(tmp,bak_dir);
					strcat(tmp,mVfile[i].c_str());
					theJSLog<<"备份文件["<<mVfile[i]<<"]到目录:"<<bak_dir<<endi;

					if(rename(filenames,tmp))		
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"移动文件[%s]到备份目录失败: %s",filenames,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
						//return false;
					}	
				}
				else
				{
					theJSLog<<"将文件["<<mVfile[i]<<"]到错误目录 "<<mConfParam.szErrPath<<endi;
					memset(tmp,0,sizeof(tmp));
					strcpy(tmp,mConfParam.szErrPath);
					strcat(tmp,mVfile[i].c_str());

					if(rename(filenames,tmp))		
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"移动文件[%s]到错误目录失败: %s",filenames,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
						//return false;
					}	

				}
			}

		}
		else
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"备份路径[%s]不存在，且无法创建",bak_dir);
			theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg);		//打开目录出错
			prcExit();
		}		
	}
	else										//仲裁失败，或者执行文件中的sql语句失败，保存到失败目录				
	{ 
		theJSLog<<"将文件移到仲裁失败目录 "<<mConfParam.szFailPath<<endi;

		for(int i = 0;i<mVfile.size();i++)
		{	
			memset(filenames,0,sizeof(filenames));
			strcpy(filenames,mConfParam.szInPath);
			strcat(filenames,mVfile[i].c_str());

			memset(tmp,0,sizeof(tmp));
			strcpy(tmp,mConfParam.szFailPath);
			strcat(tmp,mVfile[i].c_str());
			if(rename(filenames,tmp))		
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"移动文件[%s]到失败目录失败: %s",filenames,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
				//return false;
			}
			
		}
	}

	return true;
}

//2013-11-02 新增退出函数
void CSql::prcExit()
{
	//int ret = 0;

	mdrDeal.dr_ReleaseDR();
	
	PS_Process::prcExit();
}

/*
//容灾初始化
bool CSql::drInit()
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
int CSql::drVarGetSet(char* serialString)
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
		
		//主系统传递文件所在路径和文件名 只有容灾平台可以感知,备系统无法识别
		if(mdrParam.drStatus != 1)
		{
			snprintf(tmpVar, sizeof(tmpVar), "%s",input_path);
			ret = dr_SyncIdxVar("@@CHECKPATH", tmpVar,SYNC_SINGLE);  
			if(ret != 0)
			{
				theJSLog<<"传输文件所在路径失败,文件路径["<<input_path<<"]"<<endi;
				dr_AbortIDX();
				return -1;
			}
			
			snprintf(tmpVar, sizeof(tmpVar), "%s", m_Filename);
			ret = dr_SyncIdxVar("@@CHECKFILE", tmpVar,SYNC_SINGLE);  
			if(ret != 0)
			{
				theJSLog<<"传输文件失败,文件名["<<m_Filename<<"]"<<endi;
				dr_AbortIDX();
				return -1;
			}

			cout<<"传输文件路径:"<<input_path<<" 文件名:"<<m_Filename<<endl;
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
 int CSql::IsAuditSuccess(const char* dealresult)
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
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"容灾仲裁失败,结果:%d,本端:%s",ret,dealresult);
			theJSLog.writeLog(LOG_CODE_DR_AUDIT_ERR,erro_msg);

			dr_AbortIDX();
		
		}
		else if (3 == ret)
		{
			theJSLog<<"容灾仲裁超时..."<<endi;
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

bool CSql::CheckTriggerFile()
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
*/

int main(int argc,char** argv)
{
	cout<<"********************************************** "<<endl;
	cout<<"*    China Telecom. Telephone Network          "<<endl;
	cout<<"*    InterNational Account Settle System       "<<endl;
	cout<<"*                                              "<<endl;
	cout<<"*           jsextSql							  "<<endl;
	cout<<"*           sys.GJZW.Version 1.0	              "<<endl;
	cout<<"*     last update time : 2013-12-16 by  hed	  "<<endl;
	cout<<"********************************************** "<<endl;


	CSql fm ;
   	if( !fm.init( argc, argv ) )
	{
		 return -1;
	}
	while(1)
	{
		theJSLog.reSetLog();
		fm.run();
		sleep(5);
	}

   return 0;
}

