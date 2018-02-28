
#include "gwCarrierAct.h"

IBC_ParamCfgMng param_cfg;
char szInputFiletypeId[5+1];

CarrierAct::CarrierAct()
{
	memset(ratecycle,0,sizeof(ratecycle));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(sql,0,sizeof(sql));
	type = 0;
}

CarrierAct::~CarrierAct()
{


}

void printV()
{	
	cout<<"�����ʽ: \n"<<endl;
	cout<<"gjgw_dataget -t YYYYMM  1  ���ʹ�������,������Ӫ�̷�����ȡ\n"<<endl;
	cout<<"gjgw_dataget -t YYYYMM  2  ���ʹ�������(��Ԫ��2)������ȡ\n"<<endl;
	cout<<"gjgw_dataget -t YYYYMM��3  LTE�˵�ͬ��������ȡ\n" <<endl;
	cout<<"gjgw_dataget -t YYYYMM��4  ���������������ļ����\n" <<endl;
	cout<<"gjgw_dataget -t YYYYMM��5  ȫ�������ܻ��ӿ��ļ�\n" <<endl;

}

bool CarrierAct::init(int argc,char** argv)
{
	//��ȡ�������
	int flag = 0;
	
	for(int i = 1;i<argc;i++)
	{
	  if(strcmp(argv[i],"-t") == 0)
	  {
		if(argc < (i+3))		
		{
			printV();
			return false;
		}
			
		strcpy(ratecycle,argv[i+1]);
		type = atoi(argv[i+2]) ;

		flag = 1;
	  }
	 
	}
	
	if(flag == 0)
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
	
	 theJSLog.setLog(szLogPath, atoi(szLogLevel),"GJGW","GJJS", 001);	//�ļ���־�ӿڣ��������ڴ���־�ӿ�

	return true;
}

void CarrierAct::dealType()
{
	if(type == 1)
	{
		run();
	}
	else if(type == 2)
	{
		run2();

	}
	else if(type == 3)
	{
		run3();

	}
	else if(type == 4)
	{
		run4();

	}
	else if(type == 5)
	{
		run5();

	}
	else
	{
		theJSLog<<"�޷�ʶ�������:"<<type<<endw;
	}

}


void CarrierAct::run()
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
		//��ȡ���ñ���Ϣ��ȡҪ��ȡ���ļ�
