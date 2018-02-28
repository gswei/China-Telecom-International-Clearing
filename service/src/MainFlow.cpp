/****************************************************************
filename: mainflow.cpp
module:
created by: ouyh
create date: 
version: 3.0.0
description:
    �������ƽ̨
*****************************************************************/
//20131227 lij add ���ڷ���������Ϣ���������ȥ��������Ϣ�Ĵ���
#include "MainFlow.h"
#include "MdrNodeApi.h"

using namespace filterchain;
using namespace std;
using namespace tpss;

//CDatabase _DBConn; //�ӽ���
//CDatabase DBConn; //������
CLog theJSLog;
//MdrStatus syn_status;//����״̬
int pkgstatus; //������״ֵ̬
DBConnection conn;//���ݿ�����
//string auditkey;
//string sessionid;

bool bGetExitSig;

int main(int argc, char **argv)
{
	//cout<<"begin main"<<endl;
	C_MainFlow process;
	process.printVersion();

	if(!process.checkArg(argc, argv))
	{
		return -1;
	}

	try
	{
		signal(SIGTERM, dealSignal);
		
		if(!process.init(argc, argv))
		{
			goto Exit;
		}

        process.run();
		//while(!bGetExitSig)
		//{
			//process.Update();
			//PS_BillProcess::run();
			//process.run();
		//
Exit:
		theJSLog<<"��ȫ�˳�����"<<endi;
		// �Ͽ����ݿ����� // 
		//process.exit();
		//if (DBConn.IsConnected() == 1)
			//DBConn.Disconnect();
		return 0;
	}
	catch(jsexcp::CException &e) 
	{
		errLog(4, " ", e.GetAppError(), (std::string("����ƽ̨�����: ") + e.GetOrgErrMessage()).c_str(), __FILE__, __LINE__, e);
		///continue;
		return 0;
	}
	catch(...)
	{
		//cout<<"�յ�ϵͳ�������쳣��Ϣ��"<<errno<<endl;
		errLog(4, " ", errno, "����ƽ̨�����", __FILE__, __LINE__);
		return 0;
		//continue;
	}
	
	return 0;
};

C_MainFlow::C_MainFlow()
{
	//vecFile.clear();
	memManager = NULL;
	chain = NULL;
	pps = NULL;
	res = NULL;
	pListSql = NULL;
	//iRunTime = -1;
	conn = NULL;
	//m_bDBLinkError = FALSE;
	//_DBConn = NULL;
}

C_MainFlow::~C_MainFlow()
{
//	vecFile.clear();
	releasePoint();
}

void C_MainFlow::releasePoint()
{
	if(memManager != NULL)
	{
		delete memManager;
		memManager = NULL;
	}
	if(pps != NULL)
	{
		delete pps;
		pps = NULL;
	}
	if(res != NULL)
	{
		delete res;
		res = NULL;
	}
	if(pListSql != NULL)
	{
		delete pListSql;
		pListSql = NULL;
	}

  // chain = NULL;
}

void C_MainFlow::printVersion()
{
	/* ���ģ������������ơ��汾����Ϣ */
	cout<<"********************************************** "<<endl;
	cout<<"*    China Telecom. Telephone Network    * "<<endl;
	cout<<"*        International Account Settle System       * "<<endl;
	cout<<"*                                            * "<<endl;
	cout<<"*                    jsserviceprep               * "<<endl;
	cout<<"*                  sys.GJZW.Version 1.0	            * "<<endl;
	cout<<"*           created time : 2013-10-03 by  lij	* "<<endl;
	cout<<"********************************************** "<<endl;
};

bool C_MainFlow::getParam()
 {
    // ��ȡʵ��service �����Ĳ���  
    //%s <service_id> <source_group> <process_index>
     procID       = PS_BillProcess::getPrcID();  // ����ID  PROCESS_ID  service_id
     billing_line_id = PS_BillProcess::getFlowID(); //������ID   
     process_index = PS_BillProcess::getInstID();  // process_index ���̺�

    try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			string sql = "select ext_param from tp_process where process_id = :v1";		
			stmt.setSQLString(sql);
			stmt << procID;			
			stmt.execute();
			stmt >> ext_param;

			sql = "select source_group from tp_billing_line where billing_line_id = :v1";
			stmt.setSQLString(sql);
			stmt << billing_line_id;
			stmt.execute();
			stmt >> source_group;		
	 }else{
	 	//theJSLog<<"connect error."<<endl;
	 	theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,"connect error.");
	 	return false;
	 }
	 conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		theJSLog << "��ȡ�����ߺ�ʵ��ID ����" << endi;
		throw jsexcp::CException(0, "��ȡ�����ߺ�ʵ��ID ����", __FILE__, __LINE__);
		conn.close();
		return false;
     } 	 
    
	strcpy(Param.szServiceId, ext_param);
    strcpy(Param.szSourceGroupId, source_group);
    Param.iProcessId = process_index;	    
	return true;
 }

bool C_MainFlow::checkArg(int argc, char** argv)
{	
	/* �����������Ƿ���ȷ������Ϊ2����������ID ��ʵ��ID */
	if (!(argc == 3 || argc == 2))
	{
	    printf("Usage : %s <billing_line_id> <instance_id>\n",argv[0]);
		return false;
	}
 
	char szPathTmp[FILE_NAME_LEN+1];
	strcpy(szPathTmp, argv[0]);
	if(!strncmp(szPathTmp,"../",3))
	{
		getcwd(m_szExePath, PATH_NAME_LEN+1);
		sprintf(m_szExePath, "%s/%s", m_szExePath, szPathTmp);
		strncpy(m_szExeName, strrchr(szPathTmp,'/')+1, strlen(szPathTmp)-(strrchr(szPathTmp,'/')-szPathTmp)+1);
	}
	else if(!strncmp(szPathTmp,"./",2))
	{
		getcwd(m_szExePath, PATH_NAME_LEN+1);
		strncpy(m_szExeName, strrchr(szPathTmp,'/')+1, strlen(szPathTmp)-(strrchr(szPathTmp,'/')-szPathTmp)+1);
	}
	else if(!strncmp(szPathTmp,"/",1))
	{
		strncpy(m_szExeName, strrchr(szPathTmp,'/')+1, strlen(szPathTmp)-(strrchr(szPathTmp,'/')-szPathTmp)+1);
		strncpy(m_szExePath, szPathTmp, strlen(szPathTmp)-strlen(m_szExeName));
	}
	else
	{
		getcwd(m_szExePath, PATH_NAME_LEN+1);
		strcpy(m_szExeName, szPathTmp);
	}
	//theJSLog<<"szProgramPath="<<m_szExePath<<endi;
	//theJSLog<<"szProgramName="<<m_szExeName<<endi;
	return true;
};

//bool C_MainFlow::init(int argc, char **argv)
//{ 
//    pkgstatus = 0; // �������ʼ״̬Ϊδ����
//    if(!PS_BillProcess::init(argc,argv))
//   {
//      return false;
//   }
//   return true;
//}

bool C_MainFlow::init(int argc, char **argv)
{
    // ��ʼ�������̿��
   
    pkgstatus = 0; // �������ʼ״̬Ϊδ����
    if(!PS_BillProcess::init(argc,argv))
   {
      return false;
   }

   _iMainPid = getpid();

	//iUndoFlag = 0;
	iRad = 1;
	//iRunTime = 0;
	char szCurrentTime[DATETIME_LEN+1];
	char szLogStr[LOG_MSG_LEN+1];             //��־��Ϣ
	char szSqlStr[SQL_LEN+1];					//�洢��ʱsql���ı���
	char szTemp[FIELD_LEN+1];
	bGetExitSig = false;

     /* �������ݿ� */

    // ��ȡʵ����Ҫ�����Ĳ���%s <service_id> <source_group> <process_index> [<env_path>]\n
       getParam();

	/* �������̵Ĺ����ڴ��������� */
	char szIPCKeyPath[PATH_NAME_LEN+1];	

	/* ��ʼ����־ */
	//char szLogPath[PATH_NAME_LEN+1];
	//char szLogLevel[10];

	//���Ĳ�����ȡ
	/*std::string log_path,log_level;
	if( !tpss::getKenelParam( "log.path", log_path ) ) {
		tpss::writelog( 0, "��ȡ��־·��ʧ��" );
	}
	if( !tpss::getKenelParam( "log.level", log_level ) ) {
		tpss::writelog( 0, "��ȡ��־����ʧ��" );
	}
	int level_no = es::StringUtil::toInt( log_level );
	
	char szLogPath[] = {"/mboss/home/zhjs/log"};
	strcpy(szLogPath,log_path.c_str());*/
	//cout << "PS_BillProcess::szLogPath = " << PS_BillProcess::szLogPath<< "PS_BillProcess::szLogLevel = " << PS_BillProcess::szLogLevel << endl;
	theJSLog.setLog(PS_BillProcess::szLogPath, PS_BillProcess::szLogLevel, Param.szServiceId, Param.szSourceGroupId, Param.iProcessId);
	
	//����Ϊ��̨��פ����
	//memset(szTemp, 0, sizeof(szTemp));
    //szTemp = "Y"; // д��ΪY
	//if(!strcmp(szTemp, "Y"))
	 // initDaemon(true);
	//theJSLog.reSetLog();

	// ��Ϣ����־
	Param.info.SetEnvPath(m_szEnvPath);
	Param.info.configInfoLog(Param.szServiceId, Param.szSourceGroupId, Param.iProcessId, m_szExeName, m_szExePath);
	Param.info.msgInfoLog(INFO_START);
	//��ӡ��������
	theJSLog<<"�������"<<m_szExeName<<"  "<<Param.szServiceId<<"  "<<Param.szSourceGroupId<<"  "<<Param.iProcessId<<endi;
	try
	{	
	  {
	  	getSourceInfo();
	  	//Param.ProcMonitor.Attach();
		//CBindSQL ds(DBConn);	
		/* ȡ���йؼ��� */
				
		// �������̵Ĺ����ڴ��������� 
		
		/*memset(szIPCKeyPath, 0, sizeof(szIPCKeyPath));
		if( getEnvFromDB( _DBConn, Param.szServiceId, Param.szSourceGroupId, "", "IPCKEY_PATH", szIPCKeyPath ) == -1 )
		{
			sprintf(szLogStr, "������Ϣ�����ڴ�IPCKEYĿ¼[IPCKEY_PATH]δ����!");
			theJSLog<<szLogStr<<endi;
			return false;
		}
		Param.ProcMonitor.SetIPCKeyPath(szIPCKeyPath);
		Param.ProcMonitor.Attach();
		Param.ProcMonitor.Init(m_szExeName, Param.szServiceId, Param.szSourceGroupId, Param.iProcessId);
		Param.ProcMonitor.UpdateMonitor(CProcInfo::SERV_IDLE);
		getSourceInfo();

		if(pListSql != NULL)
		{
			delete pListSql;
			pListSql = NULL;
		}
		iListCount = 0;
		// ��ѯ�Ƿ���ʹ����������ʽlist���������Ҫ����list_sql���жϱ���¼�Ƿ񱻸�����
		ds.Open("select count(*) from i_list_sql", SELECT_QUERY );
		if (!(ds>>iListCount))
		{
			ds.Close();
			Param.info.msgInfoLog(INFO_INITFAIL);
			strcpy(szLogStr, "ִ�� select count(*) from i_list_sql ������");
			errLog(4, "��ʼ��ʧ��", errno, szLogStr, __FILE__, __LINE__);
			return false;
		}
		ds.Close();

		if( iListCount > 0 ) 
		{
			STmpVariable sVar;
			pListSql = new SListSql[iListCount];
			if( pListSql == NULL )
			{
				Param.info.msgInfoLog(INFO_INITFAIL);
				strcpy(szLogStr, "���� SListSql ʧ�ܣ�");
				errLog(4, "��ʼ��ʧ��", errno, szLogStr, __FILE__, __LINE__);
				return false;
			}
			
			ds.Open("select SQL_ID,UPDATE_FLAG from i_list_sql order by SQL_ID", SELECT_QUERY );
			for( int i=0; i < iListCount; i++ )
			{
				if( ds>>pListSql[i].m_szSqlId>>pListSql[i].m_szUpdateFlag )
				{
					sprintf( pListSql[i].m_szUpdateSqlName, "UPDATE_%s", pListSql[i].m_szSqlId);
					// ���ӵ��м���� 
					strcpy(sVar.szVarName, pListSql[i].m_szUpdateSqlName);
					strcpy(sVar.szVarValue, pListSql[i].m_szUpdateFlag);
					Param.map_DefVar.push_back(sVar);
				}
				else
				{
					Param.info.msgInfoLog(INFO_INITFAIL);
					strcpy(szLogStr, "ִ�� select SQL_ID,UPDATE_FLAG from i_list_sql order by SQL_ID ������");
					errLog(4, "��ʼ��ʧ��", errno, szLogStr, __FILE__, __LINE__);
					ds.Close();
					return false;	
				}
			}//end of for( int i=0; i < iListCount; i++ )
			ds.Close();
			pListSql = &pListSql[0];
		}*/
	}

		/***********************************************************************************

		*�����ݿ��л�ȡ��������Ӧ��ֵ

		***********************************************************************************/
		if( getScheme(Param) != 0 )
		{
			Param.info.msgInfoLog(INFO_INITFAIL);
			return false;
		}	

		// lij add 20131107  ���������ݿ�����ȫ�����ӵ��˴�
			// ��ȡҵ�����
     try{	
     	memset(servCat,0,sizeof(servCat));		
	  if (dbConnect(conn))
	    {
			Statement stmt = conn.createStatement();
			//string sql = "select serv_cat_id from i_source_define where source_id = :v1 ";
			// ��һ�ֻ�ȡ��ʽ��ͨ������Դ������ȡ SERV_CAT_ID
			string sql = "select distinct(serv_cat_id) from i_source_define where source_id in (select source_id from C_SOURCE_GROUP_CONFIG where source_group = :v1)";
			stmt.setSQLString(sql);
			//stmt << source.szSourceId;
			stmt<<Param.szSourceGroupId;
			stmt.execute();
			stmt >> servCat;
			theJSLog << "Param.szSourceGroupId = " << Param.szSourceGroupId <<endd;
			theJSLog <<"servCat = " << servCat <<endd;
	    }else{
	 	//theJSLog<<"connect error."<<endi;
	 	sprintf(servCat,"%s","G1");  //������Ӵ������ó�ʼֵ
	 	theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,"connect error.");
	  }
	   conn.commit();
	   conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
		//throw jsexcp::CException(0, "��ȡҵ�����ʧ��", __FILE__, __LINE__);
    } 
	 
   try{			
	if (dbConnect(conn))
	  {
			Statement stmt = conn.createStatement();
			string sql = "select record_len from c_filetype_define where filetype_id = :v1 ";		
			stmt.setSQLString(sql);
			stmt << Param.szOutputFiletypeId;
			stmt.execute();
			stmt >> recordlenth;
			//cout <<"recordlenth = " << recordlenth <<endl;

	  }else{
	 	//theJSLog<<"connect error."<<endi;
	 	theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,"connect error.");
	 	//return -1;
	 }
	 //conn.commit();
	 conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
		throw jsexcp::CException(0, "��ȡ��ʽ����ʧ��", __FILE__, __LINE__);
		//return -1;
     } 

		/***********************************************************************************

		*������Ŀ¼--vivi delete 20131020

		***********************************************************************************/
		//CheckWorkpath();

		/***********************************************************************************

		*��ȡ���������е����빲���ڴ�ĵ�ַ

		***********************************************************************************/
		// 20140512 add lij
		/*if(Param.bCommemFlag)
		{
		    //CBindSQL ds(DBConn);
			memManager = new C_AccessMem();
			memManager->init(Param.szServerId,m_szEnvPath);
		}*/

		/***********************************************************************************

		*��ʼ����������ࡢ�ּ��ࡢ�������ࡢ������־�ࡢ�����ͺ���ת����

		***********************************************************************************/
		//cout << "lancinfo  falsfasf" <<endl;

		/*abnormity_type lack = abenum_lackinfo;		//����������
		if(strlen(Param.szLackStatTableId)==0)
			Param.lack_info.Init( lack, atoi(Param.szLackSaveTableId) , NULL, Param.szServiceId );
		else
			Param.lack_info.Init(lack, atoi(Param.szLackSaveTableId) , Param.szLackStatTableId, Param.szServiceId );

		if(strlen(Param.szAbnStatTableId)==0)
			Param.abnormal.Init( lack, atoi(Param.szAbnSaveTableId) , NULL, Param.szServiceId );
		else
			Param.abnormal.Init( lack, atoi(Param.szAbnSaveTableId) , Param.szAbnStatTableId, Param.szServiceId );

		if( !strcmp(Param.szIsFmtFirst, "Y") )
		{
			if( !strcmp(Param.szFmtErr2Table, "Y") )
			{
				if(strlen(Param.szFmtErrStatTableId)==0)
					Param.fmt_err2table.Init( lack, atoi(Param.szFmtErrSaveTableId) , NULL, Param.szServiceId );
				else
					Param.fmt_err2table.Init( lack, atoi(Param.szFmtErrSaveTableId) , Param.szFmtErrStatTableId, Param.szServiceId );
			}
		}*/
	}//end of try
	catch (jsexcp::CException &e) 
	{
		Param.info.msgInfoLog(INFO_INITFAIL);
		e.PushStack(PREDEAL_ERR_IN_SELECT_DB, "��ʼ��ʧ��",  __FILE__, __LINE__);
		throw e;
	}
	catch(...)
	{
		Param.info.msgInfoLog(INFO_INITFAIL);
		//errLog(0, "��ʼ��ʧ��", errno, "����ϵͳ����", __FILE__, __LINE__);
		throw jsexcp::CException(errno, "��ʼ��ʧ��", __FILE__, __LINE__);
	}

	// ��Ϣ����־	
	Param.info.msgInfoLog(INFO_INITSUC);
	onChildInitForMainFlow();
	return true;
}

