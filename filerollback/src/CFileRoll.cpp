/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-08-28
File:			 CFileRoll.cpp
Description:	 文件回退模块
History:
<table>
revision	author            date                description
--------    ------            ----                -----------
</table>
**************************************************************************/
#include "CFileRoll.h"
CLog theJSLog;
SGW_RTInfo	rtinfo;

CFileRoll::CFileRoll()
{
	//memset(szrollfiles,0,sizeof(szrollfiles));
	memset(m_filename,0,sizeof(m_filename));
	memset(input_path,0,sizeof(input_path));
	memset(output_path,0,sizeof(output_path));
	memset(erro_path,0,sizeof(erro_path));
	memset(erro_msg,0,sizeof(erro_msg));
}

CFileRoll::~CFileRoll()
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
bool CFileRoll::init(int argc,char** argv)
{

     if(!PS_Process::init(argc,argv))
    {
      return false;
    }
	//PS_Process::setSignalPrc();

	if(!drInit())	
	{
		return false;
	}

	if(!(rtinfo.connect()))
	{
		return false;
	}	
	rtinfo.getDBSysMode(petri_status);
	cout<<"petri status:"<<petri_status<<endl;
	
	theJSLog.setLog(szLogPath, szLogLevel,"FILEROLLBACK", "GJJS", 001);
	theJSLog<<"日志路径："<<szLogPath<<" 日志级别："<<szLogLevel<<endi;

	bool bb = initializeLog(argc,argv,false);  //是否调试模式
	if(!bb)
	{
			//cout<<"初始化内存日志接口失败"<<endl;
			return false;
	}

	try
	{
		if(!(dbConnect(conn)))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"init() 连接数据库失败 connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
			return  false;
		}
		Statement stmt = conn.createStatement();
		string sql = "select VARVALUE from C_GLOBAL_ENV where VARNAME = 'ROLLFILE_PATH'";
		stmt.setSQLString(sql);
		stmt.execute();//执行sql语句
		stmt >> input_path;//获取结果
		completeDir(input_path);
		theJSLog <<"读取文件路径为"<<input_path<<endi;

		sql = "select VARVALUE from C_GLOBAL_ENV where VARNAME = 'ROLLFILE_BAK_PATH' ";
		stmt.setSQLString(sql);
		stmt.execute();//执行sql语句
		stmt >> output_path;//获取结果
		completeDir(output_path);
		theJSLog <<"备份路径为"<<output_path<<endi;

		sql = "select VARVALUE from C_GLOBAL_ENV where VARNAME = 'ROLLFILE_ERRO_PATH' ";
		stmt.setSQLString(sql);
		stmt.execute();//执行sql语句
		stmt >> erro_path;//获取结果
		completeDir(erro_path);
		theJSLog <<"错误文件路径为"<<erro_path<<endi;

		if(LoadSourceCfg() == -1)	    //加载数据源配置信息 2013-08-22
		{
			return false;
		}

		stmt.close();
		conn.close();
	}
	catch(SQLException  e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init 初始化时数据库查询异常:%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
		return false ;
	}
   
     theJSLog<<"初始化完毕"<<endi;
     return true ;
}

