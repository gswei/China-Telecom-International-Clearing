#include "Mainflow.h"
//2014-03-18  增加对表统计功能,增加对5分钟定时的处理

void main(int argc, char* argv[])
{
	system("date");
	cout<<"********************************************** "<<endl;
	cout<<"*    China Telecom. Telephone Network    * "<<endl;
	cout<<"*        International Account Settle System       * "<<endl;
	cout<<"*                                            * "<<endl;
	cout<<"*                    infopoint                 * "<<endl;
	cout<<"*                  sys.GJZW.Version 1.1	            * "<<endl;
	cout<<"*           created time : 2014-03-20 by  hedong	* "<<endl;
	cout<<"********************************************** "<<endl;

	if (argc==3||argc==5) 
	{		
  }
  else
  {
  	expTrace("Y",__FILE__,__LINE__,	"Usage : %s <server_id> <env_path>",argv[0]);
		expTrace("Y",__FILE__,__LINE__,	"or Usage : %s <server_id> <begin_time> <end_time> <env_path>",argv[0]);
		//expTrace("Y",__FILE__,__LINE__,	"      eg : %s -Z ../env",argv[0]);
		expTrace("Y",__FILE__,__LINE__,	"      eg : %s SERV1 ../env",argv[0]);
		expTrace("Y",__FILE__,__LINE__,	"      eg : %s SERV1 201202010900 201202020800 ../env",argv[0]);
		exit(-1);
  }
	
  
	

	char szDbName[TABLENAME_LEN+1];			//数据库名
	char szUserName[USER_NAME_LEN+1];		//用户名（解密前）
	char szTrueName[USER_NAME_LEN+1];		//用户名（解密后）
	char szUserPass[USER_PASS_LEN+1];		//密码（解密前）
	char szTruePass[USER_PASS_LEN+1];		//密码（解密后）
	char szDebugDir[LOG_MSG_LEN+1];	

	char szEnvPath[300];
	char szEnvFile[300];
	char szEnvName[50];
	char szEnvValue[200];
	char szService[10];
	char sszInfoPoint[50];
	char sszAuditId[20];
	char sszTypeId[20];
	char sszRate[6];
	char sszInfoDesc[150];
	char sszDealType[5];
	char sszStartTime[16];
	char sszEndTime[16];
	char szTempStartTime[16];
	char szTempEndTime[16];
	char szChStartDate[16];
	char szChEndDate[16];
	char szLastDate[16];
	char szTmpDt[16];
	char szTFrom[20];
	char szTTo[20];
	char szProMode[3];
	char szTempTo[20];
	int iRFlag;
	int  iiDelay; 
	int iMaxRec;
	int iBatchCount;
	int kk;
	
	strcpy(szProMode,"N"); //正常处理，所有信息点从上一次结束时间开始
	
	
	if (5==argc)  //五个参数，包括时间
	{
		strcpy(szProMode,"T"); //时间段内的信息点处理
		strcpy(szService, argv[1]);	
	  strcpy(szEnvPath, argv[4]);	
	  if (szEnvPath[strlen(szEnvPath) - 1] != '/')
  	{
		  strcat(szEnvPath, "/");
	  }
	  strcpy(szChStartDate, argv[2]);
	  strcpy(szChEndDate, argv[3]);
	  if(strlen(szChStartDate)!=12||strlen(szChEndDate)!=12)// 输入时间是12位的到分钟,exp by hlp  
	  {
	  		expTrace("Y",__FILE__,__LINE__,	"The input Date must be YYYYMMDDHHMM");
	  		exit(-1);
	  }
	  else //下面就是去规整时间了，开始结束时间都是15的整数倍，指分钟，exp by hlp
	  {
	  	szTmpDt[0]=szChStartDate[10];
	  	szTmpDt[1]=szChStartDate[11];
	  	szTmpDt[2]=0;
	  	
	  	kk=atoi(szTmpDt);
	  	if(kk<15)
	  	{
	  		strcpy(szTmpDt,"00");
	  	}
	  	else if (kk<30)
	  	{
	  		strcpy(szTmpDt,"15");
	  	}
	  	else if (kk<45)
	  	{
	  		strcpy(szTmpDt,"30");
	  	}
	  	else 
	  	{
	  		strcpy(szTmpDt,"45");
	  	}
	  	szChStartDate[10]=szTmpDt[0];
	  	szChStartDate[11]=szTmpDt[1];
	  	
	  	
	  	szTmpDt[0]=szChEndDate[10];
	  	szTmpDt[1]=szChEndDate[11];
	  	szTmpDt[2]=0;
	  	
	  	kk=atoi(szTmpDt);
	  	if(kk<=15)
	  	{
	  		strcpy(szTmpDt,"15");
	  	}
	  	else if (kk<=30)
	  	{
	  		strcpy(szTmpDt,"30");
	  	}
	  	else if (kk<=45)
	  	{
	  		strcpy(szTmpDt,"45");
	  	}
	  	
	  	szChEndDate[10]=szTmpDt[0];
	  	szChEndDate[11]=szTmpDt[1];	  	
	  }
	}
	else  
	{
		
	  strcpy(szService, argv[1]);			
	  strcpy(szEnvPath, argv[2]);	
	  if (szEnvPath[strlen(szEnvPath) - 1] != '/')
  	{
		  strcat(szEnvPath, "/");
	  }
	}
	
	
	initDaemon(szEnvPath);    	
	sprintf(szEnvFile, "%s%s", szEnvPath, ENV_FILE_NAME);
	wrlog_setEnvPath(szEnvFile);

	char szInfoDetect[200];
	
	/*读取日志文件根路径*/
	char szLog_Dir[PATH_NAME_LEN+1];
	
	getEnv(szEnvFile, "LOG_DIR", szLog_Dir);
	
	if(wrlog_setEnvPath(szEnvFile)!=0)//设置环境变量文件，读取错误日志路径;
  {
  	printf("setEnvPath err!\n");
  }
	
	setLogPath(szLog_Dir, "infoMgr");
	
	getEnv(szEnvFile, "INFOPOINT_DIR", szInfoDetect);

	/*初始化日志*/
	char szCurDate[8+1];
	getCurDate(szCurDate);
	strcpy(szLastDate,szCurDate);
	char szLogFile[FILE_NAME_LEN+1];
	
	
	//*********************************************************************
	// 读取环境变量
	//*********************************************************************
	char szTemp[300];
	char sz_errtime[16];

	getEnv(szEnvFile, DBSERVER_ENV_NAME, szDbName);
	getEnv(szEnvFile, DBUSER_ENV_NAME, szUserName);
	getEnv(szEnvFile, DBPASS_ENV_NAME, szUserPass);
	
	//解密用户名、密码
	CEncryptAsc ce;
	ce.Decrypt(szUserName, szTrueName);
	ce.Decrypt(szUserPass, szTruePass);
	char szLogStr[100];
	
	sprintf(szLogFile,"__.020.%s.infopointMgr.%s.log", szCurDate,szService);
	if(theLog.Open(szLogFile)<0) 
	{
	   	cout<<"open run_log file :"<<szLogFile<<".errno="<<errno<<endl;	
	    exit(-1);
	}

	try
	{
		//*********************************************************************
		// 连接数据库
		//*********************************************************************
		expTrace("Y", __FILE__, __LINE__, "szDbName=%s;", szDbName);
		expTrace("Y", __FILE__, __LINE__, "userName=%s;", szTrueName);
		expTrace("Y", __FILE__, __LINE__,	"Connect to database...");
		
		
		try
		{
			DBConn.Connect(szTrueName, szTruePass, szDbName);			
			 //dbconn_2.Connect("ctjf","ctjf123","zhjs");
			//dbconn_1.Connect("cjcl", "cjcl", "zhjf_view");// 在此连接多个子数据库连接
			expTrace("Y", __FILE__, __LINE__,
				"Connect to database successfully.");
		}
		catch (...)
		{			
			strcpy(szLogStr, "Connect to database failed!");
			getCurTime(sz_errtime);
      wrlog("infomgr","0","",'E','H',1001,002,sz_errtime,szLogStr,__FILE__,__LINE__);			
			exit(1);
		}
		
		CBindSQL ds(DBConn);
		char szSql[300];
		int i;
		
		/*if (szProMode[0]=='Z')//信息点综合
		{
			
			CInfoCom infoCom;
			sprintf(szSql, "SELECT AUDITID,TYPEID,DELAY from INFOPOINT_PUB_DEFINE ");
		  ds.Open(szSql);
		  while (ds>>sszAuditId>>sszTypeId>>iiDelay)
		  {	
		  	iMaxRec=iiDelay;//综合文件时，需要综合文件的个数，目前是3,3个系统，3个end文件
				if (0!=infoCom.Init(iMaxRec))
		    {
			     strcpy(szLogStr, "error in init infoCom!");
			     getCurTime(sz_errtime);
           wrlog("infomgr","0","",'E','H',1002,002,sz_errtime,szLogStr,__FILE__,__LINE__);	
			     exit(1);
		    }
		    else
		    {
		    	 expTrace("Y", __FILE__, __LINE__, "init infocom success;");
		    }
		  }
		  ds.Close();
		  
		  while(1)
		  {
		  	getCurDate(szCurDate);
        sprintf(szLogFile,"__.020.%s.infopointMgr.%s.log", szCurDate,szService);
	      if(theLog.Open(szLogFile)<0) 
	      {
	   	     cout<<"open run_log file :"<<szLogFile<<".errno="<<errno<<endl;	
	         exit(-1);
	      }
		  	kk=0;
		  	GetRunFlag(iRFlag,"Z");
		  	if (0==iRFlag)
		  	{
		  		expTrace("Y", __FILE__, __LINE__, "RUN_FLAG IS N, quit out!");
		  		break;
		  	}
		  	
		    if(0==infoCom.Check())
			  {
			  	kk=1;
				  infoCom.Infocom();
			  }
			 
			  if (0==kk)
			  {
			 	  expTrace("Y", __FILE__, __LINE__, "wait for 60 seconds!");
			    sleep(60);			 	
			  }			  
		  }
			
		
		}*/
	if (szProMode[0]=='T')
		{
			 //进入业务处理
		   //初始化
		   CInfoDeal infoDeal;	
		   		
		   if (0!=infoDeal.Init(szInfoDetect,szService))
		   {
			   strcpy(szLogStr, "error in init infoDeal!");
			   getCurTime(sz_errtime);
               wrlog("infomgr","0","",'E','H',1002,002,sz_errtime,szLogStr,__FILE__,__LINE__);			
			   exit(1);
		   }
		  
			 sprintf(szSql, "SELECT AuditId,TypeId, INFOPOINT_DESC,DEAL_TYPE,STARTTIME,DELAY,SRATE from INFOPOINT_PUB_DEFINE where VALID =1  order by STARTTIME");
		   ds.Open(szSql);
		   while (ds>>sszInfoPoint>>sszTypeId>>sszInfoDesc>>sszDealType>>sszStartTime>>iiDelay>>sszRate)
		   {
			   SInfo ss;
			   
			   strcpy(ss.szInfoPoint,sszInfoPoint);
			   strcpy(ss.szTypeId,sszTypeId);
			   strcpy(ss.szInfoDesc,sszInfoDesc);
			   strcpy(ss.szDealType,sszDealType);
			   strcpy(ss.szRate,sszRate);
			   strcpy(ss.szStartTime,sszStartTime);
			   ss.iDelay=iiDelay;
			   expTrace("Y", __FILE__, __LINE__, "infopoint:%s;", sszInfoPoint);
			   //ss.cTC.init(sszStartTime,iiDelay);
			   g_sInfo.push_back(ss);			
		   }
		   ds.Close();
		   
		   GetRunFlag(iRFlag,szService);
		   if (0==iRFlag)
		   {
		  	 expTrace("Y", __FILE__, __LINE__, "RUN_FLAG IS N, quit out!");
		  	 exit(0);
		   }
		   
		     strcpy(szTempStartTime, szChStartDate);			 
			 strcpy(szTempEndTime, szChEndDate);			
			 strcpy(szTempTo,"0");
			 
	     while(1)
	     {
	   	   setLogPath(szLog_Dir, "infoMgr");	   	   
	   	   addmin(szTempStartTime,15,szTmpDt);
	   	   expTrace("Y", __FILE__, __LINE__, "check time point:%s",szTmpDt);
	   	   if (strcmp(szTmpDt,szTempEndTime)>0)//某时间段内补生成文件,改时间段生成完会自动退出程序
	   	   {
	   	  	  expTrace("Y", __FILE__, __LINE__, "All Infopoint finished!");
	   	  	  break;
	   	   }
	   	   kk=0;
			   for(i=0;i<g_sInfo.size(); i++)
			   {
					  if(CheckLock(g_sInfo[i].szStartTime,g_sInfo[i].iDelay,szTmpDt,szTFrom,szTTo)==0)
					  {	
					  	if (strcmp(szTempTo,"0")==0)
					    {
						    strcpy(szTempTo,szTTo);	
					    }
					    else if(strcmp(szTempTo,szTTo)!=0)
					    {
					    	expTrace("Y", __FILE__, __LINE__, "the end date is %s",szTempTo);
						   // infoDeal.SeqSuc(szTempTo);						
						    strcpy(szTempTo,szTTo);	
					    }
						  kk=1;			
					    sprintf(szLogStr, "信息点：%s:%s统计启动，时间%s--%s,生成频率:%d分钟",g_sInfo[i].szInfoPoint,g_sInfo[i].szInfoDesc,szTFrom,szTTo,g_sInfo[i].iDelay);
		          expTrace("Y", __FILE__, __LINE__, szLogStr);		      
		          try
		          {
		      	    infoDeal.SetDate(szTFrom,szTTo);
		      	    if (infoDeal.GetBatch(g_sInfo[i].szInfoPoint,g_sInfo[i].szTypeId,g_sInfo[i].szRate)==-1)
		      	    {
		      	    	strcpy(szLogStr, "get batch error!");
		      	      getCurTime(sz_errtime);
                  wrlog("infomgr","0","",'E','H',1003,002,sz_errtime,szLogStr,__FILE__,__LINE__);	
                  exit(0);
		      	    }		
		    
		      	    expTrace("Y", __FILE__, __LINE__, "szDealType is %s ",g_sInfo[i].szDealType);
		      	    infoDeal.WriteFile(g_sInfo[i].szDealType,szService,szTFrom,szTTo);
				    strcpy(szLogStr, "信息点统计完成!");
		            expTrace("Y", __FILE__, __LINE__, szLogStr);
		          }
		          catch(...)
		          {
		      	    strcpy(szLogStr, "信息点获取错误!");
		      	    getCurTime(sz_errtime);
                    wrlog("infomgr","0","",'E','H',1003,002,sz_errtime,szLogStr,__FILE__,__LINE__);			        
		          }
					  }
				  				
			    }
			  
			    strcpy(szTempStartTime,szTmpDt);			  
			    if (1==kk)
			    {
			    	 expTrace("Y", __FILE__, __LINE__, "the end date is %s",szTempTo);
				     //infoDeal.SeqSuc(szTempTo);
			    }
			    	
			    GetRunFlag(iRFlag,szService);
		      if (0==iRFlag)
		      {
		  	    expTrace("Y", __FILE__, __LINE__, "RUN_FLAG IS N, quit out!");
		  	    exit(0);
		      }		 
		   }			
		}
	////////////////////////////////////////////////////////
		else if (szProMode[0]=='N')
		{			
			char szCurTime[20];
			char szChkTime[20];
			sprintf(szSql, "SELECT  ENV_VALUE from  INFO_PUB_ENV where ENV_NAME='%s' AND SERVER_ID='%s'","IS_PRO_LAST",szService);
		  ds.Open(szSql);
		  while(ds>>szProMode)
		  {
		  }
		  ds.Close();
		  
		  //获取批次的最大时间点，和业务类型无关
		 /* sprintf(szSql, "SELECT  max(ENDTIME),count(*) from  INFO_PUB_BATCH ");//需要修改,加上szService的条件,在语音ab机上，会出问题
		  ds.Open(szSql);
		  while(ds>>szTempStartTime>>iBatchCount)
		  {
		  }
		  ds.Close(); */

		   //获取批次的最大时间点，和业务类型无关
		   
		  sprintf(szSql, "SELECT  max(STARTTIME),count(*) from  INFO_PUB_BATCH ");//需要修改,加上szService的条件,在语音ab机上，会出问题
		  ds.Open(szSql);
		  while(ds>>szTempStartTime>>iBatchCount)
		  {
		    // expTrace("Y", __FILE__, __LINE__, "szTempStartTime is %s ",szTempStartTime);
		     addmin(szTempStartTime,15,szTempStartTime);
		  }
		  ds.Close();
          //expTrace("Y", __FILE__, __LINE__, "szTempStartTime is %s ",szTempStartTime);
		  
		  ///////////////////////////////////////
		  //expTrace("Y", __FILE__, __LINE__, "max(ENDTIME) is %s",szTempStartTime);
		  
		  CInfoDeal infoDeal;		
		
		  if (0!=infoDeal.Init(szInfoDetect,szService))
		  {
			   strcpy(szLogStr, "error in init infoDeal!");
			   getCurTime(sz_errtime);
               wrlog("infomgr","0","",'E','H',1002,002,sz_errtime,szLogStr,__FILE__,__LINE__);			
			   exit(1);
		  }
		  
		  getCurTime(szCurTime);
		  strcpy(szChkTime,szCurTime);
		  szChkTime[12]=0;
		  ////////////////////////////////////////////////
		 // expTrace("Y", __FILE__, __LINE__, "CurTime and szChkTime is %s",szChkTime);
		  
		  sprintf(szSql, "SELECT AUDITID,TYPEID, INFOPOINT_DESC,DEAL_TYPE,STARTTIME,DELAY,SRATE from INFOPOINT_PUB_DEFINE where VALID =1  order by STARTTIME");
		  ds.Open(szSql);
		  while (ds>>sszInfoPoint>>sszTypeId>>sszInfoDesc>>sszDealType>>sszStartTime>>iiDelay>>sszRate)
		  {
			   SInfo ss;
			   strcpy(ss.szTypeId,sszTypeId);
			   strcpy(ss.szInfoPoint,sszInfoPoint);
			   strcpy(ss.szInfoDesc,sszInfoDesc);
			   strcpy(ss.szDealType,sszDealType);
			   strcpy(ss.szRate,sszRate);
			   strcpy(ss.szStartTime,sszStartTime);
			   ss.iDelay=iiDelay;
			   expTrace("Y", __FILE__, __LINE__, "infopoint:%s;", sszInfoPoint);
			   ss.cTC.init(sszStartTime,iiDelay);
			   g_sInfo.push_back(ss);			
		  }
		  ds.Close();
		   
		  GetRunFlag(iRFlag,szService);
		  if (0==iRFlag)
		  {
		  	 expTrace("Y", __FILE__, __LINE__, "RUN_FLAG IS N, quit out!");
		  	 exit(0);
		  }
		    
		 		  
		  for(int j=0;j<g_sInfo.size(); j++)
			{
				g_sInfo[j].cTC.init(g_sInfo[j].szStartTime,g_sInfo[j].iDelay);
			}
		  		  
		  szTmpDt[0]=szChkTime[10];
	  	szTmpDt[1]=szChkTime[11];
	  	szTmpDt[2]=0;

	  	kk=atoi(szTmpDt);	  
	  	
	  	if(kk<15)
	  	{
	  		strcpy(szTmpDt,"00");
	  	}
	  	else if (kk<30)
	  	{
	  		strcpy(szTmpDt,"15");
	  	}
	  	else if (kk<45)
	  	{
	  		strcpy(szTmpDt,"30");
	  	}
	  	else 
	  	{
	  		strcpy(szTmpDt,"45");
	  	}
	  	szChkTime[10]=szTmpDt[0];
	  	szChkTime[11]=szTmpDt[1];
		  
		  
			strcpy(szTempEndTime, szChkTime);	
			strcpy(szTempTo,"0");

			/////////////////////////////
	  	//expTrace("Y", __FILE__, __LINE__, "szTempEndTime and szTempTo %s and %s",szTempEndTime,szTempTo);
			 
		if (iBatchCount>0)
		{
	    while(1)
	    {
	   	   addmin(szTempStartTime,15,szTmpDt);
	   	   	/////////////////////////////
	  	//expTrace("Y", __FILE__, __LINE__, "szTempStartTime and szTmpDt %s and %s",szTempStartTime,szTmpDt);
	   	   if (strcmp(szTmpDt,szTempEndTime)>0)
	   	   {
	   	  	  expTrace("Y", __FILE__, __LINE__, "All Infopoint finished!");
	   	  	  break;
	   	   }
	   	   kk=0;
			   for(i=0;i<g_sInfo.size(); i++)
			   {
					  if(CheckLock(g_sInfo[i].szStartTime,g_sInfo[i].iDelay,szTmpDt,szTFrom,szTTo)==0)
					  {	
						  kk=1;	
						   	   	/////////////////////////////
	  	//expTrace("Y", __FILE__, __LINE__, "szTFrom and szTTo %s and %s",szTFrom,szTTo);
						  if (strcmp(szTempTo,"0")==0)
					    {
						    strcpy(szTempTo,szTTo);	
					    }
					    else if(strcmp(szTempTo,szTTo)!=0)
					    {
					    	expTrace("Y", __FILE__, __LINE__, "the end date is %s",szTempTo);
						   // infoDeal.SeqSuc(szTempTo);						
						    strcpy(szTempTo,szTTo);	
					    }
					
					    sprintf(szLogStr, "信息点：%s:%s统计启动，时间%s--%s,生成频率:%d分钟",g_sInfo[i].szInfoPoint,g_sInfo[i].szInfoDesc,szTFrom,szTTo,g_sInfo[i].iDelay);
		          expTrace("Y", __FILE__, __LINE__, szLogStr);		      
		          try
		          {
		      	    infoDeal.SetDate(szTFrom,szTTo);
		      	    /////////////////////////////
	  	          //expTrace("Y", __FILE__, __LINE__, "szTFrom and szTTo %s and %s",szTFrom,szTTo);
		      	    if (infoDeal.GetBatch(g_sInfo[i].szInfoPoint,g_sInfo[i].szTypeId,g_sInfo[i].szRate)==-1)
		      	    {
		      	      strcpy(szLogStr, "get batch error!");
		      	      getCurTime(sz_errtime);
                      wrlog("infomgr","0","",'E','H',1003,002,sz_errtime,szLogStr,__FILE__,__LINE__);	
                      exit(0);
		      	    }		
		      	    if( szProMode[0]=='0'&& strcmp(g_sInfo[i].szTypeId,"60")==0)  //进程信息点从当前时间开始
		            {
		              expTrace("Y", __FILE__, __LINE__, "szDealType is %s ",g_sInfo[i].szDealType);
		  	          infoDeal.WriteFile(g_sInfo[i].szDealType,szService,szTFrom,szTTo);
		            }
		            else
		            {
		            	//DealInfo(infoDeal,g_sInfo[i].szDealType,g_sInfo[i].szInfoPoint,1);
		            	expTrace("Y", __FILE__, __LINE__, "szDealType is %s ",g_sInfo[i].szDealType);
		            	 /////////////////////////////
	  	          //expTrace("Y", __FILE__, __LINE__, "szTFrom and szTTo %s and %s",szTFrom,szTTo);
		            	infoDeal.WriteFile(g_sInfo[i].szDealType,szService,szTFrom,szTTo);
		            }				
					      
					strcpy(szLogStr, "infopoint stat finished!信息点统计完成!");
		            expTrace("Y", __FILE__, __LINE__, szLogStr);
		          }
		          catch(...)
		          {
		      	    strcpy(szLogStr, "信息点获取错误!");
		      	    getCurTime(sz_errtime);
                    wrlog("infomgr","0","",'E','H',1003,002,sz_errtime,szLogStr,__FILE__,__LINE__);			        
		          }
					  }
				  				
			    }
			  
			    strcpy(szTempStartTime,szTmpDt);			  
			    if (1==kk)
			    {
			      	expTrace("Y", __FILE__, __LINE__, "the end date is %s",szTempTo);
				   //  infoDeal.SeqSuc(szTempTo);
			    }
			    			   		 
		   }
		  }
		//
		   while(1)
		   {
		    	getCurDate(szCurDate);
            sprintf(szLogFile,"__.020.%s.infopointMgr.%s.log", szCurDate,szService);
	        if(theLog.Open(szLogFile)<0) 
	        {
	   	       cout<<"open run_log file :"<<szLogFile<<".errno="<<errno<<endl;	
	           exit(-1);
	        }
		   	  
		   	  if (strcmp(szCurDate,szLastDate)==0)
		   	  {
		   	  }
		   	  else
		   	  {
		   	  	 try
		          {
		           sprintf(szSql, "delete from info_log where dealstarttime<'%s'",szLastDate);
		           ds.Open(szSql,NONSELECT_DML);
               ds.Execute();
	             ds.Close();
	             DBConn.Commit();
	             strcpy(szLastDate,szCurDate);
		          }
		          catch(...)
		          {
		      	    strcpy(szLogStr, "info_log delete error!");
		      	    getCurTime(sz_errtime);
                wrlog("infomgr","0","",'E','H',1003,002,sz_errtime,szLogStr,__FILE__,__LINE__);			        
		          }
		   	    
		   	  }
	        
			    kk=0;
			    for(i=0;i<g_sInfo.size(); i++)
			    {
				    if(g_sInfo[i].cTC.checkTime())
				    {
					    kk=1;					
					    g_sInfo[i].cTC.getFromTo(sszStartTime,sszEndTime);
					    if (strcmp(szTempEndTime,"0")==0)
					    {
						    strcpy(szTempEndTime,sszEndTime);
					    }
					    else if(strcmp(szTempEndTime,sszEndTime)!=0)
					    {
					    	expTrace("Y", __FILE__, __LINE__, "the end date is %s",szTempEndTime);
						   // infoDeal.SeqSuc(szTempEndTime);						
						    strcpy(szTempEndTime,sszEndTime);
					    }
								
					    sprintf(szLogStr, "信息点：%s:%s统计启动，时间%s--%s,生成频率:%d分钟",g_sInfo[i].szInfoPoint,g_sInfo[i].szInfoDesc,sszStartTime,sszEndTime,g_sInfo[i].iDelay);
		          expTrace("Y", __FILE__, __LINE__, szLogStr);
		      
		          try
		          {
		           //有点问题
		      	    infoDeal.SetDate(sszStartTime,sszEndTime);
		      	   // infoDeal.SetDate(szTFrom,szTTo);
		      	    if (infoDeal.GetBatch(g_sInfo[i].szInfoPoint,g_sInfo[i].szTypeId,g_sInfo[i].szRate)==-1)
		      	    {
		      	    	strcpy(szLogStr, "get batch error!");
		      	      getCurTime(sz_errtime);
                  wrlog("infomgr","0","",'E','H',1003,002,sz_errtime,szLogStr,__FILE__,__LINE__);	
                  
		      	    }
		      	    else
		      	    {
		      	    	//DealInfo(infoDeal,g_sInfo[i].szDealType,g_sInfo[i].szInfoPoint,1);
		      	    	infoDeal.WriteFile(g_sInfo[i].szDealType,szService,sszStartTime,sszEndTime);
		      	    }	
											      
					      strcpy(szLogStr, "信息点统计完成!");
		            expTrace("Y", __FILE__, __LINE__, szLogStr);
		          }
		          catch(...)
		          {
		      	    strcpy(szLogStr, "信息点获取错误!");
		      	    getCurTime(sz_errtime);
                wrlog("infomgr","0","",'E','H',1003,002,sz_errtime,szLogStr,__FILE__,__LINE__);	
		          }					
				  }				
			  }
			  
			  if (1==kk)
			  {
			  	expTrace("Y", __FILE__, __LINE__, "the end date is %s",szTempEndTime);
				 // infoDeal.SeqSuc(szTempEndTime);
			  }
			  
			  GetRunFlag(iRFlag,szService);
		    if (0==iRFlag)
		    {
		  	   expTrace("Y", __FILE__, __LINE__, "RUN_FLAG IS N, quit out!");
		  	   break;
		    }
		    
			  expTrace("Y", __FILE__, __LINE__, "waiting ......");
			  sleep(60);
		  }
		}
		
		expTrace("Y", __FILE__, __LINE__, "program exit;");
		exit(0);
	}
	catch (...)
	{			
		strcpy(szLogStr, "Excute error!");
		getCurTime(sz_errtime);
    wrlog("infomgr","0","",'E','H',1005,002,sz_errtime,szLogStr,__FILE__,__LINE__);
		exit(1);
	}

}

