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

int g_latn_id = 0;//��������
std::string g_spec_table_id = "-1";
int g_flushtime = 0; //ˢ��ʱ��
bool g_bool_debug = false;
int g_Fork = 10; //������
paramParse  _paramParse ;
string szLogPath;string szLogLevel;

#ifndef ITOA
#define ITOA(A) Poco::NumberFormatter::format(A)
#endif

//�����˳��ź�
bool p_exit = false;
//����ˢ���ź�
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
	case 0: //����
		strql = " select to_char( to_date('"+strLastRunTime+"','YYYYMMDDHH24MISS') + "+ITOA(cycle)+"/24/60, 'YYYYMMDDHH24MISS') from dual";
		break;
	case 1: //Сʱ
		strql = "select to_char( to_date('"+strLastRunTime+"','YYYYMMDDHH24MISS') +"+ITOA(cycle)+" /24, 'YYYYMMDDHH24MISS') from dual ";
		break;
	case	2: //��
		strql = "select to_char( to_date('"+strLastRunTime+"','YYYYMMDDHH24MISS') + "+ITOA(cycle)+", 'YYYYMMDDHH24MISS') from dual "; 
		break;
	case 3://��
		strql = "select to_char( add_months(to_date('"+strLastRunTime+"', 'YYYYMMDDHH24MISS') ,"+ITOA(cycle)+"), 'YYYYMMDDHH24MISS') from dual";
		break;
	case 4://��
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
	case 0: //����
		strql = "select ceil((to_date('"+curtime+"','YYYYMMDDHH24MISS') - to_date('"+runtime+"','YYYYMMDDHH24MISS'))*24*60/"+ITOA(cycle)+") from dual";
		break;
	case 1: //Сʱ
		strql = "select ceil((to_date('"+curtime+"','YYYYMMDDHH24MISS') - to_date('"+runtime+"','YYYYMMDDHH24MISS'))*24/"+ITOA(cycle)+") from dual ";
		break;
	case	2: //��
		strql = "select ceil((to_date('"+curtime+"','YYYYMMDDHH24MISS') - to_date('"+runtime+"','YYYYMMDDHH24MISS'))/"+ITOA(cycle)+") from dual "; 
		break;
	case 3://��
		strql = "Select ceil(months_between(to_date('"+curtime+"','YYYYMMDDHH24MISS'),to_date('"+runtime+"','yyyymmddhh24miss'))/"+ITOA(cycle)+") From dual";
		break;
	case 4://��
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
		
		/* 2014-01-03 ��Ϊ����
		stmt>>imultiple;  

		strql = CalTime(runtime, imultiple*cycle,cycle_type);
		stmt.close();
		conn.close();
		return strql;
		*/
		stmt>>imultiple;  		//����ORACLE��������,�����ǰֵ������ʱ�����̫�ٵĻ��������Ϊ0  		
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

		//��ѯȫ��
		std::string sql;
        //cycle_type 0 ���� 1Сʱ 2�� 3�� 4��
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
					/*logerror(LOG_CODE_AUDIT_CFG_ERR,"script_typeֻ֧��SQL���ͺ�SHELL����");*/
					theJSLog<<"script_typeֻ֧��SQL���ͺ�SHELL����"<<endi;
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

	WRITE_LOG("�����������ݹ�"+ITOA(vecFileContrab.size())+",�ܹ���"+ITOA(g_Fork)+"���ӳ���");

	int i = 0;
	for (std::vector<FileContrab>::iterator p = vecFileContrab.begin();
		p != vecFileContrab.end(); ++p)
	{
		p->i_Fork = ( i++ %g_Fork );
	}

	theJSLog<<"���̺ŷ������£�"<<endi;
	for (std::vector<FileContrab>::iterator p = vecFileContrab.begin();p != vecFileContrab.end(); ++p)
	{
		theJSLog<<"group_id :"<< p->group_id <<",���̺�:"<<p->i_Fork<<endi;		
	}

	WRITE_LOG("FileRunContrab::AllocationForkNum invoke End!");
	return  true;
}


