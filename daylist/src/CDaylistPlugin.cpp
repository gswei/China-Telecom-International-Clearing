#include "CDaylistPlugin.h"
using namespace std;
using namespace tpss;
//CDatabase DBConn;
//CLog theJSLog;

CDaylistPlugin::CDaylistPlugin()
{
	newDealFlag=1;
}

CDaylistPlugin::~CDaylistPlugin()
{
}

void CDaylistPlugin::init( char *szSourceGroupID, char *szServiceID, int index)
{
	theJSLog<<"���˳�ʼ��..."<<endi;
	
	strcpy(szSourceGroup,szSourceGroupID);
	//CBindSQL ds(DBConn);	
	DBConnection conn;//���ݿ�����
 try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();						
			//�����˶���������дһ���ʵ�
     	char cTreeMaxSize[10];
     	memset(cTreeMaxSize, 0, 10 );
    	if( getEnvFromDB(szServiceID, szSourceGroupID,"-", "DAYLY_TREE_MAX_SIZE", cTreeMaxSize)<0)
    		{
    		strcpy(szLogStr,"TREE_MAX_SIZE ����û�ж���!");
    		theJSLog<<szLogStr<<endw;
      		IntTreeMaxSize=2000;
    		}
    	else
    		{
    		IntTreeMaxSize=atoi(cTreeMaxSize);
    		}
    	theJSLog<<"TREE_MAX_SIZE="<<IntTreeMaxSize<<endi;
    
    	//��DB�ж�ȡԤ�ʵ����ɳ����У�����ģ��ĸ�·�� - DAYLY_LIST_PATH:
    	char szDaylyListPath[256];
    	if( getEnvFromDB(szServiceID, szSourceGroupID,"-", "DAYLY_LIST_PATH", szDaylyListPath)<0)
    		{
    		strcpy(szLogStr,"DAYLY_LIST_PATH ����û�ж���!");
    		errLog(LEVEL_ERROR, "",ERR_GET_ENV, szLogStr, __FILE__, __LINE__);
    		throw CException(ERR_GET_ENV,szLogStr,__FILE__,__LINE__);
    		}
    	theJSLog<<"DAYLY_LIST_PATH="<<szDaylyListPath<<endi;
    	
    	char okpath[300];
    	sprintf(okpath,"%s/ok",szDaylyListPath);	
    	chkAllDir( okpath );	
    		
    	//ȡ��������
     	memset( cMergeCondiction, 0, 250);
    	if(getEnvFromDB(szServiceID, szSourceGroupID,"-","DAYLY_LIST_CONDICTION", cMergeCondiction )<0)
    		{
    		strcpy(szLogStr,"DAYLY_LIST_CONDICTION ����û�ж���!");
    		errLog(LEVEL_ERROR, "",ERR_GET_ENV, szLogStr, __FILE__, __LINE__);
    		throw CException(ERR_GET_ENV,szLogStr,__FILE__,__LINE__);
    		}
    	theJSLog<<"DAYLY_LIST_CONDICTION="<<cMergeCondiction<<endi;
    
    	//��������������
    	if(getEnvFromDB( szServiceID, szSourceGroupID,"-", "DAYLY_LIST_TOLLCODE_COND", cWithTollcodeCondiction )<0)
    		{
    		strcpy(szLogStr,"DAYLY_LIST_TOLLCODE_COND ����û�ж���!");
    		errLog(LEVEL_ERROR, "",ERR_GET_ENV, szLogStr, __FILE__, __LINE__);
    		throw CException(ERR_GET_ENV,szLogStr,__FILE__,__LINE__);
    		}
    	theJSLog<<"DAYLY_LIST_TOLLCODE_COND="<<cWithTollcodeCondiction<<endi;
    
    	//������Щ�ʵ�
    	char   cListConfigID[250];
    	if(getEnvFromDB( szServiceID, szSourceGroupID,"-", "DAYLY_LIST_CONFIG_ID", cListConfigID )<0)
    		{
    		strcpy(szLogStr,"DAYLY_LIST_CONFIG_ID ����û�ж���!");
    		errLog(LEVEL_ERROR, "",ERR_GET_ENV, szLogStr, __FILE__, __LINE__);
    		throw CException(ERR_GET_ENV,szLogStr,__FILE__,__LINE__);
    		}
    	theJSLog<<"DAYLY_LIST_CONFIG_ID="<<cListConfigID<<endi;
    
    	//���ʷֵ�ʱ��㣬20070313
    	char endTime[6];
    	if(getEnvFromDB(szServiceID, szSourceGroupID,"-",  "DAYLY_LIST_END_TIME", endTime )<0)
    		{
    		strcpy(szLogStr,"variable DAYLY_LIST_END_TIME has not been defined!");
    		errLog(LEVEL_ERROR, "",ERR_GET_ENV, szLogStr, __FILE__, __LINE__);
    		strcpy(endTime,"00:00");
    		}
    	theJSLog<<"DAYLY_LIST_END_TIME="<<endTime<<endi;
    	if(strlen(endTime)!=SEPERATETIMELENGTH)
    		{
    		strcpy(szLogStr,"DAYLY_LIST_END_TIME value defined error!");
      		errLog(LEVEL_ERROR, "",ERR_GET_ENV, szLogStr, __FILE__, __LINE__);
      		throw CException(ERR_GET_ENV,szLogStr,__FILE__,__LINE__);
    		}
    
        	//�����������ļ�����
        	string sql = "select FILETYPE_ID from C_SOURCE_GROUP_DEFINE where SOURCE_GROUP=:v";		
			    stmt.setSQLString(sql);
			    stmt << szSourceGroupID;			
			    stmt.execute();
			   
        	if (!(stmt>>szInputFiletypeId))
        	{
          		strcpy(szLogStr, "��ȡ�ļ�����ʧ��!");
          		errLog(LEVEL_ERROR, "",ERR_SELECT, szLogStr, __FILE__, __LINE__);
          		//ds.Close();
          		throw CException(ERR_SELECT,szLogStr,__FILE__,__LINE__);
        	}
        	//ds.Close();
    	    theJSLog<<"FILETYPE_ID="<<szInputFiletypeId<<endi;	
    	    
    	    //��ʼ���ʵ���
    		 strct_InitPara para;
    		 strcpy(para.service,szServiceID);
    		 strcpy(para.ListConfigID,cListConfigID);
    		 strcpy(para.listPath,szDaylyListPath);
    		 strcpy(para.szInputFiletypeId,szInputFiletypeId);
    		 strcpy(para.sourceGroup,szSourceGroupID);
    		 para.indexID=index;
    		 strcpy(para.endTime,endTime);
    		treeTool.Init(para);
    		//��ʼ���������Ϣ		
    		Ruleno_Condiction.Init(cListConfigID,szInputFiletypeId);
    		//��ʼ��list_ctl����LIST_SPECIAL_DEFINE����Ϣ		
    		list_special_ctl.Init(cListConfigID,szInputFiletypeId);
    		
    		//��ʼ��inputInfo,��Ҫ�ǹؼ�����Ϣ
    		sourceInputInfo.Init(szInputFiletypeId,endTime,szSourceGroupID);
		

	 }else{
	 	  cout<<"connect error."<<endl;
	 	  //return false;
	 }
	    conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "��ȡ�����ߺ�ʵ��ID ����" << endi;
  		throw jsexcp::CException(0, "��ȡ�����ߺ�ʵ��ID ����", __FILE__, __LINE__);
  		conn.close();
  		//return false;
   } 		
      /* try
	   	{
		 //��ʼ���ʵ���
		 strct_InitPara para;
		 strcpy(para.service,szServiceID);
		 strcpy(para.ListConfigID,cListConfigID);
		 strcpy(para.listPath,szDaylyListPath);
		 strcpy(para.szInputFiletypeId,szInputFiletypeId);
		 strcpy(para.sourceGroup,szSourceGroupID);
		 para.indexID=index;
		 strcpy(para.endTime,endTime);
		treeTool.Init(para);
		//��ʼ���������Ϣ		
		Ruleno_Condiction.Init(cListConfigID,szInputFiletypeId);
		//��ʼ��list_ctl����LIST_SPECIAL_DEFINE����Ϣ		
		list_special_ctl.Init(cListConfigID,szInputFiletypeId);
		
		//��ʼ��inputInfo,��Ҫ�ǹؼ�����Ϣ
		sourceInputInfo.Init(szInputFiletypeId,endTime,szSourceGroupID);

       	}
	 catch(CException e)
		{
		theJSLog<<e.GetErrMessage()<<endi;
		char errmsg[500];
		strcpy(errmsg,"��ʼ��ʧ�ܣ������˳�!!");
		errLog(LEVEL_ERROR,"",e.GetAppError(),errmsg,__FILE__,__LINE__,e);
		throw e;
		}  */
    	 treeTool.updateLog("RUN");
   	 inrcd.Init(szInputFiletypeId);
}

