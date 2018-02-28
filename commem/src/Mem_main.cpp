/*
 * =====================================================================================
 *
 *       Filename:  Mem_main.cpp
 *
 *    Description:  将数据库中的数据表放入共享内存区，并保持和数据库的同步，使其它程序更快的完成数据访问
 *
 *        Version:  1.0
 *        Created:  2010年05月09日 08时59分13秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *        Company:
 *
 *    update list: 
 *    2010-05-09 :  3.0.1 : 初始版本
 *    2010-10-21 :  3.0.2 : 长度优先索引遇到超过地址下标的字符，直接当字符串结束，跳出循环 (CommonMemLenIndex.cpp)
 *    2010-11-22 :  3.0.3 : 由于key值限制，改ftok输入参数
 *		2011-02-18 :	3.0.4 : 长度优先索引不区分大小写，减少占用空间。
 *		2011-05-25 :	3.0.5 : 由于linux问题，公共共享内存区key改成env/commem/0来计算。
 *		2011-12-24 :	3.0.6 : 笔误，commonmemTabledefine查询时里面长度优先索引建成了二叉树
 * =====================================================================================
 */

#include "Mem_main.h"
using namespace std;
using namespace tpss;
//CDatabase DBConn;
CLog theJSLog;
//extern int commenKeyValue;

