//2013-08-19 C_SUMTABLE_DEFINE�����ֶ�ORGSUMT_SOUR,��������������Դ�Ĳ�ѯ����,���»��ܺ��ջ��ܶ��˸�����ORGSUMT_SOUR=��ǰ����ԴID

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

		//PS_Process::setSignalPrc();				//�ж��ź�	
	}
	else		//���Ĳ�����Ҫ�Լ���ʼ��
	{
		if( !param_cfg.bOnInit() )
		{
			string sErr;
			int nCodeId;
			param_cfg.getError(sErr,nCodeId);
			cerr<<"�������ýӿڳ�ʼ��ʧ�ܣ�������="<<nCodeId<<", ������Ϣ="<<sErr<<endl;
			return false;
		}
	}

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
	 sprintf(sParamName, "sql.path");
	 if(param_cfg.bGetMem(sParamName, sKeyVal))
	 {
		memset(sql_path,0,sizeof(sql_path));
		strcpy(sql_path,(const char*)sKeyVal);
		completeDir(sql_path);					
		sprintf(sqlFile,"%sSummary_day.%d.sql",sql_path,getFlowID()); //ά��̬ʱ��ʱдSQL�ļ� 

	 }
	 else
	 {	
		cout<<"����Ĳ���������sql�ļ����ڵ�·��[sql.path]"<<endl;
		return false ;
	 }

	 //�ж�Ŀ¼�Ƿ����
	 DIR *dirptr = NULL; 
	 if((dirptr=opendir(szLogPath)) == NULL)
	 {
		cout<<"��־Ŀ¼["<<szLogPath<<"]��ʧ��"<<endl;	
		return false ;
	 }else closedir(dirptr);
	  if((dirptr=opendir(sql_path)) == NULL)
	 {		
		cout<<"SQLĿ¼:"<<sql_path<<"��ʧ��"<<endl;
		return false ;
	 }else  closedir(dirptr);

	 //��ʼ���ڴ���־�ӿ�
	 bool bb = initializeLog(argc,argv,false);  //�Ƿ����ģʽ
	 if(!bb)
	 {
			//cout<<"��ʼ���ڴ���־�ӿ�ʧ��"<<endl;
			return false;
	 }

	 theJSLog.setLog(szLogPath, atoi(szLogLevel),"DAY_SUM","SUMMARY", 001);	//�ļ���־�ӿڣ��������ڴ���־�ӿ�
	 
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
				//cout<<"����Դ��"<<source_group_id<<"û������"<<endl;
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
				//cout<<"����Դ��"<<source_group_id<<"û����������ԴID,����Ϊ0"<<endl;
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
		sprintf(erro_msg,"init ���ݿ����%s",e.what());
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
	//cout<<"sql = "<<sql<<endl;
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>ret;
	if(ret)
	{
		//theJSLog<<"["<<sumday<<"]�Ѿ������ջ���"<<endi;
		return 0;
	}