bool CFileRoll::getFilenames()
{	
   while(1)
   	{
	int ret=0,i;
	bool flag;
	char szBuff[1024];
	if(gbExitSig)
	{
		theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "应用程序收到退出信号");
		prcExit();
		return false;
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

	if(drStatus==1)  //备系统
	{
		//检查trigger触发文件是否存在
		if(!CheckTriggerFile())
		{
			sleep(1);
			return 0;
		}

		//获取同步变量
		memset(m_SerialString,0,sizeof(m_SerialString));
		ret=drVarGetSet(m_SerialString);
		char str[256];
		memset(str,0,sizeof(str));
		strcpy(str,m_SerialString);
		//cout << "m_SerialString = " <<m_SerialString <<"str = "<<str<<endl;
		//vector<string> data;	
		if(ret)
		{
			theJSLog<<"同步失败..."<<endw;
			return 0;
		}
				
		vector<string> data;		
		//splitString(m_SerialString,";",data,false,false);
		splitString(str,";",data,false,false);
				
		memset(m_filename,0,sizeof(m_filename));
		strcpy(m_filename,input_path);
		strcat(m_filename,data[0].c_str());
		
		char filename_tmp[256];
		char Filename[256];
        memset(Filename,0,sizeof(Filename));
		char* p = strrchr(m_filename,'/');
		if(p)
			strcpy(Filename,p+1);
		else
			strcpy(Filename,m_filename);
			
		int orglength=strlen(Filename); 
		strncpy(filename_tmp,Filename,orglength-5); //去掉后面5位字符
        //cout << "filename_tmp = " <<filename_tmp<<endl;
		//int dr_GetAuditMode()1表示同步，2表示跟随, 其它为失败，-1是配置错误，-2配置文件读取出现问题
		int iStatus = dr_GetAuditMode(module_name);
		if(iStatus == 1)		//同步模式,	主系统等待指定时间 
		{
			bool flag=false;
			int times=1;
			while(times<31)
			{
				if(access(m_filename,F_OK|R_OK))
				{
					theJSLog<<"查找了"<<times<<"次文件"<<endd;
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
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"主系统传过来的文件[%s]不存在",m_filename);
				theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);

				dr_AbortIDX();
				return 0;
			}
		}
		else if(iStatus==2) //跟随模式，备系统
		{
			while(1)
			{
			//设置中断
				if(gbExitSig)
				{
					dr_AbortIDX();
							
					theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "应用程序收到退出信号");
					prcExit();
					return ;
				}

				if(access(m_filename,F_OK|R_OK))
				{
					sleep(10);
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			char tmp[50]={0};
			snprintf(tmp,sizeof(tmp),"容灾平台dr_GetAuditMode函数配置错误，返回值[%d]",iStatus);
			theJSLog<<tmp<<endi;
			return 0;
		}
		
		int size1,size2;
		struct stat info;
		stat(m_filename,&info);
		size1=info.st_size;

		size2=atoi(data[3].c_str());
		/*if(size1!=size2)
		{
			theJSLog<<"同步失败....,传送文件中的文件大小不同"<<endi;
			dr_AbortIDX();
			return 0;
		}*/

		ifstream in(m_filename,ios::in);
		if(!in)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"getFilenames 文件%s打开出错",m_filename);
			theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败
			return false;
		}
		memset(szBuff,0,sizeof(szBuff));
		szrollfiles.clear();
		SRollFile szrollfile;
		strcpy(szrollfile.filename,filename_tmp);
		szrollfiles.push_back(szrollfile);
		//memset(szBuff,0,sizeof(szBuff));
			
		/*while(in.getline(szBuff,sizeof(szBuff)))   
		{
			//cout<<"读取到文件名为"<<szBuff<<endl;
			SRollFile szrollfile;
			strcpy(szrollfile.filename,szBuff);
			szrollfiles.push_back(szrollfile);
			memset(szBuff,0,sizeof(szBuff));
		}*/

		char m_Filename[256];
		memset(m_Filename,0,sizeof(m_Filename));
		strcpy(m_Filename,data[1].c_str());//主系统传送过来的文件列表名

		//根据文件名查询对应数据源ID，fileid和对应结果表名,从结果表中将fileid对应的结果删除。       
				
		//int j=atoi(data[2].c_str())-0; //主系统传送过来的文件列表名序号

		/*if(strcmp(szrollfiles[j].filename,m_Filename))
		{
			theJSLog<<"同步失败....,传送文件中的文件列表名不同"<<endi;
			dr_AbortIDX();
			return 0;
		}*/

		for(i=0;i<szrollfiles.size();i++)
		{
			/*memset(m_Filename,0,sizeof(m_Filename));
			char* p = strrchr(m_filename,'/');
			if(p)
				strcpy(m_Filename,p+1);
			else
				strcpy(m_Filename,m_filename);*/

			//theJSLog<<"处理文件 "<<m_Filename<<"中的第["<<i+1<<"]个文件"<<endi;
			theJSLog<<"处理文件 "<<filename_tmp<<endi;
			
			//根据文件名查询对应数据源ID，fileid和对应结果表名,从结果表中将fileid对应的结果删除。
			flag=rollFile(i);
			if(!flag)
				break;
			theJSLog<<"处理完文件 "<<filename_tmp<<endi;
		}	

		//theJSLog <<"准备备份文件"<<endi;
		ret=moveFiles(flag);	
		if(ret)
		{
			theJSLog<<"备份文件成功"<<endi;
			//continue;
		 	return true;
		}
		else
		{
			theJSLog<<"备份文件失败"<<endi;
			//continue;
			return false;
		}

	}
	else      //主系统,非容灾系统    
	{
		try
		{	
			if(scan.openDir(input_path))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"打开文件目录[%s]失败",input_path);
				theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
				return false;	
			}	
	//		theJSLog <<"扫描到文件路径"<<input_path<<endi;
			int rett = 0;
			while(1)		
			{
				memset(m_filename,0,sizeof(m_filename));
				rett = scan.getFile("*.UNDO",m_filename); 
				if(rett == 100)
				{		
						break ;
				}
				if(rett == -1)
				{	
						break ;			//表示获取文件信息失败
				}
				theJSLog<<"扫描到文件："<<m_filename<<endi;    //文件中保存的内容是一些文件名 
				//ret=getFilenames();				

				ifstream in(m_filename,ios::in);
				if(!in)
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"getFilenames 文件%s打开出错",m_filename);
					theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败
					return false;
				}

				//获取文件大小
				struct stat info;
				stat(m_filename,&info);
				int size=info.st_size;

				memset(szBuff,0,sizeof(szBuff));
				szrollfiles.clear();
				char m_Filename[256];
				char realfilename[256];
				char* p = strrchr(m_filename,'/');
				memset(m_Filename,0,sizeof(m_Filename));  //m_Filename 保存的是外层不带路径的文件名
					if(p)
						strcpy(m_Filename,p+1);
					else
						strcpy(m_Filename,m_filename);
				int orglength=strlen(m_Filename);
				strncpy(realfilename,m_Filename,orglength-5);
				//cout << "m_Filename = " << m_Filename << "realfilename = " << realfilename <<endl;
				/*while(in.getline(szBuff,sizeof(szBuff)))   
				{
					//cout<<"读取到文件名为"<<szBuff<<endl;
					SRollFile szrollfile;
					strcpy(szrollfile.filename,szBuff);
					szrollfiles.push_back(szrollfile);
					memset(szBuff,0,sizeof(szBuff));
				} */
			   SRollFile szrollfile;
			   strcpy(szrollfile.filename,realfilename);
				szrollfiles.push_back(szrollfile);
							
				//处理每个文件中的所有文件列表
				for(i=0;i<szrollfiles.size();i++)
				 {			
					//memset(m_Filename,0,sizeof(m_Filename));  //m_Filename 保存的是外层不带路径的文件名
					/*char* p = strrchr(m_filename,'/');
					if(p)
						strcpy(m_Filename,p+1);
					else
						strcpy(m_Filename,m_filename);*/

					//theJSLog<<"处理文件 "<<m_Filename<<"中的第["<<i+1<<"]个文件"<<endi;

					memset(m_SerialString,0,sizeof(m_SerialString));
					//sprintf(m_SerialString,"%s;%s;%d;%d",m_Filename,szrollfiles[i].filename,i,size);
					sprintf(m_SerialString,"%s;%d",m_Filename,size);  //传送原始文件名，带UNDO
                    //cout<<"m_SerialString"<<m_SerialString<<endl;
					ret = drVarGetSet(m_SerialString);
					if(ret)
					{
						theJSLog<<"同步失败...."<<endi;
						break;
					} else {
				    //根据文件名查询对应数据源ID，fileid和对应结果表名,从结果表中将fileid对应的结果删除。
					  flag=rollFile(i);
					  if(!flag)
					  {
						theJSLog<<"文件"<<szrollfiles[i].filename<<"回退失败"<<endi;
					   	break;
					  }
					}
					//theJSLog<<"处理完文件 "<<m_Filename<<"中的第["<<i+1<<"]个文件"<<endi;					
				}

				//2013-10-18 add
				/*memset(m_Filename,0,sizeof(m_Filename));  //m_Filename 保存的是外层不带路径的文件名
				char* p = strrchr(m_filename,'/');
				if(p)
				strcpy(m_Filename,p+1);
					else
				strcpy(m_Filename,m_filename);
					
				char realfilename[256];
				int orglength=strlen(m_Filename);
				strncpy(realfilename,m_Filename,orglength-5);
				//cout<<"m_Filename = "<< m_Filename<< "   realfilename = " <<realfilename<<endl;
				sprintf(m_SerialString,"%s;%d",m_Filename,size);
				ret = drVarGetSet(m_SerialString);				
				if(ret)
				{
					theJSLog<<"同步失败...."<<endi;
					break;
				}*/	
				
				in.close();

				//theJSLog <<"准备备份文件"<<endi;
				 ret=moveFiles(flag);	
				 if(ret)
				 {
					 theJSLog<<"备份文件"<<m_Filename<<"成功"<<endi;
				 }
				 else
				 {
					 theJSLog<<"备份文件"<<m_Filename<<"失败"<<endi;
				 }
				 break;
			}
			scan.closeDir();
		}catch (jsexcp::CException &e) 
		{	
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"openDir %s",e.GetErrMessage());
			theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);	
 		}
	  }
	
   	}
}
	
