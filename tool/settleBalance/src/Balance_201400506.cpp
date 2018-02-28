#include "Balance.h"

int mj[4][3] = {{1,2,3},{4,5,6},{7,8,9},{10,11,12}};

IBC_ParamCfgMng param_cfg;

Balance::Balance()
{
	memset(ratecycle,0,sizeof(ratecycle));
	cyclePos[0] = -1;
	memset(erro_msg,0,sizeof(erro_msg));
	memset(sql,0,sizeof(sql));
}

Balance::~Balance()
{


}

void printV()
{	
	cout<<"命令格式: \n"<<endl;
	cout<<"jssettleBalance -t 账期  \n"<<endl;
}

bool Balance::init(int argc,char** argv)
{
	//获取输入参数
	int year = -1,flag = 0;
	
	//int month = 0;
	for(int i = 1;i<argc;i++)
	{
	  if(strcmp(argv[i],"-t") == 0)
	  {
		if(argc < (i+2))		
		{
			printV();
			return false;
		}
			
		strcpy(ratecycle,argv[i+1]);

		cyclePos[0] = atoi(ratecycle);
		if(cyclePos[0] == 0)
		{
			printV();
			return false ;
		}
		else if( (cyclePos[0]%100) == 1)
		{
			cyclePos[1] = (cyclePos[0]/100-1)*100+12;
			cyclePos[2] = (cyclePos[0]/100-1)*100+11;
			cyclePos[3] = (cyclePos[0]/100-1)*100+10;	
		}
		else if( (cyclePos[0]%100) == 2)
		{
			cyclePos[1] = cyclePos[0] -1;
			cyclePos[2] = (cyclePos[0]/100-1)*100+12;
			cyclePos[3] = (cyclePos[0]/100-1)*100+11;

		}
		else if( (cyclePos[0]%100) == 3)
		{
			cyclePos[1] = cyclePos[0] -1;
			cyclePos[2] = cyclePos[0] -2;
			cyclePos[3] = (cyclePos[0]/100-1)*100+12;
		}
		else
		{
			cyclePos[1] = cyclePos[0]-1;
			cyclePos[2] = cyclePos[0]-2;
			cyclePos[3] = cyclePos[0]-3;
		}

		flag = 1;
	  }
	 
	}
	
	if(flag == 0)
	{
		printV();

		return false;
	}
	
	cout<<"cyclePos[0]="<<cyclePos[0]<<"  cyclePos[1]="<<cyclePos[1]<<"  cyclePos[2]="<<cyclePos[2]<<"  cyclePos[3]="<<cyclePos[3]<<endl;
	//exit (1) ;

	//获取日志路径和级别
	if( !param_cfg.bOnInit() )		//核心参数需要自己初始化
	{
		string sErr;
		int nCodeId;
		param_cfg.getError(sErr,nCodeId);
		cerr<<"参数配置接口初始化失败！错误码="<<nCodeId<<", 错误信息="<<sErr<<endl;
		return false;
	}

	// 从核心参数里面读取日志的路径，级别，
	 char sParamName[256],szLogPath[PATH_NAME_LEN+1],szLogLevel[10],sql_path[1024];
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
	
	 //printf("xxxxx");
	 theJSLog.setLog(szLogPath, atoi(szLogLevel),"SETT_BALANCE","GJJS", 001);	//文件日志接口，调用了内存日志接口

/*	
	 if(!(dbConnect(conn)))
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init()  连接数据库失败 connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
		return false ;
	}
	
    try
    {
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select process_id from tp_module a,tp_process b where a.module_code = '%s' and a.module_id = b.module_id",module_id);
		Statement stmt = conn.createStatement();
		stmt.setSQLString(sql);
		stmt.execute();
		if(!(stmt>>module_process_id))
		{
			cout<<"模块:"<<module_id<<"找不到对应的实例ID 请在tp_process表配置"<<endl;
			return false;
		}

    }catch(util_1_0::db::SQLException e)
	{ 
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"初始化时数据库查询异常：%s(sql)",e.what(),sql);
			theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
			return false;
	}

	conn.close();
*/
	return true;
}


