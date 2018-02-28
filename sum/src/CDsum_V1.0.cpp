
#ifndef __SUM_DAY_CPP__
#define __SUM_DAY_CPP__

#include "CDsum.h"

CDsum::CDsum()
{
	memset(szSumDate,0,sizeof(szSumDate));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(sql,0,sizeof(sql));
	//memset(source_id,0,sizeof(source_id));
	iSourceCount = 0;
	//m_enable = false;
	flag1 = true;		
	//drStatus = 2;
	pDayList = NULL;
}

CDsum::~CDsum()
{
	if(pDayList != NULL)
	{
		delete[] pDayList;   //�ͷŽṹ��ָ������
	}
	
	mdrDeal.dr_ReleaseDR();
}

bool CDsum::init(int argc,char** argv)
{
	bool ret = true;

	if(flag1)
	{
		//�̳�process,ע����ȱ���ʵ�� 
		cout<<"��פ,�̳�process.cpp"<<endl;
		if(!PS_Process::init(argc,argv))
		{
			return false;
		}
		
		memset(module_id,0,sizeof(module_id));
		strcpy(module_id,module_name);
		
		module_process_id = getPrcID();	
	}
	else		//���Ĳ�����Ҫ�Լ���ʼ��
	{
		/*
		if( !param_cfg.bOnInit() )
		{
			string sErr;
			int nCodeId;
			param_cfg.getError(sErr,nCodeId);
			cerr<<"�������ýӿڳ�ʼ��ʧ�ܣ�������="<<nCodeId<<", ������Ϣ="<<sErr<<endl;
			return false;
		}
		*/
		try
		{
			char *pMldName;
			pMldName=strrchr(argv[0], '/');
			if(pMldName==NULL) pMldName=argv[0];
			else pMldName++;
			
			memset(module_id,0,sizeof(module_id));
			strcpy(module_id,pMldName);

			//ͨ��ģ������ѯ��ʵ��ID +1
			if(!(dbConnect(conn)))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"init()  �������ݿ�ʧ�� connect error");
				theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
				return false ;
			}
		
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
			module_process_id += 1;

			stmt.close();
			conn.close();

		}
		catch(util_1_0::db::SQLException e)
		{ 
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"��ʼ��ʱ���ݿ��ѯ�쳣��%s",e.what());
			theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
			return false;
		}

		//2013-10-22 ������ȡ�����ջ��ܵ����ò�����Ϣ
		 // �Ӻ��Ĳ��������ȡ��־��·��������
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
		if(param_cfg.bGetMem(sParamName, sKeyVal) && sKeyVal.isNumber())
		{
			szLogLevel = sKeyVal.toInteger();
		}
		else
		{	
			cerr<<"���������ļ���������־�ļ��� log.level "<<endl;
			return false ;
		}

		 //�ж�Ŀ¼�Ƿ����
		 DIR *dirptr = NULL; 
		if((dirptr=opendir(szLogPath)) == NULL)
		{
			cout<<"��־Ŀ¼["<<szLogPath<<"]��ʧ��"<<endl;	
			return false ;
		}else closedir(dirptr);
	 
		char trigger_path[512];
		sprintf(sParamName, "dr.trigger.path");
		if(param_cfg.bGetMem(sParamName, sKeyVal))
		{
			memset(trigger_path,0,sizeof(trigger_path));
			strcpy(trigger_path,(const char*)sKeyVal);

			if((dirptr=opendir(trigger_path)) == NULL)
			{		
				cerr<<"triggerĿ¼:"<<trigger_path<<"��ʧ��"<<endl;
				return false ;
			}else  closedir(dirptr);
		
			if(trigger_path[strlen(trigger_path)-1] != '/')
			{
				strcat(trigger_path,"/");
			}
		
			char tmp[32];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%ld",module_process_id);

			m_triggerFile = trigger_path;
			m_triggerFile += tmp;
			m_triggerFile += ".trigger";
		
			cout<<"trigger_path:"<<m_triggerFile<<endl;
		}
		else
		{	
			cerr<<"���������ļ����������ֵĴ����ļ�·�� dr.trigger.path"<<endl;
			return false ;
		}

	}

	bool flag = true;
	for(int i=1;i<argc;i++)
	{
		if(strcmp(argv[i],"-k") == 0)
		{
			theJSLog<<"ģ��["<<module_id<<"]����������..."<<endi;
			flag = false;
			mdrDeal.mdrParam.m_enable = false;
			break;
		}	
	}
	
	if(flag && flag1)
	{
		char tmp[12];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%ld",getPrcID());

		if(!mdrDeal.drInit(module_name,tmp))  return false;
	}

	if(!(rtinfo.connect()))
	{
		return false;
	}
	short status;
	rtinfo.getDBSysMode(petri_status);
	//cout<<"petri status:"<<petri_status<<endl;

	 //��ʼ���ڴ���־�ӿ�
	 bool bb = initializeLog(argc,argv,false);  //�Ƿ����ģʽ
	 if(!bb)
	 {
			return false;
	 }

	 theJSLog.setLog(szLogPath, szLogLevel,"DAY_SUM","SUMMARY", 001);	//�ļ���־�ӿڣ��������ڴ���־�ӿ�
	 
	 return ret ;
}