/*
void C_MainFlow::CheckWorkpath()
{
	theJSLog<<"�����ʱ����Ŀ¼�Ƿ����쳣״̬���ļ�......"<<endi;

	vector<string> vFileName;
	char szTemp[PATH_NAME_LEN+1];
	char szCheckedPath[PATH_NAME_LEN+1];
	char szFileName[PATH_NAME_LEN+1];
	char szRealName[PATH_NAME_LEN+1];
	char szPathName1[FILE_NAME_LEN+PATH_NAME_LEN+1];
	char szPathName2[FILE_NAME_LEN+PATH_NAME_LEN+1];
	char szLogStr[LOG_MSG_LEN+1];
	char szSqlStr[SQL_LEN+1];
	char cFlag;
	memset(szTemp, 0, sizeof(szTemp));
	memset(szCheckedPath, 0, sizeof(szCheckedPath));
	sprintf(szTemp, "~*.%d", Param.iProcessId);

	char szCurrentTime[14+1];
	memset(szCurrentTime, 0, sizeof(szCurrentTime));
	getCurTime(szCurrentTime);
	char szTmpPart[2+1];
	memset(szTmpPart, 0, sizeof(szTmpPart));
	strncpy(szTmpPart, szCurrentTime+4, 2);
	char szPID[4];
	memset(szPID, 0, sizeof(szPID));
	sprintf(szPID, "%d", Param.iProcessId);
	
	//CBindSQL ds(DBConn);
	 try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
				    
	for( map<string, SSourceStruct>::iterator iter=mapSource.begin(); iter!=mapSource.end(); iter++ )
	{
		vFileName.clear();
		memset(szCheckedPath, 0, sizeof(szCheckedPath));
		sprintf(szCheckedPath, "%s%stmp_out/", (iter->second).szSourcePath, Param.szInPath);
		cout<<"Search:"<<szCheckedPath<<endl;
		bool doneFlag = SearchAllFiles(szCheckedPath, szTemp, vFileName);
		//���������������ļ�
		for(int k=0; k<vFileName.size(); k++)
		{
			memset(szFileName, 0, sizeof(szFileName));
			strcpy(szFileName, vFileName[k].c_str());
			strncpy(szRealName, szFileName+1, strlen(szFileName)-3);
			szRealName[strlen(szFileName)-3] = '\0';
			cout<<"Find File:"<<szFileName<<endl;

			//���˹������ü���
			string sql = "select deal_flag from :v1 where source_id=:v2 and filename=:v3 and partid in(:v4,atoi(:v5),:v6)";		
			stmt.setSQLString(sql);
			stmt << Param.szLogTabname << (iter->second).szSourceId << szRealName <<getPrePartID(szTmpPart)<<szTmpPart<<getNextPartID(szTmpPart) ;
			stmt.execute();
			stmt >> cFlag;
			
			
			
			if(cFlag == 'Y')
			{
				//�ϴ��쳣δ�������ļ�������
				sprintf(szPathName1, "%s/%s", szCheckedPath, szFileName);
				strncpy(szPathName2, szPathName1, strlen(szPathName1)-strlen(szPID));
				szPathName2[strlen(szPathName1)-strlen(szPID)+1]='\0';
				if(rename(szPathName1, szPathName2) != 0 )
				{
					sprintf(szLogStr, "���ļ� %s �ĳ� %s ʧ�ܣ�", szPathName1, szPathName2);
					throw jsexcp::CException(0, szLogStr, __FILE__, __LINE__);
				}
				theJSLog<<"���ļ� "<<szPathName1<<" �ĳ� "<<szPathName2<<endi;
			}
			else
			{
				//sprintf(szLogStr, "��%s��(����Դ%s,�ļ�%s)�ļ�¼�쳣", Param.szLogTabname, (iter->second).szSourceId,
				//	szRealName);
				//throw jsexcp::CException(0, szLogStr, __FILE__, __LINE__);
			}
		  }
	   }
	}else{
	 	//cout<<"connect error."<<endl;
	 	theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,"connect error.");
	 	throw jsexcp::CException(0, "�������ݿ�ʧ��", __FILE__, __LINE__);
	 }
	 conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		throw jsexcp::CException(0, "���ݿ����ʧ�ܼ�����·��", __FILE__, __LINE__);
		conn.close();
     } 	 
   // conn.commit();
}
*/

void C_MainFlow::Update()
{
	try
	{
		//theJSLog<<"���¹����ڴ桭��"<<endd;
		// ���¹����ڴ� /
		if(Param.bCommemFlag)
			memManager->LoadOrUpdate();

	/*if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			string sql = "select UPDATE_FLAG from i_list_sql order by SQL_ID";		
			stmt.setSQLString(sql);
			stmt.execute();			
	
            for( int i=0; i < iListCount; i++ )
		    {
			  if( !(stmt>>pListSql[i].m_szUpdateFlag) )
			  {
				throw jsexcp::CException(0, "ִ�� select UPDATE_FLAG from i_list_sql order by SQL_ID ʧ�ܣ�", __FILE__, __LINE__);
			  }
		   }
		
	 }else{
	 	//cout<<"connect error."<<endl;
	 	theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,"connect error.");
	 	throw jsexcp::CException(0, "���ݿ�����ʧ��", __FILE__, __LINE__);
	 }
	 conn.close(); */
	 
		//�������������ʽlist��������update_flag��ֵ�������ж��Ƿ�����˱���¼
		//ds.Open("select UPDATE_FLAG from i_list_sql order by SQL_ID", SELECT_QUERY );
	//	for( int i=0; i < iListCount; i++ )
		//{
	//		if( !(ds>>pListSql[i].m_szUpdateFlag) )
		//	{
		//		ds.Close();
				//theJSLog<<"select UPDATE_FLAG from i_list_sql order by SQL_ID Error!"<<ende;
		//		throw jsexcp::CException(0, "ִ�� select UPDATE_FLAG from i_list_sql order by SQL_ID ʧ�ܣ�", __FILE__, __LINE__);
		//	}
			//pListSql++;
	//	}
	//	ds.Close();
	}
	catch(jsexcp::CException &e)
	{
		conn.close();
		Param.info.msgInfoLog(INFO_BREAK);
		throw  jsexcp::CException(0, "���ݿ�����ʧ��", __FILE__, __LINE__);;
		
	}
	catch(...)
	{
		conn.close();
		Param.info.msgInfoLog(INFO_BREAK);
		throw jsexcp::CException(errno, "���ϸ���ʱ����δ֪ϵͳ����", __FILE__, __LINE__);
		
	}
}

/*
void C_MainFlow::run()
{
   // ���ü̳���run 
   PS_BillProcess::run();
}
*/
/*
void C_MainFlow::checkTime()
{
	if(chain != NULL)
	{
		//theJSLog<<"����Ƿ����(check another day)"<<endi;
		MessageParser message;
		ArgMessage argMsg;
		Argument *pMsg = &argMsg;

		char szCurrentTime[DATETIME_LEN+1];
		getCurTime(szCurrentTime);
		if( strncmp(szCurrentTime, m_szLastDay, 8) != 0)
		{
			message.setMessage(MESSAGE_NEW_DAY, "", "", 0);
			argMsg.set(message);
			iRad++;
			if(iRad > 99999)
				iRad = 1;
			argMsg.setTime(szCurrentTime, iRad);
			chain->signal(pMsg);
			strncpy(m_szLastDay, szCurrentTime, 8);
		}
	}
}
*/
/*
void C_MainFlow::resetHFiles()
{
 try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			string sql = " update :v1 set deal_flag='W' where deal_flag='H' and validflag='Y' and proc_index = :v2 and source_id in(select source_id from C_SOURCE_GROUP_CONFIG  where source_group=:v3)";			
			stmt.setSQLString(sql);
			stmt << Param.szLogTabname<< Param.iProcessId << Param.szSourceGroupId;
			stmt.execute();
			conn.commit();
            theJSLog<<"Commit succeed"<<endi;
	 }else{
	 	//cout<<"connect error."<<endl;
	 	theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,"connect error.");
	 	//theJSLog<<"���ݿ�����ʧ��"<<endi;
	 }
	 conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
		//theJSLog<<"Commit failed"<<endi;
		theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,"connect error.");
		throw jsexcp::CException(errno, "������־��ʧ��", __FILE__, __LINE__);
     } 	 
   // conn.commit();
    
	//char szSqlStr[SQL_LEN+1];
	//��־�����Ƿ����ϴ��˳�ʱδ��ɵ�H�ļ�
	//CBindSQL ds(_DBConn[child_proc]);
//	sprintf(szSqlStr, " update %s set deal_flag='W' where deal_flag='H' and validflag='Y' and \
	//	proc_index = %d and source_id in(select source_id from C_SOURCE_GROUP_CONFIG \
	//	where source_group='%s')", Param.szLogTabname, Param.iProcessId, Param.szSourceGroupId);
	//theJSLog<<szSqlStr<<endi;
	//ds.Open(szSqlStr, NONSELECT_DML);
	//ds.Execute();
	//ds.Close();
	//if(_DBConn[child_proc].Commit()==0)
	//	theJSLog<<"Commit succeed"<<endi;
	//else
	//	theJSLog<<"Commit failed"<<endi;//
}
*/

//����ֵ��-1	ʧ��
//        0		�ɹ�
//		  >0	�ļ�����

//int C_MainFlow::getFilesFromDB()
//{
//	//CBindSQL ds(_DBConn[child_proc]);
//
//	char szSqlStr[SQL_LEN+1];
//	char szLogStr[LOG_MSG_LEN+1]; 
//	char szTemp[FIELD_LEN+1];
//	char szTmpPart[2+1];
//	char szCurrentTime[DATETIME_LEN+1];
//	int iRet;
//	SFileStruct FileStruct;
//	
//	memset(szCurrentTime, 0, sizeof(szCurrentTime));
//	getCurTime(szCurrentTime);
//	memset(szTmpPart, 0, sizeof(szTmpPart));
//	strncpy(szTmpPart, szCurrentTime+4, 2);
//
//	try
//	{
//		/* �����ݿ����־���е�H�ļ� */
//		int iFileCount = 0; //�������ļ�����
//	if (dbConnect(conn))
//	 {
//			Statement stmt = conn.createStatement();
//			string sql = "select count(*) from :v1 a, C_SOURCE_GROUP_CONFIG b where a.source_id= b.source_id \
//		  and a.deal_flag='W' and a.validflag='Y' and a.proc_index = :v2 and a.partid in(:v3,atoi(:v4),:v5) \
//		  and b.source_group=:v6";		
//			stmt.setSQLString(sql);
//			stmt << Param.szLogTabname << Param.iProcessId << getPrePartID(szTmpPart) << szTmpPart <<
//		         getNextPartID(szTmpPart) << Param.szSourceGroupId;
//			stmt.execute();
//			stmt >> iFileCount;				 
//	 
//		/*sprintf(szSqlStr, "select count(*) from %s a, C_SOURCE_GROUP_CONFIG b where a.source_id= b.source_id \
//		  and a.deal_flag='W' and a.validflag='Y' and a.proc_index = %d and a.partid in(%d,%d,%d) \
//		  and b.source_group='%s'", Param.szLogTabname, Param.iProcessId, getPrePartID(szTmpPart), atoi(szTmpPart),
//		  getNextPartID(szTmpPart), Param.szSourceGroupId);
//		ds.Open(szSqlStr, SELECT_QUERY );
//		//cout<<szSqlStr<<endl;
//		ds>>iFileCount;
//		ds.Close();*/
//		
//		if (iFileCount > 0)
//		{
//			iUndoFlag ++;			
//			memset(szTemp, 0, sizeof(szTemp));
//			/*���ӿ���ж���������ļ�����Ϣ��*/
//			string sql = "select filename, a.source_id from :v1 a, C_SOURCE_GROUP_CONFIG b where a.source_id= b.source_id \
//			and a.deal_flag='W' and a.validflag='Y' and a.proc_index = :v2 and b.source_group=:v3 and partid in (:v4,atoi(:v5),:v6)";		
//			stmt.setSQLString(sql);
//			stmt << Param.szLogTabname << Param.iProcessId << Param.szSourceGroupId << 
//				getPrePartID(szTmpPart) << szTmpPart << getNextPartID(szTmpPart);
//			stmt.execute();
//			conn.commit();
//			//sprintf(szSqlStr, "select filename, a.source_id from %s a, C_SOURCE_GROUP_CONFIG b where a.source_id= b.source_id \
//			//and a.deal_flag='W' and a.validflag='Y' and a.proc_index = %d and b.source_group='%s' and partid in (%d,%d,%d)",
//			//Param.szLogTabname, Param.iProcessId, Param.szSourceGroupId, getPrePartID(szTmpPart), atoi(szTmpPart), getNextPartID(szTmpPart));
//			//ds.Open(szSqlStr, SELECT_QUERY );
//			for (int i=0; i<iFileCount; i++)
//			{
//				memset(&FileStruct, 0, sizeof(FileStruct));
//				if (!(stmt >> FileStruct.szFileName >> FileStruct.szSourceId))
//				{
//					//ds.Close();
//					strcpy(szLogStr, "the information in the source table  are not correct,please check ! ");
//					theJSLog<<szLogStr<<endd;
//					return -1;
//				}
//				else
//				{
//					if (FileStruct.szSourcePath[strlen(FileStruct.szSourcePath)-1] != '/')
//						strcat(FileStruct.szSourcePath, "/");
//					//strncpy(FileStruct.szRealFileName, FileStruct.szFileName, strrchr(FileStruct.szFileName,'.')-FileStruct.szFileName);
//					strcpy(FileStruct.szRealFileName, FileStruct.szFileName);
//					sprintf(FileStruct.szFileName, "%s.%d", FileStruct.szRealFileName, Param.iProcessId);
//					if(getFileInfo( FileStruct ))
//					{
//					  theJSLog<<"׼������("<<FileStruct.szRealFileName<<")"<<endi;
//					  vecFile.push_back( FileStruct );
//					}
//					else
//						theJSLog<<"�ļ�"<<FileStruct.szRealFileName<<"�ظ���"<<endw;
//				}
//			}
//			conn.close();	
//			return iFileCount;
//		}
//	  }else{
//	 	//theJSLog << "connect error."<<endi;
//	 	theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,"connect error.");
//	 	return -1;
//	 }			 
//		return -1;
//	}
//	catch(jsexcp::CException &e) 
//	{
//		//errLog(ERR_GET_FILE, "�Ӷ��ж�ȡ�������ļ�����", e.GetAppError(), e.GetErrMessage(), __FILE__, __LINE__);
//		conn.close();
//		e.PushStack(ERR_GET_FILE, "�Ӷ��ж�ȡ�������ļ�����", __FILE__, __LINE__);
//		throw e;
//		
//	}
//	catch(...)
//	{
//		conn.close();
//		throw jsexcp::CException(errno, "�Ӷ��ж�ȡ�������ļ�����", __FILE__, __LINE__);
//		//theJSLog<<"exit :"<<errno<<ende;
//	}
//}

void C_MainFlow::getSourceInfo()
{
	//CBindSQL ds(_DBConn[child_proc]);
	//char szSqlStr[SQL_LEN+1];
	//memset(szSqlStr, 0, sizeof(szSqlStr));
	//SSourceStruct source;
	
 try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			string sql = "select a.source_id, a.source_path, a.tollcode, a.serv_cat_id, a.file_fmt, to_number(b.priority) \
		from i_source_define a, c_source_batch b, c_source_group_config c where a.source_id=b.source_id and a.source_id=c.source_id \
		and b.service=:v1 and c.source_group = :v2";		
			stmt.setSQLString(sql);
			stmt << Param.szServiceId << Param.szSourceGroupId;
			//cout << "Param.szServiceId = " << Param.szServiceId <<"Param.szSourceGroupId = "<< Param.szSourceGroupId<<endl;
			//cout <<sql <<endl;
			stmt.execute();
            stmt >>source.szSourceId>>source.szSourcePath>>source.szTollcode>>source.szServCat
		             >>source.szSourceFiletype>>source.iMaxCount;
            //cout << "source.szSourcePath = " <<source.szSourcePath <<endl;
                if(source.szSourcePath[strlen(source.szSourcePath)-1] != '/')
			         strcat(source.szSourcePath, "/");
		    mapSource.insert(pair<string, SSourceStruct>(string(source.szSourceId), source));
	 }else{
	 	//theJSLog<<"connect error."<<endi;
	 	theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,"connect error.");
	 }
	 conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
		throw jsexcp::CException(errno, "��ȡ����Դ��Ϣʧ��", __FILE__, __LINE__);
 } 	
	 
	/*sprintf(szSqlStr, "select a.source_id, a.source_path, a.tollcode, a.serv_cat_id, a.file_fmt, to_number(b.priority) \
		from i_source_define a, c_source_batch b, c_source_group_config c where a.source_id=b.source_id and a.source_id=c.source_id \
		and b.service='%s' and c.source_group = '%s'", Param.szServiceId, Param.szSourceGroupId);
	ds.Open(szSqlStr, SELECT_QUERY);
	while(ds>>source.szSourceId>>source.szSourcePath>>source.szTollcode>>source.szServCat
		>>source.szSourceFiletype>>source.iMaxCount)
	{
		if(source.szSourcePath[strlen(source.szSourcePath)-1] != '/')
			strcat(source.szSourcePath, "/");
		mapSource.insert(pair<string, SSourceStruct>(string(source.szSourceId), source));
	}
	ds.Close();*/
}

/*
bool C_MainFlow::getFileInfo(struct SFileStruct &FileStruct, bool bNewFile)
{
	char szRealFile[FILE_NAME_LEN+PATH_NAME_LEN+1];
	char szTmpFile[FILE_NAME_LEN+PATH_NAME_LEN+1];
	char szLogStr[LOG_MSG_LEN+1];
	char szSqlStr[SQL_LEN+1];
	char szCurrentTime[14+1];
	char szTmpPart[2+1];
	//CBindSQL ds(_DBConn[child_proc]);
	int iCount =0;
	
	memset(szCurrentTime, 0, sizeof(szCurrentTime));
	getCurTime(szCurrentTime);
	memset(szTmpPart, 0, sizeof(szTmpPart));
	strncpy(szTmpPart, szCurrentTime+4, 2);

	try
	{
		if(FileStruct.szSourcePath[strlen(FileStruct.szSourcePath)-1] != '/')
			strcat(FileStruct.szSourcePath, "/"); 
		map<string, SSourceStruct>::iterator it;
		if( dbConnect(conn) )
		{
		    Statement stmt = conn.createStatement();
			string sql = "select count(*) from :v1 where source_id = :v2 and filename = :v3 and partid in (:v4,atoi(:v5),:v6)";
			//���SCH�Ƿ��Ѹ��ļ�
		if (bNewFile)
	      {					
			stmt.setSQLString(sql);
			stmt << Param.szLogTabname << FileStruct.szSourceId << FileStruct.szRealFileName <<
			  getPrePartID(szTmpPart) << szTmpPart << getNextPartID(szTmpPart);
			stmt.execute();
                if(!(stmt>>iCount))
			   {
				//ds.Close();
				sprintf(szLogStr,"ִ�� %s ������",szSqlStr);
				throw jsexcp::CException(0, szLogStr, __FILE__, __LINE__);
			   }

            if(iCount > 0)
			{
			sql = "update %s set deal_flag='D' where source_id=:v1 and filename=:v2 and partid in (:v3,atoi(:v4),:v5)";		
			stmt.setSQLString(sql);
			stmt <<Param.szLogTabname << FileStruct.szSourceId << FileStruct.szRealFileName <<
			  getPrePartID(szTmpPart) << szTmpPart << getNextPartID(szTmpPart);
			stmt.execute();	
			conn.commit();
             }           

            it = mapSource.find( string(FileStruct.szSourceId) );
            if(it == mapSource.end())
			{
				// �Ȳ�����־�������쳣 /
            sql = "insert into :v1(source_id, serv_cat_id, filename, deal_flag, validflag, proc_index, \
				  dealstarttime) values(:v2, '00', :v3, 'E', 'Y', :v4, :v5)";		
			stmt.setSQLString(sql);
			stmt <<Param.szLogTabname << FileStruct.szSourceId<< FileStruct.szRealFileName<<Param.iProcessId<< szCurrentTime;
			stmt.execute();
			conn.commit();
           }

			// ��������ļ���Ϣ������־�� /
			memset(szSqlStr, 0, sizeof(szSqlStr));
            sql =  "insert into :v1(source_id, serv_cat_id, filename, deal_flag, validflag, proc_index, \
			  dealstarttime) values(:v2, :v3, :v4, 'W', 'Y', :v5, :v6)";		
			stmt.setSQLString(sql);
			stmt <<Param.szLogTabname << 
			  FileStruct.szSourceId<< (it->second).szServCat<< FileStruct.szRealFileName<< Param.iProcessId<<
			  szCurrentTime;
			stmt.execute();
			conn.commit();

			sprintf(szRealFile, "%s%s%s.0", (it->second).szSourcePath, Param.szInPath, FileStruct.szRealFileName);
			sprintf(szTmpFile, "%s%s%s.%d", (it->second).szSourcePath, Param.szInPath, FileStruct.szRealFileName, Param.iProcessId);	
			sprintf(FileStruct.szFileName, "%s.%d", FileStruct.szRealFileName, Param.iProcessId);
			DEBUG_LOG<<"���ļ� ("<<szRealFile<<") ������ ("<<szTmpFile<<")"<<endd;
			// �ļ����� /
			if(rename(szRealFile, szTmpFile) != 0 )
			{
				//�ȸ��ļ�״̬�����쳣
				sql = "update :v1 set deal_flag='E' where source_id=:v2 and filename=:v3 and \
					partid in (:v4,atoi(:v5),:v6)";		
			    stmt.setSQLString(sql);
			    stmt <<Param.szLogTabname<< FileStruct.szSourceId<< FileStruct.szRealFileName<<
					getPrePartID(szTmpPart)<< szTmpPart<< getNextPartID(szTmpPart);
			    stmt.execute();
			    conn.commit();
			}
			// ����������Ϣ //
			strcpy(FileStruct.szSourcePath, (it->second).szSourcePath);
			strcpy(FileStruct.szTollcode, (it->second).szTollcode);
			strcpy(FileStruct.szServCat, (it->second).szServCat);
			strcpy(FileStruct.szSourceFiletype, (it->second).szSourceFiletype);
			FileStruct.iMaxCount = (it->second).iMaxCount;			
		}
		else
		{
			it = mapSource.find( string(FileStruct.szSourceId) );
			if(it == mapSource.end())
			{
				sprintf(szLogStr,"�Ҳ�������Դ %s ����Ϣ��",FileStruct.szSourceId);
				throw jsexcp::CException(0, szLogStr, __FILE__, __LINE__);
			}
			strcpy(FileStruct.szSourcePath, (it->second).szSourcePath);
			strcpy(FileStruct.szTollcode, (it->second).szTollcode);
			strcpy(FileStruct.szServCat, (it->second).szServCat);
			strcpy(FileStruct.szSourceFiletype, (it->second).szSourceFiletype);
		}
		// �и�ʽ�����ʱ���ļ���ID//
		if(!strcmp(Param.szIsFmtFirst, "Y"))
		{
		    sql = "select nvl(file_id,-1), receive_time from D_FILE_RECEIVED where Source_Id=:v1 and service=:v2 \
			  and FileName=:v3 and partid in (:v4,atoi(:v5),:v6)";		
			stmt.setSQLString(sql);
			stmt << FileStruct.szSourceId<< Param.szServiceId<<
			  FileStruct.szRealFileName<< getPrePartID(szTmpPart)<< szTmpPart<< getNextPartID(szTmpPart);
			stmt.execute();
			if(!(stmt>>FileStruct.lFileId>>FileStruct.szReceiveTime))
			{
				sleep(1);
				stmt.execute();
				if(!(stmt>>FileStruct.lFileId>>FileStruct.szReceiveTime))
				{
					sprintf(szLogStr, "��ȡ%s�ļ�IDʧ�ܣ�", FileStruct.szRealFileName);
					throw jsexcp::CException(0, szLogStr, __FILE__, __LINE__);
				}
			}
		}
		else
		{
			// ���ļ�����ʱ�� //
			 sql = "select receive_time from D_FILE_RECEIVED where Source_Id=:v1 and service=:v2 \
			  and FileName=:v3 and partid in (:v4,atoi(:v5),:v6)";		
			stmt.setSQLString(sql);
			stmt << FileStruct.szSourceId<< Param.szServiceId<<
			  FileStruct.szRealFileName<< getPrePartID(szTmpPart)<< szTmpPart<< getNextPartID(szTmpPart);
			stmt.execute();
           if(!(stmt>>FileStruct.szReceiveTime))
			{
				sleep(1);
				stmt.execute();
				if(!(stmt>>FileStruct.szReceiveTime))
				{
					sprintf(szLogStr, "��ȡ%s�ļ�����ʱ��ʧ�ܣ�", FileStruct.szRealFileName);
					throw jsexcp::CException(0, szLogStr, __FILE__, __LINE__);
				}
			}
		  }

		conn.commit();
		conn.close();
		}
		return true;
	}
	catch(jsexcp::CException &e) 
	{
		//errLog(ERR_GET_FILE, "�Ӷ��ж�ȡ�������ļ�����", e.GetAppError(), e.GetErrMessage(), __FILE__, __LINE__);
		e.PushStack(ERR_GET_FILE, "��ȡ�ļ���Ϣ����", __FILE__, __LINE__);
		throw e;
		conn.close();
		return false;
	}
	catch(...)
	{
		throw jsexcp::CException(errno, "��ȡ�ļ���Ϣ����", __FILE__, __LINE__);
		conn.close();
		return false;
		//theJSLog<<"exit :"<<errno<<ende;
	}
}


char* C_MainFlow::getMsgKey(CReadIni &IniFile, char *szServiceId, char *szSourceGroupId)
{
	char szMsgList[STR_LEN+1];
	IniFile.GetValue("MSGQ_LIST", "resp_list", szMsgList, 'N');
	char szServiceCheck[MAXLENGTH+1];
	char szSourceGroupCheck[MAXLENGTH+1];
	char szMsgKey[11];
	char tmp[11];
	char *ss0,*ss1;
	ss0 = szMsgList;
	while(( ss1 = strchr( ss0,',' ) ) != NULL)
	{
		*ss1 = 0;
		strcpy(tmp, ss0);
		ss0 = ss1+1;
		*ss1 = ',';
		strcpy(szMsgKey, tmp);
		IniFile.GetValue(szMsgKey, "service_name", szServiceCheck, 'N');
		IniFile.GetValue(szMsgKey, "source_group", szSourceGroupCheck, 'N');
		if( !strcmp(szServiceId, szServiceCheck) && !strcmp(szSourceGroupId, szSourceGroupCheck) )
			return szMsgKey;
	} 
	strcpy(tmp ,ss0);
	strcpy(szMsgKey, tmp);
	IniFile.GetValue(szMsgKey, "service_name", szServiceCheck, 'N');
	IniFile.GetValue(szMsgKey, "source_group", szSourceGroupCheck, 'N');
	if( !strcmp(szServiceId, szServiceCheck) && !strcmp(szSourceGroupId, szSourceGroupCheck) )
		return szMsgKey;
	else
		throw jsexcp::CException(0, "�Ҳ�����ص��ļ����У�", __FILE__, __LINE__);
}
*/