void Balance::run()
{
	if(!(dbConnect(conn)))
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init()  连接数据库失败 connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
		return  ;
	}
	
	Statement stmt;
	int cnt = 0;
	try
    {
		double in_adj_fee=0,in_adj_usd_fee=0,out_adj_fee=0,out_adj_usd_fee=0,rate,percent = 0;
		double in_adj_fee_tmp=0,out_adj_fee_tmp=0,in_adj_usd_fee_tmp=0,out_adj_usd_fee_tmp=0;
		double percent_tmp = 100;
		
		char condtion[256];							//2014-04-23
		memset(condtion,0,sizeof(condtion));
		if(strncmp(ratecycle+4,"04",2) == 0)
		{
			sprintf(condtion," a.RATECYCLE in (%d,%d,%d) ",cyclePos[1],cyclePos[2],cyclePos[3]);
		}
		else
		{
			sprintf(condtion," a.RATECYCLE in (%d) ",cyclePos[1]);
		}

		stmt = conn.createStatement();
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(1) from EXCHANGE_RATE  where RATE_DATE like '%s%s' and SRC_CURRENCY = 'USD' and DEST_CURRENCY = 'RMB' and SOURCE_ID = 'ISSF' ",ratecycle,"%");
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>cnt;
		if(cnt == 0)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"汇率表 EXCHANGE_RATE 没有账期[%s]的记录",ratecycle);
			theJSLog<<erro_msg<<endw;
			return ;
		}
		
		sprintf(sql,"select RATE from EXCHANGE_RATE where RATE_DATE like '%s%s' and SRC_CURRENCY = 'USD' and DEST_CURRENCY = 'RMB' and SOURCE_ID = 'ISSF' ",ratecycle,"%");
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>rate;

		theJSLog<<"当前账期["<<ratecycle<<"]美元兑人民币的汇率:"<<rate<<endi;
		
		theJSLog<<"######################调整国际固网短信,账期["<<ratecycle<<"]###########################"<<endi;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(*) from D_GWDX_ADJ_INPUT_DATA_%s a where %s ",ratecycle,condtion);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>cnt;
		if(cnt == 0)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"国际固网短信调整数据导入表 D_GWDX_ADJ_INPUT_DATA 没有账期[%s]的数据",ratecycle);
			//theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
			theJSLog<<erro_msg<<endw;
			return ;
		}
		
		//先求出美元的,然后计算出人民币
		

		sprintf(sql,"select sum(IN_ADJ_FEE),sum(OUT_ADJ_FEE) from D_GWDX_ADJ_INPUT_DATA_%s a where %s ",ratecycle,condtion);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>in_adj_usd_fee>>out_adj_usd_fee;
		in_adj_fee = in_adj_usd_fee*rate;
		out_adj_fee = out_adj_usd_fee*rate;
		theJSLog<<"in_adj_fee="<<in_adj_fee<<" out_adj_fee="<<out_adj_fee<<" in_adj_usd_fee="<<in_adj_usd_fee<<" out_adj_usd_fee="<<out_adj_usd_fee<<endi;

		memset(sql,0,sizeof(sql));
		sprintf(sql,"delete from D_GWDX_ADJ_OUTPUT_DATA_%s a where a.RATECYCLE = %s",ratecycle,ratecycle);
		theJSLog<<"删除结果表数据:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();
		
		//对于未完成签转的运营商按统一结算费率结算时存在的差异调整，2014年开始，从4月账期报表体现1-3月账期调整，
		//5月账期开始按月调整，即5月账期体现4月调整，以此类推，11月账期体现10月调整。
		//为满足财务的要求--12月完成全年调整，12月的调整量算法为：11月调整量+12月调整预估（预估算法：用11月的调整量）。
		//次年1月账期中的12月的调整量将比对原12月预估值进行差额提供
		
		//2014-04-22 占比统一用 BILL_MIN 字段衡量
		memset(sql,0,sizeof(sql));
		if(strncmp(ratecycle+4,"04",2) == 0) 
		{
			//union会合并相同的记录 所以每个表查询出来的加日期使其不被合并
			sprintf(sql,"insert into D_GWDX_ADJ_OUTPUT_DATA_%s (RATECYCLE,PROVINCE,KMT_FLAG,UNIT_BILL_MIN) "
					"select %s,b.province_id,0,sum(b.bill) from ("
					"select a.province_id,sum(a.BILL_MIN) as bill from D_GWDX_SHARE_RESULT_%d a "
					"where a.dir in(2,3) and a.callee_country_id not in ('9','24','37') "
					"group by a.province_id "
					"union all "
					"select a.province_id,sum(a.BILL_MIN) as bill from D_GWDX_SHARE_RESULT_%d a "
					"where a.dir in(2,3) and a.callee_country_id not in ('9','24','37') "
					"group by a.province_id "
					"union all "
					"select  a.province_id,sum(a.BILL_MIN) as bill from D_GWDX_SHARE_RESULT_%d a "
					"where a.dir in(2,3) and a.callee_country_id not in ('9','24','37') "
					"group by a.province_id "
					") b  group by b.province_id ",ratecycle,ratecycle,cyclePos[1],cyclePos[2],cyclePos[3]);
		}
		else //if ((strncmp(ratecycle+4,"04",2) > 0)  && (strncmp(ratecycle+4,"12",2) <= 0))
		{
			sprintf(sql,"insert into D_GWDX_ADJ_OUTPUT_DATA_%s (RATECYCLE,PROVINCE,KMT_FLAG,UNIT_BILL_MIN) "
					"select %s,b.province_id,0,sum(b.bill) from ("
					"select a.province_id,sum(a.BILL_MIN) as bill from D_GWDX_SHARE_RESULT_%d a "
					"where a.dir in(2,3) and a.callee_country_id not in ('9','24','37') "
					"group by a.province_id "
					") b  group by b.province_id ",ratecycle,ratecycle,cyclePos[1]);
		}
		
		theJSLog<<"插入国际摊分数据到结果表:"<<sql<<endi;

		stmt.setSQLString(sql);
		stmt.execute();
		
		memset(sql,0,sizeof(sql));
		if(strncmp(ratecycle+4,"04",2) == 0) 
		{
			sprintf(sql,"insert into D_GWDX_ADJ_OUTPUT_DATA_%s (RATECYCLE,PROVINCE,KMT_FLAG,UNIT_BILL_MIN) "
					"select %s,b.province_id,1,sum(b.bill) from ("
					"select a.province_id,sum(a.BILL_MIN) as bill from D_GWDX_SHARE_RESULT_%d a "
					"where dir in(2,3) and callee_country_id  in ('9','24','37') "
					"group by a.province_id "
					"union all "
					"select a.province_id,sum(a.BILL_MIN) as bill from D_GWDX_SHARE_RESULT_%d a "
					"where dir in(2,3) and callee_country_id  in ('9','24','37') "
					"group by a.province_id "
					"union all "
					"select  a.province_id,sum(a.BILL_MIN) as bill from D_GWDX_SHARE_RESULT_%d a "
					"where dir in(2,3) and callee_country_id  in ('9','24','37') "
					"group by a.province_id "
					") b  group by b.province_id ",ratecycle,ratecycle,cyclePos[1],cyclePos[2],cyclePos[3]);
		}
		else //if ((strncmp(ratecycle+4,"04",2) > 0)  && (strncmp(ratecycle+4,"12",2) <= 0))
		{
			sprintf(sql,"insert into D_GWDX_ADJ_OUTPUT_DATA_%s (RATECYCLE,PROVINCE,KMT_FLAG,UNIT_BILL_MIN) "
					"select %s,b.province_id,1,sum(b.bill) from ("
					"select a.province_id,sum(a.BILL_MIN) as bill from D_GWDX_SHARE_RESULT_%d a "
					"where dir in(2,3) and callee_country_id  in ('9','24','37') "
					"group by a.province_id "
					") b  group by b.province_id ",ratecycle,ratecycle,cyclePos[1]);
		}

		theJSLog<<"插入港澳台摊分数据到结果表:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();

		//结果表求出总时长,比例,调整费用单位时长已经算出,
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWDX_ADJ_OUTPUT_DATA_%s a set TOTAL_BILL_MIN = (select sum(UNIT_BILL_MIN) from D_GWDX_ADJ_OUTPUT_DATA_%s where RATECYCLE = %s ) "
					"where a.RATECYCLE = %s ",ratecycle,ratecycle,ratecycle,ratecycle);
		theJSLog<<"计算总时长:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();			
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWDX_ADJ_OUTPUT_DATA_%s b set b.PROV_PERCENT = b.UNIT_BILL_MIN/b.TOTAL_BILL_MIN*100 " 
					"where b.RATECYCLE = %s ",ratecycle,ratecycle);
		theJSLog<<"计算各省份所占比例:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWDX_ADJ_OUTPUT_DATA_%s c set c.IN_ADJ_FEE = c.UNIT_BILL_MIN/c.TOTAL_BILL_MIN*%lf,c.OUT_ADJ_FEE = c.UNIT_BILL_MIN/c.TOTAL_BILL_MIN*%lf,"
					"c.IN_ADJ_USD_FEE=c.UNIT_BILL_MIN/c.TOTAL_BILL_MIN*%lf,c.OUT_ADJ_USD_FEE=c.UNIT_BILL_MIN/c.TOTAL_BILL_MIN*%lf "
					"where c.RATECYCLE = %s ",ratecycle,in_adj_fee,out_adj_fee,in_adj_usd_fee,out_adj_usd_fee,ratecycle);
		theJSLog<<"计算各省份单位时长:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();	
		
		//保存100平衡,拿广东省 港台平衡数据
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(a.prov_percent),sum(a.in_adj_fee),sum(a.out_adj_fee),sum(a.in_adj_usd_fee),sum(a.out_adj_usd_fee) from D_GWDX_ADJ_OUTPUT_DATA_%s a where a.RATECYCLE = %s ",ratecycle,ratecycle);
		cout<<"sql = "<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();	
		stmt>>percent>>in_adj_fee_tmp>>out_adj_fee_tmp>>in_adj_usd_fee_tmp>>out_adj_usd_fee_tmp;
		theJSLog<<" percent="<<percent<<"  in_adj_fee_tmp="<<in_adj_fee_tmp<<" out_adj_fee_tmp="<<out_adj_fee_tmp<<" in_adj_usd_fee_tmp="<<in_adj_usd_fee_tmp<<" out_adj_usd_fee_tmp="<<out_adj_usd_fee_tmp<<endi;
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWDX_ADJ_OUTPUT_DATA_%s a set a.prov_percent=a.prov_percent+(%lf),a.in_adj_fee=a.in_adj_fee+(%lf),a.out_adj_fee=a.out_adj_fee+(%lf),"
					"a.in_adj_usd_fee=a.in_adj_usd_fee+(%lf),a.out_adj_usd_fee=a.out_adj_usd_fee+(%lf) "
					"where a.RATECYCLE = %s and a.kmt_flag = 1 and a.PROVINCE = 20 "
					,ratecycle,(percent_tmp-percent),(in_adj_fee-in_adj_fee_tmp),(out_adj_fee-out_adj_fee_tmp),(in_adj_usd_fee-in_adj_usd_fee_tmp),(out_adj_usd_fee-out_adj_usd_fee_tmp),ratecycle);
		
		theJSLog<<"保存平衡sql:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();	
		
		theJSLog<<"更新日志记录表..."<<endi;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into D_ADJUST_OPER_LOG(RATECYCLE,IN_TABNAME,OUT_TABNAME,DEAL_TIME) values("
					"%s,'%s','%s',to_char(sysdate,'yyyymmddhh24miss'))",ratecycle,"D_GWDX_ADJ_INPUT_DATA","D_GWDX_ADJ_OUTPUT_DATA");
		stmt.setSQLString(sql);
		stmt.execute();

		//######################港澳台的##################################