void CDaylistPlugin::execute(PacketParser& ps,ResParser& retValue) 
{
	inrcd=ps.m_inRcd;
	iTotalNum++;
	try
		{
		if(newDealFlag==1)
			{
			mergeCondictionCtl.Init(cMergeCondiction,szInputFiletypeId, inrcd);	
			withTollcodeCondictionCtl.Init(cWithTollcodeCondiction,szInputFiletypeId, inrcd);
			//����source�ĺ�����������һ��Ҫ����
			sourceMergeCond.init(szInputFiletypeId, inrcd,szSourceGroup);
			newDealFlag=0;
			}
		
		char result1[10];
	   	char result2[10];
		char sz_crr_abnormalListID[50];
		int withTollCode_flag=0;
		int iFieldCount = treeTool.data.m_iListLen;
		int iResult=0;
		CInputInfo *currInputInfo=NULL;
		
		sourceMergeCond.Operation(szSourceID,result1);
   	 	if(strcmp(mergeCondictionCtl.Operation(result2),"false")==0
	 		||strcmp(result1,"false")==0)
	       	{
		        //���μӺ���      	
		        DEBUG_LOG<<"not merged!!"<<endd;
		        iResult=0;
			iRightNum++;
	       	}
	      else      
			{	      	
			//�ɱ��ʽ�ж�Ŀ��������		
			//list_special_ctl.PrintMe();
			list_special_ctl.Operation(inrcd,sz_crr_abnormalListID);
			DEBUG_LOG<<"s_crr_abnormalListID=="<<sz_crr_abnormalListID<<endd; 
			if(sz_crr_abnormalListID[0]!=0)
				{
				//treeTool.print_list();
				//���ɹ����
				string s_crr_abnormalListID=sz_crr_abnormalListID;
				int curr_ruelno=Ruleno_Condiction.GetRuleNo(inrcd,s_crr_abnormalListID);
				DEBUG_LOG<<"curr_ruelno=="<<curr_ruelno<<endd;
				//Param.treeTool.print_list();
				/*
				cout<<"s_crr_abnormalListID="<< s_crr_abnormalListID<<endl;
				cout<<"bill_type="<< bill_type<<endl;	
				printf("curr_ruelno=%d\n",curr_ruelno);
				*/		

				if(curr_ruelno!=-1)
					{	

					if(strcmp(withTollcodeCondictionCtl.Operation(result1),"true")==0)
						withTollCode_flag=1;
					else 
						withTollCode_flag=0;
					DEBUG_LOG<<"withTollCode_flag="<<withTollCode_flag<<endd;
					DEBUG_LOG<<"to getRecordValue...."<<endd;
					//���������ݵ��ʵ�		  		
					//   checkd = clock();
					//   checktotalcd += checkd-checkc;

					//20070815
					//Param.inputInfo.getRecordValue(Oinrcd,&(Param.treeTool.data),s_crr_abnormalListID,curr_ruelno,iFieldCount,withTollCode_flag);
					currInputInfo= sourceInputInfo.getRecordValue(szSourceID,
															inrcd,
					   										&(treeTool.data),
					   										s_crr_abnormalListID,
					   										curr_ruelno,
					   										iFieldCount,
					   										withTollCode_flag);
								
					DEBUG_LOG<<"to input...."<<endd;
					//treeTool.print_list();
					//   checke = clock();
					//   checktotalde += checke-checkd;
					//���ڴ�������һ����¼			
					//Param.treeTool.Input(*currInputInfo,mapFullFlag);
					treeTool.Input(*currInputInfo);
					DEBUG_LOG<<"input ok!!"<<endd;
					iResult = 0;  		
					merge_count++;
					iOtherNum++;
					iRightNum++;
					}
				else
				    	{
				    	//�Ҳ���ƥ��Ĺ���ţ����������ϻ���
				    	iResult = 2;
					iLackNum++;
				    	strcpy(szLackind ,"-101"); //���������ϱ���
				    	retValue.setAnaResult(eLackInfo, szLackind, "�Ҳ���ƥ��Ĺ����");
				    	}
				}
			 else
			    	{
			    	//�Ҳ���ƥ���LIST_ID�����������ϻ���
			    	iResult = 2;
				iLackNum++;
			    	strcpy(szLackind,"-102");//���������ϱ���
			    	retValue.setAnaResult(eLackInfo, szLackind, "�Ҳ���ƥ���LIST_ID");
			    	}
			}
		}
	catch(CException e)
		{
		int cError_no = e.GetAppError();
	        switch (cError_no)
		        {
		          case ERR_MAINKEY_FIELD:retValue.setAnaResult(eRollback, "", e.GetErrMessage());
		          default:    break;
		        }
		throw e;
		}
}


