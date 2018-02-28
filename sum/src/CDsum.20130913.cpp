
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
	m_enable = false;
	flag1 = true;		
	//flag2 = true;
	pDayList = NULL;
}

CDsum::~CDsum()
{
	if(pDayList != NULL)
	{
		delete[] pDayList;   //�ͷŽṹ��ָ������
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

		//PS_Process::setSignalPrc();				//�ж��ź�	
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

	}

	if(!drInit())	return false;

	if(!(rtinfo.connect()))
	{
		return false;
	}
	short status;
	rtinfo.getDBSysMode(petri_status);
	cout<<"petri status:"<<petri_status<<endl;

	 //��ʼ���ڴ���־�ӿ�
	 bool bb = initializeLog(argc,argv,false);  //�Ƿ����ģʽ
	 if(!bb)
	 {
			//cout<<"��ʼ���ڴ���־�ӿ�ʧ��"<<endl;
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
		}
		else
		{
			ret = init();
			if(ret == false)  return false;

		}
	
	
	}catch(SQLException e)
	 {
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"init ���ݿ����%s",e.what());
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

		return false;
	 }

	conn.close();

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
		sprintf(erro_msg,"init ���ݿ����%s [%s]",e.what(),sql);
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
	sprintf(sql, "select ORGSUMT,ORGSUMT_TCOL,DAYSUMT,DAYSUMT_TCOL,ORGSUMT_SOUR from C_SUMTABLE_DEFINE where sourceid = '%s'",source_id );
	stmt.setSQLString(sql);
	stmt.execute();
	
	//SDayList pday;
	SCom scom;
	stmt>>scom.iOrgSumt>>scom.szOrgSumtCol>>scom.iDestSumt>>scom.szDestSumtCol>>pDayList[pos].org_source;
		
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
		<<" Ŀ���ID="<<scom.iDestSumt<<" Ŀ������="<<scom.iDestTableName<<" Ŀ���ʱ���ֶ�"<<scom.szDestSumtCol<<endi;
	
	ret = getItemInfo(scom,pDayList[pos].vItemInfo);		//��ȡ���������ͳ���ֶε�ֵ
	if(ret)  return -1;
	
 }
 catch (SQLException e)
 {
	memset(erro_msg,0,sizeof(erro_msg));
	sprintf(erro_msg,"loadSumConfig ���ݿ����%s",e.what());
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
try
{
	Statement stmt = conn.createStatement();
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(1) from D_SUMMARY_RESULT where sourceid = '%s' and sumtype = 'D' and sumdate = '%s'",Sday.szSourceId,sumday);
	cout<<"sql = "<<sql<<endl;
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>ret;
	if(ret)
	{
		theJSLog<<"����Դ:"<<Sday.szSourceId<<" ������:"<<"["<<sumday<<"]�Ѿ������ջ���"<<endi;
		return 0;
	}

	int in_sum = 0,out_sum = 0;
	
	//�Ӻ˶Խ����d_check_file_detail��ȡ�ļ�������ȷ��
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(1) from d_check_file_detail where check_type = 'AUD' and file_time = '%s' and check_flag = 'Y'",sumday);
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>in_sum;

	//��ѯd_sch_end���Ƿ�������ʧ�ܵ��ļ�
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(1) from d_sch_end where file_time = '%s' and deal_flag = 'N'",sumday);
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>ret;
	if(ret)
	{
		//memset(sql,0,sizeof(sql));
		//sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME)values('%s','%s','%s',%ld,to_char(sysdate,'yyyymmddhh24miss'))",Sday.szSourceId,"D",sumday,-1);
		//stmt.setSQLString(sql);
		//stmt.execute();
		stmt.close();	

		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"��������ʧ��:�����[d_sch_end]�д������ʧ�ܵ��ļ�");
		theJSLog.writeLog(0,erro_msg);
		return -1;
	}
	

	//������� �˶Ե�����ļ� = d_sch_end ����ڳɹ����ļ�Y+�����������ļ�E
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(*) from d_sch_end where file_time = '%s' and deal_flag in('Y','E')",sumday);
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>out_sum;
	
	if(in_sum != out_sum) 
	{
		theJSLog<<in_sum<<" != "<<out_sum<<",�����ļ���δ������ɣ���ȴ�"<<endi;
		stmt.close();
		return 2;
	}

	stmt.close();

 }catch(SQLException e)
 {
	memset(erro_msg,0,sizeof(erro_msg));
	sprintf(erro_msg,"checkDayCondition ���ݿ����%s [%s]",e.what(),sql);
	theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

	return -11;
 }

	return 1;
}