bool FileRunContrab::LoopProcess(int process_mod_num)
{
	/*std:: << "�ӽ���: " << process_mod_num << ",�ӽ��̺�: " << getpid() << ",�����̺�: " << getppid() << endl;*/
	std::string strTemp = " �ӽ���: " + ITOA(process_mod_num) +",�ӽ��̺�:"+ ITOA( getpid()) + ",�����̺�: " + ITOA(getppid()); 
	WRITE_LOG(strTemp);

	signal( SIGINT, sig_exit );
	signal( SIGQUIT, sig_exit );
	signal( SIGTERM, sig_exit );
	signal(12, sig_flushexit);
	//������һ������ʱ�䣬û�����У�������´ε�ʱ��
	
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
				if (cycle == 0)//˵��ֻ����һ�Σ���cycle_type ����Ϊ4(��)��cycle=99,��ζ���99��ִ��һ��
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
		++i; //�״δ�����Ҫis_immediate_run�����ж�

		/*��ȡ�˳��ź�*/
		if ( p_exit )
		{
			theJSLog<< "�ӽ���" << getpid() << "�յ��˳��źţ��������˳�ǰ����..." <<endi;
			for ( int i = 0; i < 3; i++ )
			{
				sleep( 1 ); //����3�룬�ȴ��ӽ��̷���
			}
			theJSLog<< "�ӽ���" << getpid() << "�˳�" <<endi;

			exit(0);
		}

		if (p_flushexit)
		{
			theJSLog<< "�ӽ���" << getpid() << "�յ�ˢ���˳��źţ��������˳�ǰ����..." <<endi;
			for ( int i = 0; i < 5; i++ )
			{
				sleep( 1 ); //����3�룬�ȴ��ӽ��̷���
			}
			theJSLog<< "�ӽ���" << getpid() << "�˳�" <<endi;

			exit(0);
		}


		std::vector<FileContrab>::iterator iter_sync_info_map = vecFileContrab.begin();
		for( ; iter_sync_info_map != vecFileContrab.end(); ++iter_sync_info_map )
		{      
			//���Ǵ���Ľ��̺�,continue
			if (iter_sync_info_map->i_Fork != process_mod_num )
			{
				continue;
			}

			std::string strBatchTemp = Poco::DateTimeFormatter::format( Poco::LocalDateTime(), "%Y%m%d%H%M%S" );

			bool bInRun = false;

			//��ǰʱ��==��ʱ����ʱ��
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
		//return true;//����
	}
	return true;
}


//�ַ�������
std::string FileRunContrab::stringDecryption( std::string  str )
{
	if ( strchr( str.c_str(), '/' ) || strchr( str.c_str(), '@' ) )
	{
		/*logerror(LOG_CODE_RC_FIELD_TYPE_ERR, "���õ����ݿ����Ӵ�"+str+"�����Ǽ��ܺ�ġ�����ʹ��Cipher������ܡ�!" );*/
		theJSLog<<"���õ����ݿ����Ӵ�"+str+"�����Ǽ��ܺ�ġ�����ʹ��Encryptd������ܡ�!"<<endi;
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
		//huangtodo �������ƺ���һ��
	/*	logerror(LOG_CODE_RC_FIELD_TYPE_ERR, "�������ݿ����Ӵ�ʧ��!");*/
		theJSLog<<"�������ݿ����Ӵ�ʧ��!"<<endi;
		return "";
	}

   
	theJSLog<<"�������ݿ����Ӵ��ɹ�!" <<endi;
	return strjiemi;

	

}