/***********************************************************************************
*�����ݿ��л�ȡ��������Ӧ��ֵ
***********************************************************************************/
int C_MainFlow::getScheme(struct SParameter &Param)
{
	char szLogStr[LOG_MSG_LEN+1];             //��־��Ϣ
	char szSqlStr[SQL_LEN+1];
	char szTemp[FIELD_LEN+1];

	//CBindSQL ds(DBConn);
	/*��ȡ����ļ�����·��*/
	memset( Param.szSlPath, 0, sizeof(Param.szSlPath) );
	if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "PLUGIN_PATH", Param.szSlPath )== -1 )
	{
		sprintf(szLogStr, "�������������Ҳ���VARNAME='PLUGIN_PATH'����" );
		//theJSLog<<szLogStr<<endi;
		theJSLog.writeLog(LOG_CODE_ENV_MISSING,"�������������Ҳ���VARNAME='PLUGIN_PATH'����");
		return -1;
	} 
	theJSLog<<"PLUGIN_PATH="<<Param.szSlPath<<endd;
	//cout << "PLUGIN_PATH="<<Param.szSlPath<<endd;
	/* ��ȡ����ļ��� */
	memset( Param.szSlName, 0, sizeof(Param.szSlName) );
	if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "PLUGIN_NAME", Param.szSlName ) == -1 )
	{
		sprintf(szLogStr, "�������������Ҳ���VARNAME='PLUGIN_NAME'����" );
		//theJSLog<<szLogStr<<endi;
		theJSLog.writeLog(LOG_CODE_ENV_MISSING,"�������������Ҳ���VARNAME='PLUGIN_NAME'����");
		return -1;
	}
	theJSLog<<"PLUGIN_NAME="<<Param.szSlName<<endd;
	if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "COMMON_LACKINFO_SAVE_CONFIG", Param.szLackSaveTableId ) == -1 )
	{
		sprintf(szLogStr, "�������������Ҳ���VARNAME='COMMON_LACKINFO_SAVE_CONFIG'����");
		//theJSLog<<szLogStr<<endi;
		theJSLog.writeLog(LOG_CODE_ENV_MISSING,szLogStr);
		return -1;
	}
	if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "COMMON_LACKINFO_STAT_CONFIG", Param.szLackStatTableId ) == -1 )
	{
		memset(Param.szLackStatTableId, 0, sizeof(Param.szLackStatTableId));
	}
	if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "COMMON_ABNORMAL_SAVE_CONFIG", Param.szAbnSaveTableId ) == -1 )
	{
		sprintf(szLogStr, "�������������Ҳ���VARNAME='COMMON_ABNORMAL_SAVE_CONFIG'����");
		//theJSLog<<szLogStr<<endi;
		theJSLog.writeLog(LOG_CODE_ENV_MISSING,szLogStr);
		return -1;
	}
	if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "COMMON_ABNORMAL_STAT_CONFIG", Param.szAbnStatTableId ) == -1 )
	{
		memset(Param.szAbnStatTableId, 0, sizeof(Param.szAbnStatTableId));
	}
	memset(szTemp, 0, sizeof(szTemp));
	if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "CONNECT_COMMEM_FLAG", szTemp ) == -1 )
	{
		sprintf(szLogStr, "�������������Ҳ���VARNAME='CONNECT_COMMEM_FLAG'����");
		theJSLog.writeLog(LOG_CODE_ENV_MISSING,szLogStr);
		return -1;
	}
	if(!strcmp(szTemp, "Y"))
		Param.bCommemFlag = true;
	else if(!strcmp(szTemp, "N"))
		Param.bCommemFlag = false;
	else
	{
		sprintf(szLogStr, "������������ VARNAME='CONNECT_COMMEM_FLAG' ������ %s ����ȷ��", szTemp);
		theJSLog.writeLog(LOG_CODE_ENV_MISSING,szLogStr);
		return -1;
	}
	theJSLog<<"CONNECT_COMMEM_FLAG="<<szTemp<<endd;
	if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "IS_FMT_FIRST", Param.szIsFmtFirst ) == -1 )
	{
		sprintf(szLogStr, "�������������Ҳ���VARNAME='IS_FMT_FIRST'����");
		theJSLog.writeLog(LOG_CODE_ENV_MISSING,szLogStr);
		return -1;
	}
	theJSLog<<"IS_FMT_FIRST="<<Param.szIsFmtFirst<<endd;
	if(!strcmp(Param.szIsFmtFirst, "Y"))
	{
		/* �и�ʽ��ʱ */
		
		/* ��ʽ�������Ĵ������ */
		if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "FMT_ERR_W_TABLE", Param.szFmtErr2Table ) == -1 )
		{
			sprintf(szLogStr, "�������������Ҳ���VARNAME='FMT_ERR_W_TABLE'����");
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,szLogStr);
			return -1;
		}
		theJSLog<<"FMT_ERR_W_TABLE="<<Param.szFmtErr2Table<<endd;
		if(strncmp(Param.szFmtErr2Table, "Y", 1) == 0)
		{
			//��ȡ�����ϼ�¼����������ͳ�Ʊ���ID
			memset( Param.szFmtErrSaveTableId, 0, sizeof(Param.szFmtErrSaveTableId) );
			if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "FMT_ERR_SAVE_CONFIG", Param.szFmtErrSaveTableId ) == -1 )
			{
				sprintf(szLogStr, "�������������Ҳ���VARNAME='FMT_ERR_SAVE_CONFIG'����" );
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,szLogStr);
				return -1;
			} 
			theJSLog<<"FMT_ERR_SAVE_CONFIG="<<Param.szFmtErrSaveTableId<<endd;
			memset( Param.szFmtErrStatTableId, 0, sizeof(Param.szFmtErrStatTableId) );
			if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "FMT_ERR_STAT_CONFIG", Param.szFmtErrStatTableId ) == -1 )
			{
				memset( Param.szFmtErrStatTableId, 0, sizeof(Param.szFmtErrStatTableId) );
			}
			else
				theJSLog<<"FMT_ERR_STAT_CONFIG="<<Param.szFmtErrStatTableId<<endd;
		}
		/*if( getEnvFromDB( _DBConn, Param.szServiceId, Param.szSourceGroupId, "", "FMT_ERR_W_OUTFILE", Param.szFmtErr2File ) == -1 )
		{
			sprintf(szLogStr, "�������������Ҳ���VARNAME='FMT_ERR_W_OUTFILE'����");
			theJSLog<<szLogStr<<endi;
			return -1;
		}
		theJSLog<<"FMT_ERR_W_OUTFILE="<<Param.szFmtErr2File<<endd;
		if( getEnvFromDB( _DBConn, Param.szServiceId, Param.szSourceGroupId, "", "FMT_ERR_DIR", Param.szFmtErrDir ) == -1 )
		{
			sprintf(szLogStr, "�������������Ҳ���VARNAME='FMT_ERR_DIR'����");
			theJSLog<<szLogStr<<endi;
			return -1;
		}
		if(Param.szFmtErrDir[strlen(Param.szFmtErrDir)-1] != '/')
			strcat(Param.szFmtErrDir, "/");
		theJSLog<<"FMT_ERR_DIR="<<Param.szFmtErrDir<<endd;*/
		if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "FMT_TIMEOUT_DIR", Param.szFmtTimeOutDir ) == -1 )
		{
			sprintf(szLogStr, "�������������Ҳ���VARNAME='FMT_TIMEOUT_DIR'����");
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,szLogStr);
			return -1;
		}
		if(Param.szFmtTimeOutDir[strlen(Param.szFmtTimeOutDir)-1] != '/')
			strcat(Param.szFmtTimeOutDir, "/");
		theJSLog<<"FMT_TIMEOUT_DIR="<<Param.szFmtTimeOutDir<<endd;
		if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "FMT_OTHER_DIR", Param.szFmtOtherDir ) == -1 )
		{
			sprintf(szLogStr, "�������������Ҳ���VARNAME='FMT_OTHER_DIR'����");
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,szLogStr);
			return -1;
		}
		if(Param.szFmtOtherDir[strlen(Param.szFmtOtherDir)-1] != '/')
			strcat(Param.szFmtOtherDir, "/");
		theJSLog<<"FMT_OTHER_DIR="<<Param.szFmtOtherDir<<endd;
	}//end of fmt first
	memset(szTemp, 0, sizeof(szTemp));
	if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "SERV_CAT_CONFIG", szTemp ) == -1 )
	{
		sprintf(szLogStr, "�������������Ҳ���VARNAME='SERV_CAT_CONFIG'����");
		theJSLog.writeLog(LOG_CODE_ENV_MISSING,szLogStr);
		return -1;
	}
	Param.iServCatConfig=atoi(szTemp);
	theJSLog<<"SERV_CAT_CONFIG="<<Param.iServCatConfig<<endd;
	memset(szTemp, 0, sizeof(szTemp));
	if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "OUTPUT_FILE", szTemp ) == -1 )
	{
		strcpy(szTemp, "Y");
	}
	if(!strcmp(szTemp, "Y"))
		Param.bOutputFile = true;
	else
		Param.bOutputFile = false;
	theJSLog<<"OUTPUT_FILE="<<Param.bOutputFile<<endd;

	memset(szTemp, 0, sizeof(szTemp));
	if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "INFILE_BAK_FLAG", szTemp ) == -1 )
	{
		strcpy(szTemp, "N");
	}
	if(!strcmp(szTemp, "Y"))
		Param.bBakFlag = true;
	else if(!strcmp(szTemp, "N"))
		Param.bBakFlag = false;
	else
	{
		sprintf(szLogStr, "������������ VARNAME='INFILE_BAK_FLAG' ������ %s ����ȷ��", szTemp);
		theJSLog.writeLog(LOG_CODE_ENV_MISSING,szLogStr);
		return -1;
	}
	theJSLog<<"INFILE_BAK_FLAG="<<szTemp<<endd;

	/***********************************************************************
	/*  ��ѯ������ģ��ID��workflow_id��
	/***********************************************************************/
	 string sql;
	// try{			
	 	
	if (dbConnect(conn))
	 {
		Statement stmt = conn.createStatement();
		sql = "select server_id, workflow_id, filetype_id from c_source_group_define where source_group=:v1";		
		stmt.setSQLString(sql);
		stmt << Param.szSourceGroupId;
		//cout<<"Param.szSourceGroupId:"<<Param.szSourceGroupId<<endl;
		try{
			if(!stmt.execute()){
				conn.close();
				 throw jsexcp::CException(0, "�Ҳ���������������", __FILE__, __LINE__);
			}
			stmt >> Param.szServerId>>Param.szWorkflowId>>Param.szOutputFiletypeId;
			//cout<< Param.szServerId<<Param.szWorkflowId<<Param.szOutputFiletypeId<<endl;
		}catch(SQLException e){
			cout<<e.what()<<endl;
			cout<<e.getSQLString()<<endl;
			conn.close();
			throw jsexcp::CException(0, "�Ҳ���������������", __FILE__, __LINE__);
			return -1;
		}

		/* ��ѯ����������Ʊ�����·�� */
        sql = "select a.log_tabname, a.work_path from c_service_interface a, c_service_flow b \
		where a.interface_id=b.input_id and b.workflow_id=:v1 and b.service=:v2";		
		stmt.setSQLString(sql);
		stmt << Param.szWorkflowId<< Param.szServiceId;
		stmt.execute();
		if(!(stmt >> Param.szLogTabname>>Param.szInPath))
  	    {
  		  throw jsexcp::CException(0, "�Ҳ���������������", __FILE__, __LINE__);
  	    }
		//cout << "Param.szLogTabname>>Param.szInPath = " << Param.szLogTabname <<endl;
        if(Param.szInPath[strlen(Param.szInPath)-1] != '/')
		    strcat(Param.szInPath, "/");
         sql = "select a.input_path from c_service_interface a, c_service_flow b where \
		a.interface_id=b.output_id and b.workflow_id=:v1 and b.service=:v2";		
		stmt.setSQLString(sql);
		stmt << Param.szWorkflowId<<Param.szServiceId;
		stmt.execute();
		if(!(stmt >> Param.szOutPath))
  	    {
  		  if(Param.bOutputFile)
			throw jsexcp::CException(0, "�Ҳ���������������", __FILE__, __LINE__);
  	    }
		if(Param.bOutputFile)
	   {
		if(Param.szOutPath[strlen(Param.szOutPath)-1] != '/')
			strcat(Param.szOutPath, "/");
	   }

	  /* �������ļ����� */
	  theJSLog<<"szOutputFiletypeId="<<Param.szOutputFiletypeId<<endd;
       sql = "select record_type from c_filetype_define where filetype_id=:v1";		
		stmt.setSQLString(sql);
		stmt << Param.szOutputFiletypeId;
		stmt.execute();
		if(!(stmt >>Param.szOutrcdType))
  	    {
  		   sprintf(szLogStr, "�ڱ�C_FILETYPE_DEFINE���Ҳ��� %s ������", Param.szOutputFiletypeId);
		   //theJSLog<<szLogStr<<endi;
		   throw jsexcp::CException(0, szLogStr, __FILE__, __LINE__);
  	    }

		sql = "select interval from c_process_ctl where source_group = :v1 AND service = :v2";		
		stmt.setSQLString(sql);
		stmt << Param.szSourceGroupId<< Param.szServiceId;
		stmt.execute();
		if(!(stmt >>szTemp))
  	    {
  		   throw jsexcp::CException(0, "�Ҳ���sleeptime������", __FILE__, __LINE__);
  	    }
        Param.iSleepTime = atoi(szTemp);
	    theJSLog<<"SleepTime="<<Param.iSleepTime<<endd;		
			
	 }else{
	 	//theJSLog<<"connect error."<<endi;
	 	theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,"connect error.");
	 }
	 //conn.commit();
	 conn.close();
	// } catch( SQLException e ) {
	/// 	cout<<e.what()<<endl;
	//	cout<<sql<<endl;
	//	conn.close();
	// 	throw jsexcp::CException(0, "Connect DB Err", __FILE__, __LINE__);	
 //  }
	return 0;
}

