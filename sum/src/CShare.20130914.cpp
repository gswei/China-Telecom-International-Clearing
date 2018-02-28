
#ifndef __SUM_SHARE_CPP__
#define __SUM_SHARE_CPP__

#include "CShare.h"

CShare::CShare()
{
	memset(szSumMonth,0,sizeof(szSumMonth));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(sql,0,sizeof(sql));
	iSourceCount = 0;
	m_enable = false;
	pShareList = NULL;
}

CShare::~CShare()
{
	if(pShareList != NULL)
	{
		delete[] pShareList;   //�ͷŽṹ��ָ������
	}

	if(m_enable) 
	{
		int ret = dr_ReleasePlatform();
		if(ret != 0)
		{
			char tmp[100] = {0};
			snprintf(tmp, sizeof(tmp), "�ͷ�����ƽ̨ʧ��,����ֵ=%d", ret);
			theJSLog<<tmp<<endi;
		}
	}

}

bool CShare::init(int argc,char** argv)
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
	
	bool redo =false;			//����̯�ֻ��ܺ�����̯�ֻ��ܵı�־,ʵ��ID��һ��
	for(int i=1;i<argc;i++)
	{
		if(strcmp(argv[i],"-rf") == 0) 
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

			module_process_id += 4;
			if(redo) module_process_id++;
	
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

	if(!drInit())	return false;

	if(!(rtinfo.connect()))
	{
		return false;
	}
	short status;
	rtinfo.getDBSysMode(petri_status);
	cout<<"petri status:"<<petri_status<<endl;

	 // �Ӻ��Ĳ��������ȡ��־��·��������
	 char sParamName[256],szLogPath[PATH_NAME_LEN+1],szLogLevel[10],sql_path[1024],bak_path[1024];
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
			//cout<<"��ʼ���ڴ���־�ӿ�ʧ��"<<endl;
			return false;
	 }

	 theJSLog.setLog(szLogPath, atoi(szLogLevel),"SETT_SUM","SUMMARY", 001);	//�ļ���־�ӿڣ��������ڴ���־�ӿ�
	 
	 return ret ;
}

bool CShare::init(char* source_id, char* source_group_id,char* month)
{		
	bool  ret = true;
	
	strcpy(szSumMonth,month);

	if(!(dbConnect(conn)))
	{
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

			pShareList = new SShareList [iSourceCount];   //��Ч������Դ����
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
			pShareList = new SShareList [1];
			
			//���ػ��ܶ����ͨ������ԴID,  ����ͨ������ԴID�ҵ���������Դ��
			char group[10];
			memset(group,0,sizeof(group));
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select source_group from c_source_group_config where source_id = '%s'",source_id);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>group;
			
			stmt.close();

			ret =loadSumConfig(source_id,group,0);
			if(ret)  return false;
		}

		else
		{
			ret = init(month);
			if(ret ==  false)  return false;

		}
	
	
	}catch(SQLException e)
	 {
		sprintf(erro_msg,"init ���ݿ����%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

		return false;
	 }

	conn.close();

	return ret;
}

//������������Դ��Ϣ
bool CShare::init(char* month)
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
		pShareList = new SShareList [iSourceCount];

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
		sprintf(erro_msg,"init ���ݿ����%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

		return false;
	 }
	
	//conn.close();

	return true;

}