/*		
		CBindSQL ds(*m_DBConn);
	
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(*)  from C_GJGS_CARRE_MON a where a.VALIDFLAG = 'Y' ");
		ds.Open(sql, SELECT_QUERY);
		ds.Execute();
		ds>>cnt;
		ds.Close();
*/
		

		stmt = conn.createStatement();
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(*)  from C_GJGS_CARRE_MON a where a.VALIDFLAG = 'Y' ");
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>cnt;

		if(cnt == 0)
		{
			theJSLog<<"C_GJGS_CARRE_MON û����Ч���ļ�������Ϣ"<<endi;
			conn.close();
			return ;
		}
		else 
		{
			/*
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(*)  from C_GJGS_CARRE_MON a where a.VALIDFLAG = 'Y' ");
			ds.Open(sql, SELECT_QUERY);
			ds.Execute();
			*/
		
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select a.REPORT_ID,a.sheet_no,a.col_num,a.sql_col,SEP,END_SEP,OUTFILE,OUTPATH from C_GJGS_CARRE_MON a where a.VALIDFLAG = 'Y' ");
			stmt.setSQLString(sql);
			stmt.execute();
		
		}

		GWFileDefine fileD;
		string filename,sqltxt;

		while(stmt>>fileD.report_id>>fileD.sheet_no>>fileD.col_num>>sqltxt>>fileD.seperator>>fileD.endSep>>filename>>fileD.outpath)
		{
			string_replace(filename,"$YYYYMM$",string(ratecycle));
			strcpy(fileD.filename,filename.c_str());
			theJSLog<<"filename="<<filename<<endi;

			string_replace(sqltxt,"$YYYYMM$",string(ratecycle));
			string_replace(sqltxt,"$REPORT_ID$",string(fileD.report_id));

			strcpy(fileD.sqltxt,sqltxt.c_str());
			theJSLog<<"report_sql="<<sqltxt<<endi;

			vFile.push_back(fileD);
			fileD.clear();
		}
		//ds.Close();


		//���ɹ���������Ӫ����֧ ���ʻ�ͨ������֧
		char outFPathName[512];
		for(int i=0;i<vFile.size();i++)
		{
			memset(sql,0,sizeof(sql)); 

			stmt.setSQLString(vFile[i].sqltxt);
			stmt.execute();
			//ds.Open(vFile[i].sqltxt, SELECT_QUERY);
			//ds.Execute();

			memset(outFPathName,0,sizeof(outFPathName));
			strcpy(outFPathName,vFile[i].outpath);
			strcat(outFPathName,vFile[i].filename);
			
			theJSLog<<"�����ļ�:"<<outFPathName<<endi;
			ofstream outfile(outFPathName);
			if(!outfile.is_open())
			{
				theJSLog<<"���ļ�ʧ��"<<outFPathName<<"  :"<<strerror(errno)<<endw;
				continue;
			}
			
			//�жϼ�¼����
			char mvalue[256];
			int j = 0;
			while(stmt>>mvalue)
			{
				j++;
						
				if( (j % vFile[i].col_num) == 0)
				{		
					outfile<<mvalue<<vFile[i].endSep<<endl;	
				}	
				else
				{
					outfile<<mvalue<<vFile[i].seperator;
				}

				memset(mvalue,0,sizeof(mvalue));
			}
			
			//ds.Close();

			outfile.close();
			theJSLog<<"���ɳɹ�"<<endi;
		}
	  
	}catch(util_1_0::db::SQLException e)
	{ 
		//stmt.rollback();
		//stmt.close();
		//char erro_msg[4096];
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run %s(%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
	}
	catch (jsexcp::CException &e) 
	{	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() (%s)",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		
	}

	conn.close();
}