bool CDsum::init(char *source_id, char *source_group_id)
{		
	bool  ret = true;
	
	//��ʼ����־

	if(!(dbConnect(conn)))
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init()  �������ݿ�ʧ�� connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
		return false ;
	}
	
    //int iSourceCount = 0;
	try
	{	
		Statement stmt = conn.createStatement();
		if(source_group_id[0] != '\0')		//����ָ������Դ�������Դ
		{
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1)  from C_SOURCE_GROUP_DEFINE where SOURCE_GROUP='%s'",source_group_id);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>iSourceCount;
			if(iSourceCount == 0)
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"init()  ����Դ��[%s]û������",source_group_id);
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);		

				return false ;
			}
			
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from C_SOURCE_GROUP_CONFIG where SOURCE_GROUP='%s'",source_group_id);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>iSourceCount;
			if(iSourceCount == 0)
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"init()  ����Դ��[%s]û����������ԴID,����Ϊ0",source_group_id);
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);		
				return false;
			}
			
			//��ѯ��Ч������Դ����
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(a.source_id) from C_SOURCE_GROUP_CONFIG a,C_SUMTABLE_DEFINE b where a.SOURCE_GROUP='%s' and a.source_id = b.sourceid and VALIDFLAG = 'Y'",source_group_id);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>iSourceCount;
			if(iSourceCount == 0)
			{
				theJSLog<<"����Դ��"<<source_group_id<<"ID����Ч"<<endi;
				return false ;
			}
			
			theJSLog<<"��Ч����Դ������"<<iSourceCount<<endi;

			pDayList = new SDayList[iSourceCount];   //��Ч������Դ����
			char strSourceId[8];
			memset(strSourceId,0,sizeof(strSourceId));
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select a.source_id from C_SOURCE_GROUP_CONFIG a ,C_SUMTABLE_DEFINE b where a.SOURCE_GROUP='%s' and a.source_id = b.sourceid and VALIDFLAG = 'Y' ",source_group_id);
			stmt.setSQLString(sql);
			stmt.execute();
			int i =0;
			while(stmt>>strSourceId)
			{
				//���ػ��ܶ����ͨ������ԴID
				ret = loadSumConfig(strSourceId,source_group_id,i);
				if(ret)  return false;
				memset(strSourceId,0,sizeof(strSourceId));
				i++;		
			}
		
			stmt.close();

			ret = true;
		}
		else if(source_id[0] != '\0')
		{
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from C_SOURCE_GROUP_CONFIG where source_id = '%s'",source_id);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>iSourceCount;
			if(iSourceCount == 0)
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"init() ����Դ[%s]û������",source_id);
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return false;
			}

			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from C_SUMTABLE_DEFINE where sourceid = '%s' and VALIDFLAG = 'Y' ",source_id);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>iSourceCount;
			if(iSourceCount == 0)
			{
				theJSLog<<"����ԴID"<<source_id<<"��Ч"<<endi;
				return false ;
			}

			theJSLog<<"��Ч����Դ������"<<iSourceCount<<endi;
			pDayList = new SDayList[1];
			
			//���ػ��ܶ����ͨ������ԴID,  ����ͨ������ԴID�ҵ���������Դ��
			char group[10];
			memset(group,0,sizeof(group));
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select source_group from c_source_group_config where source_id = '%s'",source_id);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>group;
			
			stmt.close();

			ret = loadSumConfig(source_id,group,0);
			if(ret)  return false;
			else
			{
				ret = true;
			}
		}
		else
		{
			ret = init();
			if(ret == false)  return false;

		}
		
	}catch(SQLException e)
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init ���ݿ����%s(%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

		return false;
	 }

	conn.close();
	
	theJSLog<<"��ʼ�����...\n"<<endi;

	return ret;
}

//������������Դ��Ϣ
bool CDsum::init()
{			
  /*	
	if(!(dbConnect(conn)))
	{
		sprintf(erro_msg,"init()  �������ݿ�ʧ�� connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
		return false ;
	}
*/
	//Statement stmt = conn.createStatement();
	char m_szSrcGrpID[8],strSourceId[8];
	int ret = 0;
	string sql ;
	try
	{	
		Statement stmt = conn.createStatement();
		sql = "select count(1) from c_source_group_define a ,C_SOURCE_GROUP_CONFIG b where a.SOURCE_GROUP = b.SOURCE_GROUP ";
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>iSourceCount;
		if(iSourceCount == 0)
		{
			//cout<<"��������Դ�鶼û����������Դ"<<endl;
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"init()  ��������Դ�鶼û����������Դ");
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
			return false ;
		}
		
		//��ѯ��Ч������Դ����
		sql = "select count(1) from c_source_group_define a,C_SOURCE_GROUP_CONFIG b,C_SUMTABLE_DEFINE c where a.SOURCE_GROUP = b.SOURCE_GROUP and b.source_id = c.sourceid and VALIDFLAG = 'Y' " ;
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>iSourceCount;
		if(iSourceCount == 0)
		{
			theJSLog<<"��������Դ��"<<"ID���ܶ���Ч"<<endi;

			return false ;
		}

		theJSLog<<"��Ч����Դ������"<<iSourceCount<<endi;
		pDayList = new SDayList[iSourceCount];

		sql = "select a.source_group,b.SOURCE_ID from c_source_group_define a ,C_SOURCE_GROUP_CONFIG b ,C_SUMTABLE_DEFINE c where a.SOURCE_GROUP = b.SOURCE_GROUP and b.source_id = c.sourceid and VALIDFLAG = 'Y' ";
		stmt.setSQLString(sql);
		stmt.execute();
		memset(m_szSrcGrpID,0,sizeof(m_szSrcGrpID));
		memset(strSourceId,0,sizeof(strSourceId));
		int i = 0;
		while(stmt>>m_szSrcGrpID>>strSourceId)
		{						
			//���ػ��ܶ����ͨ������ԴID
			ret = loadSumConfig(strSourceId,m_szSrcGrpID,i);
			if(ret)  return false;
			
			memset(m_szSrcGrpID,0,sizeof(m_szSrcGrpID));
			memset(strSourceId,0,sizeof(strSourceId));
			i++;
		}

		stmt.close();

	}catch (SQLException e)
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init ���ݿ����%s (%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

		return false;
	 }
	
	//conn.close();

	return true;

}