//根据szrollfiles的文件名，从文件接收配置表中查询文件对应的数据源ID，从格式化文件登记表D_SCH_FORMAT中
//查询fileid，根据数据源ID查询汇总配置表获取对应日汇总结果表名。最后从结果表中删除fileid对应的记录
bool CFileRoll::rollFile(int i) 
{
	char state='F';   //文件中sql执行成功标志
//	bool ret=true;  //默认仲裁成功
	Statement stmt;
	if(!(rtinfo.connect()))		//连接内存区
	{
		return false;
	}
	short petri_status ;	
	cout<<"petri status:"<< petri_status <<endl;
	while(1)
	{
	    rtinfo.getDBSysMode(petri_status);		//获取状态
		if(petri_status==DB_STATUS_ONLINE)
		{
		//	theJSLog<<"数据库为正常态"<<endi;
			break;
		}
		else  if(petri_status==DB_STATUS_OFFLINE)
		{
			theJSLog<<"数据库为只读,等待..."<<endi;
			sleep(5);
		}
		else
		{
			theJSLog<<"数据库状态错误"<<endi;
			return false;
		}
	}
	try{			
		if (dbConnect(conn))
		{
			//查询文件所属数据源ID
			int flag=0;
			for(map<string,SOURCECFG>::const_iterator it = m_SourceCfg.begin();it != m_SourceCfg.end();++it)
			{
				if(checkFormat(szrollfiles[i].filename,it->second.filterRule))		//HDC.2013---    HD*
				{		
					theJSLog<<"文件["<<szrollfiles[i].filename<<"]所属数据源:"<<it->first<<endi;

					flag = 1;
					strcpy(szrollfiles[i].sourceID ,(it->first).c_str());
				}				
			}
	
			if(flag == 0)
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"找不到文件%s所属数据源",szrollfiles[i].filename);
				theJSLog.writeLog(LOG_CODE_PARAM_INFO_LACK,erro_msg);
				return 0;
			}

			setSQLFileName(szrollfiles[i].filename) ;

			theJSLog<<"找到文件所属数据源ID 为"<<szrollfiles[i].sourceID<<endi;
			//据文件名查询对应数据源ID
		/*	string sql = "select SOURCE_ID from C_FILE_RECEIVE_ENV where FILE_FILTER = :v1";
			stmt1.setSQLString(sql);
			stmt1 <<  szrollfiles->filename;	//输入参数		
			stmt1.execute();//执行sql语
			stmt1>> szrollfiles->sourceID;	//输出参数		
			stmt1.close();*/

			//通过数据源ID去对应日汇总配置表中查找需要删除的结果表名
			stmt = conn.createStatement();
			string sql="select a.TABLE_NAME from C_STAT_TABLE_DEFINE a,C_SUMTABLE_DEFINE b where a.CONFIG_ID=b.ORGSUMT and b.SOURCEID =:v2";
			stmt.setSQLString(sql);
			stmt <<szrollfiles[i].sourceID;	//输入参数		
			stmt.execute();//执行sql语句
			stmt>> szrollfiles[i].tablename;	//输出参数		
			theJSLog<<"查找结果表: "<<szrollfiles[i].tablename<<endi;
			
			//从格式化文件登记表D_SCH_FORMAT中查询fileid
			sql="select FILE_ID from D_SCH_FORMAT where FILENAME=:v1 and DEAL_TIME =(select max(DEAL_TIME) from D_SCH_FORMAT where FILENAME=:v2)";
			stmt.setSQLString(sql);
			stmt << szrollfiles[i].filename<< szrollfiles[i].filename;	//输入参数		
			stmt.execute();//执行sql语
			int num = stmt.getCompleteRows();
            if(num ==0)
            	{
            	   theJSLog << "此文件在格式表中无记录" << endw;
            	   return false;
                }
            else
            	{
            	  stmt>> szrollfiles[i].fileID;	//输出参数
            	  char sql2[1024];
			sprintf(sql2,"delete from %s where FILEID=%d",szrollfiles[i].tablename,szrollfiles[i].fileID);
			theJSLog<<sql2<<endi;
			stmt.setSQLString(sql2);		
			stmt.execute();//执行sql语
			theJSLog<<"删除fileid 对应的结果表中的数据成功"<<endi;	

		    //执行仲裁
			memset(m_AuditMsg,0,sizeof(m_AuditMsg));
			sprintf(m_AuditMsg,"%s|%c",szrollfiles[i].filename,state);
			theJSLog<<"开始仲裁"<<endi;
			if(!IsAuditSuccess(m_AuditMsg))   //仲裁失败
			{
				theJSLog<<"仲裁失败"<<endi;
				stmt.rollback();
				stmt.close();
				conn.close();
				return false;
			}
			stmt.close();	
			conn.close();
			theJSLog<<"仲裁成功,保存到D_SQL_FILEREG表中"<<endi;
			//每处理一个文件都保存到D_SQL_FILEREG表中
			saveLog(state,i); 
			return true;
            	  
            	}				
		
			
		}else{
	 		memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"init() 连接数据库失败 connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
			return  false;
		}
	 } catch( SQLException e ) {
		state='E';
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"从结果表中将fileid对应的结果删除失败:%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		//执行仲裁
			memset(m_AuditMsg,0,sizeof(m_AuditMsg));
			sprintf(m_AuditMsg,"%s|%c",szrollfiles[i].filename,state);
			theJSLog<<"开始仲裁"<<endi;
			if(!IsAuditSuccess(m_AuditMsg))   //仲裁失败
			{
				theJSLog<<"仲裁失败"<<endi;
			//	ret=false;
				stmt.rollback();
				stmt.close();
				conn.close();
				return false;
			}
			stmt.close();	
			conn.close();
			theJSLog<<"仲裁成功,保存到D_SQL_FILEREG表中"<<endi;
			//每处理一个文件都保存到D_SQL_FILEREG表中
			saveLog(state,i); 
			
		//	rollBackSQL();
			return false;
     } 	
}

