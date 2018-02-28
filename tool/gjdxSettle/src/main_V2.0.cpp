//���ڹ��ʶ���ҵ��(IICM,IOCM)û��̯��ʡ,��Ҫ������������ҵ��(IICC,IOCC)��̯�ֱ�ĸ���̯��ʡ����ռ���� 
// Ȼ���ö���ҵ����ܷ������������ʡ�ķ���
// �۰�̨�͹��ʷֿ���,���Ӱ�������Դ��̯��
// 2014-05-15 ������λС��

#include<iostream>
#include <vector>

#include "CF_Common.h"
#include "CF_CLogger.h"

#include "CfgParam.h"
#include<dirent.h>

using namespace tpss;  //��psutil.h��Ӧ
using namespace std;

CLog theJSLog;

char mrate_cycle[6+1],msource[6],last_date[8+1],date_path[512];
bool gbExitSig=false;

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
	cout<<"�����������,���ʶ���̯�ֳ���(IISM,IOSM)\n"<<endl;

	cout<<"jsgjdx_settle -t ����    \n"<<endl;
}


//�������-s ����Դ   -t ����
int checkArg(int argc,char** argv)
{
	if(argc < 3)
	{
		printV();
		return -1;
	}

	int ret = 0;
	bool one_flag = false;

	//memset(msource,0,sizeof(msource));
	memset(mrate_cycle,0,sizeof(mrate_cycle));
	
	for(int i = 1;i<argc;i++)
	{
		if(strcmp(argv[i],"-t") == 0)
		{
			if(argc < (i+2))
			{
				cout<<"-t �������ʱ��"<<endl;
				ret =  -1;
				break ;
			}

			strcpy(mrate_cycle,argv[i+1]);
			one_flag = true;
		}
		
	}
	
	if(!one_flag)
	{
		printV();

		return -1;
	}
	
	return ret;
}