//�������е�ͳ��������Ϣ -1����  0��ʾ����
int CShare::loadSumConfig(char *source_id, char *source_group_id,int pos)
{
	
	strcpy(pShareList[pos].szSourceId,source_id);
	strcpy(pShareList[pos].szSourceGrpId,source_group_id);
	
 try
 {
	int ret = 0;

	Statement stmt = conn.createStatement();
	memset(sql,0,sizeof(sql));
	sprintf(sql, "select MONSUMT,MONSUMT_TCOL,SETTSUMT,SETTSUMT_TCOL from C_SUMTABLE_DEFINE where sourceid = '%s'",source_id );
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

	pShareList[pos].tableItem = scom;			

	theJSLog<<"����ԴID="<<source_id<<" ԭ��ID="<<scom.iOrgSumt<<" ԭ�����="<<scom.iOrgTableName<<" ԭʼ��ʱ���ֶ�"<<scom.szOrgSumtCol
		<<" Ŀ���ID="<<scom.iDestSumt<<" Ŀ������="<<scom.iDestTableName<<"  Ŀ���ʱ���ֶ�"<<scom.szDestSumtCol<<endi;
	
	ret = getItemInfo(scom,pShareList[pos].vItemInfo);		//��ȡ���������ͳ���ֶε�ֵ
	if(ret)  return -1;
	
	ret = loadShareConfig(source_id,pShareList[pos]);		//����̯�����������û���ռ�ñ�
	if(ret)  return -1;
	
 }
 catch (SQLException e)
 {
	sprintf(erro_msg,"loadSumConfig ���ݿ����%s",e.what());
	theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

	return -1;
 }

	return  0;
}


//����̯�����������û���ռ�ñ�
int CShare::loadShareConfig(char*source_id,SShareList &Share)
{
	int ret = 0,settle_type  = 0,cnt = 0;
	char settle_id[6],varname[20],varvalue[20];

try
{
	Statement stmt = conn.createStatement();

	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(1) from c_settlesum_config where source_id = '%s'",source_id);
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>cnt;
	if(cnt == 0)
	{
		sprintf(erro_msg,"����Դ[%s]û�����÷�̯ID",source_id);
		theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
		strcpy(Share.szFlag,"N");
		return -1;
	}

	memset(sql,0,sizeof(sql));
	sprintf(sql,"select settle_type,settle_id from c_settlesum_config where source_id = '%s'",source_id);
	stmt.setSQLString(sql);
	stmt.execute();
	
	memset(settle_id,0,sizeof(settle_id));
	stmt>>settle_type>>settle_id;
	Share.szSettleType = settle_type;
	strcpy(Share.szSettleId,settle_id);

	sprintf(sql,"select varname,varvalue from C_SETTLESUM_DETAIL where settle_id = '%s' ",settle_id);
	stmt.setSQLString(sql);
	stmt.execute();
	
	memset(varname,0,sizeof(varname));
	memset(varvalue,0,sizeof(varvalue));

	if(settle_type == 1)
	{
		while(stmt>>varname>>varvalue)
		{
			if(strcmp(varname,"SETTLESUM_COL") == 0)		
			{
				strcpy(Share.stShareFee.settlesum_col,varvalue);
			}
			else if(strcmp(varname,"SETTLEOUT_FOMULAID") == 0)
			{
				strcpy(Share.stShareFee.settle_formula,varvalue);
			}
			else if(strcmp(varname,"SETTLEOUT_FOMULAPARAM") == 0)
			{
				strcpy(Share.stShareFee.settle_formula_param,varvalue);
			}
			else 
			{
				//cout<<"����������["<<varname<<"]���Ϸ�"<<endl;
				sprintf(erro_msg,"����������[%s]���Ϸ�",varname);
				theJSLog.writeLog(LOG_CODE_PARAM_WRON,erro_msg);

				return -1;
			}
		}
	}
	else if(settle_type == 2)
	{
		while(stmt>>varname>>varvalue)
		{
			if(strcmp(varname,"BASE_COL") == 0)
			{
				strcpy(Share.stSharePercent.base_col,varvalue);
			}
			else if(strcmp(varname,"PROVINCE_COL") == 0)
			{
				strcpy(Share.stSharePercent.province_col,varvalue);
			}
			else if(strcmp(varname,"PERCENT_POS") == 0)
			{
				Share.stSharePercent.percent_pos = atoi(varvalue);
				if(Share.stSharePercent.percent_pos == 0) Share.stSharePercent.percent_pos = 2; //Ĭ��Ϊ2
			}
			else if(strcmp(varname,"OUTTABLE_NAME") == 0)
			{
				strcpy(Share.stSharePercent.outtable_name,varvalue);
			}
			else
			{
				sprintf(erro_msg,"ռ��������[%s]���Ϸ�",varname);
				theJSLog.writeLog(LOG_CODE_PARAM_WRON,erro_msg);
				return -1;
			}
		}

	}
	else
	{
		stmt.close();
		sprintf(erro_msg,"̯���������ʹ���(1������ú�,2����ռ��) :%d",settle_type);
		theJSLog.writeLog(LOG_CODE_PARAM_WRON,erro_msg);
		return -1;

	}
	
	stmt.close();

 }catch (SQLException e)
 {
	sprintf(erro_msg,"loadShareConfig ���ݿ����%s",e.what());
	theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

	return -1;
 }
	
   return ret ;

}