/*
int CDsum::getSql(SCom szSCom,vector< SItemPair >vItemInfo,char *fromDateValue,char* org_source,char* source_id,char *sql)
{
  //ͨ������vItemInfo��������ͳ���ͳ��ά�ȡ�����ͳ��ֵ
  //where������szOrgSumtCol=fromDateValue
  //group by ͳ��ά��
  //����sql
  //0Ϊ�ɹ���-1Ϊʧ�ܡ�
 
 try
 {

  memset(sql,0,sizeof(sql));
  sprintf(sql,"insert into %s (",szSCom.iDestTableName);
  string sqlA1,sqlA2,sqlB1,sqlB2;				//�ֱ����� ԭ��ͳͳ��ά�ȣ�ԭ��ͳ���Ŀ���ͳ����
  char currTime[15],time_flag;
 
  char time_col[20];
  memset(time_col,0,sizeof(time_col));

  //sqlA1.append("select  "); 
  for(int i = 0;i<vItemInfo.size();i++)		//��ѯ���ͳ����ͳ��ά�ȵ�
  {
	SItem fromItem = vItemInfo[i].fromItem;
	SItem toItem = vItemInfo[i].toItem;
	
	if(strcmp("",toItem.szItemName))							//Ŀ�������ͳ��ά��
	{
		switch(toItem.iItemType)		//����ͳ�����ͳ��ά�ȣ���Ϊ�˱�֤ԭ���Ŀ����ֶζ���
		{
			case 0:							
				//break;
				sqlB2.append(toItem.szItemName).append(",");
				break;
			case 1:						//����ͳ��ά��
				//break ;
			case 2:						
				sqlB1.append(toItem.szItemName).append(",");
				break ;
			case 12:					//��־ȡ��ǰʱ��
				time_flag = 'Y';
				strcpy(time_col,toItem.szItemName);  //����ʱ���ֶ�
				break;
			default :
				break ;
		}
	}
	if(strcmp("",fromItem.szItemName))					//ԭ���ֶ�����
	{
		switch(fromItem.iItemType)
		{
			case 0:					//��־ͳ����
			    sqlA2.append(" sum(").append(fromItem.szItemName).append("),");
				break;
			case 1:					//����ͳ��ά��
				//break ;
			case 2:					//�ַ���ͳ��ά��
				sqlA1.append(fromItem.szItemName).append(",");
				break ;
			case 12:
				//??????????????????
				break;
			default :
				break ;
		}
	}
  }
  if(time_flag == 'Y')
  {
		sqlB2.append(time_col) ; 							  //��ӵ�Ŀ����ʱ���ջ��ܱ���������ֶ�����ֵ��д���ܵ�����
		sqlA2.append("to_char(sysdate,'yyyymmddhh24miss')");  //ȡ���ݿ�ϵͳʱ��
  }
  else
  {
	sqlB2 = sqlB2.substr(0, sqlB2.length()-1);  //���Ŀ����ֶ�
	sqlA2 = sqlA2.substr(0, sqlA2.length()-1);  //���ԭʼ��ͳ����sum()
  }
  
  //sqlB1 = sqlB1.substr(0, sqlB1.length()-1);
  sqlA1 = sqlA1.substr(0, sqlA1.length()-1);  //���ԭʼ��ͳ��ά��
 
  sprintf(sql,"%s%s%s) select %s,%s from %s where %s like '%s%s' and %s = '%s' group by %s",sql,sqlB1,sqlB2,sqlA1,sqlA2,szSCom.iOrgTableName,szSCom.szOrgSumtCol,fromDateValue,"%",org_source,source_id,sqlA1);
  
  

 }catch(jsexcp::CException e)
 {
	char tmp[1024];
	memset(tmp,0,sizeof(tmp));
	sprintf("getSql()  sqlƴ��ʧ��: %s",e.GetErrMessage());
	theJSLog.writeLog(0,tmp);
	return -1;
 } 

  return 0;
}

*/

