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
//SGW_RTInfo	rtinfo;

Write_File::Write_File()
{  
	file_status = 0;
	record_num = 0;
	file_id = 0;

	memset(mServCatId,0,sizeof(mServCatId));
	memset(m_szSourceID,0,sizeof(m_szSourceID));
	memset(m_szFileName,0,sizeof(m_szFileName));
	memset(sql,0,sizeof(sql));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(currTime,0,sizeof(currTime));
}


Write_File::~Write_File()
{
	
}


//模块初始化动作
bool Write_File::init(int argc,char** argv)
{
   
   if(!PS_BillProcess::init(argc,argv))
   {
      return false;
   }
	
	//获取petri网状态,当系统为只读态时,数据库更新操作语句写文件
	//if(!(rtinfo.connect()))
	//{
	//	return false;
	//}	
	//rtinfo.getDBSysMode(petri_status);
	//cout<<"petri status:"<<petri_status<<endl;

	//*********2013-07-15 读取数据库的配置信息，如数据源组，错误目录 2013-03-11 add by hed*********************************************/
	
	if(!(dbConnect(conn)))
	{
		cout<<"连接数据库 connect error."<<endl;	//写日志
		return false ;
	}

	mConfParam.iflowID = getFlowID();
	mConfParam.iModuleId = getModuleID(); 

	try{

		string sql = "select source_group from TP_BILLING_LINE  where billing_line_id = :1";
		Statement stmt = conn.createStatement();
		stmt.setSQLString(sql);
		stmt<<mConfParam.iflowID;
		stmt.execute();
		if(!(stmt>>mConfParam.szSrcGrpID))
		{
			cout<<"请在tp_billing_line表中配置流水线["<<mConfParam.iflowID<<"]的数据源组"<<endl;
			return false ;
		}

		sql = "select ext_param from TP_PROCESS where billing_line_id = :1 and module_id = :2";
		stmt.setSQLString(sql);
		stmt<<mConfParam.iflowID<<mConfParam.iModuleId;
		stmt.execute();
		if(!(stmt>>mConfParam.szService))
		{
			cout<<"请在tp_process表字段ext_param中配置模块["<<mConfParam.iModuleId<<"]service"<<endl;
			return false ;
		}
		
		theJSLog.setLog(szLogPath,szLogLevel,mConfParam.szService, mConfParam.szSrcGrpID, 001);
		
		theJSLog<<"szLogPath="<<szLogPath<<"	szLogLevel="<<szLogLevel<<endi;
		theJSLog<<"flowID="<<mConfParam.iflowID<<"	ModuleId="<<mConfParam.iModuleId<<endi;
		theJSLog<<"szSrcGrpID="<<mConfParam.szSrcGrpID<<"	szService="<<mConfParam.szService<<endi;


		sql = "select a.workflow_id,c.output_path,b.input_id,b.output_id,c.log_tabname from C_SOURCE_GROUP_DEFINE a,C_SERVICE_FLOW b,C_SERVICE_INTERFACE c where a.source_group=:1" 
                    "and a.workflow_id = b.workflow_id and b.service=:2 and b.input_id = c.interface_id ";
		stmt.setSQLString(sql);
		stmt<<mConfParam.szSrcGrpID<<mConfParam.szService;
		stmt.execute();
		if(!(stmt>>mConfParam.iWorkflowId>>mConfParam.szOutPath>>mConfParam.iInputId>>mConfParam.iOutputId>>mConfParam.szSchCtlTabname))
		{
			theJSLog<<"C_SOURCE_GROUP_DEFINE,C_SERVICE_INTERFACE,C_SERVICE_INTERFACE关联查询失败:"<<sql<<endw;		
			return false ;
		}
		completeDir(mConfParam.szOutPath);
		
		theJSLog<<"WorkflowId="<<mConfParam.iWorkflowId<<"	InputId="<<mConfParam.iInputId
				<<"	 OutputId="<<mConfParam.iOutputId<<"	sch_table="<<mConfParam.szSchCtlTabname<<endi;

		sql = "select var_value from c_process_env where varname = 'WR_FILE_ERR_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<mConfParam.szSrcGrpID<<mConfParam.szService;
		stmt.execute();
		if(!(stmt>>mConfParam.szErroPath))
		{
				theJSLog<<"请在表c_process_env中配置写文件模块的错误路径 WR_FILE_ERR_DIR"<<endw; //写日志
				return false;
		}		
		completeDir(mConfParam.szErroPath);
	
		sql = "select var_value from c_process_env where varname = 'WR_FILE_BAK_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<mConfParam.szSrcGrpID<<mConfParam.szService;
		stmt.execute();
		if(!(stmt>>mConfParam.szSrcBakPath))
		{
			theJSLog<<"请在表c_process_env中配置写文件模块的备份目录(源文件目录) WR_FILE_BAK_DIR"<<endw;		//写日志
			return false;
		}
		completeDir(mConfParam.szSrcBakPath);
		
		theJSLog<<"szOutPath="<<mConfParam.szOutPath<<"  szErroPath="<<mConfParam.szErroPath<<" szSrcBakPath="<<mConfParam.szSrcBakPath<<endi;

		stmt.close();
	   
	   }catch(SQLException e)
		{
			theJSLog<<"初始化时数据库查询异常:"<<e.what()<<endw;
			return false ;
		} 

	//初始化内存日志接口
	bool bb = initializeLog(argc,argv,false);  //是否调试模式
	if(!bb)
	{
			return false;
	}
	
	//theJSLog.setLog(szLogPath,szLogLevel,m_szService , m_szSrcGrpID, 001);	//文件日志接口，调用了内存日志接口
	//theJSLog<<"数据源组："<<m_szSrcGrpID<<"   service:"<<m_szService<<"  相对输出路径:"<<out_path<<"  错误路径:"<<erro_path
	//		<<"	源文件备份路径:"<<bak_path<<"	日志路径:"<<szLogPath<<" 日志级别:"<<szLogLevel<<" sql存放路径:"<<sql_path<<endi;

	if(LoadSourceCfg() == -1) 
	{		
		return false ;  
	}
	
	conn.close();

	char out_dir[JS_MAX_FILEFULLPATH_LEN],erro_dir[JS_MAX_FILEFULLPATH_LEN],bak_dir[JS_MAX_FILEFULLPATH_LEN],other_dir[JS_MAX_FILEFULLPATH_LEN];
	int rett = 0;
	DIR *dirptr = NULL; 
	for(map<string,SOURCECFG>::const_iterator iter = m_SourceCfg.begin();iter !=m_SourceCfg.end(); ++iter)
	{		   				
			memset(out_dir,0,sizeof(out_dir));
			strcpy(out_dir,iter->second.szSourcePath);
			strcat(out_dir,mConfParam.szOutPath);
			if((dirptr=opendir(out_dir)) == NULL)
			{
					theJSLog<<"数据源【"<<iter->first<<"】的输出文件路径: "<<out_dir<<"不存在，自行创建"<<endw;
					rett = mkdir(out_dir,0755);
					if(rett == -1)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"数据源[%s]的输出文件文件路径[%s]不存在，自行创建失败",iter->first,out_dir);
						theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
						return false;
					}
			}else closedir(dirptr);
			
			memset(erro_dir,0,sizeof(erro_dir));
			strcpy(erro_dir,iter->second.szSourcePath);
			strcat(erro_dir,mConfParam.szErroPath);
			if((dirptr=opendir(erro_dir)) == NULL)
			{
					theJSLog<<"数据源【"<<iter->first<<"】的错误文件路径: "<<erro_dir<<"不存在，自行创建"<<endw;
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
			strcat(bak_dir,mConfParam.szSrcBakPath);
			if((dirptr=opendir(bak_dir)) == NULL)
			{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"数据源[%s]的原始备份文件路径[%s]不存在",iter->first,bak_dir);
					rett = mkdir(bak_dir,0755);
					//theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
					//return false ;

			}else  closedir(dirptr);
	  
	}

   theJSLog<<"初始化完毕...\n"<<endi;

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
		stmt<<mConfParam.szSrcGrpID;
		if(stmt.execute())
		{
			stmt>>mConfParam.szOutputFiletypeId;
		}
		
		theJSLog<<"szOutputFiletypeId="<<mConfParam.szOutputFiletypeId<<endi;
		outrcd.Init(mConfParam.szOutputFiletypeId);

   		//expTrace(szDebugFlag, __FILE__, __LINE__,"%s;%s",m_szSrcGrpID,m_szOutTypeId);
		sql = "select count(*) from C_SOURCE_GROUP_CONFIG where SOURCE_GROUP=:1";
		stmt.setSQLString(sql);
		stmt<<mConfParam.szSrcGrpID;
		if(stmt.execute())
		{
			stmt>>iSourceCount;
		}
		
		theJSLog<<"iSourceCount="<<iSourceCount<<endi;
		//expTrace(szDebugFlag, __FILE__, __LINE__, "iSourceCount=%d;", iSourceCount);
		
		sql = "select a.source_id,b.source_path,b.SERV_CAT_ID from C_SOURCE_GROUP_CONFIG a,I_SOURCE_DEFINE b where SOURCE_GROUP=:1 and a.source_id=b.source_id";
		stmt.setSQLString(sql);
		stmt<<mConfParam.szSrcGrpID;
		if(stmt.execute())
		{
			for (int i=0; i<iSourceCount; i++)
			{
				SOURCECFG SourceCfg;
				string strSourceId;

				stmt>>SourceCfg.szSourceId>>SourceCfg.szSourcePath>>SourceCfg.serverCatID;      
				strSourceId=SourceCfg.szSourceId;
				
				completeDir(SourceCfg.szSourcePath);

				if(getSourceFilter(SourceCfg.szSourceId,SourceCfg.filterRule,SourceCfg.file_begin,SourceCfg.file_length))
				{
							return -1;
				}
				
				m_SourceCfg[strSourceId]=SourceCfg;

				theJSLog<<"szSourceId="<<SourceCfg.szSourceId<<" szSourcePath="<<SourceCfg.szSourcePath<<" serverCatID="<<SourceCfg.serverCatID
						<<" filterRule="<<SourceCfg.filterRule<<"  filetime_begin="<<SourceCfg.file_length<<"  filetime_length="<<SourceCfg.file_length<<endi;
		     }
		}
		
	}catch (SQLException e)
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"LoadSourceCfg() 数据库出错：%s (%s)",e.what(),sql);
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
		Statement stmt = conn.createStatement();
		string sql = "select file_filter,file_time_index_len from C_FILE_RECEIVE_ENV where source_id = :1 ";		
		stmt.setSQLString(sql);
		stmt << source;
		stmt.execute();
		if(!(stmt>>filter>>file_time))
		{
				stmt.close();
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"数据源[%s]没有配置过滤规则或者文件名时间截取规则",source);
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return -1;
		}
		
		//cout<<"file_time = "<<file_time<<endl;

		char tmp[6];
		memset(tmp,0,sizeof(tmp));	
		strcpy(tmp,file_time.c_str());	
		vector<string> fileTime;		
		splitString(tmp,",",fileTime,false);
		if(fileTime.size() != 2)
		{
			stmt.close();
			memset(erro_msg,0,sizeof(erro_msg));
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
			memset(erro_msg,0,sizeof(erro_msg));
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
		sprintf(erro_msg,"getSourceFilter 数据库查询异常: %s (%s)",e.what(),sql);
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

//判断是否要申请话单块
int Write_File::onBeforeTask()
{
	//cout<<"申请话单块"<<endl;
	theJSLog.reSetLog();
	sleep(1);

	return 1;		//默认要申请话单块
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

//获取格式化备份时间目录,查找30天以内的目录是否存在源文件
int Write_File::getBackTimeDir(char* date_dir,char* orgFileName)  
{
	int ret = -1;
	char date[8+1],tmp_date[8+1],tmp[JS_MAX_FILEFULLPATH_LEN];
	memset(tmp,0,sizeof(tmp));
	memset(date,0,sizeof(date));
	memset(tmp_date,0,sizeof(tmp_date));
	getCurTime(currTime);
	
	strncpy(date,currTime,8);		//20130916
	date[8] = '\0';
	strcpy(tmp_date,date);

	for(int i = 0;i<31;i++)
	{
		memset(tmp,0,sizeof(tmp));
		strcpy(tmp,it->second.szSourcePath);
		strcat(tmp,mConfParam.szSrcBakPath);

		strncat(tmp,tmp_date,6);
		strcat(tmp,"/");
		strncat(tmp,tmp_date+6,2);
		strcat(tmp,"/");
		strcat(tmp,orgFileName);

		if(access(tmp,F_OK) == 0) 
		{
			theJSLog<<"从格式化备份目录找到错误话单文件["<<tmp<<"]"<<endi;
			ret = 0;
			strcpy(date_dir,tmp_date);
			return ret;
		}
		
		addDays(-1,date,tmp_date);
		strcpy(date,tmp_date);
	}
	
	return ret;
}


//处理成功返回话单条数(>=0)
int Write_File::onTask(void *task_addr, int offset, int ticket_num)
{

    //cout<<"任务地址"<<task_addr<<endl;		 	
	PkgBlock pkg((char*)task_addr);
	pkg.init(getTicketLength(),getBlockSize());
	
	pkg.setModuleId(getModuleID());

	if(pkg.getRecordNum() == 0)
	{
		theJSLog<<"话单块记录条数为0,有误..."<<endw;
		return ticket_num;
	}

	int ret = 0;
	char tmp[JS_MAX_FILEFULLPATH_LEN],path[JS_MAX_FILEFULLPATH_LEN],orgFileName[JS_MAX_FILENAME_LEN],full_name[JS_MAX_FILEFULLPATH_LEN],block_pos[2];

	memset(tmp,0,sizeof(tmp));
	memset(path,0,sizeof(path));
	memset(orgFileName,0,sizeof(orgFileName));
	memset(full_name,0,sizeof(full_name));
	memset(block_pos,0,sizeof(block_pos));

	strcpy(block_pos,pkg.getBlkPos());  //获取话单块位置

	record_num += pkg.getRecordNum();
	
	strcpy(m_szSourceID,pkg.getSourceId());
	it  = m_SourceCfg.find(string(m_szSourceID));
	if(it == m_SourceCfg.end())
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"配置中不存在数据源[%s]的信息",m_szSourceID);
		theJSLog.writeLog(LOG_CODE_SOURCE_NOT_FIND,erro_msg);
		return -1;
	}
	
	strcpy(m_szFileName,pkg.getFileName());
	strcpy(path,it->second.szSourcePath);
	strcat(path,mConfParam.szOutPath);
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

 try
 {	
	//表示话单文件开始块,其实前面的信息都可以在话单块开始时取得,避免每个话单块都去取
	if((strcmp(block_pos,"S") == 0) || (strcmp(block_pos,"D") == 0))
	{
		fileOverFlag = false;

		theJSLog<<"######## start deal file " <<orgFileName<<" ########"<<endi;

		char fileid[10];
		memset(fileid,0,sizeof(fileid));
		outrcd.Set_record(pkg.readPkgRecord(1).record);
		outrcd.Get_Field(FILE_ID,fileid);
		file_id = atol(fileid);
		setSQLFileName(orgFileName);
		memset(file_time,0,sizeof(file_time));			//重文件名上面截取时间
		strncpy(file_time,orgFileName+it->second.file_begin,it->second.file_length);
		file_time[8] = '\0';
		strcpy(mServCatId,it->second.serverCatID);

		theJSLog<<"source_id:"<<m_szSourceID<<" file_time:"<<file_time <<" file_id:"<<file_id<<endi;
	}
	theJSLog<<"处理话单块,split_filename:"<<m_szFileName<<" status:"<<pkg.getStatus()<<" record_num:"<<pkg.getRecordNum()<<" pos:"<<pkg.getBlkPos()<<endi;

	strcat(full_name,orgFileName);
	strcpy(tmp,full_name);	 //正式文件
	strcat(tmp,".tmp");		//临时文件
	
	//判断话单块是否正确，若正确，则写临时文件，
	//若不正确，则先删除以前的该话单块文件，写sql异常记录信息，然后循环话单块的每条记录将错误信息登记	
	//2013-07-17 判断话单块的在文件中位置通过话单块的头部标记判断S表示开始 E结束 M中间 D单独
	//做异常回退的数据库登记ERRFILE_INFO：文件名、数据源、时间、错误原因（F/R）、错误码、错误行、错误列序号、
	//处理状态---由出错的程序做登记	内部格式写死下列字段：LINE_NUM,ERR_CODE,ERR_LINE,FILE_ID
	
		if(pkg.getStatus() == 2)					 //话单块正常
		{
			if((strcmp(block_pos,"S") == 0) || (strcmp(block_pos,"M") == 0))
			{
				if(strcmp(block_pos,"S") == 0)				//第一个模块
				{
					ofstream out(tmp);						//防止出现异常后文件名相同的情况,清空文件
					if(!out)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"Ontask() 文件[%s]打开出错",tmp);
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
				else
				{
					if(file_status == 0)
					{
						ofstream out(tmp,ios::app);
						if(!out)
						{
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"Ontask() 文件[%s]打开出错",tmp);
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
				}
				
			}
			else if(strcmp(block_pos,"E") == 0)					//多个话单块已经到最后一个块
			{	
				if(file_status == -1)
				{
					//theJSLog<<"删除临时输出的话单块文件"<<endi; 有可能临时文件都没生成,先判断文件是否存在
					if(access(tmp,F_OK) == 0)
					{
						if(remove(tmp))
						{
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"临时文件[%s]删除失败: %s",tmp,strerror(errno));
							theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
						}
					}

					//从格式化原始文件目录中拿出放到错误目录
					char bak_time[8+1],erro_dir[JS_MAX_FILEFULLPATH_LEN],bak_dir[JS_MAX_FILEFULLPATH_LEN];
					memset(bak_time,0,sizeof(bak_time));
					memset(erro_dir,0,sizeof(erro_dir));
					memset(bak_dir,0,sizeof(bak_dir));				
					//2013-09-16
					if(getBackTimeDir(bak_time,orgFileName) == 0)
					{	
						strcpy(erro_dir,it->second.szSourcePath);
						strcat(erro_dir,mConfParam.szErroPath);
						strcat(erro_dir,orgFileName);
						
						strcpy(bak_dir,it->second.szSourcePath);
						strcat(bak_dir,mConfParam.szSrcBakPath);
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
						sprintf(erro_msg,"无法从备份目录[%s]查找到源文件[%s]",mConfParam.szSrcBakPath,orgFileName);
						theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);
					}

					//即使是错误话单块也要做登记调度表 2013-09-02
					getCurTime(currTime);
					memset(sql,0,sizeof(sql));							
					sprintf(sql,"insert into %s (source_id,serv_cat_id,filename,deal_flag,dealtime,input_count,file_id,file_time) values('%s','%s','%s','E','%s',%d,%ld,'%s')",mConfParam.szSchCtlTabname,m_szSourceID,mServCatId,orgFileName,currTime,record_num,file_id,file_time);
					writeSQL(sql);

					theJSLog<<"提交sql文件..."<<endi;
					commitSQLFile();
				/*	
					//2013-10-24
					rtinfo.getDBSysMode(petri_status);
					if(petri_status == DB_STATUS_OFFLINE)
					{
						theJSLog<<"数据库状态为只读态,写sql文件"<<endi;
						commitSQLFile();
					}
					else
					{
						if(!(dbConnect(conn)))
						{
							commitSQLFile();
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"onTask() 连接数据库失败,写sql文件");
							theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
						}
						else
						{
							theJSLog<<"更新数据库登记表..."<<endi;
							vsql = getvSQL();
							updateDB();
							conn.close();
						}	
 					}	
				*/
					file_status = 0;
				}
				else					//正常
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
				
					if(rename(tmp,full_name))				//将临时文件变成正式文件
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"临时文件[%s]改名失败: %s",tmp,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_RENAME_ERR,erro_msg); // 文件改名出错
					}
					else
					{
						theJSLog<<"写文件"<<orgFileName<<"成功,请查看文件到路径:"<<path<<endi;
					}		
	
					//插入记录值到调度表
					getCurTime(currTime);
					memset(sql,0,sizeof(sql));							
					sprintf(sql,"insert into %s (source_id,serv_cat_id,filename,deal_flag,dealtime,input_count,file_id,file_time) values('%s','%s','%s','Y','%s',%d,%ld,'%s')",mConfParam.szSchCtlTabname,m_szSourceID,mServCatId,orgFileName,currTime,record_num,file_id,file_time);
					writeSQL(sql);
					
					theJSLog<<"提交sql文件..."<<endi;
					commitSQLFile();
				   /*
					//2013-10-24
					rtinfo.getDBSysMode(petri_status);
					if(petri_status == DB_STATUS_OFFLINE)
					{
						theJSLog<<"数据库状态为只读态,写sql文件"<<endi;
						commitSQLFile();
					}
					else
					{
						if(!(dbConnect(conn)))
						{
							theJSLog<<"连接数据库失败,写sql文件"<<endw;
							commitSQLFile();
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"onTask() 连接数据库失败,写sql文件");
							theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
						}
						else
						{
							theJSLog<<"更新数据库登记表..."<<endi;
							vsql = getvSQL();
							updateDB();
							conn.close();
						}	
 					}	
					*/
				}	
				record_num = 0;  //文件记录数清0
				theJSLog<<"######## end deal file ########\n"<<endi;
			}
			else if(strcmp(block_pos,"D") == 0)	//表示单独一个文件一个话单块 ，正常
			{	
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
				
				if(rename(tmp,full_name))				//将临时文件变成正式文件
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"临时文件[%s]改名失败: %s",tmp,strerror(errno));
					theJSLog.writeLog(LOG_CODE_FILE_RENAME_ERR,erro_msg); // 文件改名出错
				}
				else
				{
					theJSLog<<"写文件"<<orgFileName<<"成功,请查看文件到路径:"<<path<<endi;
				}

				//插入记录值到调度表
				getCurTime(currTime);
				memset(sql,0,sizeof(sql));		
				sprintf(sql,"insert into %s (source_id,serv_cat_id,filename,deal_flag,dealtime,input_count,file_id,file_time) values('%s','%s','%s','Y','%s',%d,%ld,'%s')",mConfParam.szSchCtlTabname,m_szSourceID,mServCatId,orgFileName,currTime,record_num,file_id,file_time);
				writeSQL(sql);
				
				theJSLog<<"提交sql文件..."<<endi;
				commitSQLFile();
			    /*
				//2013-10-24
				rtinfo.getDBSysMode(petri_status);
				if(petri_status == DB_STATUS_OFFLINE)
				{
					theJSLog<<"数据库状态为只读态,写sql文件"<<endi;
					commitSQLFile();
				}
				else
				{
					if(!(dbConnect(conn)))
					{
						theJSLog<<"连接数据库失败,写sql文件"<<endw;
						commitSQLFile();
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"onTask() 连接数据库失败,写sql文件");
						theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
					}
					else
					{
						theJSLog<<"更新数据库登记表..."<<endi;
						vsql = getvSQL();
						updateDB();
						conn.close();
					}	
 				}		
				*/
				record_num = 0;				//文件记录数清0
				theJSLog<<"######## end deal file ########"<<endi;
			}
			else													
			{
				theJSLog<<"非法的话单块状态:"<<pkg.getBlkPos()<<endw;
			}		

		}
		else if(pkg.getStatus() == 3)		//本次话单块异常
		{
			theJSLog<<"话单块文件["<<m_szFileName<<"]状态异常"<<endw;

			char errocode[10],erro_col[10],errline[10],errseq[10],source_id[5+1];		
			getCurTime(currTime);
			theJSLog<<"登记话单块错误记录到错误SQL文件"<<endi;
			for(int k = 1;k<=pkg.getRecordNum();k++)
			{
				PkgFmt fmt = pkg.readPkgRecord(k);
				if(strcmp(fmt.type , "E") == 0)
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

						//cout<<"记录错误记录行的信息"<<"数据源ID: "<<source_id<<" 错误码："<<errocode<<"  错误列"<<erro_col<<"  错误行号"<<errline<<" 错误文件序列号"<<errseq<<endl;
						sprintf(sql,"insert into D_ERRFILE_INFO(filename,source_id,deal_time,err_msg,err_code,err_col,err_line,err_seq,state)"
							  "values('%s','%s','%s','%c','%s',%d,%d,%d,'%c')",orgFileName,m_szSourceID,currTime,'R',errocode,atoi(erro_col),atoi(errline),atoi(errseq),'W');
						writeSQL(sql);
				}
				
			}

			//表示单独一个文件一个话单块	//多个话单块块已经到最后一个
			if((strcmp(block_pos,"D") == 0) || (strcmp(block_pos,"E") == 0))		
			{
					if(strcmp(block_pos,"E") == 0)
					{
						//theJSLog<<"删除临时输出的话单块文件"<<endi; 有可能临时文件都没生成,先判断文件是否存在
						if(access(tmp,F_OK) == 0)
						{			
							if(remove(tmp))
							{
								memset(erro_msg,0,sizeof(erro_msg));
								sprintf(erro_msg,"临时文件[%s]删除失败: %s",tmp,strerror(errno));
								theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
							}
						}

						file_status = 0;
					}

					//即使是错误话单块也要做登记调度表 2013-09-02
					getCurTime(currTime);
					memset(sql,0,sizeof(sql));							
					sprintf(sql,"insert into %s (source_id,serv_cat_id,filename,deal_flag,dealtime,input_count,file_id,file_time) values('%s','%s','%s','E','%s',%d,%ld,'%s')",mConfParam.szSchCtlTabname,m_szSourceID,mServCatId,orgFileName,currTime,record_num,file_id,file_time);
					writeSQL(sql);
					
					record_num = 0;				//文件记录数清0

					char bak_time[8+1],erro_dir[JS_MAX_FILEFULLPATH_LEN],bak_dir[JS_MAX_FILEFULLPATH_LEN];
					memset(bak_time,0,sizeof(bak_time));
					memset(erro_dir,0,sizeof(erro_dir));
					memset(bak_dir,0,sizeof(bak_dir));	
					
					if(getBackTimeDir(bak_time,orgFileName) == 0)
					{		
							strcpy(erro_dir,it->second.szSourcePath);
							strcat(erro_dir,mConfParam.szErroPath);
							strcat(erro_dir,orgFileName);
						
							strcpy(bak_dir,it->second.szSourcePath);
							strcat(bak_dir,mConfParam.szSrcBakPath);
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
							sprintf(erro_msg,"无法从备份目录[%s]查找到源文件[%s]",mConfParam.szSrcBakPath,orgFileName);
							theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);
					}
					
					theJSLog<<"提交sql文件..."<<endi;
					commitSQLFile();
				/*
					//2013-10-24
					rtinfo.getDBSysMode(petri_status);
					if(petri_status == DB_STATUS_OFFLINE)
					{
						theJSLog<<"数据库状态为只读态,写sql文件"<<endi;
						commitSQLFile();
					}
					else
					{
						if(!(dbConnect(conn)))
						{
							theJSLog<<"连接数据库失败,写sql文件"<<endw;
							commitSQLFile();
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"onTask() 连接数据库失败,写sql文件");
							theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
						}
						else
						{
							theJSLog<<"更新数据库登记表..."<<endi;
							vsql = getvSQL();
							updateDB();
							conn.close();
						}	
 					}			
				*/
					theJSLog<<"######## end deal file ########\n"<<endi;
			}
			else										//多个话单块在中间或者开始 S,M
			{
					file_status = -1;
			}		
	
		}
		else if(pkg.getStatus() == -1)					//表示仲裁失败,一定是最后一块D,E
		{
			theJSLog<<"文件["<<orgFileName<<"]仲裁失败,舍弃"<<endi;
			record_num = 0;								//文件记录数清0
			if(strcmp(block_pos,"E") == 0)
			{
				//theJSLog<<"删除临时输出的话单块文件"<<endi; 有可能临时文件都没生成,先判断文件是否存在
				if(access(tmp,F_OK) == 0)
				{			
					if(remove(tmp))
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"临时文件[%s]删除失败: %s",tmp,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
					}
				}

				file_status = 0;
			}

			theJSLog<<"######## end deal file ########"<<endi;
		}

	}
	catch (jsexcp::CException &e) 
	{	
		rollBackSQL();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"[%s]OnTask() : %s",orgFileName,e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		//字段转换出错
		//return -1;
		ticket_num = -1;
	}
	
	if((strcmp(block_pos,"D") == 0) || (strcmp(block_pos,"E") == 0))
	{
		fileOverFlag = true;
	}

	pkg.clear();

   return ticket_num;
}