int GetRunFlag(int &iRunFlag, char *serv)
{
	CBindSQL bs(DBConn);
	char sz_Flag[3];
	strcpy(sz_Flag,"Y");
	char sztt[20];
	
	try
	{

    bs.Open("SELECT ENV_VALUE FROM INFO_PUB_ENV where SERVER_ID=:1 and ENV_NAME='RUN_FLAG'",SELECT_QUERY);
    bs<<serv;
    while(bs >>sz_Flag);
    bs.Close();
  }
  catch(...)
  {  
		getCurTime(sztt);
    wrlog("infomgr","0","",'E','H',1003,002,sztt,"ger run_flag err",__FILE__,__LINE__);	
    
  }
  
  if (sz_Flag[0] == 'Y' || sz_Flag[0] == 'y')
  	iRunFlag=1;
  else
  	iRunFlag=0;  	
  return 0;
}

char* GetSubStr(const char* szSource, int nIndex, char cSeparator, char* szDest)
{
	
	char szSrcStr[1000];
	strcpy( szSrcStr, szSource );
	
	sprintf(szDest, "%c", cSeparator);
		
	while( nIndex > 1 )
	{
		char* pSep = NULL;
		pSep = strchr( szSrcStr, cSeparator );
		if( pSep == NULL )	
			break;
			
		pSep[0] = '\0';
		strcpy( szSrcStr, pSep + 1 );
		nIndex--;
	}
	
	if(nIndex == 1)
	{
		char* pSep = NULL;
		pSep = strchr( szSrcStr, cSeparator );			
		if( pSep != NULL )	
			pSep[0] = '\0';
			
		strcpy( szDest, szSrcStr );
	}
	
	return szDest;	
}

