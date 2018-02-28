
#include "CStatPlugin.h"
using namespace std;
using namespace tpss;

extern int G_CF_CStat_UpdateCount;
CStatPlugin::CStatPlugin()
{
	IntStatRcdCout=0;
	Today[0]=0;
	Group_ID=0;
	firstRcdFlag=1;
}

CStatPlugin::~CStatPlugin()
{
}

void CStatPlugin::init(char *szSourceGroupID, char *szServiceID, int index)
{
	theJSLog<< "ͳ�Ʋ����ʼ��... "<<endi;
	theJSLog<<"szSourceGroupID="<<szSourceGroupID<<",szServiceID="<<szServiceID<<endi;		
	//��ȡconfig_id
	DBConnection conn;//���ݿ�����
  try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			char errmsg[500];
    	char configid[200];
    	configid[0]=0;
    	char sql[500];
    	sprintf(sql,"select  a.VAR_VALUE  from C_PROCESS_ENV a where a.SOURCE_GROUP='%s' and a.SERVICE='%s' and a.VARNAME='STAT_CONFIG_ID'",
    		szSourceGroupID, szServiceID);
    	stmt.setSQLString(sql);		
			stmt.execute();

    	if(!(stmt>>configid))
    		{
    		//û������
    		sprintf(errmsg,"%s,sql:%s","C_PROCESS_ENVû������STAT_CONFIG_ID����!!",sql);
    		errLog(LEVEL_ERROR,"",ERR_SELECT,errmsg,__FILE__,__LINE__);
    		throw CException(ERR_SELECT,errmsg,__FILE__,__LINE__);
    		}
    	theJSLog<<"STAT_CONFIG_ID="<<configid<<endi;		
    
    	char szStatMaxUpdate[10];
    	sprintf(sql,"select  a.VAR_VALUE  from C_PROCESS_ENV a where a.SOURCE_GROUP='%s' and a.SERVICE='%s' and a.VARNAME='STAT_MAXNUM_UPATE'",
    		szSourceGroupID, szServiceID);
    	stmt.setSQLString(sql);		
			stmt.execute();
    	if(!(stmt>>szStatMaxUpdate))
    		{
    		//û������
    		sprintf(errmsg,"%s,sql:%s","C_PROCESS_ENVû������STAT_MAXNUM_UPATE����!!",sql);
    		theJSLog<<errmsg<<endi;
    		statMaxUpdate=STAT_MAXNUM_UPATE;
    		}
    	else
    		statMaxUpdate=atoi(szStatMaxUpdate);
    	theJSLog<<"STAT_MAXNUM_UPATE="<<statMaxUpdate<<endi;	
    	
    
    	//��ѯ������ģ��ID��workflow_id��
    	char szWorkflowId[20];
    	sprintf(sql, "select workflow_id from c_source_group_define where source_group='%s'", szSourceGroupID);
    	stmt.setSQLString(sql);		
			stmt.execute();
    	if(!(stmt>>szWorkflowId))
    		{
    		sprintf(errmsg,"û���ҵ�������Ϣ:%s!!",sql);
    		errLog(LEVEL_ERROR,"",ERR_SELECT,errmsg,__FILE__,__LINE__);
    		throw CException(ERR_SELECT,errmsg,__FILE__,__LINE__);
    		}
    
    	//��ѯlog_tabname
    	sprintf(sql, "select a.log_tabname from c_service_interface a, c_service_flow b \
    			where a.interface_id=b.input_id and b.workflow_id='%s' and b.service='%s'", szWorkflowId, szServiceID);
    	stmt.setSQLString(sql);		
			stmt.execute();
    	if(!(stmt>>szLogTableName))
    		 {
    		sprintf(errmsg,"û���ҵ�������Ϣ:%s!!",sql);
    		errLog(LEVEL_ERROR,"",ERR_SELECT,errmsg,__FILE__,__LINE__);
    		throw CException(ERR_SELECT,errmsg,__FILE__,__LINE__);
    		}
    	theJSLog<<"szLogTableName="<<szLogTableName<<endi;		
    	  
           try
    	   	{
    		//strcpy(configid,"-13021");
    		theJSLog<<"cStat.Init( configid );"<<endd;
    		cStat.Init( configid );
    		
           	}
    	 catch(CException e)
    		{	
    		strcpy(errmsg,"��ʼ��ʧ��!!");
    		errLog(LEVEL_ERROR,"",e.GetAppError(),errmsg,__FILE__,__LINE__,e);
    		throw e;
    		}
    	 		//��ȡ��ڻ�����ʽ	
    	 	char szInputFiletypeId[100];
    	 	sprintf(sql,"select filetype_id from C_SOURCE_GROUP_DEFINE where Source_Group= '%s'",szSourceGroupID);
        //ds.Open("select filetype_id from C_SOURCE_GROUP_DEFINE where Source_Group= :Source_Group", SELECT_QUERY );
        stmt.setSQLString(sql);		
			  stmt.execute();
        if (!(stmt>>szInputFiletypeId))
        {
        		strcpy(errmsg, "����Դ��ͳһ��ʽ��C_SOURCE_GROUP_DEFINEδ����");
    				CException e(ERR_SELECT, errmsg, __FILE__, __LINE__);
    				throw e;
        }  	
	
	 }else{
	 	   cout<<"connect error."<<endl;
	 	   //return false;
	 }
	     conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "ͳ�Ʋ����ʼ�� ����" << endi;
  		throw jsexcp::CException(0, "ͳ�Ʋ����ʼ�� ����", __FILE__, __LINE__);
  		conn.close();
  		//return false;
  } 

	
}