void CFileRoll::saveLog(char state,int i)  //每处理一个文件都保存到D_ROLL_FILEREG表中
{
	char sql[1024];
	try{			
		if (dbConnect(conn))
		 {
			Statement stmt = conn.createStatement();
			char flag;
			if(state=='F')
				flag='Y';
			if(state=='E')
				flag='N';
	//		theJSLog<<"szrollfiles.filename="<<szrollfiles[i].filename<<"  szrollfiles.sourceID= "<<szrollfiles[i].sourceID <<"    flag="<<flag<<endi;
			
			/*把文件名的路径去掉*/
			char m_Filename[256];
			memset(m_Filename,0,sizeof(m_Filename));
			char* p = strrchr(m_filename,'/');
			if(p)
				strcpy(m_Filename,p+1);
			else
				strcpy(m_Filename,m_filename);
			sprintf(sql,"insert into D_ROLL_FILEREG(ROLLFILE_NAME,FILE_NAME,DEAL_DATE,SOURCE_ID,DEAL_FLAG) values('%s','%s',sysdate,'%s','%c')",m_Filename,szrollfiles[i].filename,szrollfiles[i].sourceID,flag);
		//	writeSQL(sql);
			theJSLog<<sql<<endd;
			stmt.setSQLString(sql);
			stmt.execute();//执行sql语句
			stmt.close();
			theJSLog<<"D_ROLL_FILEREG表插入数据成功"<<endi;
		 }else{
	 		cout<<"connect error."<<endl;
	 		return ;
		 }
	 	conn.close();
		conn.commit();
	}catch( SQLException e ) {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"处理文件的sql语句 失败,确保 D_ROLL_FILEREG 表创建成功:%s,%s",sql,e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
		conn.close();
    } 			
}

