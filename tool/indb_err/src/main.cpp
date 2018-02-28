//将系统错误日志入库,出信息点


#include<iostream>
#include <vector>

#include "CF_Common.h"
#include "CF_CLogger.h"
#include "CF_CFscan.h"

#include "CfgParam.h"
#include<dirent.h>
#include <sys/stat.h>  //stat()函数，查询文件信息

using namespace tpss;  //和psutil.h对应
using namespace std;

CLog theJSLog;

bool gbExitSig=false;

int days=-7;
CF_CFscan scan;									//文件扫描接口

struct stat fileInfo;
struct tm now;

char erro_msg[1024],szLogPath[1024],szLogLevel[10],work_path[1024];

//错误文件记录格式
struct RecordFmt
{
	char errTime[14+1];
	char ppid[10];
	char errLevel[2];
	char errCode[20];
	char errMsg[2048];
	
	RecordFmt()
	{
		memset(errTime,0,sizeof(errTime));
		memset(errLevel,0,sizeof(errLevel));
		memset(ppid,0,sizeof(ppid));
		memset(errMsg,0,sizeof(errMsg));
		memset(errCode,0,sizeof(errCode));
	}
};

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
	cout<<"indb_err  配置文件路径    \n"<<endl;
}


//输入参数-s 数据源   -t 日期
int checkArg(int argc,char** argv)
{
	if(argc < 2)
	{
		printV();
		return -1;
	}

	int ret = 0;
	
	return ret;
}

void run()
{
	//vector<string> vfile;
	char stat_date[8+1],tmp_date[8+1],currTime[14+1],errLogTime[14+1];
	char sql[1024],tmp_dir[1024],fileName[1024],m_szFileName[256],tmp[1024];
	
	DBConnection conn;
	Statement stmt;
	DIR *dirptr = NULL; 
	
	try
	{	
		int rett ;
		memset(currTime,0,sizeof(currTime));
		getCurTime(currTime);
		
		memset(stat_date,0,sizeof(stat_date));
		strncpy(stat_date,currTime,8);
		
		for(int i=days;i<=0;i++)
		{
			memset(tmp_date,0,sizeof(tmp_date));	
			addDays(i,stat_date,tmp_date);
			
			memset(tmp_dir,0,sizeof(tmp_dir));
			sprintf(tmp_dir,"%s%s",szLogPath,tmp_date);
			
			if((dirptr=opendir(tmp_dir)) == NULL)
			{
				continue;
			}
			else
			{
				 closedir(dirptr);
				 
				//扫描目录下面的文件,将其挪到工作目录
						
				if(scan.openDir(tmp_dir))
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"打开话单文件目录[%s]失败",tmp_dir);
					theJSLog<<erro_msg<<endw; //打开目录出错
					continue ;
				}		
				
				while(1)		
				{
					memset(fileName,0,sizeof(fileName));
					rett = scan.getFile("err*",fileName);  				
					if(rett == 100)
					{	
						//cout<<tmp_dir<<":目录下没有文件..."<<endl;
						break;
					}
					if(rett == -1)
					{	
						cout<<"获取文件信息失败"<<endl;
						break ;				
					}
					
					char* p = strrchr(fileName,'/');
					memset(m_szFileName,0,sizeof(m_szFileName));
					strcpy(m_szFileName,p+1);	
					
					theJSLog<<"######## scan file "<<fileName<<" ########"<<endi;
					
					//判断文件是否正在被读写
					
					//将其挪到工作目录
					sprintf(tmp,"%s%s",work_path,m_szFileName);
					//cout<<"将其挪到工作目录"<<tmp<<endl;
					
					if(rename(fileName,tmp))
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"文件[%s]移动到[%s]失败: %s",fileName,tmp,strerror(errno));
						theJSLog<<erro_msg<<endw;
						continue;
					}
					
					//vfile.push_back(tmp);
				}
					
				scan.closeDir();
			}
		}
	
		//解析错误文件入库	
		if(scan.openDir(work_path))
		{	
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"打开工作文件目录[%s]失败",work_path);
			theJSLog<<erro_msg<<endw; //打开目录出错
			return; 
		}
	    
		while(1)		
		{
			memset(fileName,0,sizeof(fileName));
			rett = scan.getFile("err*",fileName);  				
			if(rett == 100)
			{	
				break;
			}
			if(rett == -1)
			{	
				cout<<"获取文件信息失败"<<endl;
				break ;				
			}
			
			char* p = strrchr(fileName,'/');
			memset(m_szFileName,0,sizeof(m_szFileName));
			strcpy(m_szFileName,p+1);	
					
			theJSLog<<"######## deal file "<<fileName<<" ########"<<endi;	
			
			if(!(dbConnect(conn)))
			{
				scan.closeDir();
				
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"连接数据库失败 connect error");
				theJSLog<<erro_msg<<endw;		//连接数据库失败
				return  ;
			}
			
			stmt = conn.createStatement();
			
			ifstream errfile(fileName,ios::nocreate);
			if(!(errfile.is_open()))
			{
				conn.close();
				theJSLog<<"文件:"<<fileName<<"读取失败!"<<endw;
				continue;	
			}
			
			string line,tmp_line;
			stat(fileName,&fileInfo);
			
			now =*localtime ( &(fileInfo.st_mtime) ); now.tm_mon++;
			sprintf( errLogTime,"%4u%02u%02u%02u%02u%02u",	now.tm_year+1900, now.tm_mon, now.tm_mday,now.tm_hour, now.tm_min, now.tm_sec);
							
			cout<<"文件最后修改时间:"<<errLogTime<<endl;
			memset(tmp_date,0,sizeof(tmp_date));
			strncpy(tmp_date,errLogTime,8);
				
			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into D_SYSERR_LOG(FILENAME,PROCESS_ID,LOG_CONTENT,LOG_CODE,ERR_TIME,STAT_TIME,LOG_LEVEL) values(:1,:2,:3,:4,:5,:6,:7)");
			stmt.setSQLString(sql);
			
			getCurTime(currTime);
			//文件记录格式 [22:52:52][005988][4-错误] 本次核对文件核对失败,失败个数:(2)[218]
			//             [08:47:18][012580][4-错误] 本次核对文件[NROAM_201412260825_0066.AUD]核对失败,失败个数:(2)[218]
			while(getline(errfile,line))
			{
				//cout<<"文件内容:"<<line<<endl;
				
				RecordFmt rfmt;	
				
				sprintf(rfmt.errTime,"%s%s%s%s",tmp_date,line.substr(1,2),line.substr(4,2),line.substr(7,2));			
				sprintf(rfmt.ppid,"%s",line.substr(11,6));
				sprintf(rfmt.errLevel,"%s",line.substr(19,1));
				
				tmp_line=line.substr(26);
				snprintf(rfmt.errMsg ,sizeof(rfmt.errMsg),"%s",tmp_line.substr(0,tmp_line.find_last_of("[")));  
				
				tmp_line=tmp_line.substr(tmp_line.find_last_of("[")+1);
				sprintf(rfmt.errCode,"%s",tmp_line.substr(0,tmp_line.find("]")));
				
				theJSLog<<"时间:"<<rfmt.errTime<<" 进程ID:"<<rfmt.ppid<<" 日志等级:"<<rfmt.errLevel<<" 错误码:"<<rfmt.errCode<<" 错误日志内容:"<<rfmt.errMsg<<endi;
				stmt<<m_szFileName<<rfmt.ppid<<rfmt.errMsg<<rfmt.errCode<<rfmt.errTime<<currTime<<rfmt.errLevel;
				stmt.execute();
				
				line="";
			}
					
			errfile.close();
			
			if(remove(fileName))
			{
				stmt.rollback();
				
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"文件[%s]删除失败: %s",fileName,strerror(errno));
				theJSLog<<erro_msg<<endw;
			}
			else
			{
				stmt.commit();
			}
			
			stmt.close();
			conn.close();
			
			theJSLog<<"####################### end deal file #############################"<<endi;
		}	
		
		scan.closeDir();
			
	}catch (SQLException e)
	 {
		stmt.rollback();
		stmt.close();
		conn.close();
		
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"数据库出错：%s(%s)",e.what(),sql);
		theJSLog<<erro_msg<<endw;		//发生sql执行异常
		return ;
	 }

	return ;
}

