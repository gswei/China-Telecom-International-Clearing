#include "Poco/NumberParser.h"
#include "Poco/DateTimeFormatter.h"
#include "paramConfigParse.h"
#include "ShellCrontab.h"
#include "Poco/StringTokenizer.h"
#include "Poco/NumberFormatter.h" 
#include <occi.h>
#include <occiControl.h>

//using namespace std;

using namespace oracle::occi;

#ifndef DBSERVER_ENV_NAME
#define DBSERVER_ENV_NAME "SID"
#endif

#ifndef DBUSER_ENV_NAME
#define DBUSER_ENV_NAME "UNAME"
#endif

#ifndef DBPASS_ENV_NAME
#define DBPASS_ENV_NAME "UPASS"
#endif
#ifndef LOG_PATH_NAME
#define LOG_PATH_NAME "LOG_PATH"
#endif

#ifndef LOG_LEVEL_NAME
#define LOG_LEVEL_NAME "LOG_LEVEL"
#endif

int g_latn_id = 0;//地市区号
std::string g_spec_table_id = "-1";
int g_flushtime = 0; //刷新时间
bool g_bool_debug = false;
int g_Fork = 10; //进程数
paramParse  _paramParse ;
string szLogPath;string szLogLevel;

#ifndef ITOA
#define ITOA(A) Poco::NumberFormatter::format(A)
#endif

//定义退出信号
bool p_exit = false;
//定义刷新信号
bool p_flushexit = false;

void sig_exit( int sig_no )
{
	signal( sig_no, SIG_IGN );
	p_exit = true;
}

void sig_flushexit( int sig_no )
{
	signal( sig_no, SIG_IGN );
	p_flushexit = true;
}


std::string FileRunContrab::CalTime(std::string strLastRunTime, int cycle, int cycle_type)
{
	theJSLog<<"strLastRunTime: "<<strLastRunTime<<" , cycle: "<<cycle<<" , cycle_type: "<<cycle_type<<endi;
	std::string strql;


	switch (cycle_type)
	{
	case 0: //分钟
		strql = " select to_char( to_date('"+strLastRunTime+"','YYYYMMDDHH24MISS') + "+ITOA(cycle)+"/24/60, 'YYYYMMDDHH24MISS') from dual";
		break;
	case 1: //小时
		strql = "select to_char( to_date('"+strLastRunTime+"','YYYYMMDDHH24MISS') +"+ITOA(cycle)+" /24, 'YYYYMMDDHH24MISS') from dual ";
		break;
	case	2: //天
		strql = "select to_char( to_date('"+strLastRunTime+"','YYYYMMDDHH24MISS') + "+ITOA(cycle)+", 'YYYYMMDDHH24MISS') from dual "; 
		break;
	case 3://月
		strql = "select to_char( add_months(to_date('"+strLastRunTime+"', 'YYYYMMDDHH24MISS') ,"+ITOA(cycle)+"), 'YYYYMMDDHH24MISS') from dual";
		break;
	case 4://年
		strql = "select to_char( add_months(to_date('"+strLastRunTime+"', 'YYYYMMDDHH24MISS') ,"+ITOA(cycle)+"*12), 'YYYYMMDDHH24MISS') from dual";
		break;
	}

	
	//theJSLog<<strql<<endi;

	DBConnection conn;
	if (conn.isClosed())
	{		conn = DBConnectionFactory::getInstance().createConnection( user.c_str(), pwd.c_str(), serv.c_str() );
		conn.open();	}
		util_1_0::db::Statement stmt = conn.createStatement();
		stmt.setSQLString("alter session set NLS_DATE_FORMAT=YYYYMMDDHH24MISS");
		stmt.execute();
		stmt.setSQLString(strql);
		stmt.execute();
		stmt>>strql;

		stmt.close();
		conn.close();
		return strql;
}
std::string FileRunContrab::CalFistTime(std::string curtime,std::string runtime, int cycle, int cycle_type)
{
	theJSLog<<"runtime :"<<runtime<<" , cycle: "<<cycle<<" , cycle_type: "<<cycle_type<<endi;
	std::string strql;


	switch (cycle_type)
	{
	case 0: //分钟
		strql = "select ceil((to_date('"+curtime+"','YYYYMMDDHH24MISS') - to_date('"+runtime+"','YYYYMMDDHH24MISS'))*24*60/"+ITOA(cycle)+") from dual";
		break;
	case 1: //小时
		strql = "select ceil((to_date('"+curtime+"','YYYYMMDDHH24MISS') - to_date('"+runtime+"','YYYYMMDDHH24MISS'))*24/"+ITOA(cycle)+") from dual ";
		break;
	case	2: //天
		strql = "select ceil((to_date('"+curtime+"','YYYYMMDDHH24MISS') - to_date('"+runtime+"','YYYYMMDDHH24MISS'))/"+ITOA(cycle)+") from dual "; 
		break;
	case 3://月
		strql = "Select ceil(months_between(to_date('"+curtime+"','YYYYMMDDHH24MISS'),to_date('"+runtime+"','yyyymmddhh24miss'))/"+ITOA(cycle)+") From dual";
		break;
	case 4://年
		strql = "Select ceil(months_between(to_date('"+curtime+"','YYYYMMDDHH24MISS'),to_date('"+runtime+"','yyyymmddhh24miss'))/12/"+ITOA(cycle)+") From dual";
		break;
	}


	//theJSLog<<strql<<endi;

	int imultiple;
	DBConnection conn;
		if (conn.isClosed())
		{			conn = DBConnectionFactory::getInstance().createConnection( user.c_str(), pwd.c_str(), serv.c_str() );
			conn.open();		}
		util_1_0::db::Statement stmt = conn.createStatement();
		stmt.setSQLString("alter session set NLS_DATE_FORMAT=YYYYMMDDHH24MISS");
		stmt.execute();
		stmt.setSQLString(strql);
		stmt.execute();
		
		/* 2014-01-03 改为如下
		stmt>>imultiple;  

		strql = CalTime(runtime, imultiple*cycle,cycle_type);
		stmt.close();
		conn.close();
		return strql;
		*/
		stmt>>imultiple;  		//由于ORACLE精度问题,如果相差当前值与运行时间相差太少的话，会忽略为0  		
		if ( (imultiple == 0 ) && (curtime != runtime) )
		{
			imultiple = 1;
		}
		
		stmt.close(); 		
		conn.close(); 
		strql = CalTime(runtime, imultiple*cycle,cycle_type);
		return strql;
}