/*
	//sch_format�����ȡfile_id,file_name,����file_time = '����Ҫ��ʱ��' and deal_flag = 'Y' source_id
	sprintf(sql,"select file_id,filename from d_sch_format where source_id = '%s' and file_time = '%s' and deal_flag = 'Y'",Sday.szSourceId,sumday);
	stmt.setSQLString(sql);
	stmt.execute();
	string fileName;
	int file_id;
	while(stmt>>file_id>>fileName)
	{
		Sday.allFiles.insert(map<int,string>::value_type(file_id,fileName));
	}
	
	if(Sday.allFiles.size() == 0)
	{
			cout<<"����û�н��յ��ļ�"<<endl;
			return 0;
	}
	
	//ѭ��find file_id��d_sch_end��    //deal_flag = 'E'�������ô��
	char sql2[1024],sql3[1024],filename[1024];
	int cnt = 0;
	sprintf(sql2,"select filename from d_sch_end where file_id = :1 ");
	sprintf(sql3,"select count(1) from d_check_file_detail where filename = :1 and deal_flag = 'Y'");
	Statement stmt2 = conn.createStatement();
	Statement stmt3 = conn.createStatement();
	stmt.setSQLString(sql);
	stmt2.setSQLString(sql2);
	stmt3.setSQLString(sql3);
	for(map<int,string>::const_iterator iter = Sday.allFiles.begin();iter != Sday.allFiles.end(); ++iter)
	{	
		stmt<<iter->first;
		stmt.execute();
		memset(filename,0,sizeof(filename));
		if(stmt>>filename)
		{
			//�Ƚ� d_check_file_detail,�����ļ��������Ƿ�����˺˶�
			stmt2<<filename;
			stmt2.execute();
			stmt2>>cnt;
			if(cnt)
			{
				cout<<"�ļ�"<<filename<<"�˶Գɹ�"<<endl;
			}
			else
			{
				cout<<"�ļ�"<<filename<<"�˶�ʧ�ܣ��澯����"<<endl;
				stmt.close();
				stmt2.close();	
				stmt3.close();
				return -2;
			}
		}
		else		//˵����ʱд�ļ�ģ�黰�������д��err_file_info,���쳣���ģ�鴦����д d_out_file_reg ��
		{
			stmt3<<iter->first;
			stmt3.execute();
			stmt3>>cnt;
			if(cnt)
			{
				cout<<"�ҵ��ļ�"<<iter->first<<"��d_out_file_reg�����쳣���ģ�鴦���"<<endl;			
			}
			else
			{
				cout<<"�ļ�"<<iter->first<<"��δ������ɣ���ȴ�"<<endl;
				stmt.close();
				stmt2.close();
				stmt3.close();
				return 2;
			}	
		}
	}
	
	stmt.close();
	stmt2.close();
	stmt3.close();
 */

	//�Ӻ˶Խ����d_check_file_detail��ȡ�ļ�������ȷ��
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(1) from d_check_file_detail where check_type = 'AUD' and file_time = '%s' and check_flag = 'N'",sumday);
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>ret;
	if(ret)
	{
		sprintf(erro_msg,"��������ʧ��:�˶Ա�[d_check_file_detail]�д��ں˶�ʧ�ܵ��ļ�");
		theJSLog.writeLog(0,erro_msg);
		return -2;
	}

	memset(sql,0,sizeof(sql));
	sprintf(sql,"select content from d_check_file_detail where check_type = 'AUD' and file_time = '%s' and check_flag = 'Y'",sumday);
	stmt.setSQLString(sql);
	stmt.execute();
	string fileName;
	vector<string> fc;
	while(stmt>>fileName)
	{
		fc.push_back(fileName);
	}
	
	//��ѯd_sch_end���Ƿ�������ʧ�ܵĺ˶��ļ�
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(1) from d_sch_end where file_time = '%s' and deal_flag = 'E'",sumday);
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>ret;
	if(ret)
	{
		//theJSLog<<"�����[d_sch_end]�д������ʧ�ܵ��ļ�"<<endi;
		sprintf(erro_msg,"��������ʧ��:�����[d_sch_end]�д������ʧ�ܵ��ļ�");
		theJSLog.writeLog(0,erro_msg);
		return -2;
	}
	
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select filename from d_sch_end where file_time = '%s' and deal_flag = 'Y'",sumday);
	stmt.setSQLString(sql);
	stmt.execute();
	vector<string> fe;
	bool find_flag = true;
	while(stmt>>fileName)
	{
		fe.push_back(fileName);
	}
	
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(1) from d_out_file_reg where file_type = 'N' and filename = :1");
	stmt.setSQLString(sql);

	for(int i = 0;i<fc.size();i++)
	{	
		find_flag = false;
		for(int j = 0;j<fe.size();j++)
		{
			if(fc[i] == fe[j])					//�ҵ���Ӧ�ļ������˶Գɹ�
			{
				find_flag = true;
				break;
			}
		}
		
		if(!find_flag)	//˵����ʱд�ļ�ģ�黰�������д��err_file_info,���쳣���ģ�鴦����д d_out_file_reg ��
		{
			stmt<<fc[i];
			stmt.execute();
			stmt>>ret;
			if(ret)
			{
				//theJSLog<<"�ҵ��ļ�["<<fc[i]<<"]��[d_out_file_reg]�����쳣���ģ�鴦��"<<endi;			
			}
			else
			{
				theJSLog<<"�ļ�["<<fc[i]<<"]��δ������ɣ���ȴ�"<<endi;
				stmt.close();
				return 2;
			}	
		
		}
	}
	
	stmt.close();

 }catch(SQLException e)
 {
	sprintf(erro_msg,"checkDayCondition ���ݿ����%s",e.what());
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
				//sprintf(erro_msg,"ƴ��sqlʧ��")
				//theJSLog<<"sql"<<"ƴ��ʧ��"<<endl;
				return -1 ;
		}	
		cout<<"ƴ�ӵ�sql = "<<sql<<endl;

		//return -1;

		ret = insertSql(sql);					//������ܼ�¼

		if(ret == 0)
		{
			return -1;
		}
		else
		{
			theJSLog<<"�����ջ��ܱ�["<<Sday.tableItem.iDestTableName<<"]�ɹ�"<<endi;

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
				//cout<<"�ֶ�"<<Sday.tableItem.szOrgSumtCol<<"û����ͳ�Ƹ�ʽ��"<<Sday.tableItem.iOrgTableName<<"����"<<endl;
				sprintf(erro_msg,"�ֶ�[%s]û����ͳ�Ƹ�ʽ��[%s]����",Sday.tableItem.szOrgSumtCol,Sday.tableItem.iOrgTableName);
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return -1 ;
			}
			
			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from  %s where %s = '%s'",Sday.tableItem.iDestTableName,time_col,sumday);
			
			//cout<<"ͳ�ƻ��ܼ�¼����sql:"<<sql<<endl;

			Statement stmt = conn.createStatement();
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>cnt;
			//theJSLog<<"������¼����:"<<cnt<<endi;
			theJSLog<<"������¼����"<<cnt<<"	д������־��[D_SUMMARY_RESULT]"<<endi;

			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME)values('%s','%s','%s',%ld,to_char(sysdate,'yyyymmddhh24miss'))",Sday.szSourceId,"D",sumday,cnt);
			cout<<"������־sql = "<<sql<<endl;
			stmt.setSQLString(sql);
			stmt.execute();
			stmt.close();
			
						
		}

		return ret ;

}