//���ʹ��� ����������ȡ �ӿ��ļ���ʽ���ı��ļ����ļ��е��ֶ�֮���á�,�����ŷָ����ļ���һ����Ϊһ��ͳ�Ƽ�¼��������¼֮���û��з���\n����Ϊ�ָ�
void CarrierAct::run2()
{
	if(!(dbConnect(conn)))
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init()  �������ݿ�ʧ�� connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
		return  ;
	}
	
	Statement stmt;
	int column_count = 8;
	char outFPathName[512],outFPath[256],filetime[8+1];

	char mvalue[30];
	try
    {
		ofstream outfile;

		stmt = conn.createStatement();
		memset(currTime,0,sizeof(currTime));
		getCurTime(currTime);
		memset(filetime,0,sizeof(currTime));
		strncpy(filetime,currTime,8);
		
		//��ȡ�ļ�����·��  GJGW_VOC_PATH
		memset(outFPath,0,sizeof(outFPath));
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select a.varvalue from c_global_env a where a.varname='GJJS_UP_PATH'");
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>outFPath;
		completeDir(outFPath);
		
		theJSLog<<"GJJS_UP_PATH="<<outFPath<<endi;

		//���� ��������+�����ܸ�    ����ͳ�ƹ�������������ת��+�ܸ������ȥ���תȥ
		
		//����������
		memset(sql,0,sizeof(sql));
		sprintf(sql,"delete from LOCAL_INCOME_SETTLE a where START_TIME=%s ", ratecycle);
		stmt.setSQLString(sql);
		stmt.execute();

	    memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into LOCAL_INCOME_SETTLE select %s as START_TIME,%s as END_TIME,province_id,0 as AMOUNT,sum(bill_min),0 as HCD_AMOUN,0 as IS_GAT,0 as USD_RMB_RATE from ("
					"select a.province_id,sum(a.bill_min) as bill_min "
					"from d_issf_share_result_tmp_%s  a left join  V_PROVINCE t on (a.province_id = t.prov_id)  where a.dir in (1,4) group by province_id "
					"union all "
					"select a.province_id,sum(a.bill_min) as bill_min  "
					"from d_gwsf_share_result_tmp_%s  a left join  V_PROVINCE t on (a.province_id = t.prov_id) where a.dir in (2,3) group by province_id ) group by province_id "
					,ratecycle,ratecycle,ratecycle,ratecycle,ratecycle,ratecycle);

		stmt.setSQLString(sql);
		stmt.execute();
		stmt.commit();

		memset(sql,0,sizeof(sql));
		sprintf(sql,"select %s as START_TIME,%s as END_TIME,province_id,0 as AMOUNT,sum(bill_min),0 as HCD_AMOUN,0 as IS_GAT,0 as USD_RMB_RATE from ("
					"select a.province_id,sum(a.bill_min) as bill_min "
					"from d_issf_share_result_tmp_%s  a left join  V_PROVINCE t on (a.province_id = t.prov_id)  where a.dir in (1,4) group by province_id "
					"union all "
					"select a.province_id,sum(a.bill_min) as bill_min  "
					"from d_gwsf_share_result_tmp_%s  a left join  V_PROVINCE t on (a.province_id = t.prov_id) where a.dir in (2,3) group by province_id ) group by province_id "
					,ratecycle,ratecycle,ratecycle,ratecycle,ratecycle,ratecycle);

		theJSLog<<"����sql:"<<sql<<endi;

		stmt.setSQLString(sql);
		stmt.execute();
		
		
		memset(outFPathName,0,sizeof(outFPathName));
		sprintf(outFPathName,"%sLOCAL_INCOME_SETTLE.%s.dat",outFPath,filetime);
		
		theJSLog<<"�����ļ�:"<<outFPathName<<endi;
		outfile.open(outFPathName,ios::out);
		if(!outfile.is_open())
		{
			theJSLog<<"���ļ�ʧ��"<<outFPathName<<"  :"<<strerror(errno)<<endw;
			return ;
		}

		memset(mvalue,0,sizeof(mvalue));
		int j = 0;
		while(stmt>>mvalue)
		{	
			j++;
			if(j == column_count)
			{
				outfile<<mvalue<<endl;
				j=0;
			}
			else
			{
				outfile<<mvalue<<",";
			}
		}
		
		outfile.close();
		
		//ȥ�� ��������+�����ܸ�  ͳ�ƹ��������ȥ���תȥ+�ܸ�����������ת��

		memset(sql,0,sizeof(sql));
		sprintf(sql,"delete from LOCAL_EXPENSES_SETTLE a where START_TIME=%s ", ratecycle);
		stmt.setSQLString(sql);
		stmt.execute();
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into LOCAL_EXPENSES_SETTLE select %s as START_TIME,%s as END_TIME,province_id,0 as AMOUNT,sum(bill_min),0 as HCD_AMOUN,0 as IS_GAT,0 as USD_RMB_RATE from ( "
					"select a.province_id,sum(a.bill_min) as bill_min "
					"from d_issf_share_result_tmp_%s  a left join  V_PROVINCE t on (a.province_id = t.prov_id)  where a.dir in (2,3) group by province_id "
					"union all "
					"select a.province_id,sum(a.bill_min) as bill_min  "
					"from d_gwsf_share_result_tmp_%s  a left join  V_PROVINCE t on (a.province_id = t.prov_id) where a.dir in (1,4) group by province_id ) group by province_id "
					,ratecycle,ratecycle,ratecycle,ratecycle,ratecycle,ratecycle);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt.commit();

		memset(sql,0,sizeof(sql));
		sprintf(sql,"select %s as START_TIME,%s as END_TIME,province_id,0 as AMOUNT,sum(bill_min),0 as HCD_AMOUN,0 as IS_GAT,0 as USD_RMB_RATE from ( "
					"select a.province_id,sum(a.bill_min) as bill_min "
					"from d_issf_share_result_tmp_%s  a left join  V_PROVINCE t on (a.province_id = t.prov_id)  where a.dir in (2,3) group by province_id "
					"union all "
					"select a.province_id,sum(a.bill_min) as bill_min  "
					"from d_gwsf_share_result_tmp_%s  a left join  V_PROVINCE t on (a.province_id = t.prov_id) where a.dir in (1,4) group by province_id ) group by province_id "
					,ratecycle,ratecycle,ratecycle,ratecycle,ratecycle,ratecycle);

		theJSLog<<"ȥ��sql:"<<sql<<endi;

		stmt.setSQLString(sql);
		stmt.execute();
		
		memset(outFPathName,0,sizeof(outFPathName));
		sprintf(outFPathName,"%sLOCAL_EXPENSES_SETTLE.%s.dat",outFPath,filetime);
		
		theJSLog<<"�����ļ�:"<<outFPathName<<endi;
		outfile.open(outFPathName,ios::out);
		if(!outfile.is_open())
		{
			theJSLog<<"���ļ�ʧ��"<<outFPathName<<"  :"<<strerror(errno)<<endw;
			return ;
		}

		memset(mvalue,0,sizeof(mvalue));
		//int j = 0;
		while(stmt>>mvalue)
		{	
			j++;
			if(j == column_count)
			{
				outfile<<mvalue<<endl;
				j=0;
			}
			else
			{
				outfile<<mvalue<<",";
			}
		}

		outfile.close();

		stmt.close();
		conn.close();


	}catch(util_1_0::db::SQLException e)
	{ 
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run %s(%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
	}
	catch (jsexcp::CException &e) 
	{	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() (%s)",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		
	}

}