void CDaylistPlugin::message(MessageParser&  pMessage)
{
	int IntMessageType=pMessage.getMessageType();
	char szlogmsg[500];
	try
		{
		//*******************************************************
		//��������
		//*******************************************************
		if(IntMessageType==MESSAGE_NEW_DAY)
			{
			theJSLog<<"�������ڣ��ر�ǰһ�������ļ�"<<endi;
			treeTool.updateLog("OK");
			}
		//*******************************************************
		//�����˳�
		//*******************************************************
		else if(IntMessageType==MESSAGE_PROGRAM_QUIT)
			{
			theJSLog<<"�����˳�,��������״̬Ϊ:EXIT"<<endi;
			treeTool.updateLog("EXIT");
			}
		//*******************************************************
		//�ļ���ʼ
		//*******************************************************
		else if(IntMessageType==MESSAGE_NEW_FILE)
			{
			
			strcpy(szFileName,pMessage.getFileName());
			strcpy(szSourceID,pMessage.getSourceId());

			theJSLog<<"���˲����ʼ�����ļ�: "<<szFileName<<endi;

			iTotalNum=0;
			iRightNum=0;
			iLackNum=0;
			iOtherNum=0;
			merge_count=0;		
			}
		
		//*******************************************************
		//�ļ�����
		//*******************************************************
		else if(IntMessageType==MESSAGE_END_FILE)
			{
			theJSLog<<"���˲���ļ��������:"<<szFileName<<endi;
			theJSLog<<"iTotalNum="<<iTotalNum<<",iRightNum="<<iRightNum<<",iLackNum="<<iLackNum<<",merge_count="<<iOtherNum<<endi;
			theJSLog<<"crr_merge_count="<<merge_count<<endi;
			if(merge_count>=IntTreeMaxSize)
				{
				theJSLog<<"����˵�����ʱ�ļ�..."<<endi;
				treeTool.Output();
				merge_count=0;
				}
			iTotalNum=0;
			iRightNum=0;
			iLackNum=0;
			iOtherNum=0;
			}
		
		//*******************************************************
		//���ο�ʼ
		//*******************************************************
		else if(IntMessageType==MESSAGE_NEW_BATCH)
			{
			
			theJSLog<<"���ο�ʼ..."<<endi;

			}
		
		//*******************************************************
		//���ν������ύ�ļ�
		//*******************************************************
		else if(IntMessageType==MESSAGE_END_BATCH_END_FILES)
			{
			theJSLog<< "���ν������ύ�ļ�..."<<endi;
			treeTool.Output();
			treeTool.Commit();
			merge_count=0;			     		
			}
		
		//*******************************************************
		//���ν�����Ԥ�ύ���ݿ�
		//*******************************************************
		else if(IntMessageType==MESSAGE_END_BATCH_END_DATA)
			{
			theJSLog<<"���ν������������ݿ�..."<<endi;
			}
		
		//*******************************************************
		//�����쳣�ж�
		//*******************************************************
		else if(IntMessageType==MESSAGE_BREAK_BATCH)
			{
			theJSLog<<"�����쳣�жϣ��ع�..."<<endi;
			treeTool.RollBack();
			}
		}
	catch(CException e)
		{		
		throw e;
		}

}