int FileRunContrab::getDbLoginInfo( std::string str, std::string& user, std::string& pwd, std::string& serv )
{
	std::string::size_type a = str.find('/');
	if (a == string::npos)
	{
		theJSLog<<"���ݿ����Ӵ��������󣡸�ʽӦΪ���û���/����@������"<<endi;
		return -1;
	}

	std::string::size_type a2 = str.rfind('@');
	if (a2 == string::npos)
	{
		theJSLog<<"���ݿ����Ӵ��������󣡸�ʽӦΪ���û���/����@������"<<endi;
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

// ����ֵ2 �ű����гɹ��������ʧ�ܣ�1 �ű����гɹ�������ɹ��� -1 û��ִ��

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
			remark += contrabID.connectstring+"���ݿ����ʧ��";
			return 2;
		}
		std::string  user ;
		std::string  pwd;
		std::string  serv ;

		if ( 1 !=  getDbLoginInfo(strjiemi, user,  pwd,  serv ))
		{
			remark += strjiemi+"���ݿ�����ʧ��";
			return 2;
		}


		Environment      *pEv   =   NULL;
		Connection       *pConn =   NULL;
		oracle::occi::Statement *pStmt =   NULL;

		pEv   =   Environment::createEnvironment(); 
		pConn =   pEv->createConnection(user, pwd, serv);

		if ( NULL == pConn)
		{
			remark +="��������ʧ��";
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
			remark ="�ɹ�";
		}
		else
		{
			bfinish = 2;
			remark ="���ֵΪ"+ITOA(max)+",����1";
		}
		
	}
	catch(oracle::occi::SQLException ex)
	{   
		theJSLog<<"���ݿ����:"<< ex.getMessage()<<endi;	
		remark += std::string(ex.getMessage());
	}
	catch (...)
	{
		/*logerror(LOG_CODE_MPROC_OTHER_ERR, "FileRunContrab::RunSQL δ֪�쳣");*/
		WRITE_LOG("FileRunContrab::RunSQL δ֪�쳣");
		remark += "FileRunContrab::RunSQL δ֪�쳣";
		
	}

	
	theJSLog<<"FileRunContrab::RunSQL invoke End!"<<endi;
	return bfinish;

}

// ����ֵ2 ʧ�ܣ� 1�ɹ�
int FileRunContrab::RunShell(ContrabID & contrabID,std::string &remark)
{  
	theJSLog<<"FileRunContrab::RunShell invoke begin!"<<endi;
   try
   {
	//	string cmd = contrabID.run_script ;
	//	cmd.insert( 0, "(" );
	//	cmd.append( ") && echo \"11100101011\"" );
 //  
	//	WRITE_LOG("ִ�нű�:"+cmd);

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
	//		logerror(LOG_CODE_MPROC_OTHER_ERR,"popen����"+cmd);
	//		return -1;
	//	}

	//	if ( result.find( "11100101011" ) == string::npos ) 
	//	{
	///*	cout << "�ű����д���,û�����гɹ�" << endl;*/
	//		logerror(LOG_CODE_MPROC_OTHER_ERR,"�ű����д���,û�����гɹ�,�ű���"+cmd);
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
	
		theJSLog<<"ִ�нű�m_initScript:"+m_initScript<<endi;
        
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
				/*logerror(LOG_CODE_MPROC_OTHER_ERR,"�ű����д���,û�����гɹ�,�ű���"+m_initScript);*/
				theJSLog<<"�ű����д���,û�����гɹ�,�ű���"<<m_initScript<<endi;
				remark +="�ű����д���,û�����гɹ�,�ű���"+m_initScript;
				return 2;
			}

		/*	result = buff;*/
		} 
		else 
		{
			/*logerror(LOG_CODE_MPROC_OTHER_ERR,"�ű����д���,û�����гɹ�,�ű���"+m_initScript);*/
			theJSLog<<"�ű����д���,û�����гɹ�,�ű���"<<m_initScript<<endi;
			remark +="�ű����д���,û�����гɹ�,�ű���"+m_initScript;
			return 2;
			
		}

		
		if(result.find("RESULT==1") !=string::npos)
		{   
			remark ="�ɹ�";
			return 1;
		}
		else
		{   
			remark += "�Ҳ���RESULT==1";
			return 2;
		}

   }
   catch(...)
   {   
	   remark +="δ֪�쳣";
	   return -1;
   }

   	theJSLog<<"FileRunContrab::RunShell invoke End!"<<endi;

}

int  FileRunContrab::ChooseRun(ContrabID & contrabID ,std::string &remark)
{   
	theJSLog<<"FileRunContrab::ChooseRun INVOKE BEGIN!"<<endi;
	int blastRunState = 2;
	//sql���ִ��
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

//����� ������
// ��� ���ؼ�
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

//	theJSLog<<"FileRunContrab::output��"<<strCompare2<<endi;
	theJSLog<<"FileRunContrab::output invoke end!"<<endi;
}