//��������
void CShare::setDate(char* date)
{
	strcpy(szSumMonth,date);
}


//�ж��ջ��������������ṹ SShareList  �����е��ļ����ﵽ�������������ݲ�ͬ����������ز�ֵͬ��
//ע���������ܺ��ػ��ܵķ���ֵ
int CShare::checkSettCondition(SShareList  &Smonth,char *month)
{
	int ret = 0;
try
{
	Statement stmt = conn.createStatement();
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(1) from D_SUMMARY_RESULT where sourceid = '%s' and sumtype = 'F' and sumdate = '%s'",Smonth.szSourceId,month);
	//cout<<"sql = "<<sql<<endl;
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>ret;
	if(ret)
	{
		stmt.close();
		theJSLog<<"�����Ѿ�����̯�ֻ���"<<endi;
		return 0;
	}

	//***********************************

	stmt.close();
	
	
 }catch(SQLException e)
 {
	 memset(erro_msg,0,sizeof(erro_msg));
	sprintf(erro_msg,"checkSettCondition ���ݿ����%s",e.what());
	theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

	return -11;
 }

	return 1;
}

int CShare::sum(int deal_type,SShareList  &Smonth,char *month,bool del)
{
		int ret = 1;
		char sql[1024];
		long cnt = 0,last_cnt = 0;

		char time_col[30];
		memset(time_col,0,sizeof(time_col));
		
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
			//theJSLog<<"�ֶ�"<<Smonth.tableItem.szOrgSumtCol<<"û����ͳ�Ƹ�ʽ��"<<Smonth.tableItem.iOrgTableName<<"����"<<endi;
			sprintf(erro_msg,"�ֶ�[%s]û����ͳ�Ƹ�ʽ��[%s]����",Smonth.tableItem.szOrgSumtCol,Smonth.tableItem.iOrgTableName);
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
			return -1 ;
		}

	Statement stmt;
	try
	{
		stmt = conn.createStatement();
		
		if(del)				//��ɾ��ԭ�Ȼ��ܱ������,�ж�������ʱ���ֶ�ԭʼ����ORGSUMT_TCOL�ֶζ�Ӧ������ֶΣ�SItemPairͨ��Դͷ��Ŀ��
		{		
			theJSLog<<"ɾ���·�"<<month<<"������"<<endi;
			memset(sql,0,sizeof(sql));
			sprintf(sql,"delete from %s where %s = '%s'",Smonth.tableItem.iDestTableName,time_col,month);
			cout<<"ɾ�����ܱ���sql="<<sql<<endl;
			
			stmt.setSQLString(sql);
			stmt.execute();

			
			if(Smonth.szSettleType == 2)
			{
				theJSLog<<"ɾ��ռ�Ȼ��ܱ��µ�����"<<endi;
				SSharePercent percent = Smonth.stSharePercent;

				memset(sql,0,sizeof(sql));
				sprintf(sql,"delete from %s where SETTLEMON = '%s' and source_id = '%s'",percent.outtable_name,month,Smonth.szSourceId);
				cout<<"ɾ��ռ��sql:"<<sql<<endl;
				stmt.setSQLString(sql);
				stmt.execute();
			}
		}
		else
		{		
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from %s where %s = '%s'",Smonth.tableItem.iDestTableName,time_col,month);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>last_cnt;
			//cout<<"������ǰ���ݼ�¼����:"<<last_cnt<<endl;
		}
		
		ret = getSql(Smonth.tableItem,Smonth.vItemInfo,month,sql);			//ƴ��sql
		if(ret == -1)
		{		
				return -1 ;
		}	
		theJSLog<<"ƴ�ӵ�sql = "<<sql<<endi;
		stmt.setSQLString(sql);
		stmt.execute();
		
		//��ȡsql��ѯ �ٲ�����Ϊͳ���� ,���ݽ��������ȡ�ٲ�����,���������ǰ�������ݲ�һ�� ���ٲû�ʧ��(����ǰ������Ҳ������)
		string strsql;
		strsql = "select ";
		for(int i = 0;i<Smonth.vItemInfo.size();i++)		
		{
			SItem toItem = Smonth.vItemInfo[i].toItem;
			if(toItem.iItemType == 0)
			{
				strsql.append("sum(").append(toItem.szItemName).append("),");
			}		
		}
		
		memset(sql,0,sizeof(sql));
		sprintf(sql," from  %s where %s = '%s'",Smonth.tableItem.iDestTableName,time_col,month);

		strsql = strsql.substr(0, strsql.length()-1);
		strsql.append(sql);

		theJSLog<<"��ѯ�����ͳ���������sql:"<<strsql<<endi;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"%s",strsql);

		stmt.setSQLString(sql);
		stmt.execute();
		string str  = "";
		memset(m_AuditMsg,0,sizeof(m_AuditMsg));
		strcpy(m_AuditMsg,Smonth.szSourceId);
		strcat(m_AuditMsg,"|");
		while(stmt>>str)
		{
			sprintf(m_AuditMsg,"%s%s,",m_AuditMsg,str);
		}
		
		//�ж��Ƿ�����
		char sum_type[2];
		memset(sum_type,0,sizeof(sum_type));
		strcpy(sum_type,"F");
		if(deal_type == 2)
		{
			strcpy(sum_type,"RF");
		}

		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(1) from  %s where %s = '%s'",Smonth.tableItem.iDestTableName,time_col,month);		
		//cout<<"ͳ��̯�ֻ��ܼ�¼����sql:"<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>cnt;
		
		ret = shareCal(Smonth,stmt);		//�����ܷ��û���ռ�� 
		if(ret == -1)
		{
				return ret;
		}

		if(!IsAuditSuccess(m_AuditMsg))				//�ٲ�ʧ��,�ع����ݿ�,ɾ����ʱ�ļ�
		{
			theJSLog<<"�ع����ݿ�,���������־"<<endi;
			stmt.rollback();
			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME)values('%s','%s','%s',%ld,to_char(sysdate,'yyyymmddhh24miss'))",Smonth.szSourceId,sum_type,month,-1);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt.close();
		}
		else
		{	
			theJSLog<<"������¼����:"<<(cnt-last_cnt)<<"	д���ܽ����D_SUMMARY_RESULT"<<endi;

			//д̯�ֻ�����־
			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME)values('%s','%s','%s',%ld,to_char(sysdate,'yyyymmddhh24miss'))",Smonth.szSourceId,sum_type,month,(cnt-last_cnt));
			//cout<<"������־sql = "<<sql<<endl;
			stmt.setSQLString(sql);
			stmt.execute();

			stmt.close();

			
		}		

	}catch(SQLException e)
	{
		stmt.rollback();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"sum() ���ݿ����%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

		return -1;
	}

	return ret ;

}