long GetIntervalSecond(std::string cur_time, std::string last_time)
{
	int year 	= atol((cur_time.substr(0,4)).c_str());
	int month 	= atol((cur_time.substr(4,2)).c_str());
	int day 	= atol((cur_time.substr(6,2)).c_str());
	int hour 	= atol((cur_time.substr(8,2)).c_str());
	int minute  = atol((cur_time.substr(10,2)).c_str());
	int second  = atol((cur_time.substr(12,2)).c_str());
	Poco::DateTime cur_date_time(year,month,day,hour,minute,second);

	year 	= atol((last_time.substr(0,4)).c_str());
	month 	= atol((last_time.substr(4,2)).c_str());
	day 	= atol((last_time.substr(6,2)).c_str());
	hour 	= atol((last_time.substr(8,2)).c_str());
	minute  = atol((last_time.substr(10,2)).c_str());
	second  = atol((last_time.substr(12,2)).c_str());
	Poco::DateTime last_date_time(year,month,day,hour,minute,second);

	Poco::Timespan time_span = cur_date_time - last_date_time;
	return time_span.totalSeconds() ;
}

/*
//util_1_0::db::SQLException
bool CReadIni::init(const char * pszConfigPath)
{
	WRITE_LOG("CReadIni::init Invoke begin!");
	if(access(pszConfigPath,F_OK|R_OK)!=0)
		return false;

	if(IniInfo.size()!=0)
		IniInfo.clear();

	ifstream ifs;
	ifs.open(pszConfigPath);
	if(!ifs)        
		return false;
	std::string strIfs;
	while(getline(ifs, strIfs))
	{   
		if (strIfs.length() == 0)
		{
			continue;
		}
		if(strIfs[0]=='\0' || strIfs[0]=='#' || strIfs[0]==';')
		{
			continue;
		}
		std::string::size_type pos = strIfs.find_first_of('=',0);
		if (pos == std::string::npos)
		{
			continue;
		}
		if (pos == 0)
		{
			continue;
		}
		if (pos >= strIfs.length()-1)
		{
			continue;
		}

		std::string strKey = strIfs.substr(0,pos-1);
		std::string strValue = strIfs.substr(pos+1);

		strKey = Poco::trim(strKey);
		strValue = Poco::trim(strValue);


		IniInfo.insert( std::map<std::string, std::string>::value_type( strKey, strValue ));




	}

	ifs.close();
	return true;

}

bool CReadIni::GetValue(std::string &strValue, std::string strKey)
{
	WRITE_LOG("CReadIni::GetValue invoke begin!");
	std::map<std::string,std::string>::iterator p = IniInfo.find(strKey);
	if (p == IniInfo.end())
	{
		return false;
	}
	strValue = p->second;


	WRITE_LOG("CReadIni::GetValue invoke End!");
	return true;
}
*/

bool FileRunContrab::ReadConfig()
{   
	if (_conn.isClosed())
	{
		_conn.open();
	}

	util_1_0::db::Statement stmt = _conn.createStatement();
	stmt.setSQLString("alter session set NLS_DATE_FORMAT=YYYYMMDDHH24MISS");
	stmt.execute();

	try 
	{


		FileContrab fileContrab;

		//查询全表
		std::string sql;
        //cycle_type 0 分钟 1小时 2天 3月 4年
		if (g_spec_table_id == "-1")
		{
		//	sql="select group_id, cycle,runtime,IS_IMMEDIATE_RUN,cycle_type from  (select group_id, nvl(cycle,0) cycle, TO_CHAR( (nvl(runtime,sysdate)), 'YYYYMMDDHH24MISS') runtime,process_id, decode(NVL(IS_IMMEDIATE_RUN,0),1,1,0) IS_IMMEDIATE_RUN,decode(nvl(cycle_type,0),1,1,2,2,3,3,4,4,0) cycle_type from process_crontab where state='10A' and latn_id in (0,"+ITOA(g_latn_id) + ") )  A  order by A.process_id asc, A.group_id asc, A.runtime asc,A.cycle asc";
			sql="select group_id, cycle,runtime,IS_IMMEDIATE_RUN,cycle_type from  (select group_id, nvl(cycle,0) cycle, TO_CHAR( (nvl(runtime,sysdate)), 'YYYYMMDDHH24MISS') runtime,process_id, decode(NVL(IS_IMMEDIATE_RUN,0),1,1,0) IS_IMMEDIATE_RUN,decode(nvl(cycle_type,0),1,1,2,2,3,3,4,4,0) cycle_type from process_crontab where state='10A' )  A  order by A.process_id asc, A.group_id asc, A.runtime asc,A.cycle asc";
		}
		else
		{
		//	sql="select group_id, cycle,runtime,IS_IMMEDIATE_RUN,cycle_type from  (select group_id, nvl(cycle,0) cycle, TO_CHAR( (nvl(runtime,sysdate)), 'YYYYMMDDHH24MISS') runtime,process_id, decode(NVL(IS_IMMEDIATE_RUN,0),1,1,0) IS_IMMEDIATE_RUN,decode(nvl(cycle_type,0),1,1,2,2,3,3,4,4,0) cycle_type from process_crontab where group_id in("+g_spec_table_id+ ")  and state='10A' and latn_id in (0,"+ITOA(g_latn_id) + ") )  A  order by A.process_id asc, A.group_id asc, A.runtime asc,A.cycle asc";
			sql="select group_id, cycle,runtime,IS_IMMEDIATE_RUN,cycle_type from  (select group_id, nvl(cycle,0) cycle, TO_CHAR( (nvl(runtime,sysdate)), 'YYYYMMDDHH24MISS') runtime,process_id, decode(NVL(IS_IMMEDIATE_RUN,0),1,1,0) IS_IMMEDIATE_RUN,decode(nvl(cycle_type,0),1,1,2,2,3,3,4,4,0) cycle_type from process_crontab where group_id in("+g_spec_table_id+ ")  and state='10A' )  A  order by A.process_id asc, A.group_id asc, A.runtime asc,A.cycle asc";
		}

     //   theJSLog<<"sql:"<<sql<<endi;

		stmt.setSQLString(sql);
		stmt.execute();

		while(stmt >>fileContrab.group_id>>fileContrab.cycle>>fileContrab.runtime>>fileContrab.is_immediate_run>>fileContrab.cycle_type)
		{
			this->vecFileContrab.push_back(fileContrab);
		};

		for (std::vector<FileContrab>::iterator p = this->vecFileContrab.begin();p != this->vecFileContrab.end(); ++p)
		{    
			ContrabID Contrab;
			sql ="SELECT T.group_id,                         "  
				"       T.script_type,                      "  
				"       T.run_script,                       "  
				"       T.spec_order,                       "  
				"       T.remark,                           "  
				"       b.para_vale connectstring,          "  
				"       T.IsRelation   ,                      "  
				"       T.item_id                                  "
				"  FROM PROCESS_crontab_SHELL T,            "  
				"       (select b.PARA_NAME, b.para_vale    "  
				"          from system_para_config b        "  
			//	"         where b.LATN_ID IN (0, "  + ITOA(g_latn_id) +")  and"
				"     where   B.STATE = '10A' ) b         "  
				" WHERE T.STATE = '10A'                     "  
			//	"   AND T.LATN_ID IN (0, "+ ITOA(g_latn_id) +")"  
				"    AND  T.connectstring = B.PARA_NAME(+)    "  
				"   AND T.group_id = " + ITOA(p->group_id) +  
				" ORDER BY SPEC_ORDER ASC, item_id ASC     " ;  


		//	theJSLog<<"sql:"<<sql<<endi;

			stmt.setSQLString(sql);
			stmt.execute();

			while(stmt>>Contrab.group_id>>Contrab.script_type>>Contrab.run_script>>Contrab.spec_order>>Contrab.remark>>Contrab.connectstring>>Contrab.isRelation >>Contrab.item_id)
			{  

			/*	theJSLog<<"Contrab.group_id:"<<Contrab.group_id
					<<" ,Contrab.script_type:"<<Contrab.script_type
					<<" ,Contrab.spec_order:"<<Contrab.spec_order
					<<" ,Contrab.connectstring "<<Contrab.connectstring
					<<" ,Contrab.isRelation:"<<Contrab.isRelation
					<<" ,Contrab.item_id"<<Contrab.item_id<<endi;*/

				if (Contrab.script_type != "SQL" && Contrab.script_type !="SHELL")
				{
					/*logerror(LOG_CODE_AUDIT_CFG_ERR,"script_type只支持SQL类型和SHELL类型");*/
					theJSLog<<"script_type只支持SQL类型和SHELL类型"<<endi;
					_conn.close();
					return false;
				}

				p->vecContrab.push_back(Contrab);

			}


		}


		_conn.close();

		return true;
	} 
	catch (util_1_0::db::SQLException & e) {
		WRITE_LOG(e.what());
		return false;
	}

}