//int main(int argc, char **argv)
//{
//
//	/* �����������Ƿ���ȷ */
//	if (!( argc == 5))
//		{
//		/* ���������ƽ̨��� ����Դ���� ����index ���������ļ� */
//		printf("Usage : %s <service_id> <source_group> <process_index> <env_path>\n",argv[0]);
//		exit(-1);
//		}
//
//	char sourceID[10];
//	strcpy(sourceID,"GZb01");
//	char InFilePath[1000];
//	strcpy(InFilePath,"/home/zhjs/source_data/service/200X/GZb01/stat_work_path");
//	char filename[200];
//	strcpy(filename,"GZb01.00000.20100606.1228.dat.hw201006061183.txt.100");
//	char InFile[1000];
//	sprintf(InFile,"%s/%s",InFilePath,filename);
//	
//	
//	  /* ����������ж�ȡ���� */
//	char szServiceId[20];
//	char szSourceGroupId[20];
//	int iProcessId;
//	char szEnvPath[250];
//	char szEnvFile[250];
//	char szOutputFiletypeId[20];
//	char szOutrcdType[2];        
//	
//	strcpy(szServiceId, argv[1]);
//	strcpy(szSourceGroupId, argv[2]);
//	iProcessId = atoi(argv[3]);
//	strcpy(szEnvPath, argv[4]);
//	if(szEnvPath[strlen(szEnvPath)-1] != '/')
//	 	strcat(szEnvPath, "/");
//	sprintf(szEnvFile, "%szhjs.ini", szEnvPath);
//	
//
//	int ret=connectDB(szEnvFile, DBConn);
//	if(ret==0)
//		theJSLog<<"���ӳɹ�"<<endi;
//	else
//		theJSLog<<"����ʧ��"<<endi;
//
//	CBindSQL ds(DBConn);	
//		
//  	CReadIni IniFile;
//	char szLogPath[101];
//  	char szLogLevel[10];
//	char szLogStr[500];
//	  /* �ӻ��������ļ��ж�ȡ���� */
//	  if(!IniFile.init(szEnvFile))
//	  {
//		  cout<<"open ini err: "<<szEnvFile<<endl;
//		  return 0;
//	  }	
//	   /* ��ʼ����־ */
//	  IniFile.GetValue("COMMON", "log_path", szLogPath, 'N');
//	  IniFile.GetValue("COMMON", "log_level", szLogLevel, 'N');
//	  theJSLog.setLog(szLogPath, atoi(szLogLevel), szServiceId, szSourceGroupId, iProcessId);
//	  
//	  CDaylistPlugin daylist_t;
//	  daylist_t.init(szSourceGroupId, szServiceId,iProcessId);
//
//	 //�ļ�����
//	 char szSqlStr[500];
//	  sprintf(szSqlStr, "select  filetype_id from c_source_group_define where source_group='%s'", szSourceGroupId);
//	  ds.Open(szSqlStr, SELECT_QUERY );
//	  if (!(ds>>szOutputFiletypeId))
//	  	{
//	  	  strcpy(szLogStr, "select  filetype_id from c_source_group_define where source_group= :szSourceGroupId is NULL");
//		  theJSLog<<szLogStr<<endi;
//		  exit;
//	  	}
//
//	  //��¼����
//	  ds.Open("select record_type from c_filetype_define where filetype_id=:v1", SELECT_QUERY );
//	  ds<<szOutputFiletypeId;
//	  if (!(ds>>szOutrcdType))
//	  {
//		  strcpy(szLogStr, "select record_type from c_filetype_define where filetype_id= :filetype_id is NULL");
//		  theJSLog<<szLogStr<<endi;
//		  exit;
//	  }
//	  
//	  MessageParser msgParser;
//	  //int setMessage(int MessageType, char* SourceId, char* Filename);
//	  PacketParser packParser;
//	  //int setRcd(CFmt_Change &inRcd);
//	  ResParser reParser;
//	  
//	  theJSLog<<"szOutputFiletypeId="<<szOutputFiletypeId<<",szOutrcdType="<<szOutrcdType<<endi;
//	  CF_MemFileI _infile;
//	  CFmt_Change _inrcd;
//	  _infile.Init(szOutputFiletypeId);
//	  _inrcd.Init(szOutputFiletypeId, szOutrcdType[0]);
//	  _infile.Open(InFile);
//	  
//	  msgParser.setMessage(MESSAGE_NEW_BATCH,sourceID,filename);
//	  daylist_t.message(msgParser);
//	  msgParser.setMessage(MESSAGE_NEW_FILE,sourceID,filename);
//	  daylist_t.message(msgParser);
//         while(1)
//		{
//		if (_infile.readRec(_inrcd) == READ_AT_END)
//			break;
//		packParser.setRcd(_inrcd);
//		daylist_t.execute(packParser, reParser);
//		}
//	_infile.Close();
//	
//	msgParser.setMessage(MESSAGE_END_FILE,sourceID,filename);
//	daylist_t.message(msgParser);
//	msgParser.setMessage(MESSAGE_END_BATCH_END_FILES,sourceID,filename);
//	daylist_t.message(msgParser);
//	msgParser.setMessage(MESSAGE_END_BATCH_END_DATA,sourceID,filename);
//	daylist_t.message(msgParser);
//	DBConn.Commit();
//	theJSLog<<"�����������"<<endi;
//	
//}


