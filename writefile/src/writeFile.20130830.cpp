/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2012-11-4
File:			 Write_File.cpp
Description:	 读写话单块处理
History:
<table>
revision	author            date                description
--------    ------            ----                -----------
v1.0		hd          2012-11-4

2013-07-15 当一个文件写多个话单块时某个话单块的记录信息出错，则回退该文件已经处理的话单块，将其删除，并记录错误信息给sql文件，
		  当处理下个文件时，写错误登记表，并将源文件copy过来到目标文件
</table>
**************************************************************************/
//#include <io.h> // _findfirst() _findnext()
//#include <string.h> //strcat()
//#include <stdio.h>//gets() puts()
//#include<direct.h>

#include <dirent.h> //_chdir() _getcwd() 读取文件目录等，浏览文件夹信息
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>  //stat()函数，查询文件信息
#include<unistd.h>     //读取当前程序运行目录
#include<iostream>
#include<fstream>
#include "writeFile.h"

#include "CF_Common.h"
#include "CF_CLogger.h"
CLog theJSLog;
SGW_RTInfo	rtinfo;

Write_File::Write_File()
{  
	file_status = 0;
	record_num = 0;
	petri_status = 0;
	m_enable = false;

	memset(mServCatId,0,sizeof(mServCatId));
    memset(m_szSrcGrpID,0,sizeof(m_szSrcGrpID));
	memset(m_szService,0,sizeof(m_szService));
	memset(m_szSourceID,0,sizeof(m_szSourceID));
	memset(m_szOutTypeId,0,sizeof(m_szOutTypeId));

	memset(other_path,0,sizeof(other_path));
	memset(out_path,0,sizeof(out_path));
	memset(erro_path,0,sizeof(erro_path));
	memset(bak_path,0,sizeof(bak_path));

	memset(m_szFileName,0,sizeof(m_szFileName));
	memset(outFileName,0,sizeof(outFileName));

	memset(sql,0,sizeof(sql));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(erro_sql,0,sizeof(erro_sql));
	memset(currTime,0,sizeof(currTime));
}