bool FileRunContrab::AllocationForkNum()
{
	WRITE_LOG("FileRunContrab::AllocationForkNum invoke begin!");
	if (vecFileContrab.size() < g_Fork)
	{
		g_Fork = vecFileContrab.size();
	}

	WRITE_LOG("主程序处理数据共"+ITOA(vecFileContrab.size())+",总共有"+ITOA(g_Fork)+"个子程序");

	int i = 0;
	for (std::vector<FileContrab>::iterator p = vecFileContrab.begin();
		p != vecFileContrab.end(); ++p)
	{
		p->i_Fork = ( i++ %g_Fork );
	}

	theJSLog<<"进程号分配如下："<<endi;
	for (std::vector<FileContrab>::iterator p = vecFileContrab.begin();p != vecFileContrab.end(); ++p)
	{
		theJSLog<<"group_id :"<< p->group_id <<",进程号:"<<p->i_Fork<<endi;		
	}

	WRITE_LOG("FileRunContrab::AllocationForkNum invoke End!");
	return  true;
}


bool FileRunContrab::LoopProcess(int process_mod_num)
{
	/*std:: << "子进程: " << process_mod_num << ",子进程号: " << getpid() << ",父进程号: " << getppid() << endl;*/
	std::string strTemp = " 子进程: " + ITOA(process_mod_num) +",子进程号:"+ ITOA( getpid()) + ",父进程号: " + ITOA(getppid()); 
	WRITE_LOG(strTemp);

	signal( SIGINT, sig_exit );
	signal( SIGQUIT, sig_exit );
	signal( SIGTERM, sig_exit );
	signal(12, sig_flushexit);
	//计算下一次运行时间，没有运行，则计算下次的时间
	
	for( std::vector<FileContrab>::iterator iter_sync_info_map = vecFileContrab.begin();
		iter_sync_info_map != vecFileContrab.end(); ++iter_sync_info_map )
	{
		    std::string strBatchTemp = Poco::DateTimeFormatter::format( Poco::LocalDateTime(), "%Y%m%d%H%M%S" );
			std::string  runtime = iter_sync_info_map->runtime;
			int cycle_type = iter_sync_info_map->cycle_type;
			int cycle = iter_sync_info_map->cycle;
			if (strBatchTemp <= runtime)
			{
				iter_sync_info_map->next_process_time = runtime;
			}
			else 
			{	
				std::string strBatchTemp = Poco::DateTimeFormatter::format( Poco::LocalDateTime(), "%Y%m%d%H%M%S" );
				if (cycle == 0)//说明只调度一次，把cycle_type 设置为4(年)，cycle=99,意味间隔99年执行一次
				{
					iter_sync_info_map->next_process_time = CalFistTime(strBatchTemp,runtime, 99, 4);
				}
				else
				{
					iter_sync_info_map->next_process_time = CalFistTime(strBatchTemp,runtime, cycle,cycle_type);
				}
				
			}	

			theJSLog<<"next_process_time: "<<iter_sync_info_map->next_process_time<<"   runtime:"<< iter_sync_info_map->runtime
				<<"  is_immediate_run: "<<iter_sync_info_map->is_immediate_run<<endi;

	}
	
    int i = 0;
	while(true)
	{   
		++i; //首次处理需要is_immediate_run进行判断

		/*获取退出信号*/
		if ( p_exit )
		{
			theJSLog<< "子进程" << getpid() << "收到退出信号，正在做退出前处理..." <<endi;
			for ( int i = 0; i < 3; i++ )
			{
				sleep( 1 ); //休眠3秒，等待子进程返回
			}
			theJSLog<< "子进程" << getpid() << "退出" <<endi;

			exit(0);
		}

		if (p_flushexit)
		{
			theJSLog<< "子进程" << getpid() << "收到刷新退出信号，正在做退出前处理..." <<endi;
			for ( int i = 0; i < 5; i++ )
			{
				sleep( 1 ); //休眠3秒，等待子进程返回
			}
			theJSLog<< "子进程" << getpid() << "退出" <<endi;

			exit(0);
		}


		std::vector<FileContrab>::iterator iter_sync_info_map = vecFileContrab.begin();
		for( ; iter_sync_info_map != vecFileContrab.end(); ++iter_sync_info_map )
		{      
			//不是处理的进程号,continue
			if (iter_sync_info_map->i_Fork != process_mod_num )
			{
				continue;
			}

			std::string strBatchTemp = Poco::DateTimeFormatter::format( Poco::LocalDateTime(), "%Y%m%d%H%M%S" );

			bool bInRun = false;

			//当前时间==定时运行时间
			if(iter_sync_info_map->is_immediate_run == 1 && 1== i)
			{   
				bInRun = true;
			}
			else if (strBatchTemp >= iter_sync_info_map->next_process_time)
			{   
       
				std::string strCalTime = CalTime(iter_sync_info_map->next_process_time, iter_sync_info_map->cycle,iter_sync_info_map->cycle_type);
				iter_sync_info_map->next_process_time = strCalTime;
				bInRun = true;
			}
            

			if (bInRun)
			{
				SyncSingleTable(*iter_sync_info_map);
			}

		}
        
		sleep(1);
		//return true;//测试
	}
	return true;
}