//�������е�ͳ��������Ϣ
int CDsum::loadSumConfig(char *source_id, char *source_group_id,int pos)
{
	
	strcpy(pDayList[pos].szSourceId,source_id);
	strcpy(pDayList[pos].szSourceGrpId,source_group_id);
	
 try
 {
	int ret = 0;

	Statement stmt = conn.createStatement();
	memset(sql,0,sizeof(sql));
	sprintf(sql, "select ORGSUMT,ORGSUMT_TCOL,DAYSUMT,DAYSUMT_TCOL from C_SUMTABLE_DEFINE where sourceid = '%s'",source_id );
	stmt.setSQLString(sql);
	stmt.execute();
	
	//SDayList pday;
	SCom scom;
	stmt>>scom.iOrgSumt>>scom.szOrgSumtCol>>scom.iDestSumt>>scom.szDestSumtCol;
		
	//ԭʼ������Ŀ���ͨ��configID��� C_STAT_TABLE_DEFINE	
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select table_name from C_STAT_TABLE_DEFINE where CONFIG_ID = %d",scom.iOrgSumt);
	stmt.setSQLString(sql);
	ret = stmt.execute();
	if(ret == 0)  
	{
		sprintf(erro_msg,"û����C_STAT_TABLE_DEFINE��������config_id = '%d' ��Ӧ�ı�",scom.iOrgSumt);
		theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
		return -1;
	}
	stmt>>scom.iOrgTableName;

	memset(sql,0,sizeof(sql));
	sprintf(sql,"select table_name from C_STAT_TABLE_DEFINE where CONFIG_ID = %d",scom.iDestSumt);
	stmt.setSQLString(sql);
	ret = stmt.execute();
	if(ret == 0)  
	{
		sprintf(erro_msg,"û����C_STAT_TABLE_DEFINE��������config_id = '%d' ��Ӧ�ı�",scom.iDestSumt);
		theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
		return -1;
	}
	stmt>>scom.iDestTableName;
	
	stmt.close();

	pDayList[pos].tableItem = scom;			

	theJSLog<<"����ԴID="<<source_id<<" ԭ��ID="<<scom.iOrgSumt<<" ԭ�����="<<scom.iOrgTableName<<" ԭʼ��ʱ���ֶ�"<<scom.szOrgSumtCol
		<<" Ŀ���ID="<<scom.iDestSumt<<" Ŀ������="<<scom.iDestTableName<<" Ŀ���ʱ���ֶ�"<<scom.szDestSumtCol<<endi; //" �����ֶ�:"<<pDayList[pos].rate_cycle<<endi;
	
	ret = getItemInfo(scom,pDayList[pos].vItemInfo);		//��ȡ���������ͳ���ֶε�ֵ
	if(ret)  return -1;
	
 }
 catch (SQLException e)
 {
	memset(erro_msg,0,sizeof(erro_msg));
	sprintf(erro_msg,"loadSumConfig ���ݿ����%s(%s)",e.what(),sql);
	theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

	return -1;
 }
	
	return  0;
}

//��������
void CDsum::setDate(char* date)
{
	strcpy(szSumDate,date);
}

//�����Ƿ�פ
void CDsum::setDaemon(bool flag)
{
	flag1 =flag;
}

//�ж��ջ��������������ṹSDayList�����е��ļ����ﵽ�������������ݲ�ͬ����������ز�ֵͬ��
//ע���������ܺ��ػ��ܵķ���ֵ
int CDsum::checkDayCondition(SDayList &Sday,char *sumday)
{
	int ret = 0;
	//char tmp[15];
	//memset(tmp,0,sizeof(tmp));
	//strcpy(tmp,sumday);

try
{
	Statement stmt = conn.createStatement();
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(1) from D_SUMMARY_RESULT where sourceid = '%s' and sumtype = 'D' and sumdate = '%s'",Sday.szSourceId,sumday);
	//cout<<"sql = "<<sql<<endl;
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>ret;
	if(ret)
	{
		//theJSLog<<"����Դ:"<<Sday.szSourceId<<" ������:"<<"["<<sumday<<"]�Ѿ������ջ���"<<endi;
		return 1;
	}

	int in_sum = 0,out_sum = 0,out_sum0 = 0,out_sum1 = 0,out_sum2 = 0,out_sum3 = 0,out_sum4 = 0;
	
	//�Ӻ˶Խ����d_check_file_detail��ȡ�ļ�������ȷ��
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(1) from d_check_file_detail where SOURCE_ID = '%s' and check_type = 'AUD' and file_time = '%s' and check_flag = 'Y'",Sday.szSourceId,sumday);
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>in_sum;
	

	memset(sql,0,sizeof(sql));		//��ѯ��ʽ��ʧ�ܵ�w�ļ����� ��  ���ļ�
	sprintf(sql,"select count(*) from d_sch_format where SOURCE_ID = '%s' and file_time = '%s' and deal_flag  = 'E' union " 
				"select count(*) from d_sch_format where SOURCE_ID = '%s' and file_time = '%s' and deal_flag  = 'Y' and record_count = 0 ",Sday.szSourceId,sumday,Sday.szSourceId,sumday);
	
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>out_sum1>>out_sum0;
			
	memset(sql,0,sizeof(sql));	   //��ѯԤ����ʧ�ܵļ�¼����E
	sprintf(sql,"select count(*) from d_sch_wrtf where SOURCE_ID = '%s' and file_time = '%s' and deal_flag  = 'E' ",Sday.szSourceId,sumday);
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>out_sum2;

	
	memset(sql,0,sizeof(sql));	   //��ѯ���ʧ�ܵ��ļ���E ����ȷ���ļ���Y
	sprintf(sql,"select count(*) from d_sch_indb where SOURCE_ID = '%s' and file_time = '%s' and deal_flag = 'E' union "
				"select count(*) from d_sch_indb where SOURCE_ID = '%s' and file_time = '%s' and deal_flag = 'Y' ",Sday.szSourceId,sumday,Sday.szSourceId,sumday);				
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>out_sum3>>out_sum4;
	
	if(out_sum3)
	{	
		//memset(sql,0,sizeof(sql));
		//sprintf(sql,"select count(1) from D_BALANCE_DAYCHECK where source_id = '%s' and deal_date = '%s'",Sday.szSourceId,tmp);
		//stmt.setSQLString(sql);
		//stmt.execute();
		//stmt>>ret;
		//if(ret == 0)					   //��ʾ����û����ƽ��У�� 
		//{
		//	memset(sql,0,sizeof(sql));	   //��ѯ��������ļ�¼��
		//	sprintf(sql,"insert into D_BALANCE_DAYCHECK(SOURCE_ID,INPUT_COUNT,F_ERR_COUNT,R_ERR_COUNT,INDB_COUNT,INDB_ERR_COUNT,BLANCE_FLAG,DEAL_DATE) values('%s',%d,%d,%d,%d,%d,'N','%s')",Sday.szSourceId,in_sum,out_sum1,out_sum2,out_sum4,out_sum3,tmp);
		//	stmt.setSQLString(sql);
		//	stmt.execute();
		//}
	
		stmt.close();	 
		//cout<<"����Դ��"<<Sday.szSourceId<<" ���ڣ�"<<sumday<<endl;
		
	
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"  ����Դ[%s]������[%s]��������ʧ��:�����[d_sch_indb]�д������ʧ�ܵ��ļ�,����[%d]",Sday.szSourceId,sumday,out_sum3);
		theJSLog.writeLog(0,erro_msg);
		return -2;
	}
	
	out_sum = out_sum0+out_sum1+out_sum2+out_sum4;
	if(in_sum != out_sum) 
	{
		theJSLog<<in_sum<<" != "<<out_sum<<" ����Դ["<<Sday.szSourceId<<"]������["<<sumday<<"]�����ļ���δ������ɣ���ȴ�"<<endi;
		stmt.close();
		return 2;
	}
	
	//memset(sql,0,sizeof(sql));	  
	//sprintf(sql,"insert into D_BALANCE_DAYCHECK(SOURCE_ID,INPUT_COUNT,F_ERR_COUNT,R_ERR_COUNT,INDB_COUNT,INDB_ERR_COUNT,BLANCE_FLAG,DEAL_DATE) values('%s',%d,%d,%d,%d,%d,'Y','%s')",Sday.szSourceId,in_sum,out_sum1,out_sum2,out_sum4,out_sum3,tmp);
	//stmt.setSQLString(sql);
	//stmt.execute();

	stmt.close();

 }catch(SQLException e)
 {
	memset(erro_msg,0,sizeof(erro_msg));
	sprintf(erro_msg,"checkDayCondition ���ݿ����%s (%s)",e.what(),sql);
	theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

	return -1;
 }

	return 0;
}