Write_File::~Write_File()
{
	//cout<<"xxxx"<<endl;
	if(m_enable) 
	{
		//cout<<"释放容灾平台资源"<<endl;

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
bool Write_File::init(int argc,char** argv)
{
   
   if(!PS_BillProcess::init(argc,argv))
   {
      return false;
   }
	
   //获取petri网状态,当系统为只读态时,数据库更新操作语句写文件
	
	if(!(rtinfo.connect()))
	{
		return false;
	}
	
	rtinfo.getSysMode(petri_status);
	cout<<"petri status:"<<petri_status<<endl;

	//*********2013-07-15 读取数据库的配置信息，如数据源组，错误目录 2013-03-11 add by hed*********************************************/
	
	if(!(dbConnect(conn)))
	{
		cout<<"连接数据库 connect error."<<endl;	//写日志
		return false ;
	}

	int flow_id = getFlowID();
	int module_id = getModuleID();
	//char sourceGroup[8],service[8];  
	//cout<<"流水线ID:"<<flow_id<<"   模块ID:"<<module_id<<endl;
	try{

		string sql = "select source_group from TP_BILLING_LINE  where billing_line_id = :1";
		Statement stmt = conn.createStatement();
		stmt.setSQLString(sql);
		stmt<<flow_id;
		stmt.execute();
		if(!(stmt>>m_szSrcGrpID))
		{
			cout<<"请在tp_billing_line表中配置数据源组"<<endl;			//写日志
			return false ;
		}

		sql = "select ext_param from TP_PROCESS where billing_line_id = :1 and module_id = :2";
		stmt.setSQLString(sql);
		stmt<<flow_id<<module_id;
		stmt.execute();
		if(!(stmt>>m_szService))
		{
			cout<<"请在tp_process表中配置service"<<endl;
			return false ;
		}

		sql = "select c.output_path from C_SOURCE_GROUP_DEFINE a,C_SERVICE_FLOW b,C_SERVICE_INTERFACE c where a.source_group=:1" 
                    "and a.workflow_id = b.workflow_id and b.service=:2 and b.input_id = c.interface_id ";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID<<m_szService;
		stmt.execute();
		if(!(stmt>>out_path))
		{
			cout<<"写文件模块输出文件相对路径out_path没有配置"<<endl;		
			return false ;
		}

		completeDir(out_path);
				
		sql = "select var_value from c_process_env where varname = 'WR_FILE_ERR_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID<<m_szService;
		stmt.execute();
		if(!(stmt>>erro_path))
		{
				cout<<"请在表c_process_env中配置写文件模块的错误路径 WR_FILE_ERR_DIR"<<endl;		//写日志
				return false;
		}		
		completeDir(erro_path);
	
		
		sql = "select var_value from c_process_env where varname = 'WR_FILE_BAK_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID<<m_szService;
		stmt.execute();
		if(!(stmt>>bak_path))
		{
			cout<<"请在表c_process_env中配置写文件模块的备份目录(源文件目录) WR_FILE_BAK_DIR"<<endl;		//写日志
			return false;
		}
		completeDir(bak_path);

		sql = "select var_value from c_process_env where varname = 'WR_FILE_TMP_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID<<m_szService;
		stmt.execute();
		if(!(stmt>>other_path))
		{
			cout<<"请在表c_process_env中配置写文件模块的临时目录 WR_FILE_TMP_DIR"<<endl;		//写日志
			return false;
		}
		completeDir(other_path);

		stmt.close();
	   
	   }catch(SQLException e)
		{
			cout<<"初始化时数据库查询异常:"<<e.what()<<endl;
			return false ;
		}
	
	
	 DIR *dirptr = NULL; 
	
	sprintf(erro_sql,"%sjswritefileErro.%d.sql",sql_path,getFlowID());	//错误sql文件全路径

	//初始化内存日志接口
	bool bb = initializeLog(argc,argv,false);  //是否调试模式
	if(!bb)
	{
			//theJSLog<<"初始化内存日志接口失败"<<endi;
			return false;
	}
	
	theJSLog.setLog(szLogPath,szLogLevel,m_szService , m_szSrcGrpID, 001);	//文件日志接口，调用了内存日志接口
	
	theJSLog<<"数据源组："<<m_szSrcGrpID<<"   service:"<<m_szService<<"  相对输出路径:"<<out_path<<"  错误路径:"<<erro_path
			<<"	源文件备份路径:"<<bak_path<<"	日志路径:"<<szLogPath<<" 日志级别:"<<szLogLevel<<" sql存放路径:"<<sql_path<<endi;

	if(LoadSourceCfg() == -1) 
	{		
		return false ;  
	}
	
	conn.close();

	char out_dir[1024],erro_dir[1024],bak_dir[1024],other_dir[1024];
	int rett = 0;

	for(map<string,SOURCECFG>::const_iterator iter = m_SourceCfg.begin();iter !=m_SourceCfg.end(); ++iter)
	{		   				
			memset(out_dir,0,sizeof(out_dir));
			strcpy(out_dir,iter->second.szSourcePath);
			strcat(out_dir,out_path);
			if((dirptr=opendir(out_dir)) == NULL)
			{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"数据源[%s]的输出文件路径[%s]不存在",iter->first,out_dir);
					theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
					return false ;
			}else closedir(dirptr);
			
			memset(erro_dir,0,sizeof(erro_dir));
			strcpy(erro_dir,iter->second.szSourcePath);
			strcat(erro_dir,erro_path);
			if((dirptr=opendir(erro_dir)) == NULL)
			{
					theJSLog<<"数据源【"<<iter->first<<"】的错误文件路径: "<<erro_dir<<"不存在，自行创建"<<endi;
					rett = mkdir(erro_dir,0755);
					if(rett == -1)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"数据源[%s]的错误文件文件路径[%s]不存在，自行创建失败",iter->first,erro_dir);
						theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
						return false;
					}
			}else closedir(dirptr);
					
			memset(bak_dir,0,sizeof(bak_dir));
			strcpy(bak_dir,iter->second.szSourcePath);
			strcat(bak_dir,bak_path);
			if((dirptr=opendir(bak_dir)) == NULL)
			{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"数据源[%s]的备份文件路径[%s]不存在",iter->first,bak_dir);
					theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
					return false ;

			}else  closedir(dirptr);

			memset(other_dir,0,sizeof(other_dir));
			strcpy(other_dir,iter->second.szSourcePath);
			strcat(other_dir,other_path);
			if((dirptr=opendir(other_dir)) == NULL)
			{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"数据源[%s]的临时文件路径[%s]不存在",iter->first,other_dir);
					theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
					return false ;

			}else  closedir(dirptr);
	  
	}

	
   if(!drInit())  return false;

   theJSLog<<"初始化完毕！"<<endi;

   return true ;
}