//字符串解密
std::string FileRunContrab::stringDecryption( std::string  str )
{
	if ( strchr( str.c_str(), '/' ) || strchr( str.c_str(), '@' ) )
	{
		/*logerror(LOG_CODE_RC_FIELD_TYPE_ERR, "配置的数据库连接串"+str+"必须是加密后的【可以使用Cipher程序加密】!" );*/
		theJSLog<<"配置的数据库连接串"+str+"必须是加密后的【可以使用Encryptd程序加密】!"<<endi;
		return "";
	}

	CEncryptAsc encrypt;
	char pchDecryptStr[256];
	//memset(pchDecryptStr,0,sizeof(pchDecryptStr));
	encrypt.Decrypt(str.c_str(), pchDecryptStr);
	std::string  strjiemi=std::string(pchDecryptStr);
	/*Cipher cipher;
	std::string strjiemi = cipher.GetPlainText( str.c_str() );*/
	if ( strjiemi.size() == 0 )
	{   
		//huangtodo 错误码似乎不一致
	/*	logerror(LOG_CODE_RC_FIELD_TYPE_ERR, "解密数据库连接串失败!");*/
		theJSLog<<"解密数据库连接串失败!"<<endi;
		return "";
	}

   
	theJSLog<<"解密数据库连接串成功!" <<endi;
	return strjiemi;

	

}


int FileRunContrab::getDbLoginInfo( std::string str, std::string& user, std::string& pwd, std::string& serv )
{
	std::string::size_type a = str.find('/');
	if (a == string::npos)
	{
		theJSLog<<"数据库连接串解析错误！格式应为：用户名/密码@服务名"<<endi;
		return -1;
	}

	std::string::size_type a2 = str.rfind('@');
	if (a2 == string::npos)
	{
		theJSLog<<"数据库连接串解析错误！格式应为：用户名/密码@服务名"<<endi;
		return -1;
	}


	user = str.substr(0,a);
	pwd = str.substr(a+1,a2-a-1);
	serv = str.substr(a2+1, str.length()-a2-1);

//	theJSLog<<"user:"<<user<<endi;
//	theJSLog<<"pwd:"<<pwd<<endi;
//	theJSLog<<"serv:"<<serv<<endi;
	return 1;
}

// 返回值2 脚本运行成功，但结果失败，1 脚本运行成功，结果成功， -1 没有执行

int FileRunContrab::RunSQL(ContrabID & contrabID,std::string &remark)
{   
	theJSLog<<"FileRunContrab::RunSQL invoke begin!"<<endi;
	int bfinish = 2;

	//ResultSet        *pRs   =   NULL;

	try
	{ 
		std::string strjiemi = stringDecryption(contrabID.connectstring);
		if (strjiemi.empty() )
		{  
			remark += contrabID.connectstring+"数据库解密失败";
			return 2;
		}
		std::string  user ;
		std::string  pwd;
		std::string  serv ;

		if ( 1 !=  getDbLoginInfo(strjiemi, user,  pwd,  serv ))
		{
			remark += strjiemi+"数据库连接失败";
			return 2;
		}


		Environment      *pEv   =   NULL;
		Connection       *pConn =   NULL;
		oracle::occi::Statement *pStmt =   NULL;

		pEv   =   Environment::createEnvironment(); 
		pConn =   pEv->createConnection(user, pwd, serv);

		if ( NULL == pConn)
		{
			remark +="创建连接失败";
			return 2;
		}

		int max = 0;
		pStmt = pConn->createStatement(contrabID.run_script.c_str()); 

		pStmt->registerOutParam(1, OCCIINT, sizeof(max));

		pStmt->execute();

		
		max = pStmt->getInt(1);

		theJSLog<<"FileRunContrab::RunSQL result="<<ITOA(max)<<endi;

	
		pConn->terminateStatement(pStmt);
		pEv->terminateConnection(pConn);
		Environment::terminateEnvironment(pEv);


		if (max == 1)
		{
			bfinish = 1;
			remark ="成功";
		}
		else
		{
			bfinish = 2;
			remark ="输出值为"+ITOA(max)+",不是1";
		}
		
	}
	catch(oracle::occi::SQLException ex)
	{   
		theJSLog<<"数据库出错:"<< ex.getMessage()<<endi;	
		remark += std::string(ex.getMessage());
	}
	catch (...)
	{
		/*logerror(LOG_CODE_MPROC_OTHER_ERR, "FileRunContrab::RunSQL 未知异常");*/
		WRITE_LOG("FileRunContrab::RunSQL 未知异常");
		remark += "FileRunContrab::RunSQL 未知异常";
		
	}

	
	theJSLog<<"FileRunContrab::RunSQL invoke End!"<<endi;
	return bfinish;

}