//�����ܷ��û���ռ��  ������ú�ռ�Ȳ�ѯ̯�ֱ��ʱ���ֶ�(C_SUMTABLE_DEFINE�����SETTSUMT_TCOL)��ͨ��ԭʼ��ʱ���ֶ���Ŀ���ʱ���ֶ�(ͳ�Ƽ�¼������ɾ��������¼����ʹ�ø��ֶ�)��ʵ��һ���ģ������Ǳ�����ͬ
int CShare::shareCal(SShareList  &Smonth,Statement &stmt)
{	
	int ret = 0;

try
{	
	//stmt = conn.createStatement();

	if(Smonth.szSettleType == 1)
	{
		theJSLog<<"�����ܷ���:"<<endi;
		double total_fee;
		//memset(total_fee,0,sizeof(total_fee));
		SShareFee fee = Smonth.stShareFee;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(%s) from %s where %s = '%s'",fee.settlesum_col,Smonth.tableItem.iDestTableName,Smonth.tableItem.szDestSumtCol,szSumMonth);
		cout<<"total_fee sql :"<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>total_fee;
		theJSLog<<"�ܷ���:"<<total_fee<<endi;
		
		//�����ٲ���Ϣ: �������� ��  �����ֶ� ���ú�
		sprintf(m_AuditMsg,"%s|%d,%s,%s,%.2f",m_AuditMsg,Smonth.szSettleType,Smonth.tableItem.iDestTableName,fee.settlesum_col,total_fee);
		
		//string tmp_fee;
		//tmp_fee.append(total_fee); ǿ�Ʊ���2ΪС��
		memset(sql,0,sizeof(sql));
		sprintf(sql,"update C_FORMULA_PARAM_DEF set param_value = '%.2lf' where FORMULA_ID = '%s' and PARAM_NAME = '%s'",total_fee,fee.settle_formula,fee.settle_formula_param);
		cout<<"update sql: "<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		
		//if(ret == false) theJSLog<<"��������ʧ��: "<<sql<<endi;
	}
	else
	{
		double total_base = 0;
		theJSLog<<"����ռ��:"<<endi;
		SSharePercent percent = Smonth.stSharePercent;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select sum(%s) from %s where  %s = '%s'",percent.base_col,Smonth.tableItem.iDestTableName,Smonth.tableItem.szDestSumtCol,szSumMonth);
		cout<<"�ܻ���sql��"<<sql<<endl;
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>total_base;			//ռ�Ȼ�������ͨ������

		//������ܹ�����Щʡ���ٷֱ�����ʡ���ڱ���
		int prov_cnt = 0;
		double prov_base = 0,cur_base = 0,before_base = 0;
		char prov_code[20],sql2[1024],tmp_base[20];
		
		Statement stmt2 = conn.createStatement();
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(count(1))from %s where %s = '%s' group by %s",Smonth.tableItem.iDestTableName,Smonth.tableItem.szDestSumtCol,szSumMonth,percent.province_col);
		cout<<"ʡ�ݸ���sql:"<<sql<<endl;
		stmt2.setSQLString(sql);
		stmt2.execute();
		stmt2>>prov_cnt;  //�����ʡ�ݸ���
		theJSLog<<"ʡ�ݸ���:"<<prov_cnt<<endi;

		memset(sql,0,sizeof(sql));
		sprintf(sql,"select %s,sum(%s)from %s where %s = '%s' group by %s",percent.province_col,percent.base_col,Smonth.tableItem.iDestTableName,Smonth.tableItem.szDestSumtCol,szSumMonth,percent.province_col);
		cout<<"��ʡ����ռ��sql:"<<sql<<endl;
		stmt2.setSQLString(sql);
		stmt2.execute();
		
		//�����ٲ���Ϣ �������� �� ʡ���ֶ�  �������  ʡ��:ռ��
		sprintf(m_AuditMsg,"%s|%d,%s,%s,%s|",m_AuditMsg,Smonth.szSettleType,Smonth.tableItem.iDestTableName,percent.province_col,percent.outtable_name);

		memset(sql2,0,sizeof(sql2));
		sprintf(sql2,"insert into %s(PROVINCE,PERCENT,PERBASE,PERORG,SETTLEMON,SOURCE_ID)values(:1,:2,:3,%lf,'%s','%s')",percent.outtable_name,total_base,szSumMonth,Smonth.szSourceId);
		stmt.setSQLString(sql2);

		for(int i = 0;i<prov_cnt-1;i++)
		{
			memset(prov_code,0,sizeof(prov_code));
			stmt2>>prov_code>>prov_base;
	
			cur_base = prov_base/total_base;
			cur_base *= 100;		
			sprintf(tmp_base,"%.*lf",percent.percent_pos,cur_base);
			
			sprintf(m_AuditMsg,"%s%s:%s,",m_AuditMsg,prov_code,tmp_base);

			sscanf(tmp_base,"%lf",&cur_base);
			before_base += cur_base;
			stmt<<prov_code<<tmp_base<<prov_base;
			stmt.execute();
		}
		
		//���һ�����ü��㣬��100-ǰ���ֵ
		memset(prov_code,0,sizeof(prov_code));
		stmt2>>prov_code>>prov_base;

		cur_base = 100 - before_base;
		sprintf(tmp_base,"%.*lf",percent.percent_pos,cur_base);

		sprintf(m_AuditMsg,"%s%s:%s,",m_AuditMsg,prov_code,tmp_base);

		sscanf(tmp_base,"%lf",&cur_base);
		stmt<<prov_code<<tmp_base<<prov_base;
		stmt.execute();

		stmt2.close();
	}

	//stmt.close();

  }catch(SQLException e)
   {
		stmt.rollback();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"shareCal() ���ݿ����%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

		return -1;
  }

	return ret ;
}


