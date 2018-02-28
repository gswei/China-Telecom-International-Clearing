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
	cout<<"�����ʽ: \n"<<endl;
	cout<<"jssettleBalance -t ����  \n"<<endl;
}

bool Balance::init(int argc,char** argv)
{
	//��ȡ�������
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
	
	 //printf("xxxxx");
	 theJSLog.setLog(szLogPath, atoi(szLogLevel),"SETT_BALANCE","GJJS", 001);	//�ļ���־�ӿڣ��������ڴ���־�ӿ�

/*	
	 if(!(dbConnect(conn)))
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init()  �������ݿ�ʧ�� connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
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
			cout<<"ģ��:"<<module_id<<"�Ҳ�����Ӧ��ʵ��ID ����tp_process������"<<endl;
			return false;
		}

    }catch(util_1_0::db::SQLException e)
	{ 
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"��ʼ��ʱ���ݿ��ѯ�쳣��%s(sql)",e.what(),sql);
			theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
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
		sprintf(erro_msg,"init()  �������ݿ�ʧ�� connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
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
			sprintf(erro_msg,"���ʱ� EXCHANGE_RATE û������[%s]�ļ�¼",ratecycle);
			theJSLog<<erro_msg<<endw;
			return ;
		}
		
		sprintf(sql,"select RATE from EXCHANGE_RATE where RATE_DATE like '%s%s' and SRC_CURRENCY = 'USD' and DEST_CURRENCY = 'RMB' and SOURCE_ID = 'ISSF' ",ratecycle,"%");
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>rate;

		theJSLog<<"��ǰ����["<<ratecycle<<"]��Ԫ������ҵĻ���:"<<rate<<endi;
		
		theJSLog<<"######################�������ʹ�������,����["<<ratecycle<<"]###########################"<<endi;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(*) from D_GWDX_ADJ_INPUT_DATA_%s a where %s ",ratecycle,condtion);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>cnt;
		if(cnt == 0)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"���ʹ������ŵ������ݵ���� D_GWDX_ADJ_INPUT_DATA û������[%s]������",ratecycle);
			//theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
			theJSLog<<erro_msg<<endw;
			return ;
		}
		
		//�������Ԫ��,Ȼ�����������
		

		sprintf(sql,"select sum(IN_ADJ_FEE),sum(OUT_ADJ_FEE) from D_GWDX_ADJ_INPUT_DATA_%s a where %s ",ratecycle,condtion);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>in_adj_usd_fee>>out_adj_usd_fee;
		in_adj_fee = in_adj_usd_fee*rate;
		out_adj_fee = out_adj_usd_fee*rate;
		theJSLog<<"in_adj_fee="<<in_adj_fee<<" out_adj_fee="<<out_adj_fee<<" in_adj_usd_fee="<<in_adj_usd_fee<<" out_adj_usd_fee="<<out_adj_usd_fee<<endi;

		memset(sql,0,sizeof(sql));
		sprintf(sql,"delete from D_GWDX_ADJ_OUTPUT_DATA_%s a where a.RATECYCLE = %s",ratecycle,ratecycle);
		theJSLog<<"ɾ�����������:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();
		
		//����δ���ǩת����Ӫ�̰�ͳһ������ʽ���ʱ���ڵĲ��������2014�꿪ʼ����4�����ڱ�������1-3�����ڵ�����
		//5�����ڿ�ʼ���µ�������5����������4�µ������Դ����ƣ�11����������10�µ�����
		//Ϊ��������Ҫ��--12�����ȫ�������12�µĵ������㷨Ϊ��11�µ�����+12�µ���Ԥ����Ԥ���㷨����11�µĵ���������
		//����1�������е�12�µĵ��������ȶ�ԭ12��Ԥ��ֵ���в���ṩ
		
		//2014-04-22 ռ��ͳһ�� BILL_MIN �ֶκ���
		memset(sql,0,sizeof(sql));
		if(strncmp(ratecycle+4,"04",2) == 0) 
		{
			//union��ϲ���ͬ�ļ�¼ ����ÿ�����ѯ�����ļ�����ʹ�䲻���ϲ�
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
		
		theJSLog<<"�������̯�����ݵ������:"<<sql<<endi;

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

		theJSLog<<"����۰�̨̯�����ݵ������:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();

		//����������ʱ��,����,�������õ�λʱ���Ѿ����,
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWDX_ADJ_OUTPUT_DATA_%s a set TOTAL_BILL_MIN = (select sum(UNIT_BILL_MIN) from D_GWDX_ADJ_OUTPUT_DATA_%s where RATECYCLE = %s ) "
					"where a.RATECYCLE = %s ",ratecycle,ratecycle,ratecycle,ratecycle);
		theJSLog<<"������ʱ��:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();			
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWDX_ADJ_OUTPUT_DATA_%s b set b.PROV_PERCENT = b.UNIT_BILL_MIN/b.TOTAL_BILL_MIN*100 " 
					"where b.RATECYCLE = %s ",ratecycle,ratecycle);
		theJSLog<<"�����ʡ����ռ����:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWDX_ADJ_OUTPUT_DATA_%s c set c.IN_ADJ_FEE = c.UNIT_BILL_MIN/c.TOTAL_BILL_MIN*%lf,c.OUT_ADJ_FEE = c.UNIT_BILL_MIN/c.TOTAL_BILL_MIN*%lf,"
					"c.IN_ADJ_USD_FEE=c.UNIT_BILL_MIN/c.TOTAL_BILL_MIN*%lf,c.OUT_ADJ_USD_FEE=c.UNIT_BILL_MIN/c.TOTAL_BILL_MIN*%lf "
					"where c.RATECYCLE = %s ",ratecycle,in_adj_fee,out_adj_fee,in_adj_usd_fee,out_adj_usd_fee,ratecycle);
		theJSLog<<"�����ʡ�ݵ�λʱ��:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();	
		
		//����100ƽ��,�ù㶫ʡ ��̨ƽ������
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
		
		theJSLog<<"����ƽ��sql:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();	
		
		theJSLog<<"������־��¼��..."<<endi;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into D_ADJUST_OPER_LOG(RATECYCLE,IN_TABNAME,OUT_TABNAME,DEAL_TIME) values("
					"%s,'%s','%s',to_char(sysdate,'yyyymmddhh24miss'))",ratecycle,"D_GWDX_ADJ_INPUT_DATA","D_GWDX_ADJ_OUTPUT_DATA");
		stmt.setSQLString(sql);
		stmt.execute();

		//######################�۰�̨��##################################
/*		theJSLog<<"����۰�̨..."<<endi;

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
		theJSLog<<"����������ݵ������:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();
		
		//����������ʱ��,����,�������õ�λʱ���Ѿ����,
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWDX_ADJ_OUTPUT_DATA a set TOTAL_BILL_MIN = (select sum(UNIT_BILL_MIN) from D_GWDX_ADJ_OUTPUT_DATA where RATECYCLE = %s  and KMT_FLAG = 1 ) "
					"where a.RATECYCLE = %s and a.KMT_FLAG = 1",ratecycle,ratecycle);
		theJSLog<<"������ʱ��:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();			
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWDX_ADJ_OUTPUT_DATA b set b.PROV_PERCENT = b.UNIT_BILL_MIN/b.TOTAL_BILL_MIN*100 "
					"where b.RATECYCLE = %s and b.KMT_FLAG = 1",ratecycle);
		theJSLog<<"�����ʡ����ռ����:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWDX_ADJ_OUTPUT_DATA c set c.IN_ADJ_FEE = c.UNIT_BILL_MIN/c.TOTAL_BILL_MIN*%lf,c.OUT_ADJ_FEE = c.UNIT_BILL_MIN/c.TOTAL_BILL_MIN*%lf "
					"where c.RATECYCLE = %s and c.KMT_FLAG = 1",in_adj_fee,out_adj_fee,ratecycle);
		theJSLog<<"�����ʡ�ݵ�λʱ��:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();
		
		//����100ƽ��
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
		
		theJSLog<<"����ƽ��sql:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();	
*/
		theJSLog<<"#############�������ż������#############"<<endi;


		theJSLog<<"######################�������ʹ�������,����["<<ratecycle<<"]###########################"<<endi;
		
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
			sprintf(erro_msg,"���ʹ��������������ݵ���� D_GWYY_ADJ_INPUT_DATA û������[%s]������",ratecycle);
			//theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
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
		theJSLog<<"ɾ�����������:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();
		
		//2014-01-07 ���ӹ��������ܸ�ҵ���D_GWSF_SHARE_RESULT_��ͳ��

		memset(sql,0,sizeof(sql));
		//union��ϲ���ͬ�ļ�¼ ����ÿ�����ѯ�����ļ�����ʹ�䲻���ϲ�
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

		theJSLog<<"�������̯�����ݵ������:"<<sql<<endi;
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

		theJSLog<<"����۰�̨̯�����ݵ������:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();

		//����������ʱ��,����,�������õ�λʱ���Ѿ����,
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWYY_ADJ_OUTPUT_DATA_%s a set TOTAL_BILL_MIN = (select sum(UNIT_BILL_MIN) from D_GWYY_ADJ_OUTPUT_DATA_%s where RATECYCLE = %s ) "
					"where a.RATECYCLE = %s ",ratecycle,ratecycle,ratecycle,ratecycle);
		theJSLog<<"������ʱ��:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();			
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWYY_ADJ_OUTPUT_DATA_%s b set b.PROV_PERCENT = b.UNIT_BILL_MIN/b.TOTAL_BILL_MIN*100 " 
					"where b.RATECYCLE = %s ",ratecycle,ratecycle);
		theJSLog<<"�����ʡ����ռ����:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWYY_ADJ_OUTPUT_DATA_%s c set c.IN_ADJ_FEE = c.UNIT_BILL_MIN/c.TOTAL_BILL_MIN*%lf,c.OUT_ADJ_FEE = c.UNIT_BILL_MIN/c.TOTAL_BILL_MIN*%lf,"
					"c.IN_ADJ_USD_FEE=c.UNIT_BILL_MIN/c.TOTAL_BILL_MIN*%lf,c.OUT_ADJ_USD_FEE=c.UNIT_BILL_MIN/c.TOTAL_BILL_MIN*%lf "
					"where c.RATECYCLE = %s ",ratecycle,in_adj_fee,out_adj_fee,in_adj_usd_fee,out_adj_usd_fee,ratecycle);
		theJSLog<<"�����ʡ�ݵ�λʱ��:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();	
		
		//����100ƽ��,�ù㶫ʡ ��̨ƽ������
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
		
		theJSLog<<"����ƽ��sql:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();	
		
		theJSLog<<"������־��¼��..."<<endi;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into D_ADJUST_OPER_LOG(RATECYCLE,IN_TABNAME,OUT_TABNAME,DEAL_TIME) values("
					"%s,'%s','%s',to_char(sysdate,'yyyymmddhh24miss'))",ratecycle,"D_GWYY_ADJ_INPUT_DATA","D_GWYY_ADJ_OUTPUT_DATA");
		stmt.setSQLString(sql);
		stmt.execute();

		//######################�۰�̨��##################################