int CDsum::sum(SDayList &Sday,char *sumday)
{
	int ret = 1;
	char sql[1024];
    try
	{
		ret = getSql(Sday.tableItem,Sday.vItemInfo,sumday,sql);	//ƴ��sql
		
		string str(sql);
		int pos = str.find("group by ");
		if(pos  == string::npos)
		{
			theJSLog<<"û���ҵ�group by���"<<endi;
			return -1;
		}	
		
		memset(sql,0,sizeof(sql));
		strcpy(sql,str.substr(0,pos-1).c_str());
		sprintf(sql,"%s and %s = '%s' ",sql,Sday.org_source,Sday.szSourceId);
		strcat(sql,str.substr(pos).c_str());

		if(ret == -1)
		{		
				return -1 ;
		}	
		cout<<"ƴ�ӵ�sql = "<<sql<<endl;	
		
		//theJSLog<<"�����ջ��ܱ�["<<Sday.tableItem.iDestTableName<<"]�ɹ�"<<endi;

		//��ѯ�ջ��ܼ�¼�����������ж�������ʱ���ֶ�ԭʼ����ORGSUMT_TCOL�ֶζ�Ӧ������ֶΣ�SItemPairͨ��Դͷ��Ŀ��
		long cnt = 0;
		char time_col[30];
		memset(time_col,0,sizeof(time_col));

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
			
		Statement stmt = conn.createStatement();		
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
		sprintf(sql," from  %s where %s = '%s'",Sday.tableItem.iDestTableName,time_col,sumday);

		strsql = strsql.substr(0, strsql.length()-1);
		strsql.append(sql);

		theJSLog<<"��ѯ�����ͳ���������sql:"<<strsql<<endi;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"%s",strsql);

		stmt.setSQLString(sql);
		stmt.execute();
		str  = "";
		memset(m_AuditMsg,0,sizeof(m_AuditMsg));
		strcpy(m_AuditMsg,Sday.szSourceId);
		strcat(m_AuditMsg,":");
		while(stmt>>str)
		{
			sprintf(m_AuditMsg,"%s%s|",m_AuditMsg,str);
		}
		
		if(!IsAuditSuccess(m_AuditMsg))				//�ٲ�ʧ��,�ع����ݿ�,ɾ����ʱ�ļ�
		{
			stmt.rollback();
			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME)values('%s','%s','%s',%ld,to_char(sysdate,'yyyymmddhh24miss'))",Sday.szSourceId,"D",sumday,-2);
			stmt.setSQLString(sql);
			stmt.execute();

			stmt.close();
		}
		else		//�費��Ҫͳ����ǰ�Ľ�������������?????????????????????
		{
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from  %s where %s = '%s'",Sday.tableItem.iDestTableName,time_col,sumday);

			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>cnt;
			
			theJSLog<<"������¼����"<<cnt<<"	д������־��[D_SUMMARY_RESULT]"<<endi;

			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME)values('%s','%s','%s',%ld,to_char(sysdate,'yyyymmddhh24miss'))",Sday.szSourceId,"D",sumday,cnt);
			stmt.setSQLString(sql);
			stmt.execute();

			stmt.close();
		}
			
	}
	catch(SQLException e)
	{
		//dr_AbortIDX();

		//stmt.rollback();
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"sum ���ݿ����%s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

		return -1;
	}	
		
	return ret ;

}

