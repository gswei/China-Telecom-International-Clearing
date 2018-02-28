
#ifndef __SUM_MON_CPP__
#define __SUM_MON_CPP__

#include "CMsum.h"

CMsum::CMsum()
{
	memset(szSumMonth,0,sizeof(szSumMonth));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(sql,0,sizeof(sql));
	iSourceCount = 0;
	//m_enable = false;
	//drStatus = 2;
	pMonthList = NULL;
}

CMsum::~CMsum()
{
	if(pMonthList != NULL)
	{
		delete[] pMonthList;   //�ͷŽṹ��ָ������

	}
	
	/*
	if(m_enable) 
	{
		int ret = dr_ReleasePlatform();
		if(ret != 0)
		{
			char tmp[100] = {0};
			snprintf(tmp, sizeof(tmp), "�ͷ�����ƽ̨ʧ��,����ֵ=%d", ret);
			theJSLog<<tmp<<endw;
		}
	}
	*/
}

bool CMsum::init(int argc,char** argv)
{
	bool ret = true;

	if( !param_cfg.bOnInit() )		//���Ĳ�����Ҫ�Լ���ʼ��
	{
		string sErr;
		int nCodeId;
		param_cfg.getError(sErr,nCodeId);
		cerr<<"�������ýӿڳ�ʼ��ʧ�ܣ�������="<<nCodeId<<", ������Ϣ="<<sErr<<endl;
		return false;
	}
	

	char *pMldName;
	pMldName=strrchr(argv[0], '/');
	if(pMldName==NULL) pMldName=argv[0];
	else pMldName++;
			
	memset(module_id,0,sizeof(module_id));
	strcpy(module_id,pMldName);
	
	bool redo =false;							//�����»��ܺ������»��ܵı�־,ʵ��ID��һ��
	for(int i=1;i<argc;i++)
	{
		if(strcmp(argv[i],"-rm") == 0) 
		{
			redo = true;
		}
	}

	try
	{
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

			module_process_id += 2;
			if(redo) module_process_id++;
	
			stmt.close();
			conn.close();
	}
	catch(util_1_0::db::SQLException e)
	{ 
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"��ʼ��ʱ���ݿ��ѯ�쳣��%s(sql)",e.what(),sql);
			theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
			return false;
	}
/*
	bool flag = true;
	for(int i=1;i<argc;i++)
	{
		if(strcmp(argv[i],"-k") == 0)
		{
			theJSLog<<"ģ��["<<module_id<<"]����������..."<<endi;
			flag = false;
			m_enable = false;
			break;
		}	
	}
	
	if(flag && !redo)		//�������������
	{
		if(!drInit())  return false;
	}
*/

	if(!(rtinfo.connect()))
	{
		return false;
	}
	short status;
	rtinfo.getDBSysMode(petri_status);
	//cout<<"petri status:"<<petri_status<<endl;

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
	

	 //��ʼ���ڴ���־�ӿ�
	 bool bb = initializeLog(argc,argv,false);  //�Ƿ����ģʽ
	 if(!bb)
	 {
			return false;
	 }

	 theJSLog.setLog(szLogPath, atoi(szLogLevel),"MONTH_SUM","SUMMARY", 001);	//�ļ���־�ӿڣ��������ڴ���־�ӿ�
	
	 return ret ;
}


bool CMsum::init(char* source_id, char* source_group_id,char* month)
{		
	bool  ret = true;
	
	strcpy(szSumMonth,month);
	
	szSumMonth[6] = '\0';		//��ʾ��ֹ����

	if(!(dbConnect(conn)))
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init()  �������ݿ�ʧ�� connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
		return false ;
	}
		
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

			pMonthList = new SMonthList[iSourceCount];   //��Ч������Դ����
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
			pMonthList = new SMonthList[1];
			
			//���ػ��ܶ����ͨ������ԴID,  ����ͨ������ԴID�ҵ���������Դ��
			char group[10];
			memset(group,0,sizeof(group));
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select source_group from c_source_group_config where source_id = '%s'",source_id);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>group;
			
			ret = loadSumConfig(source_id,group,0);
			if(ret)  return false;
			else
			{
				ret = true;
			}
		}
		else
		{

			ret = init(month);
			if(ret ==  false)  return false;

		}
	
		stmt.close();

	}catch(SQLException e)
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init ���ݿ����%s(sql)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

		return false;
	 }

	conn.close();

	return ret;
}

