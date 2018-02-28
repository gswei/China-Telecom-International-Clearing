/****************************************************************
filename: mainflow.cpp
module:
created by: ouyh
create date: 
version: 3.0.0
description:
    �������ƽ̨
*****************************************************************/
#include "MainFlow.h"

using namespace filterchain;
using namespace std;
using namespace tpss;

//CDatabase _DBConn; //�ӽ���
//CDatabase DBConn; //������
CLog theLog;

bool bGetExitSig;

int main(int argc, char **argv)
{
	cout<<"begin main"<<endl;
//	C_MainFlow process;
//	cout<<"begin main 1"<<endl;
//	process.printVersion();
/*
	if(!process.checkArg(argc, argv))
	{
		return -1;
	}

	try
	{
		//signal(SIGTERM, dealSignal);
		
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
		theLog<<"��ȫ�˳�����"<<endi;
		// �Ͽ����ݿ����� // 
		process.exit();
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
	*/
	return 0;
};

C_MainFlow::C_MainFlow()
{
	vecFile.clear();
	memManager = NULL;
	chain = NULL;
	pps = NULL;
	res = NULL;
	pListSql = NULL;
	iRunTime = -1;
	conn = NULL;
	//m_bDBLinkError = FALSE;
	//_DBConn = NULL;
}

C_MainFlow::~C_MainFlow()
{
	vecFile.clear();
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
}

void C_MainFlow::printVersion()
{
	/* ���ģ������������ơ��汾����Ϣ */
	printf("*************************************************** \n");
	printf("*   	GuangDong Telecom. %s System	\n" , SYSTEM_NAME );
	printf("*                                                   \n");
	printf("*		%s \n", MODULE_NAME);
	printf("*		  Version %s	\n", VERSION_NO);
	printf("*      last updated: %s by %s	\n", UPDATE_TIME, WHO_UPDATE );
	printf("*************************************************** \n");
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
			//���˹������ü���
			string sql = "select ext_param from tp_process where process_id = :v1";		
			stmt.setSQLString(sql);
			stmt << procID;
			if (stmt.execute())
			{
                stmt >> ext_param;
			}

			sql = "select source_group from tp_billing_line where billing_line_id = :v1";
			stmt.setSQLString(sql);
			stmt << billing_line_id;
			if (stmt.execute())
			{
                stmt >> source_group;
			}
	 }else{
	 	cout<<"connect error."<<endl;
	 }
	 conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
     } 	 
   // conn.commit();
    
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
	//theLog<<"szProgramPath="<<m_szExePath<<endi;
	//theLog<<"szProgramName="<<m_szExeName<<endil;
	return true;
};

bool C_MainFlow::init(int argc, char **argv)
{
    // ��ʼ�������̿��
    tmp_childnum = 0;
    if(!PS_BillProcess::init(argc,argv))
   {
      return false;
   }
    _iMainPid = getpid();

	iUndoFlag = 0;
	iRad = 1;
	iRunTime = 0;
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
	char szLogPath[] = {"/mboss/home/zhjs/log"};
	char szLogLevel[] = {"2"};
	theLog.setLog(szLogPath, atoi(szLogLevel), Param.szServiceId, Param.szSourceGroupId, Param.iProcessId);
	
	//����Ϊ��̨��פ����
	//memset(szTemp, 0, sizeof(szTemp));
    //szTemp = "Y"; // д��ΪY
	//if(!strcmp(szTemp, "Y"))
	 // initDaemon(true);
	theLog.reSetLog();

	// ��Ϣ����־
	Param.info.SetEnvPath(m_szEnvPath);
	Param.info.configInfoLog(Param.szServiceId, Param.szSourceGroupId, Param.iProcessId, m_szExeName, m_szExePath);
	Param.info.msgInfoLog(INFO_START);
	//��ӡ��������
	theLog<<"�������"<<m_szExeName<<"  "<<Param.szServiceId<<"  "<<Param.szSourceGroupId<<"  "<<Param.iProcessId<<endi;
	try
	{	
	  {
		//CBindSQL ds(DBConn);	
		/* ȡ���йؼ��� */
				
		// �������̵Ĺ����ڴ��������� 
		
		/*memset(szIPCKeyPath, 0, sizeof(szIPCKeyPath));
		if( getEnvFromDB( _DBConn, Param.szServiceId, Param.szSourceGroupId, "", "IPCKEY_PATH", szIPCKeyPath ) == -1 )
		{
			sprintf(szLogStr, "������Ϣ�����ڴ�IPCKEYĿ¼[IPCKEY_PATH]δ����!");
			theLog<<szLogStr<<endi;
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

		/***********************************************************************************

		*������Ŀ¼

		***********************************************************************************/
		CheckWorkpath();

		/***********************************************************************************

		*��ȡ���������е����빲���ڴ�ĵ�ַ

		***********************************************************************************/
	/*	if(Param.bCommemFlag)
		{
		    //CBindSQL ds(DBConn);
			memManager = new C_AccessMem();
			memManager->Init(Param.szServerId,m_szEnvPath);
		}*/

		/***********************************************************************************

		*��ʼ����������ࡢ�ּ��ࡢ�������ࡢ������־�ࡢ�����ͺ���ת����

		***********************************************************************************/
		abnormity_type lack = abenum_lackinfo;		//����������
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
		}
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
	return true;
}

void C_MainFlow::CheckWorkpath()
{
	theLog<<"�����ʱ����Ŀ¼�Ƿ����쳣״̬���ļ�......"<<endi;

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
			string sql = "select deal_flag from :v1 where source_id=':v2' and filename=':v3' and partid in(:v4,:v5,:v6)";		
			stmt.setSQLString(sql);
			stmt << Param.szLogTabname << (iter->second).szSourceId << szRealName <<getPrePartID(szTmpPart)<<atoi(szTmpPart)<<getNextPartID(szTmpPart) ;
			if (stmt.execute())
			{
                stmt >> cFlag;
			}
			
			/*sprintf(szSqlStr, "select deal_flag from %s where source_id='%s' and filename='%s' and partid in(%d,%d,%d)",
				Param.szLogTabname, (iter->second).szSourceId, szRealName, getPrePartID(szTmpPart), atoi(szTmpPart), getNextPartID(szTmpPart));
			ds.Open(szSqlStr, SELECT_QUERY);
			if( !(ds>>cFlag) )
			{
				ds.Close();
				sprintf(szLogStr, "�ڱ�%s���Ҳ���(����Դ%s,�ļ�%s)�ļ�¼", Param.szLogTabname, (iter->second).szSourceId,
					szRealName);
				//theLog<<szLogStr<<endw;
				//throw jsexcp::CException(0, szLogStr, __FILE__, __LINE__);
			}*/
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
				theLog<<"���ļ� "<<szPathName1<<" �ĳ� "<<szPathName2<<endi;
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
	 	cout<<"connect error."<<endl;
	 }
	 conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
     } 	 
   // conn.commit();
}