//结算加szDatetime加上iMin分钟之后的时间zDeDatetime,主要是处理跨小时,跨天
char* addmin(char* szDatetime, int iMin,char *szDeDatetime)
{
	string strNowTime_Day,strNowTime_DD,strNowTime_HH,strNowTime_MM,strToday,strNxtDay;
  int HH,MM;
  char szTemp[50];
  
  strcpy(szTemp,szDatetime);
  strNowTime_Day=szDatetime;
  
  szTemp[8]=0; 
  strToday=szTemp;
  addDays(1,strToday.c_str(),szTemp);
  strNxtDay=szTemp;
  
  
  strNowTime_DD=strNowTime_Day.substr(0,8);
  strNowTime_HH=strNowTime_Day.substr(8,2);
  strNowTime_MM=strNowTime_Day.substr(10,2);
  
  HH=atoi(strNowTime_HH.c_str());
  MM=atoi(strNowTime_MM.c_str());

  MM=MM+iMin;
  
  if(MM>=60)
  {
  	MM=MM-60;
  	HH=HH+1;
  	if(HH>=24)
  	{
  		HH=HH-24;
  		strToday=strNxtDay;
  	}
  }

  sprintf(szTemp,"%s%02d%02d",strToday.c_str(),HH,MM);  
	strcpy(szDeDatetime,szTemp);
	return szDeDatetime;	
}