//��������Դ���л��ܴ���
void CDsum::run()
{	
	int ret = -1;
	char currDate[9];
	while(true)
	{

		if(gbExitSig)
		{
			if(gbExitSig) theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "�ջ��ܳ����յ��˳��ź�");
			PS_Process::prcExit();
			return;
		}
	
		if(!(dbConnect(conn)))
		{
			sprintf(erro_msg,"run()  �������ݿ�ʧ�� connect error");
			theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
			return  ;
		}

		getCurTime(currTime);		//��ȡ��ǰ�ջ���ʱ��
		memset(currDate,0,sizeof(currDate));
		strncpy(currDate,currTime,8);

		memset(szSumDate,0,sizeof(szSumDate));
		addDays(-1,currDate,szSumDate);			//��ȡ���������
		
		theJSLog<<"��������:"<<szSumDate<<endi;

		for(int i = 0;i<iSourceCount;i++)
		{			
			theJSLog<<"��������Դ:"<<pDayList[i].szSourceId<<endi;
	
			//ÿ������Դ�ж��Ƿ�ﵽ��������
			ret = checkDayCondition(pDayList[i],szSumDate);
			
			//pDayList[i].allFiles.clear();		//�ж���ɺ��ļ�����յ�

			if(ret == 1)			//��������
			{	
				theJSLog<<"�˶���������,׼������"<<endi;
				sum(pDayList[i],szSumDate);
			}
			else if(ret == 0)		//�Ѿ������ջ���
			{
				//sum(pDayList[i],szSumDate);
				theJSLog<<"�Ѿ������ջ���"<<endi;
				continue;
			}
			else if(ret  == -2)		//�к˶Ի������ʧ�ܵ��ļ�
			{
				continue ;
			}

			else if(ret == 2)
			{
				continue;			//������δ��ɣ��´β�ѯ
			}
			else if(ret == -11)
			{
				conn.close();
				return  ;
			}
		}
		
		conn.close();

		sleep(60);
		//break;
	}

	return ;
}