/*void C_MainFlow::exit()
{
	if(chain != NULL)
	{
		//cout<<"send message quit"<<endl;
		MessageParser message;
		ArgMessage argMsg;
		Argument *pMsg = &argMsg;

		char szCurrentTime[DATETIME_LEN+1];
		getCurTime(szCurrentTime);
		message.setMessage(MESSAGE_PROGRAM_QUIT, "", "", 0);
		argMsg.set(message);
		iRad++;
		if(iRad > 99999)
			iRad = 1;
		argMsg.setTime(szCurrentTime, iRad);
		chain->signal(pMsg);
	}
	//Param.ProcMonitor.UpdateMonitor(CProcInfo::SERV_EXIT);
	Param.info.msgInfoLog(INFO_QUIT);
}*/

/*
bool C_MainFlow::DBLinkError()
{
	return m_bDBLinkError;
}
*/
/*
void C_MainFlow::resetAll()
{
	if(chain != NULL)
	{
		MessageParser message;
		ArgMessage argMsg;
		Argument *pMsg = &argMsg;

		char szCurrentTime[DATETIME_LEN+1];
		getCurTime(szCurrentTime);
		message.setMessage(MESSAGE_BREAK_BATCH, "", "", 0);
		argMsg.set(message);
		iRad++;
		if(iRad > 99999)
			iRad = 1;
		argMsg.setTime(szCurrentTime, iRad);
		chain->signal(pMsg);
	}
}
*/

int C_MainFlow::onBeforeTask()
  {
     // ������ƽ�⻰��������
     //cout << "test onbeforeTask" << endl;
     //cout << "pkgstatus1 = " << pkgstatus <<endl;
  //   int ret=1;
	//int event_sn, event_type;
	//long param1, param2, src_id;
	
	/*
	while(ret)	//����ӽ�����Ϣ����
	{
		ret=mgetChldEvt(event_sn, event_type, param1, param2, src_id, false);
		if(ret<0)
		{
			cerr<<"����ӽ�����Ϣ����ʧ��!\n";
			return false;
		}
	}
	*/
         
     //cout << "test onbeforeTask" << endl;
     Update();
     return 1; // ��ʼ���뻰����
  }

int C_MainFlow::onTaskBeginForMainFlow(void *task_addr)
  {   
     // ����дsql �ļ�����
     //PS_BillProcess::isWriteSQLFile();  
     //PS_Process::isWriteSQLFile(); 
     //cout << "onTaskBegin" <<endl;
    
     // ���ݿ������жϵ��ӽ�������Ŀ�����仯ʱ����������     
     
   //  if (tmp_childnum == 0)
   //  {
   //     tmp_childnum = PS_BillProcess::getChldPrcNum(); 
    // }
    /* if (tmp_childnum != PS_BillProcess::getChldPrcNum())
     {
        DBConn.Disconnect();
        connectDB( "", DBConn );
     }*/

   //  PkgBlock pkg((char*)(task_addr),0,0);
     PkgBlock pkg((char*)task_addr);
	   pkg.init(getTicketLength(),getBlockSize());
	
	  if(pkg.getRecordNum() == 0)
	  {
		  theJSLog<<"�������¼����Ϊ0,����..."<<endw;
		  //return ticket_num;
	  }
	
    // int length = PS_BillProcess::getTicketLength();
    // int block = PS_BillProcess::getBlockSize();
    // pkg.init( length , block ); //�����¼�д�С�Ϳ��С

     pkgstatus = pkg.getStatus(); //��ȡ������״ֵ̬

      // �����ļ���
     //��������ĳ���ļ��е�λ��	D(��ʾһ���ļ�ȫ��һ��������) S(���������ļ���ʼλ��) M(���������ļ���λ��) E(���������ļ�ĩβ)
     //char* getBlkPos();
     char* pos = pkg.getBlkPos();
     char tmpFileName[MAXLENGTH+1];
     char orgFileName[MAXLENGTH+1];
     char block_pos[3] = {0};
     strcpy(block_pos,pos);
     if( (strcmp(block_pos,"D") == 0) || (strcmp(block_pos,"E") == 0) )
	 {
		fileOverFlag = true;
	 }
     if( strcmp(block_pos,"S") == 0 )
     	fileOverFlag = false;
     //cout << "pos = " << *pos <<endl;
     //theJSLog<<"���������ļ��е�λ��Ϊ "<<tmppos<<endi;
    // if (strcmp(tmppos,"D") == 0)
   //  {
         //�����ļ���
         //char tmpfilename[40] = {0};
         strcpy(tmpFileName,pkg.getFileName());
        PS_Process::setSQLFileName(tmpFileName);
        theJSLog<<"*********************************************************"<<endi;
        theJSLog<<"��ʼ�����ļ� "<<tmpFileName<<endi;
  //   }
  //   else if(strcmp(tmppos,"S") == 0) // ȡ���֮ǰ������
  //   {
 //       strcpy( tmpFileName,pkg.getFileName() );
        //char* p = strrchr(tmpFileName,'#');			//���ԭ�ļ���
	  //  if(p)
	   // {
	//	  strncpy(orgFileName,tmpFileName,(p-tmpFileName));
	 //   }
	  //  else
	   // {
		//   strcpy(orgFileName,tmpFileName);
	//    }
	    //����ԭʼ�ļ���
	 //   theJSLog<<"�ļ���Ϊ "<<orgFileName<<endi;
	 //   PS_Process::setSQLFileName(orgFileName);
   //  }
     
     //cout << "pkgstatus2 = " << pkgstatus <<endl;
     if ( pkgstatus == 3 || pkgstatus == 2)  //״̬Ϊ3������,״̬Ϊ2��ʾ���������
     	{
     	   return 0;
     	}
     else {
     	pkg.setStatus(1);
     	}
     theJSLog<<"������״̬Ϊ "<<pkgstatus<<endi;
     //ticket_num   = pkg.getRecordNum();
     //cout << "ticket_num = " << pkg.getRecordNum() << endl;
     
     ArgMessage argMsg;   
     char szCurrentTime[DATETIME_LEN+1];
	 MessageParser message;
     Argument *pMsg = &argMsg;

     //block_addr = task_addr;
     //cout << "task_addr " << *task_addr <<endl;
     strcpy(szFileName,pkg.getFileName());
     strcpy(szSourceId,pkg.getSourceId());
     
	 getCurTime(szCurrentTime);
	 getCurTime(m_szBeginTime);  //�ļ���ʼ����ʱ��
     	//}
	// ��ȡҵ�����
    /* try{	
     	memset(servCat,0,sizeof(servCat));		
	  if (dbConnect(conn))
	    {
			Statement stmt = conn.createStatement();
			string sql = "select serv_cat_id from i_source_define where source_id = :v1 ";		
			stmt.setSQLString(sql);
			stmt << szSourceId;
			stmt.execute();
			stmt >> servCat;			
	    }else{
	 	//theJSLog<<"connect error."<<endi;
	 	sprintf(servCat,"%s","G1");  //������Ӵ������ó�ʼֵ
	 	theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,"connect error.");
	  }
	   conn.commit();
	   conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
		//throw jsexcp::CException(0, "��ȡҵ�����ʧ��", __FILE__, __LINE__);
    } 	  */ 
	  // if(strlen(servCat)==0)
	   //	 sprintf(servCat,"%s","G1");
	// ׼�������뻰����
     return 1;
  }

/*
int C_MainFlow::onTaskOver(int child_ret)
  {
  // �ӻ������л�ȡ��������
    //cout << "onTaskOver �����̺���" << endl;
    PkgBlock pkg((char*)(block_addr));
    int length = PS_BillProcess::getTicketLength();
    int block = PS_BillProcess::getBlockSize();
    pkg.init( length , block ); //�����¼�д�С�Ϳ��С 

    pkgstatus = pkg.getStatus(); //��ȡ������״ֵ̬
    //cout << "pkgstatus5 = " << pkgstatus <<endl;
   
    char * pkgRecord;                    //��������
	int pkgSize;                         //�����м�¼��С
    
    int ticket_num = pkg.getRecordNum();
    //cout << "�������ַ" << block_addr << endl; 
    //cout << "ticket_num = " << ticket_num << endl;
    


    
      child_ret = PS_BillProcess::getChldPrcNum();  

      // �����������ο�ʼ����Ϣ 
     ArgMessage argMsg;   
     char szCurrentTime[DATETIME_LEN+1];
	 MessageParser message;
     Argument *pMsg = &argMsg;     
		
	// 3��д��Ϣ��
	
    char szTemp[FIELD_LEN+1];
	memset(szTemp, 0, sizeof(szTemp));
	int iRet;
	int iFileIndex = vecFile.size() - 1;  
	Param.info.SetFile(vecFile[iFileIndex].szFileName, vecFile[iFileIndex].szSourceId, vecFile[iFileIndex].szDealStartTime, \
		vecFile[iFileIndex].szDealEndTime, 'Y');
	Param.info.SetCount(m_dealRcdNum.getTotal(), m_dealRcdNum.getRight(), m_dealRcdNum.getError(), \
		m_dealRcdNum.getLack(), m_dealRcdNum.getPick(), m_dealRcdNum.getOther());
	Param.info.SetFee(m_dealRcdNum.getTotalFee(), m_dealRcdNum.getRightFee(), m_dealRcdNum.getErrorFee(), \
		m_dealRcdNum.getLackFee(), m_dealRcdNum.getPickFee(), m_dealRcdNum.getOtherFee());
	Param.info.SetDuration(m_dealRcdNum.getTotalTime(), m_dealRcdNum.getRightTime(), m_dealRcdNum.getErrorTime(), \
		m_dealRcdNum.getLackTime(), m_dealRcdNum.getPickTime(), m_dealRcdNum.getOtherTime());
	Param.info.InsertData();
	
      // �ӽ��̴�����ϣ������̴Ӷ�����ȡ������
       // ƽ�⻰�������
     long _total = 0;  //�ܻ�����
     long _right = 0 ;  //��ȷ������
     long _lack = 0 ;  //�����ϵ�
     long _error = 0 ;  //����������ʽ��ȡ����ȥ�ص�Ҳ�Ǵ���     
     long _pick = 0 ;   //�ּ�,����ּ�
     long _other = 0 ;  //����������
     
     //�����ӽ�����Ŀ������Ϣ�����л�ȡ����
     for (int i=1;i<=child_ret;i++)
     {
        long param1=0.0,param2=0.0;
        long srcid = 0;
        int ret=mgetChldEvt(0, EVT_REQ_TRNUM, param1, param2, srcid , false);
        _total = param1/100000+_total;
        _right = param1%100000+_right;
        _lack = param2/100000+_lack;
        _error = param2%100000+_error;       
        //cout << "srcid = " << srcid << "param1 = " << param1 << "param2 = " << param2 <<endl;                   
     }

      char allval[1024];  // ���ڷŵ���������
      //�ּ𵥻�ȡͨ������־
      for (int i = 1; i<=ticket_num ; i++)
      {
        char *tmp = pkg.getRcdType(i);
        char *ruletype = pkg.getRcdCode(i);
        //cout << "ruletype = " << ruletype<< endl;
        //cout << "tmp = " << tmp<< endl;
  
      }

      
     cout <<"�������ۼ�ֵ"<< " total = "<<_total<<"  _right = "<<_right<<"  _lack= "<<_lack<<"  _error = "<<_error
     	 <<"  _pick = "<< _pick << "  _other = " << _other <<endl;  
        if(_total != _right)
        {
           pkg.setStatus(3);
        }
        
       
///////////////////////////////////////////////////////////////////////////////////////////////
     	//��ȡ��Ҫ�ٲõ����ݣ��������ִ���
		//int tmpruleid =0;
		//std::vector<string> tmpfilenames;
		//Param.classify.getAllRule(tmpruleid,tmpfilenames);
		Param.classify.endFile();
		// �ύ �ּ�
		Param.classify.commit();
		
		m_dealRcdNum.set(servCat,_total,_right,_lack,_error,_pick,_other); //��ƽ�⻰�������õ���������     
        m_dealRcdNum.check();
        m_dealRcdNum.updateTable(Param.szLogTabname,szFileName,m_szBeginTime, szSourceId,Param.iProcessId);
        // sql ������
        //cout << " ��Ҫ�������m_dealRcdNum.outSqlStr = " << m_dealRcdNum.getSqlstr() <<endl;
       
        PS_Process::writeSQL(m_dealRcdNum.getSqlstr());
        if( strlen(m_dealRcdNum.getSqlstr()) == 0)
        	PS_Process::rollBackSQL();
        PS_Process::commitSQLFile();

    	
      
        
     

       
     
   
     //cout<<"test DBConn is connected or not!"<<endl;
     //cout<<"DBConn.IsConnected():"<<DBConn.IsConnected()<<endl;
   
     //cout<<"return child_ret:"<<child_ret<<endl;
    //m_dealRcdNum.updateTable(Param.szLogTabname,szFileName,m_szBeginTime, szSourceId,Param.iProcessId);
    
      // �����ӽ����� 
       if ( pkgstatus == 3 )  //״̬Ϊ3������
     	{
     	   return child_ret;
     	}
        else {
     	pkg.setStatus(2);   //������֮�����״̬����
     	} 
     return child_ret;
  }
*/


bool C_MainFlow::onChildInitForMainFlow()
  {
  			/***********************************************************************************

		*��ȡ���������е����빲���ڴ�ĵ�ַ

		***********************************************************************************/
		if(Param.bCommemFlag)
		{
		    //CBindSQL ds(DBConn);
		    //theJSLog << "C_AccessMem init" <<endd;
			memManager = new C_AccessMem();
			memManager->init(Param.szServerId,m_szEnvPath);
		}
		
	    Param.pluginInitializer.initialize(Param.szServiceId, Param.szSourceGroupId, Param.szSlPath, Param.szSlName);	  	    
    	    theJSLog<<"��ʼ��������ɹ���"<<endi;
    
    	    if(chain == NULL)
    		{
    			theJSLog<<"chain is null"<<endi;
    		}	    
    
        char szCurrentTime[DATETIME_LEN+1];
    
    		initPlugin.set(Param.szSourceGroupId, Param.szServiceId, Param.iProcessId, memManager);
    	getCurTime(szCurrentTime);
    	initPlugin.setTime(szCurrentTime, 0);
  	//cout <<"Param.szServiceId  "<< Param.szServiceId <<endl;

    chain = Param.pluginInitializer.getFilterChain();
	//cout <<"chain ="<<chain<<endl;
		if(chain == NULL)
		{
			Param.info.msgInfoLog(INFO_INITFAIL);
			cout<<"init chain error"<<endl;
			//sprintf(szLogStr, "empty plugin ");
			//errLog(4, "��ʼ��ʧ��", errno, szLogStr, __FILE__, __LINE__);
			return false;
		}
		
	chain->signal(&initPlugin);
	theJSLog<<"chain init successful..."<<endi;
     pps = new PacketParser();
	 res = new ResParser();
	 theJSLog<<"init service successful..."<<endi;
	      return true;
  }