int CShare::run(int deal_type,bool del)
{
	int  ret = -1;
	
	rtinfo.getDBSysMode(petri_status);
	if(petri_status == DB_STATUS_OFFLINE)	return ;
	
	if(!(dbConnect(conn)))
	{
		sprintf(erro_msg,"run()  �������ݿ�ʧ�� connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
		return -1 ;
	}

	if(drStatus == 1)		//����ϵͳ
	{
		//���trigger�����ļ��Ƿ����
		bool flag = false;
		int times = 1;
		while(times < 31)
		{
			if(!CheckTriggerFile())
			{
				times++;
				sleep(10);
				continue;
			}
			flag = true;
		}
		
		if(!flag)	return ;

		memset(m_SerialString,0,sizeof(m_SerialString));
		ret = drVarGetSet(m_SerialString);
		if(ret)
		{
			theJSLog<<"��ϵͳͬ��ʧ��...."<<endi;
			return ;
		}
		
		//��ȡͬ������,����ԴID,�»�������,�������ܻ����ػ���,ɾ����־
		vector<string> data;		
		splitString(m_SerialString,"|",data,true,true);
		
		memset(szSumMonth,0,sizeof(szSumMonth));
		strcpy(szSumMonth,data[1].c_str());
		deal_type = atoi(data[2].c_str());
		del = atoi(data[3].c_str());

		if(deal_type == 1)
		{
			theJSLog<<"����̯�ֻ����·�["<<szSumMonth<<"]������"<<endi;
		}
		else if(deal_type == 2)
		{
			theJSLog<<"����̯�ֻ����·�["<<szSumMonth<<"]������"<<endi;
		}
		else
		{
			theJSLog<<"�������ʹ���:"<<deal_type<<endi;
			dr_AbortIDX();
			return ;
		}

		theJSLog<<"��������Դ:"<<data[0]<<endi;
		
		int pos = -1;
		for(int i = 0;i< iSourceCount;i++)
		{
			if(strcmp(pShareList[i].szSourceId,data[0].c_str()) == 0)
			{
				pos = i;
				break;
			}
		}

		if(pos == -1)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"û���ҵ�������Դ��Ϣ[%s]",data[0]);		//��������δ����
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				
			dr_AbortIDX();  
			return ;
		}

		ret = sum(deal_type,pShareList[pos],szSumMonth,del);
		if(ret == -1)
		{
			dr_AbortIDX();  
		}	
	}
	else
	{
		if(deal_type == 1)
		{
			theJSLog<<"����̯�ֻ����·�["<<szSumMonth<<"]������"<<endi;
		}
		else if(deal_type == 2)
		{
			theJSLog<<"����̯�ֻ����·�["<<szSumMonth<<"]������"<<endi;
		}
		else
		{
			theJSLog<<"���ʹ���"<<endi;
			return -1;
		}
	
		for(int i = 0;i<iSourceCount;i++)
		{			
			theJSLog<<"��������Դ"<<pShareList[i].szSourceId<<endi;

			//ÿ������Դ�ж��Ƿ�ﵽ��������
			ret = checkSettCondition(pShareList[i],szSumMonth);

			switch(ret)
			{
				case 0:						//��ʾ�Ѿ��������ܣ��������̯�ֻ���
					if(deal_type == 2)
					{			
						//����ͬ����Ϣ
						memset(m_SerialString,0,sizeof(m_SerialString));
						sprintf(m_SerialString,"%s|%s|%d|%d",pShareList[i].szSourceId,szSumMonth,deal_type,del);
						ret = drVarGetSet(m_SerialString);
						if(ret)
						{
							theJSLog<<"��ϵͳͬ��ʧ��...."<<endi;
							return ;
						}

						ret = sum(deal_type,pShareList[i],szSumMonth,del);
						if(ret == -1)
						{
							dr_AbortIDX();  
						}
					}
					else if(deal_type == 1)
					{
						theJSLog<<"���·�������̯�ֻ���,����������,��ʹ����������"<<endi;
					}

					break;

				case 1:						//��û������������»���
					if(deal_type == 1)
					{
						//����ͬ����Ϣ
						memset(m_SerialString,0,sizeof(m_SerialString));
						sprintf(m_SerialString,"%s|%s|%d|%d",pShareList[i].szSourceId,szSumMonth,deal_type,del);
						ret = drVarGetSet(m_SerialString);
						if(ret)
						{
							theJSLog<<"��ϵͳͬ��ʧ��...."<<endi;
							return ;
						}

						ret = sum(deal_type,pShareList[i],szSumMonth,del);
						if(ret == -1)
						{
							dr_AbortIDX();  
						}
					}
					else if(deal_type == 2)
					{
						theJSLog<<"���·ݻ�û����̯�ֻ��ܣ�������̯�ֻ���"<<endi;
					}

					break;

				case -11:		//���ݿ��쳣
					break;

				default:
					break;
			}

		}
	}

	conn.close();
	
	if(deal_type == 1)
	{
	    theJSLog<<"����̯�ֻ����·����"<<endi;
	}
	else if(deal_type == 2)
	{
		theJSLog<<"����̯�ֻ����·����"<<endi;
	}

	return ret ;
}