// 返回值2 失败， 1成功
int FileRunContrab::RunShell(ContrabID & contrabID,std::string &remark)
{  
	theJSLog<<"FileRunContrab::RunShell invoke begin!"<<endi;
   try
   {
	//	string cmd = contrabID.run_script ;
	//	cmd.insert( 0, "(" );
	//	cmd.append( ") && echo \"11100101011\"" );
 //  
	//	WRITE_LOG("执行脚本:"+cmd);

	//	char buff[5120];
	//	strcpy( buff, "ABNORMAL" );
	//	string result;

	//	FILE* file;

	//	if ( ( file = popen( cmd.c_str(), "r" ) ) != NULL )
	//	{
	//		while (	fgets( buff, 5120, file ) ) 
	//		{
	//			result += buff;
	//		}

	//		pclose( file );
	//	}

	//	memset(buff,sizeof(buff), 0);
	//	if ( strcmp( buff, "ABNORMAL" ) == 0 )
	//	{
	//		logerror(LOG_CODE_MPROC_OTHER_ERR,"popen出错"+cmd);
	//		return -1;
	//	}

	//	if ( result.find( "11100101011" ) == string::npos ) 
	//	{
	///*	cout << "脚本运行错误,没有运行成功" << endl;*/
	//		logerror(LOG_CODE_MPROC_OTHER_ERR,"脚本运行错误,没有运行成功,脚本："+cmd);
	//		return -1;
	//	}

	//	if (result.find("RESULT==1") ==string::npos)
	//	{
	//		return 2;
	//	}
	//	else
	//	{
	//		return  1;
	//	}


	   std::string m_initScript;

		m_initScript =contrabID.run_script;
			m_initScript.insert( 0, "(\nset -e\n" );

		m_initScript.append( "\n)\nret=$?\nwait\nexit $ret\n" );
	
		theJSLog<<"执行脚本m_initScript:"+m_initScript<<endi;
        
		std::string result;
		char buff[5120] = {0};
		memset(buff,sizeof(buff), 0);

		FILE *file = NULL;

	

		strcpy( buff, "ABNORMAL" );

		if( ( file = popen( m_initScript.c_str(), "r" ) ) != NULL )
		{
			while( fgets( buff, 5120, file ) )
			{
				result += buff;
				/*printf("%s", buff);*/
			}

			int ret = pclose( file );

			if( ret != 0 )
			{
				/*logerror(LOG_CODE_MPROC_OTHER_ERR,"脚本运行错误,没有运行成功,脚本："+m_initScript);*/
				theJSLog<<"脚本运行错误,没有运行成功,脚本："<<m_initScript<<endi;
				remark +="脚本运行错误,没有运行成功,脚本："+m_initScript;
				return 2;
			}

		/*	result = buff;*/
		} 
		else 
		{
			/*logerror(LOG_CODE_MPROC_OTHER_ERR,"脚本运行错误,没有运行成功,脚本："+m_initScript);*/
			theJSLog<<"脚本运行错误,没有运行成功,脚本："<<m_initScript<<endi;
			remark +="脚本运行错误,没有运行成功,脚本："+m_initScript;
			return 2;
			
		}

		
		if(result.find("RESULT==1") !=string::npos)
		{   
			remark ="成功";
			return 1;
		}
		else
		{   
			remark += "找不到RESULT==1";
			return 2;
		}

   }
   catch(...)
   {   
	   remark +="未知异常";
	   return -1;
   }

   	theJSLog<<"FileRunContrab::RunShell invoke End!"<<endi;

}

int  FileRunContrab::ChooseRun(ContrabID & contrabID ,std::string &remark)
{   
	theJSLog<<"FileRunContrab::ChooseRun INVOKE BEGIN!"<<endi;
	int blastRunState = 2;
	//sql语句执行
	if(contrabID.script_type == "SQL")
	{
		blastRunState = RunSQL(contrabID, remark);
	}
	else //Shell
	{
		blastRunState = RunShell(contrabID, remark);
	}

	theJSLog<<"FileRunContrab::ChooseRun INVOKE End!"<<endi;
	return blastRunState ;
}

//不相等 返回真
// 相等 返回假
bool FileRunContrab::operator!=(FileRunContrab &runcontrab ) const 
{

	theJSLog<<"FileRunContrab::operator!= invoke begin!"<<endi;
	std::string strCompare;
	for(std::vector<FileContrab>::const_iterator p = runcontrab.vecFileContrab.begin();
		p != runcontrab.vecFileContrab.end(); ++p )
	{  
	    strCompare += ITOA(p->group_id) + ITOA(p->cycle) + p->runtime;
		for (std::vector<ContrabID>::const_iterator iter = p->vecContrab.begin();
			iter != p->vecContrab.end(); ++iter)
		{
			strCompare += iter->script_type + iter->run_script+ ITOA(iter->spec_order) + iter->connectstring + ITOA(iter->isRelation);
		}
	}


	std::string strCompare2;
	for(std::vector<FileContrab>::const_iterator p = vecFileContrab.begin();
		p != vecFileContrab.end(); ++p )
	{  
		strCompare2 += ITOA(p->group_id) + ITOA(p->cycle) + p->runtime;
		for (std::vector<ContrabID>::const_iterator iter = p->vecContrab.begin();
			iter != p->vecContrab.end(); ++iter)
		{
			strCompare2 += iter->script_type + iter->run_script+ ITOA(iter->spec_order) + iter->connectstring + ITOA(iter->isRelation);
		}
	}

//	theJSLog<<"strCompare2:"<<strCompare2<<endi;
//	theJSLog<<"strCompare:"<<strCompare<<endi;

	if (strCompare2 == strCompare)
	{
		theJSLog<<"FileRunContrab::operator!= invoke End!(strCompare2 == strCompare)"<<endi;
		return false;
	}
	else
	{
		theJSLog<<"FileRunContrab::operator!= invoke End!(strCompare2 != strCompare)"<<endi;
		return true;
	}


	
}




void FileRunContrab::output()
{   

	theJSLog<<"FileRunContrab::output invoke begin!"<<endi;
	std::string strCompare2;
	for(std::vector<FileContrab>::const_iterator p = vecFileContrab.begin();
		p != vecFileContrab.end(); ++p )
	{  
		strCompare2 += ITOA(p->group_id) + ITOA(p->cycle) + p->runtime;
		for (std::vector<ContrabID>::const_iterator iter = p->vecContrab.begin();
			iter != p->vecContrab.end(); ++iter)
		{
			strCompare2 += iter->script_type + iter->run_script+ ITOA(iter->spec_order) + iter->connectstring + ITOA(iter->isRelation);
		}
	}

//	theJSLog<<"FileRunContrab::output："<<strCompare2<<endi;
	theJSLog<<"FileRunContrab::output invoke end!"<<endi;
}