int main(int argc,char** argv)
{
	cout<<"********************************************* "<<endl;
	cout<<"*    China Telecom. Telephone Network         "<<endl;
	cout<<"*    InterNational Account Settle System      "<<endl;
	cout<<"*                                             "<<endl;
	cout<<"*           jsgjdx_settle					 "<<endl;
	cout<<"*           sys.GJZW.Version 1.0	             "<<endl;
	cout<<"*     created time :      2014-04-08 by  hed	 "<<endl;
	cout<<"*     last updaye time :  2013-05-15 by  hed	 "<<endl;
	cout<<"********************************************* "<<endl;
	
	if(checkArg(argc,argv))
	{
		return -1;
	}
	
	DBConnection conn;
	char erro_msg[1024],before_date[8+1],currTime[14+1];

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

	 char sParamName[256],szLogPath[PATH_NAME_LEN+1],szLogLevel[10];
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

	 //�ж�Ŀ¼�Ƿ����
	 DIR *dirptr = NULL; 
	 if((dirptr=opendir(szLogPath)) == NULL)
	 {
		cout<<"��־Ŀ¼["<<szLogPath<<"]��ʧ��"<<endl;	
		return false ;
	 }else closedir(dirptr);

	theJSLog.setLog(szLogPath, atoi(szLogLevel),"DX_SETTLE","GJJS", 001);
	
	setSignalPrc();

	if(!(dbConnect(conn)))
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"�������ݿ�ʧ�� connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
		return false ;
	}
	
	//select province_id,sum(city_settle_fee) from D_IICC_SHARE_RESULT_201310 where province_id <>199 group by province_id order by province_id;

	//int ret = 0,iSourceCount = 0;
	//char m_szSrcGrpID[8],strSourceId[8];
	//string sql ;
	Statement stmt;
	char sql[1024];

	double total_base = 0,before_total_base = 0;
	double sms_total_fee = 0;
	
	int prov_cnt = 0,percent_pos = 3;
	double prov_base = 0,cur_base = 0,before_base = 0;
	char prov_code[20],tmp_base[20];

	vector<string> v_prov_code;
	vector<double> v_prov_base;	

	try
	{		
		stmt = conn.createStatement();
		
		//��ɾ���������ڵ�̯�ֽ����
		memset(sql,0,sizeof(sql));
		sprintf(sql,"delete  from D_SMS_SETTLE_%s ",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();

		//�������ñ�� D_IICC_SHARE_RESULT_201310  ���Ŷ�Ӧ��D_SMS_ACCOUNT_201310��ȡ�����ܶ� roamtypeΪ1 ���ܶ�
		
		//�����ܷ���
		
		theJSLog<<"��������Դ IISM ....."<<endi;

		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(SETTLE_FEE_SDR) from D_SMS_ACCOUNT_%s where ROAM_TYPE = 1 ",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>sms_total_fee;	
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(city_settle_fee) from D_IICC_SHARE_RESULT_%s where province_id <> 199 ",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>total_base;
		
		theJSLog<<"sms_total_fee="<<sms_total_fee<<"  total_base="<<total_base<<endi;

		//��̨����-----------------------------------------------------------
		theJSLog<<"�����̨����"<<endi;
		//�����ʡ����ռ����
/*		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(count(1))from D_IICC_SHARE_RESULT_%s where province_id <> 199  and home_country_id in('9','37') group by province_id order by province_id ",mrate_cycle);
		cout<<"ʡ�ݸ���sql:"<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>prov_cnt;  //�����ʡ�ݸ���
		theJSLog<<"IICC ʡ�ݸ���:"<<prov_cnt<<endi;
*/
		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into D_SMS_SETTLE_%s(KMT_FLAG,SOURCE_ID,PROVINCE_ID,PERCENT,PERORG,PERORG_RMB,PERBASE,RMB_SDR_RATE) " 
					"select 1,'IISM',province_id,sum(city_settle_fee)/%lf,sum(city_settle_fee),sum(city_settle_fee),%lf,RMB_SDR_RATE from D_IICC_SHARE_RESULT_%s where province_id <> 199 and home_country_id  in('9','37') group by province_id,RMB_SDR_RATE order by province_id ",mrate_cycle,total_base,sms_total_fee,mrate_cycle);
		cout<<"sql = "<<sql<<endl;

		stmt.setSQLString(sql);
		stmt.execute();
		
		//��������-----------------------------------------------------------------
		theJSLog<<"�����������"<<endi;
		//�����ʡ����ռ����
/*		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(count(1))from D_IICC_SHARE_RESULT_%s where province_id <> 199  and home_country_id not in('9','37') group by province_id order by province_id ",mrate_cycle);
		cout<<"ʡ�ݸ���sql:"<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>prov_cnt;  //�����ʡ�ݸ���
		theJSLog<<"IICC ʡ�ݸ���:"<<prov_cnt<<endi;
*/
		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into D_SMS_SETTLE_%s(KMT_FLAG,SOURCE_ID,PROVINCE_ID,PERCENT,PERORG,PERORG_RMB,PERBASE,RMB_SDR_RATE) " 
					"select 0,'IISM',province_id,sum(city_settle_fee)/%lf,sum(city_settle_fee),sum(city_settle_fee),%lf,RMB_SDR_RATE from D_IICC_SHARE_RESULT_%s where province_id <> 199 and home_country_id not in('9','37') group by province_id,RMB_SDR_RATE order by province_id ",mrate_cycle,total_base,sms_total_fee,mrate_cycle);
		cout<<"sql = "<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		
		//����
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_SMS_SETTLE_%s set PERORG=PERORG*PERBASE/%lf,PERORG_RMB=(PERORG*PERBASE*RMB_SDR_RATE/1000)/%lf where SOURCE_ID='IISM' ",mrate_cycle,total_base,total_base);
		stmt.setSQLString(sql);
		stmt.execute();
		

	
		//�������ñ�� D_IOCC_SHARE_RESULT_201310  ���Ŷ�Ӧ��D_SMS_ACCOUNT_201310��ȡ�����ܶ� roamtypeΪ4 ���ܶ�,���ɵ�̯�ֽ�������д� IOSM������	
		theJSLog<<"��������Դ IOSM ....."<<endi;
		//�����ܷ���
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(SETTLE_FEE_SDR) from D_SMS_ACCOUNT_%s where ROAM_TYPE = 4 ",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>sms_total_fee;
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(city_settle_fee) from D_IOCC_SHARE_RESULT_%s where province_id <> 199 ",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>total_base;
		
		theJSLog<<"sms_total_fee="<<sms_total_fee<<"  total_base="<<total_base<<endi;
		//��̨����-----------------------------------------------------------
		theJSLog<<"�����̨����"<<endi;
	
		//�����ʡ����ռ����
/*		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(count(1))from D_IOCC_SHARE_RESULT_%s where province_id <> 199  and visit_country_id  in('9','37') group by province_id order by province_id ",mrate_cycle);
		cout<<"ʡ�ݸ���sql:"<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>prov_cnt;  //�����ʡ�ݸ���
		theJSLog<<"IOCC ʡ�ݸ���:"<<prov_cnt<<endi;
*/		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into D_SMS_SETTLE_%s(KMT_FLAG,SOURCE_ID,PROVINCE_ID,PERCENT,PERORG,PERORG_RMB,PERBASE,RMB_SDR_RATE) " 
					"select 1,'IOSM',province_id,sum(city_settle_fee)/%lf,sum(city_settle_fee),sum(city_settle_fee),%lf,RMB_SDR_RATE from D_IOCC_SHARE_RESULT_%s where province_id <> 199 and visit_country_id  in('9','37') group by province_id,RMB_SDR_RATE order by province_id ",mrate_cycle,total_base,sms_total_fee,mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();
		
		
		//��������-----------------------------------------------------------------
		theJSLog<<"�����������"<<endi;

		//�����ʡ����ռ����
		memset(sql,0,sizeof(sql));