// 2014-08-25 LTE�˵�ͬ�������ļ��ӿ�
// 2015-03-26 �����ֶ� Rate_currency
void CarrierAct::run3()
{
	if(!(dbConnect(conn)))
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init()  �������ݿ�ʧ�� connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
		return  ;
	}
	
	Statement stmt;
	int column_count = 24;
	char outFPathName[512],outFPath[256],filetime[8+1];

	char mvalue[30];
	try
    {
		ofstream outfile;

		stmt = conn.createStatement();
		memset(currTime,0,sizeof(currTime));
		getCurTime(currTime);
		memset(filetime,0,sizeof(currTime));
		strncpy(filetime,currTime,8);
		
		//��ȡ�ļ�����·�� GJGW_VOC_PATH
		memset(outFPath,0,sizeof(outFPath));
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select a.varvalue from c_global_env a where a.varname='GJJS_UP_LTE_PATH'");
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>outFPath;
		completeDir(outFPath);
		
		theJSLog<<"GJJS_UP_LTE_PATH="<<outFPath<<endi;

		//
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select a.settle_month,a.cust_id,a.cust_code,a.roam_type,a.service_type,a.tap_seq,a.settle_unit,TO_CHAR(a.settle_rate,'FM999999990.999999'),Rate_currency,a.sdr_currency,a.usd_currency,"
					"a.sdr_usd_rate,a.sdr_usd_rate_date,a.sdr_amount_net,a.sdr_amount_vat,a.sdr_amount_gross,a.usd_amount_net,a.usd_amount_vat,a.usd_amount_gross,"
					"a.redundancy1,a.redundancy2,a.redundancy3,a.redundancy4,a.redundancy5 "
					"from int_lte_account_detail a where a.settle_month='%s'",ratecycle);

		theJSLog<<"sql:"<<sql<<endi;

		stmt.setSQLString(sql);
		stmt.execute();
		
		
		memset(outFPathName,0,sizeof(outFPathName));
		sprintf(outFPathName,"%sINT_LTE_ACCOUNT_DETAIL.%s.dat",outFPath,ratecycle);
		
		theJSLog<<"�����ļ�:"<<outFPathName<<endi;
		outfile.open(outFPathName,ios::out);
		if(!outfile.is_open())
		{
			theJSLog<<"���ļ�ʧ��"<<outFPathName<<"  :"<<strerror(errno)<<endw;
			return ;
		}

		memset(mvalue,0,sizeof(mvalue));
		int j = 0;
		while(stmt>>mvalue)
		{	
			j++;
			if(j == column_count)
			{
				outfile<<mvalue<<endl;
				j=0;
			}
			else
			{
				outfile<<mvalue<<",";
			}
		}
		
		outfile.close();

	}catch(util_1_0::db::SQLException e)
	{ 
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run %s(%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
	}
	catch (jsexcp::CException &e) 
	{	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() (%s)",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		
	}

}