bool FileRunContrab::SyncSingleTable(FileContrab &fileContrab)
{
	theJSLog<<"FileRunContrab::SyncSingleTable invoke begin!"<<endi;
     
	if ( p_exit )
	{
		theJSLog<< "子进程" << getpid() << "收到退出信号，正在做退出前处理..." <<endi;		
		sleep( 1 ); //休眠1秒，等待子进程返回
		theJSLog<< "子进程" << getpid() << "退出" <<endi;

		exit(0);
	}
	

	std::string strBatchTemp = Poco::DateTimeFormatter::format( Poco::LocalDateTime(), "%Y%m%d%H%M%S" );
	theJSLog<<"当前时间为"<<strBatchTemp<<", 主程序的group_id:"<<fileContrab.group_id
		<<", 主程序的指定时间:"<<fileContrab.runtime<<" ,调度时间："<<fileContrab.cycle<<endi;

     
//	theJSLog<<"user:"<<user<<" ,pwd:"<<pwd<<", serv:"<<serv<<endi;
	DBConnection conn = DBConnectionFactory::getInstance().createConnection( user.c_str(), pwd.c_str(), serv.c_str() );
	conn.open();
	util_1_0::db::Statement stmt = conn.createStatement();
	stmt.setSQLString("alter session set NLS_DATE_FORMAT=YYYYMMDDHH24MISS");
	stmt.execute();

	int ilastRunState = -1; // 0 常驻运行, 1,运行成功 2，运行失败，3，没有运行

	std::string sql = "INSERT INTO process_crontab_run_log(group_id,item_id,begindate,enddate,state,remark,create_date )"
		" VALUES(:group_id,:item_id, to_date(:begindate,'YYYYMMDDHH24MISS'), to_date(:Enddate,'YYYYMMDDHH24MISS'), :STATE, :REMARK, SYSDATE )";
	
	stmt.setSQLString(sql);



	for (std::vector<ContrabID>::iterator p = fileContrab.vecContrab.begin();
		p != fileContrab.vecContrab.end(); ++p)
	{   
        strBatchTemp = Poco::DateTimeFormatter::format( Poco::LocalDateTime(), "%Y%m%d%H%M%S" );
		
		theJSLog<<"strBatchTemp:"<<strBatchTemp<<endi;
		//isRelation 说明
		// -1 运行失败
		// 0  常驻运行
		// 1  上一条运行成功，才运行，否则跳过,到下一条
		// 2  上一条运行失败，才运行，否则跳过，到下一条
		// 3  上一条跳过，才运行，否则跳过，到下一条
		std::string remark ;
		if (p->isRelation == 0)
		{
			ilastRunState = ChooseRun(*p,remark);
		}
		else if(p->isRelation == 1)
		{
			if(ilastRunState  ==1 )
			{
				ilastRunState = ChooseRun(*p,remark);	
			}
			else
			{
				ilastRunState = 3;
				remark ="跳过";
			}
		}
		else if(p->isRelation == 2)
		{
			if (ilastRunState ==2)
			{
				ilastRunState = ChooseRun(*p,remark);	
			}
			else
			{
				ilastRunState = 3;
				remark ="跳过";
			}
		}
		else if(p->isRelation == 3)
		{
			if (ilastRunState == 3)
			{
				ilastRunState = ChooseRun(*p,remark);
			}
			else
			{
				ilastRunState = 3;
				remark ="跳过";
			}
		}
		std::string strBatchTempEnd = Poco::DateTimeFormatter::format( Poco::LocalDateTime(), "%Y%m%d%H%M%S" );
        stmt<<p->group_id<<p->item_id<<strBatchTemp<<strBatchTempEnd<<ilastRunState<<remark;
		stmt.execute();
	
//		std::cout<<"插入日志表sql: "<<sql<<endl;


		theJSLog<<"p->group_id: "<<p->group_id<<"  p->spec_order "<<p->spec_order<<" 的状态为 "<<ilastRunState<<endi;

	}

		stmt.commit();
		conn.close();
	

	theJSLog<<"FileRunContrab::SyncSingleTable invoke End!"<<endi;		std::cout<<std::endl;
	return true;
}


void showHelpInfo() 
{
	WRITE_LOG("范例：");
//	WRITE_LOG("psShellCrontab <-lgz> [-t1] [-r0] ");	WRITE_LOG("psShellCrontab <-t1> [-j5] [-f10] ");
//	WRITE_LOG("-l是必选参数,该参数后面紧接的是地市首位拼音,广州的首位拼音即为gz ");
	WRITE_LOG("-j是可选参数,该参数后面紧接的是参数是代表刷新数据的时间");
	/*WRITE_LOG("-d是可选参数,该参数表示是否打印日志,默认不输出");*/
	WRITE_LOG("-f是可选参数,该参数表示是指进程数，当小于运行的条数，则自动取运行的条数");
    WRITE_LOG("-t是必选参数,运行的group_id 例如-t1,3");
	WRITE_LOG("-h 帮助命令");

}