void C_MainFlow::Update()
{
	try
	{
		//theLog<<"���¹����ڴ桭��"<<endd;
		/* ���¹����ڴ� */
		if(Param.bCommemFlag)
			memManager->LoadOrUpdate();

	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			//���˹������ü���
			string sql = "select UPDATE_FLAG from i_list_sql order by SQL_ID";		
			stmt.setSQLString(sql);
			
		if (stmt.execute())
		{
              for( int i=0; i < iListCount; i++ )
		    {
			if( !(stmt>>pListSql[i].m_szUpdateFlag) )
			{
				throw jsexcp::CException(0, "ִ�� select UPDATE_FLAG from i_list_sql order by SQL_ID ʧ�ܣ�", __FILE__, __LINE__);
			}
		   }
		}
	 }else{
	 	cout<<"connect error."<<endl;
	 }
	 conn.close();
	 
		//�������������ʽlist��������update_flag��ֵ�������ж��Ƿ�����˱���¼
		/*ds.Open("select UPDATE_FLAG from i_list_sql order by SQL_ID", SELECT_QUERY );
		for( int i=0; i < iListCount; i++ )
		{
			if( !(ds>>pListSql[i].m_szUpdateFlag) )
			{
				ds.Close();
				//theLog<<"select UPDATE_FLAG from i_list_sql order by SQL_ID Error!"<<ende;
				throw jsexcp::CException(0, "ִ�� select UPDATE_FLAG from i_list_sql order by SQL_ID ʧ�ܣ�", __FILE__, __LINE__);
			}
			//pListSql++;
		}
		ds.Close();*/
		theLog<<"�������ϳɹ���"<<endd;
	}
	catch(jsexcp::CException &e)
	{
		conn.close();
		Param.info.msgInfoLog(INFO_BREAK);
		throw;
		
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

void C_MainFlow::checkTime()
{
	if(chain != NULL)
	{
		//theLog<<"����Ƿ����(check another day)"<<endi;
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

/*void C_MainFlow::process()
{
	CBindSQL ds(_DBConn[child_proc]);
	char szSqlStr[SQL_LEN+1];
	char szLogStr[LOG_MSG_LEN+1]; 
	char szTemp[PATH_NAME_LEN+1];
	char szCurrentTime[DATETIME_LEN+1];
	char szTmpPart[2+1];
	int iDBRet=0;

	MessageParser message;
	ArgMessage argMsg;
	Argument *pMsg = &argMsg;
	char szFullInPath[PATH_NAME_LEN+1];	  	//����·����ȫ·����
	char szFullOutPath[PATH_NAME_LEN+1];	//���·����ȫ·����
	char szRealFile[FILE_NAME_LEN+PATH_NAME_LEN+1];
	char szTmpFile[FILE_NAME_LEN+PATH_NAME_LEN+1];
	int iDealCount = 0, iBatchCount = 0;

	vector<SAlterRecordAfterDeal> vecAftDeal;

	if(iUndoFlag == 1 || m_bDBLinkError)
	{
		//���������쳣��Ϣ
		getCurTime(szCurrentTime);
		theLog<<"������Ϣ�������쳣��"<<endi;
		message.setMessage(MESSAGE_BREAK_BATCH, "", "", 0);
		argMsg.set(message);
		iRad++;
		if(iRad > 99999)
			iRad = 1;
		argMsg.setTime(szCurrentTime, iRad);
		chain->signal(pMsg);
		if(iUndoFlag == 1)
			iUndoFlag++;
		if(m_bDBLinkError)
			m_bDBLinkError = FALSE;
	}
PROC_ENTRY:
	int iFileCount = vecFile.size();
	STmpVariable sVar;
	for(int i =0; i<iFileCount; i++)
	{
		if(vecFile[i].iDealFlag != 0)
		{
			theLog<<"�ļ�"<<vecFile[i].szFileName<<"�Ѵ���"<<endi;
			continue;
		}
		//cout<<"�ļ�"<<vecFile[i].szFileName<<"δ����"<<endl;
		getCurTime(vecFile[i].szDealStartTime);
		// ������ʱ���� 
		strcpy(m_szSourceId, vecFile[i].szSourceId);
		strcpy(m_szLocalTollcode, vecFile[i].szTollcode);
		strcpy(m_szSysTime, vecFile[i].szDealStartTime);
		strcpy(m_szFileName, vecFile[i].szRealFileName);
		strcpy(m_szNewName, vecFile[i].szFileName);
		strcpy(m_szReceiveTime, vecFile[i].szReceiveTime);
		
		//Param.defaultVar.setDefVar("SOURCE_ID", vecFile[i].szSourceId);
		//Param.defaultVar.setDefVar("LOCALTOLL", vecFile[i].szTollcode);
		//Param.defaultVar.setDefVar("SYS_TIME", vecFile[i].szDealStartTime);
		//Param.defaultVar.setDefVar("FILE_NAME", vecFile[i].szRealFileName);
		//Param.defaultVar.setDefVar("NEW_NAME", vecFile[i].szFileName);
		
		strcpy(sVar.szVarName, "SOURCE_ID");
		strcpy(sVar.szVarValue, vecFile[i].szSourceId);
		Param.map_DefVar.push_back(sVar);
		strcpy(sVar.szVarName, "LOCALTOLL");
		strcpy(sVar.szVarValue, vecFile[i].szTollcode);
		Param.map_DefVar.push_back(sVar);
		strcpy(sVar.szVarName, "SYS_TIME");
		strcpy(sVar.szVarValue, vecFile[i].szDealStartTime);
		Param.map_DefVar.push_back(sVar);
		strcpy(sVar.szVarName, "FILE_NAME");
		strcpy(sVar.szVarValue, vecFile[i].szRealFileName);
		Param.map_DefVar.push_back(sVar);
		strcpy(sVar.szVarName, "NEW_NAME");
		strcpy(sVar.szVarValue, vecFile[i].szFileName);
		Param.map_DefVar.push_back(sVar);
		strcpy(sVar.szVarName, "RECEIVE_TIME");
		strcpy(sVar.szVarValue, vecFile[i].szReceiveTime);
		Param.map_DefVar.push_back(sVar);

		if(iDealCount == 0 || iBatchCount == 0)
		{
			// �����������ο�ʼ����Ϣ 
			getCurTime(szCurrentTime);
			theLog<<"������Ϣ�����ο�ʼ��"<<endi;
			message.setMessage(MESSAGE_NEW_BATCH, vecFile[i].szSourceId, vecFile[i].szRealFileName, vecFile[i].lFileId);
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
		}
		sprintf(szFullInPath, "%s%s", vecFile[i].szSourcePath, Param.szInPath);
		if(Param.bOutputFile)
			sprintf(szFullOutPath, "%s%s", vecFile[i].szSourcePath, Param.szOutPath);
		theLog<<"*************************************************************"<<endi;
		theLog<<"FileName="<<vecFile[i].szRealFileName<<endi;
		theLog<<"InPath="<<szFullInPath<<endi;
		if(Param.bOutputFile)
			theLog<<"OutPath="<<szFullOutPath<<endi;
		theLog<<"SourceId="<<vecFile[i].szSourceId<<endi;
		theLog<<"SourcePath="<<vecFile[i].szSourcePath<<endi;

		// ���������ļ���ʼ����Ϣ 
		getCurTime(szCurrentTime);
		theLog<<"������Ϣ���ļ���ʼ��"<<endi;
		message.setMessage(MESSAGE_NEW_FILE, vecFile[i].szSourceId, vecFile[i].szRealFileName, vecFile[i].lFileId);
		argMsg.set(message);
		iRad++;
		if(iRad > 99999)
			iRad = 1;
		argMsg.setTime(szCurrentTime, iRad);
		chain->signal(pMsg);

		//Param.ProcMonitor.UpdateMonitor(vecFile[i].szFileName, 0, 'Y', CProcInfo::SERV_BUSY);
		// ��ȡ��ʼ�����ļ���ʱ�� 
		//getCurTime(vecFile[i].szDealStartTime);
		memset(szTmpPart, 0, sizeof(szTmpPart));
		strncpy(szTmpPart, vecFile[i].szDealStartTime+4, 2);
		vecFile[i].iPartID = atoi(szTmpPart);

		int iRet=0;
		try {
			iRet = DealFile.dealfile(Param, vecFile[i], pps, res);
		} catch (jsexcp::CException &e) {
			sprintf(szLogStr, "�����ļ�(%s)����", vecFile[i].szFileName);
			e.PushStack(e.GetAppError(), szLogStr, __FILE__, __LINE__);
			throw e;
		} catch (const std::exception &e) {
			throw jsexcp::CException(1331609, e.what(), __FILE__, __LINE__);
		}

		try
		{
			switch (iRet)
			{
			case 0:
				//�����ɹ����ύ����
				Param.lack_info.Commit();
				Param.abnormal.Commit();
				if( !strcmp(Param.szIsFmtFirst, "Y") && !strcmp(Param.szFmtErr2Table, "Y") )
					Param.fmt_err2table.Commit();
				iRet = 0;
				//�ļ�����
				getCurTime(szCurrentTime);
				theLog<<"������Ϣ���ļ�������"<<endi;
				message.setMessage(MESSAGE_END_FILE, vecFile[i].szSourceId, vecFile[i].szRealFileName, vecFile[i].lFileId);
				vecAftDeal.clear();
				message.setAlterInfo(&vecAftDeal);
				argMsg.set(message);
				iRad++;
				if(iRad > 99999)
					iRad = 1;
				argMsg.setTime(szCurrentTime, iRad);
				chain->signal(pMsg);
				//Param.lack_info.Close();
				DealFile.endfile(vecAftDeal, Param.info, Param.bOutputFile);
				//����ɾ���򱸷ݱ�־,�������ļ����д���
				char InFileName[FILE_NAME_LEN+1];
				sprintf(InFileName, "%s%s", szFullInPath, vecFile[i].szFileName);
				if(!Param.bBakFlag)
					vecFile[i].iDealFlag = 1;
				iDealCount++;
				iBatchCount++;
				strcpy(m_szLastSource, vecFile[i].szSourceId);
				break;
			case -1: //����ʧ�ܣ��ع����񣬲����ļ�״̬��Ϊ��W��
				//����ʧ�ܣ��ع�����
				Param.lack_info.RollBack();
				Param.abnormal.RollBack();
				if( !strcmp(Param.szIsFmtFirst, "Y") && !strcmp(Param.szFmtErr2Table, "Y") )
					Param.fmt_err2table.RollBack();
				if(_DBConn[child_proc].Rollback()==0)
					theLog<<"Rollback succeed"<<endi;
				else					
					theLog<<"Rollback failed"<<endi;

				getCurTime(szCurrentTime);
				theLog<<"������Ϣ�������쳣��"<<endi;
				message.setMessage(MESSAGE_BREAK_BATCH, vecFile[i].szSourceId, vecFile[i].szRealFileName, vecFile[i].lFileId);
				argMsg.set(message);
				iRad++;
				if(iRad > 99999)
					iRad = 1;
				argMsg.setTime(szCurrentTime, iRad);
				chain->signal(pMsg);
				DealFile.rollback();
				iRet = -1;
				break;
			case -2:  //����ʧ�ܣ��ع����񣬲����ļ�״̬��Ϊ��E��
				//����ʧ�ܣ��ع�����
				theLog<<"����ʧ�ܣ��ع�����"<<endi;
				Param.lack_info.RollBack();
				Param.abnormal.RollBack();
				if( !strcmp(Param.szIsFmtFirst, "Y") && !strcmp(Param.szFmtErr2Table, "Y") )
					Param.fmt_err2table.RollBack();
				if(_DBConn[child_proc].Rollback()==0)
					theLog<<"Rollback succeed"<<endi;
				else					
					theLog<<"Rollback failed"<<endi;
				
				sprintf(szSqlStr, "update %s a set deal_flag='E' where proc_index = %d and filename = '%s' and \
					exists (select source_id from C_SOURCE_GROUP_CONFIG b where a.source_id = b.source_id and \
					b.source_group='%s') and partid in (%d,%d,%d)", Param.szLogTabname, Param.iProcessId,
					vecFile[i].szRealFileName, Param.szSourceGroupId, getPrePartID(vecFile[i].iPartID),
					vecFile[i].iPartID, getNextPartID(vecFile[i].iPartID));
				//theLog<<szSqlStr<<endi;
				ds.Open(szSqlStr, NONSELECT_DML);
				ds.Execute();
				ds.Close();
				if(_DBConn[child_proc].Commit()==0)
					theLog<<"Commit succeed"<<endi;
				else					
					theLog<<"Commit failed"<<endi;
				vecFile[i].iDealFlag = 3;
				
				getCurTime(szCurrentTime);
				theLog<<"������Ϣ�������쳣��"<<endi;
				message.setMessage(MESSAGE_BREAK_BATCH, vecFile[i].szSourceId, vecFile[i].szRealFileName, vecFile[i].lFileId);
				argMsg.set(message);
				iRad++;
				if(iRad > 99999)
					iRad = 1;
				argMsg.setTime(szCurrentTime, iRad);
				chain->signal(pMsg);
				DealFile.rollback();
				// ͬ�������Ѵ������ļ�״̬��Ϊδ���� 
				for(int j=0; j<i; j++)
				{
					if(vecFile[j].iDealFlag == 1)
					{
						theLog<<"�ļ�"<<vecFile[i].szFileName<<"״̬��ΪW"<<endi;
						vecFile[j].iDealFlag = 0;
					}
				}
				i = 0;
				goto PROC_ENTRY;
			default:
				iRet = -100;
				//������һ�㲻�ᷢ��
			}//end of switch
			if( i<(iFileCount-1)  ) 
			{
				if( !strcmp(vecFile[i].szSourceId,vecFile[i+1].szSourceId) && iDealCount<vecFile[i].iMaxCount )
				{
					continue;
				}
			}
			getCurTime(szCurrentTime);
			theLog<<"������Ϣ�����ν�����׼���ύ��"<<endi;
			message.setMessage(MESSAGE_END_BATCH_END_FILES, vecFile[i].szSourceId, vecFile[i].szRealFileName, vecFile[i].lFileId);
			argMsg.set(message);
			iRad++;
			if(iRad > 99999)
				iRad = 1;
			argMsg.setTime(szCurrentTime, iRad);
			chain->signal(pMsg);
			// �ύ 
			Param.classify.commit();

			getCurTime(szCurrentTime);
			theLog<<"������Ϣ�����ν������ύ���ݿ⣡"<<endi;
			message.setMessage(MESSAGE_END_BATCH_END_DATA, vecFile[i].szSourceId, vecFile[i].szRealFileName, vecFile[i].lFileId);
			argMsg.set(message);
			iRad++;
			if(iRad > 99999)
				iRad = 1;
			argMsg.setTime(szCurrentTime, iRad);
			chain->signal(pMsg);
			if(_DBConn[child_proc].Commit()==0)
				theLog<<"Commit succeed"<<endi;
			else					
				theLog<<"Commit failed"<<endi;
			iBatchCount=0;
			// ���ݱ��ݱ�־�Ƿ�ɾ���ļ�
			DealFile.commit(Param.bOutputFile);
			if(!Param.bBakFlag)
			{
				for(int j=0; j<=i; j++)
				{
					if(vecFile[j].iDealFlag == 1)
					{
						sprintf(szTemp, "%s%s", szFullInPath, vecFile[j].szFileName);
						theLog<<"unlink "<<szTemp<<endi;
						unlink(szTemp);
						vecFile[j].iDealFlag = 2;
					}
				}
			}
			
			vecFile.clear();
		}//end of try
		catch (jsexcp::CException &e) 
		{
			if(_DBConn[child_proc].Rollback()==0)
				theLog<<"Rollback succeed"<<endi;
			else					
				theLog<<"Rollback failed"<<endi;
			sprintf(szLogStr, "�����ļ�(%s)�Ĵ����������", vecFile[i].szFileName);
			e.PushStack(e.GetAppError(), szLogStr, __FILE__, __LINE__);
			throw e;
		} 
		catch (const std::exception &e) 
		{
			if(_DBConn[child_proc].Rollback()==0)
				theLog<<"Rollback succeed"<<endi;
			else					
				theLog<<"Rollback failed"<<endi;
			throw jsexcp::CException(1331609, e.what(), __FILE__, __LINE__);
		}
		catch(...)
		{
			if(_DBConn[child_proc].Rollback()==0)
				theLog<<"Rollback succeed"<<endi;
			else					
				theLog<<"Rollback failed"<<endi;
			throw jsexcp::CException(errno, "δ֪ϵͳ����", __FILE__, __LINE__);
		}
	}//end of for

}*/

void C_MainFlow::resetHFiles()
{
 try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			//���˹������ü���
			string sql = " update :v1 set deal_flag='W' where deal_flag='H' and validflag='Y' and proc_index = :v2 and source_id in(select source_id from C_SOURCE_GROUP_CONFIG  where source_group=':v3')";			
			stmt.setSQLString(sql);
			stmt << Param.szLogTabname<< Param.iProcessId << Param.szSourceGroupId;
			stmt.execute();
            theLog<<"Commit succeed"<<endi;
	 }else{
	 	cout<<"connect error."<<endl;
	 }
	 conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
		theLog<<"Commit failed"<<endi;
     } 	 
   // conn.commit();
    
	/*char szSqlStr[SQL_LEN+1];
	//��־�����Ƿ����ϴ��˳�ʱδ��ɵ�H�ļ�
	CBindSQL ds(_DBConn[child_proc]);
	sprintf(szSqlStr, " update %s set deal_flag='W' where deal_flag='H' and validflag='Y' and \
		proc_index = %d and source_id in(select source_id from C_SOURCE_GROUP_CONFIG \
		where source_group='%s')", Param.szLogTabname, Param.iProcessId, Param.szSourceGroupId);
	//theLog<<szSqlStr<<endi;
	ds.Open(szSqlStr, NONSELECT_DML);
	ds.Execute();
	ds.Close();
	if(_DBConn[child_proc].Commit()==0)
		theLog<<"Commit succeed"<<endi;
	else
		theLog<<"Commit failed"<<endi;*/
}

//����ֵ��-1	ʧ��
//        0		�ɹ�
//		  >0	�ļ�����
int C_MainFlow::getFilesFromDB()
{
	//CBindSQL ds(_DBConn[child_proc]);

	char szSqlStr[SQL_LEN+1];
	char szLogStr[LOG_MSG_LEN+1]; 
	char szTemp[FIELD_LEN+1];
	char szTmpPart[2+1];
	char szCurrentTime[DATETIME_LEN+1];
	int iRet;
	SFileStruct FileStruct;
	
	memset(szCurrentTime, 0, sizeof(szCurrentTime));
	getCurTime(szCurrentTime);
	memset(szTmpPart, 0, sizeof(szTmpPart));
	strncpy(szTmpPart, szCurrentTime+4, 2);

	try
	{
		/* �����ݿ����־���е�H�ļ� */
		int iFileCount = 0; //�������ļ�����
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			string sql = "select count(*) from :v1 a, C_SOURCE_GROUP_CONFIG b where a.source_id= b.source_id \
		  and a.deal_flag='W' and a.validflag='Y' and a.proc_index = :v2 and a.partid in(:v3,atoi(:v4),:v5) \
		  and b.source_group=':v6'";		
			stmt.setSQLString(sql);
			stmt << Param.szLogTabname << Param.iProcessId << getPrePartID(szTmpPart) << szTmpPart <<
		         getNextPartID(szTmpPart) << Param.szSourceGroupId;
			if (stmt.execute())
			{
                stmt >> iFileCount;
			}
	 
	 
		/*sprintf(szSqlStr, "select count(*) from %s a, C_SOURCE_GROUP_CONFIG b where a.source_id= b.source_id \
		  and a.deal_flag='W' and a.validflag='Y' and a.proc_index = %d and a.partid in(%d,%d,%d) \
		  and b.source_group='%s'", Param.szLogTabname, Param.iProcessId, getPrePartID(szTmpPart), atoi(szTmpPart),
		  getNextPartID(szTmpPart), Param.szSourceGroupId);
		ds.Open(szSqlStr, SELECT_QUERY );
		//cout<<szSqlStr<<endl;
		ds>>iFileCount;
		ds.Close();*/
		
		if (iFileCount > 0)
		{
			iUndoFlag ++;			
			memset(szTemp, 0, sizeof(szTemp));
			/*���ӿ���ж���������ļ�����Ϣ��*/
			string sql = "select filename, a.source_id from :v1 a, C_SOURCE_GROUP_CONFIG b where a.source_id= b.source_id \
			and a.deal_flag='W' and a.validflag='Y' and a.proc_index = :v2 and b.source_group=':v3' and partid in (:v4,atoi(:v5),:v6)";		
			stmt.setSQLString(sql);
			stmt << Param.szLogTabname << Param.iProcessId << Param.szSourceGroupId << 
				getPrePartID(szTmpPart) << szTmpPart << getNextPartID(szTmpPart);

			stmt.execute();
			conn.commit();
			//sprintf(szSqlStr, "select filename, a.source_id from %s a, C_SOURCE_GROUP_CONFIG b where a.source_id= b.source_id \
			//and a.deal_flag='W' and a.validflag='Y' and a.proc_index = %d and b.source_group='%s' and partid in (%d,%d,%d)",
			//Param.szLogTabname, Param.iProcessId, Param.szSourceGroupId, getPrePartID(szTmpPart), atoi(szTmpPart), getNextPartID(szTmpPart));
			//ds.Open(szSqlStr, SELECT_QUERY );
			for (int i=0; i<iFileCount; i++)
			{
				memset(&FileStruct, 0, sizeof(FileStruct));
				if (!(stmt >> FileStruct.szFileName >> FileStruct.szSourceId))
				{
					//ds.Close();
					strcpy(szLogStr, "the information in the source table  are not correct,please check ! ");
					theLog<<szLogStr<<endd;
					return -1;
				}
				else
				{
					if (FileStruct.szSourcePath[strlen(FileStruct.szSourcePath)-1] != '/')
						strcat(FileStruct.szSourcePath, "/");
					//strncpy(FileStruct.szRealFileName, FileStruct.szFileName, strrchr(FileStruct.szFileName,'.')-FileStruct.szFileName);
					strcpy(FileStruct.szRealFileName, FileStruct.szFileName);
					sprintf(FileStruct.szFileName, "%s.%d", FileStruct.szRealFileName, Param.iProcessId);
					if(getFileInfo( FileStruct ))
					{
					  theLog<<"׼������("<<FileStruct.szRealFileName<<")"<<endi;
					  vecFile.push_back( FileStruct );
					}
					else
						theLog<<"�ļ�"<<FileStruct.szRealFileName<<"�ظ���"<<endw;
				}
			}
			conn.close();
  	
			return iFileCount;
		}
	  }else{
	 	cout<<"connect error."<<endl;
	 }	
		 
		return -1;
	}
	catch(jsexcp::CException &e) 
	{
		//errLog(ERR_GET_FILE, "�Ӷ��ж�ȡ�������ļ�����", e.GetAppError(), e.GetErrMessage(), __FILE__, __LINE__);
		conn.close();
		e.PushStack(ERR_GET_FILE, "�Ӷ��ж�ȡ�������ļ�����", __FILE__, __LINE__);
		throw e;
		
	}
	catch(...)
	{
		conn.close();
		throw jsexcp::CException(errno, "�Ӷ��ж�ȡ�������ļ�����", __FILE__, __LINE__);
		//theLog<<"exit :"<<errno<<ende;
	}
}