//���ֳ�ʼ��
bool CShare::drInit()			//���ڴ�������ʵ��
{
		//����ģ������ʵ��ID
		char tmp[32];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%ld",module_process_id);

		theJSLog << "��ʼ������ƽ̨,ģ����:"<< module_id<<" ʵ����:"<<tmp<<endi;

		int ret = dr_InitPlatform(module_id,tmp);
		if(ret != 0)
		{
			theJSLog << "����ƽ̨��ʼ��ʧ��,����ֵ=" << ret<<endi;
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
			theJSLog<<"��ȡ����ƽ̨״̬����: ����ֵ="<<drStatus<<endi;
			return false;
		}
		
		if(drStatus == 0)		theJSLog<<"��ǰϵͳ����Ϊ��ϵͳ"<<endi;
		else if(drStatus == 1)	theJSLog<<"��ǰϵͳ����Ϊ��ϵͳ"<<endi;
		else if(drStatus == 2)	theJSLog<<"��ǰϵͳ���÷�����ϵͳ"<<endi;

		return true;
}

//��ϵͳ����ͬ������,��ϵͳ��ȡͬ������
int CShare::drVarGetSet(char* serialString)
{
		int ret ;
		char tmpVar[5000] = {0};

		//�������ƽ̨���л���
		ret = dr_CheckSwitchLock();   
		if(ret != 0)  
		{  
			theJSLog<<"��������л���ʧ��,����ֵ="<<ret<<endi;
			return -1;  
		} 
		//��ʼ��index  
		ret = dr_InitIDX();  
		if(ret != 0)  
		{  
			theJSLog<<"��ʼ��indexʧ��,����ֵ=" <<ret<<endi;
			dr_AbortIDX();
			return -1;  
		}

		snprintf(tmpVar, sizeof(tmpVar), "%s", serialString);
		//��ϵͳ��Ҫͬ����index ����ֵ�ԡ�д������ƽ̨ά����index �ļ���
		//��ϵͳ���øú����Ľ���ǣ�var��ú���ϵͳһ�������������ֵ��	SYNC_SINGLE��ʾע�ᵥһ���������
		ret = dr_SyncIdxVar("serialString", tmpVar, SYNC_SINGLE);		
		if (ret != 0)
		{
			theJSLog<<"�����д�ʱʧ�ܣ���������["<<serialString<<"]"<<endi;
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
			theJSLog<<"����ʵ����ʧ�ܣ�"<<tmpVar<<endi;
			dr_AbortIDX();  
			return -1;
		}
		
		
		// <6> Ԥ�ύindex  �˹ؼ������ڽ�ƽ̨��ǰ�ڴ��е��������д�����
		ret = dr_SyncIdxVar("@@FLUSH","SUCCESS",SYNC_SINGLE);  
		if (ret != 0 )
		{
			theJSLog<<"Ԥ�ύindexʧ��"<<endi;
			dr_AbortIDX();
			return -1;
		}
		
		
		// <7> �ύindex  	�ύIndex����index�ļ���������ɱ�־
		ret = dr_CommitIDX();  
		if(ret != 0)  
		{  
			theJSLog<<"�ύindexʧ��,����ֵ="<<ret<<endi;
			dr_AbortIDX();  
			return -1;  
		}

		//��ϵͳ����Ŀ¼
		//if(!m_syncDr.isMaster())thelog<<"��ϵͳSerialString��"<<m_SerialString<<endi;

		theJSLog<<"���ε�ͬ����serialString:"<<serialString<<endi;//for test

		return ret;

}