//加载数据源配置信息
int Write_File::LoadSourceCfg()
{
	char szSqlStr[400];
	int iSourceCount=0;
	string sql ;
	try
	{
		Statement stmt = conn.createStatement();
		sql = "select FILETYPE_ID from C_SOURCE_GROUP_DEFINE where SOURCE_GROUP=:1";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID;
		if(stmt.execute())
		{
			stmt>>m_szOutTypeId;
		}
		
		outrcd.Init(m_szOutTypeId);

   		//expTrace(szDebugFlag, __FILE__, __LINE__,"%s;%s",m_szSrcGrpID,m_szOutTypeId);
		sql = "select count(*) from C_SOURCE_GROUP_CONFIG where SOURCE_GROUP=:1";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID;
		if(stmt.execute())
		{
			stmt>>iSourceCount;
		}

		//expTrace(szDebugFlag, __FILE__, __LINE__, "iSourceCount=%d;", iSourceCount);
		
		sql = "select a.source_id,b.file_fmt,b.source_path,b.TOLLCODE,b.SERV_CAT_ID from C_SOURCE_GROUP_CONFIG a,I_SOURCE_DEFINE b where SOURCE_GROUP=:1 and a.source_id=b.source_id";
		stmt.setSQLString(sql);
		stmt<<m_szSrcGrpID;
		if(stmt.execute())
		{
			for (int i=0; i<iSourceCount; i++)
			{
				SOURCECFG SourceCfg;
				string strSourceId;

				stmt>>SourceCfg.szSourceId>>SourceCfg.szInFileFmt>>SourceCfg.szSourcePath>>SourceCfg.szTollCode>>SourceCfg.serverCatID;      
				strSourceId=SourceCfg.szSourceId;
				
				completeDir(SourceCfg.szSourcePath);

				if(getSourceFilter(SourceCfg.szSourceId,SourceCfg.filterRule,SourceCfg.file_begin,SourceCfg.file_length))
				{
							return -1;
				}
				
				m_SourceCfg[strSourceId]=SourceCfg;
		     }
		}
		
		//2013-07-29
		string source_id;
		int config_id = 0;
		sql = "select source_id, var_value from c_source_env a  where a.varname = 'INS_TABLE_CONFIGID' and a.service = 'WRTF'";
		stmt.setSQLString(sql);
		stmt.execute();
		while(stmt>>source_id>>config_id)
		{
			mapConfig.insert(map<string,int>::value_type(source_id,config_id));
		}
		stmt.close();

		//判断是否和配置config_id个数相同，以免数据源找不到对应的表
		if(iSourceCount > mapConfig.size())
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"source_env中配置的INS_TABLE_CONFIGID的数据源没有配置完全：%s");
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
			return -1;
		}	
		//cout<<"iSourceCount = "<<iSourceCount<<" mapConfig.size()="<<mapConfig.size()<<endl;

		for(map<string,int>::const_iterator iter = mapConfig.begin();iter != mapConfig.end();++iter)
		{
			CF_CError_Table tab ;
			tab.Init(iter->second);
			mapTabConf.insert(map< string,CF_CError_Table>::value_type(iter->first,tab));
		}

		
	}catch (SQLException e)
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"LoadSourceCfg() 数据库出错：%s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
		//throw jsexcp::CException(0, "LoadSourceCfg加载数据库出错：", __FILE__, __LINE__);
		return -1;
	 }
	 catch (jsexcp::CException &e) 
	 {	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"LoadSourceCfg() : %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		//字段转换出错
		return -1;
	 }

	return 0;
}