bool checkParam( int argc, char *argv[] ) 
{
	WRITE_LOG("checkParam begin!");
	if( argc == 1 )
	{
		/*logerror(LOG_CODE_APP_PARAM_LACK, "命令行参数个数异常,缺少地市参数-l ");*/
	//	theJSLog<<"命令行参数个数异常,缺少地市参数-l "<<endi;		theJSLog<<"命令行参数个数异常,缺少主程序序号参数-t "<<endi;
		showHelpInfo();
		return false;
	}

	g_bool_debug = false;
	g_Fork = 10;
	g_flushtime = 0;

	for ( int i = 1; i < argc; i++ ) 
	{
		string _argv_i = string( argv[i] );
		// _argv_i.substr( 0, 2 ) == "-l" ||
		if (_argv_i.substr( 0, 2 ) == "-h"
			|| _argv_i.substr(0,2) == "-f" 
			|| _argv_i.substr(0,2) == "-j" 
			|| _argv_i.substr(0,2) == "-t") 
		{
			continue;
		} 
		else if ( _argv_i.substr( 0, 2 ) == "-d")
		{
			g_bool_debug = true;
		}
		else 
		{
			/*logerror(LOG_CODE_APP_PARAM_NONSUPPORT, "参数不正确,不支持:" + _argv_i);*/
			theJSLog<<"参数不正确,不支持:"<<_argv_i<<endi;
			showHelpInfo();
			return false;
		}
	}


	/*initializeLog( argc, argv, g_bool_debug);*/

	for ( int j = 1; j < argc; j++ ) 
	{
		string _argv_i = string( argv[j] );
		/*if ( _argv_i.substr( 0, 2 ) == "-l" ) 
		{
			string argvValue = _argv_i.substr( 2, _argv_i.length() );
			int ctg_corp_id;
			std::string corp_name;
			g_latn_id=_paramParse.getInt("CITY_ID",0);
		}
		else */		if( _argv_i.substr( 0, 2 ) == "-t" ) 
		{   
			g_spec_table_id = "-1";
			string argvValue = _argv_i.substr( 2, _argv_i.length() );
			for (std::string::size_type index = 0; index  !=argvValue.size();++index)
			{
				if ( !isdigit(argvValue[index]) &&  argvValue[index] != ',' )
				{
					showHelpInfo();
					return false;
				}
			}
			g_spec_table_id = argvValue;
		}
		else if( _argv_i.substr( 0, 2 ) == "-f" )
		{
			string ForkNum ;
			ForkNum = _argv_i.substr( 2, _argv_i.length() );
			for (std::string::size_type index = 0; index  !=ForkNum.size();++index)
			{
				if ( ! isdigit(ForkNum[index])  )
				{
					showHelpInfo();
					return false;
				}
			}
			g_Fork = atoi(ForkNum.c_str());
		}
		else if(_argv_i.substr(0,2) == "-j")
		{
			string  strFlushTime = _argv_i.substr( 2, _argv_i.length() );
			for (std::string::size_type index = 0; index  !=strFlushTime.size();++index)
			{
				if ( ! isdigit(strFlushTime[index])  )
				{
					showHelpInfo();
					return false;
				}
			}
			g_flushtime = atoi(strFlushTime.c_str());
		}
	}

	if( g_latn_id < 0 )
	{
	/*	logerror(LOG_CODE_APP_PARAM_LACK, "缺少地市参数-l ");*/
	//	theJSLog<<"缺少地市参数-l "<<endi;		theJSLog<<"缺少主程序序号参数-t "<<endi;
		showHelpInfo();
		return false;
	}

   WRITE_LOG("checkParam End!");
	return true;
};

FileRunContrab::FileRunContrab()
{

}

/*
bool FileRunContrab::Init()
{
	CReadIni ReadIni;
	szEnvFile = _paramParse.expand(szEnvFile);
	if(!ReadIni.init(szEnvFile.c_str()))
	{
		theJSLog<<"配置加载失败"<<endi;
		return  false;
	}

		CEncryptAsc encrypt;



		char pchUserName[50];
		memset(pchUserName,0,sizeof(pchUserName));


		char pchUserPWD[50];
		memset(pchUserPWD,0,sizeof(pchUserPWD));



	    
		std::string strDBname;
		if ( ! ReadIni.GetValue(strDBname, DBSERVER_ENV_NAME))
		{
			return -1;
		}
		 

		//读取数据库用户名
		std::string strUser;
		if(!ReadIni.GetValue(strUser, DBUSER_ENV_NAME)) 
		{
			return -1;
		}
		encrypt.Decrypt(strUser.c_str(), pchUserName);

		//读取数据库用户密码
		std::string strPwd;
		if (!ReadIni.GetValue(strPwd,DBPASS_ENV_NAME))
		{
			return -1;
		}
		encrypt.Decrypt(strPwd.c_str(), pchUserPWD);
		///读取程序的打印日志路径		
		std::string strLog_path;		
		if(!ReadIni.GetValue(strLog_path,LOG_PATH_NAME))		
		{			
			return -1;		
		}		
		szLogPath=strLog_path;		
	//	std::cout<<"szLogPath "<<szLogPath<<endl;		
		//读取程序的打印日志级别		
		std::string strLog_level;		
		if(!ReadIni.GetValue(strLog_level,LOG_LEVEL_NAME))		
		{			
			return -1;		
		}		
		szLogLevel=strLog_level;
	//	std::cout<<"szLogLevel "<<szLogLevel<<endl;
	//	theJSLog<<"pchUserName="<<pchUserName<<",pchUserPWD="<<pchUserPWD<<",strDBname="<<strDBname<<endi;

		_conn = DBConnectionFactory::getInstance().createConnection( pchUserName, pchUserPWD, strDBname.c_str() );
		_conn.open();
	
		user = std::string(pchUserName);
		pwd = std::string(pchUserPWD);
		serv = strDBname;
		return true;

	// tpss::dbConnect(_conn) ;
}
*/

//2013-12-31 将读配置文件改为读核心参数
bool FileRunContrab::Init()
{
	IBC_ParamCfgMng param_cfg;

	if( !param_cfg.bOnInit() )		//核心参数需要自己初始化
	{
		string sErr;
		int nCodeId;
		param_cfg.getError(sErr,nCodeId);
		cerr<<"参数配置接口初始化失败！错误码="<<nCodeId<<", 错误信息="<<sErr<<endl;
		return false;
	}

	// 从核心参数里面读取日志的路径，级别，
	 char sParamName[256];
	 CString sKeyVal;
	 sprintf(sParamName, "log.path");
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		szLogPath = sKeyVal;

	 }
	 else
	 {	
		cout<<"请在核心参数里配置日志的路径"<<endl;
		return false ;
	 }	 
	 sprintf(sParamName, "log.level");
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		szLogLevel = sKeyVal;
	 }
	 else
	 {	
		cout<<"请在核心参数里配置日志的级别"<<endl;
		return false ;
	 }

	 //判断目录是否存在
	 DIR *dirptr = NULL; 
	 if((dirptr=opendir(szLogPath.c_str())) == NULL)
	 {
		cout<<"日志目录["<<szLogPath<<"]打开失败"<<endl;	
		return false ;
	 }else closedir(dirptr);
		
	//theJSLog.setLog(szLogPath.c_str(),atoi(szLogLevel.c_str()),"GJJS","DSQ",001);
	
	//if(!(dbConnect(conn)))
	// {
	//	memset(erro_msg,0,sizeof(erro_msg));
	//	sprintf(erro_msg,"init()  连接数据库失败 connect error");
	//	theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
	//	return  ;
	//}
	
	std::string  pchUserName,pchUserPWD,strDBname;

