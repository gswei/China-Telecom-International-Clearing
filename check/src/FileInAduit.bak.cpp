/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-07-22
File:			 FileInAduit.cpp
Description:	 文件核对输出模块
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
#include "FileInAduit.h"

#include "CF_Common.h"
#include "CF_CLogger.h"
CLog theJSLog;


FileInAduit::FileInAduit()
{  
	memset(m_szFileName,0,sizeof(m_szFileName));
	
	memset(receive_path,0,sizeof(receive_path));	
	memset(input_path,0,sizeof(input_path));		
	memset(output_path,0,sizeof(output_path));
	memset(bak_path1,0,sizeof(bak_path1));
	memset(bak_path2,0,sizeof(bak_path2));
	memset(month_input_path,0,sizeof(month_input_path));

	memset(currTime,0,sizeof(currTime));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(sql,0,sizeof(sql));

	m_enable = false;
}

FileInAduit::~FileInAduit()
{

}

//模块初始化动作
bool FileInAduit::init(int argc,char** argv)
{ 
    if(!PS_Process::init(argc,argv))
    {
      return false;
    }
	
	//PS_Process::setSignalPrc(); 

	//cout<<"流水线ID:"<<getFlowID()<<"   模块ID:"<<getModuleID()<<endl;

	 // 从核心参数里面读取日志的路径，级别，
	 char sParamName[256],bak_path[1024];
	 CString sKeyVal;
	 
	 sprintf(sParamName, "file.check.receive_path");			//源文件接收目录
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(receive_path,0,sizeof(receive_path));
		strcpy(receive_path,(const char*)sKeyVal);
		completeDir(receive_path);

	 }
	 else
	 {	
		cout<<"请在核心参数里配置源文件接收路径[file.check.receive_path]"<<endl;
		return false ;
	 }	 
	 
	 sprintf(sParamName, "file.check.output_path");			  //源文件输出目录
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(output_path,0,sizeof(output_path));
		strcpy(output_path,(const char*)sKeyVal);
		completeDir(output_path);
	 }
	 else
	 {	
		cout<<"请在核心参数里配置源文件的输出路径[file.check.output_path]"<<endl;
		return false ;
	 }	


	 sprintf(sParamName, "file.check.input_path");			//核对文件输入目录
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(input_path,0,sizeof(input_path));
		strcpy(input_path,(const char*)sKeyVal);

	 }
	 else
	 {	
		cout<<"请在核心参数里配置核对文件的输入路径[file.check.input_path]"<<endl;
		return false ;
	 }	 

	 sprintf(sParamName, "file.check.month_input_path");			//月汇总文件输入目录
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(month_input_path,0,sizeof(month_input_path));
		strcpy(month_input_path,(const char*)sKeyVal);
	 }
	 else
	 {	
		cout<<"请在核心参数里配置月汇总文件的输入路径[file.check.month_input_path]"<<endl;
		return false ;
	 }	 
	
	 sprintf(sParamName, "file.check.bak_path");			
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(bak_path,0,sizeof(bak_path));
		strcpy(bak_path,(const char*)sKeyVal);

	 }
	 else
	 {	
		cout<<"请在核心参数里配置核对模块的备份路径[file.check.bak_path]"<<endl;
		return false ;
	 }	 	

	//判断目录是否存在
	 DIR *dirptr = NULL; 
	
	 if((dirptr=opendir(input_path)) == NULL)
	 {
	 	cout<<"核对文件输入目录["<<input_path<<"]打开失败"<<endl;
	 	return false ;

	 }else closedir(dirptr);
	 completeDir(input_path);
	

	 if((dirptr=opendir(month_input_path)) == NULL)
	 {
	 	cout<<"月汇总文件输入目录["<<month_input_path<<"]打开失败"<<endl;
	 	return false ;

	 }else closedir(dirptr);
	 completeDir(month_input_path);

	 if((dirptr=opendir(bak_path)) == NULL)
	 {
	 	cout<<"备份目录["<<bak_path<<"]打开失败"<<endl;
	 	return false ;

	 }else closedir(dirptr);
	  completeDir(bak_path);
	 
	 //备份目录下面应新建两个目录来区分核对文件和月汇总文件
	 strcpy(bak_path1,bak_path);
	 strcat(bak_path1,"AUD");
	 if(chkDir(bak_path1))
	 {
			cout<<"备份目录["<<bak_path1<<"]打开失败"<<endl;
			return false;
	 }
	 completeDir(bak_path1);

	
	 strcpy(bak_path2,bak_path);
	 strcat(bak_path2,"SUM");
	 if(chkDir(bak_path2))
	 {
			cout<<"备份目录["<<bak_path2<<"]打开失败"<<endl;
			return false ;
	 }
	 completeDir(bak_path2);

	 if((dirptr=opendir(sql_path)) == NULL)
	 {		
		cout<<"SQL目录:"<<sql_path<<"打开失败"<<endl;
		return false ;
	 }else  closedir(dirptr);


	//初始化内存日志接口
	bool bb = initializeLog(argc,argv,false);  //是否调试模式
	if(!bb)
	{
			//cout<<"初始化内存日志接口失败"<<endl;
			return false;
	}
	
	//getCurTime(currTime);
	//char tmp[15];
	//memset(tmp,0,sizeof(tmp));
	//strncpy(tmp,currTime,6);
	theJSLog.setLog(szLogPath, szLogLevel,"CHECK_MONTH","CHECK", 001);	//文件日志接口，调用了内存日志接口
	
	theJSLog<<"	日志路径:"<<szLogPath<<" 日志级别:"<<"核对文件输入目录:"<<input_path<<"月汇总文件输入目录:"<<month_input_path
		    <<" 源文件接收目录:"<<receive_path<<" 源文件输出目录:"<<output_path <<"  备份目录:"<<bak_path<<" sql文件目录:"<<sql_path<<endi;
   	
		
	//查询月核对文件的配置信息
	try
	{
		if(!(dbConnect(conn)))
		{
			sprintf(erro_msg,"init() 连接数据库失败 connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
			return  false;
		}
		
		if(LoadSourceCfg() == -1)	    //加载数据源配置信息 2013-08-07
		{
			return false;
		}

		//判断输入输出的路径是否存在
		char input_dir[1024],output_dir[1024];
		for(map<string,SOURCECFG>::const_iterator iter = m_SourceCfg.begin();iter !=m_SourceCfg.end(); ++iter)
		{		   		
			memset(input_dir,0,sizeof(input_dir));
			strcpy(input_dir,iter->second.szSourcePath);
			strcat(input_dir,receive_path);
			if((dirptr=opendir(input_dir)) == NULL)
			{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"数据源[%s]的接收文件路径[%s]不存在",iter->first,input_dir);
					theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错

					return false ;
			}else closedir(dirptr);

			memset(output_dir,0,sizeof(output_dir));
			strcpy(output_dir,iter->second.szSourcePath);
			strcat(output_dir,output_path);
			if((dirptr=opendir(output_dir)) == NULL)
			{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"数据源[%s]的输出文件路径[%s]不存在",iter->first,output_dir);
					theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错

					return false ;
			}else closedir(dirptr);
		}


		Check_Sum_Conf fmt;
		char fee[256];
		memset(fee,0,sizeof(fee));
		
		theJSLog<<"加载月汇总文件配置信息..."<<endi;

		string sql = "select check_type,sum_table,cdr_count,cdr_duration,cdr_fee,rate_cycle from c_check_file_config ";
		Statement stmt = conn.createStatement();
		stmt.setSQLString(sql);
		stmt.execute();
		while(stmt>>fmt.check_type>>fmt.sum_table>>fmt.cdr_count>>fmt.cdr_duration>>fee>>fmt.rate_cycle)
		{
			//cout<<"文件类型："<<fmt.check_type<<endl;
			splitString(fee,",",fmt.cdr_fee,"true");
			monthSumMap.insert( map< string,Check_Sum_Conf >::value_type(fmt.check_type,fmt));

			memset(fee,0,sizeof(fee));
			fmt.cdr_fee.clear();
		}
		
		stmt.close();

		conn.close();
	}
	catch(SQLException  e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init 初始化时数据库查询异常:%s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
		return false ;
	}

  if(!drInit()) return false;
   
   theJSLog<<"初始化完毕"<<endi;

   return true ;
}