/*		theJSLog<<"����۰�̨..."<<endi;

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
		theJSLog<<"����������ݵ������:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();
		
		//����������ʱ��,����,�������õ�λʱ���Ѿ����,
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWYY_ADJ_OUTPUT_DATA a set TOTAL_BILL_MIN = (select sum(UNIT_BILL_MIN) from D_GWYY_ADJ_OUTPUT_DATA where RATECYCLE = %s  and KMT_FLAG = 1 ) "
					"where a.RATECYCLE = %s and a.KMT_FLAG = 1",ratecycle,ratecycle);
		theJSLog<<"������ʱ��:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();			
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWYY_ADJ_OUTPUT_DATA b set b.PROV_PERCENT = b.UNIT_BILL_MIN/b.TOTAL_BILL_MIN*100 "
					"where b.RATECYCLE = %s and b.KMT_FLAG = 1",ratecycle);
		theJSLog<<"�����ʡ����ռ����:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_GWYY_ADJ_OUTPUT_DATA c set c.IN_ADJ_FEE = c.UNIT_BILL_MIN/c.TOTAL_BILL_MIN*%lf,c.OUT_ADJ_FEE = c.UNIT_BILL_MIN/c.TOTAL_BILL_MIN*%lf "
					"where c.RATECYCLE = %s and c.KMT_FLAG = 1",in_adj_fee,out_adj_fee,ratecycle);
		theJSLog<<"�����ʡ�ݵ�λʱ��:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();
		
		//����100ƽ��
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
		
		theJSLog<<"����ƽ��sql:"<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();	
*/
		stmt.close();
		
		theJSLog<<"#############���������������#############"<<endi;
		
	}catch(util_1_0::db::SQLException e)
	{ 
		stmt.rollback();
		stmt.close();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run %s(%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
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