/******根据数据源获取过滤规则 0没有查到规则，1查到规则了*******************考虑放在加载数据源**/
//int Write_File::getSourceFilter(char* source,char* filter)
int Write_File::getSourceFilter(char* source,char* filter,int &index,int &length)
{	
	try
	{	
		string file_time;
		char tmp[5];
		memset(tmp,0,sizeof(tmp));

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
		
		//cout<<"file_time = "<<file_time<<endl;
		strcpy(tmp,file_time.c_str());

		vector<string> fileTime;		
		splitString(tmp,",",fileTime,true);
		if(fileTime.size() != 2)
		{
			stmt.close();
			sprintf(erro_msg,"数据源[%s]文件名时间截取规则配置规则错误:%s  [如3,8]",source,file_time);
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
			return -1;
		}
		
		index = atoi(fileTime[0].c_str());
		length = atoi(fileTime[1].c_str());
		
		//cout<<"index = "<<index<<"  length = "<<length<<" file_time = "<<file_time<<endl;
		if(index < 1 || length == 0)
		{
			stmt.close();
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
		sprintf(erro_msg,"getSourceFilter 数据库查询异常: %s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);
		return -1 ;
	}
	catch(jsexcp::CException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"getSourceFilter 字段转化出错：%s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);
		return -1;
	}
	
	return 0;
}

//数据库的更新操作，便于捕获异常sql 便于重现
int Write_File::updateDB(char* sql)
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
		sprintf(erro_msg,"updateDB 数据库出错：%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
		return -1;
	}

	return 0;
}


//查看sql文件中是否有数据，若有则执行
int Write_File::scanSQLFile()
{
	char szBuff[1024];
	ifstream in(sqlFile,ios::in) ;
	if(!in)
	{
		memset(erro_msg,0,sizeof(erro_msg));
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

	in.open(sqlFile,ios::trunc);
	in.close();

	return 0;

}

//判断是否要申请话单块
int Write_File::onBeforeTask()
{
	//cout<<"申请话单块"<<endl;
	return 1;
  
}

//主进程开始分配话单前的处理
int Write_File::onTaskBegin(void *task_addr)
{
      return 1;
}

//子进程初始化
bool Write_File::onChildInit()
{
   //cout<<"模块3子进程初始化"<<endl;
   return true;
   
}

//处理成功返回话单条数(>=0)
int Write_File::onTask(void *task_addr, int offset, int ticket_num)
{
    //cout<<"\n模块3处理话单喽！！！！！！！！！！！"<<endl;

    cout<<"任务地址"<<task_addr<<endl;		 
	
	PkgBlock pkg((char*)task_addr);
	pkg.init(getTicketLength(),getBlockSize());

	//cout<<"---------------------测试写进去的记录------------------------"<<endl;	
    cout<<"话单记录数: "<<pkg.getRecordNum()<<endl;
	cout<<"模块ID: "<<pkg.getModuleId()<<endl;
	cout<<"数据源： "<<pkg.getSourceId()<<endl;
	cout<<"话单文件名："<<pkg.getFileName()<<endl;
	cout<<"话单状态："<<pkg.getStatus()<<endl;	
	cout<<"话单块在文件中所处位置："<<pkg.getBlkPos()<<endl;
	
	int ret = 0;
	char deal_flag = 'Y';   //处理状态

	char tmp[1024],path[1024],orgFileName[1024],sql[1024],full_name[1024],block_pos[2];

	memset(tmp,0,sizeof(tmp));
	memset(path,0,sizeof(path));
	memset(orgFileName,0,sizeof(orgFileName));
	memset(full_name,0,sizeof(full_name));
	memset(block_pos,0,sizeof(block_pos));

	strcpy(block_pos,pkg.getBlkPos());  //获取话单块位置

	//此时需要获取petri网状态
	if((strcmp(block_pos,"S") == 0) || (strcmp(block_pos,"D") == 0))
	{
		rtinfo.getDBSysMode(petri_status);
		theJSLog<<"话单块文件["<<orgFileName<<"] petri状态:"<<petri_status<<endi;
	}

	//if(record_num  == 0)  
	//{
		record_num += pkg.getRecordNum();
	//	return 0;
	//}
	
	strcpy(m_szSourceID,pkg.getSourceId());
	it  = m_SourceCfg.find(string(m_szSourceID));
	if(it == m_SourceCfg.end())
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"配置中不存在数据源%s的信息",m_szSourceID);
		theJSLog.writeLog(LOG_CODE_PARAM_INFO_LACK,erro_msg);
		return -1;
	}
	strcpy(mServCatId,it->second.serverCatID);

	strcpy(m_szFileName,pkg.getFileName());
	strcpy(path,it->second.szSourcePath);
	strcat(path,out_path);
	strcpy(full_name,path);

	char* p = strrchr(m_szFileName,'#');			//求出原文件名
	if(p)
	{
		strncpy(orgFileName,m_szFileName,(p-m_szFileName));
	}
	else
	{
		strcpy(orgFileName,m_szFileName);
	}
	//cout<<"原始文件名："<<orgFileName<<endl;
	
	memset(file_time,0,sizeof(file_time));			//重文件名上面截取时间
	strncpy(file_time,orgFileName+it->second.file_begin,it->second.file_length);
	
	strcat(full_name,orgFileName);
	strcpy(tmp,full_name);	 //正式文件
	strcat(tmp,".tmp");		//临时文件

	//判断话单块是否正确，若正确，则写临时文件，
	//若不正确，则先删除以前的该话单块文件，写sql异常记录信息，然后循环话单块的每条记录将错误信息登记	
	try
	{	
		if(!(dbConnect(conn)))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"Ontask() 连接数据库失败 connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
			return -1 ;
		}
		
		//2013-07-17 判断话单块的在文件中位置通过话单块的头部标记判断S表示开始 E结束 M中间 D单独

		//做异常回退的数据库登记ERRFILE_INFO：文件名、数据源、时间、错误原因（F/R）、错误码、错误行、错误列序号、
		//处理状态---由出错的程序做登记	内部格式写死下列字段：LINE_NUM,ERR_CODE,ERR_LINE,FILE_ID
	
		if(pkg.getStatus() == 2)					 //话单块正常
		{
			if((strcmp(block_pos,"S") == 0) || (strcmp(block_pos,"M") == 0))
			{
				if(strcmp(block_pos,"S") == 0)				//第一个模块
				{
					//获取petri网状态
					//rtinfo.getSysMode(petri_status);

					// 将文件注册到调度表，文件获取当前文件时间,file_id,以第一条为准
					getCurTime(currTime);							
					outrcd.Set_record(pkg.readPkgRecord(1).record);
					char file_id[10];
					memset(file_id,0,sizeof(file_id));
					memset(sql,0,sizeof(sql));
					outrcd.Get_Field(FILE_ID,file_id);
					sprintf(sql,"insert into d_sch_end(source_id,serv_cat_id,filename,deal_flag,dealstarttime,file_id,file_time) values('%s','%s','%s','W','%s',%ld,'%s')",m_szSourceID,mServCatId,orgFileName,currTime,atol(file_id),file_time);
					writeSQL(sql);

					ofstream out(tmp);									//防止出现异常后文件名相同的情况,清空文件
					if(!out)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"Ontask() 文件%s打开出错",tmp);
						theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败

						return -1;
					}
					out.close();
				}
						
				if(file_status == 0)							//上个文件话单块状态正常
				{
					ofstream out(tmp,ios::app);
					if(!out)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"Ontask() 文件%s打开出错",tmp);
						theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败

						return -1;
					}
					for(int k = 1;k<=pkg.getRecordNum();k++)
					{
						PkgFmt fmt = pkg.readPkgRecord(k);
						out<<fmt.record<<"\n";
						
					}					
					out.close();		
				}

				//每处理一个话单块，更新输入文件记录条数
				getCurTime(currTime);	
				memset(sql,0,sizeof(sql));
				sprintf(sql,"update d_sch_end set deal_flag = 'H',dealendtime = '%s',input_count = '%d' where filename = '%s' and source_id = '%s' ",currTime,record_num,orgFileName,m_szSourceID);
				writeSQL(sql);

			}
			else if(strcmp(block_pos,"E") == 0)		//多个话单块已经到最后一个块
			{	
				if(file_status == -1)
				{
					commitErrMsg();
					//theJSLog<<"删除临时输出的话单块文件"<<endi;
					if(remove(tmp))
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"临时文件[%s]删除失败: %s",tmp,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
					}

					//从格式化原始文件目录中拿出放到错误目录
					char bak_time[15],erro_dir[1024],bak_dir[1024];

					memset(bak_time,0,sizeof(bak_time));
					memset(erro_dir,0,sizeof(erro_dir));
					memset(bak_dir,0,sizeof(bak_dir));
					memset(sql,0,sizeof(sql));

					//从格式化原始文件目录中拿出放到错误目录
					sprintf(sql,"select deal_time from D_SCH_FORMAT where source_id='%s' and filename='%s' and deal_flag='Y'",m_szSourceID,orgFileName);
					Statement stmt = conn.createStatement();
					stmt.setSQLString(sql);
					if((stmt.execute()))
					{
						stmt>>bak_time;
						strcpy(erro_dir,it->second.szSourcePath);
						strcat(erro_dir,erro_path);
						strcat(erro_dir,orgFileName);
						
						strcpy(bak_dir,it->second.szSourcePath);
						strcat(bak_dir,bak_path);
						strncat(bak_dir,bak_time,6);
						strcat(bak_dir,"/");
						strncat(bak_dir,bak_time+6,2);
						strcat(bak_dir,"/");
						strcat(bak_dir,orgFileName);
						
						theJSLog<<"将原始文件从备份目录"<<bak_dir<<"上传到错误目录"<<erro_dir<<endi;
						if(link(bak_dir,erro_dir))					 //复制一份文件到错误目录
						{
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"将原始文件从备份目录上传到错误目录失败: %s",strerror(errno));
							theJSLog.writeLog(LOG_CODE_FILE_RENAME_ERR,erro_msg);
						}
					}
					else
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"没有从格式化登记表中找到文件[%s]",orgFileName);
						theJSLog.writeLog(LOG_CODE_TABLE_QUERY_ERR_CODE,erro_msg);
					}
					

					file_status = 0;
				}
				else												//正常
				{
					//先写临时文件，再改名
					ofstream out(tmp,ios::app);
					if(!out)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"Ontask() 写文件,文件%s打开出错",tmp);
						theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败

						return -1;
					}
					for(int k = 1;k<=pkg.getRecordNum();k++)
					{
						PkgFmt fmt = pkg.readPkgRecord(k);
						out<<fmt.record<<"\n";
					}					
					out.close();	
					
					//判断数据库状态是否能入库还是写文件
					
					if (petri_status == DB_STATUS_ONLINE)   //判断是否处于维护态
					{
						ret = indb(tmp,orgFileName) ;
						if(ret)								//入库失败或者不全，将文件挪到其他目录
						{
							deal_flag = 'E';
							char other_dir[1024];
							memset(other_dir,0,sizeof(other_dir));
							strcpy(other_dir,it->second.szSourcePath);
							strcat(other_dir,other_path);
							theJSLog<<"将文件移到其他目录"<<other_dir<<endi;
							strcat(other_dir,orgFileName);
							rename(tmp,other_dir);
						}
						else
						{
							ret = record_num;

							rename(tmp,full_name) ;				//将临时文件变成正式文件
							theJSLog<<"写文件["<<orgFileName<<"]成功,请查看文件到路径："<<path<<endi;
						}

					}
					else
					{
						//数据库只读 将文件名和路径保存下来vector,等待下次数据库状态正常时再将目录下面的文件入库
						deal_flag  = 'D';
					}
	
					//更新数据库状态信息
					memset(sql,0,sizeof(sql));
					sprintf(sql,"update d_sch_end set deal_flag = '%c',input_count='%d',mainflow_count='%d' where filename = '%s' and source_id = '%s'",deal_flag,record_num,ret,orgFileName,m_szSourceID);
					writeSQL(sql);			
				}
				
				record_num = 0;  //文件记录数清0
			}
			else if(strcmp(block_pos,"D") == 0)	//表示单独一个文件一个话单块 ，正常
			{
				//获取petri网状态
				//rtinfo.getSysMode(petri_status);

				// 将文件注册到调度表，文件获取当前文件时间,file_id,以第一条为准
				getCurTime(currTime);							
				outrcd.Set_record(pkg.readPkgRecord(1).record);
				char file_id[10];
				memset(file_id,0,sizeof(file_id));
				memset(sql,0,sizeof(sql));
				outrcd.Get_Field(FILE_ID,file_id);
				sprintf(sql,"insert into d_sch_end(source_id,serv_cat_id,filename,deal_flag,dealstarttime,file_id,file_time) values('%s','%s','%s','W','%s',%ld,'%s')",m_szSourceID,mServCatId,orgFileName,currTime,atol(file_id),file_time);
				writeSQL(sql);
				
				//先写临时文件，再改名
				ofstream out(tmp);
				if(!out)
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"Ontask() 写文件,文件%s打开出错",tmp);
					theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败

					return -1;
				}
				for(int k = 1;k<=pkg.getRecordNum();k++)
				{
					PkgFmt fmt = pkg.readPkgRecord(k);
					out<<fmt.record<<"\n";
					//cout<<"第"<<k<<"条记录状态："<<fmt.status<<"  记录编码："<<fmt.code<<"  记录值："<<fmt.record<<endl;
				}		
			
				out.close();
				
				//判断数据库状态
				ret = indb(tmp,orgFileName) ;
				if(ret)									//入库失败或者不全，将文件挪到其他目录
				{
					deal_flag = 'E';
					char other_dir[1024];
					memset(other_dir,0,sizeof(other_dir));
					strcpy(other_dir,it->second.szSourcePath);
					strcat(other_dir,other_path);
					theJSLog<<"将文件移到其他目录"<<other_dir<<endi;
					strcat(other_dir,orgFileName);
					rename(tmp,other_dir);
				}
				else
				{
					ret = record_num;

					rename(tmp,full_name) ;				//将临时文件变成正式文件
					theJSLog<<"写文件"<<orgFileName<<"成功,请查看文件到路径："<<path<<endi;
				}
		
				//更新数据库状态信息
				getCurTime(currTime);
				memset(sql,0,sizeof(sql));
				sprintf(sql,"update d_sch_end set deal_flag='%c',input_count='%d',mainflow_count='%d',dealendtime='%s' where filename='%s' and source_id='%s'",deal_flag,record_num,ret,currTime,orgFileName,m_szSourceID);
				writeSQL(sql);

				record_num = 0;				//文件记录数清0
				
			}
			else													
			{
				theJSLog<<"非法的话单块状态:"<<pkg.getBlkPos()<<endi;
			}		

		}
		else		//本次话单块异常
		{
			theJSLog<<"话单块文件["<<m_szFileName<<"]状态异常"<<endi;

			getCurTime(currTime);
			char erro_sql_tmp[1024];
			memset(erro_sql_tmp,0,sizeof(erro_sql_tmp));
			strcpy(erro_sql_tmp,erro_sql);
			strcat(erro_sql_tmp,".tmp");

			ofstream erroFile(erro_sql_tmp,ios::app);	//追加方式写错误登记信息文件,先写临时的
			if(!erroFile)
			{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"Ontask() 打开文件%s失败！！！ ",erro_sql_tmp);
					theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败
					return -1;
			}
			
			//CF_MemFileI _infile;
			//_infile.Init(m_szOutTypeId);
			//CFmt_Change fmts;			
			//fmts.Init(m_szOutTypeId);

			char errocode[10],erro_col[10],errline[10],errseq[10],source_id[5];

			theJSLog<<"登记话单块错误记录到错误SQL文件"<<endi;
			for(int k = 1;k<=pkg.getRecordNum();k++)
			{
				PkgFmt fmt = pkg.readPkgRecord(k);
				if(strcmp(fmt.status , "E") == 0)
				{		
						//cout<<"记录值："<<fmt.record<<endl;
					    outrcd.Set_record(fmt.record);				
						//memset(source_id,0,sizeof(source_id));
						memset(errocode,0,sizeof(errocode));
						memset(erro_col,0,sizeof(erro_col));
						memset(errline,0,sizeof(errline));
						memset(errseq,0,sizeof(errseq));				
						memset(sql,0,sizeof(sql));

						//outrcd.Get_Field("SourceID",source_id);
						outrcd.Get_Field(ERR_CODE,errocode);
						outrcd.Get_Field(ERR_COLINDEX,erro_col);
						outrcd.Get_Field(LINE_NUM,errline);
						outrcd.Get_Field(FILE_ID,errseq);

						cout<<"写sql文件，记录错误记录行的信息"<<"数据源ID: "<<source_id<<" 错误码："<<errocode<<"  错误列"<<erro_col<<"  错误行号"<<errline<<" 错误文件序列号"<<errseq<<endl;
						sprintf(sql,"insert into D_ERRFILE_INFO(filename,source_id,deal_time,err_msg,err_code,err_col,err_line,err_seq,state)"
							  "values('%s','%s','%s','%c',%s,%d,%d,%d,'%c')",orgFileName,m_szSourceID,currTime,'R',errocode,atoi(erro_col),atoi(errline),atoi(errseq),'W');
						erroFile<<sql<<endl;
				}
				
			}

			erroFile.close();
			
			//将临时文件改名
			if(rename(erro_sql_tmp,erro_sql))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"错误sql临时文件[%s]改名正式失败: %s",erro_sql_tmp,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_RENAME_ERR,erro_msg);
			}

			//表示单独一个文件一个话单块	//多个话单块块已经到最后一个
			if((strcmp(block_pos,"D") == 0) || (strcmp(block_pos,"E") == 0))		
			{
					commitErrMsg();
					
					record_num = 0;				//文件记录数清0

					char bak_time[15],erro_dir[1024],bak_dir[1024];

					memset(bak_time,0,sizeof(bak_time));
					memset(erro_dir,0,sizeof(erro_dir));
					memset(bak_dir,0,sizeof(bak_dir));	
					memset(sql,0,sizeof(sql));

					//从格式化原始文件目录中拿出放到错误目录,通过处理时间查找
					sprintf(sql,"select deal_time from D_SCH_FORMAT where source_id='%s' and filename='%s' and deal_flag='Y'",m_szSourceID,orgFileName);
					Statement stmt = conn.createStatement();
					stmt.setSQLString(sql);
					if((stmt.execute()))
					{
							stmt>>bak_time;
							strcpy(erro_dir,it->second.szSourcePath);
							strcat(erro_dir,erro_path);
							strcat(erro_dir,orgFileName);
						
							strcpy(bak_dir,it->second.szSourcePath);
							strcat(bak_dir,bak_path);
							strncat(bak_dir,bak_time,6);
							strcat(bak_dir,"/");
							strncat(bak_dir,bak_time+6,2);
							strcat(bak_dir,"/");
							strcat(bak_dir,orgFileName);
							
							theJSLog<<"将原始文件从备份目录"<<bak_dir<<"上传到错误目录"<<erro_dir<<endi;
							link(bak_dir,erro_dir);  //复制一份文件到错误目录
					}
					else
					{
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"没有从格式化登记表中找到文件[%s]",orgFileName);
							theJSLog.writeLog(LOG_CODE_TABLE_QUERY_ERR_CODE,erro_msg);
							return -1;
					}
					
					if(strcmp(block_pos,"E") == 0)
					{
						theJSLog<<"删除临时输出的话单块文件"<<endi;
						if(remove(tmp))
						{
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"临时文件[%s]删除失败: %s",tmp,strerror(errno));
							theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
						}

						file_status = 0;
					}
			}
			else						//多个话单块在中间或者开始 S,M
			{
					file_status = -1;
			}		
	
		}
		
		conn.close();
	}
	catch (SQLException e )
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"OnTask() 数据库操作异常: %s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
		return -1;
	}
	catch (jsexcp::CException &e) 
	{	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"OnTask() : %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		//字段转换出错
		return -1;
	}

   //sleep(3);

   return ret;
}