//��������Դ���л��ܴ���
void CDsum::run()
{	
	int ret = -1;
	char currDate[9],tmpTime[2+1];
	

	if(gbExitSig)
	{
		if(gbExitSig) theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "�ջ��ܳ����յ��˳��ź�");
		PS_Process::prcExit();
		return;
	}
	
	rtinfo.getDBSysMode(petri_status);
	//cout<<"״ֵ̬:"<<petri_status<<endl;
	if(petri_status == DB_STATUS_OFFLINE)	return ;
		
	if(!(dbConnect(conn)))
	{
		sprintf(erro_msg,"run()  �������ݿ�ʧ�� connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
		return  ;
	}

	if(drStatus == 1)		//����ϵͳ
	{
		//���trigger�����ļ��Ƿ����
		if(!CheckTriggerFile())
		{
			sleep(1);
			return ;
		}

		memset(m_SerialString,0,sizeof(m_SerialString));
		ret = drVarGetSet(m_SerialString);
		if(ret)
		{
			theJSLog<<"��ϵͳͬ��ʧ��...."<<endi;
			return ;
		}
		
		//��ȡͬ������,����ԴID,�˶�����
		vector<string> data;		
		splitString(m_SerialString,"|",data,true,true);
		
		memset(szSumDate,0,sizeof(szSumDate));
		strcpy(szSumDate,data[1].c_str());
		theJSLog<<"��������:"<<szSumDate<<"����ԴID:"<<data[0]<<endi;

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
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"û���ҵ�������Դ��Ϣ[%s]",data[0]);		//��������δ����
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				
			dr_AbortIDX();  
			return ;
		}
		
		int iStatus = dr_GetAuditMode(module_id);
		if(iStatus == 1)		//ͬ��ģʽ,	��ϵͳ�ȴ�ָ��ʱ��
		{	
				bool flag = false;
				int times = 1;
				while(times < 31)
				{
					ret = checkDayCondition(pDayList[pos],szSumDate);
					if( ret == 2)
					{
						times++;
						theJSLog<<"�ջ�������������:������"<<times<<"��"<<endi;
						sleep(10);
					}
					else
					{
						flag = true;
						break;
					}
				}
				if(!flag)
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"�ջ�������������");
					theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);
					
					dr_AbortIDX();  
					return ;
				}		
		}
		else if(iStatus == 2)		//����ģʽ,��ϵͳ
		{
				while(1)
				{
					//�����ж�
					if(gbExitSig)
					{
						dr_AbortIDX();

						if(gbExitSig) theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "Ӧ�ó����յ��˳��ź�");
						PS_Process::prcExit();
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
		else
		{
				char tmp[50] = {0};
				snprintf(tmp, sizeof(tmp), "����ƽ̨dr_GetAuditMode�������ô��󣬷���ֵ[%d]", iStatus);
				theJSLog<<tmp<<endi;
				return ;
		}

		//��ϵͳֻ�����˻��ܲŻ���ϵͳ������Ϣ,����ʱ��ϵͳӦ���������������
		if(ret != 1)
		{
			dr_AbortIDX();

			return ;
		}
		
		theJSLog<<"�˶���������,׼������"<<endi;
		ret = sum(pDayList[pos],szSumDate);
		if(ret == -1) dr_AbortIDX();

	}

	else
	{
		getCurTime(currTime);		//��ȡ��ǰ�ջ���ʱ��
		memset(currDate,0,sizeof(currDate));
		strncpy(currDate,currTime,8);
		
		memset(tmpTime,0,sizeof(tmpTime));
		strncpy(tmpTime,currTime+8,2);
		if(strcmp(tmpTime,"04") < 0)	return ;  //4�����Ժ�ִ��

		memset(szSumDate,0,sizeof(szSumDate));
		addDays(-1,currDate,szSumDate);			//��ȡ���������
		
		theJSLog<<"��������:"<<szSumDate<<endi;
	}

	for(int i = 0;i<iSourceCount;i++)
	{			
			theJSLog<<"��������Դ:"<<pDayList[i].szSourceId<<endi;
	
			//ÿ������Դ�ж��Ƿ�ﵽ��������
			ret = checkDayCondition(pDayList[i],szSumDate);		//����������
			
			switch(ret)
			{
				case 0:
						//theJSLog<<"�Ѿ������ջ���"<<endi;
						break;
				case 1:
					
						//����ͬ����Ϣ
						memset(m_SerialString,0,sizeof(m_SerialString));
						sprintf(m_SerialString,"%s|%s",pDayList[i].szSourceId,szSumDate);
						ret = drVarGetSet(m_SerialString);
						if(ret)
						{
							theJSLog<<"��ϵͳͬ��ʧ��...."<<endi;
							return ;
						}

						theJSLog<<"�˶���������,׼������"<<endi;
						ret = sum(pDayList[i],szSumDate);
						if(ret == -1) dr_AbortIDX();

						//�ٲ�
						break;
				case -1:				//�к˶Ի������ʧ�ܵ��ļ�,д��־��
						break;
				case 2:					//������δ��ɣ��´β�ѯ
						break;		
				case -11:
						conn.close();
						break  ;
				default:

						break;
			}
		
	}
	
	conn.close();

	return ;
}