int CheckLock(char* szChTime,int iChDelay,char *szChCurDatetime,char*sFrom,char *sTo)
{
	char szTemp[50],szFrom[20],szTo[20];
  char szTime[20],szCur[20];
  int iiDelay;
  
  strcpy(szTime,szChTime);
  strcpy(szCur,szChCurDatetime);
  iiDelay=iChDelay;
  
  string strNowTime_Day,strCom,strNowTime_DD,strNowTime_HH,strNowTime_MM,strCom_DD,strCom_HH,strCom_MM;
  string strLastday,strToday,strLastMonth,strCurMonth;
  int HH,MM,HH_COM,MM_COM; 
  strNowTime_Day=szCur;
  
  strToday=strNowTime_Day.substr(0,8);
  strCurMonth=strNowTime_Day.substr(0,6);
  strNowTime_DD=strNowTime_Day.substr(6,2);
  strNowTime_HH=strNowTime_Day.substr(8,2);
  strNowTime_MM=strNowTime_Day.substr(10,2);
  
  HH=atoi(strNowTime_HH.c_str());
  MM=atoi(strNowTime_MM.c_str());
  
  memset(szTemp,0,sizeof(szTemp));
  addDays(-1,strToday.c_str(),szTemp);
  szTemp[8]=0;
  strLastday=szTemp;
  
  addDays(-20,strToday.c_str(),szTemp);
  szTemp[6]=0;
  strLastMonth=szTemp;
  
  
  strCom=szChTime;
  strCom_DD=strCom.substr(1,2);
  strCom_HH=strCom.substr(3,2);
  strCom_MM=strCom.substr(5,2);
  
  HH_COM=atoi(strCom_HH.c_str());
  MM_COM=atoi(strCom_MM.c_str());
  
  if(szTime[0]=='E')
  {
	if (5 == iiDelay)
	{
		if(MM>0)
  		{
  			sprintf(szFrom,"%s%02d%02d00",strToday.c_str(),HH,(MM-5));
  			sprintf(szTo,"%s%02d%02d00",strToday.c_str(),HH,MM);
  		}
		else
  		{
  			if(0==HH)
  			{
  				sprintf(szFrom,"%s%02d%02d00",strLastday.c_str(),23,55);
  				sprintf(szTo,"%s%02d%02d00",strToday.c_str(),HH,MM);
  			}
  			else
  			{
  				sprintf(szFrom,"%s%02d%02d00",strToday.c_str(),(HH-1),55);
  				sprintf(szTo,"%s%02d%02d00",strToday.c_str(),HH,MM);
  			}
  		}
  		strcpy(sFrom,szFrom);
  		strcpy(sTo,szTo);
  		return 0;
	}
  	else if (15 == iiDelay)
  	{
  		if(MM>0)
  		{
  			sprintf(szFrom,"%s%02d%02d00",strToday.c_str(),HH,MM-15);
  			sprintf(szTo,"%s%02d%02d00",strToday.c_str(),HH,MM);
  		}
  		else
  		{
  			if(0==HH)
  			{
  				sprintf(szFrom,"%s%02d%02d00",strLastday.c_str(),23,45);
  			  sprintf(szTo,"%s%02d%02d00",strToday.c_str(),HH,MM);
  			}
  			else
  			{
  				sprintf(szFrom,"%s%02d%02d00",strToday.c_str(),HH-1,45);
  			  sprintf(szTo,"%s%02d%02d00",strToday.c_str(),HH,MM);
  			}
  		}
  		strcpy(sFrom,szFrom);
  		strcpy(sTo,szTo);
  		return 0;
  	}
  	else if (30 == iiDelay)
  	{
  		if(30==MM||0==MM)
  		{
  			if(MM>0)
  		  {
  			  sprintf(szFrom,"%s%02d%02d00",strToday.c_str(),HH,0);
  			  sprintf(szTo,"%s%02d%02d00",strToday.c_str(),HH,MM);
  		  }
  	  	else
  		  {
  			  if(0==HH)
  			  {
  				  sprintf(szFrom,"%s%02d%02d00",strLastday.c_str(),23,30);
  			    sprintf(szTo,"%s%02d%02d00",strToday.c_str(),HH,MM);
  			  }
  			  else
  			  {
  				  sprintf(szFrom,"%s%02d%02d00",strToday.c_str(),HH-1,30);
  			    sprintf(szTo,"%s%02d%02d00",strToday.c_str(),HH,MM);
  			  }
  		  }
  		  strcpy(sFrom,szFrom);
  		  strcpy(sTo,szTo);
  			return 0;
  		}
  		else
  		{
  			strcpy(sFrom,"");
  		  strcpy(sTo,"");
  			return -1;
  		}
  	}
  	else
  	{
  		if(0==MM)
  		{
  			 if(0==HH)
  			  {
  				  sprintf(szFrom,"%s%02d%02d00",strLastday.c_str(),23,0);
  			    sprintf(szTo,"%s%02d%02d00",strToday.c_str(),HH,MM);
  			  }
  			  else
  			  {
  				  sprintf(szFrom,"%s%02d%02d00",strToday.c_str(),HH-1,0);
  			    sprintf(szTo,"%s%02d%02d00",strToday.c_str(),HH,MM);
  			  }
  			strcpy(sFrom,szFrom);
  		  strcpy(sTo,szTo);
  			return 0;
  		}
  		else
  		{
  			strcpy(sFrom,"");
  		  strcpy(sTo,"");
  			return -1;
  		}
  	}  	
  }
  else if(szTime[0]=='D')
  {
  ///////////////////////////////记录在表格中的时间字段  
    	/*sprintf(szFrom,"%s%000000",strLastday.c_str());
  		sprintf(szTo,"%s%000000",strToday.c_str());
 
  		strcpy(sFrom,szFrom);
  		strcpy(sTo,szTo);

  		expTrace("Y", __FILE__, __LINE__, "szFrom is %s ",szFrom);
  		expTrace("Y", __FILE__, __LINE__, "szTo is %s ",szTo);
  		return 0;*/  		
  	if((HH_COM==HH)&&((MM-MM_COM)<15 && (MM>=MM_COM)))
  	{
  	  	sprintf(szFrom,"%s%000000",strLastday.c_str());
  		sprintf(szTo,"%s%000000",strToday.c_str());
 
  		strcpy(sFrom,szFrom);
  		strcpy(sTo,szTo);
  		return 0;
  	}  	
  	else
  	{
  		strcpy(sFrom,"");
  		strcpy(sTo,"");
  		return -1;
  	}
  		
  }
  else if(szTime[0]=='M')
  {
  	if(strcmp(strCom_DD.c_str(),strNowTime_DD.c_str())==0&&(HH_COM==HH)&&((MM-MM_COM)<15 && (MM>=MM_COM)))
  	{
  		expTrace("Y", __FILE__, __LINE__, "MM:%d,COM_MM:%d",MM,MM_COM);
  		sprintf(szFrom,"%s%",strLastMonth.c_str());
  		sprintf(szTo,"%s%",strCurMonth.c_str());
  		strcpy(sFrom,szFrom);
  		strcpy(sTo,szTo);
  		return 0;
  	}
  	else
  	{
  		strcpy(sFrom,"");
  		strcpy(sTo,"");
  		return -1;
  	}
  		
  }
  else
  {
  	strcpy(sFrom,"");
  	strcpy(sTo,"");
  	return -1;
  }
  
// expTrace("Y", __FILE__, __LINE__, "end checklock");
}