// 2014-09-22 ���������������ļ����
void CarrierAct::run4()
{
	if(!(dbConnect(conn)))
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init()  �������ݿ�ʧ�� connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
		return  ;
	}
	
	Statement stmt;
	int column_count = 23;
	char outFPathName[512],outFPath[256],filetime[8+1],tmp[512];
	
	strcpy(szInputFiletypeId,"IACC");

	char mvalue[30];
	try
    {
		ofstream outfile;

		stmt = conn.createStatement();
		memset(currTime,0,sizeof(currTime));
		getCurTime(currTime);
		memset(filetime,0,sizeof(currTime));
		strncpy(filetime,currTime,8);
		
		//��ȡ�·��ļ�·�� ��ȡ�ļ� IACC_YYYYMM_BBBB.REP�����У�YYYYMM ��ʾ�˶����ڣ�BBBBΪ���кš�
		memset(outFPath,0,sizeof(outFPath));
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select a.varvalue from c_global_env a where a.varname='GJJS_DOWN_PATH'");
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>outFPath;
		completeDir(outFPath);
		
		theJSLog<<"GJJS_DOWN_PATH="<<outFPath<<endi;
		
		CF_CFscan scan;
		//���ļ�Ŀ¼
		if(scan.openDir(outFPath))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"���·��ļ�Ŀ¼[%s]ʧ��",outFPath);
			theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����
			return ;		//�����˳�
		}
		
		char filter[50],fileName[512],m_szFileName[50];
		memset(filter,0,sizeof(filter));
		sprintf(filter,"IACC_%s*.REP",ratecycle);
		int rett = 0;

		while(1)		
		{
				memset(fileName,0,sizeof(fileName));
				rett = scan.getFile(filter,fileName);  				
				if(rett == 100)
				{		
					scan.closeDir();		
					cout<<"��Ŀ¼:"<<outFPath<<"û���ļ�..."<<endl;
					return ;
				}
				else if(rett == -1)
				{	
					scan.closeDir();	
					cout<<"��ʾ��ȡ�ļ���Ϣʧ��..."<<endw;
					return  ;			
				}
				else
				{
					theJSLog<<"��ѯ���ļ�:"<<fileName<<endi;
					//break;
				}

				/*�����ļ�*.tmp,*TMP,~* */			
				char* p = strrchr(fileName,'/');
				memset(tmp,0,sizeof(tmp));
				strcpy(tmp,p+1);
				
				if(tmp[0] == '~' )	continue ;
				if(strlen(tmp) > 2)
				{		
						int pos = strlen(tmp)-3;
						if((strcmp(tmp+pos,"tmp") && strcmp(tmp+pos,"TMP")) == 0) 
						{
							continue;
						}
				}
				
				strcpy(m_szFileName,p+1);

				break;
		}
		
		scan.closeDir();
		
		CF_MemFileI mInfile;
		mInfile.Init(szInputFiletypeId);

		CFmt_Change outrcd;
		outrcd.Init(szInputFiletypeId);
		
		char szBuff[1024],mValue[20];	
		IACC_GJYY miacc;
		vector<IACC_GJYY>	mViacc;

		mInfile.Open(fileName);
		while(1)
		{
			memset(szBuff,0,sizeof(szBuff));
			if( mInfile.readRec(szBuff, MAX_LINE_LENGTH) == READ_AT_END )
			{	
				break;									
			}
			
			outrcd.Set_record(szBuff);
			
			//�Ž��ڴ�,�洢
			miacc.clear();
			outrcd.Get_Field("DIRECTION",mValue);
			miacc.DIRECTION = atoi(mValue);

			outrcd.Get_Field("BATCH_TYPE",mValue);
			miacc.BATCH_TYPE = atoi(mValue);

			outrcd.Get_Field("BATCH_CREATION_DATE",mValue);
			miacc.BATCH_CREATION_DATE = atoi(mValue);

			outrcd.Get_Field("HOME_BID",miacc.HOME_BID);
			outrcd.Get_Field("SERVING_BID",miacc.SERVING_BID);

			outrcd.Get_Field("BATCH_NUMBER",mValue);
			miacc.BATCH_NUMBER = atoi(mValue);

			outrcd.Get_Field("RECORD_NUMBER",mValue);
			miacc.RECORD_NUMBER = atoi(mValue);

			outrcd.Get_Field("BATCH_AMOUNT",mValue);
			miacc.BATCH_AMOUNT = atoi(mValue);

			outrcd.Get_Field("BATCH_AMOUNT_CURRENC",miacc.BATCH_AMOUNT_CURRENC);
			

			mViacc.push_back(miacc);
		}
				
		mInfile.Close();
		//outrcd.close();

		//�����¼
		cout<<"��¼����:"<<mViacc.size()<<endl;
		
		char sql[1024];

		//��ɾ����ǰ�ļ�¼
		memset(sql,0,sizeof(sql));
		sprintf(sql,"delete from D_VOICE_APT_DETAIL_ACCT where settle_month=%s",ratecycle);
		stmt.setSQLString(sql);
		stmt.execute();

		for(int i=0;i<mViacc.size();i++)
		{
			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into D_VOICE_APT_DETAIL_ACCT(SETTLE_MONTH,SERVING_BID,HOME_BID,BATCH_TYPE,BATCH_CREATION_DATE,BATCH_NUMBER,RECORD_NUMBER,BATCH_AMOUNT,BATCH_AMOUNT_CURRENCY,DIRECTION) "
						"values(%s,'%s','%s',%d,%d,%ld,%ld,%ld,'%s',%d)",ratecycle,mViacc[i].SERVING_BID,mViacc[i].HOME_BID,mViacc[i].BATCH_TYPE,mViacc[i].BATCH_CREATION_DATE,mViacc[i].BATCH_NUMBER,
						mViacc[i].RECORD_NUMBER,mViacc[i].BATCH_AMOUNT,mViacc[i].BATCH_AMOUNT_CURRENC,mViacc[i].DIRECTION);
			
			//cout<<"sql:"<<sql<<endl;
			stmt.setSQLString(sql);
			stmt.execute();

		}
		
		stmt.close();
		conn.close();
		
		theJSLog<<"�ļ�:"<<m_szFileName<<"���ɹ�..."<<endi;
		mViacc.clear();
		
		//�ж��Ƿ���Ҫ����										
		char bak_dir[512];
		memset(bak_dir,0,sizeof(bak_dir));
		strcpy(bak_dir,outFPath);
		strcat(bak_dir,"BAK");

		//strncat(bak_dir,currTime,6);
		//completeDir(bak_dir);
		//strncat(bak_dir,currTime+6,2);
		completeDir(bak_dir);

		if(chkAllDir(bak_dir) == 0)
		{
			//theJSLog<<"�����ļ�["<<m_szFileName<<"]��Ŀ¼"<<bak_dir<<endi;
			strcat(bak_dir,m_szFileName);
			if(rename(fileName,bak_dir))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"�ļ�[%s]�ƶ�[%s]ʧ��: %s",fileName,bak_dir,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);		//�ƶ��ļ�ʧ��
			}
		}
		else
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"����·��[%s]�����ڣ����޷�����",bak_dir);
			theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg);		//��Ŀ¼����
		}
		

	}catch(util_1_0::db::SQLException e)
	{ 
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run %s(%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
	}
	catch (jsexcp::CException &e) 
	{	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() (%s)",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		
	}

}