//子进程退出前的处理
void Write_File::onChildExit()
{
    cout<<"模块3子进程退出"<<endl;
}

//所有子进程完成任务后主进程的处理
int Write_File::onTaskOver(int child_ret)
{
   cout<<"模块3子进程已处理完任务,处子进程理完的记录数"<<child_ret<<endl;
   return  child_ret ;

}


//写文件
int Write_File::writeFile(char* fileName,PkgBlock pkg)
{
		int ret = 0;
		char tmp[1024];
		memset(tmp,0,sizeof(tmp));

		ofstream out(fileName);
		if(!out)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"writeFile 文件%s打开出错",fileName);
			theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败
			return -1;
		}

		for(int k = 1;k<=pkg.getRecordNum();k++)
		{
			PkgFmt fmt = pkg.readPkgRecord(k);
			out<<fmt.record<<"\n";
						
		}		
			
		out.close();		
		return ret ;
}

//文件入库，通过数据源找到对应的表 0表示正常,否则表示异常
int  Write_File::indb(char* file,char* name)
{
	int ret = -1;
	char szBuff[1024];
	
	//测试***********************：
	//strcpy(m_szSourceID,"HD");
	//strcpy(m_szOutTypeId,"SJSTD");
	//strcpy(m_szFileName,"HDC.2013.201307181150");
	//file = "/mboss/jtcbs/zbjs1_a/data/service/HED/HD/end_path/HDC.2013.201307181150";

	//if(!(dbConnect(conn)))
	//{
	//	cout<<"连接数据库失败 connect error."<<endl;
	//	return -1 ;
	//}

	theJSLog<<"文件["<<name<<"]入库"<<endi;

	map< string,CF_CError_Table>::const_iterator iter =  mapTabConf.find(m_szSourceID);
	if( iter == mapTabConf.end())
	{
		sprintf(erro_msg,"indb() 数据库中没有数据源%s的配置信息",m_szSourceID);
		theJSLog.writeLog(LOG_CODE_PARAM_INFO_LACK,erro_msg);	
		return -1;
	}
	
	Statement stmt = conn.createStatement();

	try
	{
	
	ifstream in(file,ios::in) ;
	if(!in)
	{
		sprintf(erro_msg,"indb() 文件%s打开出错",file);
		theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败

		return -1;
	}
	
	CF_CError_Table tab = iter->second;
	tab.setFileName(name,m_szSourceID,stmt);  
		
	memset(szBuff,0,sizeof(szBuff));
	//CFmt_Change rc;
	//rc.Init(m_szOutTypeId);
	
	while(in.getline(szBuff,sizeof(szBuff)))   
	{		
			outrcd.Set_record(szBuff);
			tab.dealInsertRec(outrcd,NULL,NULL,NULL);
			ret++;
			//if(ret == 8)  
			//{
			//	cout<<"文件入库失败！"<<endl;
			//	return ret ;
			//}
			memset(szBuff,0,sizeof(szBuff));		
	}	

	tab.commit();			//防止没达到记录条数插入条件，强制插入	
	stmt.close();

	in.close();

	}
	catch (jsexcp::CException e)
	{
		sprintf(erro_msg,"indb() error: %s",e.GetErrMessage());
		theJSLog.writeLog(0,erro_msg);
		stmt.close();
		//in.close();
		return ret ;
	}
	catch(SQLException e)
	{
		sprintf(erro_msg,"indb() error: %s [%s]",e.what(),szBuff);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);
    	stmt.rollback();
		//in.close();
    	return ret ;
    }
	

	return 0;
}

