
#include "ismp_lackinfo.h"

IBC_ParamCfgMng param_cfg;

Ismp::Ismp()
{
	memset(ratecycle,0,sizeof(ratecycle));
	memset(filetime,0,sizeof(filetime));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(sql,0,sizeof(sql));
	type = 0;
}

Ismp::~Ismp()
{


}

void printV()
{	
	cout<<"命令格式: \n"<<endl;
	cout<<"ismp_lackinfo  -t YYYYMM -d YYYYMMDD    \n"<<endl;
	cout<<"ismp_lackinfo  -t YYYYMM    \n"<<endl;
}

bool Ismp::init(int argc,char** argv)
{
	//获取输入参数
	//int flag = 0;
	
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
		
		theJSLog<<"账期:"<<ratecycle<<endi;

		type = 2;
	  }
	  else if(strcmp(argv[i],"-d") == 0)
	  {
		if(argc < (i+2))		
		{
			printV();
			return false;
		}
			
		strcpy(filetime,argv[i+1]);

		theJSLog<<"日期:"<<filetime<<endi;

		type = 1;
	  }
	 
	}
	
	if(type == 0)
	{
		printV();

		return false;
	}

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
	
	 theJSLog.setLog(szLogPath, atoi(szLogLevel),"NISMP","GNJS", 001);	//文件日志接口，调用了内存日志接口

	return true;
}

void Ismp::dealType()
{	
	run();	
}


