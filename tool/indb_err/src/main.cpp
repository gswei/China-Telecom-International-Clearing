//��ϵͳ������־���,����Ϣ��


#include<iostream>
#include <vector>

#include "CF_Common.h"
#include "CF_CLogger.h"
#include "CF_CFscan.h"

#include "CfgParam.h"
#include<dirent.h>
#include <sys/stat.h>  //stat()��������ѯ�ļ���Ϣ

using namespace tpss;  //��psutil.h��Ӧ
using namespace std;

CLog theJSLog;

bool gbExitSig=false;

int days=-7;
CF_CFscan scan;									//�ļ�ɨ��ӿ�

struct stat fileInfo;
struct tm now;

char erro_msg[1024],szLogPath[1024],szLogLevel[10],work_path[1024];

//�����ļ���¼��ʽ
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
	cerr<<"  ���յ��˳��ź�!!!\n";
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
	cout<<"indb_err  �����ļ�·��    \n"<<endl;
}


//�������-s ����Դ   -t ����
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
				 
				//ɨ��Ŀ¼������ļ�,����Ų������Ŀ¼
						
				if(scan.openDir(tmp_dir))
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"�򿪻����ļ�Ŀ¼[%s]ʧ��",tmp_dir);
					theJSLog<<erro_msg<<endw; //��Ŀ¼����
					continue ;
				}		
				
				while(1)		
				{
					memset(fileName,0,sizeof(fileName));
					rett = scan.getFile("err*",fileName);  				
					if(rett == 100)
					{	
						//cout<<tmp_dir<<":Ŀ¼��û���ļ�..."<<endl;
						break;
					}
					if(rett == -1)
					{	
						cout<<"��ȡ�ļ���Ϣʧ��"<<endl;
						break ;				
					}
					
					char* p = strrchr(fileName,'/');
					memset(m_szFileName,0,sizeof(m_szFileName));
					strcpy(m_szFileName,p+1);	
					
					theJSLog<<"######## scan file "<<fileName<<" ########"<<endi;
					
					//�ж��ļ��Ƿ����ڱ���д
					
					//����Ų������Ŀ¼
					sprintf(tmp,"%s%s",work_path,m_szFileName);
					//cout<<"����Ų������Ŀ¼"<<tmp<<endl;
					
					if(rename(fileName,tmp))
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"�ļ�[%s]�ƶ���[%s]ʧ��: %s",fileName,tmp,strerror(errno));
						theJSLog<<erro_msg<<endw;
						continue;
					}
					
					//vfile.push_back(tmp);
				}
					
				scan.closeDir();
			}
		}
	
		//���������ļ����	
		if(scan.openDir(work_path))
		{	
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"�򿪹����ļ�Ŀ¼[%s]ʧ��",work_path);
			theJSLog<<erro_msg<<endw; //��Ŀ¼����
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
				cout<<"��ȡ�ļ���Ϣʧ��"<<endl;
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
				sprintf(erro_msg,"�������ݿ�ʧ�� connect error");
				theJSLog<<erro_msg<<endw;		//�������ݿ�ʧ��
				return  ;
			}
			
			stmt = conn.createStatement();
			
			ifstream errfile(fileName,ios::nocreate);
			if(!(errfile.is_open()))
			{
				conn.close();
				theJSLog<<"�ļ�:"<<fileName<<"��ȡʧ��!"<<endw;
				continue;	
			}
			
			string line,tmp_line;
			stat(fileName,&fileInfo);
			
			now =*localtime ( &(fileInfo.st_mtime) ); now.tm_mon++;
			sprintf( errLogTime,"%4u%02u%02u%02u%02u%02u",	now.tm_year+1900, now.tm_mon, now.tm_mday,now.tm_hour, now.tm_min, now.tm_sec);
							
			cout<<"�ļ�����޸�ʱ��:"<<errLogTime<<endl;
			memset(tmp_date,0,sizeof(tmp_date));
			strncpy(tmp_date,errLogTime,8);
				
			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into D_SYSERR_LOG(FILENAME,PROCESS_ID,LOG_CONTENT,LOG_CODE,ERR_TIME,STAT_TIME,LOG_LEVEL) values(:1,:2,:3,:4,:5,:6,:7)");
			stmt.setSQLString(sql);
			
			getCurTime(currTime);
			//�ļ���¼��ʽ [22:52:52][005988][4-����] ���κ˶��ļ��˶�ʧ��,ʧ�ܸ���:(2)[218]
			//             [08:47:18][012580][4-����] ���κ˶��ļ�[NROAM_201412260825_0066.AUD]�˶�ʧ��,ʧ�ܸ���:(2)[218]
			while(getline(errfile,line))
			{
				//cout<<"�ļ�����:"<<line<<endl;
				
				RecordFmt rfmt;	
				
				sprintf(rfmt.errTime,"%s%s%s%s",tmp_date,line.substr(1,2),line.substr(4,2),line.substr(7,2));			
				sprintf(rfmt.ppid,"%s",line.substr(11,6));
				sprintf(rfmt.errLevel,"%s",line.substr(19,1));
				
				tmp_line=line.substr(26);
				snprintf(rfmt.errMsg ,sizeof(rfmt.errMsg),"%s",tmp_line.substr(0,tmp_line.find_last_of("[")));  
				
				tmp_line=tmp_line.substr(tmp_line.find_last_of("[")+1);
				sprintf(rfmt.errCode,"%s",tmp_line.substr(0,tmp_line.find("]")));
				
				theJSLog<<"ʱ��:"<<rfmt.errTime<<" ����ID:"<<rfmt.ppid<<" ��־�ȼ�:"<<rfmt.errLevel<<" ������:"<<rfmt.errCode<<" ������־����:"<<rfmt.errMsg<<endi;
				stmt<<m_szFileName<<rfmt.ppid<<rfmt.errMsg<<rfmt.errCode<<rfmt.errTime<<currTime<<rfmt.errLevel;
				stmt.execute();
				
				line="";
			}
					
			errfile.close();
			
			if(remove(fileName))
			{
				stmt.rollback();
				
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"�ļ�[%s]ɾ��ʧ��: %s",fileName,strerror(errno));
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
		sprintf(erro_msg,"���ݿ����%s(%s)",e.what(),sql);
		theJSLog<<erro_msg<<endw;		//����sqlִ���쳣
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
	
	if((isProcExist("indb_err")-1))		//�ӿ������� ��-1ȥ������
	{
		cout<<"�����Ѿ�����!"<<endl;
		return 1;
	}
	
	initDaemon(true);
		
	// �Ӻ��Ĳ��������ȡ��־��·��������
	IBC_ParamCfgMng param_cfg;
	
	if( !param_cfg.bOnInit() )		//���Ĳ�����Ҫ�Լ���ʼ��
	{
		string sErr;
		int nCodeId;
		param_cfg.getError(sErr,nCodeId);
		cerr<<"�������ýӿڳ�ʼ��ʧ�ܣ�������="<<nCodeId<<", ������Ϣ="<<sErr<<endl;
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
		cout<<"���ں��Ĳ�����������־��·��"<<endl;
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
		cout<<"���ں��Ĳ�����������־�ļ���"<<endl;
		return false ;
	 }
	
	theJSLog.setLog(szLogPath, atoi(szLogLevel),"INDB_ERR","GJJS", 001);
	
	 completeDir(szLogPath);
	 sprintf(szLogPath,"%serror_log/",szLogPath);
	
	 //�ж�Ŀ¼�Ƿ����
	 DIR *dirptr = NULL; 
	 
	 if((dirptr=opendir(szLogPath)) == NULL)
	 {
		cout<<"������־Ŀ¼["<<szLogPath<<"]��ʧ��"<<endl;	
		return false ;
	 }else closedir(dirptr);
	
	theJSLog<<"������־Ŀ¼:"<<szLogPath<<endi;
	
	memset(work_path,0,sizeof(work_path));
	sprintf(work_path,"%swork_path/",szLogPath);	
	theJSLog<<"����Ŀ¼:"<<work_path<<endi;
	
	setSignalPrc();
	
	while(1)
	{
		run();
		sleep(60);
		theJSLog<<"sleep 60s ...."<<endi;
		theJSLog.reSetLog();
	}
	
}






