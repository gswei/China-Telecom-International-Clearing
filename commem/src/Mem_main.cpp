/*
 * =====================================================================================
 *
 *       Filename:  Mem_main.cpp
 *
 *    Description:  �����ݿ��е����ݱ���빲���ڴ����������ֺ����ݿ��ͬ����ʹ������������������ݷ���
 *
 *        Version:  1.0
 *        Created:  2010��05��09�� 08ʱ59��13��
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *        Company:
 *
 *    update list: 
 *    2010-05-09 :  3.0.1 : ��ʼ�汾
 *    2010-10-21 :  3.0.2 : ����������������������ַ�±���ַ���ֱ�ӵ��ַ�������������ѭ�� (CommonMemLenIndex.cpp)
 *    2010-11-22 :  3.0.3 : ����keyֵ���ƣ���ftok�������
 *		2011-02-18 :	3.0.4 : �����������������ִ�Сд������ռ�ÿռ䡣
 *		2011-05-25 :	3.0.5 : ����linux���⣬���������ڴ���key�ĳ�env/commem/0�����㡣
 *		2011-12-24 :	3.0.6 : ����commonmemTabledefine��ѯʱ���泤���������������˶�����
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
		printf("�汾�� 3.0.6, ����������:2011-12-24 by sunhua\n");
		printf("Usage:	%s  -h ask for help. \n",argv[0]);
		return 0;
	}

	if ( strcmp(argv[1],"-h" )==0 )
	{
		printf("�汾�� 3.0.6, ����������:2011-12-24 by sunhua\n");
		printf("    -c         :  �������Ϲ����ڴ�\n");
		printf("    -k         :  ɾ�����Ϲ����ڴ�\n");
		printf("    -rc [line] :  ��ȡ���Ϲ����ڴ湫������Ϣ [x]Ϊ��ѡ����ֻ����x�м�¼\n");
		printf("    -f [mem_name] [queryCondition] [indexID] : \n" ); 
		printf("        ����queryCondition����,ʹ��indexID������ѯ�����ڴ�mem_name��ƥ�������\n");
		return 0;
	}

	char v_envpath[256];
	char iMessage[512];
	char logpath[256];
	char envname[50];
	char errmsg[500]; 
	int  debugFlag;

	//���Ĳ�����ȡ
	std::string log_path,log_level;
	if( !tpss::getKenelParam( "log.path", log_path ) ) {
		tpss::writelog( 0, "��ȡ��־·��ʧ��" );
	}
	if( !tpss::getKenelParam( "log.level", log_level ) ) {
		tpss::writelog( 0, "��ȡ��־����ʧ��" );
	}
	int level_no = es::StringUtil::toInt( log_level );

	strcpy(logpath,log_path.c_str());
	debugFlag =  level_no;
    //���envpath
	strcpy(v_envpath,log_path.c_str());   
	//cout << "log_path = " <<log_path<<endl;
	//cout << "level_no = " <<level_no<<endl;
	/*strcpy(v_envpath,getenv("ENVPATH"));
	completeDir(v_envpath);
	string DBPATH=v_envpath;
	DBPATH+="zhjs.ini";

	//��ϵͳ���������л�ȡ�����ļ�·��
	
	if ( strlen(v_envpath)<1 )
	{
		printf("��ϵͳ���������л�ȡ��������·��[ENVPATH]����\n");
		return (-1);
	}
	
	CReadIni v_ini;
	v_ini.init(DBPATH.c_str());
	cout<<DBPATH.c_str()<<endl;
	
	//�����ļ��л�ȡ��־�ȼ�(��־�ȼ������Ǵ������ļ��������ݿ���)
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
	
	//�����ļ��л�ȡ��־·��
	if ( ! v_ini.GetValue("COMMON","LOG_PATH",logpath,'Y') )
	{
		sprintf(errmsg,"get env LOG_PATH from %s err!",DBPATH.c_str());
		printf( "%s\n",errmsg );
		return ( -1 );
	}	*/
	completeDir( logpath );
	
	theJSLog.setLog(logpath, debugFlag, "1", "comem", 1);
	
	theJSLog<<"���Եȼ�="<<debugFlag<<"=,"<<__FILE__<<","<<__LINE__<<endd; 
	theJSLog<<"��־Ŀ¼·��="<<logpath<<"=,"<<__FILE__<<","<<__LINE__<<endd;
	
	//��ȡͬ�����ݿ�ʱ��
	/*long iSleepTime;
	char temp[20];
	if (! v_ini.GetValue("COMMON","UPDATE_DB_TIME",temp,'Y')<0)
	{
		errLog(LEVEL_WARN,"-",10001,"�ӻ������������ļ��л�ȡ��������ʱ�����[UPDATE_DB_TIME].",__FILE__,__LINE__);
		iSleepTime=60;
	}
	else
	{
		iSleepTime=atol(temp);
	}
	theJSLog<<"��ѯ���ݿ�ʱ��="<<logpath<<"=,"<<__FILE__<<","<<__LINE__<<endd;*/

	CommonMemManager vCommonMgr;
	if ( strcmp(argv[1],"-c") == 0 )
	{
		int ret=ProcIsExist(argc, argv);
		theJSLog<<"ProcIsExist������="<<ret<<"=,"<<__FILE__<<","<<__LINE__<<endd;
		if (ret > 1)
		{
			errLog(LEVEL_FATAL,"-",10006,"�����ڴ�����Ѵ��ڣ����Ҫ��������ִ��commem -k.",__FILE__,__LINE__);
			return ( -1 );
		}
		
		try
		{
			theJSLog<<"��ʼ�����ػ�ģʽ"<<endi;
			bool daemonFlag=true;
			/*if (! v_ini.GetValue("COMMON","DAEMON_MODE",temp,'Y')<0)
			{
				errLog(LEVEL_WARN,"-",10001,"�ӻ������������ļ��л�ȡ�����ػ�ģʽ����[DAEMON_MODE].",__FILE__,__LINE__);
				
				daemonFlag=true;
			} else {
				if ( strcmp( temp,"Y" ) == 0 ) daemonFlag=true;
				else daemonFlag=false;
			}*/
			if (daemonFlag) theJSLog<<"����Ϊ�ػ�ģʽ"<<endi;
			initDaemon(daemonFlag);
			
			//theJSLog<<"�������ݿ�����"<<endi;
		//	connectDB( (char *)DBPATH.c_str(), DBConn ); 
			
			theJSLog<<"�����ڴ濪ʼ��ʼ��"<<endi;
			vCommonMgr.init(v_envpath);
			
			theJSLog<<"�����ڴ濪ʼ����������"<<endi;
			vCommonMgr.createCommonTable();
			
			theJSLog<<"�����ڴ濪ʼ�����������ϱ�"<<endi;
			vCommonMgr.loadAllTable();
			
		//	if (DBConn.IsConnected() == 1) DBConn.Disconnect();
		//	theJSLog<<"�Ͽ����ݿ�����"<<endd;
		}
		catch(CException e)
		{
			errLog(LEVEL_FATAL, "-", 11000,
				"���������ڴ�����г���", __FILE__, __LINE__, e);
			return(-1);
		}

		//��ʱ��⹲�����Ƿ���Ҫ����
		while (1)
		{
			try
			{
				theJSLog.reSetLog();
				theJSLog<<"������ݿ�����"<<endd;
			//	if (DBConn.IsConnected() == -1) connectDB( (char *)DBPATH.c_str(), DBConn ); 
								
				theJSLog<<"��ѯ�Ƿ��п����Ҫ�������"<<endi;
				vCommonMgr.searchUpdate();
				
			//	if (DBConn.IsConnected() == 1) DBConn.Disconnect();
		//		theJSLog<<"�Ͽ����ݿ����ӣ�����"<<endd;
				
				sleep(60);

			}
			catch(CException e)
			{
			errLog(LEVEL_FATAL, "-", 11001,
				"���¹����ڴ�����г���", __FILE__, __LINE__, e);
				return(-1);
			}
		}

	} else if ( strcmp(argv[1],"-k") == 0 )
	{
		int ret=ProcIsExist(argc, argv);
		theJSLog<<"ProcIsExist������="<<ret<<"=,"<<__FILE__<<","<<__LINE__<<endd;
		if (ret > 1)
		{
			KillProc(ret);
		}
		
		try
		{
		//	theJSLog<<"�������ݿ�����"<<endi;
		//	connectDB( (char *)DBPATH.c_str(), DBConn ); 
			
			theJSLog<<"�����ڴ濪ʼ��ʼ��"<<endi;
			vCommonMgr.init(v_envpath);
			
			theJSLog<<"�����ڴ濪ʼж���������ϱ�"<<endi;
			vCommonMgr.freeAllTable();
			
			theJSLog<<"�����ڴ濪ʼɾ��������"<<endi;
			vCommonMgr.deleteCommonTable();
			
	//		if (DBConn.IsConnected() == 1) DBConn.Disconnect();
	//		theJSLog<<"�Ͽ����ݿ����ӣ��˳�"<<endi;
			
			return 0;
		}
		catch(CException e)
		{
			errLog(LEVEL_FATAL, "-", 11002,
				"ɾ�������ڴ�����г���", __FILE__, __LINE__, e);
				return(-1);
		}
	} else if ( strcmp(argv[1],"-rc" ) == 0 )
	{
		int readLine;
		if ( argc == 2 ) readLine = -1;
		else readLine = atoi( argv[2] );
		
		theJSLog<<"�����ڴ濪ʼ��ʼ��"<<endi;
		vCommonMgr.init(v_envpath);
		theJSLog<<"���ӹ��������ڴ���"<<endi;
		vCommonMgr.AttachCommonTable();
		theJSLog<<"��ȡ���������ڴ�����Ϣ��"<<endi;
		vCommonMgr.ReadCommonInfo(readLine);
		vCommonMgr.detach();
		return 0;
	} else if ( strcmp(argv[1],"-f") == 0 )
	{
		if ( argc != 5 )
		{
			printf("-f �����������ȷ��eg:commem -f [mem_name] [queryCondition] [indexID]\n");
			return 0;
		}
		try
		{
		//	theJSLog<<"�������ݿ�����"<<endi;
		//	connectDB( (char *)DBPATH.c_str(), DBConn ); 
				
			theJSLog<<"�����ڴ濪ʼ��ʼ��"<<endi;
			vCommonMgr.init(v_envpath);
    	
			theJSLog<<"���ӹ��������ڴ���"<<endi;
			vCommonMgr.AttachCommonTable();
    	
			theJSLog<<"����������["<<argv[2]<<"]�����ڴ���"<<endi;
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
			theJSLog<<"-f �ֹ���ѯ�����ڴ�����г���,�����:"<<e.GetAppError()<<"������Ϣ:"<<e.GetErrMessage()<<","<<__FILE__<<","<<__LINE__<<endi;
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
		errLog(LEVEL_FATAL,"-",10000,"��ϵͳ���������л�ȡ��������·��[LOGNAME]����.",__FILE__,__LINE__);
		return -1;
	}
	
	if((pid = (int)getpid()) <= 0)
	{
		errLog(LEVEL_FATAL,"-",10000,"getpid�����޷���ȡ����id.",__FILE__,__LINE__);
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
		theJSLog<<"�ɹ� KILL -9 ɱ������="<<iProc<<"="<<endi;
	} else
	{
		theJSLog<<"�ɹ�ɱ������="<<iProc<<"="<<endi;
	}
	
	return;
}