void Ismp::run()
{
	
	if(!(dbConnect(conn)))
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init()  连接数据库失败 connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
		return  ;
	}
	
	Statement stmt,stmt2;
	int cnt = 0;
	try
    {

		//加载资料表 sp_product
		vector<SP_PRODUCT> v_sp_product;
		SP_PRODUCT sp_product;
		
		theJSLog<<"加载资料表 sp_product...."<<endi;
		stmt = conn.createStatement();
		stmt2 = conn.createStatement();

		memset(sql,0,sizeof(sql));
		sprintf(sql,"select a.cust_id,a.catalog_id,a.eff_date,a.exp_date,a.cp_code,a.child_cp_code,a.product_id,a.product_type,a.rule_id,a.settle_rule_type,a.settle_base from v_sp_product a ");
		theJSLog<<"###sql: sp_product="<<sql<<endi;

		stmt.setSQLString(sql);
		stmt.execute();
		
		while(stmt>>sp_product.cust_id>>sp_product.CATALOG_ID>>sp_product.eff_date>>sp_product.exp_date>>sp_product.cp_code
				  >>sp_product.child_cp_code>>sp_product.product_id>>sp_product.product_type>>sp_product.rule_id>>sp_product.settle_rule_type>>sp_product.settle_base)
		{

			v_sp_product.push_back(sp_product);
		}
		
		theJSLog<<" sp_product record_count="<<v_sp_product.size()<<endi;

		int find_flag = 0;
		int record_count = 0 ;

		SP_CDR_INFO sp_cdr_info;
		memset(sql,0,sizeof(sql));

		if(type == 1)
		{
			sprintf(sql,"update d_nismp_day_result_%s a set base_id=-1,settle_base=0,product_type=0,settle_rule_type=0,child_cp_code=sp_code,rule_id=-1,settle_flag='00' "
						" where a.filetime=%s ",ratecycle,filetime);
			stmt.setSQLString(sql);
			stmt.execute();
			
			sprintf(sql,"select count(count(*)) "
						"from d_nismp_day_result_%s a where a.filetime=%s group by product_catalog_id,sp_code,product_id,cdr_time ",ratecycle,filetime);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>record_count;

			sprintf(sql,"select a.product_catalog_id,a.sp_code,a.product_id,a.cdr_time "
						"from d_nismp_day_result_%s a where a.filetime=%s group by product_catalog_id,sp_code,product_id,cdr_time  ",ratecycle,filetime);
		}
		else if (type == 2)
		{
			sprintf(sql,"update d_nismp_day_result_%s a set base_id=-1,settle_base=0,product_type=0,settle_rule_type=0,child_cp_code=sp_code,rule_id=-1,settle_flag='00' "
						" ",ratecycle);
			stmt.setSQLString(sql);
			stmt.execute();
			
			sprintf(sql,"select count(count(*)) "
						"from d_nismp_day_result_%s a where  group by product_catalog_id,sp_code,product_id,cdr_time ",ratecycle);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>record_count;

			sprintf(sql,"select a.product_catalog_id,a.sp_code,a.product_id,a.cdr_time "
						"from d_nismp_day_result_%s a  group by product_catalog_id,sp_code,product_id,cdr_time ",ratecycle);
		}
		
		theJSLog<<"cdr_record_count="<<record_count<<endi;
		theJSLog<<"###sql: ismp_cdr="<<sql<<endi;

		stmt.setSQLString(sql);
		stmt.execute();
		
		//对每条话单进行资料匹配 找出结算规则
		theJSLog<<"查找结算规则....."<<endi;		
		char settle_flag[2+1];
		
		while(stmt>>sp_cdr_info.CATALOG_ID>>sp_cdr_info.cp_code>>sp_cdr_info.product_id>>sp_cdr_info.cdr_time)
		{
			find_flag = 0;
			record_count++;
			memset(settle_flag,0,sizeof(settle_flag));
			
			//cout<<sp_cdr_info.CATALOG_ID<<":"<<sp_cdr_info.cp_code<<":"<<sp_cdr_info.product_id<<":"<<sp_cdr_info.cdr_time<<endl;
			
			if((strcmp(sp_cdr_info.product_id,"-1") != 0) && (strncmp(sp_cdr_info.product_id,"0",1) != 0))
			{
				//先按SP+业务能力编码+产品ID去查找 按产品或者按业务结算
				for(int i=0;i<v_sp_product.size();i++)
				{
					if((sp_cdr_info.CATALOG_ID == v_sp_product[i].CATALOG_ID) && (sp_cdr_info.cp_code == v_sp_product[i].cp_code) && (strcmp(sp_cdr_info.product_id,v_sp_product[i].product_id) ==0))
					{
						//再匹配生失效时间
						if((strncmp(sp_cdr_info.cdr_time,v_sp_product[i].eff_date,8) >= 0) && (strncmp(sp_cdr_info.cdr_time,v_sp_product[i].exp_date,8) <= 0))
						{
							memset(sql,0,sizeof(sql));

							if(v_sp_product[i].settle_base == 1)  //按产品查找到按业务结算,此时child_cp_code=sp_code
							{
								v_sp_product[i].child_cp_code=v_sp_product[i].cp_code;
							}

							sprintf(sql,"update d_nismp_day_result_%s a set settle_base=%d,product_type=%d,settle_rule_type=%d,child_cp_code=%d,rule_id=%d "
										" where a.filetime=%s and product_catalog_id='%d' and sp_code='%d' and product_id='%s' and cdr_time='%s' "
										,ratecycle,v_sp_product[i].settle_base,v_sp_product[i].product_type,v_sp_product[i].settle_rule_type,v_sp_product[i].child_cp_code,v_sp_product[i].rule_id
										,filetime,sp_cdr_info.CATALOG_ID,sp_cdr_info.cp_code,sp_cdr_info.product_id,sp_cdr_info.cdr_time);

							stmt2.setSQLString(sql);
							stmt2.execute();
							
							find_flag = 1;

							break;
						}
						else	// 按产品查找 但资料失效 这时候不能去按业务查找了
						{				
							strcpy(settle_flag,"03");
							find_flag = -1;
						}
					}
				}
			}
			
			if(find_flag == 0)
			{
				for(int i=0;i<v_sp_product.size();i++)
				{
					//  SP+业务能力编码查找 按业务结算
					if((1 == v_sp_product[i].settle_base) && (sp_cdr_info.CATALOG_ID == v_sp_product[i].CATALOG_ID) && (sp_cdr_info.cp_code == v_sp_product[i].cp_code))
					{
						//再匹配生失效时间
						if((strncmp(sp_cdr_info.cdr_time,v_sp_product[i].eff_date,8) >= 0) && (strncmp(sp_cdr_info.cdr_time,v_sp_product[i].exp_date,8) <= 0))
						{
							memset(sql,0,sizeof(sql));
							sprintf(sql,"update d_nismp_day_result_%s a set settle_base=%d,product_type=%d,settle_rule_type=%d,rule_id=%d "
										" where a.filetime=%s and product_catalog_id='%d' and sp_code='%d' and product_id='%s' and cdr_time='%s' "
										,ratecycle,v_sp_product[i].settle_base,v_sp_product[i].product_type,v_sp_product[i].settle_rule_type,v_sp_product[i].rule_id
										,filetime,sp_cdr_info.CATALOG_ID,sp_cdr_info.cp_code,sp_cdr_info.product_id,sp_cdr_info.cdr_time);

							stmt2.setSQLString(sql);
							stmt2.execute();
							
							find_flag = 1;
							break ;
						}
						else	// 按业务查找 但资料失效
						{
							strcpy(settle_flag,"02");
							find_flag = -2;
						}
					}
				}
				
				//按产品按业务都找不到
				if(find_flag == 0)
				{
					strcpy(settle_flag,"01");
				}
			}


			if(find_flag != 1)
			{
				memset(sql,0,sizeof(sql));
				sprintf(sql,"update d_nismp_day_result_%s a set settle_flag='%s' "
							" where a.filetime=%s and product_catalog_id='%d' and sp_code='%d' and product_id='%s' and cdr_time='%s' "
							,ratecycle,settle_flag,filetime,sp_cdr_info.CATALOG_ID,sp_cdr_info.cp_code,sp_cdr_info.product_id,sp_cdr_info.cdr_time);
				
				stmt2.setSQLString(sql);
				stmt2.execute();
			}

			if(record_count%2 == 0)
			{
				stmt2.commit();
				theJSLog<<"commit... record_count="<<record_count<<endi;

				break;
			}

			cout<<" record_count"<<record_count<<endl;
		}
		
		cout<<"all record_count="<<record_count<<endl;

		stmt.commit();
	
		
		//计算话单所属基地
		theJSLog<<"查找话单所属基地类型 base_id.... "<<endi;

		memset(sql,0,sizeof(sql));
		if(type == 1)
		{
			sprintf(sql,"update d_nismp_day_result_%s a set base_id=nvl((select b.prov_id from cfg_sms_cp_code b where a.child_cp_code=b.cp_code and a.cdr_time >= b.eff_date and a.cdr_time <= b.exp_date ),-1) "
						" where a.filetime=%s ",ratecycle,filetime);
		}
		else if(type == 2)
		{
			sprintf(sql,"update d_nismp_day_result_%s a set base_id=nvl((select b.prov_id from cfg_sms_cp_code b where a.child_cp_code=b.cp_code and a.cdr_time >= b.eff_date and a.cdr_time <= b.exp_date ),-1) "
						,ratecycle);		
		}

		theJSLog<<"sp_code_cdr sql: "<<sql<<endi;

		stmt.setSQLString(sql);
		stmt.execute();
		stmt.commit();


		stmt.close();
		conn.close();
		
		theJSLog<<"资料查找完毕.................."<<endi;

	}catch(util_1_0::db::SQLException e)
	{ 
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run %s(%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常
	}
	catch (jsexcp::CException &e) 
	{	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() %s(%s)",e.GetErrMessage(),sql);
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		
	}

	conn.close();
}