//�ٲ��ַ���
 bool CShare::IsAuditSuccess(const char* dealresult)
 {
		int auitStatus = 0, ret = 0;
		ret = dr_Audit(dealresult);
		if(2 == ret )
		{
			theJSLog << "�����ٲ�ʧ��,���:" << ret <<"���ˣ�"<<dealresult<< endi;
			dr_AbortIDX();
			return false;
		}
		else if (3 == ret)
		{
			theJSLog<<"�����ٲó�ʱ..."<<endi;
			dr_AbortIDX();
			return false;
		}
		else if(1 == ret)
		{
			ret = dr_CommitSuccess();
			if(ret != 0)
			{
				theJSLog << "ҵ��ȫ���ύʧ��(����ƽ̨)" << endi;
				dr_AbortIDX();
				return false;
			}
			theJSLog<<"ret = "<<ret<<"�ٲóɹ�...\n�ٲ����ݣ�"<<dealresult<<endi;
			return true;
		}
		else
		{
			theJSLog<<"δ֪ret="<<ret<<"	�ٲ����ݣ�"<<dealresult<<endi;
			dr_AbortIDX();
			return false;
		}
	
	return true;
 }

bool CShare::CheckTriggerFile()
{
	int ret = 0;
	if(access(m_triggerFile.c_str(),F_OK) != 0)	return false;

	theJSLog<< "��鵽trigger�ļ�,��ɾ��"<< m_triggerFile <<endl;

	ret = remove(m_triggerFile.c_str());	
	if(ret) 
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"ɾ��trigger�ļ�[%s]ʧ��: %s",m_triggerFile,strerror(errno));
		theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
	}
}


#endif