try{
	//读取核心参数的数据库配置信息

    tpss::getKenelParam( "db.zb.db_server_name", strDBname );
    tpss::getKenelParam( "db.zb.username", pchUserName );
    tpss::getKenelParam( "db.zb.password", pchUserPWD );

    //将密码转为明文
    Cipher cipher;
    pchUserPWD = cipher.GetPlainText( pchUserPWD.c_str() );

	//	std::cout<<"szLogLevel "<<szLogLevel<<endl;
	//	theJSLog<<"pchUserName="<<pchUserName<<",pchUserPWD="<<pchUserPWD<<",strDBname="<<strDBname<<endi;

	_conn = DBConnectionFactory::getInstance().createConnection( pchUserName, pchUserPWD, strDBname.c_str() );
	_conn.open();
	
	user = pchUserName;
	pwd = pchUserPWD;
	serv = strDBname;
	
	theJSLog<<"连接数据库成功..."<<endi;

  }catch( util_1_0::db::SQLException &ex )
   {
       if( ex.getErrorCode() == -1017 ) 
		{
            writelog( LOG_CODE_USER_PASSWORD_ERR, "数据库连接用户名密码错" );
        }else
		{
            writelog( 69101, "建立数据库连接失败: " + std::string( ex.what() ) );
        }
        return false;
    }	

	return true;
}

FileRunContrab::~FileRunContrab()
{
	if (!_conn.isClosed())
	{
		_conn.close();
	}
	
}



int main(int argc,char ** argv)
{
   
    cout<<"********************************************* "<<endl;
	cout<<"*    China Telecom. Telephone Network         "<<endl;
	cout<<"*    InterNational Account Settle System      "<<endl;
	cout<<"*                                             "<<endl;
	cout<<"*           psShellCrontab                    "<<endl;
	cout<<"*           sys.GJZW.Version 1.0	             "<<endl;
	cout<<"*    created time :      2013-11-03 by  hed   "<<endl;
	cout<<"*    last update time :  2013-12-25 by  hed	 "<<endl;
	cout<<"********************************************* "<<endl;
	
	signal( SIGINT, sig_exit );
	signal( SIGQUIT, sig_exit );
	signal( SIGTERM, sig_exit );

	theJSLog<<"设置为守护进程..."<<endi;
	initDaemon(true);		//2014-01-10设置为守护模块

	//_paramParse  = new paramParse();
	_paramParse.init(argc, argv);

	if ( ! checkParam( argc, argv ) ) 
	{
		return -1;
	}
	
	//theJSLog<<"g_flushtime:"<<g_flushtime<<" ,  g_latn_id:"<<g_latn_id<<" ,  g_Fork:"<<g_Fork<<endi;
	theJSLog<<"g_flushtime:"<<g_flushtime<<" ,  g_Fork:"<<g_Fork<<endi;

	int ttt= 0;
	while(true)
	{   
		p_exit = false;
		p_flushexit = false;
		
		theJSLog<<"第"<<(++ttt)<<"次加载配置"<<endi;
		FileRunContrab RunContrab;
		RunContrab.Init();				
		theJSLog.setLog(szLogPath.c_str(),atoi(szLogLevel.c_str()),"ShellCrontab", "GJJS", 001);

		theJSLog<<"日志路径："<<szLogPath<<" 日志级别："<<szLogLevel<<endi;
		if ( ! RunContrab.ReadConfig())
		{
			return -1;
		}
		RunContrab.output();

		RunContrab.AllocationForkNum();

		if(RunContrab.vecFileContrab.size()  ==0)
		{
			WRITE_LOG("没有数据");
			return 1;
		}

		//fork 之前必须先检查是否关闭
		if (! RunContrab._conn.isClosed())
		{
			RunContrab._conn.close();
		}

		pid_t pid_num;
		std::vector<int> vec_pid;
		for( int index = 0; index < g_Fork; ++index )
		{
			pid_num = fork();

			if( pid_num == 0 )
			{
				RunContrab.LoopProcess(index);
				exit( 0);
			} 
			else if( pid_num < 0 )
			{
				/*theJSLog<< "创建子进程失败" <<endi;*/
		/*		logerror(LOG_CODE_PROCESS_CREATE_ERR, "创建子进程失败");*/
				theJSLog<<"创建子进程失败"<<endi;
				exit(-1);
			}
			else
			{
				vec_pid.push_back(pid_num);
			}
		}


		time_t cur= time(NULL);
		//theJSLog<<"cur:"<<cur<<endi;

		while ( true)
		{   
			time_t now = time(NULL);
			/*theJSLog<<"now - cur :"<<(now-cur)<<endi;*/

			if ( ((now - cur) >= (g_flushtime * 60))  && (g_flushtime != 0) )
			{    
				//数据加载完
				cur = now;

				FileRunContrab RunContrabCompare;
				RunContrabCompare.Init();
				RunContrabCompare.ReadConfig();		
		
				
				if (RunContrabCompare != RunContrab)
				{

					WRITE_LOG("重新加载的数据与源数据不符合");

					for(std::vector<int>::iterator iter = vec_pid.begin();
						iter != vec_pid.end(); ++iter)
					{
						int error = kill(*iter, 12);
						if (error < 0)
						{
							WRITE_LOG("向子进程"+ITOA(*iter)+" 发送信号量12 失败");
						}
						else
						{
							WRITE_LOG("向子进程"+ITOA(*iter)+" 发送信号量12 成功");
						}

					}

					p_flushexit = true;
					break;
				}
				
			}
			

			/*获取退出信号*/
			if ( p_exit )
			{   
				theJSLog<< "收到退出信号，正在做退出前处理..." <<endi;
				for ( int i = 0; i < 5; i++ )
				{
					sleep( 1 ); //休眠5秒，等待子进程返回
				}
				theJSLog<< "程序退出" <<endi;

				exit( 0 );
			}
			sleep( 1 ); //休眠1秒
		}
        
		int n2 = 0;
		if (p_flushexit)
		{
			while(n2 < g_Fork)
			{   	
				if ( waitpid( -1, NULL, 0 ) > 0 )
				{
					n2++;
				}

				sleep(1);
			}
		}

		theJSLog<<"重新加载数据了！"<<endi;

	}
}