bool CFileRoll::moveFiles(bool flag)//将已经处理后的文件移动到指定备份目录	
{
			
			theJSLog <<"备份文件全路径名为:"<<m_filename<<endi;
			
			/*把文件名的路径去掉*/
			char m_Filename[256],path[256];;
			memset(m_Filename,0,sizeof(m_Filename));
			char* p = strrchr(m_filename,'/');
			if(p)
				strcpy(m_Filename,p+1);
			else
				strcpy(m_Filename,m_filename);
			if (flag)
			{
				theJSLog<<"移动文件 "<<m_filename<<" 到备份目录 "<<output_path<<endi;
				memset(path,0,sizeof(path));
				strcpy(path,output_path);
				strcat(path,m_Filename);
				if(rename(m_filename,path))	
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"移动文件[%s]到备份目录失败: %s",m_filename,strerror(errno));
					theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
					return false;
				}	
			}
			else
			{
				theJSLog<<"移动文件 "<<m_filename<<" 到错误目录 "<<erro_path<<endi;
				memset(path,0,sizeof(path));
				strcpy(path,erro_path);
				strcat(path,m_Filename);
				if(rename(m_filename,path))	
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"移动文件[%s]到错误目录失败: %s",m_filename,strerror(errno));
					theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
					return false;
				}
			}					
			return true;
}