//加载数据源配置信息，取全部数据源组的数据源信息
int FileInAduit::LoadSourceCfg()
{
	char m_szSrcGrpID[8];
	int iSourceCount=0;
	string sql ;
	try
	{		
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

					if(getSourceFilter(SourceCfg.szSourceId,SourceCfg.filterRule,SourceCfg.file_begin,SourceCfg.file_length))
					{
							return -1;
					}
		
					m_SourceCfg[strSourceId]=SourceCfg;
				}
			}
			
			memset(m_szSrcGrpID,0,sizeof(m_szSrcGrpID));
		}
		
		stmt2.close();
		stmt.close();

	}catch (SQLException e)
	 {
		sprintf(erro_msg,"LoadSourceCfg数据库出错：%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		return -1;
	 }

	return 0;
}

/******根据数据源获取过滤规则 0没有查到规则，1查到规则了 增加获取文件名上时间的起始位置,和长度*********************/
int FileInAduit::getSourceFilter(char* source,char* filter,int &index,int &length)
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
		sprintf(erro_msg,"getSourceFilter 数据库查询异常: %s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);
		return -1 ;
	}
	catch(jsexcp::CException e)
	{
		sprintf(erro_msg,"getSourceFilter 字段转化出错：%s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);
		return -1;
	}
	
	return 0;
}