void C_MainFlow::getSourceInfo()
{
	//CBindSQL ds(_DBConn[child_proc]);
	//char szSqlStr[SQL_LEN+1];
	//memset(szSqlStr, 0, sizeof(szSqlStr));
	SSourceStruct source;
	
 try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			//���˹������ü���
			string sql = "select a.source_id, a.source_path, a.tollcode, a.serv_cat_id, a.file_fmt, to_number(b.priority) \
		from i_source_define a, c_source_batch b, c_source_group_config c where a.source_id=b.source_id and a.source_id=c.source_id \
		and b.service=':v1' and c.source_group = ':v2'";		
			stmt.setSQLString(sql);
			stmt << Param.szServiceId << Param.szSourceGroupId;
			if (stmt.execute())
			{
                stmt >>source.szSourceId>>source.szSourcePath>>source.szTollcode>>source.szServCat
		             >>source.szSourceFiletype>>source.iMaxCount;
                if(source.szSourcePath[strlen(source.szSourcePath)-1] != '/')
			         strcat(source.szSourcePath, "/");
		    mapSource.insert(pair<string, SSourceStruct>(string(source.szSourceId), source));
			}
	 }else{
	 	cout<<"connect error."<<endl;
	 }
	// conn.commit();
	 conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
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
			string sql = "select count(*) from :v1 where source_id = ':v2' and filename = ':v3' and partid in (:v4,atoi(:v5),:v6)";
			//���SCH�Ƿ��Ѹ��ļ�
		if (bNewFile)
	      {					
			stmt.setSQLString(sql);
			stmt << Param.szLogTabname << FileStruct.szSourceId << FileStruct.szRealFileName <<
			  getPrePartID(szTmpPart) << szTmpPart << getNextPartID(szTmpPart);
			if (stmt.execute())
			{
                if(!(stmt>>iCount))
			   {
				//ds.Close();
				sprintf(szLogStr,"ִ�� %s ������",szSqlStr);
				throw jsexcp::CException(0, szLogStr, __FILE__, __LINE__);
			   }
			}

            if(iCount > 0)
			{
			sql = "update %s set deal_flag='D' where source_id=':v1' and filename=':v2' and partid in (:v3,atoi(:v4),:v5)";		
			stmt.setSQLString(sql);
			stmt <<Param.szLogTabname << FileStruct.szSourceId << FileStruct.szRealFileName <<
			  getPrePartID(szTmpPart) << szTmpPart << getNextPartID(szTmpPart);
			stmt.execute();			
          }
            
            conn.commit();
            it = mapSource.find( string(FileStruct.szSourceId) );
            if(it == mapSource.end())
			{
				/* �Ȳ�����־�������쳣 */
            sql = "insert into :v1(source_id, serv_cat_id, filename, deal_flag, validflag, proc_index, \
				  dealstarttime) values(':v2', '00', ':v3', 'E', 'Y', :v4, ':v5')";		
			stmt.setSQLString(sql);
			stmt <<Param.szLogTabname << FileStruct.szSourceId<< FileStruct.szRealFileName<<Param.iProcessId<< szCurrentTime;
			stmt.execute();
           }

			/* ��������ļ���Ϣ������־�� */
			memset(szSqlStr, 0, sizeof(szSqlStr));
            sql =  "insert into :v1(source_id, serv_cat_id, filename, deal_flag, validflag, proc_index, \
			  dealstarttime) values(':v2', ':v3', ':v4', 'W', 'Y', :v5, ':v6')";		
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
			/* �ļ����� */
			if(rename(szRealFile, szTmpFile) != 0 )
			{
				//�ȸ��ļ�״̬�����쳣
				sql = "update :v1 set deal_flag='E' where source_id=':v2' and filename=':v3' and \
					partid in (:v4,atoi(:v5),:v6)";		
			    stmt.setSQLString(sql);
			    stmt <<Param.szLogTabname<< FileStruct.szSourceId<< FileStruct.szRealFileName<<
					getPrePartID(szTmpPart)<< szTmpPart<< getNextPartID(szTmpPart);
			    stmt.execute();
			    conn.commit();
			}
			/* ����������Ϣ */
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
		/* �и�ʽ�����ʱ���ļ���ID */
		if(!strcmp(Param.szIsFmtFirst, "Y"))
		{
		    sql = "select nvl(file_id,-1), receive_time from D_FILE_RECEIVED where Source_Id=':v1' and service=':v2' \
			  and FileName=':v3' and partid in (:v4,atoi(:v5),:v6)";		
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
			/* ���ļ�����ʱ�� */
			 sql = "select receive_time from D_FILE_RECEIVED where Source_Id=':v1' and service=':v2' \
			  and FileName=':v3' and partid in (:v4,atoi(:v5),:v6)";		
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
	}
	catch(...)
	{
		throw jsexcp::CException(errno, "��ȡ�ļ���Ϣ����", __FILE__, __LINE__);
		conn.close();
		//theLog<<"exit :"<<errno<<ende;
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
	if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "PLUGIN_PATH", Param.szSlPath ) == -1 )
	{
		sprintf(szLogStr, "�������������Ҳ���VARNAME='PLUGIN_PATH'����" );
		theLog<<szLogStr<<endi;
		return -1;
	} 
	theLog<<"PLUGIN_PATH="<<Param.szSlPath<<endd;
	//cout << "PLUGIN_PATH="<<Param.szSlPath<<endd;
	/* ��ȡ����ļ��� */
	memset( Param.szSlName, 0, sizeof(Param.szSlName) );
	if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "PLUGIN_NAME", Param.szSlName ) == -1 )
	{
		sprintf(szLogStr, "�������������Ҳ���VARNAME='PLUGIN_NAME'����" );
		theLog<<szLogStr<<endi;
		return -1;
	}
	theLog<<"PLUGIN_NAME="<<Param.szSlName<<endd;
	if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "COMMON_LACKINFO_SAVE_CONFIG", Param.szLackSaveTableId ) == -1 )
	{
		sprintf(szLogStr, "�������������Ҳ���VARNAME='COMMON_LACKINFO_SAVE_CONFIG'����");
		theLog<<szLogStr<<endi;
		return -1;
	}
	if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "COMMON_LACKINFO_STAT_CONFIG", Param.szLackStatTableId ) == -1 )
	{
		memset(Param.szLackStatTableId, 0, sizeof(Param.szLackStatTableId));
	}
	if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "COMMON_ABNORMAL_SAVE_CONFIG", Param.szAbnSaveTableId ) == -1 )
	{
		sprintf(szLogStr, "�������������Ҳ���VARNAME='COMMON_ABNORMAL_SAVE_CONFIG'����");
		theLog<<szLogStr<<endi;
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
		theLog<<szLogStr<<endi;
		return -1;
	}
	if(!strcmp(szTemp, "Y"))
		Param.bCommemFlag = true;
	else if(!strcmp(szTemp, "N"))
		Param.bCommemFlag = false;
	else
	{
		sprintf(szLogStr, "������������ VARNAME='CONNECT_COMMEM_FLAG' ������ %s ����ȷ��", szTemp);
		theLog<<szLogStr<<endi;
		return -1;
	}
	theLog<<"CONNECT_COMMEM_FLAG="<<szTemp<<endd;
	if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "IS_FMT_FIRST", Param.szIsFmtFirst ) == -1 )
	{
		sprintf(szLogStr, "�������������Ҳ���VARNAME='IS_FMT_FIRST'����");
		theLog<<szLogStr<<endi;
		return -1;
	}
	theLog<<"IS_FMT_FIRST="<<Param.szIsFmtFirst<<endd;
	if(!strcmp(Param.szIsFmtFirst, "Y"))
	{
		/* �и�ʽ��ʱ */
		
		/* ��ʽ�������Ĵ������ */
		if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "FMT_ERR_W_TABLE", Param.szFmtErr2Table ) == -1 )
		{
			sprintf(szLogStr, "�������������Ҳ���VARNAME='FMT_ERR_W_TABLE'����");
			theLog<<szLogStr<<endi;
			return -1;
		}
		theLog<<"FMT_ERR_W_TABLE="<<Param.szFmtErr2Table<<endd;
		if(strncmp(Param.szFmtErr2Table, "Y", 1) == 0)
		{
			//��ȡ�����ϼ�¼����������ͳ�Ʊ���ID
			memset( Param.szFmtErrSaveTableId, 0, sizeof(Param.szFmtErrSaveTableId) );
			if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "FMT_ERR_SAVE_CONFIG", Param.szFmtErrSaveTableId ) == -1 )
			{
				sprintf(szLogStr, "�������������Ҳ���VARNAME='FMT_ERR_SAVE_CONFIG'����" );
				theLog<<szLogStr<<endi;
				return -1;
			} 
			theLog<<"FMT_ERR_SAVE_CONFIG="<<Param.szFmtErrSaveTableId<<endd;
			memset( Param.szFmtErrStatTableId, 0, sizeof(Param.szFmtErrStatTableId) );
			if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "FMT_ERR_STAT_CONFIG", Param.szFmtErrStatTableId ) == -1 )
			{
				memset( Param.szFmtErrStatTableId, 0, sizeof(Param.szFmtErrStatTableId) );
			}
			else
				theLog<<"FMT_ERR_STAT_CONFIG="<<Param.szFmtErrStatTableId<<endd;
		}
		/*if( getEnvFromDB( _DBConn, Param.szServiceId, Param.szSourceGroupId, "", "FMT_ERR_W_OUTFILE", Param.szFmtErr2File ) == -1 )
		{
			sprintf(szLogStr, "�������������Ҳ���VARNAME='FMT_ERR_W_OUTFILE'����");
			theLog<<szLogStr<<endi;
			return -1;
		}
		theLog<<"FMT_ERR_W_OUTFILE="<<Param.szFmtErr2File<<endd;
		if( getEnvFromDB( _DBConn, Param.szServiceId, Param.szSourceGroupId, "", "FMT_ERR_DIR", Param.szFmtErrDir ) == -1 )
		{
			sprintf(szLogStr, "�������������Ҳ���VARNAME='FMT_ERR_DIR'����");
			theLog<<szLogStr<<endi;
			return -1;
		}
		if(Param.szFmtErrDir[strlen(Param.szFmtErrDir)-1] != '/')
			strcat(Param.szFmtErrDir, "/");
		theLog<<"FMT_ERR_DIR="<<Param.szFmtErrDir<<endd;*/
		if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "FMT_TIMEOUT_DIR", Param.szFmtTimeOutDir ) == -1 )
		{
			sprintf(szLogStr, "�������������Ҳ���VARNAME='FMT_TIMEOUT_DIR'����");
			theLog<<szLogStr<<endi;
			return -1;
		}
		if(Param.szFmtTimeOutDir[strlen(Param.szFmtTimeOutDir)-1] != '/')
			strcat(Param.szFmtTimeOutDir, "/");
		theLog<<"FMT_TIMEOUT_DIR="<<Param.szFmtTimeOutDir<<endd;
		if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "FMT_OTHER_DIR", Param.szFmtOtherDir ) == -1 )
		{
			sprintf(szLogStr, "�������������Ҳ���VARNAME='FMT_OTHER_DIR'����");
			theLog<<szLogStr<<endi;
			return -1;
		}
		if(Param.szFmtOtherDir[strlen(Param.szFmtOtherDir)-1] != '/')
			strcat(Param.szFmtOtherDir, "/");
		theLog<<"FMT_OTHER_DIR="<<Param.szFmtOtherDir<<endd;
	}//end of fmt first
	memset(szTemp, 0, sizeof(szTemp));
	if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "SERV_CAT_CONFIG", szTemp ) == -1 )
	{
		sprintf(szLogStr, "�������������Ҳ���VARNAME='SERV_CAT_CONFIG'����");
		theLog<<szLogStr<<endi;
		return -1;
	}
	Param.iServCatConfig=atoi(szTemp);
	theLog<<"SERV_CAT_CONFIG="<<Param.iServCatConfig<<endd;
	memset(szTemp, 0, sizeof(szTemp));
	if( getEnvFromDB( Param.szServiceId, Param.szSourceGroupId, "", "OUTPUT_FILE", szTemp ) == -1 )
	{
		strcpy(szTemp, "Y");
	}
	if(!strcmp(szTemp, "Y"))
		Param.bOutputFile = true;
	else
		Param.bOutputFile = false;
	theLog<<"OUTPUT_FILE="<<Param.bOutputFile<<endd;

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
		theLog<<szLogStr<<endi;
		return -1;
	}
	theLog<<"INFILE_BAK_FLAG="<<szTemp<<endd;

	/***********************************************************************
	/*  ��ѯ������ģ��ID��workflow_id��
	/***********************************************************************/
	 try{			
	if (dbConnect(conn))
	 {
		Statement stmt = conn.createStatement();
		string sql = "select server_id, workflow_id, filetype_id from c_source_group_define where source_group=':v1'";		
		stmt.setSQLString(sql);
		stmt << Param.szSourceGroupId;
		stmt.execute();
		if(!(stmt >> Param.szServerId>>Param.szWorkflowId>>Param.szOutputFiletypeId))
  	    {
  		  throw jsexcp::CException(0, "�Ҳ���������������", __FILE__, __LINE__);
  	    }

		/* ��ѯ����������Ʊ�����·�� */
        sql = "select a.log_tabname, a.work_path from c_service_interface a, c_service_flow b \
		where a.interface_id=b.input_id and b.workflow_id=':v1' and b.service=':v2'";		
		stmt.setSQLString(sql);
		stmt << Param.szWorkflowId<< Param.szServiceId;
		stmt.execute();
		if(!(stmt >> Param.szLogTabname>>Param.szInPath))
  	    {
  		  throw jsexcp::CException(0, "�Ҳ���������������", __FILE__, __LINE__);
  	    }
		
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
	  theLog<<"szOutputFiletypeId="<<Param.szOutputFiletypeId<<endd;
       sql = "select record_type from c_filetype_define where filetype_id=:v1";		
		stmt.setSQLString(sql);
		stmt << Param.szOutputFiletypeId;
		stmt.execute();
		if(!(stmt >>Param.szOutrcdType))
  	    {
  		   sprintf(szLogStr, "�ڱ�C_FILETYPE_DEFINE���Ҳ��� %s ������", Param.szOutputFiletypeId);
		   //theLog<<szLogStr<<endi;
		   throw jsexcp::CException(0, szLogStr, __FILE__, __LINE__);
  	    }

		sql = "select interval from c_process_ctl where source_group = '%s' AND service = '%s'";		
		stmt.setSQLString(sql);
		stmt << Param.szSourceGroupId<< Param.szServiceId;
		stmt.execute();
		if(!(stmt >>szTemp))
  	    {
  		   throw jsexcp::CException(0, "�Ҳ���sleeptime������", __FILE__, __LINE__);
  	    }
        Param.iSleepTime = atoi(szTemp);
	    theLog<<"SleepTime="<<Param.iSleepTime<<endd;		
			
	 }else{
	 	cout<<"connect error."<<endl;
	 }
	 conn.commit();
	 conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
    }
	return 0;
}