bool FileRunContrab::SyncSingleTable(FileContrab &fileContrab)
{
	theJSLog<<"FileRunContrab::SyncSingleTable invoke begin!"<<endi;
     
	if ( p_exit )
	{
		theJSLog<< "�ӽ���" << getpid() << "�յ��˳��źţ��������˳�ǰ����..." <<endi;		
		sleep( 1 ); //����1�룬�ȴ��ӽ��̷���
		theJSLog<< "�ӽ���" << getpid() << "�˳�" <<endi;

		exit(0);
	}
	

	std::string strBatchTemp = Poco::DateTimeFormatter::format( Poco::LocalDateTime(), "%Y%m%d%H%M%S" );
	theJSLog<<"��ǰʱ��Ϊ"<<strBatchTemp<<", �������group_id:"<<fileContrab.group_id
		<<", �������ָ��ʱ��:"<<fileContrab.runtime<<" ,����ʱ�䣺"<<fileContrab.cycle<<endi;

     
//	theJSLog<<"user:"<<user<<" ,pwd:"<<pwd<<", serv:"<<serv<<endi;
	DBConnection conn = DBConnectionFactory::getInstance().createConnection( user.c_str(), pwd.c_str(), serv.c_str() );
	conn.open();
	util_1_0::db::Statement stmt = conn.createStatement();
	stmt.setSQLString("alter session set NLS_DATE_FORMAT=YYYYMMDDHH24MISS");
	stmt.execute();

	int ilastRunState = -1; // 0 ��פ����, 1,���гɹ� 2������ʧ�ܣ�3��û������

	std::string sql = "INSERT INTO process_crontab_run_log(group_id,item_id,begindate,enddate,state,remark,create_date )"
		" VALUES(:group_id,:item_id, to_date(:begindate,'YYYYMMDDHH24MISS'), to_date(:Enddate,'YYYYMMDDHH24MISS'), :STATE, :REMARK, SYSDATE )";
	
	stmt.setSQLString(sql);



	for (std::vector<ContrabID>::iterator p = fileContrab.vecContrab.begin();
		p != fileContrab.vecContrab.end(); ++p)
	{   
        strBatchTemp = Poco::DateTimeFormatter::format( Poco::LocalDateTime(), "%Y%m%d%H%M%S" );
		
		theJSLog<<"strBatchTemp:"<<strBatchTemp<<endi;
		//isRelation ˵��
		// -1 ����ʧ��
		// 0  ��פ����
		// 1  ��һ�����гɹ��������У���������,����һ��
		// 2  ��һ������ʧ�ܣ������У���������������һ��
		// 3  ��һ�������������У���������������һ��
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
				remark ="����";
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
				remark ="����";
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
				remark ="����";
			}
		}
		std::string strBatchTempEnd = Poco::DateTimeFormatter::format( Poco::LocalDateTime(), "%Y%m%d%H%M%S" );
        stmt<<p->group_id<<p->item_id<<strBatchTemp<<strBatchTempEnd<<ilastRunState<<remark;
		stmt.execute();
	
//		std::cout<<"������־��sql: "<<sql<<endl;


		theJSLog<<"p->group_id: "<<p->group_id<<"  p->spec_order "<<p->spec_order<<" ��״̬Ϊ "<<ilastRunState<<endi;

	}

		stmt.commit();
		conn.close();
	

	theJSLog<<"FileRunContrab::SyncSingleTable invoke End!"<<endi;		std::cout<<std::endl;
	return true;
}


void showHelpInfo() 
{
	WRITE_LOG("������");
//	WRITE_LOG("psShellCrontab <-lgz> [-t1] [-r0] ");	WRITE_LOG("psShellCrontab <-t1> [-j5] [-f10] ");
//	WRITE_LOG("-l�Ǳ�ѡ����,�ò���������ӵ��ǵ�����λƴ��,���ݵ���λƴ����Ϊgz ");
	WRITE_LOG("-j�ǿ�ѡ����,�ò���������ӵ��ǲ����Ǵ���ˢ�����ݵ�ʱ��");
	/*WRITE_LOG("-d�ǿ�ѡ����,�ò�����ʾ�Ƿ��ӡ��־,Ĭ�ϲ����");*/
	WRITE_LOG("-f�ǿ�ѡ����,�ò�����ʾ��ָ����������С�����е����������Զ�ȡ���е�����");
    WRITE_LOG("-t�Ǳ�ѡ����,���е�group_id ����-t1,3");
	WRITE_LOG("-h ��������");

}