//扫描核对详细表中出错的文件，再次核对
int FileInAduit::check_before_file()
{
	int ret = 0;
	
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select check_file,content from d_check_file_detail where check_type  = 'AUD' and check_flag = 'N'");
	try
	{
		if(!(dbConnect(conn)))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"check_before_file() 连接数据库失败 connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
			return  ;
		}
		
		map< string,vector<string> > failFileMap;
		string f1,f2;

		Statement stmt = conn.createStatement();
		stmt.setSQLString(sql);
		stmt.execute();
		while(stmt>>f1>>f2)
		{
			map< string,vector<string> >::iterator	 it = failFileMap.find(f1);
			if(it == failFileMap.end())
			{
				vector<string> vv ;
				vv.push_back(f2);
			    failFileMap.insert(map< string,vector<string> >::value_type(f1,vv));
			}
			else
			{
				it->second.push_back(f2);
			}
		}
		

		stmt.close();
		conn.close();

		if(failFileMap.size() == 0)
		{
				return 0;
		}

		for(map< string,vector<string> >::iterator	it = failFileMap.begin();it != failFileMap.end();++it)
		{
			vector<string> vv = it->second;
			for(int i = 0;i<vv.size();i++)
			{
				ret = check_file_exist(vv[i].c_str());			
				if(ret)									//核对成功就更新原先的记录
				{
					theJSLog<<"文件["<<vv[i]<<"]重新核对成功"<<endi;

					getCurTime(currTime);				//获取当前文件时间
					memset(sql,0,sizeof(sql));
					sprintf(sql,"update d_check_file_detail set check_flag = 'Y',deal_time = '%s' where check_file = '%s' and content = '%s'",currTime,it->first,vv[i]);
					writeSQL(sql);
					memset(sql,0,sizeof(sql));
					sprintf(sql,"update d_check_file_result set suc_cnt = suc_cnt + 1 ,err_cnt = err_cnt -1 ,deal_time = '%s' where check_file = '%s' ",currTime,it->first);
					writeSQL(sql);
				}
			}

		}
		
		//需要增加仲裁信息吗

		
	}
	catch (util_1_0::db::SQLException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"check_before_file() 数据库操作异常%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
	}	

}