void C_MainFlow::exit()
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
}

bool C_MainFlow::DBLinkError()
{
	return m_bDBLinkError;
}

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

 int C_MainFlow::onBeforeTask()
  {
     // ������ƽ�⻰��������
     //cout << "test onbeforeTask" << endl;
     int ret=1;
	int event_sn, event_type;
	long param1, param2, src_id;
	while(ret)	//����ӽ�����Ϣ����
	{
		ret=mgetChldEvt(event_sn, event_type, param1, param2, src_id, false);
		if(ret<0)
		{
			cerr<<"����ӽ�����Ϣ����ʧ��!\n";
			return false;
		}
	}
	
     m_dealRcdNum.clear();      
     return 1; // ��ʼ���뻰����
  }

  int C_MainFlow::onTaskBegin(void *task_addr)
  {     
     // ���ݿ������жϵ��ӽ�������Ŀ�����仯ʱ����������
     if (tmp_childnum == 0)
     {
        tmp_childnum = PS_BillProcess::getChldPrcNum(); 
     }
    /* if (tmp_childnum != PS_BillProcess::getChldPrcNum())
     {
        DBConn.Disconnect();
        connectDB( "", DBConn );
     }*/

     PkgBlock pkg((char*)(task_addr),0,0);
     int length = PS_BillProcess::getTicketLength();
     int block = PS_BillProcess::getBlockSize();
     pkg.init( length , block ); //�����¼�д�С�Ϳ��С
     //ticket_num   = pkg.getRecordNum();
     //cout << "ticket_num = " << pkg.getRecordNum() << endl;
     
     ArgMessage argMsg;   
     char szCurrentTime[DATETIME_LEN+1];
	 MessageParser message;
     Argument *pMsg = &argMsg;

     block_addr = task_addr;

     strcpy(szFileName,pkg.getFileName());
     strcpy(szSourceId,pkg.getSourceId());
     
	 getCurTime(szCurrentTime);
	 getCurTime(m_szBeginTime);  //�ļ���ʼ����ʱ��
	// ��ȡҵ�����
     try{			
	  if (dbConnect(conn))
	    {
			Statement stmt = conn.createStatement();
			string sql = "select serv_cat_id from i_source_define where source_id = ':v1' ";		
			stmt.setSQLString(sql);
			stmt << szSourceId;
			if (stmt.execute())
			{
                stmt >> servCat;
			}
			
	    }else{
	 	cout<<"connect error."<<endl;
	  }
	   conn.commit();
	   conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
    } 	   
	   
	// ׼�������뻰����
     return 1;
  }

  int C_MainFlow::onTaskOver(int child_ret)
  {
  // �ӻ������л�ȡ��������
    PkgBlock pkg((char*)(block_addr));
    int length = PS_BillProcess::getTicketLength();
    int block = PS_BillProcess::getBlockSize();
    
    char * pkgRecord;                    //��������
	int pkgSize;                         //�����м�¼��С
    pkg.init( length , block ); //�����¼�д�С�Ϳ��С 
    int ticket_num = pkg.getRecordNum();
    //cout << "�������ַ" << block_addr << endl; 
    //cout << "ticket_num = " << ticket_num << endl;
    
    /*cout<<"������¼��: "<<pkg.getRecordNum()<<endl;
	cout<<"ģ��ID: "<<pkg.getModuleId()<<endl;
	cout<<"����Դ�� "<<pkg.getSourceId()<<endl;
	cout<<"�����ļ�����"<<pkg.getFileName()<<endl;
	cout<<"��¼״̬��"<<pkg.getStatus()<<endl;			
	cout << "��¼�д�С" << pkg.getLineSize() << endl; 
	cout << "length =" << length <<endl;
	cout << "block =" << block <<endl;
    for(int k = 1;k<=ticket_num;k++)
       {
		  PkgFmt fmt = pkg.readPkgRecord(k);
          pkgRecord = pkg.getPkgRcd(k);  //  ��ȡָ���м�¼ֵ        
		  cout<<"��¼״̬��"<<fmt.status<<"  ��¼���룺"<<fmt.code<<"  ��¼ֵ��|"<<fmt.record<<"|"<<endl;
    	} */

     /* if (DBConn.IsConnected() == -1) {
      	cout<<"in over DBConn reconnect..."<<endl;
      	connectDB( "", DBConn );
      }*/
      child_ret = PS_BillProcess::getChldPrcNum();  

      /* �����������ο�ʼ����Ϣ */
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

      //�ּ𵥻�ȡͨ������־
      for (int i = 1; i<=ticket_num ; i++)
      {
        char *tmp = pkg.getRcdType(i);
        if(strcmp(tmp,"C")==0)
        {
           _pick = _pick+1;
           _other = _other +1;
        } else if(strcmp(tmp,"B")==0)
        {
          _other = _other+1;
        }
      }
     cout <<"�������ۼ�ֵ"<< " total = "<<_total<<"  _right = "<<_right<<"  _lack= "<<_lack<<"  _error = "<<_error
     	 <<"  _pick = "<< _pick << "  _other = " << _other <<endl;  
     // ������ͳ�����
     m_dealRcdNum.set(servCat,_total,_right,_lack,_error,_pick,_other); //��ƽ�⻰�������õ���������     
     m_dealRcdNum.check();
     m_dealRcdNum.updateTable(Param.szLogTabname,szFileName,m_szBeginTime, szSourceId,Param.iProcessId);
   
     //cout<<"test DBConn is connected or not!"<<endl;
     //cout<<"DBConn.IsConnected():"<<DBConn.IsConnected()<<endl;
    /* if (DBConn.IsConnected() == -1) {
      	cout<<"in over DBConn reconnect..."<<endl;
      	connectDB( "", DBConn );
     }
      
     DBConn.Commit();*/
     //cout<<"return child_ret:"<<child_ret<<endl;
    
      // �����ӽ�����      
     return child_ret;
  }


  bool C_MainFlow::onChildInit()
  {
     // ��ʼ���������͹��ñ���     
     char szLogPath[] = {"/mboss/home/zhjs/log"};
	 char szLogLevel[] = {"2"};
     theLog.setLog(szLogPath, atoi(szLogLevel), Param.szServiceId, Param.szSourceGroupId, m_lPrc_ID);
       /* �������ݿ� */
	 /*theLog<<"�ӽ��̿�ʼ�������ݿ⡭��"<<endd;
		try
		{
             // �޸��������ݿⷽʽ
             connectDB("", _DBConn[child_proc]);
			theLog<<"�ӽ����������ݿ�ɹ���"<<endi;
		}
		catch(jsexcp::CException &e)
		{
			Param.info.msgInfoLog(INFO_INITFAIL);
			e.PushStack(PREDEAL_ERR_IN_CONNECT_DB, "�ӽ����������ݿ�ʧ��", __FILE__, __LINE__);
			throw e;
		}
		catch(...)
		{
			Param.info.msgInfoLog(INFO_INITFAIL);
			theLog<<"errno "<<errno<<ende;
			throw jsexcp::CException(PREDEAL_ERR_IN_CONNECT_DB, "�ӽ����������ݿ�ʧ��", __FILE__, __LINE__);
		}*/

			/***********************************************************************************

		*��ȡ���������е����빲���ڴ�ĵ�ַ

		***********************************************************************************/
		if(Param.bCommemFlag)
		{
		    //CBindSQL ds(DBConn);
			memManager = new C_AccessMem();
			memManager->init(Param.szServerId,m_szEnvPath);
		}
		
	    Param.pluginInitializer.initialize(Param.szServiceId, Param.szSourceGroupId, Param.szSlPath, Param.szSlName);	  	    
	    cout<<"��ʼ��������ɹ���"<<endl;

	    if(chain == NULL)
		{
			cout<<"chain is null"<<endl;
		}	    
		
     pps = new PacketParser();
	 res = new ResParser();
     return true;
  }

  int C_MainFlow::onTask(void *task_addr, int offset, int ticket_num)
  {

    ////////////////////////////////////////
    //theLog << "offset = " << offset << endi;
    //theLog << "ticket_num = " << ticket_num << endi;
    
        char szCurrentTime[DATETIME_LEN+1];
		char szLogStr[LOG_MSG_LEN+1]; 
		if(chain != NULL)
		{
			//delete chain;
			chain = NULL;
		}
		chain = Param.pluginInitializer.getFilterChain();
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
     
	ArgInit initPlugin;
	initPlugin.set(Param.szSourceGroupId, Param.szServiceId, Param.iProcessId, memManager);
	getCurTime(szCurrentTime);
	initPlugin.setTime(szCurrentTime, 0);
		
	chain->signal(&initPlugin);
	 theLog<<"��ʼ���ɹ���"<<endi;
		
	 theLog<<"������Ϣ�����ο�ʼ��"<<endi;
	 message.setMessage(MESSAGE_NEW_BATCH, szSourceId, szFileName, 0);
	 
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
		theLog<<"������Ϣ���ļ���ʼ��"<<endi;
		message.setMessage(MESSAGE_NEW_FILE, szSourceId, szFileName, 0);
		argMsg.set(message);
		iRad++;
		if(iRad > 99999)
			iRad = 1;
		argMsg.setTime(szCurrentTime, iRad);
		chain->signal(pMsg);		

///////////////////////////////////////////////////////////////////////////
    // �ӻ������л�ȡ��������
    PkgBlock pkg((char*)(task_addr),offset,ticket_num);
    int length = PS_BillProcess::getTicketLength();
    int block = PS_BillProcess::getBlockSize();
    
	//pkg.init(1024,4096*500); 
	//block_addr = (char*)task_addr; 
    pkg.init( length , block ); //�����¼�д�С�Ϳ��С    

	char szSourcePath[PATH_NAME_LEN+1];  //����Դ·��
	char szServCat[SERVER_LEN+1];        //����ԴĬ�ϵ�ҵ�����
	char * pkgRecord;                    //��������
	int pkgSize;                         //�����м�¼��С
	    
	//ticket_num   = pkg.getRecordNum();
	pkgSize   = pkg.getLineSize();    //�����м�¼��С
	/*cout << "�������ַ" << task_addr << endl; 
    cout<<"������¼��: "<<pkg.getRecordNum()<<endl;
	cout<<"ģ��ID: "<<pkg.getModuleId()<<endl;
	cout<<"����Դ�� "<<pkg.getSourceId()<<endl;
	cout<<"�����ļ�����"<<pkg.getFileName()<<endl;
	cout<<"��¼״̬��"<<pkg.getStatus()<<endl;			
	cout << "��¼�д�С" << pkgSize << endl; */
	
     // 1����ȡָ�룬��������    
     // 2�����ò���������������
    C_DealCalculator m_dealRcdNum;   
    vector<CDealedFileInfo> vecFile;
	vector<string> vecRec;
	
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

	/* ��ʼ�������Ͻӿ� */

	char szFmtTimeOutFile[FILE_NAME_LEN+1];
	char szFmtTimeOutTmp[FILE_NAME_LEN+1];
	FILE *fpFmtTimeOut = NULL;
	char szFmtOtherFile[FILE_NAME_LEN+1];
	char szFmtOtherTmp[FILE_NAME_LEN+1];
	FILE *fpFmtOther = NULL;
	char szFmtErrFile[FILE_NAME_LEN+1];
	char szFmtErrTmp[FILE_NAME_LEN+1];
	int iFmtTimeOutNum = 0;
	int iFmtOtherNum = 0;
	int iFmtErrorNum = 0;

	/*��ʼ���ʹ�����ļ�*/
	char OutTmpFilePath[FILE_NAME_LEN+PATH_NAME_LEN+1];
	char OutRealFilePath[FILE_NAME_LEN+PATH_NAME_LEN+1];
	char OutRealFileName[FILE_NAME_LEN+1];
	//CF_MemFileO _outfile;
    CFmt_Change _inrcd;
	//CBindSQL ds(_DBConn[child_proc]);
    _inrcd.Init(Param.szOutputFiletypeId,'0');

	/*if(Param.bOutputFile)
	{
        ///  ����ļ���·����ȡд��·��  /mboss/home/zhjs/data/service/SOURCEGROUP/SOURCE
		//strcpy(OutRealFileName, FileStruct.szRealFileName); 
        sprintf(OutRealFileName,szFileName);
		sprintf(OutTmpFilePath, "/mboss/home/zhjs/data/service/%s%stmp_out/~%s.%d",Param.szSourceGroupId , Param.szInPath, OutRealFileName, Param.iProcessId);
		sprintf(OutRealFilePath, "/mboss/home/zhjs/data/service/%s%s%s", Param.szSourceGroupId, Param.szOutPath, OutRealFileName);
		//_outfile.Init( Param.szOutputFiletypeId );
		//_outfile.Open(OutTmpFilePath);
	}  */
	/* ��ʼ�������Ͻӿ� */

	char CurrentTime[DATETIME_LEN+1];
	getCurTime(CurrentTime);
    Param.classify.initFile(Param.szServiceId, szSourceId, OutRealFilePath, 
		OutRealFileName, Param.szOutputFiletypeId, CurrentTime);
    //cout << "Param.szOutputFiletypeId = " << Param.szOutputFiletypeId << endl;
////////////////////////////////////////////////////////
	CDealedFileInfo fname;
	strcpy(fname.szOutputFiletypeId, Param.szOutputFiletypeId);
	fname.szOutrcdType = Param.szOutrcdType[0];
	vecRec.clear();
	string strRec;
	char tmpRec[RECLEN+1];
	memset(tmpRec, 0, sizeof(tmpRec));  
	int iTotalNum = 0, iLackInfoType, iBcount, iOcount;
	pluginAnaResult Result;
	int classifyType;
	char RecordNo[FIELD_LEN];
	char szBuff[MAX_LINE_LENGTH+1];
	memset(szBuff, 0, sizeof(szBuff));
	
	memset(szServCat, 0, sizeof(szServCat));
	FilterChain *chain = Param.pluginInitializer.getFilterChain();
	if(chain == NULL)
	{
		sprintf(szLogStr, "�Ҳ��������");
		theLog<<szLogStr<<ende;
		return -1;
	}
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

   int recordlenth;
   try{			
	if (dbConnect(conn))
	  {
			Statement stmt = conn.createStatement();
			string sql = "select record_len from c_filetype_define where filetype_id = ':v1' ";		
			stmt.setSQLString(sql);
			stmt << Param.szOutputFiletypeId;
			if (stmt.execute())
			{
                stmt >> recordlenth;
			}
	  }else{
	 	cout<<"connect error."<<endl;
	 }
	 conn.commit();
	 conn.close();
	 } catch( SQLException e ) {
		cout<<e.what()<<endl;
		conn.close();
     } 	 
	
	/* �ظ���ȡ���еĻ�����������д��� */ 

	theLog<<"File in processing..."<<endd;
	try
	{
		//while(1)
		//{
			// Init record
	   m_dealRcdNum.clear();
	   for(int k = 1;k<=ticket_num;k++)
       {
		  PkgFmt fmt = pkg.readPkgRecord(k);
          pkgRecord = pkg.getPkgRcd(k);  //  ��ȡָ���м�¼ֵ      
          //���û���������־  �Ѵ�����Y�����ڴ�����T����������H��δ������N         
          pkg.setRcdStatus(k,"T");
          if (k != ticket_num)  //�����һ��
          {
             pkg.setRcdStatus(k+1,"H");
             //cout << "next ����״ֵ̬ = " << pkg.getRcdStatus(k+1) << endl;
          }
		  //cout << "����״ֵ̬ = " << pkg.getRcdStatus(k) << endl;
		  
		  //cout<<"��¼״̬��"<<fmt.status<<"  ��¼���룺"<<fmt.code<<"  ��¼ֵ��|"<<fmt.record<<"|"<<endl;
			try {
				pps->clear();
				res->clear();
				Result = eNormal;
				classifyType = 0;
				/* д�ļ�ƫ���� */
                pps->setOffset(offset);
                _inrcd.Set_record(fmt.record,recordlenth);
 
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
				e.PushStack(e.GetAppError(), "initialize record error", __FILE__,__LINE__);
				throw e;
			} catch (const std::exception &e) {
				throw jsexcp::CException(0, e.what(), __FILE__, __LINE__);
			} catch (...) {
				throw jsexcp::CException(0, "unknown error", __FILE__, __LINE__);
			}

		try{
			/* ������� */
			while( chain->hasNextAction() )
			{
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
						pkg.setRcdType(k,"C");
						//cout << "����ּ𻰵�" << pkg.getRcdType(k) << endl;
						pkg.setRcdCode(k,res->getRuleType());
						//cout << "����ּ�code = " << res->getRuleType() << endl;
						break;
					}
					else if(classifyType == 3)
					{
						//���ݷּ𻰵�
						pkg.setRcdType(k,"B");  
						//cout << "���ݷּ𻰵�" << pkg.getRcdType(k) << endl;
						//cout << "���ݷּ�code = " << pkg.getRcdCode(k) << endl;
						
						res->resetAnaResult();
						Result = eNormal;
						/* ��¼������ */
						memset(szServCat, 0, sizeof(szServCat));
						(pps->m_inRcd).Get_Field(Param.iServCatConfig, szServCat);
						if(strlen(szServCat)==0)
							strcpy(szServCat, servCat);
						m_dealRcdNum.set(szServCat, 0, 0, 0, 0, 0, 1);
						m_dealRcdNum.setFee(0, 0, 0, 0, 0, (pps->m_inRcd).Get_Fee());
						m_dealRcdNum.setTime(0, 0, 0, 0, 0, (pps->m_inRcd).Get_Time());
						pkg.setRcdCode(k,res->getRuleType());
					}
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
					}
				}
				else if( Result == eAbnormal )  /* �쳣�澯 */
				{
					//cout<<"abnormal: "<<res->getRuleType()<<"\t"<<res->getReason()<<endl;
					Param.abnormal.saveErrLackRec(pps->m_inRcd, szSourceId, OutRealFileName, res->getRuleType(), res->getReason());
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
				e.PushStack(e.GetAppError(), "process record error", __FILE__, __LINE__);
				throw e;
			} catch (const std::exception &e) {
				throw jsexcp::CException(0, e.what(), __FILE__, __LINE__);
			} catch (...) {
				throw jsexcp::CException(0, "unknown error", __FILE__, __LINE__);
			}

			try {
				switch (Result)
				{
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
						pkg.setRcdType(k,"R");
					}

					/* ��¼������ */

					memset(szServCat, 0, sizeof(szServCat));
					(res->m_outRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, servCat);
					m_dealRcdNum.set(szServCat, 1, 1, 0, 0, 0, 0);
					m_dealRcdNum.setFee((res->m_outRcd).Get_Fee(), (res->m_outRcd).Get_Fee(), 0, 0, 0, 0);
					m_dealRcdNum.setTime((res->m_outRcd).Get_Time(), (res->m_outRcd).Get_Time(), 0, 0, 0, 0);
					break;
				case eClassifiable:  /*����ּ𻰵�*/
					/* ��¼������ */
					memset(szServCat, 0, sizeof(szServCat));
					(res->m_outRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, servCat);
					m_dealRcdNum.set(szServCat, 1, 0, 0, 0, 1, 1);
					m_dealRcdNum.setFee((res->m_outRcd).Get_Fee(), 0, 0, 0, (res->m_outRcd).Get_Fee(), (res->m_outRcd).Get_Fee());
					m_dealRcdNum.setTime((res->m_outRcd).Get_Time(), 0, 0, 0, (res->m_outRcd).Get_Time(), (res->m_outRcd).Get_Time());
                    pkg.setRcdType(k,"C"); //����ּ�ͬʱ�ŵ����ݷּ���
					break;
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
					pkg.setRcdType(k,"W"); //�ص����� error
					cout << "�ص�" << endl;
					break;
				case eFmtErr:  /* ��ʽ������ */
					iFmtErrorNum++;
					/* �������ļ� */
					if( !strcmp(Param.szFmtErr2Table, "Y") )
						Param.fmt_err2table.saveErrLackRec(pps->m_inRcd, szSourceId, OutRealFileName, res->getRuleType(), res->getReason());
					if( !strcmp(Param.szFmtErr2File, "Y") )
					{
						sprintf(szLongStr, "%d%c%s%c%s%c%s", pps->getOffset(), pps->m_inRcd.Get_FieldSep(1), res->getRuleType(), pps->m_inRcd.Get_FieldSep(1), res->getReason(), pps->m_inRcd.Get_FieldSep(1), pps->m_inRcd.Get_record());
						Param.fmt_err2file.writeRec(szLongStr);
					}
					/* ��¼������ */
					memset(szServCat, 0, sizeof(szServCat));
					(pps->m_inRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, servCat);
					m_dealRcdNum.set(szServCat, 1, 0, 0, 1, 0, 0);
					m_dealRcdNum.setFee((pps->m_inRcd).Get_Fee(), 0, 0, (pps->m_inRcd).Get_Fee(), 0, 0);
					m_dealRcdNum.setTime((pps->m_inRcd).Get_Time(), 0, 0, (pps->m_inRcd).Get_Time(), 0, 0);
					break;
				case eFmtTimeOut:  /* ��ʽ����ʱ��  */
					iFmtTimeOutNum++;
					/* ���ļ� */
					fprintf(fpFmtTimeOut, "%s\n", (pps->m_inRcd).Get_record());
					/* ��¼������ */
					memset(szServCat, 0, sizeof(szServCat));
					(pps->m_inRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, servCat);
					m_dealRcdNum.set(szServCat, 1, 0, 0, 1, 0, 0);
					m_dealRcdNum.setFee((pps->m_inRcd).Get_Fee(), 0, 0, (pps->m_inRcd).Get_Fee(), 0, 0);
					m_dealRcdNum.setTime((pps->m_inRcd).Get_Time(), 0, 0, (pps->m_inRcd).Get_Time(), 0, 0);
					break;
				case eFmtOther:  /* ��ʽ��δ�����ʽ���� */
					iFmtOtherNum++;
					/* ���ļ� */
					fprintf(fpFmtOther, "%s\n", pps->getString());
					/* ��¼������ */
					memset(szServCat, 0, sizeof(szServCat));
					strcpy(szServCat, servCat);
					m_dealRcdNum.set(szServCat, 1, 0, 0, 1, 0, 0);
					m_dealRcdNum.setFee(0, 0, 0, 0, 0, 0);
					m_dealRcdNum.setTime(0, 0, 0, 0, 0, 0);
					break;
				case eAbnormal:  /* �쳣�澯 */
					Param.abnormal.saveErrLackRec(pps->m_inRcd, szSourceId, OutRealFileName, res->getRuleType(), res->getReason());
					/* ��¼������ */
					memset(szServCat, 0, sizeof(szServCat));
					(pps->m_inRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, servCat);
					m_dealRcdNum.set(szServCat, 1, 1, 0, 0, 0, 1);
					m_dealRcdNum.setFee((pps->m_inRcd).Get_Fee(), (pps->m_inRcd).Get_Fee(), 0, 0, 0, (pps->m_inRcd).Get_Fee());
					m_dealRcdNum.setTime((pps->m_inRcd).Get_Time(), (pps->m_inRcd).Get_Time(), 0, 0, 0, (pps->m_inRcd).Get_Time());
                    pkg.setRcdType(k,"L");  //������lackinfo
					break;
				default:  /* �����ϻ��쳣 */
					Param.lack_info.saveErrLackRec(pps->m_inRcd, szSourceId, OutRealFileName, res->getRuleType(), res->getReason());
					/* ��¼������ */
					memset(szServCat, 0, sizeof(szServCat));
					(pps->m_inRcd).Get_Field(Param.iServCatConfig, szServCat);
					if(strlen(szServCat)==0)
						strcpy(szServCat, servCat);
					m_dealRcdNum.set(szServCat, 1, 0, 1, 0, 0, 0);
					m_dealRcdNum.setFee((pps->m_inRcd).Get_Fee(), 0, (pps->m_inRcd).Get_Fee(), 0, 0, 0);
					m_dealRcdNum.setTime((pps->m_inRcd).Get_Time(), 0, (pps->m_inRcd).Get_Time(), 0, 0, 0);
					pkg.setRcdType(k,"L"); //������lackinfo
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
				throw e;
			} catch (const std::exception &e) {
				throw jsexcp::CException(1331609, e.what(), __FILE__, __LINE__);
			} catch (...) {
				throw jsexcp::CException(1331609, "unknown error", __FILE__, __LINE__);
			}
		}
		
		/* �ر���������ļ� */
		//_outfile.Close();
		//Param.classify.endFile();

		/* ������ʽ������ */

		/* �洢�ļ���Ϣ */
		strcpy(fname.m_szOutTmpFileName, OutTmpFilePath);
		strcpy(fname.m_szOutRealFileName, OutRealFilePath);
		strcpy(fname.szFileName, szFileName);
		strcpy(fname.szSourceId, szSourceId);
		//strcpy(fname.szDealStartTime, FileStruct.szDealStartTime);
		//strcpy(fname.szDealEndTime, FileStruct.szDealEndTime);
		vecFile.push_back(fname);

		// Print performance, ����������������־����process_env�а�PLUGIN_ENABLE_PERFORMANCE_ANA��Ϊfalse
		filterchain::util::PerformanceAnalyzer* analyzer =
				Param.pluginInitializer.getPerformanceAnalyzer();
		if (analyzer) {
			gettimeofday(&file_finish_time, NULL);
			unsigned long file_time = (file_finish_time.tv_sec
					- file_start_time.tv_sec) * (long) 1000 * (long) 1000
					+ (file_finish_time.tv_usec - file_start_time.tv_usec);
			theLog << "process current file with in " << file_time
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
				theLog << "action:" << ap->getActionId() << " time:"
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
     // 3��д��Ϣ�� ���븸����
	
     // 4����ӡƽ�⻰����
      // ƽ�⻰�������
     long _total = 0;
     _total = m_dealRcdNum.getTotal();  //�ܻ�����
     long _right = 0;
     _right = m_dealRcdNum.getRight();  //��ȷ������
     long _lack  = 0;
     _lack = m_dealRcdNum.getLack();  //�����ϵ�
     long _error = 0;
     _error = m_dealRcdNum.getError();  //����������ʽ��ȡ����ȥ�ص�Ҳ�Ǵ���     
     long _pick  =  0;
     _pick = m_dealRcdNum.getPick();   //�ּ�,����ּ�
     long _other = 0;
     _other = m_dealRcdNum.getOther();  //����������

     
     cout <<"�ӽ���"<< "  total = "<<_total<<"  _right = "<<_right<<"  _lack= "<<_lack<<"  _error = "<<_error<<endl;
    // long param1,param2;
    // param1 = _total*100000 + _right;
     //param2 = _lack*100000 + _error;
     //cout << "param1 = " <<param1 << "param2 " << param2<< endl;

     // ���������ŵ���Ϣ������        
     //cout << "child_proc= "<< child_proc << "tmp= "<< tmp << endl;
     //cout << "ontask m_lPrc_ID = " << m_lPrc_ID<< endl;
    if( !mputChldEvt(0, EVT_REQ_TRNUM, _total*100000 + _right, _lack*100000 + _error, m_lPrc_ID-child_proc-1) )
	{
		cerr<<"������"<<m_lPrc_ID<<"��������ʧ��!"<<endl;
		return false;
	}     

     sleep(10);   


      getCurTime(szCurrentTime);
	 theLog<<"������Ϣ�����ν�����"<<endi;
	 message.setMessage(MESSAGE_NEW_BATCH, szSourceId, szFileName, 0);
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
		theLog<<"������Ϣ���ļ�������"<<endi;
		message.setMessage(MESSAGE_NEW_FILE, szSourceId, szFileName, 0);
		argMsg.set(message);
		iRad++;
		if(iRad > 99999)
			iRad = 1;
		argMsg.setTime(szCurrentTime, iRad);
		chain->signal(pMsg);
     
	 //DBConn.Disconnect();
     return ticket_num;
  }

  void C_MainFlow::onChildExit()
  {
     delete pps;
     delete res;
     //int i = DBConn.IsConnected();
     //cout << "����������exit �Ƿ�����" << i << endl;
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
 
void dealSignal(int sig)
{
	if(sig == SIGTERM)
	{
		theLog<<"�յ���ֹ������źţ�"<<sig<<endi;
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