/*		theJSLog<<"计算港澳台..."<<endi;

		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into D_GWDX_ADJ_OUTPUT_DATA (RATECYCLE,PROVINCE,KMT_FLAG,UNIT_BILL_MIN) "
					"select %s,b.province_id,1,sum(b.bill) from ("
					"select a.province_id,sum(a.seconds) as bill,%d from D_GWDX_SHARE_RESULT_%d a "
					"where (dir=1 and caller_country_id  in ('9','24','37')) or (dir=2 and callee_country_id  in ('9','24','37')) "
					"group by a.province_id "
					"union "
					"select a.province_id,sum(a.seconds) as bill,%d from D_GWDX_SHARE_RESULT_%d a "
					"where (dir=1 and caller_country_id  in ('9','24','37')) or (dir=2 and callee_country_id  in ('9','24','37')) "
					"group by a.province_id "
					"union "
					"select  a.province_id,sum(a.seconds) as bill,%d from D_GWDX_SHARE_RESULT_%d a "
					"where (dir=1 and caller_country_id  in ('9','24','37')) or (dir=2 and callee_country_id  in ('9','24','37')) "
					"group by a.province_id "
					") b  group by b.province_id ",ratecycle,cyclePos[0],cyclePos[0],cyclePos[1],cyclePos[1],cyclePos[2],cyclePos[2]);
		theJSLog<<"插入调账数据到结果表:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();
		
		//结果表求出总时长,比例,调整费用单位时长已经算出,
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWDX_ADJ_OUTPUT_DATA a set TOTAL_BILL_MIN = (select sum(UNIT_BILL_MIN) from D_GWDX_ADJ_OUTPUT_DATA where RATECYCLE = %s  and KMT_FLAG = 1 ) "
					"where a.RATECYCLE = %s and a.KMT_FLAG = 1",ratecycle,ratecycle);
		theJSLog<<"计算总时长:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();			
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWDX_ADJ_OUTPUT_DATA b set b.PROV_PERCENT = b.UNIT_BILL_MIN/b.TOTAL_BILL_MIN*100 "
					"where b.RATECYCLE = %s and b.KMT_FLAG = 1",ratecycle);
		theJSLog<<"计算各省份所占比例:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWDX_ADJ_OUTPUT_DATA c set c.IN_ADJ_FEE = c.UNIT_BILL_MIN/c.TOTAL_BILL_MIN*%lf,c.OUT_ADJ_FEE = c.UNIT_BILL_MIN/c.TOTAL_BILL_MIN*%lf "
					"where c.RATECYCLE = %s and c.KMT_FLAG = 1",in_adj_fee,out_adj_fee,ratecycle);
		theJSLog<<"计算各省份单位时长:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();
		
		//保存100平衡
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(a.prov_percent),sum(a.in_adj_fee),sum(a.out_adj_fee) from D_GWDX_ADJ_OUTPUT_DATA a where a.RATECYCLE = %s and a.kmt_flag = 1",ratecycle);
		cout<<"sql = "<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();	
		stmt>>percent>>in_adj_fee_tmp>>out_adj_fee_tmp;
		theJSLog<<" percent="<<percent<<"  in_adj_fee_tmp="<<in_adj_fee_tmp<<" out_adj_fee_tmp="<<out_adj_fee_tmp<<endi;

		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWDX_ADJ_OUTPUT_DATA a set a.prov_percent=a.prov_percent+%lf,a.in_adj_fee=a.in_adj_fee+%lf,a.out_adj_fee=a.out_adj_fee+%lf "
					"where a.RATECYCLE = %s and a.kmt_flag = 1 and a.PROVINCE = 20 "
					,(100-percent),(in_adj_fee-in_adj_fee_tmp),(out_adj_fee-out_adj_fee_tmp),ratecycle);
		
		theJSLog<<"保存平衡sql:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();	
*/
		theJSLog<<"#############固网短信计算完毕#############"<<endi;


		theJSLog<<"######################调整国际固网语音,账期["<<ratecycle<<"]###########################"<<endi;
		
		in_adj_fee=0,in_adj_usd_fee=0,out_adj_fee=0,out_adj_usd_fee=0,percent = 0;
		in_adj_fee_tmp=0,out_adj_fee_tmp=0,in_adj_usd_fee_tmp=0,out_adj_usd_fee_tmp=0;

		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(*) from D_GWYY_ADJ_INPUT_DATA_%s a where %s ",ratecycle,condtion);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>cnt;
		if(cnt == 0)
		{
			stmt.close();
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"国际固网语音调整数据导入表 D_GWYY_ADJ_INPUT_DATA 没有账期[%s]的数据",ratecycle);
			//theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
			theJSLog<<erro_msg<<endw;
			return ;
		}
		
		//double test_fee1;

		sprintf(sql,"select sum(IN_ADJ_FEE),sum(OUT_ADJ_FEE) from D_GWYY_ADJ_INPUT_DATA_%s a where %s ",ratecycle,condtion);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>in_adj_usd_fee>>out_adj_usd_fee;
		//printf("$$$$$$$$$$$$$$$$$$$$$$$$$test_fee1=%lf$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n",out_adj_usd_fee);
		//out_adj_usd_fee =  test_fee1 ;
		in_adj_fee = in_adj_usd_fee*rate;
		out_adj_fee = out_adj_usd_fee*rate;
		theJSLog<<"in_adj_fee="<<in_adj_fee<<" out_adj_fee="<<out_adj_fee<<" in_adj_usd_fee="<<in_adj_usd_fee<<" out_adj_usd_fee="<<out_adj_usd_fee<<endi;
				
		memset(sql,0,sizeof(sql));
		sprintf(sql,"delete from D_GWYY_ADJ_OUTPUT_DATA_%s a where a.RATECYCLE = %s",ratecycle,ratecycle);
		theJSLog<<"删除结果表数据:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();
		
		//2014-01-07 增加固网语音受付业务表D_GWSF_SHARE_RESULT_的统计

		memset(sql,0,sizeof(sql));
		//union会合并相同的记录 所以每个表查询出来的加日期使其不被合并
		if(strncmp(ratecycle+4,"04",2) == 0) 
		{
			sprintf(sql,"insert into D_GWYY_ADJ_OUTPUT_DATA_%s (RATECYCLE,PROVINCE,KMT_FLAG,UNIT_BILL_MIN) "
					"select %s,b.province_id,0,sum(b.bill) from ("
					"select a.province_id,sum(a.BILL_MIN) as bill from D_ISSF_SHARE_RESULT_%d a "
					"where a.dir in(2,3) and a.callee_country_id not in ('9','24','37') "
					"group by a.province_id "
					"union all "
					"select a.province_id,sum(a.BILL_MIN) as bill from D_ISSF_SHARE_RESULT_%d a "
					"where a.dir in(2,3) and a.callee_country_id not in ('9','24','37') "
					"group by a.province_id "
					"union all "
					"select  a.province_id,sum(a.BILL_MIN) as bill from D_ISSF_SHARE_RESULT_%d a "
					"where a.dir in(2,3) and a.callee_country_id not in ('9','24','37') "
					"group by a.province_id "
					
					"union all "
					"select a.province_id,sum(a.BILL_MIN) as bill from D_GWSF_SHARE_RESULT_%d a "
					"where a.dir in(2,3) and a.callee_country_id not in ('9','24','37') "
					"group by a.province_id "
					"union all "
					"select a.province_id,sum(a.BILL_MIN) as bill from D_GWSF_SHARE_RESULT_%d a "
					"where a.dir in(2,3) and a.callee_country_id not in ('9','24','37') "
					"group by a.province_id "
					"union all "
					"select a.province_id,sum(a.BILL_MIN) as bill from D_GWSF_SHARE_RESULT_%d a "
					"where a.dir in(2,3) and a.callee_country_id not in ('9','24','37') "
					"group by a.province_id "
					") b  group by b.province_id ",ratecycle,ratecycle,cyclePos[1],cyclePos[2],cyclePos[3],cyclePos[1],cyclePos[2],cyclePos[3]);
		}
		else //if ((strncmp(ratecycle+4,"04",2) > 0)  && (strncmp(ratecycle+4,"12",2) <= 0))
		{
			sprintf(sql,"insert into D_GWYY_ADJ_OUTPUT_DATA_%s (RATECYCLE,PROVINCE,KMT_FLAG,UNIT_BILL_MIN) "
					"select %s,b.province_id,0,sum(b.bill) from ("
					"select a.province_id,sum(a.BILL_MIN) as bill from D_ISSF_SHARE_RESULT_%d a "
					"where a.dir in(2,3) and a.callee_country_id not in ('9','24','37') "
					"group by a.province_id "

					"union all "
					"select a.province_id,sum(a.BILL_MIN) as bill from D_GWSF_SHARE_RESULT_%d a "
					"where a.dir in(2,3) and a.callee_country_id not in ('9','24','37') "
					"group by a.province_id "
					") b  group by b.province_id ",ratecycle,ratecycle,cyclePos[1],cyclePos[1]);
		}

		theJSLog<<"插入国际摊分数据到结果表:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();
		
		memset(sql,0,sizeof(sql));
		if(strncmp(ratecycle+4,"04",2) == 0) 
		{
			sprintf(sql,"insert into D_GWYY_ADJ_OUTPUT_DATA_%s (RATECYCLE,PROVINCE,KMT_FLAG,UNIT_BILL_MIN) "
					"select %s,b.province_id,1,sum(b.bill) from ("
					"select a.province_id,sum(a.BILL_MIN) as bill from D_ISSF_SHARE_RESULT_%d a "
					"where dir in(2,3) and callee_country_id  in ('9','24','37') "
					"group by a.province_id "
					"union all "
					"select a.province_id,sum(a.BILL_MIN) as bill from D_ISSF_SHARE_RESULT_%d a "
					"where dir in(2,3) and callee_country_id  in ('9','24','37') "
					"group by a.province_id "
					"union all "
					"select a.province_id,sum(a.BILL_MIN) as bill from D_ISSF_SHARE_RESULT_%d a "
					"where dir in(2,3) and callee_country_id  in ('9','24','37') "
					"group by a.province_id "	
						
					"union all "
					"select a.province_id,sum(a.BILL_MIN) as bill from D_GWSF_SHARE_RESULT_%d a "
					"where dir in(2,3) and callee_country_id  in ('9','24','37') "
					"group by a.province_id "
					"union all "
					"select a.province_id,sum(a.BILL_MIN) as bill from D_GWSF_SHARE_RESULT_%d a "
					"where dir in(2,3) and callee_country_id  in ('9','24','37') "
					"group by a.province_id "
					"union all "
					"select  a.province_id,sum(a.BILL_MIN) as bill from D_GWSF_SHARE_RESULT_%d a "
					"where dir in(2,3) and callee_country_id  in ('9','24','37') "
					"group by a.province_id "
					") b  group by b.province_id ",ratecycle,ratecycle,cyclePos[1],cyclePos[2],cyclePos[3],cyclePos[1],cyclePos[2],cyclePos[3]);
		}
		else //if ((strncmp(ratecycle+4,"04",2) > 0)  && (strncmp(ratecycle+4,"12",2) <= 0))
		{
			sprintf(sql,"insert into D_GWYY_ADJ_OUTPUT_DATA_%s (RATECYCLE,PROVINCE,KMT_FLAG,UNIT_BILL_MIN) "
					"select %s,b.province_id,1,sum(b.bill) from ("
					"select a.province_id,sum(a.BILL_MIN) as bill from D_ISSF_SHARE_RESULT_%d a "
					"where dir in(2,3) and callee_country_id  in ('9','24','37') "
					"group by a.province_id "

					"union all "
					"select  a.province_id,sum(a.BILL_MIN) as bill from D_GWSF_SHARE_RESULT_%d a "
					"where dir in(2,3) and callee_country_id  in ('9','24','37') "
					"group by a.province_id "
					") b  group by b.province_id ",ratecycle,ratecycle,cyclePos[1],cyclePos[1]);
		}

		theJSLog<<"插入港澳台摊分数据到结果表:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();

		//结果表求出总时长,比例,调整费用单位时长已经算出,
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWYY_ADJ_OUTPUT_DATA_%s a set TOTAL_BILL_MIN = (select sum(UNIT_BILL_MIN) from D_GWYY_ADJ_OUTPUT_DATA_%s where RATECYCLE = %s ) "
					"where a.RATECYCLE = %s ",ratecycle,ratecycle,ratecycle,ratecycle);
		theJSLog<<"计算总时长:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();			
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWYY_ADJ_OUTPUT_DATA_%s b set b.PROV_PERCENT = b.UNIT_BILL_MIN/b.TOTAL_BILL_MIN*100 " 
					"where b.RATECYCLE = %s ",ratecycle,ratecycle);
		theJSLog<<"计算各省份所占比例:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWYY_ADJ_OUTPUT_DATA_%s c set c.IN_ADJ_FEE = c.UNIT_BILL_MIN/c.TOTAL_BILL_MIN*%lf,c.OUT_ADJ_FEE = c.UNIT_BILL_MIN/c.TOTAL_BILL_MIN*%lf,"
					"c.IN_ADJ_USD_FEE=c.UNIT_BILL_MIN/c.TOTAL_BILL_MIN*%lf,c.OUT_ADJ_USD_FEE=c.UNIT_BILL_MIN/c.TOTAL_BILL_MIN*%lf "
					"where c.RATECYCLE = %s ",ratecycle,in_adj_fee,out_adj_fee,in_adj_usd_fee,out_adj_usd_fee,ratecycle);
		theJSLog<<"计算各省份单位时长:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();	
		
		//保存100平衡,拿广东省 港台平衡数据
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(a.prov_percent),sum(a.in_adj_fee),sum(a.out_adj_fee),sum(a.in_adj_usd_fee),sum(a.out_adj_usd_fee) from D_GWYY_ADJ_OUTPUT_DATA_%s a where a.RATECYCLE = %s ",ratecycle,ratecycle);
		cout<<"sql = "<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();	
		stmt>>percent>>in_adj_fee_tmp>>out_adj_fee_tmp>>in_adj_usd_fee_tmp>>out_adj_usd_fee_tmp;
		theJSLog<<" percent="<<percent<<"  in_adj_fee_tmp="<<in_adj_fee_tmp<<" out_adj_fee_tmp="<<out_adj_fee_tmp<<" in_adj_usd_fee_tmp="<<in_adj_usd_fee_tmp<<" out_adj_usd_fee_tmp="<<out_adj_usd_fee_tmp<<endi;

		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWYY_ADJ_OUTPUT_DATA_%s a set a.prov_percent=a.prov_percent+(%lf),a.in_adj_fee=a.in_adj_fee+(%lf),a.out_adj_fee=a.out_adj_fee+(%lf),"
					"a.in_adj_usd_fee=a.in_adj_usd_fee+(%lf),a.out_adj_usd_fee=a.out_adj_usd_fee+(%lf) "
					"where a.RATECYCLE = %s and a.kmt_flag = 1 and a.PROVINCE = 20 "
					,ratecycle,(percent_tmp-percent),(in_adj_fee-in_adj_fee_tmp),(out_adj_fee-out_adj_fee_tmp),(in_adj_usd_fee-in_adj_usd_fee_tmp),(out_adj_usd_fee-out_adj_usd_fee_tmp),ratecycle);
		
		theJSLog<<"保存平衡sql:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();	
		
		theJSLog<<"更新日志记录表..."<<endi;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into D_ADJUST_OPER_LOG(RATECYCLE,IN_TABNAME,OUT_TABNAME,DEAL_TIME) values("
					"%s,'%s','%s',to_char(sysdate,'yyyymmddhh24miss'))",ratecycle,"D_GWYY_ADJ_INPUT_DATA","D_GWYY_ADJ_OUTPUT_DATA");
		stmt.setSQLString(sql);
		stmt.execute();

		//######################港澳台的##################################