//检查输入的文件名是否在文件接收路径下面找到,先通过文件名找到所属数据源 0不存在，1 存在
int FileInAduit::check_file_exist(char* file)
{			
	int ret = 0,flag = 0;
	CF_CFscan scan2;
	char tmp[1024],tmp2[1024], *p,out_path[1024],receive_dir[1024],filter[256];
	string source_id;

	memset(file_time,0,sizeof(file_time));
	//查询文件所属数据源ID,再到目录下面去查找原始文件
	for(map<string,SOURCECFG>::const_iterator it = m_SourceCfg.begin();it != m_SourceCfg.end();++it)
	{
			if(checkFormat(file,it->second.filterRule))		//HDC.2013---    HD*
			{		
					theJSLog<<"文件["<<file<<"]所属数据源:"<<it->first<<endi;

					flag = 1;
					source_id = it->first;
					memset(out_path,0,sizeof(out_path));
					memset(receive_dir,0,sizeof(receive_dir));
					strcpy(out_path,it->second.szSourcePath);
					strcpy(receive_dir,it->second.szSourcePath);

					strncpy(file_time,file+it->second.file_begin,it->second.file_length);
			}
	}
	
	if(flag == 0)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"找不到文件[%s]所属数据源",file);
		theJSLog.writeLog(LOG_CODE_PARAM_INFO_LACK,erro_msg);
		return 0;
	}

	strcat(receive_dir,receive_path);
	flag = scan2.openDir(receive_dir);		//查询文件所在路径
	if(flag)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"打开文件接收目录[%s]失败",receive_dir);
		theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg);		//打开目录出错
		return 0;
	}

	//cout<<"扫描接收目录"<<receive_dir<<endl;
	flag = 1;  
	memset(tmp,0,sizeof(tmp));		
	while(true)
	{
		ret = scan2.getFile("*",tmp);			//找文件
		
		if(ret == 0)
		{
			p = strrchr(tmp,'/');
			memset(tmp2,0,sizeof(tmp2));
			strcpy(tmp2,p+1);

			//cout<<"扫到文件["<<tmp2<<"]  对比文件["<<file<<"]  :"<<ret<<endl;

			if(strcmp(tmp2,file) == 0)
			{							
				//通过文件名找到数据源
				//getSourFromFileName(fileList[i].fileName,source_id);
			/*				
				//通过数据源ID，找打数据源绝对路径+相对输出路径，将该接收文件放到输出路径供格式化使用
				map<string,SOURCECFG>::const_iterator it  = m_SourceCfg.find(source_id);
				if(it == m_SourceCfg.end())
				{
					sprintf(erro_msg,"配置中不存在数据源%s的信息",source_id);
					theJSLog.writeLog(LOG_CODE_PARAM_INFO_LACK,erro_msg);
					flag = 0;
					break;
				}
							
				memset(out_path,0,sizeof(out_path));
				strcpy(out_path,it->second.szSourcePath);
			*/
				
				//cout<<"----------ok"<<endl;
				
				theJSLog<<"文件["<<file<<"]从目录["<<receive_path<<"]移到输出目录"<<endi;

				strcat(out_path,output_path);
				strcat(out_path,tmp2);
							
				if(rename(tmp,out_path) )  //移动文件
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"移动文件失败: %s",strerror(errno));
					theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
					flag = 0;
					break;
				}
		
				break ;
			}
		}
		else if(ret == 100)			//文件扫描完
		{	
			flag = 0;
			break ;	
		}
		else if(ret == -1) 			//获取文件信息失败
		{
			 flag = 0;
			 break;
		}
	}
				
	scan2.closeDir();	
	
	return flag;
}