//Summary -rd-t YYYYMMDD �����ջ��� ���������Ѿ�����,�һ��������Ѿ�����
int CDsum::redorun(char* date,bool del)
{
	int  ret = -1;
	//char currDate[9];	
	
	theJSLog<<"���»�������["<<date<<"]������"<<endi;
	
	rtinfo.getDBSysMode(petri_status);
	if(petri_status == DB_STATUS_OFFLINE)	
	{
		theJSLog<<"��ǰ���ݿ�״̬Ϊ����̬,�޷�����..."<<endi;
		return ;
	}

	if(!(dbConnect(conn)))
	{
		sprintf(erro_msg,"run()  �������ݿ�ʧ�� connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
		return -1 ;
	}
	
	if(drStatus == 1)		//����ϵͳ
	{
		//���trigger�����ļ��Ƿ����
		if(!CheckTriggerFile())
		{
			sleep(1);
			return ;
		}

		memset(m_SerialString,0,sizeof(m_SerialString));
		ret = drVarGetSet(m_SerialString);
		if(ret)
		{
			theJSLog<<"��ϵͳͬ��ʧ��...."<<endi;
			return ;
		}
		
		//��ȡͬ������,����ԴID,�˶�����,ɾ����־
		vector<string> data;		
		splitString(m_SerialString,"|",data,true,true);
		
		memset(szSumDate,0,sizeof(szSumDate));
		strcpy(szSumDate,data[1].c_str());
		theJSLog<<"������������:"<<szSumDate<<"����ԴID:"<<data[0]<<endi;

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
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"û���ҵ�������Դ��Ϣ[%s]",data[0]);		//��������δ����
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				
			dr_AbortIDX();  
			return ;
		}
		
		int iStatus = dr_GetAuditMode(module_id);
		if(iStatus == 1)		//ͬ��ģʽ,	��ϵͳ�ȴ�ָ��ʱ��
		{	
				bool flag = false;
				int times = 1;
				while(times < 31)
				{
					ret = checkDayCondition(pDayList[pos],szSumDate);
					if( ret == 2)
					{
						times++;
						theJSLog<<"�ջ�������������:������"<<times<<"��"<<endi;
						sleep(10);
					}
					else
					{
						flag = true;
						break;
					}
				}
				if(!flag)
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"�ջ�������������");
					theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);
					
					dr_AbortIDX();  
					return ;
				}		
		}
		else if(iStatus == 2)		//����ģʽ,��ϵͳ
		{
				while(1)
				{
					//�����ж�
					if(gbExitSig)
					{
						dr_AbortIDX();

						if(gbExitSig) theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "Ӧ�ó����յ��˳��ź�");
						PS_Process::prcExit();
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
		else
		{
				char tmp[50] = {0};
				snprintf(tmp, sizeof(tmp), "����ƽ̨dr_GetAuditMode�������ô��󣬷���ֵ[%d]", iStatus);
				theJSLog<<tmp<<endi;
				return ;
		}

		//��ϵͳֻ�����˻��ܲŻ���ϵͳ������Ϣ,����ʱ��ϵͳӦ���������������
		if(ret == 0 || ret == 1)
		{
				theJSLog<<"�˶���������,׼������"<<endi;
				ret = sum(pDayList[pos],szSumDate);
				if(ret == -1) dr_AbortIDX();
			
		}
		else
		{
				dr_AbortIDX();
				return ;
		}

	}
	
	else
	{
		for(int i = 0;i<iSourceCount;i++)
		{			
			theJSLog<<"��������Դ:"<<pDayList[i].szSourceId<<endi;

    /*	����ŵ�����������ʱȥ�ж�
		getCurTime(currTime);		//��ȡ��ǰ�ջ���ʱ��
		memset(currDate,0,sizeof(currDate));
		strncpy(currDate,currTime,8);

		memset(sumDate,0,sizeof(sumDate));
		addDays(-1,currDate,sumDate);			//��ȡ���������
		if(strcmp(date,sumDate) > 0)
		{
			cout<<"�������ڲ��ܳ���"<<sumDate<<endl;
			return -1 ;
		}

	*/
			//ÿ������Դ�ж��Ƿ�ﵽ��������
			ret = checkDayCondition(pDayList[i],date);

			switch(ret)
			{
				case 0:					//��ʾ�Ѿ��������ܣ�������ᣬ��������������

				case 1:					//��ʾ��û��������
					
					//����ͬ����Ϣ
					memset(m_SerialString,0,sizeof(m_SerialString));
					sprintf(m_SerialString,"%s|%s|%d",pDayList[i].szSourceId,szSumDate,del);
					ret = drVarGetSet(m_SerialString);
					if(ret)
					{
							theJSLog<<"��ϵͳͬ��ʧ��...."<<endi;
							return ;
					}
					
					theJSLog<<"�˶���������,׼������"<<endi;
					ret = redosum(pDayList[i],date,del);
					if(ret == -1) dr_AbortIDX();
					break;

				case 2:			//�ļ�δ������
					break;
		
				case -1 :		//�ļ��˶�ʧ�ܣ��澯����
					break;

				case -11:		//���ݿ��쳣
					conn.close();
					return -1;

				default:
					break;
			}
		}


	}


	conn.close();

	theJSLog<<"��������["<<date<<"]���"<<endi;

	return ret ;
}


