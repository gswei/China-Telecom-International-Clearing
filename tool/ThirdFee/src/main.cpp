//�����������,CTOG��CTOC�ĵ���������ѷֿ���,�ҷֱ������ű�������
//2014-05-15 Ϊ��֤ƽ�⽨�������̯�ֵ��㶫��̨������ȥ

#include<iostream>
#include<fstream>
#include<string>
#include <vector>

#include "CF_Common.h"
#include "CF_CLogger.h"

#include "CfgParam.h"
#include<dirent.h>

using namespace tpss;  //��psutil.h��Ӧ
using namespace std;

CLog theJSLog;

char mrate_cycle[6+1],mlast_rate_cycle[6+1],msource[6],last_date[8+1],date_path[512];
bool gbExitSig=false;

struct ThirdFee{
	string strRateCycle;
	string strThirdFee;
	string strCurrency;
	string strFeeType;
	string strCarrierId;
	string strStaffId;
	
	ThirdFee(){
		strRateCycle = strThirdFee = strCurrency = "";
		strFeeType = strCarrierId = "";
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
	cout<<"�����������,���ʶ���̯�ֳ���(IISM,IOSM)\n"<<endl;

	cout<<"jsThirdFee -t ����    \n"<<endl;
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
			if(strncmp(argv[i+1]+4,"01",2) == 0)
			{
				sprintf(mlast_rate_cycle,"%d",(atoi(argv[i+1])/100-1)*100+12);
			}
			else
			{
				sprintf(mlast_rate_cycle,"%d",atoi(argv[i+1])-1);
			}
		}
		
	}
	
	theJSLog<<"mrate_cycle="<<mrate_cycle<<" mlast_rate_cycle="<<mlast_rate_cycle<<endi;

	if(!one_flag)
	{
		printV();

		return -1;
	}
	
	//ret=-1;
	return ret;
}


/***********add by zhangj at 20150417 start************/
void split(const string& s, const string& c, vector<string>& v)
{ 
	string::size_type i = 0;
	string::size_type j = s.find(c); 
	while(j != s.npos){ 
		v.push_back(s.substr(i, j-i));
		//theJSLog<<"..."<<s.substr(i, j-i)<<endi;
		i = j = j + c.size();
		j = s.find(c, j);
	}
	if(i > 0){
		v.push_back(s.substr(i, s.size()-1-i));
		//theJSLog<<s.substr(i,s.size()-1-i)<<endi;
	}
	return;
}

int getThirdFee(string inFPathName, char* ratecycle, vector<ThirdFee>& vThirdFee)
{
	ifstream infile;
	infile.open(inFPathName.c_str(),ios::in);
	if(!infile)
	{
		theJSLog<<"���ļ�ʧ��"<<inFPathName<<"  :"<<strerror(errno)<<endw;
		return -1;
	}
	string rcd,tmpType;
	vThirdFee.clear();
	int cnt = 0,cntType = 0;
	while(getline(infile,rcd))
	{
		ThirdFee thirdFee;
		vector<string> vrcd;
		split(rcd,"@@||@@",vrcd);
		//theJSLog<<"split"<<endi;
		
		if(vrcd[0] != ratecycle){
			continue;
		}
		
		if("C2G" != vrcd[8].substr(0,3) && "SYN" != vrcd[8].substr(0,3) ){
			theJSLog<<"��Ч��������ͣ�"<<vrcd[8].substr(0,3)<<endw;
		}
		if(cnt == 0){
			tmpType = vrcd[8].substr(0,3);
		}else if(tmpType != vrcd[8].substr(0,3) ){
			cntType = 2;
		}
		
		thirdFee.strRateCycle = vrcd[0];
		thirdFee.strThirdFee  = vrcd[1];
		thirdFee.strCurrency  = vrcd[3];
		thirdFee.strFeeType   = vrcd[8].substr(0,3);
		thirdFee.strCarrierId = vrcd[8].substr(4,5);
		thirdFee.strStaffId   = vrcd[9];
		
		//theJSLog<<thirdFee.strRateCycle<<thirdFee.strThirdFee<<thirdFee.strCurrency<<thirdFee.strFeeType<<thirdFee.strCarrierId<<thirdFee.strStaffId<<endi;
		vThirdFee.push_back(thirdFee);
		
		cnt++;
	}
	
	if(cnt == 0)
		return 0;
	if(cntType == 2)
		return 2;
	else
		return 1;
}
/***********add by zhangj at 20150417   end************/