//扫描输入文件路径，查找上游发过来的核对文件，查询其中的文件名是否都在调度表中有登记，并记录核对信息
void FileInAduit::run(int flag)
{
	cout<<"程序运行！！！！！！"<<flag<<endl;
	
	int ret = -1 ;

	if(gbExitSig)
	{
		if(gbExitSig) theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "应用程序收到退出信号");
		PS_Process::prcExit();
		return;
	}

	try
	{
		if(flag)
		{	
			//2013-08-07 先查询表中是否有核对失败的文件，若有，则再核对，更新状态
			check_before_file();

			if(scan.openDir(input_path))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"打开文件目录[%s]失败",input_path);
				theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
				return ;	
			}	
		}
		else
		{
			if(scan.openDir(month_input_path))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"打开文件目录[%s]失败",month_input_path);
				theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错

				return ;	
			}	

		}

		char tmp[1024];
		int rett = 0;
		while(1)		
		{
				memset(fileName,0,sizeof(fileName));
				if(flag)
				{
					rett = scan.getFile("*.AUD",fileName); 
				}
				else
				{
					rett = scan.getFile("*.SUM",fileName);
				}

				if(rett == 100)
				{		
						break ;
				}
				else if(rett == -1)
				{	
						break ;			//表示获取文件信息失败
				}
				
				/*过滤文件*.tmp,*.TMP,~* */			
				char* p = strrchr(fileName,'/');
				memset(tmp,0,sizeof(tmp));
				strcpy(tmp,p+1);
			
				if(tmp[0] == '~' )	continue ;
				//if(strlen(tmp) <= 3) continue;

				if(strlen(tmp) > 3)						//条件可考虑舍弃，前面扫描文件已过滤
				{		
						int pos = strlen(tmp)-4;
						//cout<<"后缀名为："<<tmp+pos<<endl;
						if((strcmp(tmp+pos,".tmp") && strcmp(tmp+pos,".TMP")) == 0) 
						{
							cout<<"扫描到临时文件，舍弃"<<fileName<<endl;
							continue;
						}
				}

				theJSLog<<"扫描到文件："<<fileName<<endi;
				
				strcpy(m_szFileName,p+1);

				//发送同步串....................

				//判断是核对文件还是月汇总文件，通过文件名来判断
				if(flag) ret = dealFile();
				else	 ret = dealMonthFile();

				//仲裁......................................
		}
		
		scan.closeDir();

	}catch (jsexcp::CException &e) 
	{	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		
	}

}

//处理核对文件，核对规则，通过文件名去查找接收目录目录下面是否有该文件
int FileInAduit::dealFile()
{
	int ret = 0 ;
	try
	{	
		char szBuff[1024];
		ifstream in(fileName,ios::in) ;
		if(!in)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"dealFile 文件%s打开出错",fileName);
			theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败
			return -1;
		}

		memset(szBuff,0,sizeof(szBuff));
		vector<string> vf ;
		int count = 0 ,total = 0,month = 0,cnt = 0,err_cnt = 0,suc_cnt = 0;
		while(in.getline(szBuff,sizeof(szBuff)))   
		{	
			if(count == 0)
			{
				cout<<"文件头:"<<szBuff<<endl;  //可以对文件头进行解析，判断里面文件名的个数
				vf.clear();
				splitString(szBuff,",",vf,true);
				total = atoi(vf[3].c_str());					//解析文件头

				total++;
				count++;
				continue;
			}
			
			if(count == total)		
			{
				cout<<"文件尾："<<szBuff<<endl;
				break ;
			}

			vf.clear();
			splitString(szBuff,";",vf,true);
			if(vf.size() != 3)
			{
				theJSLog<<"核对文件 记录行："<<szBuff<<"格式不对!"<<" 行号："<<count<<endi;
				err_cnt++;
				continue ;
			}

			Check_Rec_Fmt rc ;					//解析文件记录行，文件名，账期标志，账期
			rc.fileName = vf[0];
			rc.rate_flag = vf[1];
			rc.month = vf[2];
			fileList.push_back(rc);
			
			count++;
			memset(szBuff,0,sizeof(szBuff));
			
		}
		in.close();
		
		total--;
		theJSLog<<"记录条数："<<total<<endi;

		char check_flag ;
		
		getCurTime(currTime);    //获取当前文件时间
		
		for(int i = 0;i<fileList.size();i++)
		{		
				cnt = 1;
				check_flag = 'Y';
			
				cnt = check_file_exist(fileList[i].fileName.c_str());
			
				if(cnt == 0)
				{
						theJSLog<<"文件["<<fileList[i].fileName<<"]核对失败"<<endi;
						err_cnt++;
						check_flag = 'N';
				}
				else
				{		theJSLog<<"文件["<<fileList[i].fileName<<"]核对成功"<<endi;
						suc_cnt++;
				}
		
				//每查询一个文件都要登记
				getCurTime(currTime);    //获取当前文件时间
				sprintf(sql,"insert into d_check_file_detail(check_file,content,deal_time,check_flag,check_type,rate_cycle,file_time) values('%s','%s','%s','%c','AUD','%s','%s')",m_szFileName,fileList[i].fileName,currTime,check_flag,fileList[i].month,file_time);		
				writeSQL(sql);
		}

		theJSLog<<"登记文件核对总的情况"<<endi;
		//最后登记总的情况
		sprintf(sql,"insert into d_check_file_result(check_file,total_cnt,suc_cnt,err_cnt,deal_time,check_type) values('%s',%d,%d,%d,'%s','AUD')",m_szFileName,total,suc_cnt,err_cnt,currTime);
		writeSQL(sql);
		commitSQLFile();

		fileList.clear();  //清空文件列表

		//将文件备份 按照日期来备份
		char bak_dir[1024];
		theJSLog<<"文件"<<m_szFileName<<"备份到"<<bak_path1<<endi;
		memset(bak_dir,0,sizeof(bak_dir));
		strcpy(bak_dir,bak_path1);

		strncat(bak_dir,currTime,6);
		completeDir(bak_dir);
		strncat(bak_dir,currTime+6,2);
		completeDir(bak_dir);
		
		if(chkAllDir(bak_dir) == 0)			//备份失败,将程序写改为临时文件,放在当前目录,需要手工干预
		{
			strcat(bak_dir,m_szFileName);
			if(rename(fileName,bak_dir))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"文件[%s]移动[%s]失败: %s",m_szFileName,bak_dir,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);		//移动文件失败
				rename(fileName,strcat(fileName,".tmp"));
			}
		}
		else
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"核对文件备份路径[%s]不存在，且无法创建",bak_dir);
			theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg);		//打开目录出错
			rename(fileName,strcat(fileName,".tmp"));
		}

	}
	catch (jsexcp::CException &e) 
	{	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"dealFile() %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);	
		
		ret = -1;
	}

	return ret ;
}