//������������Դ��Ϣ
bool CMsum::init(char* month)
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
		pMonthList = new SMonthList[iSourceCount];

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
		sprintf(erro_msg,"init ���ݿ����%s(%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

		return false;
	 }
	
	//conn.close();

	return true;

}

//�������е�ͳ��������Ϣ
int CMsum::loadSumConfig(char *source_id, char *source_group_id,int pos)
{
	
	strcpy(pMonthList[pos].szSourceId,source_id);
	strcpy(pMonthList[pos].szSourceGrpId,source_group_id);
	
 try
 {
	int ret = 0;
	Statement stmt = conn.createStatement();
	memset(sql,0,sizeof(sql));
	sprintf(sql, "select DAYSUMT,DAYSUMT_TCOL,MONSUMT,MONSUMT_TCOL from C_SUMTABLE_DEFINE where sourceid = '%s'",source_id );
	stmt.setSQLString(sql);
	stmt.execute();

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

	pMonthList[pos].tableItem = scom;			

	theJSLog<<"����ԴID="<<source_id<<" ԭ��ID="<<scom.iOrgSumt<<" ԭ�����="<<scom.iOrgTableName<<" ԭʼ��ʱ���ֶ�"<<scom.szOrgSumtCol
			<<" Ŀ���ID="<<scom.iDestSumt<<" Ŀ������="<<scom.iDestTableName<<" Ŀ���ʱ���ֶ�"<<scom.szDestSumtCol<<endi;
	
	ret = getItemInfo(scom,pMonthList[pos].vItemInfo);		//��ȡ���������ͳ���ֶε�ֵ
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
void CMsum::setDate(char* date)
{
	strcpy(szSumMonth,date);
}


//�ж��ջ��������������ṹ SMonthList �����е��ļ����ﵽ�������������ݲ�ͬ����������ز�ֵͬ��
//ע���������ܺ��ػ��ܵķ���ֵ
int CMsum::checkMonthCondition(SMonthList &Smonth,char *month)
{
	int ret = 0,cnt = 0;
try
{
	Statement stmt = conn.createStatement();
	
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(1) from D_SUMMARY_RESULT where sourceid = '%s' and sumtype ='M' and sumdate = '%s' and deal_flag = 'Y' ",Smonth.szSourceId,month);
	//cout<<"sql = "<<sql<<endl;
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>ret;
	if(ret)
	{
		theJSLog<<"�����Ѿ������»���"<<endi;
		return 1;
	}

	char tmp_month[6+1];
	memset(tmp_month,0,sizeof(tmp_month));
	strcpy(tmp_month,month);
	tmp_month[6] = '\0';

	//��ѯ�����Ƿ����
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(*) from D_CHECK_FILE_DETAIL where source_id like '%s%s' and check_type = 'AUD' and rate_cycle = '%s' and cycle_flag = '1'",Smonth.szSourceId,"%",tmp_month);
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>ret;
	if(ret == 0)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"����Դ[%s]��������[%s]��û����",Smonth.szSourceId,tmp_month);
		theJSLog<<erro_msg<<endw;

		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME,DEAL_FLAG,DEAL_DESC)values('%s','%s','%s',0,to_char(sysdate,'yyyymmddhh24miss'),'N','%s')",Smonth.szSourceId,"M",tmp_month,erro_msg); 
		stmt.setSQLString(sql);
		stmt.execute();
		stmt.close();

		return 2;	
	}
	
	//�����µķ��ʺ��ʱ���,���ջ��ܺ��ʱ��Ƚ�
	vector<string> vft;
	string ft;
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select distinct(file_time) from D_CHECK_FILE_DETAIL where source_id like '%s%s' and check_type = 'AUD' and rate_cycle = '%s'  and cycle_flag=0 order by  file_time desc ",Smonth.szSourceId,"%",tmp_month);
	stmt.setSQLString(sql);
	stmt.execute();
	while(stmt>>ft)
	{
		vft.push_back(ft);
	}
	
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(*) from D_SUMMARY_RESULT where sourceid = '%s' and sumtype in('D','RD') and DEAL_FLAG = 'Y' and sumdate = :1 ",Smonth.szSourceId);
	for(int  i = 0;i<vft.size();i++)
	{
		stmt.setSQLString(sql);
		stmt<<vft[i];
		stmt.execute();
		stmt>>ret;
		if(ret == 0)
		{		
			//theJSLog<<"����Դ["<<Smonth.szSourceId<<"]������["<<vft[i]<<"]��û������ջ���..."<<endw;
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"����Դ[%s]������[%s]��û������ջ���",Smonth.szSourceId,vft[i]);
			theJSLog<<erro_msg<<endw;

			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME,DEAL_FLAG,DEAL_DESC)values('%s','%s','%s',0,to_char(sysdate,'yyyymmddhh24miss'),'N','%s')",Smonth.szSourceId,"M",tmp_month,erro_msg); 
			stmt.setSQLString(sql);
			stmt.execute();
			stmt.close();

			return 2;
		}
	}
	
	vft.clear();
	stmt.close();
	
	theJSLog<<"������������..."<<endi;

 }catch(SQLException e)
 {
	memset(erro_msg,0,sizeof(erro_msg));
	sprintf(erro_msg,"checkMonthCondition ���ݿ����%s (%s)",e.what(),sql);
	theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

	return -1;
 }

	return 0;
}