int main(int argc, char *argv[])
{
	printf("*************************************************** \n");
	printf("*         China Telecom. ZHJS System          * \n");
	printf("*        International Account Settle System       * \n");
	printf("*                                                 * \n");
	printf("*                 jscommem  module                  * \n");
	printf("*                 sys.GJZW.Version 1.0                  * \n");
	printf("*        last updated: 2011-12-24 by sunhua       * \n");
	printf("*                                                 * \n");
	printf("*************************************************** \n");

	if (argc < 2 || argc == 2 and strcmp(argv[1],"-v")==0 ) 
	{
		printf("版本号 3.0.6, 最后更新日期:2011-12-24 by sunhua\n");
		printf("Usage:	%s  -h ask for help. \n",argv[0]);
		return 0;
	}

	if ( strcmp(argv[1],"-h" )==0 )
	{
		printf("版本号 3.0.6, 最后更新日期:2011-12-24 by sunhua\n");
		printf("    -c         :  创建资料共享内存\n");
		printf("    -k         :  删除资料共享内存\n");
		printf("    -rc [line] :  读取资料共享内存公共区信息 [x]为可选参数只读第x行记录\n");
		printf("    -f [mem_name] [queryCondition] [indexID] : \n" ); 
		printf("        根据queryCondition条件,使用indexID索引查询共享内存mem_name中匹配的资料\n");
		return 0;
	}

	char v_envpath[256];
	char iMessage[512];
	char logpath[256];
	char envname[50];
	char errmsg[500]; 
	int  debugFlag;

	//核心参数获取
	std::string log_path,log_level;
	if( !tpss::getKenelParam( "log.path", log_path ) ) {
		tpss::writelog( 0, "获取日志路径失败" );
	}
	if( !tpss::getKenelParam( "log.level", log_level ) ) {
		tpss::writelog( 0, "获取日志级别失败" );
	}
	int level_no = es::StringUtil::toInt( log_level );

	strcpy(logpath,log_path.c_str());
	debugFlag =  level_no;
    //添加envpath
	strcpy(v_envpath,log_path.c_str());   
	//cout << "log_path = " <<log_path<<endl;
	//cout << "level_no = " <<level_no<<endl;
	/*strcpy(v_envpath,getenv("ENVPATH"));
	completeDir(v_envpath);
	string DBPATH=v_envpath;
	DBPATH+="zhjs.ini";

	//从系统环境变量中获取配置文件路径
	
	if ( strlen(v_envpath)<1 )
	{
		printf("从系统环境变量中获取环境变量路径[ENVPATH]出错\n");
		return (-1);
	}
	
	CReadIni v_ini;
	v_ini.init(DBPATH.c_str());
	cout<<DBPATH.c_str()<<endl;
	
	//配置文件中获取日志等级(日志等级看看是从配置文件还是数据库中)
	int nLogLevel=1;
	strcpy( envname,"DEBUG_FLAG" );
	int ret;

	if ( ! v_ini.GetValue("COMMON","log_level",logpath,'Y' ) )
	{
		sprintf(errmsg,"get env log_level from %s err!",DBPATH.c_str());
		printf( "%s\n",errmsg );
		return ( -1 );
	}	
	debugFlag=atoi(logpath);
	
	//配置文件中获取日志路径
	if ( ! v_ini.GetValue("COMMON","LOG_PATH",logpath,'Y') )
	{
		sprintf(errmsg,"get env LOG_PATH from %s err!",DBPATH.c_str());
		printf( "%s\n",errmsg );
		return ( -1 );
	}	*/
	completeDir( logpath );
	
	theJSLog.setLog(logpath, debugFlag, "1", "comem", 1);
	
	theJSLog<<"调试等级="<<debugFlag<<"=,"<<__FILE__<<","<<__LINE__<<endd; 
	theJSLog<<"日志目录路径="<<logpath<<"=,"<<__FILE__<<","<<__LINE__<<endd;
	
	//获取同步数据库时间
	/*long iSleepTime;
	char temp[20];
	if (! v_ini.GetValue("COMMON","UPDATE_DB_TIME",temp,'Y')<0)
	{
		errLog(LEVEL_WARN,"-",10001,"从环境变量配置文件中获取不到更新时间参数[UPDATE_DB_TIME].",__FILE__,__LINE__);
		iSleepTime=60;
	}
	else
	{
		iSleepTime=atol(temp);
	}
	theJSLog<<"轮询数据库时间="<<logpath<<"=,"<<__FILE__<<","<<__LINE__<<endd;*/

	CommonMemManager vCommonMgr;
	if ( strcmp(argv[1],"-c") == 0 )
	{
		int ret=ProcIsExist(argc, argv);
		theJSLog<<"ProcIsExist进程数="<<ret<<"=,"<<__FILE__<<","<<__LINE__<<endd;
		if (ret > 1)
		{
			errLog(LEVEL_FATAL,"-",10006,"共享内存进程已存在，如果要创建，先执行commem -k.",__FILE__,__LINE__);
			return ( -1 );
		}
		
		try
		{
			theJSLog<<"开始设置守护模式"<<endi;
			bool daemonFlag=true;
			/*if (! v_ini.GetValue("COMMON","DAEMON_MODE",temp,'Y')<0)
			{
				errLog(LEVEL_WARN,"-",10001,"从环境变量配置文件中获取不到守护模式参数[DAEMON_MODE].",__FILE__,__LINE__);
				
				daemonFlag=true;
			} else {
				if ( strcmp( temp,"Y" ) == 0 ) daemonFlag=true;
				else daemonFlag=false;
			}*/
			if (daemonFlag) theJSLog<<"定义为守护模式"<<endi;
			initDaemon(daemonFlag);
			
			//theJSLog<<"连接数据库链接"<<endi;
		//	connectDB( (char *)DBPATH.c_str(), DBConn ); 
			
			theJSLog<<"共享内存开始初始化"<<endi;
			vCommonMgr.init(v_envpath);
			
			theJSLog<<"共享内存开始创建公共区"<<endi;
			vCommonMgr.createCommonTable();
			
			theJSLog<<"共享内存开始加载所有资料表"<<endi;
			vCommonMgr.loadAllTable();
			
		//	if (DBConn.IsConnected() == 1) DBConn.Disconnect();
		//	theJSLog<<"断开数据库链接"<<endd;
		}
		catch(CException e)
		{
			errLog(LEVEL_FATAL, "-", 11000,
				"创建共享内存过程中出错", __FILE__, __LINE__, e);
			return(-1);
		}

		//定时检测共享内是否需要更新
		while (1)
		{
			try
			{
				theJSLog.reSetLog();
				theJSLog<<"检查数据库链接"<<endd;
			//	if (DBConn.IsConnected() == -1) connectDB( (char *)DBPATH.c_str(), DBConn ); 
								
				theJSLog<<"查询是否有库表需要整表更新"<<endi;
				vCommonMgr.searchUpdate();
				
			//	if (DBConn.IsConnected() == 1) DBConn.Disconnect();
		//		theJSLog<<"断开数据库链接，休眠"<<endd;
				
				sleep(60);

			}
			catch(CException e)
			{
			errLog(LEVEL_FATAL, "-", 11001,
				"更新共享内存过程中出错", __FILE__, __LINE__, e);
				return(-1);
			}
		}

	} else if ( strcmp(argv[1],"-k") == 0 )
	{
		int ret=ProcIsExist(argc, argv);
		theJSLog<<"ProcIsExist进程数="<<ret<<"=,"<<__FILE__<<","<<__LINE__<<endd;
		if (ret > 1)
		{
			KillProc(ret);
		}
		
		try
		{
		//	theJSLog<<"连接数据库链接"<<endi;
		//	connectDB( (char *)DBPATH.c_str(), DBConn ); 
			
			theJSLog<<"共享内存开始初始化"<<endi;
			vCommonMgr.init(v_envpath);
			
			theJSLog<<"共享内存开始卸载所有资料表"<<endi;
			vCommonMgr.freeAllTable();
			
			theJSLog<<"共享内存开始删除公共区"<<endi;
			vCommonMgr.deleteCommonTable();
			
	//		if (DBConn.IsConnected() == 1) DBConn.Disconnect();
	//		theJSLog<<"断开数据库链接，退出"<<endi;
			
			return 0;
		}
		catch(CException e)
		{
			errLog(LEVEL_FATAL, "-", 11002,
				"删除共享内存过程中出错", __FILE__, __LINE__, e);
				return(-1);
		}
	} else if ( strcmp(argv[1],"-rc" ) == 0 )
	{
		int readLine;
		if ( argc == 2 ) readLine = -1;
		else readLine = atoi( argv[2] );
		
		theJSLog<<"共享内存开始初始化"<<endi;
		vCommonMgr.init(v_envpath);
		theJSLog<<"链接公共共享内存区"<<endi;
		vCommonMgr.AttachCommonTable();
		theJSLog<<"读取公共共享内存区信息："<<endi;
		vCommonMgr.ReadCommonInfo(readLine);
		vCommonMgr.detach();
		return 0;
	} else if ( strcmp(argv[1],"-f") == 0 )
	{
		if ( argc != 5 )
		{
			printf("-f 输入参数不正确，eg:commem -f [mem_name] [queryCondition] [indexID]\n");
			return 0;
		}
		try
		{
		//	theJSLog<<"连接数据库链接"<<endi;
		//	connectDB( (char *)DBPATH.c_str(), DBConn ); 
				
			theJSLog<<"共享内存开始初始化"<<endi;
			vCommonMgr.init(v_envpath);
    	
			theJSLog<<"链接公共共享内存区"<<endi;
			vCommonMgr.AttachCommonTable();
    	
			theJSLog<<"链接数据区["<<argv[2]<<"]共享内存区"<<endi;
			string mem_name ;
			char temp[256];
			strcpy( temp,argv[2] );
			toUpper( temp );
			mem_name = temp;
			vCommonMgr.AttachTable(mem_name);		
			int indexid;
			indexid=atoi(argv[4]);
			vCommonMgr.queryTableRecord(mem_name,argv[3],indexid);		
			vCommonMgr.detach();
			return 0;
		} 
		catch(CException e)
		{
			theJSLog<<"-f 手工查询共享内存过程中出错,错误号:"<<e.GetAppError()<<"错误信息:"<<e.GetErrMessage()<<","<<__FILE__<<","<<__LINE__<<endi;
			return(-1);
		}
		
	}
		
	
	return 0;

}