//处理月汇总文件  文件的头尾都没有校验，可以考虑！！！！！！！！！！！
int FileInAduit::dealMonthFile()
{
	int ret = -1 ;
	try
	{
		if(!(dbConnect(conn)))
		{
			sprintf(erro_msg,"dealMonthFile() 连接数据库失败 connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
			return  ;
		}
		
		
		ifstream in(fileName,ios::in) ;
		if(!in)
		{

			sprintf(erro_msg,"dealMonthFile 文件%s打开出错",sqlFile);
			theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败
			return -1;
		}

		
		int count = 0 ,total = 0,month = 0, err_cnt = 0,suc_cnt = 0;
		long cdr_cnt = 0,cdr_duration = 0;
		double totalFee = 0,feeTmp = 0;
		char check_flag ;
		
		char szBuff[1024];
		memset(szBuff,0,sizeof(szBuff));
		vector<string> vf ;
		while(in.getline(szBuff,sizeof(szBuff)))   
		{	
			if(count == 0)
			{
				cout<<"文件头:"<<szBuff<<endl;  
				vf.clear();
				splitString(szBuff,",",vf,true);
				total = atoi(vf[3].c_str());	//解析文件头，记录类型，文件产生日期，核对账期，记录个数
				month = atoi(vf[2].c_str());

				total++;
				count++;
				continue;
			}
			
			if(count == total)		
			{
				cout<<"文件尾："<<szBuff<<endl;
				break ;

			}

			vf.clear();
			splitString(szBuff,";",vf,false,false);
			if(vf.size() != 4)
			{
				theJSLog<<"月汇总文件 记录行："<<szBuff<<"格式不对!"<<" 行号："<<count<<endi;
				err_cnt++;					//格式不正确 暂时不登记到数据库
				continue ;
			}

			Check_Sum_Rec_Fmt rc ;			//解析文件记录行,文件类型，话单条数，通话时长，总费用
			rc.file_type = vf[0];
			rc.cdr_count = atol(vf[1].c_str());
			rc.cdr_duration = atol(vf[2].c_str());
			rc.cdr_fee = atof(vf[3].c_str());

			fileList2.push_back(rc);
			
			count++;
			memset(szBuff,0,sizeof(szBuff));
			
		}
		in.close();
		
		total--; //月汇总文件总的记录数
		theJSLog<<"记录条数："<<total<<endi;

		getCurTime(currTime);    //获取当前文件时间

		//由于可能存在文件格式不正确，导致总的记录数不一定=匹配到的记录数
		for(int i = 0;i<fileList2.size();i++)
		{		
				check_flag = 'Y';

				Check_Sum_Rec_Fmt ff = fileList2[i];

				map< string,Check_Sum_Conf >::const_iterator iter = monthSumMap.find(ff.file_type);
				if(iter == monthSumMap.end())
				{					
						theJSLog<<"配置表中没有找到月汇总文件文件类型为["<<ff.file_type<<"]的信息"<<endi;
						check_flag = 'N';
						err_cnt++;
						sprintf(sql,"insert into d_check_file_detail(check_file,content,deal_time,check_flag,check_type,rate_cycle) values('%s','%s','%s','%c','SUM','%d')",m_szFileName,ff.file_type,currTime,check_flag,month);
						writeSQL(sql);
						continue;
				}
				
				//查询表，求出对应表中的数据与核对记录进行合计
				sprintf(sql,"select sum(%s),sum(%s)",iter->second.cdr_count,iter->second.cdr_duration);
				vector<string> vfee = iter->second.cdr_fee;
				for(int i = 0;i<vfee.size();i++)
				{
						sprintf(sql,"%s,sum(%s)",sql,vfee[i]);
				}
				sprintf(sql,"%s from %s where %s = %d",sql,iter->second.sum_table,iter->second.rate_cycle,month);

				//cout<<"sql = "<<sql<<endl;

				Statement stmt = conn.createStatement();
				stmt.setSQLString(sql);
				stmt.execute();

				stmt>>cdr_cnt>>cdr_duration;
				totalFee = 0;
				for(int i = 0;i<vfee.size();i++)
				{	
					stmt>>feeTmp;
					totalFee += feeTmp;
					
				}
				
				if(cdr_cnt != fileList2[i].cdr_count)
				{
					theJSLog<<"文件类型：["<<ff.file_type<<"] 通话次数核对失败 "<<cdr_cnt<<" != "<<fileList2[i].cdr_count<<endi;
					check_flag = 'N';
				}
				if(cdr_duration != fileList2[i].cdr_duration)
				{
					theJSLog<<"文件类型：["<<ff.file_type<<"] 通话时长核对失败 "<<cdr_duration<<" != "<<fileList2[i].cdr_duration<<endi;
					check_flag = 'N';
				}
				
				if(totalFee != fileList2[i].cdr_fee)
				{
					theJSLog<<"文件类型：["<<ff.file_type<<"] 费用核对失败 "<<totalFee<<" != "<<fileList2[i].cdr_fee<<endi;
					check_flag = 'N';
				}


				if(check_flag == 'Y')
				{
						theJSLog<<"文件类型：["<<ff.file_type<<"]核对成功"<<endi;
						suc_cnt++;
				}
				else
				{		
						//theJSLog<<"文件类型：["<<ff.file_type<<"]核对失败"<<endi;
						err_cnt++;
				}

				//每查询一个文件类型都要登记
				sprintf(sql,"insert into d_check_file_detail(check_file,content,deal_time,check_flag,check_type,rate_cycle) values('%s','%s','%s','%c','SUM','%d')",m_szFileName,ff.file_type,currTime,check_flag,month);
				writeSQL(sql);
			 
		}

		theJSLog<<"登记月汇总文件总的核对情况"<<endi;
		//最后登记总的情况
		sprintf(sql,"insert into d_check_file_result(check_file,total_cnt,suc_cnt,err_cnt,deal_time,check_type) values('%s',%d,%d,%d,'%s','SUM')",m_szFileName,total,suc_cnt,err_cnt,currTime);
		writeSQL(sql);
		commitSQLFile();
		
		conn.close();

		fileList2.clear();  //清空文件列表

		//将文件备份
		theJSLog<<"文件"<<m_szFileName<<"备份到"<<bak_path2<<endi;
		memset(szBuff,0,sizeof(szBuff));
		strcpy(szBuff,bak_path2);
		strcat(szBuff,m_szFileName);
		rename(fileName,szBuff);



	}catch (util_1_0::db::SQLException e)
	{
		sprintf(erro_msg,"dealFile() 数据库操作异常%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
	}	
	catch (jsexcp::CException &e) 
	{	
		sprintf(erro_msg,"dealFile() %s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		
	}
		
	

	return ret;
}


//容灾初始化
bool FileInAduit::drInit()
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
int FileInAduit::drVarGetSet(char* serialString)
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
 bool FileInAduit::IsAuditSuccess(const char* dealresult)
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

bool FileInAduit::CheckTriggerFile()
{
	int ret = 0;
	if(access(m_triggerFile.c_str(),F_OK) != 0)	return false;

	theJSLog<< "检查到trigger文件，并删除"<< m_triggerFile <<endl;

	ret = remove(m_triggerFile.c_str());	
	if(ret) theJSLog<<"删除trigger失败"<<endi;

}


/**************************************************
*	Function Name:	checkFormat
*	Description:	比较两个字符串是否匹配（相等）
*	Input Param:
*		cmpString -------> 被比较的字符串
*		format	   -------> 匹配的字符串，支持*,?,[]等通配符
*	Returns:
*		如果两个字符串匹配，返回SUC
*		如果两个字符串不匹配，返回FAIL
*	complete:	2001/12/13
******************************************************/
bool FileInAduit::checkFormat(const char *cmpString,const char *format)
{
	while(1)
	{
		switch(*format)
	  	{
	  		case '\0':
					if (*cmpString == '\0')
					{
						return true;
					}
					else
					{
						return false;
					}
			case '!':
					if (checkFormat(cmpString,format + 1) == true)
					{
						return false;
					}
					else
					{
						return true;
					}
			case '?' :
					if(*cmpString == '\0')
					{
						return false;
					}
					return checkFormat(cmpString + 1,format + 1);
			case '*' :
					if(*(format+1) == '\0')
					{
						return true;
					}
					do
					{
						if(checkFormat(cmpString,format+1)==true)
						{
							return true;
						}
					}while(*(cmpString++));
					return false;
			case '[' :
					format++;
					do
					{
						
						if(*format == *cmpString)
						{
							while(*format != '\0' && *format != ']')
							{
								format++;
							}
							if(*format == ']')
							{
								format++;
							}
							return checkFormat(cmpString+1,format);			
						}
						format++;
						if((*format == ':') && (*(format+1) != ']'))
						{
							if((*cmpString >= *(format - 1)) && (*cmpString <= *(format + 1)))
							{
								while(*format != '\0' && *format != ']')
								{
									format++;
								}
								if(*format == ']')
								{
									format++;
								}
								return checkFormat(cmpString+1,format);
							}
							format++;
							format++;

						}
					}while(*format != '\0' && *format != ']');

					return false;
			default  :
					if(*cmpString == *format)
					{
						return checkFormat(cmpString+1,format+1);
					}
					else
					{
						return false;
					}
		}//switch
	}//while(1)
}

int main(int argc,char** argv)
{
	cout<<"********************************************** "<<endl;
	cout<<"*    GuangDong Telecom. Telephone Network    * "<<endl;
	cout<<"*       Centralized Settlement System        * "<<endl;
	cout<<"*                                            * "<<endl;
	cout<<"*            fileInAduit                    * "<<endl;
	cout<<"*              Version 1.0	                * "<<endl;
	cout<<"*     last update time : 2013-08-26 by  hed	   * "<<endl;
	cout<<"********************************************** "<<endl;


	FileInAduit fm ;


	if( !fm.init( argc, argv ) )
	{
		 return -1;
	}
        
	while(1)
	{
		fm.run();		//核对文件
		fm.run(0);		//月汇总文件
		sleep(10);
	}

   return 0;
}


