
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
	theJSLog<< "统计插件初始化... "<<endi;
	theJSLog<<"szSourceGroupID="<<szSourceGroupID<<",szServiceID="<<szServiceID<<endi;		
	//获取config_id
	DBConnection conn;//数据库连接
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
    		//没有配置
    		sprintf(errmsg,"%s,sql:%s","C_PROCESS_ENV没有配置STAT_CONFIG_ID变量!!",sql);
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
    		//没有配置
    		sprintf(errmsg,"%s,sql:%s","C_PROCESS_ENV没有配置STAT_MAXNUM_UPATE变量!!",sql);
    		theJSLog<<errmsg<<endi;
    		statMaxUpdate=STAT_MAXNUM_UPATE;
    		}
    	else
    		statMaxUpdate=atoi(szStatMaxUpdate);
    	theJSLog<<"STAT_MAXNUM_UPATE="<<statMaxUpdate<<endi;	
    	
    
    	//查询工作流模板ID（workflow_id）
    	char szWorkflowId[20];
    	sprintf(sql, "select workflow_id from c_source_group_define where source_group='%s'", szSourceGroupID);
    	stmt.setSQLString(sql);		
			stmt.execute();
    	if(!(stmt>>szWorkflowId))
    		{
    		sprintf(errmsg,"没有找到配置信息:%s!!",sql);
    		errLog(LEVEL_ERROR,"",ERR_SELECT,errmsg,__FILE__,__LINE__);
    		throw CException(ERR_SELECT,errmsg,__FILE__,__LINE__);
    		}
    
    	//查询log_tabname
    	sprintf(sql, "select a.log_tabname from c_service_interface a, c_service_flow b \
    			where a.interface_id=b.input_id and b.workflow_id='%s' and b.service='%s'", szWorkflowId, szServiceID);
    	stmt.setSQLString(sql);		
			stmt.execute();
    	if(!(stmt>>szLogTableName))
    		 {
    		sprintf(errmsg,"没有找到配置信息:%s!!",sql);
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
    		strcpy(errmsg,"初始化失败!!");
    		errLog(LEVEL_ERROR,"",e.GetAppError(),errmsg,__FILE__,__LINE__,e);
    		throw e;
    		}
    	 		//获取入口话单格式	
    	 	char szInputFiletypeId[100];
    	 	sprintf(sql,"select filetype_id from C_SOURCE_GROUP_DEFINE where Source_Group= '%s'",szSourceGroupID);
        //ds.Open("select filetype_id from C_SOURCE_GROUP_DEFINE where Source_Group= :Source_Group", SELECT_QUERY );
        stmt.setSQLString(sql);		
			  stmt.execute();
        if (!(stmt>>szInputFiletypeId))
        {
        		strcpy(errmsg, "数据源组统一格式在C_SOURCE_GROUP_DEFINE未定义");
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
  		theJSLog << "统计插件初始化 出错" << endi;
  		throw jsexcp::CException(0, "统计插件初始化 出错", __FILE__, __LINE__);
  		conn.close();
  		//return false;
  } 

	
}

void CStatPlugin::execute(PacketParser& ps,ResParser& retValue) 
{
	//inrcd=ps.m_inRcd;
	DBConnection conn;//数据库连接
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
		strcpy(errmsg,"处理单条话单失败!!");
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
		//文件开始
		//*******************************************************
		if(IntMessageType==MESSAGE_NEW_FILE)
			{
			strcpy(szFileName,pMessage.getFileName());
			theJSLog<<"统计插件开始处理文件: "<<szFileName<<endi;
			
			strcpy(szSourceID,pMessage.getSourceId());
			//char dealtime[15+1];	    	
    			//getCurTime(dealtime);  
			//cStat.setFileName(szFileName,szSourceID,dealtime);		
			//cStat.Set_TempOutFile();
			firstRcdFlag=1;
			}
		
		//*******************************************************
		//文件结束
		//*******************************************************
		else if(IntMessageType==MESSAGE_END_FILE)
			{
			cStat.update_commit();
			cStat.Close_Temp_Outfile();
			runLog(LEVEL_INFO, "统计插件文件处理结束");
			}
		
		//*******************************************************
		//批次开始
		//*******************************************************
		else if(IntMessageType==MESSAGE_NEW_BATCH)
			{
			runLog(LEVEL_INFO, "批次开始...");
			}
		
		//*******************************************************
		//批次结束，提交文件
		//*******************************************************
		else if(IntMessageType==MESSAGE_END_BATCH_END_FILES)
			{
			runLog(LEVEL_INFO, "批次结束，提交文件...");   
			cStat.Rename_AllOutFile();
			}
		
		//*******************************************************
		//批次结束，预提交数据库
		//*******************************************************
		else if(IntMessageType==MESSAGE_END_BATCH_END_DATA)
			{
			runLog(LEVEL_INFO, "批次结束，更新数据库...");
			}
		
		//*******************************************************
		//批次异常中断
		//*******************************************************
		else if(IntMessageType==MESSAGE_BREAK_BATCH)
			{
			runLog(LEVEL_INFO, "批次异常中断，回滚...");
			cStat.rollback();
			}
		}
	catch(CException e)
		{
		throw e;
		}

}