//ȫ�������ܻ�������ȡ �ӿ��ļ���ʽ���ı��ļ����ļ��е��ֶ�֮���á�|�����ŷָ����ļ���һ����Ϊһ��ͳ�Ƽ�¼��������¼֮���û��з���\n����Ϊ�ָ�
void CarrierAct::run5()
{
	if(!(dbConnect(conn)))
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init()  �������ݿ�ʧ�� connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
		return  ;
	}
	
	Statement stmt;
	//int column_count = 7;
	char outFPathName[512],outFPath[256],filetime[8+1];

	char mvalue[30];
	try
	{
		ofstream outfile;

		stmt = conn.createStatement();
		memset(currTime,0,sizeof(currTime));
		getCurTime(currTime);
		memset(filetime,0,sizeof(filetime));
		strncpy(filetime,currTime,8);
		
		//��ȡ�ļ�����·��  GJJS_UP_PATH
		memset(outFPath,0,sizeof(outFPath));
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select a.varvalue from c_global_env a where a.varname='GJJS_UP_PATH'");
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>outFPath;
		completeDir(outFPath);
		
		theJSLog<<"GJJS_UP_PATH="<<outFPath<<endi;

		memset(sql,0,sizeof(sql));

		sprintf(sql,"select prov_id,sum(fee),currency,Settle_month,spare,settle_prov from ("
					"select decode(PhoneHomeCom,'00',to_char(CustArea),PhoneHomeCom) as prov_id,"
           "round(nvl(Fee01,0)/100,2) as fee,'CNY' as currency,Settle_month,' ' as spare,"
           "decode(PhoneHomeCom,'00',to_char(CustArea),PhoneHomeCom) as settle_prov "
          "from D_GVE_SETTLE_RESULT_%s "
				"union all "
					"select decode(PhoneHomeCom,'00',to_char(CustArea),PhoneHomeCom) as prov_id,"
           "round(nvl(Fee02,0)/100,2) as fee,'CNY' as currency,Settle_month,' ' as spare,"
           "PhoneApplyCom  as settle_prov "
          "from D_GVE_SETTLE_RESULT_%s "
				"union all "
					"select decode(PhoneHomeCom,'00',to_char(CustArea),PhoneHomeCom) as prov_id,"
           "round(nvl(Fee03,0)/100,2) as fee,'CNY' as currency,Settle_month,' ' as spare,"
           "decode(CalledNumberCom,'00',to_char(CalledNumberArea),CalledNumberCom) as settle_prov "
          "from D_GVE_SETTLE_RESULT_%s "
				") a where fee <> 0 "
				"group by a.prov_id,a.currency,a.Settle_month,a.spare,a.settle_prov "
				"order by a.prov_id,a.settle_prov "
				,ratecycle,ratecycle,ratecycle);

		stmt.setSQLString(sql);
		stmt.execute();
		
		
		memset(outFPathName,0,sizeof(outFPathName));
		sprintf(outFPathName,"%sGVE_%s.0001",outFPath,filetime);
		
		theJSLog<<"�����ļ�:"<<outFPathName<<endi;
		outfile.open(outFPathName,ios::out);
		if(!outfile.is_open())
		{
			theJSLog<<"���ļ�ʧ��"<<outFPathName<<"  :"<<strerror(errno)<<endw;
			return ;
		}

    //�ļ�ͷ
    outfile<<"STA10|0001|"<<currTime<<"|10"<<endl;
    
		memset(mvalue,0,sizeof(mvalue));
		int j = 0;
		int i = 0;
		float fee = 0;
		
		while(stmt>>mvalue)
		{	
			switch(++j){
			case 1:
				outfile<<++i<<"|";
				outfile<<mvalue<<"|";
				break;
			case 2:
				fee += atof(mvalue);
				outfile<<atof(mvalue)<<"|";
				break;
			case 6:
				outfile<<mvalue<<endl;
				j = 0;
				break;
			default:
				outfile<<mvalue<<"|";
				break;
			}
		}

    //�ļ�β
    outfile<<"END90|0001|"<<i<<"|"<<fee<<endl;
    	
		outfile.close();
		
		stmt.close();
		conn.close();


	}catch(util_1_0::db::SQLException e)
	{ 
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run %s(%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
	}
	catch (jsexcp::CException &e) 
	{	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run() (%s)",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		
	}

}
