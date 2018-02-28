
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
	cout<<"�����ʽ: \n"<<endl;
	cout<<"ismp_lackinfo  -t YYYYMM -d YYYYMMDD    \n"<<endl;
	cout<<"ismp_lackinfo  -t YYYYMM    \n"<<endl;
}

bool Ismp::init(int argc,char** argv)
{
	//��ȡ�������
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
		
		theJSLog<<"����:"<<ratecycle<<endi;

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

		theJSLog<<"����:"<<filetime<<endi;

		type = 1;
	  }
	 
	}
	
	if(type == 0)
	{
		printV();

		return false;
	}

	//��ȡ��־·���ͼ���
	if( !param_cfg.bOnInit() )		//���Ĳ�����Ҫ�Լ���ʼ��
	{
		string sErr;
		int nCodeId;
		param_cfg.getError(sErr,nCodeId);
		cerr<<"�������ýӿڳ�ʼ��ʧ�ܣ�������="<<nCodeId<<", ������Ϣ="<<sErr<<endl;
		return false;
	}

	// �Ӻ��Ĳ��������ȡ��־��·��������
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
	
	 theJSLog.setLog(szLogPath, atoi(szLogLevel),"NISMP","GNJS", 001);	//�ļ���־�ӿڣ��������ڴ���־�ӿ�

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
		sprintf(erro_msg,"init()  �������ݿ�ʧ�� connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
		return  ;
	}
	
	Statement stmt,stmt2;
	int cnt = 0;
	try
    {

		//�������ϱ� sp_product
		vector<SP_PRODUCT> v_sp_product;
		SP_PRODUCT sp_product;
		
		theJSLog<<"�������ϱ� sp_product...."<<endi;
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
		
		//��ÿ��������������ƥ�� �ҳ��������
		theJSLog<<"���ҽ������....."<<endi;		
		char settle_flag[2+1];
		
		while(stmt>>sp_cdr_info.CATALOG_ID>>sp_cdr_info.cp_code>>sp_cdr_info.product_id>>sp_cdr_info.cdr_time)
		{
			find_flag = 0;
			record_count++;
			memset(settle_flag,0,sizeof(settle_flag));
			
			//cout<<sp_cdr_info.CATALOG_ID<<":"<<sp_cdr_info.cp_code<<":"<<sp_cdr_info.product_id<<":"<<sp_cdr_info.cdr_time<<endl;
			
			if((strcmp(sp_cdr_info.product_id,"-1") != 0) && (strncmp(sp_cdr_info.product_id,"0",1) != 0))
			{
				//�Ȱ�SP+ҵ����������+��ƷIDȥ���� ����Ʒ���߰�ҵ�����
				for(int i=0;i<v_sp_product.size();i++)
				{
					if((sp_cdr_info.CATALOG_ID == v_sp_product[i].CATALOG_ID) && (sp_cdr_info.cp_code == v_sp_product[i].cp_code) && (strcmp(sp_cdr_info.product_id,v_sp_product[i].product_id) ==0))
					{
						//��ƥ����ʧЧʱ��
						if((strncmp(sp_cdr_info.cdr_time,v_sp_product[i].eff_date,8) >= 0) && (strncmp(sp_cdr_info.cdr_time,v_sp_product[i].exp_date,8) <= 0))
						{
							memset(sql,0,sizeof(sql));

							if(v_sp_product[i].settle_base == 1)  //����Ʒ���ҵ���ҵ�����,��ʱchild_cp_code=sp_code
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
						else	// ����Ʒ���� ������ʧЧ ��ʱ����ȥ��ҵ�������
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
					//  SP+ҵ������������� ��ҵ�����
					if((1 == v_sp_product[i].settle_base) && (sp_cdr_info.CATALOG_ID == v_sp_product[i].CATALOG_ID) && (sp_cdr_info.cp_code == v_sp_product[i].cp_code))
					{
						//��ƥ����ʧЧʱ��
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
						else	// ��ҵ����� ������ʧЧ
						{
							strcpy(settle_flag,"02");
							find_flag = -2;
						}
					}
				}
				
				//����Ʒ��ҵ���Ҳ���
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
	
		
		//���㻰����������
		theJSLog<<"���һ��������������� base_id.... "<<endi;

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
		
		theJSLog<<"���ϲ������.................."<<endi;

	}catch(util_1_0::db::SQLException e)
	{ 
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run %s(%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
	}
	catch (jsexcp::CException &e) 
	{	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() %s(%s)",e.GetErrMessage(),sql);
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		
	}

	conn.close();
}