//Summary -rd-t YYYYMMDD �����ջ���
int CDsum::redorun(char* date,bool del)
{
	int  ret = -1;
	//char currDate[9];	
	
	theJSLog<<"���»�������["<<date<<"]������"<<endi;

	if(!(dbConnect(conn)))
	{
		sprintf(erro_msg,"run()  �������ݿ�ʧ�� connect error");
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
		return -1 ;
	}
	
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

		//pDayList[i].allFiles.clear();		//�ж���ɺ��ļ�����յ�

		switch(ret)
		{
			case 0:								//��ʾ�Ѿ��������ܣ�������ᣬ��������������
				  redosum(pDayList[i],date,del);
				  break;

			case 1:							    //��ʾ��û��������
				 theJSLog<<"Ŀǰ��û���ջ��ܵ��������"<<endi;
				 break;

			case 2:			//�ļ�δ������
				break;
		
			case -2 :		//�ļ��˶�ʧ�ܣ��澯����
				break;

			case -11:		//���ݿ��쳣
				 conn.close();
				 return -1;

			default:
				break;
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
			//Statement stmt = conn.createStatement();

		if(del)				//��ɾ��ԭ�Ȼ��ܱ������,�ж�������ʱ���ֶ�ԭʼ����ORGSUMT_TCOL�ֶζ�Ӧ������ֶΣ�SItemPairͨ��Դͷ��Ŀ��
		{		
			theJSLog<<"ɾ������"<<sumday<<"������"<<endi;
			memset(sql,0,sizeof(sql));
			sprintf(sql,"delete from %s where %s = '%s'",Sday.tableItem.iDestTableName,time_col,sumday);
			cout<<"ɾ�����ܱ���sql="<<sql<<endl;
			
			Statement stmt = conn.createStatement();
			stmt.setSQLString(sql);
			stmt.execute();
			stmt.close();
		}
		else
		{		
			Statement stmt = conn.createStatement();
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
				//cout<<"sql"<<"ƴ��ʧ��"<<endl;
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


		theJSLog<<"ƴ�ӵ�sql = "<<sql<<endi;
		ret = insertSql(sql);											//������ܼ�¼
	
		if(ret == 0)
		{
			return -1;
		}
		else
		{
			theJSLog<<"�����ջ��ܱ�"<<Sday.tableItem.iDestTableName<<"�ɹ�"<<endi;

			memset(sql,0,sizeof(sql));
			sprintf(sql,"select count(1) from  %s where %s = '%s'",Sday.tableItem.iDestTableName,time_col,sumday);
			
			cout<<"ͳ���ػ��ܼ�¼����sql:"<<sql<<endl;
			Statement stmt = conn.createStatement();
			stmt.setSQLString(sql);
			stmt.execute();
			stmt>>cnt;
			theJSLog<<"������¼����:"<<(cnt-last_cnt)<<"	д���ܽ����D_SUMMARY_RESULT"<<endi;
			
			memset(sql,0,sizeof(sql));
			sprintf(sql,"insert into D_SUMMARY_RESULT(SOURCEID,SUMTYPE,SUMDATE,SUMCOUNT,DEALTIME)values('%s','%s','%s',%ld,to_char(sysdate,'yyyymmddhh24miss'))",Sday.szSourceId,"RD",sumday,(cnt-last_cnt));
			cout<<"������־sql = "<<sql<<endl;
			stmt.setSQLString(sql);
			stmt.execute();

			stmt.close();
		}
		
	}catch(SQLException e)
	{
		sprintf(erro_msg,"redosum sql err %s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣

		return -11;
	}

		return -1 ;
}



#endif
