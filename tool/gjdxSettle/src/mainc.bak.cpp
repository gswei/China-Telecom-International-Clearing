//由于国际短信业务(IICM,IOCM)没有摊分省,需要借助国际语音业务(IICC,IOCC)的摊分表的各个摊分省及所占比例 
// 然后用短信业务的总费用来算出各个省的费用
// 港澳台和国际分开算

#include<iostream>
#include <vector>

#include "CF_Common.h"
#include "CF_CLogger.h"

#include "CfgParam.h"
#include<dirent.h>

using namespace tpss;  //和psutil.h对应
using namespace std;

CLog theJSLog;

char mrate_cycle[6+1],msource[6],last_date[8+1],date_path[512];
bool gbExitSig=false;

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
	cout<<"命令参数不对,国际短信摊分程序(IISM,IOSM)\n"<<endl;

	cout<<"jsgjdx_settle -t 账期    \n"<<endl;
}


//输入参数-s 数据源   -t 日期
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
				cout<<"-t 后面请跟时间"<<endl;
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
	cout<<"*     last updaye time :  2013-04-09 by  hed	 "<<endl;
	cout<<"********************************************* "<<endl;
	
	if(checkArg(argc,argv))
	{
		return -1;
	}
	
	DBConnection conn;
	char erro_msg[1024],before_date[8+1],currTime[14+1];

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

	 //判断目录是否存在
	 DIR *dirptr = NULL; 
	 if((dirptr=opendir(szLogPath)) == NULL)
	 {
		cout<<"日志目录["<<szLogPath<<"]打开失败"<<endl;	
		return false ;
	 }else closedir(dirptr);

	theJSLog.setLog(szLogPath, atoi(szLogLevel),"DX_SETTLE","GJJS", 001);
	
	setSignalPrc();

	if(!(dbConnect(conn)))
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"连接数据库失败 connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
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

		//语音来访表格 D_IICC_SHARE_RESULT_201310  短信对应表D_SMS_ACCOUNT_201310中取结算总额 roamtype为1 的总额
		
		//先删除当月账期的摊分结果表
		memset(sql,0,sizeof(sql));
		sprintf(sql,"delete  from D_SMS_SETTLE_%s ",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();

		//计算总费用
		
		theJSLog<<"处理数据源 IISM ....."<<endi;

		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(SETTLE_FEE_SDR) from D_SMS_ACCOUNT_%s where ROAM_TYPE = 1 ",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>sms_total_fee;
		
		//港台数据-----------------------------------------------------------
		theJSLog<<"计算港台数据"<<endi;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(city_settle_fee) from D_IICC_SHARE_RESULT_%s where province_id <> 199  and home_country_id  in('9','37') ",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>total_base;
		
		//计算各省级所占比例
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(count(1))from D_IICC_SHARE_RESULT_%s where province_id <> 199  and home_country_id in('9','37') group by province_id order by province_id ",mrate_cycle);
		cout<<"省份个数sql:"<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>prov_cnt;  //计算出省份个数
		theJSLog<<"IICC 省份个数:"<<prov_cnt<<endi;

		memset(sql,0,sizeof(sql));
		sprintf(sql,"select province_id,sum(city_settle_fee) from D_IICC_SHARE_RESULT_%s where province_id <> 199 and home_country_id  in('9','37') group by province_id order by province_id ",mrate_cycle);
		cout<<"各省份所占比sql:"<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		
		//先将数据保存到内存中 省份代码,省份基数
		v_prov_code.clear();
		v_prov_base.clear();
		while(stmt>>prov_code>>prov_base)
		{
			v_prov_code.push_back(prov_code);
			v_prov_base.push_back(prov_base);
			
			//cout<<"prov_code:"<<prov_code<<" prov_base:"<<prov_base<<endl;

			memset(prov_code,0,sizeof(prov_code));
			prov_base=0;
		}

		if(v_prov_code.size())
		{
			before_total_base = 0;
			before_base = 0;
			for(int i = 0;i<v_prov_code.size()-1;i++)
			{	
				cur_base = v_prov_base[i]/total_base;
				cur_base *= 100;		
				sprintf(tmp_base,"%.*lf",percent_pos,cur_base);		//保留6位小数
				
				//sprintf(m_AuditMsg,"%s%s:%s,",m_AuditMsg,v_prov_code[i],tmp_base);

				sscanf(tmp_base,"%lf",&cur_base);
				before_base += cur_base;
				before_total_base += v_prov_base[i]*sms_total_fee/total_base;
				//cout<<"省份:"<< v_prov_code[i]<<" 值:"<<v_prov_base[i]<<" 总值:"<<total_base<<"  占比:"<<tmp_base<<"::"<<cur_base<<endl;
				
				memset(sql,0,sizeof(sql));
				sprintf(sql,"insert into D_SMS_SETTLE_%s(KMT_FLAG,SOURCE_ID,PROVINCE_ID,PERCENT,PERBASE,PERORG)values(1,'%s','IISM',%s,%s,%lf,%lf)",mrate_cycle,v_prov_code[i],tmp_base,sms_total_fee,v_prov_base[i]*sms_total_fee/total_base);
				stmt.setSQLString(sql);
			
				stmt.execute();
			}
			
			cur_base = 100 - before_base;
			sprintf(tmp_base,"%.*lf",percent_pos,cur_base);

			sscanf(tmp_base,"%lf",&cur_base);
			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into D_SMS_SETTLE_%s(KMT_FLAG,SOURCE_ID,PROVINCE_ID,PERCENT,PERBASE,PERORG)values(1,'%s','IISM',%s,%s,%lf,%lf)",mrate_cycle,v_prov_code[v_prov_code.size()-1],tmp_base,sms_total_fee,v_prov_base[v_prov_base.size()-1]*sms_total_fee/total_base);
			stmt.setSQLString(sql);	
			stmt.execute();
		}
		
		//国际数据-----------------------------------------------------------------
		theJSLog<<"计算国际数据"<<endi;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(city_settle_fee) from D_IICC_SHARE_RESULT_%s where province_id <> 199  and home_country_id not in('9','37') ",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>total_base;
		
		//计算各省级所占比例
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(count(1))from D_IICC_SHARE_RESULT_%s where province_id <> 199  and home_country_id not in('9','37') group by province_id order by province_id ",mrate_cycle);
		cout<<"省份个数sql:"<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>prov_cnt;  //计算出省份个数
		theJSLog<<"IICC 省份个数:"<<prov_cnt<<endi;

		memset(sql,0,sizeof(sql));
		sprintf(sql,"select province_id,sum(city_settle_fee) from D_IICC_SHARE_RESULT_%s where province_id <> 199 and home_country_id not in('9','37') group by province_id order by province_id ",mrate_cycle);
		cout<<"各省份所占比sql:"<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		
		//先将数据保存到内存中 省份代码,省份基数
		v_prov_code.clear();
		v_prov_base.clear();
		while(stmt>>prov_code>>prov_base)
		{
			v_prov_code.push_back(prov_code);
			v_prov_base.push_back(prov_base);
			
			//cout<<"prov_code:"<<prov_code<<" prov_base:"<<prov_base<<endl;

			memset(prov_code,0,sizeof(prov_code));
			prov_base=0;
		}

		if(v_prov_code.size())
		{
			before_total_base = 0;
			before_base = 0;
			for(int i = 0;i<v_prov_code.size()-1;i++)
			{	
				cur_base = v_prov_base[i]/total_base;
				cur_base *= 100;		
				sprintf(tmp_base,"%.*lf",percent_pos,cur_base);		//保留6位小数
				
				//sprintf(m_AuditMsg,"%s%s:%s,",m_AuditMsg,v_prov_code[i],tmp_base);

				sscanf(tmp_base,"%lf",&cur_base);
				before_base += cur_base;
				before_total_base += v_prov_base[i]*sms_total_fee/total_base;
				//cout<<"省份:"<< v_prov_code[i]<<" 值:"<<v_prov_base[i]<<" 总值:"<<total_base<<"  占比:"<<tmp_base<<"::"<<cur_base<<endl;
				
				memset(sql,0,sizeof(sql));
				sprintf(sql,"insert into D_SMS_SETTLE_%s(KMT_FLAG,SOURCE_ID,PROVINCE_ID,PERCENT,PERBASE,PERORG)values(0,'%s','IISM',%s,%s,%lf,%lf)",mrate_cycle,v_prov_code[i],tmp_base,sms_total_fee,v_prov_base[i]*sms_total_fee/total_base);
				stmt.setSQLString(sql);
			
				stmt.execute();
			}
			
			cur_base = 100 - before_base;
			sprintf(tmp_base,"%.*lf",percent_pos,cur_base);

			sscanf(tmp_base,"%lf",&cur_base);
			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into D_SMS_SETTLE_%s(KMT_FLAG,SOURCE_ID,PROVINCE_ID,PERCENT,PERBASE,PERORG)values(0,'%s','IISM',%s,%s,%lf,%lf)",mrate_cycle,v_prov_code[v_prov_code.size()-1],tmp_base,sms_total_fee,v_prov_base[v_prov_base.size()-1]*sms_total_fee/total_base);
			stmt.setSQLString(sql);	
			stmt.execute();
		}

	
		//语音出访表格 D_IOCC_SHARE_RESULT_201310  短信对应表D_SMS_ACCOUNT_201310中取结算总额 roamtype为4 的总额,生成的摊分结果表名中带 IOSM和账期
		
		theJSLog<<"处理数据源 IOSM ....."<<endi;
		//计算总费用
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(SETTLE_FEE_SDR) from D_SMS_ACCOUNT_%s where ROAM_TYPE = 4 ",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>sms_total_fee;
		
		//港台数据-----------------------------------------------------------
		theJSLog<<"计算港台数据"<<endi;

		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(city_settle_fee) from D_IOCC_SHARE_RESULT_%s where province_id <> 199  and visit_country_id  in('9','37') ",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>total_base;
		
		//计算各省级所占比例
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(count(1))from D_IOCC_SHARE_RESULT_%s where province_id <> 199  and visit_country_id  in('9','37') group by province_id order by province_id ",mrate_cycle);
		cout<<"省份个数sql:"<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>prov_cnt;  //计算出省份个数
		theJSLog<<"IOCC 省份个数:"<<prov_cnt<<endi;

		memset(sql,0,sizeof(sql));
		sprintf(sql,"select province_id,sum(city_settle_fee) from D_IOCC_SHARE_RESULT_%s where province_id <> 199  and visit_country_id  in('9','37') group by province_id order by province_id ",mrate_cycle);
		cout<<"各省份所占比sql:"<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		
		//先将数据保存到内存中 省份代码,省份基数
		v_prov_code.clear();
		v_prov_base.clear();
		while(stmt>>prov_code>>prov_base)
		{
			v_prov_code.push_back(prov_code);
			v_prov_base.push_back(prov_base);
			
			//cout<<"prov_code:"<<prov_code<<" prov_base:"<<prov_base<<endl;

			memset(prov_code,0,sizeof(prov_code));
			prov_base=0;
		}
		
		if(v_prov_code.size())
		{
			before_total_base = 0;
			before_base = 0;
			for(int i = 0;i<v_prov_code.size()-1;i++)
			{	
				cur_base = v_prov_base[i]/total_base;
				cur_base *= 100;		
				sprintf(tmp_base,"%.*lf",percent_pos,cur_base);		//保留6位小数
				
				//sprintf(m_AuditMsg,"%s%s:%s,",m_AuditMsg,v_prov_code[i],tmp_base);

				sscanf(tmp_base,"%lf",&cur_base);
				before_base += cur_base;
				before_total_base += v_prov_base[i]*sms_total_fee/total_base;

				//cout<<"省份:"<< v_prov_code[i]<<" 值:"<<v_prov_base[i]<<" 总值:"<<total_base<<"  占比:"<<tmp_base<<"::"<<cur_base<<endl;
				
				memset(sql,0,sizeof(sql));
				sprintf(sql,"insert into D_SMS_SETTLE_%s(KMT_FLAG,SOURCE_ID,PROVINCE_ID,PERCENT,PERBASE,PERORG)values(1,'%s','IOSM',%s,%s,%lf,%lf)",mrate_cycle,v_prov_code[i],tmp_base,sms_total_fee,v_prov_base[i]*sms_total_fee/total_base);
				stmt.setSQLString(sql);
			
				stmt.execute();
			}
			
			cur_base = 100 - before_base;
			sprintf(tmp_base,"%.*lf",percent_pos,cur_base);

			sscanf(tmp_base,"%lf",&cur_base);
			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into D_SMS_SETTLE_%s(KMT_FLAG,SOURCE_ID,PROVINCE_ID,PERCENT,PERBASE,PERORG)values(1,'%s','IOSM',%s,%s,%lf,%lf)",mrate_cycle,v_prov_code[v_prov_code.size()-1],tmp_base,sms_total_fee,sms_total_fee-before_total_base);
			stmt.setSQLString(sql);	
			stmt.execute();

		}
		
		//国际数据-----------------------------------------------------------------
		theJSLog<<"计算国际数据"<<endi;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(city_settle_fee) from D_IOCC_SHARE_RESULT_%s where province_id <> 199  and visit_country_id not in('9','37') ",mrate_cycle);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>total_base;
		
		//计算各省级所占比例
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(count(1))from D_IOCC_SHARE_RESULT_%s where province_id <> 199  and visit_country_id not in('9','37') group by province_id order by province_id ",mrate_cycle);
		cout<<"省份个数sql:"<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>prov_cnt;  //计算出省份个数
		theJSLog<<"IOCC 省份个数:"<<prov_cnt<<endi;

		memset(sql,0,sizeof(sql));
		sprintf(sql,"select province_id,sum(city_settle_fee) from D_IOCC_SHARE_RESULT_%s where province_id <> 199  and visit_country_id not in('9','37') group by province_id order by province_id ",mrate_cycle);
		cout<<"各省份所占比sql:"<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		
		//先将数据保存到内存中 省份代码,省份基数
		v_prov_code.clear();
		v_prov_base.clear();
		while(stmt>>prov_code>>prov_base)
		{
			v_prov_code.push_back(prov_code);
			v_prov_base.push_back(prov_base);
			
			//cout<<"prov_code:"<<prov_code<<" prov_base:"<<prov_base<<endl;

			memset(prov_code,0,sizeof(prov_code));
			prov_base=0;
		}
		
		if(v_prov_code.size())
		{
			before_total_base = 0;
			before_base = 0;
			for(int i = 0;i<v_prov_code.size()-1;i++)
			{	
				cur_base = v_prov_base[i]/total_base;
				cur_base *= 100;		
				sprintf(tmp_base,"%.*lf",percent_pos,cur_base);		//保留6位小数
				
				//sprintf(m_AuditMsg,"%s%s:%s,",m_AuditMsg,v_prov_code[i],tmp_base);

				sscanf(tmp_base,"%lf",&cur_base);
				before_base += cur_base;
				before_total_base += v_prov_base[i]*sms_total_fee/total_base;

				//cout<<"省份:"<< v_prov_code[i]<<" 值:"<<v_prov_base[i]<<" 总值:"<<total_base<<"  占比:"<<tmp_base<<"::"<<cur_base<<endl;
				
				memset(sql,0,sizeof(sql));
				sprintf(sql,"insert into D_SMS_SETTLE_%s(KMT_FLAG,SOURCE_ID,PROVINCE_ID,PERCENT,PERBASE,PERORG)values(0,'%s','IOSM',%s,%s,%lf,%lf)",mrate_cycle,v_prov_code[i],tmp_base,sms_total_fee,v_prov_base[i]*sms_total_fee/total_base);
				stmt.setSQLString(sql);
			
				stmt.execute();
			}
			
			cur_base = 100 - before_base;
			sprintf(tmp_base,"%.*lf",percent_pos,cur_base);

			sscanf(tmp_base,"%lf",&cur_base);
			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into D_SMS_SETTLE_%s(KMT_FLAG,SOURCE_ID,PROVINCE_ID,PERCENT,PERBASE,PERORG)values(0,'%s','IOSM',%s,%s,%lf,%lf)",mrate_cycle,v_prov_code[v_prov_code.size()-1],tmp_base,sms_total_fee,sms_total_fee-before_total_base);
			stmt.setSQLString(sql);	
			stmt.execute();

		}

		stmt.close();

	}catch (SQLException e)
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"数据库出错：%s(%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常

		return false;
	 }

	conn.close();

		
	theJSLog<<"处理完成...."<<endi;


	return 0;
}