/*		theJSLog<<"计算港澳台..."<<endi;

		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into D_GWYY_ADJ_OUTPUT_DATA (RATECYCLE,PROVINCE,KMT_FLAG,UNIT_BILL_MIN) "
					"select %s,b.province_id,1,sum(b.bill) from ("
					"select a.province_id,sum(a.seconds) as bill,%d from D_ISSF_SHARE_RESULT_%d a "
					"where (dir=1 and caller_country_id  in ('9','24','37')) or (dir=2 and callee_country_id  in ('9','24','37')) "
					"group by a.province_id "
					"union "
					"select a.province_id,sum(a.seconds) as bill,%d from D_ISSF_SHARE_RESULT_%d a "
					"where (dir=1 and caller_country_id  in ('9','24','37')) or (dir=2 and callee_country_id  in ('9','24','37')) "
					"group by a.province_id "
					"union "
					"select  a.province_id,sum(a.seconds) as bill,%d from D_ISSF_SHARE_RESULT_%d a "
					"where (dir=1 and caller_country_id  in ('9','24','37')) or (dir=2 and callee_country_id  in ('9','24','37')) "
					"group by a.province_id "
					") b  group by b.province_id ",ratecycle,cyclePos[0],cyclePos[0],cyclePos[1],cyclePos[1],cyclePos[2],cyclePos[2]);
		theJSLog<<"插入调账数据到结果表:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();
		
		//结果表求出总时长,比例,调整费用单位时长已经算出,
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWYY_ADJ_OUTPUT_DATA a set TOTAL_BILL_MIN = (select sum(UNIT_BILL_MIN) from D_GWYY_ADJ_OUTPUT_DATA where RATECYCLE = %s  and KMT_FLAG = 1 ) "
					"where a.RATECYCLE = %s and a.KMT_FLAG = 1",ratecycle,ratecycle);
		theJSLog<<"计算总时长:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();			
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWYY_ADJ_OUTPUT_DATA b set b.PROV_PERCENT = b.UNIT_BILL_MIN/b.TOTAL_BILL_MIN*100 "
					"where b.RATECYCLE = %s and b.KMT_FLAG = 1",ratecycle);
		theJSLog<<"计算各省份所占比例:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWYY_ADJ_OUTPUT_DATA c set c.IN_ADJ_FEE = c.UNIT_BILL_MIN/c.TOTAL_BILL_MIN*%lf,c.OUT_ADJ_FEE = c.UNIT_BILL_MIN/c.TOTAL_BILL_MIN*%lf "
					"where c.RATECYCLE = %s and c.KMT_FLAG = 1",in_adj_fee,out_adj_fee,ratecycle);
		theJSLog<<"计算各省份单位时长:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();
		
		//保存100平衡
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(a.prov_percent),sum(a.in_adj_fee),sum(a.out_adj_fee) from D_GWYY_ADJ_OUTPUT_DATA a where a.RATECYCLE = %s and a.kmt_flag = 1",ratecycle);
		cout<<"sql = "<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();	
		stmt>>percent>>in_adj_fee_tmp>>out_adj_fee_tmp;
		theJSLog<<" percent="<<percent<<"  in_adj_fee_tmp="<<in_adj_fee_tmp<<" out_adj_fee_tmp="<<out_adj_fee_tmp<<endi;

		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWYY_ADJ_OUTPUT_DATA a set a.prov_percent=a.prov_percent+%lf,a.in_adj_fee=a.in_adj_fee+%lf,a.out_adj_fee=a.out_adj_fee+%lf "
					"where a.RATECYCLE = %s and a.kmt_flag = 1 and a.PROVINCE = 20 "
					,(100-percent),(in_adj_fee-in_adj_fee_tmp),(out_adj_fee-out_adj_fee_tmp),ratecycle);
		
		theJSLog<<"保存平衡sql:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();	
*/
		stmt.close();
		
		theJSLog<<"#############固网语音计算完毕#############"<<endi;
		
	}catch(util_1_0::db::SQLException e)
	{ 
		stmt.rollback();
		stmt.close();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run %s(%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
		return ;
	}
	catch (jsexcp::CException &e) 
	{	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() %s(%s)",e.GetErrMessage(),sql);
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		
	}

	conn.close();
}