int CDsum::sum(SDayList &Sday,char *sumday)
{
	int ret = 1;
	char sql[2048];
	Statement stmt;

    try
	{	
		long cnt = 0,total = 0;
		char time_col[30];
		memset(time_col,0,sizeof(time_col));
		//��ѯ�ջ��ܼ�¼�����������ж�������ʱ���ֶ�ԭʼ����ORGSUMT_TCOL�ֶζ�Ӧ������ֶΣ�SItemPairͨ��Դͷ��Ŀ��
		for(int i = 0;i<Sday.vItemInfo.size();i++)		//��ѯ���ͳ����ͳ��ά�ȵ�
		{
				SItem fromItem = Sday.vItemInfo[i].fromItem;
				//SItem toItem = vItemInfo[i].toItem;
				if(strcmp(fromItem.szItemName,Sday.tableItem.szOrgSumtCol) == 0)
				{
						strcpy(time_col,Sday.vItemInfo[i].toItem.szItemName);
						cnt = 1;
						break;
				}
		}			
		if(cnt == 0)
		{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"�ֶ�[%s]û����ͳ�Ƹ�ʽ��[%s]����",Sday.tableItem.szOrgSumtCol,Sday.tableItem.iOrgTableName);
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return -1 ;
		}
		
		memset(mdrDeal.m_AuditMsg,0,sizeof(mdrDeal.m_AuditMsg));
		strcpy(mdrDeal.m_AuditMsg,Sday.szSourceId);
		strcat(mdrDeal.m_AuditMsg,":");
		
		stmt = conn.createStatement();

		//������������,1����2��
		vector<string> vrc;
		string rc;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select distinct(rate_cycle) from D_CHECK_FILE_DETAIL where source_id = '%s' and check_type = 'AUD' and file_time = '%s' ",Sday.szSourceId,sumday);
		cout<<"sql = "<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		while(stmt>>rc)
		{
			vrc.push_back(rc);
		}

		if(vrc.size() == 0)
		{
			theJSLog<<"����û���嵥�ļ�����"<<endi;
		}

		for(int i =0;i<vrc.size();i++)
		{	
			theJSLog<<"�����ļ�����:"<<vrc[i]<<endi;
			
			//2013-12-11
			theJSLog<<"ɾ������"<<sumday<<"������"<<endi;
			memset(sql,0,sizeof(sql));
			sprintf(sql,"delete from %s_%s where %s = '%s'",Sday.tableItem.iDestTableName,vrc[i],time_col,sumday);
			cout<<"ɾ�����ܱ���sql="<<sql<<endl;	
			stmt.setSQLString(sql);
			stmt.execute();

			ret = getSql(Sday.tableItem,Sday.vItemInfo,sumday,sql,1,vrc[i].c_str());	//ƴ��sql

			if(ret == -1)
			{		
				return -1 ;
			}	
			cout<<"ƴ�ӵ�sql = "<<sql<<endl;		
				
			stmt.setSQLString(sql);
			stmt.execute();

			//��ȡsql��ѯ �ٲ�����Ϊͳ���� ,���ݽ��������ȡ�ٲ�����
			string strsql;
			strsql = "select ";
			for(int i = 0;i<Sday.vItemInfo.size();i++)		
			{
				SItem toItem = Sday.vItemInfo[i].toItem;
				if(toItem.iItemType == 0)
				{
					strsql.append("sum(").append(toItem.szItemName).append("),");
				}
			
			}
		
			memset(sql,0,sizeof(sql));
			sprintf(sql," from  %s_%s where %s = '%s'",Sday.tableItem.iDestTableName,vrc[i],time_col,sumday);

			strsql = strsql.substr(0, strsql.length()-1);
			strsql.append(sql);

			theJSLog<<"��ѯ�����ͳ���������sql:"<<strsql<<endi;
			memset(sql,0,sizeof(sql));
			sprintf(sql,"%s",strsql);
			sprintf(mdrDeal.m_AuditMsg,"%s%s_%s>",mdrDeal.m_AuditMsg,Sday.tableItem.iDestTableName,vrc[i]);
			stmt.setSQLString(sql);
			stmt.execute();
			string str  = "";
			while(stmt>>str)
			{
				sprintf(mdrDeal.m_AuditMsg,"%s%s|",mdrDeal.m_AuditMsg,str);
			}
			
			//�費��Ҫͳ����ǰ�Ľ�������������?????????????????????
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from  %s_%s where %s = '%s'",Sday.tableItem.iDestTableName,vrc[i],time_col,sumday);

			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>cnt;
			
			theJSLog<<"Ԥ����¼����:"<<cnt<<endi;
			total += cnt;
	
		}//for rate_cycle	
		
		//theJSLog<<"wait dr_audit() ..."<<endi;
		ret = mdrDeal.IsAuditSuccess(mdrDeal.m_AuditMsg);
		if(ret)									//�ٲ�ʧ��,�ع����ݿ�,ɾ����ʱ�ļ�
		{
			stmt.rollback();
			//memset(sql,0,sizeof(sql));
			//sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME)values('%s','%s','%s',%ld,to_char(sysdate,'yyyymmddhh24miss'))",Sday.szSourceId,"D",sumday,-2);
			//stmt.setSQLString(sql);
			//stmt.execute();

			stmt.close();
		}
		else									
		{	
			theJSLog<<"�ܹ�������¼����"<<total<<"	д������־��[D_SUMMARY_RESULT]"<<endi;

			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME)values('%s','%s','%s',%ld,to_char(sysdate,'yyyymmddhh24miss'))",Sday.szSourceId,"D",sumday,total);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt.close();
		}
		
		ret = 0;
	}
	catch(SQLException e)
	{
		//dr_AbortIDX();

		stmt.rollback();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"sum ���ݿ����%s (%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

		return -1;
	}	
		
	return ret ;

}