int CMsum::sum(int deal_type,SMonthList &Smonth,char *month,bool del)
{
		int ret = 1;
		char sql[2048];
		long cnt = 0,last_cnt = 0;
/*
		char time_col[30];
		memset(time_col,0,sizeof(time_col));
		
		//cout<<"month = "<<month<<endl;
		//��ѯ�ջ��ܼ�¼�����������ж�������ʱ���ֶ�ԭʼ����ORGSUMT_TCOL�ֶζ�Ӧ������ֶΣ�SItemPairͨ��Դͷ��Ŀ��
		for(int i = 0;i<Smonth.vItemInfo.size();i++)		//��ѯ���ͳ����ͳ��ά�ȵ�
		{
			SItem fromItem = Smonth.vItemInfo[i].fromItem;
			if(strcmp(fromItem.szItemName,Smonth.tableItem.szOrgSumtCol) == 0)
			{
						strcpy(time_col,Smonth.vItemInfo[i].toItem.szItemName);
						cnt = 1;
						break;
			}
		}
		
		if(cnt == 0)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"�ֶ�[%s]û����ͳ�Ƹ�ʽ��[%s]����",Smonth.tableItem.szOrgSumtCol,Smonth.tableItem.iOrgTableName);
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
			return -1 ;
		}
*/

	char sum_type[2];
	memset(sum_type,0,sizeof(sum_type));
	if(deal_type == 1)
	{
		strcpy(sum_type,"M");
	}
	else if(deal_type == 2)
	{
		strcpy(sum_type,"RM");
	}

	Statement stmt;
	try
	{	
		stmt = conn.createStatement();
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,DEALTIME,DEAL_FLAG) values('%s','%s','%s',to_char(sysdate,'yyyymmddhh24miss'),'W')",Smonth.szSourceId,sum_type,month); 
		stmt.setSQLString(sql);
		stmt.execute();
		stmt.commit();

		//if(del)				//��ɾ��ԭ�Ȼ��ܱ������,�ж�������ʱ���ֶ�ԭʼ����ORGSUMT_TCOL�ֶζ�Ӧ������ֶΣ�SItemPairͨ��Դͷ��Ŀ��
		//{		
			theJSLog<<"��ɾ���·�["<<month<<"]������"<<endi;
			memset(sql,0,sizeof(sql));
			sprintf(sql,"delete from %s_%s ",Smonth.tableItem.iDestTableName,month);
			cout<<"ɾ�����ܱ���sql="<<sql<<endl;
			
			stmt.setSQLString(sql);
			stmt.execute();
		//}
		//else
		//{		
			
		//	memset(sql,0,sizeof(sql));
		//	sprintf(sql,"select count(1) from %s_%s ",Smonth.tableItem.iDestTableName,month);
		//	stmt.setSQLString(sql);
		//	stmt.execute();
		//	stmt>>last_cnt;
			//cout<<"������ǰ���ݼ�¼����:"<<last_cnt<<endl;
		//}
		
		ret = getSql(Smonth.tableItem,Smonth.vItemInfo,month,sql,2);			//ƴ��sql
		if(ret == -1)
		{		
				return -1 ;
		}	
		theJSLog<<"ƴ�ӵ�sql = "<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(1) from  %s_%s ",Smonth.tableItem.iDestTableName,month);		
		cout<<"ͳ���»��ܼ�¼����sql:"<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>cnt;	

		//memset(m_AuditMsg,0,sizeof(m_AuditMsg));	
			
		//д�»�����־
		theJSLog<<"������¼����:"<<(cnt-last_cnt)<<"	д���ܽ����D_SUMMARY_RESULT"<<endi;
		memset(sql,0,sizeof(sql));
		//sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME)values('%s','%s','%s',%ld,to_char(sysdate,'yyyymmddhh24miss'))",Smonth.szSourceId,sum_type,month,(cnt-last_cnt));
		sprintf(sql,"update D_SUMMARY_RESULT set DEAL_FLAG='Y',SUMCOUNT=%d,DEALTIME=to_char(sysdate,'yyyymmddhh24miss') where SOURCEID='%s' and SUMDATE='%s' and DEAL_FLAG='W' ",(cnt-last_cnt),Smonth.szSourceId,month);
		cout<<"������־sql = "<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		stmt.close();
		
		ret = 0;

	}catch(SQLException e)
	{
		stmt.rollback();
		
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"sum() ���ݿ����%s (%s)",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
		
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update D_SUMMARY_RESULT set DEAL_FLAG='N',DEAL_DESC='%s',DEALTIME=to_char(sysdate,'yyyymmddhh24miss') where SOURCEID='%s' and SUMDATE='%s' and DEAL_FLAG='W' ",e.what(),Smonth.szSourceId,month);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt.commit();	
		//cout<<"sql = "<<sql<<endl;

		return -1;
	}

	return ret ;
}