int ProcIsExist(int argc, char *argv[])
{
	int i;
	int iProc = 0;
	FILE *fp = 0;
	char *lv_chPoint = 0;
	char *pchLoginName = 0;
	char chScanBuff[BUFFER];
	char chCommandLine[BUFFER];
	char chCommandLine1[BUFFER];
	char chTemp[BUFFER];
	memset(chScanBuff, 0, sizeof(chScanBuff));
	memset(chCommandLine, 0, sizeof(chCommandLine));
	memset(chCommandLine1, 0, sizeof(chCommandLine1));
	memset(chTemp, 0, sizeof(chTemp));
	int pid=0;

	if((pchLoginName = getenv("LOGNAME")) == 0)
	{
		errLog(LEVEL_FATAL,"-",10000,"从系统环境变量中获取环境变量路径[LOGNAME]出错.",__FILE__,__LINE__);
		return -1;
	}
	
	if((pid = (int)getpid()) <= 0)
	{
		errLog(LEVEL_FATAL,"-",10000,"getpid函数无法获取进程id.",__FILE__,__LINE__);
		return -1;
	}
	
	lv_chPoint = argv[0];
	for(i = 0; *lv_chPoint; lv_chPoint++, i++)
	{
		if (*lv_chPoint == '/')
		{
			i = -1;
			continue;
		}
		chCommandLine[i] = *lv_chPoint;
	}
	chCommandLine[i] = '\0';

	delSpace(chCommandLine,strlen(chCommandLine));
	
	sprintf(
		chScanBuff,
		"ps -eo user,comm,pid| grep '^ *%s ' | grep '%s' | grep -v %d ",		
		pchLoginName,
		chCommandLine,
		pid
	);

	if((fp = popen(chScanBuff , "r")) == 0)
	{
		return false;
	}
	while(fgets(chScanBuff, sizeof(chScanBuff), fp) != 0)
	{		
		strcpy(chTemp, chScanBuff);
		char Comm[50],Pid[50],User[20];
		sscanf( chTemp,"%s %s %s",User,Comm,Pid );
		delSpace(Comm,strlen(Comm));
		
		lv_chPoint = Comm;
		for(i = 0; *lv_chPoint; lv_chPoint++, i++)
		{
			if (*lv_chPoint == '/')
			{
				i = -1;
				continue;
			}
			chCommandLine1[i] = *lv_chPoint;
		}
		chCommandLine1[i] = '\0';

		delSpace(chCommandLine1,strlen(chCommandLine1));
		strcpy( Comm, chCommandLine1 );
		
		if(memcmp(Comm, chCommandLine,strlen(chCommandLine)) == 0)
		{
			iProc = atoi(Pid);
			if (iProc != getpid())
				break;
      else
				iProc =0;			        
		}
		//memset(chScanBuff, 0, sizeof(chScanBuff));
	}//end while(fgets(chScanBuff, sizeof(chScanBuff), fp) != 0)
	pclose(fp);

	return iProc;

}

void  KillProc(int iProc)
{
	char cmdline[BUFFER];
	memset(cmdline,0,sizeof(cmdline));
	sprintf(cmdline,"kill -9 %d",iProc);
	kill(iProc,SIGTERM);
	sleep(1);

	if ( (getpgid(iProc)) > 0 )
	{
		system(cmdline);
		sleep(1);
		theJSLog<<"成功 KILL -9 杀掉进程="<<iProc<<"="<<endi;
	} else
	{
		theJSLog<<"成功杀掉进程="<<iProc<<"="<<endi;
	}
	
	return;
}