void CDsum::prcExit()
{
	mdrDeal.dr_ReleaseDR();

	PS_Process::prcExit();
}

//��������Դ���л��ܴ���
void CDsum::run()
{	
	//int ret = -1;
	char currDate[8+1],tmpTime[2+1];
	int ret = 0,event_sn, event_type;
	long param1, param2, src_id;
	short db_status = 0;

while(1)
{	
	if(gbExitSig)
	{
		theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "�ջ��ܳ����յ��˳��ź�");
		prcExit();
		return;
	}
	
	ret=getCmd(event_sn, event_type, param1, param2, src_id);
	if(ret == 1)
	{
		if(event_type == EVT_CMD_STOP)
		{
			theJSLog<<"***********���յ��˳�����**********************\n"<<endw;
			prcExit();
		}
	}
	
	theJSLog.reSetLog();

	rtinfo.getDBSysMode(db_status);
	if(db_status != petri_status)
	{
		theJSLog<<"���ݿ�״̬�л�... "<<petri_status<<"->"<<db_status<<endw;
		int cmd_sn = 0;
		if( !putEvt(cmd_sn, EVT_RPT_DBSTATUS, 0, db_status, DSPCH_PRC_ID) )
		{
			theJSLog<<"�������ݿ����״̬ʧ�ܣ�\n"<<endw;
			continue ;
		}
		petri_status = db_status;
	}
	if(petri_status == DB_STATUS_OFFLINE)	continue ;
	
	
	if(mdrDeal.mdrParam.drStatus == 1)		//����ϵͳ
	{
		//���trigger�����ļ��Ƿ����
		if(!mdrDeal.CheckTriggerFile(m_triggerFile))
		{
			sleep(1);
			return ;
		}

		memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));
		ret = mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
		if(ret)
		{
			theJSLog<<"��ϵͳͬ��ʧ��...."<<endw;
			continue ;
		}
		
		//��ȡͬ������,����ԴID,�˶�����
		vector<string> data;		
		splitString(mdrDeal.m_SerialString,"|",data,false,false);
		
		memset(szSumDate,0,sizeof(szSumDate));
		strcpy(szSumDate,data[1].c_str());
		szSumDate[8]='\0';
		theJSLog<<"��������:"<<szSumDate<<"����ԴID:"<<data[0]<<endi;
		
		if(!(dbConnect(conn)))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"run()  �������ݿ�ʧ�� connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
			continue  ;
		}

		int pos = -1;
		for(int i = 0;i< iSourceCount;i++)
		{
			if(strcmp(pDayList[i].szSourceId,data[0].c_str()) == 0)
			{
				pos = i;
				break;
			}
		}

		if(pos == -1)
		{
			//dr_AbortIDX();
			mdrDeal.dr_abort();
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"û���ҵ�������Դ��Ϣ[%s]",data[0]);		//��������δ����
			theJSLog.writeLog(LOG_CODE_SOURCE_NOT_FIND,erro_msg);  
			continue ;
		}
		
		//int iStatus = dr_GetAuditMode(module_id);
		int iStatus = mdrDeal.mdrParam.aduitMode;

		if(iStatus == 1)		//ͬ��ģʽ,	��ϵͳ�ȴ�ָ��ʱ��
		{	
				bool flag = false;
				int times = 0;
				while(times < 11)
				{
					ret = checkDayCondition(pDayList[pos],szSumDate);
					if( ret == 2)
					{
						times++;
						theJSLog<<"�ջ�������������:������"<<times<<"��"<<endi;
						sleep(30);
					}
					else
					{
						flag = true;
						break;
					}
				}
				if(!flag)
				{
					//dr_AbortIDX();
					mdrDeal.dr_abort();
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"�ջ�������������");
					theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg); 
					continue ;
				}		
		}
		else if(iStatus == 2)		//����ģʽ,��ϵͳ
		{
				while(1)
				{
					//�����ж�
					if(gbExitSig)
					{
						//dr_AbortIDX();
						mdrDeal.dr_abort();
						theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "Ӧ�ó����յ��˳��ź�");
						prcExit();
						return;
					}
					
					ret = checkDayCondition(pDayList[pos],szSumDate);
					if(ret == 2)
					{
						sleep(10);
					}
					else
					{
						break;
					}
				}	
		}
	
		//��ϵͳֻ�����˻��ܲŻ���ϵͳ������Ϣ,����ʱ��ϵͳӦ���������������
		if(ret != 0)
		{
			//dr_AbortIDX();
			mdrDeal.dr_abort();
			continue ;
		}
		
		theJSLog<<"�˶���������,׼������"<<endi;
		ret = sum(pDayList[pos],szSumDate);
		if(ret == -1) 
		{
			//dr_AbortIDX();
			mdrDeal.dr_abort();
		}
		
		conn.close();
	}
	else
	{
		getCurTime(currTime);			//��ȡ��ǰ�ջ���ʱ��
		memset(currDate,0,sizeof(currDate));
		strncpy(currDate,currTime,8);
		currDate[8] = '\0';

		memset(tmpTime,0,sizeof(tmpTime));
		strncpy(tmpTime,currTime+8,2);
		tmpTime[2] = '\0';
		if(strcmp(tmpTime,"06") < 0)			continue ;  //6�����Ժ�ִ��

		memset(szSumDate,0,sizeof(szSumDate));
		addDays(-1,currDate,szSumDate);			//��ȡ���������
		
		//strcpy(szSumDate,"20131008");			//���� 2013-12-29

		if(!(dbConnect(conn)))
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"run()  �������ݿ�ʧ�� connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
			continue  ;
		}

		for(int i = 0;i<iSourceCount;i++)
		{				
			//ÿ������Դ�ж��Ƿ�ﵽ��������
			ret = checkDayCondition(pDayList[i],szSumDate);		//����������
			
			switch(ret)
			{
				case 0:
						theJSLog<<"������������,��������Դ:"<<pDayList[i].szSourceId<<"��������:"<<szSumDate<<endi;
						//����ͬ����Ϣ
						memset(mdrDeal.m_SerialString,0,sizeof(mdrDeal.m_SerialString));
						sprintf(mdrDeal.m_SerialString,"%s|%s",pDayList[i].szSourceId,szSumDate);
						ret = mdrDeal.drVarGetSet(mdrDeal.m_SerialString);
						if(ret)
						{
							theJSLog<<"��ϵͳͬ��ʧ��...."<<endw;
							break;
						}

						ret = sum(pDayList[i],szSumDate);
						if(ret == -1) 
						{
							//dr_AbortIDX();
							mdrDeal.dr_abort();
						}
						break;

				case 1:
						break;
				case -2:				//�к˶Ի������ʧ�ܵ��ļ�,д��־��
						break;
				case 2:					//������δ��ɣ��´β�ѯ
						break;		
				case -1:
						break  ;
				default:
						break;
			}
		}
		
		conn.close();

		sleep(1800);
	}
	
	sleep(30);
  } //while(1)

	//return ;
}