int C_MainFlow::onTask(void *task_addr, int offset, int ticket_num)
  {

    ////////////////////////////////////////
   // cout << "ontask" <<endl;
    //Update();
    theJSLog.reSetLog();
    m_dealRcdNum.clear(); 
    onTaskBeginForMainFlow(task_addr);
    
    //PkgBlock pkg((char*)(task_addr),offset,ticket_num);
    //int length = PS_BillProcess::getTicketLength();
    //int block = PS_BillProcess::getBlockSize();
    PkgBlock pkg((char*)task_addr);
	  pkg.init(getTicketLength(),getBlockSize());
	
	  if(pkg.getRecordNum() == 0){
	  	theJSLog<<"�������¼����Ϊ0,����..."<<endw;
	  }
	  	
    theJSLog.reSetLog();

    pkgstatus = pkg.getStatus(); //��ȡ������״ֵ̬
    pkg.setModuleId(getModuleID());
     if (pkgstatus == 3 || pkgstatus ==-1)  //״̬Ϊ3������
     	{
     	   return 0;
     	}
     else {
     	pkg.setStatus(1);
     	}  

	//cout << "�������ַ" << task_addr << endl; 
    //cout<<"������¼��: "<<pkg.getRecordNum()<<endl;
	//cout<<"ģ��ID: "<<pkg.getModuleId()<<endl;
	//cout<<"����Դ�� "<<pkg.getSourceId()<<endl;
	//cout<<"�����ļ�����"<<pkg.getFileName()<<endl;
	//cout<<"��¼״̬��"<<pkg.getStatus()<<endl;			
	//cout << "��¼�д�С" << pkgSize << endl; 
	char * pkgRecord;                    //��������
    
        char szCurrentTime[DATETIME_LEN+1];
		char szLogStr[LOG_MSG_LEN+1]; 
		
		/*if(chain != NULL)
		{
			//delete chain;
			chain = NULL;
		}*/
		chain = Param.pluginInitializer.getFilterChain();
		//cout <<"chain "<<chain<<endl;
		if(chain == NULL)
		{
			Param.info.msgInfoLog(INFO_INITFAIL);
			sprintf(szLogStr, "empty plugin ");
			errLog(4, "��ʼ��ʧ��", errno, szLogStr, __FILE__, __LINE__);
			return false;
		}
		
     ArgMessage argMsg;   
	 MessageParser message;
     Argument *pMsg = &argMsg;

	 theJSLog<<"��ʼ���ɹ���"<<endi;
		
	 theJSLog<<"������Ϣ�����ο�ʼ��"<<endi;
	 strcpy(szFileName,pkg.getFileName());
	 //cout <<"û���ͳ�ȥ	" <<szFileName<<endl;
     strcpy(szSourceId,pkg.getSourceId());
	 //20131127 lij add
	 /*message.setMessage(MESSAGE_NEW_BATCH, szSourceId, szFileName, 0);	 
	 argMsg.set(message);
	 
	 iRad++;
		if(iRad > 99999)
			iRad = 1;
			argMsg.setTime(szCurrentTime, iRad);
			if(chain == NULL)
			{
				cout<<"chain is null"<<endl;
			}
			chain->signal(pMsg);

		getCurTime(szCurrentTime);
		theJSLog<<"������Ϣ���ļ���ʼ��"<<endi;
		// cout <<"û���ͳ�ȥ	" <<szFileName<<endl;
		//message.setMessage(MESSAGE_NEW_FILE, szSourceId, szFileName, 0);
		//argMsg.set(message);
		iRad++;
		if(iRad > 99999)
			iRad = 1;
		argMsg.setTime(szCurrentTime, iRad);*/
		theJSLog << "setTime " << szCurrentTime <<endi;

///////////////////////////////////////////////////////////////////////////
    // �ӻ������л�ȡ��������  

	char szSourcePath[PATH_NAME_LEN+1];  //����Դ·��
	char szServCat[SERVER_LEN+1];        //����ԴĬ�ϵ�ҵ�����
	//char * pkgRecord;                    //��������
	int pkgSize;                         //�����м�¼��С
	    
	ticket_num   = pkg.getRecordNum();
	pkgSize   = pkg.getLineSize();    //�����м�¼��С	
	
     // 1����ȡָ�룬��������    
     // 2�����ò���������������
    C_DealCalculator m_dealRcdNum;   
  //  vector<CDealedFileInfo> vecFile;
	//vector<string> vecRec;
	
	bool isEnablePerformanceAnalyzer =
	Param.pluginInitializer.getPerformanceAnalyzer() != NULL;
	unsigned long plugin_process_counter = 0, other_process_counter = 0;
	struct timeval file_start_time, file_finish_time, ana1_start_time,ana1_finish_time;
	timerclear(&file_start_time);
	timerclear(&file_finish_time);
	timerclear(&ana1_start_time);
	timerclear(&ana1_finish_time);
	gettimeofday(&file_start_time, NULL);

	/*�������*/

	//char szLogStr[LOG_MSG_LEN+1];
	char szLongStr[MAX_LINE_LENGTH+1];

	int iFmtTimeOutNum = 0;
	int iFmtOtherNum = 0;
	int iFmtErrorNum = 0;

    CFmt_Change _inrcd;
	//CBindSQL ds(_DBConn[child_proc]);
	//cout << "Param.szOutputFiletypeId " <<Param.szOutputFiletypeId<<endl;
    _inrcd.Init(Param.szOutputFiletypeId,'0');
    theJSLog << "Param.szOutputFiletypeId " <<Param.szOutputFiletypeId<<endi;

	char tmpRec[RECLEN+1];
	memset(tmpRec, 0, sizeof(tmpRec));  
	int iTotalNum = 0, iLackInfoType, iBcount, iOcount;
	pluginAnaResult Result;
	int classifyType;
	//char RecordNo[FIELD_LEN];
	//char szBuff[MAX_LINE_LENGTH+1];
	//memset(szBuff, 0, sizeof(szBuff));
	
	memset(szServCat, 0, sizeof(szServCat));
	//FilterChain *_chain = Param.pluginInitializer.getFilterChain();
	//20131127 lij add
	/*if(chain == NULL)
	{
		sprintf(szLogStr, "�Ҳ��������");
		theJSLog<<szLogStr<<endd;
		theJSLog.writeLog(LOG_CODE_FINDPLUGIN_ERR,"�Ҳ��������");
		return -1;
	}*/
	ArgRecord rcd;
	Argument *pRcd = &rcd;
	ActionResult *ret;
	pps->clearAll();
	res->clearAll();

	/* ��Ĭ�ϵ���ʱ���� */

	for(TMP_VAR::iterator it = Param.map_DefVar.begin(); it != Param.map_DefVar.end(); it++)
	{
    	pps->addVariable(it->szVarName, it->szVarValue);
		res->addVariable(it->szVarName, it->szVarValue);
		//cout << "it->szVarName = " << it->szVarName << " , it->szVarValue = " << it->szVarValue << endl;
	}	

  /* int recordlenth;
   try{			
	if (dbConnect(conn))
	  {
			Statement stmt = conn.createStatement();
			string sql = "select record_len from c_filetype_define where filetype_id = :v1 ";		
			stmt.setSQLString(sql);
			stmt << Param.szOutputFiletypeId;
			stmt.execute();
			stmt >> recordlenth;

	  }else{
	 	//theJSLog<<"connect error."<<endi;
	 	theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,"connect error.");
	 	//return -1;
	 }
	 //conn.commit();
	 conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
		throw jsexcp::CException(0, "��ȡ��ʽ����ʧ��", __FILE__, __LINE__);
		return -1;
     } 	 */
	
	/* �ظ���ȡ���еĻ�����������д��� */ 
    //cout << "Param.szSourceGroupId = " << Param.szSourceGroupId <<endl;
	//cout <<"servCat = " << servCat <<endl;
	//cout << "recordlenth = " << recordlenth <<endl;
	theJSLog<<"File in processing..."<<endd;
	try
	{
		//while(1)
		//{
			// Init record
	   m_dealRcdNum.clear();
	   theJSLog<<"File record count is "<<ticket_num<<endd;
	   for(int k = 1;k<=ticket_num;k++)
       {
		  PkgFmt fmt = pkg.readPkgRecord(k);
          //pkgRecord = pkg.getPkgRcd(k);  //  ��ȡָ���м�¼ֵ      
          //���û���������־  �Ѵ�����Y�����ڴ�����T����������H��δ������N         
          pkg.setRcdStatus(k,"T");
          if (k != ticket_num)  //�����һ��
          {
             pkg.setRcdStatus(k+1,"H");
             //cout << "next ����״ֵ̬ = " << pkg.getRcdStatus(k+1) << endl;
          }
		  //cout << "����״ֵ̬ = " << pkg.getRcdStatus(k) << endl;
		  
		 // theJSLog<<"��¼״̬��"<<fmt.status<<"  ��¼���룺"<<fmt.code<<"  ��¼ֵ��|"<<fmt.record<<"|"<<endd;
		  //cout<<"��¼״̬��"<<fmt.status<<"  ��¼���룺"<<fmt.code<<"  ��¼ֵ��|"<<fmt.record<<"|"<<endl;
			try {
			    //cout << "plugin 1" <<endl;
				pps->clear();
				res->clear();
				Result = eNormal;
				classifyType = 0;
				/* д�ļ�ƫ���� */
                pps->setOffset(offset);     
                _inrcd.Set_record(fmt.record,recordlenth); 
               // cout<<"recordlenth = " <<recordlenth<<endl;
 
                if (iTotalNum == k)
               	{
               	   m_dealRcdNum.set(servCat, 0, 0, 0, 0, 0, 0);
				   break;
                }
				iTotalNum++;
				pps->setRcd(_inrcd);
				res->setRcd(_inrcd);
				pRcd->set(*pps, *res);
			} catch (jsexcp::CException &e) {
			    theJSLog << "initialize record error" <<endw;
			    pkg.setStatus(3);
			    return 0;
				e.PushStack(e.GetAppError(), "initialize record error", __FILE__,__LINE__);				
				throw e;
			} catch (const std::exception &e) {
			    theJSLog << "Set_record error !" <<endw;
			    pkg.setStatus(3);
			    return 0;
				throw jsexcp::CException(0, e.what(), __FILE__, __LINE__);
			} catch (...) {
			    theJSLog << "unknown error !" <<endw;
			    pkg.setStatus(3);
			    return 0;
				throw jsexcp::CException(0, "unknown error", __FILE__, __LINE__);
			}

		try{
			/* ������� */
		//	theJSLog << "plugin analyse begin" <<endd;
			chain->reset();
			while( chain->hasNextAction() )
			{
			  //cout << "to find next action" <<endl;
				ret = chain->runNextAction(pRcd);
				if (isEnablePerformanceAnalyzer) {
					plugin_process_counter += ret->getUsedTimeMicros();
					gettimeofday(&ana1_start_time, NULL);
				}
				if( ret->getRelatedActionType() != "PLUGIN-EXECUTOR" )
					continue;
				Result = pRcd->getAnaType();
				if( Result == eNormal)
				{
					pps->copyRes(*res);
					continue;
				}
				else if( Result == eClassifiable)
				{
					classifyType = Param.classify.dealRecord(res->getRuleType(), pps->m_inRcd);
					if(classifyType == 1)
					{
						//����ּ𻰵�
						pkg.setRcdType(k,"E");
						fmt = pkg.readPkgRecord(k);
						//cout << "����ּ𻰵�" << pkg.getRcdType(k) << endl;
						pkg.setRcdCode(k,res->getRuleType());
						//cout << "����ּ�code = " << res->getRuleType() << endl;
						break;
					}
					else if(classifyType == 3)
					{
						//���ݷּ𻰵�
						pkg.setRcdType(k,"E");  
						//cout << "���ݷּ𻰵�" << pkg.getRcdType(k) << endl;
						//cout << "���ݷּ�code = " << pkg.getRcdCode(k) << endl;
						
						res->resetAnaResult();
						Result = eNormal;
						// ��¼������ //
						memset(szServCat, 0, sizeof(szServCat));
						(pps->m_inRcd).Get_Field(Param.iServCatConfig, szServCat);
						if(strlen(szServCat)==0)
							strcpy(szServCat, servCat);
						m_dealRcdNum.set(szServCat, 0, 0, 0, 0, 0, 1);
						m_dealRcdNum.setFee(0, 0, 0, 0, 0, (pps->m_inRcd).Get_Fee());
						m_dealRcdNum.setTime(0, 0, 0, 0, 0, (pps->m_inRcd).Get_Time());
						pkg.setRcdCode(k,res->getRuleType());
					}
					/*
					else
					{
						// �ּ����
						if( !strcmp(Param.szIsFmtFirst, "Y") )
						{
							fclose(fpFmtTimeOut);
							fclose(fpFmtOther);
							if( !strcmp(Param.szFmtErr2File, "Y") )
								Param.fmt_err2file.Close();
						}
						//return -1;
						throw jsexcp::CException(errno, "�ּ���������", __FILE__, __LINE__);
					}*/
				}
				else if( Result == eAbnormal )  /* �쳣�澯 */
				{
				//	cout<<"abnormal: "<<res->getRuleType()<<"\t"<<res->getReason()<<endl;
				//	Param.abnormal.saveErrLackRec(pps->m_inRcd, szSourceId, OutRealFileName, res->getRuleType(), res->getReason());
					/* ��¼������ */
					memset(szServCat, 0, sizeof(szServCat));
					(pps->m_inRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, servCat);
					m_dealRcdNum.set(szServCat, 0, 0, 0, 0, 0, 1);
					m_dealRcdNum.setFee( 0, 0, 0, 0, 0, (pps->m_inRcd).Get_Fee());
					m_dealRcdNum.setTime(0, 0, 0, 0, 0, (pps->m_inRcd).Get_Time());
					res->resetAnaResult();
					Result = eNormal;
				}
				else
				{
					break;
				}
				if (isEnablePerformanceAnalyzer) 
					{
						gettimeofday(&ana1_finish_time, NULL);
						other_process_counter += (ana1_finish_time.tv_sec
								- ana1_start_time.tv_sec) * (long) 1000
								* (long) 1000 + (ana1_finish_time.tv_usec
								- ana1_start_time.tv_usec);
					}
			}//end of while
			} catch (jsexcp::CException &e) {
			     theJSLog << "process record error !" <<endw;
			    pkg.setStatus(3);
			    return 0;
				e.PushStack(e.GetAppError(), "process record error", __FILE__, __LINE__);
				throw e;
			} catch (const std::exception &e) {
			theJSLog << "plugin analyse error !" <<endw;
			    pkg.setStatus(3);
			    return 0;
				throw jsexcp::CException(0, e.what(), __FILE__, __LINE__);
			} catch (...) {
			theJSLog << "unknown record error !" <<endw;
			    pkg.setStatus(3);
			    return 0;
				throw jsexcp::CException(0, "unknown error", __FILE__, __LINE__);
			}

			try {
				switch (Result)
				{
				//theJSLog << "deal Result" <<endd;
				case eNormal:  /* ����  */
					if(Param.bOutputFile)
					{
						(res->m_outRcd).Get_record(tmpRec, RECLEN+1);//(res->m_outRcd).Get_record(tmpRec.record, RECLEN+1);
						//_outfile.writeRec(res->m_outRcd);
						//CFmt_Change cft_tmp ;
						//cft_tmp = res->m_outRcd;
						//cft_tmp.Get_record
						//cout << "before set pkg ��¼ֵ" << pkg.getPkgRcd(k) << endl;
						//cout << "before set pkg ��¼��" << pkg.getRecordNum() << endl;
						pkg.setPkgRcd(k,(res->m_outRcd).Get_record());
						//cout << "after set pkg ��¼ֵ" << pkg.getPkgRcd(k) << endl;
						pkgRecord = pkg.getPkgRcd(k);
						pkg.setRcdType(k,"R");
					}

					/* ��¼������ */

					memset(szServCat, 0, sizeof(szServCat));
					(res->m_outRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, servCat);
					//cout << "insert szServCat = " << szServCat <<endl;
					m_dealRcdNum.set(szServCat, 1, 1, 0, 0, 0, 0);
					m_dealRcdNum.setFee((res->m_outRcd).Get_Fee(), (res->m_outRcd).Get_Fee(), 0, 0, 0, 0);
					m_dealRcdNum.setTime((res->m_outRcd).Get_Time(), (res->m_outRcd).Get_Time(), 0, 0, 0, 0);
					pkg.setStatus(2); 
					break;
				case eClassifiable:  /*����ּ𻰵�*/
					/* ��¼������ */
					memset(szServCat, 0, sizeof(szServCat));
					(res->m_outRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, servCat);
					m_dealRcdNum.set(szServCat, 1, 0, 0, 1, 0, 0);
					m_dealRcdNum.setFee((res->m_outRcd).Get_Fee(), 0, 0, 0, (res->m_outRcd).Get_Fee(), (res->m_outRcd).Get_Fee());
					m_dealRcdNum.setTime((res->m_outRcd).Get_Time(), 0, 0, 0, (res->m_outRcd).Get_Time(), (res->m_outRcd).Get_Time());
                    pkg.setPkgRcd(k,(res->m_outRcd).Get_record());
                    pkg.setRcdType(k,"E"); //����ּ�ͬʱ�ŵ����ݷּ���
                    theJSLog << "���󻰵�" <<endw;
                    
                    pkg.setStatus(3);
					break;
					//���¶��������
				case eRepeat:  /* �ص� */
					Param.classify.dealRecord(res->getRuleType(), pps->m_inRcd);
					/* ��¼������ */
					memset(szServCat, 0, sizeof(szServCat));
					(pps->m_inRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, servCat);
					m_dealRcdNum.set(szServCat, 1, 0, 0, 1, 0, 0);
					m_dealRcdNum.setFee((pps->m_inRcd).Get_Fee(), 0, 0, (pps->m_inRcd).Get_Fee(), 0, 0);
					m_dealRcdNum.setTime((pps->m_inRcd).Get_Time(), 0, 0, (pps->m_inRcd).Get_Time(), 0, 0);
					pkg.setPkgRcd(k,(res->m_outRcd).Get_record());
					pkg.setRcdType(k,"E"); //�ص����� error
					theJSLog << "���󻰵�" << endw;
					pkg.setStatus(3);
					break;
				case eFmtErr:  /* ��ʽ������ */
					iFmtErrorNum++;
					/* �������ļ� */
					/*
					if( !strcmp(Param.szFmtErr2Table, "Y") )
						Param.fmt_err2table.saveErrLackRec(pps->m_inRcd, szSourceId, OutRealFileName, res->getRuleType(), res->getReason());
					if( !strcmp(Param.szFmtErr2File, "Y") )
					{
						sprintf(szLongStr, "%d%c%s%c%s%c%s", pps->getOffset(), pps->m_inRcd.Get_FieldSep(1), res->getRuleType(), pps->m_inRcd.Get_FieldSep(1), res->getReason(), pps->m_inRcd.Get_FieldSep(1), pps->m_inRcd.Get_record());
						Param.fmt_err2file.writeRec(szLongStr);
					}*/
					/* ��¼������ */
					memset(szServCat, 0, sizeof(szServCat));
					(pps->m_inRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, servCat);
					m_dealRcdNum.set(szServCat, 1, 0, 0, 1, 0, 0);
					m_dealRcdNum.setFee((pps->m_inRcd).Get_Fee(), 0, 0, (pps->m_inRcd).Get_Fee(), 0, 0);
					m_dealRcdNum.setTime((pps->m_inRcd).Get_Time(), 0, 0, (pps->m_inRcd).Get_Time(), 0, 0);
					pkg.setPkgRcd(k,(res->m_outRcd).Get_record());
					pkg.setStatus(3);
					break;
				case eFmtTimeOut:  /* ��ʽ����ʱ��  */
					iFmtTimeOutNum++;
					/* ���ļ� */
				//	fprintf(fpFmtTimeOut, "%s\n", (pps->m_inRcd).Get_record());
					/* ��¼������ */
					memset(szServCat, 0, sizeof(szServCat));
					(pps->m_inRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, servCat);
					m_dealRcdNum.set(szServCat, 1, 0, 0, 1, 0, 0);
					m_dealRcdNum.setFee((pps->m_inRcd).Get_Fee(), 0, 0, (pps->m_inRcd).Get_Fee(), 0, 0);
					m_dealRcdNum.setTime((pps->m_inRcd).Get_Time(), 0, 0, (pps->m_inRcd).Get_Time(), 0, 0);
					pkg.setStatus(3);
					break;
				case eFmtOther:  /* ��ʽ��δ�����ʽ���� */
					iFmtOtherNum++;
					/* ���ļ� */
					//fprintf(fpFmtOther, "%s\n", pps->getString());
					/* ��¼������ */
					memset(szServCat, 0, sizeof(szServCat));
					strcpy(szServCat, servCat);
					m_dealRcdNum.set(szServCat, 1, 0, 0, 1, 0, 0);
					m_dealRcdNum.setFee(0, 0, 0, 0, 0, 0);
					m_dealRcdNum.setTime(0, 0, 0, 0, 0, 0);
					pkg.setStatus(3);
					break;
				case eAbnormal:  /* �쳣�澯 */
					//Param.abnormal.saveErrLackRec(pps->m_inRcd, szSourceId, OutRealFileName, res->getRuleType(), res->getReason());
					/* ��¼������ */
					memset(szServCat, 0, sizeof(szServCat));
					(pps->m_inRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, servCat);
					m_dealRcdNum.set(szServCat, 1, 1, 0, 0, 0, 1);
					m_dealRcdNum.setFee((pps->m_inRcd).Get_Fee(), (pps->m_inRcd).Get_Fee(), 0, 0, 0, (pps->m_inRcd).Get_Fee());
					m_dealRcdNum.setTime((pps->m_inRcd).Get_Time(), (pps->m_inRcd).Get_Time(), 0, 0, 0, (pps->m_inRcd).Get_Time());
					pkg.setPkgRcd(k,(res->m_outRcd).Get_record());
                    pkg.setRcdType(k,"E");  //������lackinfo
                    theJSLog << "�����ϻ���" << (res->m_outRcd).Get_record()<<endw;
                    pkg.setStatus(3);
					break;
				default:  /* �����ϻ��쳣 */
					//Param.lack_info.saveErrLackRec(pps->m_inRcd, szSourceId, OutRealFileName, res->getRuleType(), res->getReason());
					/* ��¼������ */
					memset(szServCat, 0, sizeof(szServCat));
					(pps->m_inRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, servCat);
					m_dealRcdNum.set(szServCat, 1, 0, 1, 0, 0, 0);
					m_dealRcdNum.setFee((pps->m_inRcd).Get_Fee(), 0, (pps->m_inRcd).Get_Fee(), 0, 0, 0);
					m_dealRcdNum.setTime((pps->m_inRcd).Get_Time(), 0, (pps->m_inRcd).Get_Time(), 0, 0, 0);
					pkg.setPkgRcd(k,(res->m_outRcd).Get_record());
					pkg.setRcdType(k,"E"); //������lackinfo
					theJSLog << "�����ϻ���" << (res->m_outRcd).Get_record()<<endw;
					//cout << "res->getRuleType() = " << res->getRuleType()<<endl;
					pkg.setStatus(3);
					break;
				}

				//Param.ProcMonitor++;
				// �Ѵ�����Y�����ڴ�����T����������H��δ������N
                pkg.setRcdStatus(k,"Y");
				//cout << "����״ֵ̬ = " << pkg.getRcdStatus(k) << endl;
				//cout << "������¼����= " << pkg.getRcdType(k) << endl;
				
				chain->reset();
			} catch (jsexcp::CException &e) {
				e.PushStack(e.GetAppError(), "analyze record result error",
						__FILE__, __LINE__);
				theJSLog << "analyze record result error !" <<endw;
			    pkg.setStatus(3);
			    return 0;
				throw e;
			} catch (const std::exception &e) {
			    theJSLog << "analyze record result error !" <<endw;
			    pkg.setStatus(3);
			    return 0;
				throw jsexcp::CException(1331609, e.what(), __FILE__, __LINE__);
			} catch (...) {
			    theJSLog << "unknown error !" <<endw;
			    pkg.setStatus(3);
			    return 0;
				throw jsexcp::CException(1331609, "unknown error", __FILE__, __LINE__);
			}
		}

		// Print performance, ����������������־����process_env�а�PLUGIN_ENABLE_PERFORMANCE_ANA��Ϊfalse
		filterchain::util::PerformanceAnalyzer* analyzer =
				Param.pluginInitializer.getPerformanceAnalyzer();
		if (analyzer) {
			gettimeofday(&file_finish_time, NULL);
			unsigned long file_time = (file_finish_time.tv_sec
					- file_start_time.tv_sec) * (long) 1000 * (long) 1000
					+ (file_finish_time.tv_usec - file_start_time.tv_usec);
			theJSLog << "process current file with in " << file_time
					<< "us, plugin:" << plugin_process_counter << " other:"
					<< other_process_counter << endi;
			std::map<std::string,filterchain::util::PerformanceAnalyzer::ActionPerformance*>
					actionPerformance = analyzer->getActionPerformances();

			for (std::map<std::string,
					filterchain::util::PerformanceAnalyzer::ActionPerformance*>::iterator
					it = actionPerformance.begin(); it
					!= actionPerformance.end(); it++) {
				filterchain::util::PerformanceAnalyzer::ActionPerformance* ap =
						it->second;
				theJSLog << "action:" << ap->getActionId() << " time:"
						<< ap->getTimeMillisCounter() << " run:"
						<< ap->getRunningCounter() << " reset:"
						<< ap->getResetCounter() << " signal:"
						<< ap->getSignalCounter() << endi;
			}
		//}/////
		}
	}
	catch(jsexcp::CException &e)
	{
		sprintf(szLogStr, "�����ļ�(%s)��%d��ʧ��", &szFileName, iTotalNum);
		e.PushStack(e.GetAppError(), szLogStr, __FILE__, __LINE__);
		throw e;
	}
	catch(const std::exception &e){
		sprintf(szLogStr, "%s\n�����ļ�(%s)��%d��ʧ��", e.what(), &szFileName, iTotalNum);
		throw jsexcp::CException(1331609, szLogStr, __FILE__, __LINE__);
	}

      // ƽ�⻰�������
     long _total = 0;
     _total = m_dealRcdNum.getTotal();  //�ܻ�����
     long _right = 0;
     _right = m_dealRcdNum.getRight();  //��ȷ������
     long _lack  = 0;
     _lack = m_dealRcdNum.getLack();  //�����ϵ�
     long _error = 0;
     _error = m_dealRcdNum.getError();  //��Ϊ�ּ�����Ļ���    

     
     theJSLog <<"�������"<< "  total = "<<_total<<"  _right = "<<_right<<"  _lack= "<<_lack<<"  _error = "<<_error<<endi;
     if(_total != _right)
        {
           pkg.setStatus(3);
        }   
      m_dealRcdNum.check();
      
      // sql ������
      //cout << " ��Ҫ�������m_dealRcdNum.outSqlStr = " << m_dealRcdNum.getSqlstr() <<endl;
      m_dealRcdNum.updateTable(Param.szLogTabname,szFileName,m_szBeginTime, szSourceId,Param.iProcessId);
      
      
      //theJSLog << " ��Ҫ�������m_dealRcdNum.outSqlStr = " << m_dealRcdNum.getSqlstr() <<endd;
      PS_Process::writeSQL(m_dealRcdNum.getSqlstr());
      if( strlen(m_dealRcdNum.getSqlstr()) == 0)
      	PS_Process::rollBackSQL();
      PS_Process::commitSQLFile();
      theJSLog<<"*********************************************************"<<endi;
       //pkg.setStatus(2); 	
     //PS_Process::writeSQL(m_dealRcdNum.getSqlstr()) ;
     
    // long param1,param2;
    // param1 = _total*100000 + _right;
     //param2 = _lack*100000 + _error;
     //cout << "param1 = " <<param1 << "param2 " << param2<< endl;

     // ���������ŵ���Ϣ������        
     //cout << "child_proc= "<< child_proc << "tmp= "<< tmp << endl;
     //cout << "ontask m_lPrc_ID = " << m_lPrc_ID<< endl;
  //  if( !mputChldEvt(0, EVT_REQ_TRNUM, _total*100000 + _right, _lack*100000 + _error, m_lPrc_ID-child_proc-1) )
	//{
	//	cerr<<"������"<<m_lPrc_ID<<"��������ʧ��!"<<endl;
	//	return false;
	//}     

     //sleep(1);   

      //20131127 lij add
      getCurTime(szCurrentTime);
	 
	/* theJSLog<<"������Ϣ���ļ�������"<<endi;
	 message.setMessage(MESSAGE_END_FILE, szSourceId, szFileName, 0);
	 argMsg.set(message);
	 iRad++;
		if(iRad > 99999)
			iRad = 1;
			argMsg.setTime(szCurrentTime, iRad);
			if(chain == NULL)
			{
				cout<<"chain is null"<<endl;
			}
			//chain->signal(pMsg);

		getCurTime(szCurrentTime);
		theJSLog<<"������Ϣ�����ν�����"<<endi;
		message.setMessage(MESSAGE_END_BATCH_END_FILES, szSourceId, szFileName, 0);
		argMsg.set(message);
		iRad++;
		if(iRad > 99999)
			iRad = 1;
		argMsg.setTime(szCurrentTime, iRad);
		chain->signal(pMsg);*/

		/* �����������ν������ύ���ݿ����Ϣ */
		getCurTime(szCurrentTime);
		theJSLog<<"������Ϣ�����ν������ύ���ݿ⣡"<<endi;
		/*message.setMessage(MESSAGE_END_BATCH_END_DATA, szSourceId, szFileName, 0);
		argMsg.set(message);
		iRad++;
		if(iRad > 99999)
			iRad = 1;
		argMsg.setTime(szCurrentTime, iRad);
		chain->signal(pMsg);*/
     //cout << "onTask�ӽ��̽���" << endl;
	 //DBConn.Disconnect();
    /* theJSLog<<"ticket_num = "<<ticket_num<<endi;
     for(int j = 1;j<=ticket_num;j++)
       {
		  PkgFmt fmt = pkg.readPkgRecord(j);		  
          pkgRecord = pkg.getPkgRcd(j);  //  ��ȡָ���м�¼ֵ 
          theJSLog<<fmt.record<<endi;
	 	}*/
	 	
     return ticket_num;
  }