void CStatPlugin::execute(PacketParser& ps,ResParser& retValue) 
{
	//inrcd=ps.m_inRcd;
	DBConnection conn;//���ݿ�����
	try
		{
			if (dbConnect(conn))
  	 {
  			Statement stmt = conn.createStatement();
  			if(firstRcdFlag==1)
  			{
  			char dealstarttime[15]; 			
  			char sql[500];
  			sprintf(sql,"select nvl(a.DEALSTARTTIME,'0')  from %s a where a.SOURCE_ID=:v1 and a.FILENAME=:v2",szLogTableName);
  			stmt.setSQLString(sql);
			  stmt << szSourceID<<szFileName;
			  stmt.execute();
			  stmt >> dealstarttime;

  			if(strcmp(dealstarttime,"0")==0)
  				{
  				char errmsg[500];
  				sprintf(errmsg,"szSourceID=%s,szFileName=%s,sql:%s",szSourceID,szFileName,sql);
  				errLog(LEVEL_ERROR,szFileName,ERR_DEAL_RECORD,errmsg,__FILE__,__LINE__);
  					theJSLog<<"DEALSTARTTIME="<<dealstarttime<<",check "<<szLogTableName<<ende;
  					exit(1);
  				}
  			cStat.setFileName(szFileName,szSourceID,dealstarttime);
  			cStat.Set_TempOutFile();
  			theJSLog<<"DEALSTARTTIME: "<<dealstarttime<<endi;
  			firstRcdFlag=0;
  			}
  		
  		cStat.dealRedoRec(ps.m_inRcd);
  		IntStatRcdCout++;
  		//theJSLog<<"IntStatRcdCout="<<IntStatRcdCout<<endi;
  		//if(IntStatRcdCout==statMaxUpdate)
  		if(G_CF_CStat_UpdateCount==statMaxUpdate)
  			{
  			cStat.update_commit();
  			IntStatRcdCout=0;
  			}
  		
  	 }else{
  	 	   cout<<"connect error."<<endl;
  	 	  // return false;
  	 }
  	     conn.close();	     	     
		}
	catch(CException e)
		{
		 int cError_no = e.GetAppError();
	        switch (cError_no)
		        {
		          case ERR_FIELD_NULL:
		          case ERR_GET_SUBSTR:retValue.setAnaResult(eRollback, "", e.GetErrMessage());
		          default:    break;
		        }
		char errmsg[500];	
		strcpy(errmsg,"����������ʧ��!!");
		e.PushStack(ERR_DEAL_RECORD,errmsg, __FILE__,__LINE__);	
		throw e;
		}
}


void CStatPlugin::message(MessageParser&  pMessage)
{
	int IntMessageType=pMessage.getMessageType();
	char szlogmsg[500];
	try
		{
		//*******************************************************
		//�ļ���ʼ
		//*******************************************************
		if(IntMessageType==MESSAGE_NEW_FILE)
			{
			strcpy(szFileName,pMessage.getFileName());
			theJSLog<<"ͳ�Ʋ����ʼ�����ļ�: "<<szFileName<<endi;
			
			strcpy(szSourceID,pMessage.getSourceId());
			//char dealtime[15+1];	    	
    			//getCurTime(dealtime);  
			//cStat.setFileName(szFileName,szSourceID,dealtime);		
			//cStat.Set_TempOutFile();
			firstRcdFlag=1;
			}
		
		//*******************************************************
		//�ļ�����
		//*******************************************************
		else if(IntMessageType==MESSAGE_END_FILE)
			{
			cStat.update_commit();
			cStat.Close_Temp_Outfile();
			runLog(LEVEL_INFO, "ͳ�Ʋ���ļ��������");
			}
		
		//*******************************************************
		//���ο�ʼ
		//*******************************************************
		else if(IntMessageType==MESSAGE_NEW_BATCH)
			{
			runLog(LEVEL_INFO, "���ο�ʼ...");
			}
		
		//*******************************************************
		//���ν������ύ�ļ�
		//*******************************************************
		else if(IntMessageType==MESSAGE_END_BATCH_END_FILES)
			{
			runLog(LEVEL_INFO, "���ν������ύ�ļ�...");   
			cStat.Rename_AllOutFile();
			}
		
		//*******************************************************
		//���ν�����Ԥ�ύ���ݿ�
		//*******************************************************
		else if(IntMessageType==MESSAGE_END_BATCH_END_DATA)
			{
			runLog(LEVEL_INFO, "���ν������������ݿ�...");
			}
		
		//*******************************************************
		//�����쳣�ж�
		//*******************************************************
		else if(IntMessageType==MESSAGE_BREAK_BATCH)
			{
			runLog(LEVEL_INFO, "�����쳣�жϣ��ع�...");
			cStat.rollback();
			}
		}
	catch(CException e)
		{
		throw e;
		}

}