int main(int argc,char** argv)
{
	cout<<"********************************************* "<<endl;
	cout<<"*    China Telecom. Telephone Network         "<<endl;
	cout<<"*    InterNational Account Settle System      "<<endl;
	cout<<"*                                             "<<endl;
	cout<<"*           jsgjdx_settle					 "<<endl;
	cout<<"*           sys.GJZW.Version 1.0	             "<<endl;
	cout<<"*     created time :      2014-05-13 by  hed	 "<<endl;
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

	theJSLog.setLog(szLogPath, atoi(szLogLevel),"ThridFee","GJJS", 001);
	
	setSignalPrc();

	if(!(dbConnect(conn)))
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"�������ݿ�ʧ�� connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
		return false ;
	}
	
	Statement stmt;
	char sql[4096];
	

	try
	{		
		stmt = conn.createStatement();

		/***********add by zhangj at 20150417 start************/
		char inFPathName[512],inFPath[256];
		memset(inFPath,0,sizeof(inFPath));
		sprintf(sql,"select a.varvalue from c_global_env a where a.varname='THIRDFEE_IN_PATH'");
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>inFPath;

		theJSLog<<"THIRDFEE_IN_PATH="<<inFPath<<endi;

		memset(inFPathName,0,sizeof(inFPathName));
		sprintf(inFPathName,"%sCT_ACCT_PEER_SYNIVERSE_ACCOUNT.%s.dat",inFPath,mlast_rate_cycle);

		vector<ThirdFee> vThirdFee;
		int ret = getThirdFee(inFPathName,mlast_rate_cycle,vThirdFee);
    //theJSLog<<"getThirdFee:ret="<<ret<<endi;
    if(ret == 0){
    	theJSLog<<"ERROR���ӿ��ļ�"<<inFPathName<<"  :"<<"����Ч��¼"<<endw;
    	return false;
    }else if(ret == 1){
    	theJSLog<<"ERROR���ӿ��ļ�"<<inFPathName<<"  :"<<"ֻ��һ�ַ��������"<<endw;
    	return false;
    }else if(ret == 2){
    	//��ɾ���������ڵĵ����������
    	memset(sql,0,sizeof(sql));
    	sprintf(sql,"delete from D_THIRDFEE_IN_DATA where ratecycle=%s ",mrate_cycle);
    	stmt.setSQLString(sql);
    	stmt.execute();

			for(int i=0;i<vThirdFee.size();i++){
				memset(sql,0,sizeof(sql));
    		sprintf(sql,"insert into D_THIRDFEE_IN_DATA (ratecycle,carrier_id,carrier_name,fee_type,"
    		    "usd_fee,fee,currency,rmb_fee,rmb_sdr_rate,rate_date,staff_id,oper_date,id,state) "
    		    "select %s,'%s',a.cust_name,decode('%s','C2G',1,0),0,%s/1000,'%s',"
    		    "0,0,'','%s',sysdate,seq_thirdfee_in_data_ID.nextval,0 "
    		    "from stt_object a,carrier_code b "
            "where a.cust_id=b.cust_id and b.carrier_code='%s'",
             mrate_cycle,vThirdFee[i].strCarrierId,vThirdFee[i].strFeeType,vThirdFee[i].strThirdFee,
             vThirdFee[i].strCurrency,vThirdFee[i].strStaffId,vThirdFee[i].strCarrierId);
    		stmt.setSQLString(sql);
    		stmt.execute();
        //theJSLog<<sql<<endi;
			}
    }
		memset(sql,0,sizeof(sql));
    sprintf(sql,"update D_THIRDFEE_IN_DATA r set (r.rmb_fee,r.rmb_sdr_rate,r.rate_date,r.state)=("
    		    "select r.fee*s.rate,s.rate,to_date(s.rate_date,'yyyymmdd'),1 "
    		    "from exchange_rate s "
            "where s.rate_date=to_char(add_months(to_date('%s'||'10','yyyymmdd'),-2),'yyyymmdd') "
            "and r.currency=s.src_currency and s.dest_currency='RMB') "
          "where exists (select 1 from exchange_rate v "
            "where v.rate_date=to_char(add_months(to_date('%s'||'10','yyyymmdd'),-2),'yyyymmdd') "
            "and r.currency=v.src_currency and v.dest_currency='RMB') "
          "and r.ratecycle=%s and r.state=0",
           mrate_cycle,mrate_cycle,mrate_cycle);
    stmt.setSQLString(sql);
    stmt.execute();
    
    memset(sql,0,sizeof(sql));
    sprintf(sql,"update D_THIRDFEE_IN_DATA r set usd_fee=decode(r.currency,'USD',r.fee,"
    		    "(select r.rmb_fee*s.rate from exchange_rate s "
            "where s.rate_date=to_char(add_months(to_date('%s'||'10','yyyymmdd'),-2),'yyyymmdd') "
            "and s.src_currency='RMB' and s.dest_currency='USD')) "
          "where (exists (select 1 from exchange_rate v "
            "where v.rate_date=to_char(add_months(to_date('%s'||'10','yyyymmdd'),-2),'yyyymmdd') "
            "and v.src_currency='RMB' and v.dest_currency='USD') or r.currency='USD')"
          "and r.ratecycle=%s and r.state=1",
           mrate_cycle,mrate_cycle,mrate_cycle);
    stmt.setSQLString(sql);
    stmt.execute();
  
		int cnt = 0;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(1) from D_THIRDFEE_IN_DATA where RATECYCLE = %s",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>cnt;
		theJSLog<<"D_THIRDFEE_IN_DATA:"<<cnt<<endi;
		if(cnt = 0){
			theJSLog<<"ERROR���޷���ѽӿ��ļ�"<<endw;
    	return false;
		}

		cnt = 0;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(1) from D_THIRDFEE_IN_DATA where RATECYCLE = %s and state = 0",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>cnt;
		if(cnt > 0){
			char mvalue[4];
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select distinct currency from D_THIRDFEE_IN_DATA where RATECYCLE = %s and state = 0",mrate_cycle);
			stmt.setSQLString(sql);
			stmt.execute();

			while(stmt>>mvalue){
				theJSLog<<"ERROR�����ʣ�"<<mvalue<<" To RMB ����10��δ����"<<endw;
			}
    	return false;
		}
		
		/***********add by zhangj at 20150417   end************/

		
		//��ɾ���������ڵĵ����������
		memset(sql,0,sizeof(sql));
		sprintf(sql,"delete  from D_THIRDFEE_OUT_DATA_%s ",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();

		//�����ܷ��� CTOC CTOG ǰ̨¼��
		double ctoc_total_fee = 0,ctog_total_fee = 0;
		theJSLog<<"���������������ܷ��� CTOC CTOG ǰ̨¼��"<<endi;

		memset(sql,0,sizeof(sql));
		sprintf(sql,"select round(sum(RMB_FEE),2) from D_THIRDFEE_IN_DATA where RATECYCLE = %s and fee_type = 0 ",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>ctoc_total_fee;	
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select round(sum(RMB_FEE),2) from D_THIRDFEE_IN_DATA where RATECYCLE = %s and fee_type = 1 ",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>ctog_total_fee;

		theJSLog<<"ctoc_total_fee="<<ctoc_total_fee<<" ctog_total_fee="<<ctog_total_fee<<endi;
		
		//�����̨���� �� ���������� CTOG���������ܺ�
		//sprintf(mlast_rate_cycle,"%s",mrate_cycle);
		
		//�������α� #################################################################### 
		
		//CTOG
		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into D_THIRDFEE_OUT_DATA_%s (ratecycle,kmt_flag,ctoc_ctog_flag,prov_name,prov_id,settle_fee) "
					"select %s,0,1,c.prov_name,c.prov_id,sum(cg_jszc) from ("
					"select b.prov_name as prov_name,round(sum(settle_fee),2) as cg_jszc, b.prov_id as prov_id from D_IOCG_SHARE_RESULT_%s a,TP_PROVINCE b where a.province_id=b.prov_id and province_id<>199 " 
					"and a.visitcarriercode not in(select distinct carrier_cd from CTOG_CARRIER where substr(carrier_cd,1,3) in "
					"(select ciber_ctry_cd from ctry WHERE ctry_id in('9','37')) and exp_date > sysdate ) group by b.prov_name,b.prov_id "
					"union all "
					"select b.prov_name as prov_name,round(sum(settle_fee),2) as cg_jszc ,b.prov_id as prov_id from D_IXCG_SHARE_RESULT_%s a,TP_PROVINCE b where a.province_id=b.prov_id and province_id<>199 "
					 "and a.settle_carrier_id not in(select distinct carrier_cd from CTOG_CARRIER where substr(carrier_cd,1,3) in "
					"(select ciber_ctry_cd from ctry WHERE ctry_id in('9','37')) and exp_date > sysdate ) group by b.prov_name,b.prov_id "
					"union all "
					"select b.prov_name as prov_name,round(sum(settle_fee),2) as cg_jszc, b.prov_id as prov_id from D_IMCG_SHARE_RESULT_%s a,TP_PROVINCE b where a.province_id=b.prov_id and province_id<>199 "
					"and a.visitcarriercode not in(select distinct carrier_cd from CTOG_CARRIER where substr(carrier_cd,1,3) in "
                    "(select ciber_ctry_cd from ctry WHERE ctry_id in('9','37')) and exp_date > sysdate) group by b.prov_name,b.prov_id "		
					") c group by c.prov_name,c.prov_id",mrate_cycle,mrate_cycle,mlast_rate_cycle,mlast_rate_cycle,mlast_rate_cycle);
		
		cout<<"��������CTOG sql="<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();

		// CTOC
		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into D_THIRDFEE_OUT_DATA_%s (ratecycle,kmt_flag,ctoc_ctog_flag,prov_name,prov_id,settle_fee) "
					"select %s,0,0,c.prov_name,c.prov_id,sum(cc_jssrzc) from ("
					"select round(sum(a.city_settle_fee),2) as cc_jssrzc,b.prov_name as prov_name,b.prov_id as prov_id from D_IICC_SHARE_RESULT_%s a,TP_PROVINCE b where a.settle_month='%s' "
					"and a.home_country_id not in(37,9) and a.province_id=b.prov_id and a.visit_zone_code !=199 group by b.prov_name,b.prov_id  "
					"union all "
					"select round(sum(a.city_settle_fee),2) as cc_jssrzc,b.prov_name as prov_name,b.prov_id as prov_id from D_IIXC_SHARE_RESULT_%s a,TP_PROVINCE b "
					"where a.SETTLE_CARRIER_ID not in(7463,7566,7455) and a.province_id = b.prov_id group by b.prov_name,b.prov_id  "
					"union all "
					"select round(perorg_rmb,2) as cc_jssrzc ,b.prov_name as prov_name,b.prov_id as prov_id from D_SMS_SETTLE_%s a,TP_PROVINCE b "
					"where a.province_id=b.prov_id and kmt_flag =0 and source_id ='IISM' "		
					"union all "
					"select round(sum(a.city_settle_fee),2) as cc_jssrzc,b.prov_name as prov_name,b.prov_id as prov_id from D_IOCC_SHARE_RESULT_%s a,TP_PROVINCE b  where a.settle_month='%s' "
					"and a.VISIT_country_id not in(37,9) and a.province_id=b.prov_id and a.home_zone_code !=199 group by b.prov_name,b.prov_id  "
					"union all "
					"select round(sum(a.city_settle_fee),2) as cc_jssrzc,b.prov_name as prov_name,b.prov_id as prov_id from D_IOXC_SHARE_RESULT_%s a,TP_PROVINCE b "
					"where a.SETTLE_CARRIER_ID not in(7463,7566,7455) and a.province_id = b.prov_id group by b.prov_name,b.prov_id  "
					"union all "
					"select round(perorg_rmb,2) as cc_jssrzc,b.prov_name as prov_name,b.prov_id as prov_id from D_SMS_SETTLE_%s a,TP_PROVINCE b "
					"where a.province_id=b.prov_id and kmt_flag =0 and source_id ='IOSM' "
					") c group by c.prov_name,c.prov_id",mrate_cycle,mrate_cycle,mrate_cycle,mrate_cycle,mrate_cycle,mrate_cycle,mrate_cycle,mrate_cycle,mrate_cycle,mrate_cycle);
		cout<<"��������CTOC sql="<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();


		//��̨���α� ##################################################################
		//CTOG  
		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into D_THIRDFEE_OUT_DATA_%s (ratecycle,kmt_flag,ctoc_ctog_flag,prov_name,prov_id,settle_fee) "
					"select %s,1,1,c.prov_name,c.prov_id,sum(cg_jszc) from ("
					"select b.prov_name as prov_name, round(sum(settle_fee),2) as cg_jszc, b.prov_id as prov_id from D_IOCG_SHARE_RESULT_%s a, TP_PROVINCE b where a.province_id = b.prov_id and province_id <> 199 "
					"and a.visitcarriercode in(select distinct carrier_cd from CTOG_CARRIER where substr(carrier_cd, 1, 3) in "
                    "(select ciber_ctry_cd from ctry WHERE ctry_id in ('9', '37')) and exp_date > sysdate) group by b.prov_name, b.prov_id "
					"union all "
					"select b.prov_name as prov_name, round(sum(settle_fee),2) as cg_jszc, b.prov_id as prov_id from D_IXCG_SHARE_RESULT_%s a, TP_PROVINCE b where a.province_id = b.prov_id and province_id <> 199 "
					"and a.settle_carrier_id in (select distinct carrier_cd from CTOG_CARRIER where substr(carrier_cd, 1, 3) in "
                    "(select ciber_ctry_cd from ctry WHERE ctry_id in ('9', '37')) and exp_date > sysdate) group by b.prov_name, b.prov_id "
					"union all "
					"select b.prov_name as prov_name, round(sum(settle_fee),2) as cg_jszc ,b.prov_id as prov_id from D_IMCG_SHARE_RESULT_%s a, TP_PROVINCE b where a.province_id = b.prov_id and province_id <> 199 "
					"and a.visitcarriercode in (select distinct carrier_cd from CTOG_CARRIER where substr(carrier_cd, 1, 3) in "
                    "(select ciber_ctry_cd from ctry WHERE ctry_id in ('9', '37')) and exp_date > sysdate) group by b.prov_name, b.prov_id "	
					") c group by c.prov_name,c.prov_id",mrate_cycle,mrate_cycle,mlast_rate_cycle,mlast_rate_cycle,mlast_rate_cycle);
		cout<<"��̨����CTOG sql="<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		

		//CTOC 
		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into D_THIRDFEE_OUT_DATA_%s (ratecycle,kmt_flag,ctoc_ctog_flag,prov_name,prov_id,settle_fee) "
					"select %s,1,0,c.prov_name,c.prov_id,sum(cg_jssrzc) from ("
					"select round(sum(a.city_settle_fee),2) as cg_jssrzc,b.prov_name as prov_name,b.prov_id as prov_id from D_IICC_SHARE_RESULT_%s a,TP_PROVINCE b where a.settle_month = '%s' "
					"and a.home_country_id in(37,9) and a.province_id = b.prov_id and a.visit_zone_code != 199 group by b.prov_name, b.prov_id  "
					"union all "
					"select round(sum(a.city_settle_fee),2) as cg_jssrzc,b.prov_name as prov_name,b.prov_id as prov_id from D_IIXC_SHARE_RESULT_%s a, TP_PROVINCE b "
					"where a.SETTLE_CARRIER_ID in (7455, 7566) and a.province_id = b.prov_id group by b.prov_name, b.prov_id  "
					"union all "
					"select round(perorg_rmb,2) as cg_jssrzc,b.prov_name as prov_name,b.prov_id as prov_id from D_SMS_SETTLE_%s a,TP_PROVINCE b "
					"where a.province_id=b.prov_id and kmt_flag =1 and source_id ='IISM' "		
					"union all "
					"select round(sum(a.city_settle_fee),2) as cg_jssrzc,b.prov_name as prov_name,b.prov_id as prov_id from D_IOCC_SHARE_RESULT_%s a, TP_PROVINCE b where a.settle_month = '%s' "
					"and a.VISIT_country_id in(37, 9) and a.province_id = b.prov_id and a.home_zone_code != 199 group by b.prov_name, b.prov_id "
					"union all "
					"select round(sum(a.city_settle_fee),2) as cg_jssrzc, b.prov_name as prov_name, b.prov_id as prov_id from D_IOXC_SHARE_RESULT_%s a, TP_PROVINCE b "
					"where a.SETTLE_CARRIER_ID in (7455, 7566) and a.province_id = b.prov_id group by b.prov_name, b.prov_id "
					"union all "
					"select round(perorg_rmb,2) as cg_jssrzc,b.prov_name as prov_name,b.prov_id as prov_id from D_SMS_SETTLE_%s a,TP_PROVINCE b "
					"where a.province_id=b.prov_id and kmt_flag =1 and source_id ='IOSM' "		
					") c group by c.prov_name,c.prov_id",mrate_cycle,mrate_cycle,mrate_cycle,mrate_cycle,mrate_cycle,mrate_cycle,mrate_cycle,mrate_cycle,mrate_cycle,mrate_cycle);
		cout<<"��̨����CTOC sql="<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();

		
		//���ʱ����CTOC CTOG����
		double gj_cc_total_fee=0,gj_cg_total_fee=0,gt_cc_total_fee,gt_cg_total_fee=0;
		theJSLog<<"���� ��̨�͹��ʱ�����CTOC ����֧��֮��, CTOG֧��֮��"<<endi;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(a.settle_fee) from D_THIRDFEE_OUT_DATA_%s a where a.kmt_flag=0 and ctoc_ctog_flag=0 ",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>gj_cc_total_fee;

		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(a.settle_fee) from D_THIRDFEE_OUT_DATA_%s a where a.kmt_flag=0 and ctoc_ctog_flag=1 ",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>gj_cg_total_fee;
		
		//��̨�����CTOC CTOG�ܷ���
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(a.settle_fee) from D_THIRDFEE_OUT_DATA_%s a where a.kmt_flag=1 and ctoc_ctog_flag=0 ",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>gt_cc_total_fee;

		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(a.settle_fee) from D_THIRDFEE_OUT_DATA_%s a where a.kmt_flag=1 and ctoc_ctog_flag=1 ",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>gt_cg_total_fee;
		
		theJSLog<<"gj_cc_total_fee="<<gj_cc_total_fee<<" gj_cg_total_fee="<<gj_cg_total_fee<<" gt_cc_total_fee="<<gt_cc_total_fee<<" gt_cg_total_fee="<<gt_cg_total_fee<<endi;


		//�������ű����ܵ�CTOC ��CTOG���Ե�ռ��
		double gj_cc_total_fee_t=0,gj_cg_total_fee_t=0,gt_cc_total_fee_t=0,gt_cg_total_fee_t=0;

		theJSLog<<"������ʸ�̨ ����������� CTOC��CTOGռ�ȷ���"<<endi;
		gj_cc_total_fee_t=gj_cc_total_fee*ctoc_total_fee/(gj_cc_total_fee+gt_cc_total_fee);
		gj_cg_total_fee_t=gj_cg_total_fee*ctog_total_fee/(gj_cg_total_fee+gt_cg_total_fee);
		
		gt_cc_total_fee_t = ctoc_total_fee-gj_cc_total_fee_t;
		gt_cg_total_fee_t = ctog_total_fee-gj_cg_total_fee_t;

		theJSLog<<"gj_cc_total_fee_t="<<gj_cc_total_fee_t<<" gj_cg_total_fee_t="<<gj_cg_total_fee_t<<" gt_cc_total_fee_t="<<gt_cc_total_fee_t<<" gt_cg_total_fee_t="<<gt_cg_total_fee_t<<endi;


		//�����ʡ�ڸ�����̨�͹����������� �ֱ�����������

		//���ʱ���
		theJSLog<<"������ʱ����ʡ�����������"<<endi;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_THIRDFEE_OUT_DATA_%s  a set settle_fee=round(settle_fee*%lf/%lf,2) where a.kmt_flag=0 and ctoc_ctog_flag=0 "
					,mrate_cycle,gj_cc_total_fee_t,gj_cc_total_fee);
		stmt.setSQLString(sql);
		stmt.execute();
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_THIRDFEE_OUT_DATA_%s  a set settle_fee=round(settle_fee*%lf/%lf,2) where a.kmt_flag=0 and ctoc_ctog_flag=1 "
					,mrate_cycle,gj_cg_total_fee_t,gj_cg_total_fee);
		stmt.setSQLString(sql);
		stmt.execute();

		
		//��̨����
		theJSLog<<"�����̨�����ʡ�����������"<<endi;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_THIRDFEE_OUT_DATA_%s a set settle_fee=round(settle_fee*%lf/%lf,2) where a.kmt_flag=1 and ctoc_ctog_flag=0 "
					,mrate_cycle,gt_cc_total_fee_t,gt_cc_total_fee);
		stmt.setSQLString(sql);
		stmt.execute();
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_THIRDFEE_OUT_DATA_%s a set settle_fee=round(settle_fee*%lf/%lf,2) where a.kmt_flag=1 and ctoc_ctog_flag=1 "
					,mrate_cycle,gt_cg_total_fee_t,gt_cg_total_fee);
		stmt.setSQLString(sql);
		stmt.execute();

		//Ϊ��֤�������ƽ�� ����̨�㶫CTOC���в�����
		double out_third_total_fee=0,in_third_total_fee =ctoc_total_fee+ctog_total_fee;

		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(a.settle_fee) from d_thirdfee_out_data_%s a ",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>out_third_total_fee;

		theJSLog<<"������:"<<(in_third_total_fee-out_third_total_fee)<<endi;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update d_thirdfee_out_data_%s a set a.SETTLE_FEE=a.SETTLE_FEE+round(%lf,2) "
					"where a.kmt_flag=1 and a.ctoc_ctog_flag=0 and a.PROV_ID=20 ",mrate_cycle,(in_third_total_fee-out_third_total_fee));
		cout<<"������sql="<<sql<<endl;
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