/*
void C_MainFlow::onChildExit()
  {
     //cout << "����������exit �Ƿ��˳�1" << endl;
     delete pps;
     delete res;
     delete pListSql;
     chain = NULL;
     //int i = DBConn.IsConnected();
      exit(-1) ;
     //cout << "����������exit �Ƿ��˳�2" << endl;
    // exit(-1) ;
  }


 //���ӽ���ͨ����Ϣ�����﷢����Ϣ�����ӽ��̾��ɵ���
bool C_MainFlow::mputChldEvt(int event_sn, int event_type, long param1, long param2, long dest_id)
{
    int chldprc_evtchnl_key;
    CString sKeyVal;
	if( param_cfg.bGetMem("memory.MT_DSPCH_CHILD.msgq_key_value", sKeyVal) && sKeyVal.isNumber() )
	{
		chldprc_evtchnl_key=sKeyVal.toInteger();
	}
	else
	{
		cerr<<"δ���ø��ӽ��̼���Ϣͨ����ֵ\n";
		return false;
	}
	int _iChldPrcMsgID=msgget(chldprc_evtchnl_key, 0600);
	//cout << "put _iChldPrcMsgID = "<<_iChldPrcMsgID<<endl;
	
	if(_iChldPrcMsgID==-1) return false;
	EVENT_MSG event_msg;
	event_msg.msgtype=dest_id+90;
	event_msg.event.event_sn=event_sn;
	event_msg.event.event_type=event_type;
	event_msg.event.param1=param1;
	event_msg.event.param2=param2;
	event_msg.event.src_id=PS_BillProcess::m_lPrc_ID;
	time(&(event_msg.event.event_time));
	if( msgsnd(_iChldPrcMsgID, &event_msg, sizeof(EVENT), IPC_NOWAIT)<0 ) return false;
	return true;
}


 //���ӽ���ͨ����Ϣ��������ȡ��Ϣ�����ӽ��̾��ɵ���
int C_MainFlow::mgetChldEvt(int &event_sn, int &event_type, long &param1, long &param2, long &src_id, bool sync)
{
    int chldprc_evtchnl_key;
    CString sKeyVal;
    //cout << "mgetChldEvt m_lPrc_ID= " << m_lPrc_ID<< endl;
	if( param_cfg.bGetMem("memory.MT_DSPCH_CHILD.msgq_key_value", sKeyVal) && sKeyVal.isNumber() )
	{
		chldprc_evtchnl_key=sKeyVal.toInteger();
	}
	else
	{
		cerr<<"δ���ø��ӽ��̼���Ϣͨ����ֵ\n";
		return false;
	}
	int _iChldPrcMsgID=msgget(chldprc_evtchnl_key, 0600);

    //cout << "get _iChldPrcMsgID = "<<_iChldPrcMsgID<<endl;
	if(_iChldPrcMsgID==-1) return -1;
	EVENT_MSG event_msg;
	while(true)
	{
		if( msgrcv(_iChldPrcMsgID, &event_msg, sizeof(EVENT), m_lPrc_ID+90, sync?0:IPC_NOWAIT)<0 )
		{
			if(errno==ENOMSG) return 0;
			else if(errno==EINTR)
			{   
				if( getpid()==_iMainPid ) return 0;
				else continue;
			}
			else return -1;
		}
		else break;
	}
	event_sn=event_msg.event.event_sn;
	event_type=event_msg.event.event_type;
	param1=event_msg.event.param1;
	param2=event_msg.event.param2;
	src_id=event_msg.event.src_id;
	return 1;
}
*/