/*		sprintf(sql,"select count(count(1))from D_IOCC_SHARE_RESULT_%s where province_id <> 199  and visit_country_id not in('9','37') group by province_id order by province_id ",mrate_cycle);
		cout<<"ʡ�ݸ���sql:"<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>prov_cnt;  //�����ʡ�ݸ���
		theJSLog<<"IOCC ʡ�ݸ���:"<<prov_cnt<<endi;
*/		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into D_SMS_SETTLE_%s(KMT_FLAG,SOURCE_ID,PROVINCE_ID,PERCENT,PERORG,PERORG_RMB,PERBASE,RMB_SDR_RATE) " 
					"select 0,'IOSM',province_id,sum(city_settle_fee)/%lf,sum(city_settle_fee),sum(city_settle_fee),%lf,RMB_SDR_RATE from D_IOCC_SHARE_RESULT_%s where province_id <> 199 and visit_country_id not in('9','37') group by province_id,RMB_SDR_RATE order by province_id ",mrate_cycle,total_base,sms_total_fee,mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_SMS_SETTLE_%s set PERORG=PERORG*PERBASE/%lf,PERORG_RMB=(PERORG*PERBASE*RMB_SDR_RATE/1000)/%lf where SOURCE_ID='IOSM' ",mrate_cycle,total_base,total_base);
		stmt.setSQLString(sql);
		stmt.execute();
		

		//���ų��ñ�� D_IOCM_SHARE_RESULT_201310  ���Ŷ�Ӧ��D_SMS_ACCOUNT_201310��ȡ�����ܶ� roamtypeΪ6 ���ܶ�,���ɵ�̯�ֽ�������д� IOSM������	
		theJSLog<<"��������Դ IICM ....."<<endi;
		//�����ܷ���
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(SETTLE_FEE_SDR) from D_SMS_ACCOUNT_%s where ROAM_TYPE = 6 ",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>sms_total_fee;
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(city_settle_fee) from D_IOCM_SHARE_RESULT_%s ",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>total_base;
		
		theJSLog<<"sms_total_fee="<<sms_total_fee<<"  total_base="<<total_base<<endi;
		//��̨����-----------------------------------------------------------
		theJSLog<<"�����������"<<endi;
	
		//�����ʡ����ռ����
/*		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(count(1))from D_IOCC_SHARE_RESULT_%s where province_id <> 199  and visit_country_id  in('9','37') group by province_id order by province_id ",mrate_cycle);
		cout<<"ʡ�ݸ���sql:"<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>prov_cnt;  //�����ʡ�ݸ���
		theJSLog<<"IOCC ʡ�ݸ���:"<<prov_cnt<<endi;
*/		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into D_SMS_SETTLE_%s(KMT_FLAG,SOURCE_ID,PROVINCE_ID,PERCENT,PERORG,PERORG_RMB,PERBASE,RMB_SDR_RATE) " 
					"select 2,'IOCM',province_id,sum(city_settle_fee)/%lf,sum(city_settle_fee),sum(city_settle_fee),%lf,RMB_SDR_RATE from D_IOCM_SHARE_RESULT_%s group by province_id,RMB_SDR_RATE order by province_id ",mrate_cycle,total_base,sms_total_fee,mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();

		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_SMS_SETTLE_%s set PERORG=PERORG*PERBASE/%lf,PERORG_RMB=(PERORG*PERBASE*RMB_SDR_RATE/1000)/%lf where SOURCE_ID='IOCM' ",mrate_cycle,total_base,total_base);
		stmt.setSQLString(sql);
		stmt.execute();


		//������λС�� �����Ӿ���
		memset(sql,0,sizeof(sql));
		//sprintf(sql,"update D_SMS_SETTLE_%s set PERORG_RMB=PERBASE*PERCENT*RMB_SDR_RATE/1000 ",mrate_cycle);
		sprintf(sql,"update D_SMS_SETTLE_%s set PERORG_RMB=round(PERORG_RMB,2) ",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();

		stmt.close();

	}catch (SQLException e)
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"���ݿ����%s(%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

		return false;
	 }

	conn.close();

		
	theJSLog<<"�������...."<<endi;


	return 0;
}