//提交错误记录信息,扫描错误sql文件，插入到错误表中
int Write_File::commitErrMsg()
{
	int rett = 0;
	char szBuff[1024];
	ifstream in(erro_sql,ios::in) ;
	if(!in)
	{
		sprintf(erro_msg,"commitErrMsg 文件%s打开出错",erro_sql);
		theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败
		return -1;
	}
	
	theJSLog<<"提交错误记录信息到登记表"<<endi;
	
	memset(szBuff,0,sizeof(szBuff));
	while(in.getline(szBuff,sizeof(szBuff)))   
	{	
		writeSQL(szBuff);
		memset(szBuff,0,sizeof(szBuff));		
	}

	in.close();

	in.open(erro_sql,ios::trunc);	//清空文件
	in.close();

	return rett;

}

//从指定路径删除指定格式的文件
int Write_File::deleteFile(char* path,char* filter)
{
	char tmp[512],fileName[1024];
	int rett = 0;
	memset(tmp,0,sizeof(tmp));
	
	strcpy(tmp,filter);
	strcat(tmp,"*");
	scan.openDir(path);
	while(1)
	{
		memset(fileName,0,sizeof(fileName));
		rett = scan.getFile(filter,fileName);  				
		if(rett == 100)
		{	
			cout<<"不存在指定格式"<<filter<<"的文件"<<endl;
			return 0;
		}
		if(rett == -1)
		{
			return -1 ;			//表示获取文件信息失败
		}
		
		theJSLog<<"删除扫描到的文件"<<fileName<<endi;
		remove(fileName);
		
	}
	
	return rett;
}

//容灾初始化
bool Write_File::drInit()
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
int Write_File::drVarGetSet(char* serialString)
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
 bool Write_File::IsAuditSuccess(const char* dealresult)
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

bool Write_File::CheckTriggerFile()
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
	cout<<"*            writeFile                       * "<<endl;
	cout<<"*              Version 1.0.0	                * "<<endl;
	cout<<"*    last update time :  2013-08-29 by  hed	* "<<endl;
	cout<<"********************************************** "<<endl;


	Write_File fm ;
 
	if( !fm.init( argc, argv ) )
	{
     return -1;
	}
	
	//fm.indb("/mboss/jtcbs/zbjs1_a/data/service/HED/HD/end_path/HDC.2013.201307181148");
	//fm.indb("/mboss/jtcbs/zbjs1_a/data/service/HED/HD/end_path/HDC.2013.201307301230","HDC.2013.201307301230");

	fm.run();

	return 0;
    

}