/*
//2013-10-24 批量提交sql,保证一个事物完整性
int Write_File::updateDB()
{	
	int ret = 0;
	Statement stmt;
	string ssql;
    try
    {	
		stmt = conn.createStatement();
		for(int i =0;i<vsql.size();i++)
		{	
			//cout<<"sql = "<<vsql[i]<<endl;
			ssql = vsql[i];
			stmt.setSQLString(ssql);
			ret = stmt.execute();
		}
		stmt.close();

		rollBackSQL();			//sql语句成功执行,回滚sql文件
	}
	catch(util_1_0::db::SQLException e)
	{ 
		stmt.rollback();
		commitSQLFile();		//抛异常时写sql文件编译查找问题

		vsql.clear();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"updateDB 数据库出错%s (%s),写sql文件",e.what(),ssql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		return -1;
	}

	vsql.clear();
	
	return ret ;
}
*/

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

int main(int argc,char** argv)
{
    cout<<"********************************************* "<<endl;
	cout<<"*    China Telecom. Telephone Network         "<<endl;
	cout<<"*    InterNational Account Settle System      "<<endl;
	cout<<"*                                             "<<endl;
	cout<<"*           jswritefile                       "<<endl;
	cout<<"*           sys.GJZW.Version 1.0	             "<<endl;
	cout<<"*    created time :      2013-07-01 by  hed   "<<endl;
	cout<<"*    last update time :  2013-12-01 by  hed	 "<<endl;
	cout<<"********************************************* "<<endl;


	Write_File fm ;
 
	if( !fm.init( argc, argv ) )
	{
		return -1;
	}

	fm.run();

	return 0;
    

}