//��ʼ������ƽ̨��������״̬
/*MdrStatus C_MainFlow::syncInit()
{
    MdrRetCode rc_;
    rc_ = mdr_InitPlatform();  //��ʼ������ƽ̨
    printf( "%s,%d-->mdr_InitPlatform, rc_=%d, (rc=>0:SUCCESS,1:FAILURE,-1:INVALID)\n", __FILE__, __LINE__, rc_ );
    //writelog( 0, "mdr_InitPlatform" );

    if( rc_ != MDR_SUCCESS ) {
        //printf( "%s,%d-->FATAL! mdr_InitPlatform failed\n", __FILE__, __LINE__ );
        //writelog( 0, "��ʼ������ƽ̨����!" );
        cerr<<"��ʼ������ƽ̨����!"<<endi;
    }

    MdrStatus stat_;
    rc_ = mdr_GetDRStatus( stat_ );
    printf( "%s,%d-->mdr_GetDRStatus, rc_=%d, stat_=%d, (stat=>0:MASTER,1:SLAVE,2:NODR)\n", __FILE__, __LINE__, rc_, stat_ );

    if( rc_ != MDR_SUCCESS ) {
        printf( "%s,%d-->FATAL! mdr_GetDRStatus failed\n", __FILE__, __LINE__ );
        writelog( 0, "��ȡ����ƽ̨״̬ʧ��!" );
        cerr<<"��ȡ����ƽ̨״̬ʧ��!"<<endi;
    }

    MdrNodeType node_type_;
    rc_ = mdr_GetNodeType( node_type_ );
    printf( "%s,%d-->mdr_GetNodeType, rc_=%d, node_type_=%d (node_type=>0:DUPLEX,1:SINGLE)\n", __FILE__, __LINE__, rc_, node_type_ );

    if( rc_ != MDR_SUCCESS ) {
        //printf( "%s,%d-->FATAL! mdr_GetNodeType failed\n", __FILE__, __LINE__ );
        writelog( 0, "��ʼ������ƽ̨����!" );
        cerr<<"��ʼ������ƽ̨����!"<<endi;
    }

    rc_ = mdr_GetNodeTypeSR( node_type_ );
    printf( "%s,%d-->mdr_GetNodeTypeSR, rc_=%d, sr_node_type_=%d\n", __FILE__, __LINE__, rc_, node_type_ );

    if( rc_ != MDR_SUCCESS ) {
        //printf( "%s,%d-->FATAL! mdr_GetNodeTypeSR failed\n", __FILE__, __LINE__ );
        //writelog( 0, "��ʼ������ƽ̨����!" );
        cerr<<"��ʼ������ƽ̨����!"<<endi;
    }

    return stat_;
}

//�����ϵͳ����
void C_MainFlow::fillMasterAuditInfo( MdrAuditInfo& audit_info, char* value  )
{
    int pid_ = getpid();
    char buf_[1024];
    audit_info.node = "TPSS1";
    audit_info.srvContext = "srvContext";

    struct timeval tv_;
    ::gettimeofday( &tv_, NULL );
    tv_.tv_sec --;
    struct tm tm_;
    ::localtime_r( &tv_.tv_sec, &tm_ );
    char tmp_[64];
    strftime( tmp_, sizeof( tmp_ ), "%Y%m%d%H%M%S", &tm_ );
    sprintf( buf_, "%s.%06d", tmp_, ( int )( tv_.tv_usec % 1000000 ) );
    audit_info.ccrRcvTime = buf_;

    sprintf( buf_, "auditKey_%d_%s", pid_, audit_info.ccrRcvTime.c_str() );
    audit_info.auditKey = buf_;
    sprintf( buf_, "sessionId_%d_%s", pid_, audit_info.ccrRcvTime.c_str() );
    audit_info.sessionId = buf_;

    audit_info.rflag = 2;			// must fill 2

    audit_info.syncVar = string( value );
    audit_info.auditVal = audit_info.syncVar;
    tv_.tv_sec --;
    ::localtime_r( &tv_.tv_sec, &tm_ );
    strftime( tmp_, sizeof( tmp_ ), "%Y%m%d%H%M%S", &tm_ );
    sprintf( buf_, "%s.%06d", tmp_, ( int )( tv_.tv_usec % 1000000 ) );
    audit_info.ccrEvtTime = buf_;
    audit_info.result = 0;    
    //cout << "auditvalue = " << audit_info.syncVar << endl;
    //cout << "auditkey = " << auditkey << endl;
    //cout << "sessionid = " << sessionid << endl;
}

bool C_MainFlow::masterAudit( MdrAuditInfo& audit_info,char* value )
{
    fillMasterAuditInfo( audit_info, value );
    MdrRetCode rc_ = mdr_Audit( audit_info );
    std::string audit_info_str_;
    //audit_info.toStr( audit_info_str_ );
    //audit_info_str_ = audit_info.toStr();
    cout << "auditKey " << audit_info.auditKey << endl;
    cout << "auditVal " << audit_info.auditVal << endl;
    cout << "sessionId " << audit_info.sessionId << endl;
    cout << "rflag " << audit_info.rflag << endl;
    
    printf( "%s,%d-->mdr_Audit, rc_=%d, %s\n", __FILE__, __LINE__, rc_, audit_info_str_.data() );
    //writelog( 0, "��ϵͳ�ٲ�����Ϊ " + audit_info_str_ );

    if( rc_ != MDR_SUCCESS ) {
        printf( "%s,%d-->FATAL! mdr_Audit failed\n", __FILE__, __LINE__ );
        //writelog( 0, "��ϵͳ�ٲ�ʧ�� " );
        return false;
    }

    return true;
}

bool C_MainFlow::slaveAudit( MdrAuditInfo& audit_info ,char *allval)
{
    audit_info.node = "TPSS2";
    audit_info.srvContext = "srvContext";
    audit_info.rflag = 2;				// must fill 2
    audit_info.auditKey = auditkey;
    audit_info.sessionId = sessionid;
    audit_info.auditVal = allval;
    audit_info.ccrEvtTime = "20000101000000";		// SLAVE side: dummy value to pass fmt validation
    audit_info.ccrRcvTime = "20000101000000.000000";	// SLAVE side: dummy value to pass fmt validation

    MdrRetCode rc_ = mdr_Audit( audit_info );
    cout << "auditKey " << audit_info.auditKey << endl;
    cout << "auditVal " << audit_info.auditVal << endl;
    cout << "sessionId " << audit_info.sessionId << endl;
    cout << "rflag " << audit_info.rflag << endl;
    
    //printf( "%s,%d-->mdr_Audit, rc_=%d, %s\n", __FILE__, __LINE__, rc_, audit_info.data() );
    printf( "%s,%d-->mdr_Audit, rc_=%d\n", __FILE__, __LINE__, rc_ );
   // writelog( 0, "��ϵͳ�ٲ�����Ϊ" + audit_info.toStr() );

    if( rc_ != MDR_SUCCESS ) {
        printf( "%s,%d-->FATAL! mdr_Audit failed\n", __FILE__, __LINE__ );
        writelog( 0, "��ϵͳ�ٲ�ʧ�� " );
        return false ;
    }

    return true ;
}

void C_MainFlow::cmtResult( const MdrAuditInfo& audit_info )
{
    MdrRetCode rc_ = mdr_CmtResult( audit_info );
    std::string audit_info_str_;
    //audit_info.toStr( audit_info_str_ );
    printf( "%s,%d-->mdr_CmtResult, rc_=%d, %s\n", __FILE__, __LINE__, rc_, audit_info_str_.data() );
    writelog( 0, "mdr_CmtResult ���Ϊ" + audit_info_str_ );

    if( rc_ != MDR_SUCCESS ) {
        //printf( "%s,%d-->FATAL! mdr_CmtResult failed\n", __FILE__, __LINE__ );
        writelog( 0, "����ʧ��" );
    }
}

bool C_MainFlow::runmdr( char* value )
{
    if( syn_status == 1 ) {
        std::vector<MdrVarInfo> var_list_;
        var_list_.clear();
        return true;
    } else {
        MdrAuditInfo audit_info_;
        cout << "������ϵͳ" << endl;
        if( masterAudit( audit_info_, value ) ) {
            cmtResult( audit_info_ );
            return true;
        } else {
            return false;
        }

    }
}

bool C_MainFlow::dealSyntable( MdrVarInfo &var_info_, std::string &Sql )
{
    try {
        dcc_stmt.setSQLString( Sql );
        std::cout << Sql << std::endl;

        if( dcc_stmt.execute() ) {
            //std::cout << "������ϵͳCRM_ORDER_ITEM ��" << std::endl;
            writelog( 0, "������ϵͳCRM_ORDER_ITEM ��" );

            MdrAuditInfo audit_info_;

            if( slaveAudit( var_info_, audit_info_ ) ) {
                cmtResult( audit_info_ );
            }
            dcc_stmt.commit();
        }
    } catch( SQLException e ) {
        MdrAuditInfo audit_info_;
        var_info_.syncVar += "_F";

        if( slaveAudit( var_info_, audit_info_ ) ) {
            cmtResult( audit_info_ );
        }

        return false;
    }

    return true;
}*/


void dealSignal(int sig)
{
	if(sig == SIGTERM)
	{
		theJSLog<<"�յ���ֹ������źţ�"<<sig<<endi;
		bGetExitSig = true;
	}
	else
		bGetExitSig = false;
}

bool checkAnotherDay(char *startTime)
{
	char szNow[DATETIME_LEN+1];
	getCurTime(szNow);
	if( strncmp(szNow, startTime, 8) != 0 )
	{
		strcpy(startTime, szNow);
		return true;
	}
	else
		return false;
}

//�ӽ����˳�ǰ�Ĵ���
void C_MainFlow::onChildExit()
{
    cout<<"ģ��3�ӽ����˳�"<<endl;
}

//�����ӽ����������������̵Ĵ���
int C_MainFlow::onTaskOver(int child_ret)
{
   cout<<"ģ��3�ӽ����Ѵ���������,���ӽ�������ļ�¼��"<<child_ret<<endl;
   return  child_ret ;

}

//�����̿�ʼ���仰��ǰ�Ĵ���
int C_MainFlow::onTaskBegin(void *task_addr)
{
      return 1;
}

//�ӽ��̳�ʼ��
bool C_MainFlow::onChildInit()
{
   //cout<<"ģ��3�ӽ��̳�ʼ��"<<endl;
   return true;
   
}