//Summary -rd-t YYYYMMDD �����ջ���
int CMsum::run(int deal_type,bool del)
{
	int  ret = -1;
	
	rtinfo.getDBSysMode(petri_status);
	if(petri_status == DB_STATUS_OFFLINE)	
	{
		theJSLog<<"��ǰ���ݿ�Ϊ����̬,�޷������»���"<<endw;
		return ;
	}
	
	if(!(dbConnect(conn)))
	{
		sprintf(erro_msg,"run()  �������ݿ�ʧ�� connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
		return -1 ;
	}
	
	if(deal_type == 1)
	{
		theJSLog<<"####################�����»����·�["<<szSumMonth<<"]������#################"<<endi;
	}
	else if(deal_type == 2)
	{
		theJSLog<<"###################�����»����·�["<<szSumMonth<<"]������##################"<<endi;
	}
	else
	{
		theJSLog<<"##########�������ʹ���:"<<deal_type<<"#################"<<endw;
		return -1;
	}

	for(int i = 0;i<iSourceCount;i++)
	{			
		theJSLog<<"########��������Դ"<<pMonthList[i].szSourceId<<"##########"<<endi;
			
		if(deal_type == 1)				
		{														
				ret = checkMonthCondition(pMonthList[i],szSumMonth);	//ÿ������Դ�ж��Ƿ�ﵽ��������
				switch(ret)
				{
					case 0:								

					case 1:									
						ret = sum(deal_type,pMonthList[i],szSumMonth,del);
						break;
					case 2:								//�ջ��ܴ�����
						break;
					case -1:							//���ݿ��쳣
						break;
					default:
						break;
				}
		}
		else if(deal_type == 2)							//��������Ҫ��������
		{
				//ret = 1;
				ret = sum(deal_type,pMonthList[i],szSumMonth,del);
		}		
	}

	conn.close();
	
	if(deal_type == 1)
	{
	    theJSLog<<"################�����»����·����#########################"<<endi;
	}
	else if(deal_type == 2)
	{
		theJSLog<<"################�����»����·����#########################"<<endi;
	}

	return ret ;
}

/*
//���ֳ�ʼ��
bool CMsum::drInit()															//���ڴ�������ʵ��
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
int CMsum::drVarGetSet(char* serialString)
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
			theJSLog<<"��������л���ʧ��,����ֵ="<<ret<<endw;
			return -1;  
		} 
		//��ʼ��index  
		ret = dr_InitIDX();  
		if(ret != 0)  
		{  
			theJSLog<<"��ʼ��indexʧ��,����ֵ=" <<ret<<endw;
			dr_AbortIDX();
			return -1;  
		}

		snprintf(tmpVar, sizeof(tmpVar), "%s", serialString);
		//��ϵͳ��Ҫͬ����index ����ֵ�ԡ�д������ƽ̨ά����index �ļ���
		//��ϵͳ���øú����Ľ���ǣ�var��ú���ϵͳһ�������������ֵ��	SYNC_SINGLE��ʾע�ᵥһ���������
		ret = dr_SyncIdxVar("serialString", tmpVar, SYNC_SINGLE);		
		if (ret != 0)
		{
			theJSLog<<"�����д�ʱʧ�ܣ���������["<<serialString<<"]"<<endw;
			dr_AbortIDX();
			return -1;
		}
		//serialString = tmpVar;			//ͬ�������ַ���,��ϵͳ�Ǹ�ֵ����ϵͳ��ȡֵ
		strcpy(serialString,tmpVar);
		//m_AuditMsg = tmpVar;			//Ҫ�ٲõ��ַ���

		// <5> ����ʵ����  ������ϵͳע���IDX��ģ����ò�����
		//��ϵͳ��index manager���IDX��������󣬰�ʹ�øú���ע������������Ϊģ��ĵ��ò���trigger��Ӧ�Ľ���
		snprintf(tmpVar, sizeof(tmpVar), "%ld", module_process_id);
		ret = dr_SyncIdxVar("@@ARG", tmpVar,SYNC_SINGLE);  
		if(ret !=0)
		{
			theJSLog<<"����ʵ����ʧ�ܣ�"<<tmpVar<<endw;
			dr_AbortIDX();  
			return -1;
		}
		
		
		// <6> Ԥ�ύindex  �˹ؼ������ڽ�ƽ̨��ǰ�ڴ��е��������д�����
		ret = dr_SyncIdxVar("@@FLUSH","SUCCESS",SYNC_SINGLE);  
		if (ret != 0 )
		{
			theJSLog<<"Ԥ�ύindexʧ��"<<endw;
			dr_AbortIDX();
			return -1;
		}
		
		
		// <7> �ύindex  	�ύIndex����index�ļ���������ɱ�־
		ret = dr_CommitIDX();  
		if(ret != 0)  
		{  
			theJSLog<<"�ύindexʧ��,����ֵ="<<ret<<endw;
			dr_AbortIDX();  
			return -1;  
		}

		//��ϵͳ����Ŀ¼
		//if(!m_syncDr.isMaster())thelog<<"��ϵͳSerialString��"<<m_SerialString<<endi;

		theJSLog<<"���ε�ͬ����serialString:"<<serialString<<endi;//for test

		return ret;

}

//�ٲ��ַ���
 int CMsum::IsAuditSuccess(const char* dealresult)
 {
		int auitStatus = 0, ret = 0;

		if(!m_enable)
		{
			return ret;
		}

		ret = dr_Audit(dealresult);
		if(2 == ret )
		{
			theJSLog << "�����ٲ�ʧ��,���:" << ret <<"���ˣ�"<<dealresult<< endw;
			dr_AbortIDX();
			//return false;
		}
		else if (3 == ret)
		{
			theJSLog<<"�����ٲó�ʱ..."<<endw;
			dr_AbortIDX();
			//return false;
		}
		else if(4 == ret)
		{
			theJSLog<<"�Զ�idx�쳣��ֹ..."<<endw;
			dr_AbortIDX();
			//return false;
		}
		else if(1 == ret)
		{
			ret = dr_CommitSuccess();
			if(ret != 0)
			{
				theJSLog << "ҵ��ȫ���ύʧ��(����ƽ̨)" << endw;
				dr_AbortIDX();
				//return false;
			}
			theJSLog<<"ret = "<<ret<<"�ٲóɹ�...\n�ٲ����ݣ�"<<dealresult<<endi;
			//return true;
		}
		else
		{
			theJSLog<<"δ֪ret="<<ret<<"	�ٲ����ݣ�"<<dealresult<<endw;
			dr_AbortIDX();
			//return false;
		}
	
	return ret;
 }

bool CMsum::CheckTriggerFile()
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