//Summary -rd-t YYYYMMDD �����ջ��� ���������Ѿ�����,�һ��������Ѿ�����
int CDsum::redorun(char* date,bool del)
{
	int  ret = -1;
	
	theJSLog<<"############################���»�������["<<date<<"]������################################"<<endi;
	
	rtinfo.getDBSysMode(petri_status);
	if(petri_status == DB_STATUS_OFFLINE)	
	{
		theJSLog<<"��ǰ���ݿ�״̬Ϊ����̬,�޷�����..."<<endw;
		return ;
	}

	if(!(dbConnect(conn)))
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"run()  �������ݿ�ʧ�� connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
		return -1 ;
	}	
	
	memset(szSumDate,0,sizeof(szSumDate));
	strcpy(szSumDate,date);
	szSumDate[8] = '\0';

	for(int i = 0;i<iSourceCount;i++)
	{			
		theJSLog<<"##########################��������Դ:"<<pDayList[i].szSourceId<<"####################"<<endi;
			
		ret = redosum(pDayList[i],date,del);	
	}

	conn.close();

	theJSLog<<"##########################��������["<<date<<"]���################################"<<endi;

	return ret ;
}


int CDsum::redosum(SDayList &Sday,char *sumday,bool del)
{
	int ret = 0;
	char sql[2048];
	long cnt = 0,last_cnt = 0,total = 0;
	
	char time_col[30];
	memset(time_col,0,sizeof(time_col));

	for(int i = 0;i<Sday.vItemInfo.size();i++)		//��ʱ���ֶβ����
	{
		SItem fromItem = Sday.vItemInfo[i].fromItem;
		if(strcmp(fromItem.szItemName,Sday.tableItem.szOrgSumtCol) == 0)
		{
				strcpy(time_col,Sday.vItemInfo[i].toItem.szItemName);
				ret = 1;
				break;
		}
	}
		
	if(ret == 0)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"�ֶ�[%s]û����ͳ�Ƹ�ʽ��[%s]����",Sday.tableItem.szOrgSumtCol,Sday.tableItem.iOrgTableName);
		theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);

		return -1 ;
	}

	Statement stmt;
	try
	{
		stmt = conn.createStatement();
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,DEALTIME,DEAL_FLAG) values('%s','%s','%s',to_char(sysdate,'yyyymmddhh24miss'),'W')",Sday.szSourceId,"RD",sumday); 
		stmt.setSQLString(sql);
		stmt.execute();
		stmt.commit();

		//memset(m_AuditMsg,0,sizeof(m_AuditMsg));
		//strcpy(m_AuditMsg,Sday.szSourceId);
		//strcat(m_AuditMsg,":");

		//������������,1����2��
		vector<string> vrc;
		string rc;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select distinct(rate_cycle) from D_CHECK_FILE_DETAIL where source_id = '%s' and check_type = 'AUD' and file_time = '%s' ",Sday.szSourceId,sumday);
		stmt.setSQLString(sql);
		stmt.execute();
		while(stmt>>rc)
		{
			vrc.push_back(rc);
		}

		if(vrc.size() == 0)
		{
			theJSLog<<"����û���嵥�ļ�����"<<endi;
		}
		
		for(int i =0;i<vrc.size();i++)
		{
			theJSLog<<"�����ļ�����:"<<vrc[i]<<endi;

			if(del)				//��ɾ��ԭ�Ȼ��ܱ������,�ж�������ʱ���ֶ�ԭʼ����ORGSUMT_TCOL�ֶζ�Ӧ������ֶΣ�SItemPairͨ��Դͷ��Ŀ��
			{		
				theJSLog<<"ɾ������"<<sumday<<"������"<<endi;
				memset(sql,0,sizeof(sql));
				sprintf(sql,"delete from %s_%s where %s = '%s'",Sday.tableItem.iDestTableName,vrc[i],time_col,sumday);
				cout<<"ɾ�����ܱ���sql="<<sql<<endl;
			
				stmt.setSQLString(sql);
				stmt.execute();
			}
			else
			{		
				memset(sql,0,sizeof(sql));
				sprintf(sql,"select count(1) from %s_%s where %s = '%s'",Sday.tableItem.iDestTableName,vrc[i],time_col,sumday);
				stmt.setSQLString(sql);
				stmt.execute();
				stmt>>last_cnt;
				//cout<<"������ǰ���ݼ�¼����:"<<last_cnt<<endl;
			}

			ret = getSql(Sday.tableItem,Sday.vItemInfo,sumday,sql,1,vrc[i].c_str());			//ƴ��sql

			if(ret == -1)
			{		
				return -1 ;
			}

			theJSLog<<"ƴ�ӵ�sql = "<<sql<<endi;	//������ܼ�¼
			stmt.setSQLString(sql);
			stmt.execute();
		
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from  %s_%s where %s = '%s'",Sday.tableItem.iDestTableName,vrc[i],time_col,sumday);	
			cout<<"ͳ���ػ��ܼ�¼����sql:"<<sql<<endl;		
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>cnt;
			
			theJSLog<<"Ԥ����¼����:"<<(cnt-last_cnt)<<endi;
			total += (cnt-last_cnt);

		}
			
		theJSLog<<"������¼����:"<<total<<"	д���ܽ����D_SUMMARY_RESULT"<<endi;		
		memset(sql,0,sizeof(sql));
		//sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME)values('%s','%s','%s',%ld,to_char(sysdate,'yyyymmddhh24miss'))",Sday.szSourceId,"RD",sumday,total);
		sprintf(sql,"update D_SUMMARY_RESULT set DEAL_FLAG='Y',SUMCOUNT=%d,DEALTIME=to_char(sysdate,'yyyymmddhh24miss') where SOURCEID='%s' and SUMDATE='%s' and DEAL_FLAG='W' ",total,Sday.szSourceId,sumday);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt.close();
		
		ret = 0;
		
	}catch(SQLException e)
	{
		stmt.rollback();

		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"redosum ���ݿ���� %s (%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_SUMMARY_RESULT set DEAL_FLAG='N',DEAL_DESC='%s',DEALTIME=to_char(sysdate,'yyyymmddhh24miss') where SOURCEID='%s' and SUMDATE='%s' and DEAL_FLAG='W' ",e.what(),Sday.szSourceId,sumday);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt.commit();	
		//cout<<"sql = "<<sql<<endl;

		return -1;
	}

	return 0 ;
}