int CDsum::redosum(SDayList &Sday,char *sumday,bool del)
{
		int ret = 0;
		char sql[1024];
		long cnt = 0,last_cnt = 0;
		
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
			sprintf(erro_msg,"�ֶ�[%s]û����ͳ�Ƹ�ʽ��[%s]����",Sday.tableItem.szOrgSumtCol,Sday.tableItem.iOrgTableName);
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);

			return -1 ;
		}

	try
	{
		Statement stmt = conn.createStatement();

		if(del)				//��ɾ��ԭ�Ȼ��ܱ������,�ж�������ʱ���ֶ�ԭʼ����ORGSUMT_TCOL�ֶζ�Ӧ������ֶΣ�SItemPairͨ��Դͷ��Ŀ��
		{		
			theJSLog<<"ɾ������"<<sumday<<"������"<<endi;
			memset(sql,0,sizeof(sql));
			sprintf(sql,"delete from %s where %s = '%s'",Sday.tableItem.iDestTableName,time_col,sumday);
			//cout<<"ɾ�����ܱ���sql="<<sql<<endl;
			
			//Statement stmt = conn.createStatement();
			stmt.setSQLString(sql);
			stmt.execute();
			//stmt.close();
		}
		else
		{		
			//Statement stmt = conn.createStatement();
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from %s where %s = '%s'",Sday.tableItem.iDestTableName,time_col,sumday);
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>last_cnt;
			//cout<<"������ǰ���ݼ�¼����:"<<last_cnt<<endl;
		}

		ret = getSql(Sday.tableItem,Sday.vItemInfo,sumday,sql);			//ƴ��sql
		if(ret == -1)
		{		
				return -1 ;
		}
		
		string str(sql);
		int pos = str.find("group by ");
		if(pos  == string::npos)
		{
			theJSLog<<"û���ҵ�group by���"<<endi;
			return -1;
		}	
		
		memset(sql,0,sizeof(sql));
		strcpy(sql,str.substr(0,pos-1).c_str());
		sprintf(sql,"%s and %s = '%s' ",sql,Sday.org_source,Sday.szSourceId);
		strcat(sql,str.substr(pos).c_str());


		theJSLog<<"ƴ�ӵ�sql = "<<sql<<endi;	//������ܼ�¼
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
		sprintf(sql," from  %s where %s = '%s'",Sday.tableItem.iDestTableName,time_col,sumday);

		strsql = strsql.substr(0, strsql.length()-1);
		strsql.append(sql);

		theJSLog<<"��ѯ�����ͳ���������sql:"<<strsql<<endi;
		memset(sql,0,sizeof(sql));
		sprintf(sql,"%s",strsql);

		stmt.setSQLString(sql);
		stmt.execute();
		str  = "";
		memset(m_AuditMsg,0,sizeof(m_AuditMsg));
		strcpy(m_AuditMsg,Sday.szSourceId);
		strcat(m_AuditMsg,":");
		while(stmt>>str)
		{
			sprintf(m_AuditMsg,"%s%s|",m_AuditMsg,str);
		}
		
		if(!IsAuditSuccess(m_AuditMsg))				//�ٲ�ʧ��,�ع����ݿ�,ɾ����ʱ�ļ�
		{
			stmt.rollback();
			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME)values('%s','%s','%s',%ld,to_char(sysdate,'yyyymmddhh24miss'))",Sday.szSourceId,"D",sumday,-1);
			stmt.setSQLString(sql);
			stmt.execute();

			stmt.close();
		}
		else
		{
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from  %s where %s = '%s'",Sday.tableItem.iDestTableName,time_col,sumday);
			
			cout<<"ͳ���ػ��ܼ�¼����sql:"<<sql<<endl;
			
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>cnt;
			theJSLog<<"������¼����:"<<(cnt-last_cnt)<<"	д���ܽ����D_SUMMARY_RESULT"<<endi;
			
			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME)values('%s','%s','%s',%ld,to_char(sysdate,'yyyymmddhh24miss'))",Sday.szSourceId,"RD",sumday,(cnt-last_cnt));
			//cout<<"������־sql = "<<sql<<endl;
			stmt.setSQLString(sql);
			stmt.execute();

			stmt.close();
		}
		
		
	}catch(SQLException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"redosum ���ݿ���� %s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

		return -1;
	}

	return 0 ;
}

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
int CDsum::drVarGetSet(char* serialString)
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
 bool CDsum::IsAuditSuccess(const char* dealresult)
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

bool CDsum::CheckTriggerFile()
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