int main(int argc,char** argv)
{
	cout<<"********************************************* "<<endl;
	cout<<"*    China Telecom. Telephone Network         "<<endl;
	cout<<"*    InterNational Account Settle System      "<<endl;
	cout<<"*                                             "<<endl;
	cout<<"*           indb_err					 		 "<<endl;
	cout<<"*           sys.GJZW.Version 1.0	             "<<endl;
	cout<<"*     created time :      2014-12-24 by  hed	 "<<endl;
	cout<<"*     last updaye time :  2014-12-26 by  hed	 "<<endl;
	cout<<"********************************************* "<<endl;
	
	
	if(checkArg(argc,argv))
	{
		return -1;
	}
	
	if((isProcExist("indb_err")-1))		//接口有问题 需-1去除自身
	{
		cout<<"进程已经存在!"<<endl;
		return 1;
	}
	
	initDaemon(true);
		
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

	 char sParamName[256];
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
	
	theJSLog.setLog(szLogPath, atoi(szLogLevel),"INDB_ERR","GJJS", 001);
	
	 completeDir(szLogPath);
	 sprintf(szLogPath,"%serror_log/",szLogPath);
	
	 //判断目录是否存在
	 DIR *dirptr = NULL; 
	 
	 if((dirptr=opendir(szLogPath)) == NULL)
	 {
		cout<<"错误日志目录["<<szLogPath<<"]打开失败"<<endl;	
		return false ;
	 }else closedir(dirptr);
	
	theJSLog<<"错误日志目录:"<<szLogPath<<endi;
	
	memset(work_path,0,sizeof(work_path));
	sprintf(work_path,"%swork_path/",szLogPath);	
	theJSLog<<"工作目录:"<<work_path<<endi;
	
	setSignalPrc();
	
	while(1)
	{
		run();
		sleep(60);
		theJSLog<<"sleep 60s ...."<<endi;
		theJSLog.reSetLog();
	}
	
}