bool checkParam( int argc, char *argv[] ) 
{
	WRITE_LOG("checkParam begin!");
	if( argc == 1 )
	{
		/*logerror(LOG_CODE_APP_PARAM_LACK, "�����в��������쳣,ȱ�ٵ��в���-l ");*/
	//	theJSLog<<"�����в��������쳣,ȱ�ٵ��в���-l "<<endi;		theJSLog<<"�����в��������쳣,ȱ����������Ų���-t "<<endi;
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
			/*logerror(LOG_CODE_APP_PARAM_NONSUPPORT, "��������ȷ,��֧��:" + _argv_i);*/
			theJSLog<<"��������ȷ,��֧��:"<<_argv_i<<endi;
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
	/*	logerror(LOG_CODE_APP_PARAM_LACK, "ȱ�ٵ��в���-l ");*/
	//	theJSLog<<"ȱ�ٵ��в���-l "<<endi;		theJSLog<<"ȱ����������Ų���-t "<<endi;
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
		theJSLog<<"���ü���ʧ��"<<endi;
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
		 

		//��ȡ���ݿ��û���
		std::string strUser;
		if(!ReadIni.GetValue(strUser, DBUSER_ENV_NAME)) 
		{
			return -1;
		}
		encrypt.Decrypt(strUser.c_str(), pchUserName);

		//��ȡ���ݿ��û�����
		std::string strPwd;
		if (!ReadIni.GetValue(strPwd,DBPASS_ENV_NAME))
		{
			return -1;
		}
		encrypt.Decrypt(strPwd.c_str(), pchUserPWD);
		///��ȡ����Ĵ�ӡ��־·��		
		std::string strLog_path;		
		if(!ReadIni.GetValue(strLog_path,LOG_PATH_NAME))		
		{			
			return -1;		
		}		
		szLogPath=strLog_path;		
	//	std::cout<<"szLogPath "<<szLogPath<<endl;		
		//��ȡ����Ĵ�ӡ��־����		
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

//2013-12-31 ���������ļ���Ϊ�����Ĳ���
bool FileRunContrab::Init()
{
	IBC_ParamCfgMng param_cfg;

	if( !param_cfg.bOnInit() )		//���Ĳ�����Ҫ�Լ���ʼ��
	{
		string sErr;
		int nCodeId;
		param_cfg.getError(sErr,nCodeId);
		cerr<<"�������ýӿڳ�ʼ��ʧ�ܣ�������="<<nCodeId<<", ������Ϣ="<<sErr<<endl;
		return false;
	}

	// �Ӻ��Ĳ��������ȡ��־��·��������
	 char sParamName[256];
	 CString sKeyVal;
	 sprintf(sParamName, "log.path");
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		szLogPath = sKeyVal;

	 }
	 else
	 {	
		cout<<"���ں��Ĳ�����������־��·��"<<endl;
		return false ;
	 }	 
	 sprintf(sParamName, "log.level");
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		szLogLevel = sKeyVal;
	 }
	 else
	 {	
		cout<<"���ں��Ĳ�����������־�ļ���"<<endl;
		return false ;
	 }

	 //�ж�Ŀ¼�Ƿ����
	 DIR *dirptr = NULL; 
	 if((dirptr=opendir(szLogPath.c_str())) == NULL)
	 {
		cout<<"��־Ŀ¼["<<szLogPath<<"]��ʧ��"<<endl;	
		return false ;
	 }else closedir(dirptr);
		
	//theJSLog.setLog(szLogPath.c_str(),atoi(szLogLevel.c_str()),"GJJS","DSQ",001);
	
	//if(!(dbConnect(conn)))
	// {
	//	memset(erro_msg,0,sizeof(erro_msg));
	//	sprintf(erro_msg,"init()  �������ݿ�ʧ�� connect error");
	//	theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
	//	return  ;
	//}
	
	std::string  pchUserName,pchUserPWD,strDBname;

try{
	//��ȡ���Ĳ��������ݿ�������Ϣ

    tpss::getKenelParam( "db.zb.db_server_name", strDBname );
    tpss::getKenelParam( "db.zb.username", pchUserName );
    tpss::getKenelParam( "db.zb.password", pchUserPWD );

    //������תΪ����
    Cipher cipher;
    pchUserPWD = cipher.GetPlainText( pchUserPWD.c_str() );

	//	std::cout<<"szLogLevel "<<szLogLevel<<endl;
	//	theJSLog<<"pchUserName="<<pchUserName<<",pchUserPWD="<<pchUserPWD<<",strDBname="<<strDBname<<endi;

	_conn = DBConnectionFactory::getInstance().createConnection( pchUserName, pchUserPWD, strDBname.c_str() );
	_conn.open();
	
	user = pchUserName;
	pwd = pchUserPWD;
	serv = strDBname;
	
	theJSLog<<"�������ݿ�ɹ�..."<<endi;

  }catch( util_1_0::db::SQLException &ex )
   {
       if( ex.getErrorCode() == -1017 ) 
		{
            writelog( LOG_CODE_USER_PASSWORD_ERR, "���ݿ������û��������" );
        }else
		{
            writelog( 69101, "�������ݿ�����ʧ��: " + std::string( ex.what() ) );
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

	theJSLog<<"����Ϊ�ػ�����..."<<endi;
	initDaemon(true);		//2014-01-10����Ϊ�ػ�ģ��

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
		
		theJSLog<<"��"<<(++ttt)<<"�μ�������"<<endi;
		FileRunContrab RunContrab;
		RunContrab.Init();				
		theJSLog.setLog(szLogPath.c_str(),atoi(szLogLevel.c_str()),"ShellCrontab", "GJJS", 001);

		theJSLog<<"��־·����"<<szLogPath<<" ��־����"<<szLogLevel<<endi;
		if ( ! RunContrab.ReadConfig())
		{
			return -1;
		}
		RunContrab.output();

		RunContrab.AllocationForkNum();

		if(RunContrab.vecFileContrab.size()  ==0)
		{
			WRITE_LOG("û������");
			return 1;
		}

		//fork ֮ǰ�����ȼ���Ƿ�ر�
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
				/*theJSLog<< "�����ӽ���ʧ��" <<endi;*/
		/*		logerror(LOG_CODE_PROCESS_CREATE_ERR, "�����ӽ���ʧ��");*/
				theJSLog<<"�����ӽ���ʧ��"<<endi;
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
				//���ݼ�����
				cur = now;

				FileRunContrab RunContrabCompare;
				RunContrabCompare.Init();
				RunContrabCompare.ReadConfig();		
		
				
				if (RunContrabCompare != RunContrab)
				{

					WRITE_LOG("���¼��ص�������Դ���ݲ�����");

					for(std::vector<int>::iterator iter = vec_pid.begin();
						iter != vec_pid.end(); ++iter)
					{
						int error = kill(*iter, 12);
						if (error < 0)
						{
							WRITE_LOG("���ӽ���"+ITOA(*iter)+" �����ź���12 ʧ��");
						}
						else
						{
							WRITE_LOG("���ӽ���"+ITOA(*iter)+" �����ź���12 �ɹ�");
						}

					}

					p_flushexit = true;
					break;
				}
				
			}
			

			/*��ȡ�˳��ź�*/
			if ( p_exit )
			{   
				theJSLog<< "�յ��˳��źţ��������˳�ǰ����..." <<endi;
				for ( int i = 0; i < 5; i++ )
				{
					sleep( 1 ); //����5�룬�ȴ��ӽ��̷���
				}
				theJSLog<< "�����˳�" <<endi;

				exit( 0 );
			}
			sleep( 1 ); //����1��
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

		theJSLog<<"���¼��������ˣ�"<<endi;

	}
}