//加载数据源配置信息，取全部数据源组的数据源信息
int CFileRoll::LoadSourceCfg()
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
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"LoadSourceCfg数据库出错：%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		return -1;
	 }

	return 0;
}

/******根据数据源获取过滤规则 0没有查到规则，1查到规则了 增加获取文件名上时间的起始位置,和长度*********************/
int CFileRoll::getSourceFilter(char* source,char* filter,int &index,int &length)
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
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"数据源[%s]没有配置过滤规则或者文件名时间截取规则",source);
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return -1;
		}
		
		//cout<<"file_time = "<<file_time<<endl;
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
		sprintf(erro_msg,"getSourceFilter 数据库查询异常: %s",e.what());
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

bool CFileRoll::checkFormat(const char *cmpString,const char *format)
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

//2013-11-02 新增退出函数
void CFileRoll::prcExit()
{
	int ret = 0;
	if(m_enable) 
	{
		ret = dr_ReleasePlatform();
		if(ret != 0)
		{
			char tmp[100] = {0};
			snprintf(tmp, sizeof(tmp), "释放容灾平台失败,返回值=%d", ret);
			theJSLog<<tmp<<endw;
		}
		
		m_enable = false;
	}
	
	PS_Process::prcExit();
}

//容灾初始化
bool CFileRoll::drInit()
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
int CFileRoll::drVarGetSet(char* serialString)
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
		
		//主系统传递文件所在路径和文件名 只有容灾平台可以感知,备系统无法识别
	/*	if(drStatus != 1)
		{
			snprintf(tmpVar, sizeof(tmpVar), "%s",input_path);
			ret = dr_SyncIdxVar("@@CHECKPATH", tmpVar,SYNC_SINGLE);  
			if(ret != 0)
			{
				theJSLog<<"传输文件所在路径失败,文件路径["<<input_path<<"]"<<endi;
				dr_AbortIDX();
				return -1;
			}
			vector<string> data;		
			splitString(serialString,";",data,false,false);			
			char m_Filename[256];
			memset(m_Filename,0,sizeof(m_Filename));
			strcpy(m_Filename,data[0].c_str());
			snprintf(tmpVar, sizeof(tmpVar), "%s", m_Filename);
			ret = dr_SyncIdxVar("@@CHECKFILE", tmpVar,SYNC_SINGLE);  
			if(ret != 0)
			{
				theJSLog<<"传输文件失败,文件名["<<tmpVar<<"]"<<endi;
				dr_AbortIDX();
				return -1;
			}

			theJSLog<<"传输文件路径:"<<input_path<<" 文件名:"<<tmpVar<<endl;
		}*/


		snprintf(tmpVar, sizeof(tmpVar), "%s", serialString);
		//主系统把要同步的index “键值对”写入容灾平台维护的index 文件中
		//备系统调用该函数的结果是，var获得和主系统一样的随机变量的值。	SYNC_SINGLE表示注册单一的随机变量
		ret = dr_SyncIdxVar("serialString", tmpVar, SYNC_SINGLE);		
		if (ret != 0)
		{
			theJSLog<<"传序列串时失败,序列名:["<<serialString<<"]"<<endi;
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
 bool CFileRoll::IsAuditSuccess(const char* dealresult)
 {
		int auitStatus = 0, ret = 0;
//cout<<"开始仲裁"<<endl;
		ret = dr_Audit(dealresult);
//cout<<"已经仲裁"<<endl;
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
		else if(4 == ret)
		{
			theJSLog<<"对端idx异常终止..."<<endi;
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
			theJSLog<<"仲裁成功...\n仲裁内容："<<dealresult<<endi;
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

bool CFileRoll::CheckTriggerFile()
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

int main(int argc,char**argv)
{
	cout<<"********************************************** "<<endl;
	cout<<"*    China Telecom. Telephone Network          "<<endl;
	cout<<"*    InterNational Account Settle System       "<<endl;
	cout<<"*                                              "<<endl;
	cout<<"*           jsrollfile						  "<<endl;
	cout<<"*           sys.GJZW.Version 1.0	              "<<endl;
	cout<<"*     last update time : 2013-08-28 by  cwf	  "<<endl;
	cout<<"********************************************** "<<endl;


	CFileRoll fm ;
   	if( !fm.init( argc, argv ) )
	{
		 return -1;
	}
	while(1)
	{
		theJSLog.reSetLog();
		fm.getFilenames();
		sleep(30);
	}

   return 0;
}