/*
//���ֳ�ʼ��
bool CDsum::drInit()			//���ڴ�������ʵ��
{
		//����ģ������ʵ��ID
		char tmp[32];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%ld",module_process_id);

		theJSLog << "��ʼ������ƽ̨,ģ����:"<< module_id<<" ʵ����:"<<tmp<<endi;

		int ret = dr_InitPlatform(module_id,tmp);
		if(ret != 0)
		{
			theJSLog << "����ƽ̨��ʼ��ʧ��,����ֵ=" << ret<<endw;
			return false;
		}
		else
		{
			theJSLog<<"dr_InitPlatform ok."<<endi;
		}

		m_enable = true;

		drStatus = _dr_GetSystemState();	//��ȡ����ϵͳ״̬
		if(drStatus < 0)
		{
			theJSLog<<"��ȡ����ƽ̨״̬����: ����ֵ="<<drStatus<<endw;
			return false;
		}
		
		if(drStatus == 0)		theJSLog<<"��ǰϵͳ����Ϊ��ϵͳ"<<endi;
		else if(drStatus == 1)	theJSLog<<"��ǰϵͳ����Ϊ��ϵͳ"<<endi;
		else if(drStatus == 2)	theJSLog<<"��ǰϵͳ���÷�����ϵͳ"<<endi;

		return true;
}

//��ϵͳ����ͬ������,��ϵͳ��ȡͬ������
int CDsum::drVarGetSet(char* serialString)
{
		int ret  = 0;
		char tmpVar[5000] = {0};
		
		if(!m_enable)
		{
			return ret;
		}

		//�������ƽ̨���л���
		ret = dr_CheckSwitchLock();   
		if(ret != 0)  
		{  
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"��������л���ʧ��,����ֵ=%d",ret);
			theJSLog.writeLog(LOG_CODE_DR_CHECK_LOCK_ERR,erro_msg);

			return -1;  
		} 
		//��ʼ��index  
		ret = dr_InitIDX();  
		if(ret != 0)  
		{  
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"��ʼ��indexʧ��,����ֵ=%d",ret);
			theJSLog.writeLog(LOG_CODE_DR_INIT_IDX_ERR,erro_msg);

			//dr_AbortIDX();
			return -1;  
		}

		snprintf(tmpVar, sizeof(tmpVar), "%s", serialString);
		//��ϵͳ��Ҫͬ����index ����ֵ�ԡ�д������ƽ̨ά����index �ļ���
		//��ϵͳ���øú����Ľ���ǣ�var��ú���ϵͳһ�������������ֵ��	SYNC_SINGLE��ʾע�ᵥһ���������
		ret = dr_SyncIdxVar("serialString", tmpVar, SYNC_SINGLE);		
		if (ret != 0)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"�����д�ʱʧ�ܣ�������:%s",serialString);
			theJSLog.writeLog(LOG_CODE_DR_SYSC_IDXVAR_ERR,erro_msg);

			dr_AbortIDX();
			return -1;
		}
		if(drStatus == 1)
		{
			//serialString = tmpVar;			//ͬ�������ַ���,��ϵͳ�Ǹ�ֵ����ϵͳ��ȡֵ
			strcpy(serialString,tmpVar);
			//m_AuditMsg = tmpVar;			//Ҫ�ٲõ��ַ���
		}

		theJSLog<<"���ε�ͬ����serialString:"<<serialString<<endi;//for test

		// <5> ����ʵ����  ������ϵͳע���IDX��ģ����ò�����
		//��ϵͳ��index manager���IDX��������󣬰�ʹ�øú���ע������������Ϊģ��ĵ��ò���trigger��Ӧ�Ľ���
		snprintf(tmpVar, sizeof(tmpVar), "%ld", module_process_id);
		ret = dr_SyncIdxVar("@@ARG", tmpVar,SYNC_SINGLE);  
		if(ret !=0)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"����ʵ����ʧ��:%s",tmpVar);
			theJSLog.writeLog(LOG_CODE_DR_SYSC_IDXVAR_ERR,erro_msg);

			dr_AbortIDX();  
			return -1;
		}
		
		
		// <6> Ԥ�ύindex  �˹ؼ������ڽ�ƽ̨��ǰ�ڴ��е��������д�����
		ret = dr_SyncIdxVar("@@FLUSH","SUCCESS",SYNC_SINGLE);  
		if (ret != 0 )
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"Ԥ�ύindexʧ��");
			theJSLog.writeLog(LOG_CODE_DR_SYSC_IDXVAR_ERR,erro_msg);

			dr_AbortIDX();
			return -1;
		}
		
		
		// <7> �ύindex  	�ύIndex����index�ļ���������ɱ�־
		ret = dr_CommitIDX();  
		if(ret != 0)  
		{  
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"�ύindexʧ��,����ֵ=%d",ret);
			theJSLog.writeLog(LOG_CODE_DR_COMMIT_IDX_ERR,erro_msg);

			dr_AbortIDX();  
			return -1;  
		}

		//��ϵͳ����Ŀ¼
		//if(!m_syncDr.isMaster())thelog<<"��ϵͳSerialString��"<<m_SerialString<<endi;

		return ret;

}

//�ٲ��ַ���
 int CDsum::IsAuditSuccess(const char* dealresult)
 {
		int auitStatus = 0, ret = 0;

		if(!m_enable)
		{
			return ret;
		}

		ret = dr_Audit(dealresult);
		if(2 == ret )
		{
			//theJSLog << "�����ٲ�ʧ��,���:" << ret <<"���ˣ�"<<dealresult<< endw;
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"�����ٲ�ʧ��,���:%d,����:%s",ret,dealresult);
			theJSLog.writeLog(LOG_CODE_DR_AUDIT_ERR,erro_msg);

			dr_AbortIDX();
		}
		else if (3 == ret)
		{
			theJSLog<<"�����ٲó�ʱ..."<<endw;
			dr_AbortIDX();
		}
		else if(4 == ret)
		{
			theJSLog<<"�Զ�idx�쳣��ֹ..."<<endw;
			dr_AbortIDX();
		}
		else if(1 == ret)
		{
			ret = dr_CommitSuccess();
			if(ret != 0)
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"ҵ��ȫ���ύʧ��(����ƽ̨),����ֵ=%d",ret);
				theJSLog.writeLog(LOG_CODE_DR_COMMIT_SUCCESS_ERR,erro_msg);

				//dr_AbortIDX();
			}
			theJSLog<<"ret = "<<ret<<"�ٲóɹ�...\n�ٲ����ݣ�"<<dealresult<<endi;
		}
		else
		{
			theJSLog<<"δ֪ret="<<ret<<"	�ٲ����ݣ�"<<dealresult<<endw;
			dr_AbortIDX();
		}
	
	return ret;
 }

bool CDsum::CheckTriggerFile()
{
	int ret = 0;
	if(access(m_triggerFile.c_str(),F_OK) != 0)	return false;

	theJSLog<< "��鵽trigger�ļ�,��ɾ��"<< m_triggerFile <<endi;

	ret = remove(m_triggerFile.c_str());	
	if(ret) 
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"ɾ��trigger�ļ�[%s]ʧ��: %s",m_triggerFile,strerror(errno));
		theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
		return false;
	}

	return true;
}

*/

#endif
