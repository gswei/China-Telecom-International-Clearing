#include "CFreceive.h"
/*
   更新记录：
   2005-06-30 增加对解压缩文件的支持。
   2005-07-08 增加文件的备份功能,默认路径 sourcePath/revbak下。
   2005-07-11 增加文件超大超小的判断。
   2005-07-11 增加程序空转的判断。
   2005-07-12 增加守护进程。
   2005-10-16 在file_received增加PROCESS_ID字段，供平衡报表调用。
   2005-11-29 接收程序支持全业务的处理模式。
   2005-11-30 增加对文件名的配置功能。
   2006-01-17 超大超小文件不流入主流程时不插入文件接收登记表。
   2006-01-18 增加每天的文件数超过10000，序列号从1开始的判断。
   2006-02-08 增加文件名格式错误时写错误日志。
   2006-02-10 修改文件的WAITINGTIME可设。
   2006-03-10 增加读取ENV时候关联到PROCESSID。
   2006-03-28 修改备份模式，接收前备份，接收后备份，修改备份目录按时间进行归整。
   2006-04-03 修改接收QC的模式，未到QC的文件，文件先入库，不往后流。
   2006-04-03 增加QC_RECEIVED表，对QC文件进行登记。
   2006-04-12 修改重复文件命名，在原文件的基础上加重复时间。
   2006-04-19 增加对每个SOURCE的环境变量的配置。
   2006-04-19 增加对D.开头的文件登记重采表,对R.开头的文件对回退是否完成的判断。
   2006-04-20 修改R,D文件重复性判断的条件,原始文件名重复,压缩时间相同。
   2006-05-17 修改文件多次备份的错误，增加跨文件系统的处理方式。
   2006-05-25 增加不含QC文件的文件接收对.Z文件的解压缩的支持。
   2006-06-20 增加对帐务周期的判断，不符合的移到ERR目录，文件名后加.timeout。
   2006-06-30 修改D模式两次UPDATE--RECOLLECT_INFO的错误。
   2006-09-14 修改检查临时文件时BUFF的大小。
   2006-09-14 增加从原始文件名上获取序列号翻转的时间（目前只支持市话业务，且文件名必须为标准文件名）。
   2006-09-14 增加序列号位数可配置。
   2006-10-27 修改原始文件名上获取序列号翻转的时间时，隔天出错的 BUG。
   2007-02-06 修改接收的插SCH表失败，不COMMIT，而是rollback!
   2007-02-09 修改对iRereceiveDay=0的支持。 
   2007-03-16 增加只备份QC文件的标志RECEIVE_QC_BAK_ONLY_FLAG。
   2007-04-28 修改只有在RECEIVE_QC_BAK_ONLY_FLAG为Y时，才备份话单文件的错误。
   2007-09-29 修改统配符长度，由100改为200。
   2007-11-19 增加新信息点日志。
   2008-03-06 修改信息点BUFF为4W

   注:
   	  取消了对输入文件名进行数据源ID匹配的条件。
   语音长途处理过程：
   1.获取文件标志,对R文件进行回退是否完成的判断;
   2.判重；
   3.超大超小文件的判断。
   4.生新文件名
   5.查SCH
   6.解压
   7.插表(file_received)
   8.插表(D文件插重采登记表)
   9.备份文件
*/
//int iii=0;

extern int _CmpCreateTime(const void *p1, const void *p2)
{
    int i;
    struct FILE_INFO *ptr1, *ptr2;
    ptr1 = (struct FILE_INFO *) p1; 
    ptr2 = (struct FILE_INFO *) p2;
    i =ptr1->lCreateTime-ptr2->lCreateTime;
    if(i==0)
      i=strcmp(ptr1->sz_orgFname,ptr2->sz_orgFname);
    return i;
}

extern int _CmpFileSize(const void *p1, const void *p2)
{
    int i;
    struct FILE_INFO *ptr1, *ptr2;
    ptr1 = (struct FILE_INFO *) p1; 
    ptr2 = (struct FILE_INFO *) p2;
    i =ptr1->iFileSize-ptr2->iFileSize;
    if(i==0)
      i=strcmp(ptr1->sz_orgFname,ptr2->sz_orgFname);

    return i;
}

extern int _CmpFileName(const void *p1, const void *p2)
{
    int i;
    struct FILE_INFO *ptr1, *ptr2;
    ptr1 = (struct FILE_INFO *) p1; 
    ptr2 = (struct FILE_INFO *) p2;
 
    i=strcmp(ptr1->sz_orgFname,ptr2->sz_orgFname);
    
    return i;
}

CFreceive::CFreceive()
{
	 filelist=NULL;
	 sourcelist=NULL;
	 QCRecordlist=NULL;
	 c_QCtableFlag='Y';
	 filelist = new FILE_INFO[FILEREC_INIT_FILE_COUNT+1];
	 sz_cycleTime[0]='\0';
	 cQcBakOnlyFlag='N';

}

CFreceive::~CFreceive()
{
	if(filelist!=NULL)
	{
		delete []filelist;
	}
	if(sourcelist!=NULL)
	{
		delete []sourcelist;
	}
	if(QCRecordlist!=NULL)
	{
		delete []QCRecordlist;
	}
}


int CFreceive::Init(char *pch_pipeId,char *pch_processId,char *pch_envpath,char *pch_program)
{
	char sz_envfile[255],sz_envpath[255],sz_logPath[255];
	char sz_filename[255],sz_nowtime[255];
	char sz_buff[1000];
	
	strcpy(sz_pipeId,pch_pipeId);
	iProcessId=atoi(pch_processId);
	strcpy(sz_envpath,pch_envpath);
	completeDir(sz_envpath);
	sprintf(sz_envfile,"%szhjs.env",sz_envpath);

 /*********************************
  *    初始化错误日志             *
  *********************************/
	if(wrlog_setEnvPath(sz_envfile)!=0)//设置环境变量文件，读取错误日志路径;
  {
  	printf("setEnvPath err!\n");
  }
	/*********************************
  *    初始化信息点日志            * 
  **********************************/
	sprintf(sz_infoPoint,"%s_%s",sz_pipeId,pch_processId);
  DenyInterrupt(INFO_FILEREC_ERROR,sz_infoPoint);
  
 	/*********************************
  *    初始化新信息点日志            * 
  **********************************/ 
  infoLog_setEnvPath(sz_envpath);
  
  /*********************************
  *    初始化运行日志              *
  **********************************/
  if(getEnv(sz_envfile,(char *)"LOG_DIR",sz_logPath)!=0)
  {
    sprintf(sz_errmsg,"can't load the env LOG_DIR");
  	getCurTime(sz_errtime);
  	wrlog(sz_pipeId,iProcessId,sz_envfile,'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,__FILE__,__LINE__);
  	return FAIL;
  }
  
  setLogPath(sz_logPath,sz_pipeId);
  getCurTime(sz_nowtime);
  sz_nowtime[8]=0;
  sprintf(sz_filename,"%s.%s.%s.receive.log",sz_nowtime,sz_pipeId,pch_processId);
	theLog.Open(sz_filename);
	
	/*********************************
  *    初始化处理日志              * 
  **********************************/
  try
  {
  	DealLog.Init_Pipe(sz_pipeId,iProcessId,sz_envfile);
  }
  catch(CF_CError e)
  {
    getCurTime(sz_errtime);
    wrlog(sz_pipeId,iProcessId,"DealLogInit",sz_errtime,e);
  	return FAIL;
  }
  
	initDaemon(sz_envpath);
	try
	{
		if(connectDB(sz_envfile,DBConn)!=0)
		{
			sprintf(sz_errmsg,"ConnectDB Err");
  	  getCurTime(sz_errtime);
  	  wrlog(sz_pipeId,iProcessId,(char*)"ConnectDB_ERR",'E','H',FILEREC_ERR_CONNECT_DB,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
			return FAIL;
		}
	}
	catch(CF_CError e)
	{
		getCurTime(sz_errtime);
    wrlog(sz_pipeId,iProcessId,(char *)"ConnectDB_ERR",sz_errtime,e);
		return FAIL;
	}
	
  if(GetAllEnv()==FAIL)
  {
  	return FAIL;
  }
	if(GetOraData()==FAIL)
	{
		return FAIL;
	}
	
	char sz_ProgramPath[255],sz_ProgramName[255],sz_InputPath[40000];
  dispartFullName(pch_program,sz_ProgramPath,sz_ProgramName);
	sz_InputPath[0]=0;
	for(int i=0;i<iSourceNum;i++)
	{
    sprintf(sz_buff,"%s%s",sourcelist[i].sz_sourcePath,sz_fileInPath);
    sprintf(sz_InputPath,"%s%s;",sz_InputPath,sz_buff);
	}
	if(strlen(sz_InputPath)!=0)
	  sz_InputPath[strlen(sz_InputPath)-1]='\0';
	configInfoLog(iWorkFlow,iProcessId,sz_ProgramName,sz_ProgramPath,"zhjs.env",sz_envpath,sz_InputPath);

  return SUCC;
}
/***************************************
DEBUG_FLAG              Y
SOURCE_ENV_FLAG         Y
RECEIVE_COPY_FLAG       Y
RECEIVE_WAITING_TIME    60
RECEIVE_ERROR_PATH      reverror/
RECEIVE_BAK_PATH        revbak/
RECEIVE_BAK_DATE        Y
RECEIVE_BAK_FRONT       Y

***************************************/
int CFreceive::GetAllEnv()
{
	char sz_buff[255];
	CBindSQL bs(DBConn);
	
	try
	{
   	if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"DEBUG_FLAG",sz_DebugFlag)==-1)
	  {
      strcpy(sz_DebugFlag,"Y");
	  }
	  
	  if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"SOURCE_ENV_FLAG",sz_buff)==-1)
	  {
	  	c_SourceEnvFlag='Y';
	  }
	  else
	  {
	  	c_SourceEnvFlag=sz_buff[0];
	  }
	  
	  
	  if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_COPY_FLAG",sz_buff)==-1)
	  {
	  	c_copyFlag='N';
	  }
	  else
	  {
	  	c_copyFlag=sz_buff[0];
	  }
	  if(c_copyFlag!='Y'&&c_copyFlag!='N')
	  {
	    sprintf(sz_errmsg,"can't load the env RECEIVE_COPY_FLAG");
      getCurTime(sz_errtime);
  	  wrlog(sz_pipeId,iProcessId,(char*)"GET_ENV",'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	    return FAIL;
	  }
	  
	 	if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_WAITING_TIME",sz_buff)==-1)
	  {
      FILEREC_REC_WAITING_TIME=60;
    }
    else
    {
    	FILEREC_REC_WAITING_TIME=atoi(sz_buff);
    }

	 	if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_SORT_FLAG",sz_buff)==-1)
	  {
      cSortFlag='N';
    }
    else
    {
    	cSortFlag=sz_buff[0];
    }

	  if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_ERROR_PATH",sz_fileErrPath)==-1)
	  {//获取错误文件存放的相对路径
      strcpy(sz_fileErrPath,"reverror/");
	  }
	  else
	  {
	  	completeDir(sz_fileErrPath);
	  }
	  
	  if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_BAK_PATH",sz_fileBakPath)==-1)
	  {//获取备份文件存放的相对路径
      c_BakFlag='N';
	  }
	  else
	  {
	    c_BakFlag='Y';
	  	completeDir(sz_fileBakPath);
	  	
	    if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_QC_BAK_ONLY_FLAG",sz_buff)==-1)
	    {//获取QC备份
        cQcBakOnlyFlag='N';
	    }
	    else
	    {
	  	  if((sz_buff[0]=='Y')||(sz_buff[0]=='y'))
	  	    cQcBakOnlyFlag='Y';
	  	  else
	  	   cQcBakOnlyFlag='N';
	    }
	  	
	  	if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_BAK_DATE",sz_bakType)==-1)
	  	{
	  		strcpy(sz_bakType,"N");
	  	}
	  	else
	  	{
	  	  if(strlen(sz_bakType)>1)
	  	  {
	  	    c_OrgBatchFlag='Y';
	  	    bakdatecol=new struct COL_INFO[1];
          int iTmpNum;
          if(ParseFileName(sz_bakType,bakdatecol,iTmpNum,1)==FAIL)
          {
          	sprintf(sz_errmsg,"can't load the env RECEIVE_BAK_DATE");
            getCurTime(sz_errtime);
  	        wrlog(sz_pipeId,iProcessId,(char*)"GET_ENV",'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	      return FAIL;
          }
        }
        else
        {
        	c_OrgBatchFlag='Y';
        }
      }
      
	  	if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_BAK_FRONT",sz_buff)==-1)
      {
      	c_bakFrontFlag='N';
      }
      else
        c_bakFrontFlag=sz_buff[0];
	  }

	  if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_CHANGE_NAME_FLAG",sz_buff)==-1)
	  {//获取改名标志信息
      c_renameFlag='N';
	  }
	  else
	  {
	  	c_renameFlag=sz_buff[0];
	  }
	  	  
    if(c_renameFlag=='Y'||c_renameFlag=='y')
    {
      if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_FILENAME",sz_fileNameBuff)==-1)
      {
      	sz_fileNameBuff[0]='\0';
      }
    }

    if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_ORGNAME_FLAG",sz_buff)==-1)
    {
	  	c_OrgNameFlag='N';
	  }
	  else
	  {
	  	if((strlen(sz_buff)==1)&&(sz_buff[0]=='N'))
	  	{
	  		c_OrgNameFlag='N';
	  	}
	  	else
	  	{
	  	  c_OrgNameFlag='Y';
	  	  orgnamecol=new struct COL_INFO[1];
        int iTmpNum;
        if(ParseFileName(sz_buff,orgnamecol,iTmpNum,1)==FAIL)
        {
        	sprintf(sz_errmsg,"can't load the env RECEIVE_ORGNAME_FLAG");
          getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,(char*)"GET_ENV",'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	    return FAIL;
        }
        expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"RECEIVE_ORGNAME_FLAG=%s",sz_buff);
      }
    }

   	if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_ORGSOUREID_FLAG",sz_buff)==-1)
    {
	  	c_OrgSourceFlag='N';
	  }
	  else
	  {
	  	if((strlen(sz_buff)==1)&&(sz_buff[0]=='N'))
	  	{
	  		c_OrgSourceFlag='N';
	  	}
	  	else
	  	{
	  	  c_OrgSourceFlag='Y';
	  	  sourcecol=new struct COL_INFO[1];
        int iTmpNum;
        if(ParseFileName(sz_buff,sourcecol,iTmpNum,1)==FAIL)
        {
        	sprintf(sz_errmsg,"can't load the env RECEIVE_ORGSOUREID_FLAG");
          getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,(char*)"GET_ENV",'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	    return FAIL;
        }
        expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"RECEIVE_ORGSOUREID_FLAG=%s",sz_buff);
      }
    }
    
    if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_BILLCYCLE_FLAG",sz_buff)==-1)
    {
	  	c_BillCycleFlag='N';
	  }
	  else
	  {
	  	if((strlen(sz_buff)==1)&&(sz_buff[0]=='N'))
	  	{
	  		c_BillCycleFlag='N';
	  	}
	  	else
	  	{
	  	  c_BillCycleFlag='Y';
	  	  billcyclecol=new struct COL_INFO[1];
        int iTmpNum;
        if(ParseFileName(sz_buff,billcyclecol,iTmpNum,1)==FAIL)
        {
        	sprintf(sz_errmsg,"can't load the env RECEIVE_BILLCYCLE_FLAG");
          getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,(char*)"GET_ENV",'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	    return FAIL;
        }
        expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"RECEIVE_BILLCYCLE_FLAG=%s",sz_buff);
      }
    }
   
    if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_DROPTIME",sz_dropTime)==-1)
    {
    	c_DropTimeFlag='N';
    }
    else
    {
    	if((strlen(sz_dropTime)==1)&&sz_dropTime[0]=='N')
    	{
    		c_DropTimeFlag='N';
    	}
    	else if(c_BillCycleFlag=='N')
    	{
    		sprintf(sz_errmsg,"can't load the env RECEIVE_BILLCYCLE_FLAG");
        getCurTime(sz_errtime);
  	    wrlog(sz_pipeId,iProcessId,(char*)"GET_ENV",'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	  return FAIL;
    	}
    	else if(strlen(sz_dropTime)!=8)
    	{
    		sprintf(sz_errmsg,"can't load the env RECEIVE_DROPTIME");
        getCurTime(sz_errtime);
  	    wrlog(sz_pipeId,iProcessId,(char*)"GET_ENV",'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	  return FAIL;
    	}
    	else
    	{
    		c_DropTimeFlag='Y';
    	}
    }

    if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_ORGBATCH_FLAG",sz_buff)==-1)
    {
	  	c_OrgBatchFlag='N';
	  }
	  else
	  {
	  	if((strlen(sz_buff)==1)&&(sz_buff[0]=='N'))
	  	{
	  		c_OrgBatchFlag='N';
	  	}
	  	else
	  	{
	  	  c_OrgBatchFlag='Y';
	  	  batchcol=new struct COL_INFO[1];
        int iTmpNum;
        if(ParseFileName(sz_buff,batchcol,iTmpNum,1)==FAIL)
        {
        	sprintf(sz_errmsg,"can't load the env RECEIVE_ORGBATCH_FLAG");
          getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,(char*)"GET_ENV",'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	    return FAIL;
        }
        expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"RECEIVE_ORGBATCH_FLAG=%s",sz_buff);
      }
    }
    
    if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_ORGCOLDATE_FLAG",sz_buff)==-1)
    {
	  	c_OrgColDateFlag='N';
	  }
	  else
	  {
	  	if((strlen(sz_buff)==1)&&(sz_buff[0]=='N'))
	  	{
	  		c_OrgColDateFlag='N';
	  	}
	  	else
	  	{
	  	  c_OrgColDateFlag='Y';
	  	  datecol=new struct COL_INFO[1];
        int iTmpNum;
        if(ParseFileName(sz_buff,datecol,iTmpNum,1)==FAIL)
        {
        	sprintf(sz_errmsg,"can't load the env RECEIVE_ORGCOLDATE_FLAG");
          getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,(char*)"GET_ENV",'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	    return FAIL;
        }
        expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"RECEIVE_ORGCOLDATE_FLAG=%s",sz_buff);
      }
    }

    if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_SERIALTIME_FLAG",sz_buff)==-1)
    {
	  	c_serialTimeFlag='N';
	  }
	  else
	  {
	  	if((strlen(sz_buff)==1)&&(sz_buff[0]=='N'))
	  	{
	  		c_serialTimeFlag='N';
	  	}
	  	else
	  	{
	  	  c_serialTimeFlag='Y';
	  	  serialtimecol=new struct COL_INFO[1];
        int iTmpNum;
        if(ParseFileName(sz_buff,serialtimecol,iTmpNum,1)==FAIL)
        {
        	sprintf(sz_errmsg,"can't load the env RECEIVE_SERIALTIME_FLAG");
          getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,(char*)"GET_ENV",'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	    return FAIL;
        }
        expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"RECEIVE_SERIALTIME_FLAG=%s",sz_buff);
        if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_SERIAL_POS",sz_buff)==-1)
        {
        	iSerialPos=23;
        }
        else
          iSerialPos=atoi(sz_buff);
      }
    }

	  if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_FILE_TYPE",sz_buff)==-1)
    {//获取接收文件的类型
	  	sprintf(sz_errmsg,"can't load the env RECEIVE_FILE_TYPE");
  	  getCurTime(sz_errtime);
  	  wrlog(sz_pipeId,iProcessId,(char*)"GET_ENV",'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	return FAIL;
	  }
    c_FileType=sz_buff[0]; 
/*
    if(c_SourceEnvFlag=='N')
    {
      if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_FILE_FILTER",sz_fileFiter)==-1)
      {//获取接收文件的通配符
	  	  sprintf(sz_errmsg,"can't load the env RECEIVE_FILE_FILTER");
  	    getCurTime(sz_errtime);
  	    wrlog(sz_pipeId,iProcessId,(char*)"GET_ENV",'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	  return FAIL;
	  	}
	  	
	  	if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_QC_FLAG",sz_buff)==-1)
      {
        c_QCFlag='N';
	    }
	    else
	    {
        c_QCFlag=sz_buff[0];
      }
	  	
	    if(c_QCFlag=='Y')
      {
    	  if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_QC_FILTER",sz_QCFiter)==-1)
        {
	  	    sz_QCFiter[0]='\0';
	      }
	    
	      if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_QC_FILETYPE",sz_QCFileType)==-1)
        {
	  	    sprintf(sz_errmsg,"can't load the env RECEIVE_QC_FILETYPE");
  	      getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,(char*)"GET_ENV",'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	    return FAIL;
	      }
	    
	      if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_QC_DROPTIME",sz_buff)==-1)
        {
          iQCDropTime=24*60*60*30;
	      }
	      else
	      {
	    	  iQCDropTime=atoi(sz_buff);
	    	  iQCDropTime=iQCDropTime*24*60*60;
	      }
      }
	  	
	    if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_ALTERNATION_DAY",sz_buff)==-1)
    {
	  	c_ReceiveFlag='N';
	  }
	  else
	  {
	  	c_ReceiveFlag='Y';
	  	iRereceiveDay=atoi(sz_buff);
	  }
	  	
	    if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_LARGE_FILE",sz_buff)==-1)
      {//获取超大文件定义的大小
	      iLargeFile=-1;
	    }
	    else
	    { 
	  	  iLargeFile=atoi(sz_buff);
	    }
	  
	    if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_SMALL_FILE",sz_buff)==-1)
      {//获取超小文件定义的大小
        iSmallFile=-1;
	    }
	    else
	    {
	      iSmallFile=atoi(sz_buff);
      }
	  }
*/


    if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_COMPRESS_TIME_FLAG",sz_buff)==-1)
    {//获取接收文件的压缩时间
	    c_CompressTimeFlag='N';
	  }
	  else
	  {
	  	c_CompressTimeFlag=sz_buff[0];
	  }

	  if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_RACING_TIMES",sz_buff)==-1)
    {//获取接收空转告警的次数
      iRacingTime=-1;
	  }
	  else
	    iRacingTime=atoi(sz_buff);
    
	  if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_LARGE_SMALL_FLAG",sz_buff)==-1)
    {
      c_CheckFile='N';
	  }
	  else
	    c_CheckFile=sz_buff[0];
    
    if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_UNZIP_FLAG",sz_buff)==-1)
    {
	  	c_UzipFlag='N';
	  }
	  else
	  {
	  	c_UzipFlag=sz_buff[0];
	  }
	  
	  if((c_UzipFlag!='Y')&&(c_UzipFlag!='N'))
	  {
	  	sprintf(sz_errmsg,"can't load the env RECEIVE_UNZIP_FLAG");
  	  getCurTime(sz_errtime);
  	  wrlog(sz_pipeId,iProcessId,(char*)"GET_ENV",'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	return FAIL;
	  }
	  if(c_UzipFlag=='Y')
	  {
      if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_SH_FILE",sz_shFile)==-1)
	    {
	  	  sprintf(sz_errmsg,"can't load the env RECEIVE_SH_FILE");
  	    getCurTime(sz_errtime);
  	    wrlog(sz_pipeId,iProcessId,(char*)"GET_ENV",'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	  return FAIL;
	    }
	  }
	  
    if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_BATCH_FLAG",sz_buff)==-1)
    { //获取批次标识，为Y要获取指定的批次标识，为N要获取批次间隔时间
	  	sprintf(sz_errmsg,"can't load the env RECEIVE_BATCH_FLAG");
  	  getCurTime(sz_errtime);
  	  wrlog(sz_pipeId,iProcessId,(char*)"GET_ENV",'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	return FAIL;
    }
    else
      c_BatchFlag=sz_buff[0];
    
    if(c_BatchFlag=='Y')
    {
    	if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_BATCH_VALUES", sz_buff)==-1)
      {
	  	  sprintf(sz_errmsg,"can't load the env RECEIVE_BATCH_VALUES");
  	    getCurTime(sz_errtime);
  	    wrlog(sz_pipeId,iProcessId,(char*)"GET_ENV",'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	  return FAIL;
	  	}
      iBatchVal=atoi(sz_buff);
      if((iBatchVal>99999)||(iBatchVal<0))
      {
        sprintf(sz_errmsg,"can't load the env RECEIVE_BATCH_VALUES");
  	    getCurTime(sz_errtime);
  	    wrlog(sz_pipeId,iProcessId,(char*)"GET_ENV",'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	  return FAIL;
      }
    }
    else if(c_BatchFlag=='N')
    {
      if(getEnvFromDB(DBConn,sz_pipeId,iProcessId,(char *)"RECEIVE_BATCH_TIME", sz_buff)==-1)
        iBatchTime=15;
      else
        iBatchTime=atoi(sz_buff);
    }
    else
    {
	  	sprintf(sz_errmsg,"can't load the env RECEIVE_BATCH_FLAG");
  	  getCurTime(sz_errtime);
  	  wrlog(sz_pipeId,iProcessId,(char*)"GET_ENV",'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	return FAIL;
    }
  }

  catch(CF_CError e)
  {
  	getCurTime(sz_errtime);
    wrlog(sz_pipeId,iProcessId,"oracleErr",sz_errtime,e);
    DBConn.Rollback();
    return FAIL;
  }

  sprintf(sz_buff,"RECEIVE_FILE_TYPE=%c",c_FileType);
  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,sz_buff);
  sprintf(sz_buff,"RECEIVE_CHANGE_NAME_FLAG=%c",c_renameFlag);
  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,sz_buff);
  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"RECEIVE_ERROR_PATH=%s",sz_fileErrPath);
  
  if(c_BakFlag=='Y')
  {
  	if(cQcBakOnlyFlag=='Y')
  	{
      sprintf(sz_buff,"RECEIVE_QC_BAK_ONLY_FLAG=%c",cQcBakOnlyFlag);
      expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,sz_buff);
    }
    expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"RECEIVE_BAK_DATE=%s",sz_bakType);
    expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"RECEIVE_BAK_PATH=%s",sz_fileBakPath);
  }
  
  if(c_CompressTimeFlag=='Y')
  {
    sprintf(sz_buff,"RECEIVE_COMPRESS_TIME_FLAG=%c",c_CompressTimeFlag);
    expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,sz_buff);
  }
  
  sprintf(sz_buff,"RECEIVE_UNZIP_FLAG=%c",c_UzipFlag);
  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,sz_buff);

  if(c_UzipFlag=='Y')
  {
     expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"RECEIVE_SH_FILE=%s",sz_shFile);
  }

  sprintf(sz_buff,"RECEIVE_BATCH_FLAG=%c",c_BatchFlag);
  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,sz_buff);
  if(c_BatchFlag=='Y')
  {
    expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"RECEIVE_BATCH_VALUES=%d",iBatchVal);
  }
  else
  {
  	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"RECEIVE_BATCH_TIME=%d",iBatchTime);
  }
  
  if(c_DropTimeFlag=='Y')
  {
  	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"RECEIVE_DROPTIME=%s",sz_dropTime);
  }

  return SUCC;
}

int CFreceive::GetOraData()
{
	int iInputId,iOutputId;
	char sz_table[50];
	char sz_buff[255];
	char sz_proName[255];
	int iMemIndex;

  CBindSQL bs(DBConn);
  CBindSQL ps(DBConn);

  try
  {
    strcpy(sz_table,(char *)"PROCESS_CTL");
    bs.Open("SELECT PROCESS_NAME,MEM_INDEX,INTERVAL FROM PROCESS_CTL WHERE PIPE_ID= :1 and PROCESS_ID=:2",SELECT_QUERY);
    bs<<sz_pipeId<<iProcessId;
    bs >>sz_proName>>iMemIndex>>iSleepTime;
    bs.Close();
   
    strcpy(sz_table,(char *)"SOURCE");
    bs.Open("SELECT count(SOURCE_ID) FROM SOURCE WHERE PIPE_ID= :id",SELECT_QUERY);
    bs<<sz_pipeId;
    bs >>iSourceNum;
    bs.Close();

    sourcelist = new SOURCE_INFO[iSourceNum+1];

    bs.Open("SELECT SOURCE_ID,SOURCE_PATH FROM SOURCE WHERE PIPE_ID= :id ORDER BY SOURCE_ID",SELECT_QUERY);
    bs<<sz_pipeId;
    int i=0;
    while(bs>>sourcelist[i].sz_sourceId>>sourcelist[i].sz_sourcePath)
    {
    	delSpace(sourcelist[i].sz_sourcePath,0);
    	completeDir(sourcelist[i].sz_sourcePath);
      i++;
    }
    iSourceNum=i;
    bs.Close();
    
    if(c_renameFlag=='Y')
    {
      int iCount;
      bs.Open("select count(*) from user_tables where table_name=:1",SELECT_QUERY);
      bs<<"SOURCE_ENV";
      bs>>iCount;
      bs.Close();
      
      if(iCount==0)
      {
      	if(strlen(sz_fileNameBuff)==0)
      	{
          sprintf(sz_errmsg,"can't load the env RECEIVE_FILENAME");
          getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,(char*)"GET_ENV",'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	    return FAIL;
      	}
      	for(i=0;i<iSourceNum;i++)
      	{
      	  if(ParseFileName(sz_fileNameBuff,sourcelist[i].sFileNameList,sourcelist[i].iColNum,FILEREC_MAX_COLNUM)==FAIL)
          {
            sprintf(sz_errmsg,"can't load the env RECEIVE_FILENAME");
            getCurTime(sz_errtime);
  	        wrlog(sz_pipeId,iProcessId,(char*)"GET_ENV",'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	      return FAIL;
          }
          
          expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"[%s]=%s",sourcelist[i].sz_sourceId,sz_fileNameBuff);
          sourcelist[i].c_SerialFlag=c_SerialFlag;
          sourcelist[i].iMaxSerialValue=iMaxSerialValue;
          if(sourcelist[i].c_SerialFlag=='Y')
          {
            strcpy(sz_table,(char *)"FILE_SERIAL_INFO");
      	    bs.Open("SELECT DEAL_TIME,NEW_SERIAL FROM FILE_SERIAL_INFO WHERE SOURCE_ID=:id",SELECT_QUERY);
            bs<<sourcelist[i].sz_sourceId;
            if(!(bs>>sourcelist[i].sz_nowday>>sourcelist[i].iSerial))
            {
        	    char sz_nowtime[15];
        	    getCurTime(sz_nowtime);
        	    sz_nowtime[8]=0;
        	    strcpy(sourcelist[i].sz_nowday,(char *)"00000000");
        	    ps.Open("INSERT INTO FILE_SERIAL_INFO(SOURCE_ID,DEAL_TIME,NEW_SERIAL) VALUES(:1,:2,:3)",NONSELECT_DML);
              ps<<sourcelist[i].sz_sourceId<<sourcelist[i].sz_nowday<<'0';
              ps.Execute();
	            ps.Close();
	            sourcelist[i].iSerial=1;
            }
            else
            {
            	sourcelist[i].iSerial++;
            }
            sourcelist[i].sz_nowday[8]=0;

            if(sourcelist[i].iSerial>sourcelist[i].iMaxSerialValue) sourcelist[i].iSerial=1;
            bs.Close();
          }
          DBConn.Commit();
      	}
      }
    	else
    	{
      	for(i=0;i<iSourceNum;i++)
      	{
          int iCount;
          bs.Open("SELECT count(*) from source_env where source_id=:1 and varname=:2 ",SELECT_QUERY);
          bs<<sourcelist[i].sz_sourceId<<"RECEIVE_FILENAME";
          bs>>iCount;
          bs.Close();
          
          if(iCount==0)
          {
            if(strlen(sz_fileNameBuff)==0)
      	    {
              sprintf(sz_errmsg,"can't load the env RECEIVE_FILENAME");
              getCurTime(sz_errtime);
  	          wrlog(sz_pipeId,iProcessId,(char*)"GET_ENV",'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	        return FAIL;
          	}
          	else
          	{
            	if(ParseFileName(sz_fileNameBuff,sourcelist[i].sFileNameList,sourcelist[i].iColNum,FILEREC_MAX_COLNUM)==FAIL)
              {
                sprintf(sz_errmsg,"can't load the env RECEIVE_FILENAME");
                getCurTime(sz_errtime);
  	            wrlog(sz_pipeId,iProcessId,(char*)"GET_ENV",'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	          return FAIL;
              }
              expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"[%s]=%s",sourcelist[i].sz_sourceId,sz_fileNameBuff);
              sourcelist[i].c_SerialFlag=c_SerialFlag;
              sourcelist[i].iMaxSerialValue=iMaxSerialValue;

              if(sourcelist[i].c_SerialFlag=='Y')
              {
                strcpy(sz_table,(char *)"FILE_SERIAL_INFO");
      	        bs.Open("SELECT DEAL_TIME,NEW_SERIAL FROM FILE_SERIAL_INFO WHERE SOURCE_ID=:id",SELECT_QUERY);
                bs<<sourcelist[i].sz_sourceId;
                if(!(bs>>sourcelist[i].sz_nowday>>sourcelist[i].iSerial))
                {
            	    char sz_nowtime[15];
            	    getCurTime(sz_nowtime);
            	    sz_nowtime[8]=0;
            	    strcpy(sourcelist[i].sz_nowday,(char *)"00000000");
            	    ps.Open("INSERT INTO FILE_SERIAL_INFO(SOURCE_ID,DEAL_TIME,NEW_SERIAL) VALUES(:1,:2,:3)",NONSELECT_DML);
                  ps<<sourcelist[i].sz_sourceId<<sourcelist[i].sz_nowday<<'0';
                  ps.Execute();
	                ps.Close();
	                sourcelist[i].iSerial=1;
                }
                else
                {
                	sourcelist[i].iSerial++;
                }
                sourcelist[i].sz_nowday[8]=0;
            
                if(sourcelist[i].iSerial>sourcelist[i].iMaxSerialValue) sourcelist[i].iSerial=1;
                bs.Close();
              }
              DBConn.Commit();
          	}
          }
          else
          {
            bs.Open("SELECT var_value from source_env where source_id=:1 and varname=:2 ",SELECT_QUERY);
            bs<<sourcelist[i].sz_sourceId<<"RECEIVE_FILENAME";
            bs>>sz_buff;
            bs.Close();
            
            if(ParseFileName(sz_buff,sourcelist[i].sFileNameList,sourcelist[i].iColNum,FILEREC_MAX_COLNUM)==FAIL)
            {
              sprintf(sz_errmsg,"can't load the env RECEIVE_FILENAME");
              getCurTime(sz_errtime);
              wrlog(sz_pipeId,iProcessId,(char*)"GET_ENV",'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
		          return FAIL;
            }
            expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"[%s]=%s",sourcelist[i].sz_sourceId,sz_buff);
            sourcelist[i].c_SerialFlag=c_SerialFlag;
            sourcelist[i].iMaxSerialValue=iMaxSerialValue;
            if(sourcelist[i].c_SerialFlag=='Y')
            {
              strcpy(sz_table,(char *)"FILE_SERIAL_INFO");
              bs.Open("SELECT DEAL_TIME,NEW_SERIAL FROM FILE_SERIAL_INFO WHERE SOURCE_ID=:id",SELECT_QUERY);
              bs<<sourcelist[i].sz_sourceId;
              if(!(bs>>sourcelist[i].sz_nowday>>sourcelist[i].iSerial))
              {
           	    char sz_nowtime[15];
           	    getCurTime(sz_nowtime);
           	    sz_nowtime[8]=0;
           	    strcpy(sourcelist[i].sz_nowday,(char *)"00000000");
           	    ps.Open("INSERT INTO FILE_SERIAL_INFO(SOURCE_ID,DEAL_TIME,NEW_SERIAL) VALUES(:1,:2,:3)",NONSELECT_DML);
                ps<<sourcelist[i].sz_sourceId<<sourcelist[i].sz_nowday<<'0';
                ps.Execute();
	              ps.Close();
	              sourcelist[i].iSerial=1;
              }
              else
              {
              	sourcelist[i].iSerial++;
              }
              sourcelist[i].sz_nowday[8]=0;
            
              if(sourcelist[i].iSerial>sourcelist[i].iMaxSerialValue) sourcelist[i].iSerial=1;
              bs.Close();
            }
            DBConn.Commit();
          }
        }
    	}
    }
 
    for(i=0;i<iSourceNum;i++)//获取SOURCE环境变量
    {
    	int iNum;
    	iNum=0;
    	bs.Open("SELECT count(*) FROM FILE_RECEIVE_ENV WHERE SOURCE_ID=:1",SELECT_QUERY);
      bs<<sourcelist[i].sz_sourceId;
      bs>>iNum;
      bs.Close();
      
      if(iNum==0) 
      {
      	sprintf(sz_errmsg,"can't load the env for %s",sourcelist[i].sz_sourceId);
  	    getCurTime(sz_errtime);
  	    wrlog(sz_pipeId,iProcessId,sourcelist[i].sz_sourceId,'E','H',FILEREC_ERR_GET_ENV,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
        return FAIL;
      }
      else
      {
        bs.Open("SELECT FILE_FILTER,QC_FLAG,QC_FILTER,QC_FILETYPE,ALTERNATION_DAY,LARGE_FSIZE,SMALL_FSIZE FROM FILE_RECEIVE_ENV WHERE SOURCE_ID=:1",SELECT_QUERY);
        bs<<sourcelist[i].sz_sourceId;
        bs>>sourcelist[i].sz_fileFiter>>sourcelist[i].c_QcFlag>>sourcelist[i].sz_QCFiter>>sourcelist[i].sz_QcType>>sourcelist[i].iRereceiveDay>>sourcelist[i].iLargeFsize>>sourcelist[i].iSmallFsize;
        bs.Close();
        if(sourcelist[i].c_QcFlag=='Y') c_QCFlag='Y';
      }
    }
    
    strcpy(sz_table,(char *)"PIPE");
    bs.Open("SELECT WORKFLOW_ID FROM PIPE WHERE PIPE_ID=:1",SELECT_QUERY);
    bs<<sz_pipeId;
    bs>>iWorkFlow;
    bs.Close();

    strcpy(sz_table,(char *)"WORKFLOW");
    bs.Open("SELECT INPUT_ID,OUTPUT_ID FROM WORKFLOW WHERE(WORKFLOW_ID=:1 and PROCESS_ID=:2)",SELECT_QUERY);
    bs<<iWorkFlow<<iProcessId;
    bs>>iInputId>>iOutputId;//获得输入接口序号,输出接口序号;
    bs.Close();

    strcpy(sz_table,(char *)"MODEL_INTERFACE");
    bs.Open("SELECT PATH FROM MODEL_INTERFACE WHERE INTERFACE_ID=:1",SELECT_QUERY);
    bs<<iInputId;
    bs>>sz_fileInPath;//获得输入控制表；输入文件目录信息;
    bs.Close();
    delSpace(sz_fileInPath,0);
    completeDir(sz_fileInPath);

    bs.Open("SELECT CTL_TABNAME,PATH FROM MODEL_INTERFACE WHERE INTERFACE_ID=:1",SELECT_QUERY);
    bs<<iOutputId;
    bs>>sz_outControlTable>>sz_fileOutPath;//获得输出控制表；输出文件目录信息;
    bs.Close();
    delSpace(sz_outControlTable,0);
    delSpace(sz_fileOutPath,0);
    completeDir(sz_fileOutPath);
    
    if(c_QCFlag=='Y')
    {
    	int i,iNum;

   	  bs.Open("SELECT count(DISTINCT QC_FILETYPE) FROM SOURCE S,FILE_RECEIVE_ENV F WHERE S.SOURCE_ID=F.SOURCE_ID AND S.PIPE_ID=:1 AND F.QC_FLAG=:2 ORDER BY QC_FILETYPE",SELECT_QUERY);
		  bs<<sz_pipeId<<'Y';
		  bs>>iQcTypeNum;
		  bs.Close();

		  QCRecordlist=new QC_INFO[iQcTypeNum+1];
		  
		  i =0;
		  bs.Open("SELECT DISTINCT QC_FILETYPE FROM SOURCE S,FILE_RECEIVE_ENV F WHERE S.SOURCE_ID=F.SOURCE_ID AND S.PIPE_ID=:1 AND F.QC_FLAG=:2 ORDER BY QC_FILETYPE",SELECT_QUERY);
		  bs<<sz_pipeId<<'Y';
		  while(bs>>QCRecordlist[i].sz_QcType)
		  {
		  	i++;
		  }
		  bs.Close();
		  
		  for(i=0;i<iQcTypeNum;i++)
		  {
    	  bs.Open("SELECT RECORD_TYPE FROM FILETYPE_DEFINE WHERE FILETYPE_ID=:1 ",SELECT_QUERY);
		    bs<<QCRecordlist[i].sz_QcType;
		    bs>>QCRecordlist[i].c_messA;
		    bs.Close();
    	 
    	
    	  bs.Open("SELECT COL_INX FROM QC_FILETYPE_DEFINE WHERE FILETYPE_ID=:1 AND COL_FLAG=:3 ORDER BY PRIORITY",SELECT_QUERY);   	 
        bs<<QCRecordlist[i].sz_QcType<<'N';//获取文件名
        int j;
        j=0;
        while(bs>>QCRecordlist[i].iRecord[j][j])
        {
        	j++;
        }
        QCRecordlist[i].iNum[0]=j;
        bs.Close();
        if(QCRecordlist[i].iNum[0]!=1)
        {
      	  sprintf(sz_errmsg,"QC fileType Init Err!");
  	      getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,(char*)"QC_Init",'E','H',FILEREC_ERR_QC_INIT,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	    return FAIL;
        }

        j=0;
    	  bs.Open("SELECT COL_INX FROM QC_FILETYPE_DEFINE WHERE FILETYPE_ID=:1 AND COL_FLAG=:3 ORDER BY PRIORITY",SELECT_QUERY);   	 
        bs<<QCRecordlist[i].sz_QcType<<'S';//获取文件大小
        while(bs>>QCRecordlist[i].iRecord[1][j])
        {
      	  j++;
        }
        QCRecordlist[i].iNum[1]=j;
        bs.Close();

        j=0;
    	  bs.Open("SELECT COL_INX FROM QC_FILETYPE_DEFINE WHERE FILETYPE_ID=:1 AND COL_FLAG=:3 ORDER BY PRIORITY",SELECT_QUERY);   	 
        bs<<QCRecordlist[i].sz_QcType<<'R';//获取文件记录数
        while(bs>>QCRecordlist[i].iRecord[2][j])
        {
      	  j++;
        }
        QCRecordlist[i].iNum[2]=j;
        bs.Close();
       
        j=0;
    	  bs.Open("SELECT COL_INX FROM QC_FILETYPE_DEFINE WHERE FILETYPE_ID=:1 AND COL_FLAG=:3 ORDER BY PRIORITY",SELECT_QUERY);   	 
        bs<<QCRecordlist[i].sz_QcType<<'F';//获取费率
        while(bs>>QCRecordlist[i].iRecord[3][j])
        {
        	j++;
        }
        QCRecordlist[i].iNum[3]=j;
        bs.Close();

        j=0;
    	  bs.Open("SELECT COL_INX FROM QC_FILETYPE_DEFINE WHERE FILETYPE_ID=:1 AND COL_FLAG=:3 ORDER BY PRIORITY",SELECT_QUERY);   	 
        bs<<QCRecordlist[i].sz_QcType<<'D';//获取时长
        while(bs>>QCRecordlist[i].iRecord[4][j])
        {
      	  j++;
        }
        QCRecordlist[i].iNum[4]=j;
        bs.Close();
      }
    }
  }
  catch(CF_CError e)
  {
     getCurTime(sz_errtime);
     wrlog(sz_pipeId,iProcessId,sz_table,sz_errtime,e);
     return FAIL;
  }

  try
  {
    monitor.Attach();
  	monitor.Init(sz_pipeId,iProcessId,sz_proName,iMemIndex);
  }
  catch(CF_CError e)
  {
     getCurTime(sz_errtime);
     wrlog(sz_pipeId,iProcessId,"Memory",sz_errtime,e);
     return FAIL;
  }

  if(sz_DebugFlag[0]=='Y')
  {
  	for(int i=0;i<iSourceNum;i++)
  	{
  		expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"------------------------------------------------------------");
  		expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"sourceId[%d]=%s",i,sourcelist[i].sz_sourceId);
  		expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"sourcePath[%d]=%s",i,sourcelist[i].sz_sourcePath);
  		expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"sourceFilter[%d]=%s",i,sourcelist[i].sz_fileFiter);
  		if(sourcelist[i].c_QcFlag=='Y')
  		{
  		  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"sourceQCFilter[%d]=%s",i,sourcelist[i].sz_QCFiter);
  		  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"sourceQcType[%d]=%s",i,sourcelist[i].sz_QcType);
  		}
  		if(sourcelist[i].iLargeFsize>=0)
  		  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"sourceiLargeFsize[%d]=%d",i,sourcelist[i].iLargeFsize);
  		if(sourcelist[i].iSmallFsize>=0)
  		  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"sourceiSmallFsize[%d]=%d",i,sourcelist[i].iSmallFsize);
  		if(sourcelist[i].iRereceiveDay>=0)
  		  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"iRereceiveDay[%d]=%d",i,sourcelist[i].iRereceiveDay);
  	}
  	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"------------------------------------------------------------");
  	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"outControlTable=%s",sz_outControlTable);
  	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"fileInPath=%s",sz_fileInPath);
  	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"fileOutPath=%s",sz_fileOutPath);
  }
  if(c_QCFlag=='Y')
  {
  	for(int i=0;i<iQcTypeNum;i++)
  	{
  		for(int j=0;j<QCRecordlist[i].iNum[0];j++)
  	  {
  	  	sprintf(sz_buff,"QC.Name=%d",QCRecordlist[i].iRecord[0][j]);
  	  	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"%s",sz_buff);
      }
      
  		for(int j=0;j<QCRecordlist[i].iNum[1];j++)
  	  {
  	  	sprintf(sz_buff,"QC.Size=%d",QCRecordlist[i].iRecord[1][j]);
  	  	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"%s",sz_buff);
  	  }
  	
  		for(int j=0;j<QCRecordlist[i].iNum[2];j++)
  	  {
  	  	sprintf(sz_buff,"QC.Record=%d",QCRecordlist[i].iRecord[2][j]);
  	  	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"%s",sz_buff);
  	  }
  	
  		for(int j=0;j<QCRecordlist[i].iNum[3];j++)
  	  {
  	  	sprintf(sz_buff,"QC.Fee=%d",QCRecordlist[i].iRecord[3][j]);
  	  	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"%s",sz_buff);
  	  }

  		for(int j=0;j<QCRecordlist[i].iNum[4];j++)
      {
      	sprintf(sz_buff,"QC.Duration=%d",QCRecordlist[i].iRecord[4][j]);
  	  	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"%s",sz_buff);
      }
    }
  }
	return SUCC;
}

int CFreceive::Proc()
{
  int iRecNum,iRepNum,iErrNum,iDropNum,iTimeDropNum;

  try
  {
	while(1)
	{
		try
  	{//初始化运行日志
  		int iRunFlag;
  		char sz_nowday[8+1],sz_filename[255];
  		getCurDate(sz_nowday);
  		sprintf(sz_filename,"%s.%s.%d.receive.log",sz_nowday,sz_pipeId,iProcessId);
	    theLog.Open(sz_filename);
  		GetRunFlag(iRunFlag);
  		if(!iRunFlag)
  		{
  			monitor.Detach();
  	    expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"Run_flag=0,exit.");
  			exit(0);
  		}
  	}
  	catch (CF_CError e)
  	{
  		break;
  	}
		for(int i=0;i<iSourceNum;i++)
  	{
  	  iRecNum=0;
      iRepNum=0;
      iErrNum=0;
      iDropNum=0;
      iTimeDropNum=0;
      iSourceNo=i;
  	  if(sourcelist[i].c_QcFlag=='N')//不含QC文件的接收
  	  { 	
        iCountMax=0;
  	  	if(ScanFiles(sourcelist[i].sz_sourcePath,sourcelist[i].sz_fileFiter)==FAIL)
  	    {
  	    	return FAIL;
  	    }
  	    expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"Scan File Num=%d",iCountMax);

  	    if(iCountMax==0)
  	      sourcelist[i].iRacing++;
  	    else
  	      sourcelist[i].iRacing=0;
  	    if(sourcelist[i].iRacing==iRacingTime)
  	    {
  	      sprintf(sz_errmsg,"the PRO RACING %d TIMES",iRacingTime);
  	      getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,(char*)"RACING",'E','M',FILEREC_ERR_RACING,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
  	      sourcelist[i].iRacing=0;
  	    }
  	    for(int j=0;j<iCountMax;j++)
      	{
    		   monitor.UpdateMonitor(CProcInfo::PROC_BUSY,filelist[j].sz_orgFname);
    		   int ret;
    		   ret=ProcFile(sourcelist[i],filelist[j].sz_orgFname);
    		   if(ret==SUCC) iRecNum++;
    	     else if(ret==REC_FILE_REPEAT) iRepNum++;
    	     else if(ret==REC_FILE_ERR) iErrNum++;
    	     else if(ret==REC_FILE_DROP) iDropNum++;
    	     else if(ret==REC_TIME_NOT_ENOUGH) iTimeDropNum++;
    	     else continue;
    		   try
  	       {
  	         int iRunFlag;
  		       GetRunFlag(iRunFlag);
  		       if(!iRunFlag)
  		       {
  			       monitor.Detach();
  	           expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"Run_flag=0,exit.");
  		         exit(0);
  		       }
  	       }
  	       catch (CF_CError e)
  	       {
  		        break;
  	       }
	      }
    	}
    	else//含QC文件的接收
    	{
    		iCountMax=0;
    		if(ScanFiles(sourcelist[i].sz_sourcePath,sourcelist[i].sz_QCFiter)==FAIL)
    		{
    			return FAIL;
    		}
    		expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"Scan QC File Num=%d",iCountMax);
        
        int iQcTypeNo=-1;
        
        if(iCountMax!=0)
        {
          try
          {
          	for(int j=0;j<iCountMax;j++)
          	{
          		if(strcmp(sourcelist[i].sz_QcType,QCRecordlist[j].sz_QcType)==0)
          		{
                QCRecord.Init(QCRecordlist[j].sz_QcType,QCRecordlist[j].c_messA);
                iQcTypeNo=j;
                break;
          		}
          	}
          	if(iQcTypeNo==-1) 
          	{
          		getCurTime(sz_errtime);
          		sprintf(sz_errmsg,"%s check Err!",sourcelist[i].sz_QcType);
          		wrlog(sz_pipeId,iProcessId,sourcelist[i].sz_QcType,'E','M',FILEREC_CHECK_QCTYPE,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
  	          continue;
          	}
          }
          catch(CF_CError e)
          {
  	        getCurTime(sz_errtime);
  	        wrlog(sz_pipeId,iProcessId,(char*)"Init_FMT_ERR",sz_errtime,e);
  	        continue;
          }
        }

    		for(int j=0;j<iCountMax;j++)
    		{
    			InsertQCToFileRec(sourcelist[i],filelist[j].sz_orgFname,iQcTypeNo);
    			try
  	      {
  		      int iRunFlag;
  		      GetRunFlag(iRunFlag);
  		      if(!iRunFlag)
  		      {
  		      	monitor.Detach();
  	          expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"Run_flag=0,exit.");
  		       	exit(0);
  		      }
          }
          catch (CF_CError e)
  	      {
  		      break;
        	}
    		}

	      if(ScanFiles(sourcelist[i].sz_sourcePath,sourcelist[i].sz_fileFiter)==FAIL)
    		{
    			return FAIL;
    		}
    		
    		expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"Scan File Num=%d",iCountMax);
        
  	    if(iCountMax==0)
  	      sourcelist[i].iRacing++;
  	    else
  	      sourcelist[i].iRacing=0;
  	    if(sourcelist[i].iRacing==iRacingTime)
  	    {
  	      sprintf(sz_errmsg,"the PRO RACING %d TIMES",iRacingTime);
  	      getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,(char*)"RACING",'E','M',FILEREC_ERR_RACING,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
  	      sourcelist[i].iRacing=0;
  	    }
  	    
    		for(int j=0;j<iCountMax;j++)
    		{
    			int ret;
    			ret=ProcFileQC(sourcelist[i],filelist[j].sz_orgFname);
    		  if(ret==SUCC) iRecNum++;
    	    else if(ret==REC_FILE_REPEAT) iRepNum++;
    	    else if(ret==REC_FILE_ERR) iErrNum++;
    	    else if(ret==REC_FILE_DROP) iDropNum++;
    	    else if(ret==REC_TIME_NOT_ENOUGH) iTimeDropNum++;
    			try
  	      {
  		      int iRunFlag;
  		      GetRunFlag(iRunFlag);
  		      if(!iRunFlag)
  		      {
  		      	monitor.Detach();
  	          expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"Run_flag=0,exit.");
  		       	exit(0);
  		      }
          }
          catch (CF_CError e)
  	      {
  		      break;
        	}
    		}
    	}
    	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"scanFiles:%d,receiveFiles:%d,TimeDrop:%d,repeatFiles:%d,errFile:%d,dropFiles:%d!",\
        iCountMax,iRecNum,iTimeDropNum,iRepNum,iErrNum,iDropNum);
      expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"this source(%s)receive files finish\n",sourcelist[i].sz_sourceId);
    }
  	monitor.UpdateMonitor(CProcInfo::PROC_IDLE);
  	SleepSender();
  }
  }
  catch(...)
  {
  	sprintf(sz_errmsg,"system Err!!");
  	getCurTime(sz_errtime);
    wrlog(sz_pipeId,iProcessId,(char*)"sysErr",'E','H',FILEREC_ERR_SYSTEM,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  return FAIL;
  }
  return SUCC;
}

int CFreceive::ProcFile(struct SOURCE_INFO &e,char *pch_orgname)
{
	char sz_nowtime[14+1],sz_buff[255],sz_orgSourceId[5+1];
	char sz_nowday[8+1],sz_colday[14+1];
	char sz_bakday[8+1];
	char sz_realFileName[255];
	char sz_unzipFile[255];
	char sz_inFilename[255],sz_outFilename[255];
	char sz_compTime[100],sz_orgName[255],sz_ErrName[255];
	int  iFileCycle;
	char sz_endSuff[20],sz_serialDay[8+1];
	char cPrefixFlag,iCheckSerialFlag;
	int  iBatch,iFileSize,iTmpSerialNo;//临时序列号
	
	struct stat buf;
	time_t timer;
  
  sprintf(sz_inFilename,"%s%s%s",e.sz_sourcePath,sz_fileInPath,pch_orgname);
  
  time(&timer);
  stat(sz_inFilename,&buf);
  if(timer-buf.st_mtime<FILEREC_REC_WAITING_TIME)
  {
  	return REC_TIME_NOT_ENOUGH;
  }

/*
  if(c_BillCycleFlag=='Y')
  {
  	char sz_tmpBatch[10];
    if(GetCol(pch_orgname,billcyclecol[0],sz_tmpBatch)==FAIL)
    {
    	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"GET BillCycle Err!");
    	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
    	sprintf(sz_errmsg,"file(%s) GET BillCycle Err!",pch_orgname);
    	MoveToErrPath(e,sz_inFilename,pch_orgname);
	    return REC_FILE_ERR;
    }
    iFileCycle=atoi(sz_tmpBatch);
  }
  
  if(c_DropTimeFlag=='Y')
  {
  	if(strncmp(sz_nowtime,sz_cycleTime,6)!=0)
  	{
  	  if(SelectBillCycle(sz_nowtime,iBillCycle)==FAIL)
  	  {
        getCurTime(sz_errtime);
        sprintf(sz_errmsg,"GET BillCycle(%s) From DB Err!",sz_nowtime);
        wrlog(sz_pipeId,iProcessId,pch_orgname,'E','H',FILEREC_ERR_CYCLEOUT,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
        expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"GET BillCycle From DB Err!");
  		  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
  	    return REC_FILE_DROP;
  	  }
  	  strncpy(sz_cycleTime,sz_nowtime,6);
  		sz_cycleTime[6]=0;
  	}
  	if(iBillCycle>iFileCycle)
  	{
  		if(strncmp(sz_nowtime+6,sz_dropTime,8)>0)
  		{
  			getCurTime(sz_errtime);
        sprintf(sz_errmsg,"the File(%s) BillCycle Out Err!",pch_orgname);
        wrlog(sz_pipeId,iProcessId,pch_orgname,'E','H',FILEREC_ERR_CYCLEOUT,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
  			expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"BillCycle Out Err!");
    	  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
    	  sprintf(sz_buff,"%s.timeout",pch_orgname);
    	  MoveToErrPath(e,sz_inFilename,sz_buff);
    	  return REC_FILE_ERR;
  		}
  	}
  }
*/


  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------start----------------------------------");
  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"begin to receive the file %s",pch_orgname);
  getCurTime(sz_nowtime);//开始处理时间
  
  strcpy(sz_compTime,sz_nowtime);
  strncpy(sz_nowday,sz_nowtime,8);
  sz_nowday[8]=0;//处理日期
 
  if(c_UzipFlag=='Y')
  {
  	char *pch_temp;
  	pch_temp=NULL;
  	if((pch_temp=strrchr(pch_orgname,'.'))==NULL)
  	{
  		getCurTime(sz_errtime);
      sprintf(sz_errmsg,"the InFile(%s) Err!",pch_orgname);
      wrlog(sz_pipeId,iProcessId,pch_orgname,'E','H',FILEREC_ERR_ORGNAME,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
    	MoveToErrPath(e,sz_inFilename,pch_orgname);
  		return FAIL;
  	}
  	strcpy(sz_endSuff,pch_temp+1);
  }

  if(c_OrgNameFlag=='Y')//获取原始文件名(sz_orgName)
  {
    if(GetCol(pch_orgname,orgnamecol[0],sz_orgName)==FAIL)
    {
    	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"GET OrgName Err");
    	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
    	sprintf(sz_errmsg,"file(%s) GET orgname Err!",pch_orgname);
    	getCurTime(sz_errtime);
    	wrlog(sz_pipeId,iProcessId,pch_orgname,'E','M',FILEREC_ERR_GETCOL,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	   
    	MoveToErrPath(e,sz_inFilename,pch_orgname);
    	return REC_FILE_ERR;
    }
  }
  else
  {
    strcpy(sz_orgName,pch_orgname);
  }
  try
	{
    int iFlag;

    if(c_CompressTimeFlag=='Y')
    {
    	char *pch_tmp;
    	
    	if((pch_tmp=strrchr(pch_orgname,'.'))==NULL)//获取压缩时间
    	{
    		expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"GET CompressTime Err");
    	  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
    	  getCurTime(sz_errtime);
        sprintf(sz_errmsg,"the InFile(%s) Err!",pch_orgname);
        wrlog(sz_pipeId,iProcessId,pch_orgname,'E','H',FILEREC_ERR_ORGNAME,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
    		MoveToErrPath(e,sz_inFilename,pch_orgname);
	  	  return REC_FILE_ERR;
    	}
    	else
    	{
    		strcpy(sz_compTime,pch_tmp+1);
    	}

    	if(strncmp(pch_orgname,"C.",2)==0)//获取文件标志
      {
  	    cPrefixFlag='C';
      } 
      else if(strncmp(pch_orgname,"R.",2)==0)
      {
  	    cPrefixFlag='R';
      }
      else if(strncmp(pch_orgname,"D.",2)==0)
      {
  	    cPrefixFlag='D';
      }
      else
      {
      	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"GET File Flag Err");
    	  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
        getCurTime(sz_errtime);
        sprintf(sz_errmsg,"the InFile(%s) Err!",pch_orgname);
        wrlog(sz_pipeId,iProcessId,pch_orgname,'E','H',FILEREC_ERR_ORGNAME,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
    	  MoveToErrPath(e,sz_inFilename,pch_orgname);
	  	  return REC_FILE_ERR;
      }
      
      if(cPrefixFlag=='R')
      {
      	if(CheckRecollectInfo(e.sz_sourceId,sz_orgName,sz_nowtime)==REC_FILE_DROP)
      	  return REC_FILE_DROP;
      }

      iFlag=LDTCheckFileInFileReceive(e.sz_sourceId,pch_orgname,sz_orgName,sz_compTime,cPrefixFlag);    //判断是否是重复文件
    }
    else
    {
      iFlag=CheckFileInFileReceive(e.sz_sourceId,pch_orgname);
    }

	  if(iFlag==FAIL)
	  {
	  	getCurTime(sz_errtime);
      sprintf(sz_errmsg,"the InFile(%s) Err!",pch_orgname);
      wrlog(sz_pipeId,iProcessId,pch_orgname,'E','H',FILEREC_ERR_ORGNAME,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
      MoveToErrPath(e,sz_inFilename,pch_orgname);
	  	return REC_FILE_ERR;
	  }
	  
	  if(iFlag==FILEREC_REPEAT_FILE)
	  {
		  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"the file is repeat file");
	    char sz_repeatYear[6+1],sz_repeatDay[2+1];
	    char sz_repeatRname[255];
	    strncpy(sz_repeatYear,sz_nowtime,6);
	    sz_repeatYear[6]=0;
	    strncpy(sz_repeatDay,sz_nowtime+6,2);
      sz_repeatDay[2]=0;
	    sprintf(sz_repeatRname,"%srepeat_file/%s/%s",e.sz_sourcePath,sz_repeatYear,sz_repeatDay);
	    chkAllDir(sz_repeatRname);
	    completeDir(sz_repeatRname);
	    sprintf(sz_repeatRname,"%s%s.%s",sz_repeatRname,sz_orgName,sz_nowtime);

      expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"Insert the file to REPEAT_FILE table");

	    InsertRepeatFile(sz_orgName,e.sz_sourceId,sz_nowtime);

      expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"move the file to repeat DIR");
      
      if(c_copyFlag=='Y')
      {
      	if(copyFile(sz_inFilename,sz_repeatRname)!=0)
        {
        	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"copy File Err!");
          expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
      	  sprintf(sz_errmsg,"the file (%s) copy to (%s) Err!",sz_inFilename,sz_repeatRname);
  	      getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,sz_inFilename,'E','H',FILEREC_ERR_COPY,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
	  	    DBConn.Rollback();
  		    return REC_FILE_DROP;
  		  }
  		  if(unlink(sz_inFilename))
  		  {
  		  	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"unlink File Err!");
          expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
  		  	sprintf(sz_errmsg,"the file (%s) unlink Err!",sz_inFilename);
  	      getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,sz_inFilename,'E','H',FILEREC_ERR_UNLINK,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
          DBConn.Rollback();
  	      return REC_FILE_DROP;
  		  }
      }
      else
      {
	      if(rename(sz_inFilename,sz_repeatRname)!=0)
	      {
	      	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"rename File Err!");
          expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
	      	sprintf(sz_errmsg,"the repeatfile (%s) rename to (%s) Err!",sz_inFilename,sz_repeatRname);
  	      getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,sz_inFilename,'E','H',FILEREC_ERR_RENAME,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	    DBConn.Rollback();
          return REC_FILE_DROP;
	      }
	    }
    	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
	    DBConn.Commit();
	    return REC_FILE_REPEAT;
    }
    
  	iFileSize=GetFileSize(sz_inFilename);//检测文件大小

  	if(((iFileSize>=e.iLargeFsize)&&(e.iLargeFsize>0))||((iFileSize<=(e.iSmallFsize))&&(e.iSmallFsize>0)))
    {
    	sprintf(sz_errmsg,"find the large or small file(%s)",pch_orgname);
  	  getCurTime(sz_errtime);
  	  wrlog(sz_pipeId,iProcessId,pch_orgname,'E','M',FILEREC_ERR_LARGE_SMALL_FILE,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
      if(c_CheckFile=='Y')
      {
      	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"File Size Err");
    	  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
    	  MoveToErrPath(e,sz_inFilename,pch_orgname);
	  		DBConn.Rollback();
	  	  return REC_FILE_ERR;
      }
    }
    
    if(c_serialTimeFlag=='Y')//获取序列号时间
    {
    	if(GetCol(pch_orgname,serialtimecol[0],sz_serialDay)==FAIL)
    	{
    		expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"GET serialDay Err!");
    	  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
    		sprintf(sz_errmsg,"file(%s) GET serialDay Err!",pch_orgname);
    	  MoveToErrPath(e,sz_inFilename,pch_orgname);
	    	return REC_FILE_ERR;
    	}
    }
    else
    {
    	strcpy(sz_serialDay,sz_nowday);
    }
    
    if(e.c_SerialFlag=='Y')//获取序列号
    {
      iCheckSerialFlag=strcmp(sz_serialDay,e.sz_nowday);
	    if(iCheckSerialFlag>0)
	    {
	  	  iTmpSerialNo=1;
	    }
	    else if(iCheckSerialFlag==0)
	    {
	    	iTmpSerialNo=e.iSerial;
	    }
	    else if(iCheckSerialFlag<0)
	    {
	    	iTmpSerialNo=GetMaxSerial(e.sz_sourceId,sz_serialDay);
	    	iTmpSerialNo++;
	    	if(iTmpSerialNo>e.iMaxSerialValue) iTmpSerialNo=1;
	    }
	  }

    if(c_OrgBatchFlag=='Y')//获取原批次号
    {
    	char sz_tmpBatch[10];
    	if(GetCol(pch_orgname,batchcol[0],sz_tmpBatch)==FAIL)
    	{
    		expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"GET OrgBatch Err!");
    	  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
    		sprintf(sz_errmsg,"file(%s) GET orgBatch Err!",pch_orgname);
    	  MoveToErrPath(e,sz_inFilename,pch_orgname);
	    	return REC_FILE_ERR;
    	}
    	iBatch=atoi(sz_tmpBatch);
    }
    else
    {
	    GetBatchId(sz_nowtime,iBatch);
    }
    
    if(c_BakFlag=='Y')
    {
      if(sz_bakType[0]=='Y')//获取备份方式
      {
      	strcpy(sz_bakday,sz_nowday);
      }
      else if(sz_bakType[0]=='D')
      {
      	strcpy(sz_bakday,sz_nowday);
      }
      else if(sz_bakType[0]=='M')
      {
      	strncpy(sz_bakday,sz_nowday,6);
      	sz_bakday[6]='\0';
      }
      else if(sz_bakType[0]!='N')
      {
    	  if(GetCol(pch_orgname,bakdatecol[0],sz_bakday)==FAIL)
	    	{
	    		expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"GET bakDate Err!");
    	    expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
	  	  	sprintf(sz_errmsg,"file(%s) GET bakDate Err!",pch_orgname);
    	    getCurTime(sz_errtime);
    	    wrlog(sz_pipeId,iProcessId,pch_orgname,'E','M',FILEREC_ERR_GETCOL,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
    	    MoveToErrPath(e,sz_inFilename,pch_orgname);
	    	  return REC_FILE_ERR;
	  	  }
      }
    }
    
	  if(c_OrgSourceFlag=='Y')//获取原数据源ID
	  {
	  	if(GetCol(pch_orgname,sourcecol[0],sz_orgSourceId)==FAIL)
	  	{
        expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"GET orgSource Err!");
    	  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
	  		sprintf(sz_errmsg,"file(%s) GET orgSource Err!",pch_orgname);
    	  getCurTime(sz_errtime);
    	  wrlog(sz_pipeId,iProcessId,pch_orgname,'E','M',FILEREC_ERR_GETCOL,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
    	  MoveToErrPath(e,sz_inFilename,pch_orgname);
	    	return REC_FILE_ERR;
	  	}
	  }
	  else
	  {
	  	strcpy(sz_orgSourceId,e.sz_sourceId);
	  }

    if(c_OrgColDateFlag=='Y')//获取原采集日期
    {
    	if(GetCol(pch_orgname,datecol[0],sz_colday)==FAIL)
    	{
    		expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"GET orgColDate Err!");
    	  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
    		sprintf(sz_errmsg,"file(%s) GET orgColDate Err!",pch_orgname);
    	  getCurTime(sz_errtime);
    	  wrlog(sz_pipeId,iProcessId,pch_orgname,'E','M',FILEREC_ERR_GETCOL,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	      MoveToErrPath(e,sz_inFilename,pch_orgname);
	      return REC_FILE_ERR;
    	}
    }
    else
    {
    	strcpy(sz_colday,sz_nowday);
    }
    
    if(e.c_SerialFlag=='Y')//获取序列号
    {
    	strcpy(sz_colday,sz_serialDay);
    }
  
	  if(c_renameFlag=='Y')//获取新文件名
	  {
	    if(GetFileName(sz_realFileName,e.sz_sourceId,sz_nowday,iBatch,iTmpSerialNo,pch_orgname,e)==FAIL)
      {
      	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"GET newName Err!");
    	  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
      	sprintf(sz_errmsg,"file(%s) changeName Err!",pch_orgname);
    	  getCurTime(sz_errtime);
    	  wrlog(sz_pipeId,iProcessId,pch_orgname,'E','M',FILEREC_ERR_CHANGENAME,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	      MoveToErrPath(e,sz_inFilename,pch_orgname);
	    	return REC_FILE_ERR;
	    }
    }
    else
    {
    	strcpy(sz_realFileName,pch_orgname);
    }

    if(c_UzipFlag=='Y')
	  {
		  sprintf(sz_inFilename,"%s%s%s",e.sz_sourcePath,sz_fileInPath,pch_orgname);
		  sprintf(sz_unzipFile,"%s.TMP.%s",sz_inFilename,sz_endSuff);
		  copyFile(sz_inFilename,sz_unzipFile);
		  sprintf(sz_buff,"%s %s",sz_shFile,sz_unzipFile);

      if(system(sz_buff))
      {
        sprintf(sz_errmsg,"System: %s err",sz_buff);
    	  getCurTime(sz_errtime);
    	  wrlog(sz_pipeId,iProcessId,"System",'E','M',FILEREC_ERR_COMMAND,errno,sz_errtime,sz_errmsg,(char *)__FILE__,__LINE__);
        return REC_FILE_DROP;
      }
      //sz_unzipFile[strlen(sz_unzipFile)-3]=0;
      sz_unzipFile[strlen(sz_unzipFile)-strlen(sz_endSuff)-1]=0;
      iFileSize=GetFileSize(sz_unzipFile);
    }
    
  	if(c_CompressTimeFlag=='Y')
  	{
  	  if(cPrefixFlag=='C')
      {
      	InsertFileReceive(e.sz_sourceId,sz_realFileName,sz_orgName,iBatch,iFileSize,sz_orgSourceId,sz_colday,sz_compTime,sz_nowtime,'Y',c_FileType);
      }
  	  if(cPrefixFlag=='D')
  	  {
      	InsertFileReceive(e.sz_sourceId,sz_realFileName,sz_orgName,iBatch,iFileSize,sz_orgSourceId,sz_colday,sz_compTime,sz_nowtime,'Y',c_FileType);
  	  	InsertRecollectInfo(e.sz_sourceId,sz_orgName,sz_realFileName,sz_nowtime);
  	  }
  	  else if(cPrefixFlag=='R')
  	  {
      	InsertFileReceive(e.sz_sourceId,sz_realFileName,sz_orgName,iBatch,iFileSize,sz_orgSourceId,sz_colday,sz_compTime,sz_nowtime,'Y','R');
  	  	UpdateRecollectInfo(e.sz_sourceId,sz_orgName,sz_realFileName,sz_nowtime);
  	  }
  	}
  	else
  	{
  	  InsertFileReceive(e.sz_sourceId,sz_realFileName,sz_orgName,iBatch,iFileSize,sz_orgSourceId,sz_colday,sz_compTime,sz_nowtime,'Y',c_FileType);
    } 

    expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"Insert the file to SchFormat table");

  	if(InsertSchFormat(sz_realFileName,e.sz_sourceId)==FILEREC_SCH_REPEAT)
  	{
  		//改名之后的文件名在FMT表里存在
  	  getCurTime(sz_errtime);
	  	sprintf(sz_errmsg,"Err insert sch table!");
  		wrlog(sz_pipeId,iProcessId,pch_orgname,'E','H',FILEREC_ERR_INSERT_SCH,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);

  		if(e.c_SerialFlag=='Y')
  		{
  			if(iCheckSerialFlag>=0)
  			{
	        UpdateSerialInfo(e.sz_sourceId,sz_serialDay,iTmpSerialNo);
	        strcpy(e.sz_nowday,sz_serialDay);
  	  	  e.iSerial=iTmpSerialNo;
  	  	  e.iSerial++;
  	  	  if(e.iSerial>e.iMaxSerialValue) e.iSerial=1;
  	  	}
  	  	DBConn.Rollback();
  	  	return REC_FILE_DROP;
  		}

  		msglog(INFO_PREDEAL_ERROR,sz_infoPoint,INFO_VALUE_FALSE,sz_errmsg,sz_nowday,sz_errtime,sz_errtime);
      
  		UpdateErrFileReceive(pch_orgname,e.sz_sourceId);
  		sprintf(sz_ErrName,"%s%s%s",e.sz_sourcePath,sz_fileErrPath,pch_orgname);

      expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"move the file to ErrDir");
      
      if(c_copyFlag=='Y')
      {
      	if(copyFile(sz_inFilename,sz_ErrName)!=0)
        {
        	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"copy File Err!");
          expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
      	  sprintf(sz_errmsg,"the file (%s) copy to (%s) Err!",sz_inFilename,sz_ErrName);
  	      getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,sz_inFilename,'E','H',FILEREC_ERR_COPY,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
	  	    DBConn.Rollback();
  		    return REC_FILE_ERR;
  		  }
  		  if(unlink(sz_inFilename))
  		  {
  		  	sprintf(sz_errmsg,"the file (%s) unlink Err!",sz_inFilename);
  	      getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,sz_inFilename,'E','H',FILEREC_ERR_UNLINK,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
          DBConn.Rollback();
  	      return REC_FILE_ERR;
  		  }
      }
      else
      {
  		  if(rename(sz_inFilename,sz_ErrName)!=0)
	  	  {
        	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"File rename Err!");
    	    expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
	  	  	sprintf(sz_errmsg,"the errfile (%s) rename to (%s) Err!",sz_inFilename,sz_ErrName);
  	      getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,sz_inFilename,'E','H',FILEREC_ERR_RENAME,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	  	DBConn.Rollback();
          return REC_FILE_ERR;
	  	  }
	  	}
	  	if(c_UzipFlag=='Y')
      {
	  	  unlink(sz_unzipFile);
	  	}
	    DBConn.Rollback();

      expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"File repeat in SCH_Table Err!");
      expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
	  	return REC_FILE_ERR;
  	}

    if(e.c_SerialFlag=='Y')
    {
      expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"Update the SERIALINFO table!");
      if(iCheckSerialFlag>=0)
	      UpdateSerialInfo(e.sz_sourceId,sz_serialDay,iTmpSerialNo);
    }

    try
    {
      DealLog.DealLog_Recv(sz_realFileName,sz_nowtime,iFileSize,sz_orgName);
      collectLog(iWorkFlow,iProcessId,e.sz_sourceId,sz_realFileName,iFileSize,sz_nowtime);
    }
    catch(CF_CError e)
    {
    	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"Oracle Err!");
      expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
      DBConn.Rollback();
      getCurTime(sz_errtime);
      wrlog(sz_pipeId,iProcessId,sz_realFileName,sz_errtime,e);
  	  return REC_FILE_DROP;
    }
	  
	  char sz_backupFile[255];
	  if(c_BakFlag=='Y')//备份文件到备份目录
	  {
	  	char sz_bakFile[255];
	    if(sz_bakType[0]!='N')
    	{
    		char sz_bakYear[6+1],sz_bakDate[2+1];
	      strncpy(sz_bakYear,sz_bakday,6);
	      sz_bakYear[6]=0;
        if(strlen(sz_bakday)==6)
        {
        	sprintf(sz_outFilename,"%s%s%s/",e.sz_sourcePath,sz_fileBakPath,sz_bakYear);
        }
        else
        {
        	strncpy(sz_bakDate,sz_bakday+6,2);
          sz_bakDate[2]=0;
	        sprintf(sz_outFilename,"%s%s%s/%s/",e.sz_sourcePath,sz_fileBakPath,sz_bakYear,sz_bakDate);
	      }
	      chkAllDir(sz_outFilename);
	      completeDir(sz_outFilename);
	    }
	    else
	    {
	    	sprintf(sz_outFilename,"%s%s",e.sz_sourcePath,sz_fileBakPath);
	    }

    	if(c_bakFrontFlag=='Y')
    	{
    		strcat(sz_outFilename,pch_orgname);
        strcpy(sz_bakFile,sz_inFilename);
 	    }
 	    else
 	    {
    		strcat(sz_outFilename,sz_realFileName);
        if(c_UzipFlag=='Y')
        {
        	 strcpy(sz_bakFile,sz_unzipFile);
        }
        else
        {
          strcpy(sz_bakFile,sz_inFilename);
        }
 	    }
 	    strcpy(sz_backupFile,sz_outFilename);
 	    
      if(copyFile(sz_bakFile,sz_outFilename)!=0)
      {
      	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"copy File Err!");
        expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
      	sprintf(sz_errmsg,"the file (%s) copy to (%s) Err!",sz_bakFile,sz_outFilename);
  	    getCurTime(sz_errtime);
  	    wrlog(sz_pipeId,iProcessId,sz_bakFile,'E','H',FILEREC_ERR_COPY,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
  		  DBConn.Rollback();
  		  return REC_FILE_DROP;
      }
    }
    expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"move the file to outputDir!");
	  sprintf(sz_outFilename,"%s%s%s",e.sz_sourcePath,sz_fileOutPath,sz_realFileName);

    if(c_UzipFlag=='Y')
    {
    	if(c_copyFlag=='Y')
    	{
        if(copyFile(sz_unzipFile,sz_outFilename)!=0)
        {
        	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"copy File Err!");
          expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
      	  sprintf(sz_errmsg,"the file (%s) copy to (%s) Err!",sz_unzipFile,sz_outFilename);
  	      getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,sz_unzipFile,'E','H',FILEREC_ERR_COPY,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
  		    if(c_BakFlag=='Y')
	  	      unlink(sz_backupFile);
	  	    DBConn.Rollback();
  		    return REC_FILE_DROP;  
        }
        if(unlink(sz_unzipFile))
        {
          sprintf(sz_errmsg,"the file (%s) unlink Err!",sz_unzipFile);
  	      getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,sz_unzipFile,'E','H',FILEREC_ERR_UNLINK,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
  	      if(c_BakFlag=='Y')
	  	      unlink(sz_backupFile);
	  	    DBConn.Rollback(); 
  	      return REC_FILE_DROP;
        }
    	}
    	else
    	{
    	  if(rename(sz_unzipFile,sz_outFilename)!=0)
      	{
      		sprintf(sz_errmsg,"the file (%s) rename to (%s) Err!",sz_unzipFile,sz_outFilename);
  	      getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,sz_unzipFile,'E','H',FILEREC_ERR_RENAME,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
  		    if(c_BakFlag=='Y')
	  	      unlink(sz_backupFile);
	  	    DBConn.Rollback();
  		    return REC_FILE_DROP;
      	}
      }
    }
    else
    {
    	if(c_copyFlag=='Y')
      {
        if(copyFile(sz_inFilename,sz_outFilename)!=0)
        {
        	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"copy File Err!");
          expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
      	  sprintf(sz_errmsg,"the file (%s) copy to (%s) Err!",sz_inFilename,sz_outFilename);
  	      getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,sz_inFilename,'E','H',FILEREC_ERR_COPY,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
  		    if(c_BakFlag=='Y')
	  	      unlink(sz_backupFile);
	  	    DBConn.Rollback();
  		    return REC_FILE_DROP;  
        }
    	}
    	else
    	{
	      if(link(sz_inFilename,sz_outFilename)!=0)
  	    {
  	    	unlink(sz_outFilename);
  	    	if(link(sz_inFilename,sz_outFilename)!=0)
          {
            expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"link File Err!");
            expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
  		      sprintf(sz_errmsg,"the file (%s) rename to (%s) Err!",sz_inFilename,sz_outFilename);
  	        getCurTime(sz_errtime);
  	        wrlog(sz_pipeId,iProcessId,sz_inFilename,'E','H',FILEREC_ERR_RENAME,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
  		      if(c_BakFlag=='Y')
	  	        unlink(sz_backupFile);
	  	      DBConn.Rollback();
  		      return REC_FILE_DROP;
  		    }
  		  }
  	  }
  	}
  }
  catch(CF_CError e)
  {
    getCurTime(sz_errtime);
    wrlog(sz_pipeId,iProcessId,"oracleErr",sz_errtime,e);
    DBConn.Rollback();
    return REC_FILE_DROP;
  }
	DBConn.Commit();
	if(unlink(sz_inFilename))
	{
		sprintf(sz_errmsg,"the file (%s) unlink Err!",sz_inFilename);
  	getCurTime(sz_errtime);
    wrlog(sz_pipeId,iProcessId,sz_inFilename,'E','H',FILEREC_ERR_UNLINK,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	      
	}
	
  if(iCheckSerialFlag>=0)
  {	  	  
  	strcpy(e.sz_nowday,sz_serialDay);
	  e.iSerial=iTmpSerialNo;
	  e.iSerial++;
	  if(e.iSerial>e.iMaxSerialValue) e.iSerial-=e.iMaxSerialValue;
	}
		
  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");

	return SUCC;
}

int CFreceive::ProcFileQC(struct SOURCE_INFO &e,char *pch_filename)
{
	char sz_nowtime[14+1],sz_compTime[14+1],sz_orgSourceId[5+1];
  char sz_nowday[8+1],sz_colday[14+1],sz_bakday[8+1],sz_serialDay[8+1];
	char sz_realFileName[255];
	char sz_inFilename[255],sz_outFilename[255],sz_tempFile[255];
	char sz_unzipFile[255];
	char sz_endSuff[20];
	int  iQCRecFlag,iTimeOutFlag;
	int iFileSize,iFileCycle;
  int iBatch,iTmpSerialNo,iCheckSerialFlag;
  struct stat buf;
	time_t timer;
	
  iTimeOutFlag=0;
  sprintf(sz_inFilename,"%s%s%s",e.sz_sourcePath,sz_fileInPath,pch_filename);

  time(&timer);
  stat(sz_inFilename,&buf);

  if(timer-buf.st_mtime<FILEREC_REC_WAITING_TIME)
  {
  	return REC_TIME_NOT_ENOUGH;
  }
  
  strcpy(sz_tempFile,pch_filename);
  if(c_UzipFlag=='Y')
  {
  	char *pch_temp;
  	pch_temp=NULL;
  	if((pch_temp=strrchr(pch_filename,'.'))==NULL)
  	{
  		getCurTime(sz_errtime);
      sprintf(sz_errmsg,"the InFile(%s) Err!",pch_filename);
      wrlog(sz_pipeId,iProcessId,pch_filename,'E','H',FILEREC_ERR_ORGNAME,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
  		MoveToErrPath(e,sz_inFilename,pch_filename);
  		return FAIL;
  	}
  	strcpy(sz_endSuff,pch_temp+1);
  	pch_temp[0]=0;
  }

	try
	{
	  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------start----------------------------------");
    expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"begin to receive the file %s",pch_filename);
    getCurTime(sz_nowtime);
    strncpy(sz_nowday,sz_nowtime,8);
    sz_nowday[8]='\0';
    
    if(c_BillCycleFlag=='Y')
    {
    	char sz_tmpBatch[10];
      if(GetCol(pch_filename,billcyclecol[0],sz_tmpBatch)==FAIL)
      {
      	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"GET BillCycle Err!");
      	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
      	sprintf(sz_errmsg,"file(%s) GET BillCycle Err!",pch_filename);
      	MoveToErrPath(e,sz_inFilename,pch_filename);
	      return REC_FILE_ERR;
      }
      iFileCycle=atoi(sz_tmpBatch);
    }
    
    if(c_DropTimeFlag=='Y')
    {
    	if(strncmp(sz_nowtime,sz_cycleTime,6)!=0)
    	{
    	  if(SelectBillCycle(sz_nowtime,iBillCycle)==FAIL)
    	  {
          getCurTime(sz_errtime);
          sprintf(sz_errmsg,"GET BillCycle(%s) From DB Err!",sz_nowtime);
          wrlog(sz_pipeId,iProcessId,pch_filename,'E','H',FILEREC_ERR_CYCLEOUT,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
          expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"GET BillCycle From DB Err!");
    		  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
    	    return REC_FILE_DROP;
    	  }
    	  strncpy(sz_cycleTime,sz_nowtime,6);
    		sz_cycleTime[6]=0;
    	}
    	if(iBillCycle>iFileCycle)
    	{
    		if(iBillCycle-iFileCycle>1)
    		{
    			iTimeOutFlag=1;
    		}
    		else if(strncmp(sz_nowtime+6,sz_dropTime,8)>0)
    		{
          iTimeOutFlag=1;
    		}
    	}
    }
    
    iQCRecFlag=CheckFileInFileReceive(e.sz_sourceId,pch_filename);

	  if(iQCRecFlag==FILEREC_REPEAT_FILE)
	  {
	  	char sz_repeatYear[6+1],sz_repeatDay[2+1];
	    strncpy(sz_repeatYear,sz_nowtime,6);
	    sz_repeatYear[6]=0;
	    strncpy(sz_repeatDay,sz_nowtime+6,2);
      sz_repeatDay[2]=0;
	    sprintf(sz_outFilename,"%srepeat_file/%s/%s/",e.sz_sourcePath,sz_repeatYear,sz_repeatDay);
	  	chkAllDir(sz_outFilename);
      expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"Insert the file to REPEAT_FILE table");

	  	sprintf(sz_outFilename,"%s%s.%s",sz_outFilename,sz_tempFile,sz_nowtime);
	  	InsertRepeatFile(sz_tempFile,e.sz_sourceId,sz_nowtime);

    	if(c_copyFlag=='Y')//move to repeat
    	{
        if(copyFile(sz_inFilename,sz_outFilename)!=0)
        {
        	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"copy File Err!");
          expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
      	  sprintf(sz_errmsg,"the file (%s) copy to (%s) Err!",sz_inFilename,sz_outFilename);
  	      getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,sz_inFilename,'E','H',FILEREC_ERR_COPY,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
  		    return SUCC;  
        }
        if(unlink(sz_inFilename))
        {
        	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"unlink File Err!");
        	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
          sprintf(sz_errmsg,"the file (%s) unlink Err!",sz_inFilename);
  	      getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,sz_inFilename,'E','H',FILEREC_ERR_UNLINK,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
  	      return SUCC;
        }
    	}
      else
      {       		
        if(rename(sz_inFilename,sz_outFilename)!=0)
        {
        	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"rename File Err!");
        	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
          sprintf(sz_errmsg,"move the file from(%s)to(%s)error",sz_inFilename,sz_outFilename);
          getCurTime(sz_errtime);
      	  wrlog(sz_pipeId,iProcessId,pch_filename,'O','S',errno,FILEREC_ERR_RENAME,sz_errtime,sz_errmsg,__FILE__,__LINE__);
          return SUCC;
        }
      }
      expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"the file is repeat_file!!");
      expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
	    DBConn.Commit();
	    return REC_FILE_REPEAT;
	  }

  	iFileSize=GetFileSize(sz_inFilename);

  	if(((iFileSize>=e.iLargeFsize)&&(e.iLargeFsize>0))||(iFileSize<=e.iSmallFsize)&&(e.iSmallFsize>0))
    {
    	sprintf(sz_errmsg,"find the large or small file(%s)",pch_filename);
  	  getCurTime(sz_errtime);
  	  wrlog(sz_pipeId,iProcessId,pch_filename,'E','M',FILEREC_ERR_LARGE_SMALL_FILE,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
      if(c_CheckFile=='Y')
      {
    	  MoveToErrPath(e,sz_inFilename,pch_filename);
	  		DBConn.Rollback();
        expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"the file is large or small file!");
        expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
	  	  return REC_FILE_ERR;
      }
    }

	  if(c_OrgSourceFlag=='Y')
	  {
		  if(GetCol(pch_filename,sourcecol[0],sz_orgSourceId)==FAIL)
		  {
			  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"GET orgSource Err!");
        expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
			  sprintf(sz_errmsg,"file(%s) GET orgSource Err!",pch_filename);
  	    getCurTime(sz_errtime);
  	    wrlog(sz_pipeId,iProcessId,pch_filename,'E','M',FILEREC_ERR_GETCOL,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	  return REC_FILE_DROP;
		  }
	  }
    else
    {
	    strcpy(sz_orgSourceId,e.sz_sourceId);
	  }

    if(c_serialTimeFlag=='Y')//获取序列号时间
    {
    	if(GetCol(pch_filename,serialtimecol[0],sz_serialDay)==FAIL)
    	{
    		expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"GET serialDay Err!");
    	  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
    		sprintf(sz_errmsg,"file(%s) GET serialDay Err!",pch_filename);
    	  MoveToErrPath(e,sz_inFilename,pch_filename);
	    	return REC_FILE_ERR;
    	}
    }
    else
    {
    	strcpy(sz_serialDay,sz_nowday);
    }

	  if(e.c_SerialFlag=='Y')
	  {
      iCheckSerialFlag=strcmp(sz_serialDay,e.sz_nowday);
	    if(iCheckSerialFlag>0)
	    {
	  	  iTmpSerialNo=1;
	    }
	    else if(iCheckSerialFlag==0)
	    {
	    	iTmpSerialNo=e.iSerial;
	    }
	    else if(iCheckSerialFlag<0)
	    {
	    	iTmpSerialNo=GetMaxSerial(e.sz_sourceId,sz_serialDay);
	    	iTmpSerialNo++;
	    	if(iTmpSerialNo>e.iMaxSerialValue) iTmpSerialNo=1;
	    }
    }
    
    if((cQcBakOnlyFlag!='Y')&&(c_BakFlag=='Y'))
    {
      if(sz_bakType[0]=='Y')//获取备份方式
      {
      	strcpy(sz_bakday,sz_nowday);
      }
      else if(sz_bakType[0]=='D')
      {
      	strcpy(sz_bakday,sz_nowday);
      }
      else if(sz_bakType[0]=='M')
      {
      	strncpy(sz_bakday,sz_nowday,6);
      	sz_bakday[6]='\0';
      }
      else if(sz_bakType[0]!='N')
      {
      	if(GetCol(pch_filename,bakdatecol[0],sz_bakday)==FAIL)
	    	{
          expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"GET bakDate Err!");
          expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
	  	  	sprintf(sz_errmsg,"file(%s) GET bakDate Err!",pch_filename);
    	    getCurTime(sz_errtime);
    	    wrlog(sz_pipeId,iProcessId,pch_filename,'E','M',FILEREC_ERR_GETCOL,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	    return REC_FILE_DROP;
	  	  }
      }
    }

    if(c_OrgBatchFlag=='Y')
    {
  	  char sz_tmpBatch[10];
  	  if(GetCol(pch_filename,batchcol[0],sz_tmpBatch)==FAIL)
  	  {
        expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"GET orgBatch Err!");
        expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
  	    sprintf(sz_errmsg,"file(%s) GET orgBatch Err!",pch_filename);
  	    getCurTime(sz_errtime);
  	    wrlog(sz_pipeId,iProcessId,pch_filename,'E','M',FILEREC_ERR_GETCOL,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	  return REC_FILE_DROP;
  	  }
  	  iBatch=atoi(sz_tmpBatch);
    }
    else
    {
  	  GetBatchId(sz_nowtime,iBatch);
    }

    if(c_OrgColDateFlag=='Y')
    {
    	if(GetCol(pch_filename,datecol[0],sz_colday)==FAIL)
    	{
    		expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"GET orgColDate Err!");
        expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
    	  sprintf(sz_errmsg,"file(%s) GET orgColDate Err!",pch_filename);
  	    getCurTime(sz_errtime);
  	    wrlog(sz_pipeId,iProcessId,pch_filename,'E','M',FILEREC_ERR_GETCOL,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	  return REC_FILE_DROP;
    	}
    }
    else
    {
  	  strcpy(sz_colday,sz_nowday);
    }
    if(c_serialTimeFlag=='Y')
    {
    	strcpy(sz_colday,sz_serialDay);
    }
    
	  if(c_renameFlag=='Y')//获取新文件名
    {
    	if(GetFileName(sz_realFileName,sz_orgSourceId,sz_colday,iBatch,iTmpSerialNo,pch_filename,e)==FAIL)
    	{
    		expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"changeName Err!!");
        expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
    		sprintf(sz_errmsg,"file(%s) changeName Err!",pch_filename);
  	    getCurTime(sz_errtime);
  	    wrlog(sz_pipeId,iProcessId,pch_filename,'E','M',FILEREC_ERR_CHANGENAME,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	  return REC_FILE_DROP;
    	}
    	iTmpSerialNo++;
    }
    else
    {
    	  strcpy(sz_realFileName,pch_filename);
    }

	  if(iQCRecFlag==FILEREC_DROP_FILE)
	  {
	  	if(iTimeOutFlag==1)
	  	{
	  	   InsertFileReceive(e.sz_sourceId,sz_realFileName,pch_filename,iBatch,iFileSize,sz_orgSourceId,sz_colday,sz_nowtime,sz_nowtime,'Q','O');
	  	}
	  	else
	  	{
	  	  InsertFileReceive(e.sz_sourceId,sz_realFileName,pch_filename,iBatch,iFileSize,sz_orgSourceId,sz_colday,sz_nowtime,sz_nowtime,'Q',c_FileType);
	    }
	  }
    else if(iTimeOutFlag==1)
    {
    	if(iQCRecFlag==FILEREC_MANY_QFILE)
        UpdateFileReceive(e.sz_sourceId,pch_filename,sz_realFileName,iBatch,sz_colday,'O',iFileSize);
      else
        UpdateFileReceive(e.sz_sourceId,pch_filename,sz_realFileName,iBatch,sz_colday,'O',0);
    }
    else
    {
    	if(iQCRecFlag==FILEREC_MANY_QFILE)
        UpdateFileReceive(e.sz_sourceId,pch_filename,sz_realFileName,iBatch,sz_colday,c_FileType,iFileSize);
      else
        UpdateFileReceive(e.sz_sourceId,pch_filename,sz_realFileName,iBatch,sz_colday,c_FileType,0);
      if(InsertSchFormat(sz_realFileName,e.sz_sourceId)==FILEREC_SCH_REPEAT)
      {
  		  getCurTime(sz_errtime);
	  	  sprintf(sz_errmsg,"Err insert sch table!");
  		  msglog(INFO_PREDEAL_ERROR,sz_infoPoint,INFO_VALUE_FALSE,sz_errmsg,sz_nowday,sz_errtime,sz_errtime);
  		  wrlog(sz_pipeId,iProcessId,pch_filename,'E','M',FILEREC_ERR_INSERT_SCH,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);

      	if(sz_DebugFlag[0]=='Y')
        {
           expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"Insert the file SchFmormat Err");
	      }

  	    UpdateErrFileReceive(pch_filename,e.sz_sourceId);
  	  
  	    char sz_repeatSchOname[255],sz_repeatSchRname[255];
  	    sprintf(sz_repeatSchRname,"%s%s",e.sz_sourcePath,sz_fileErrPath);
  	    chkAllDir(sz_repeatSchRname);
	      completeDir(sz_repeatSchRname);

	      strcat(sz_repeatSchRname,sz_tempFile);
	    
        expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"move the file to ErrDir");

  	    if(c_copyFlag=='Y')
    	  {
          if(copyFile(sz_inFilename,sz_repeatSchRname)!=0)
          {
          	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"the file copy Err!"); 
            expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
        	  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"copy File Err!");
            expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
      	    sprintf(sz_errmsg,"the file (%s) copy to (%s) Err!",sz_inFilename,sz_repeatSchRname);
  	        getCurTime(sz_errtime);
  	        wrlog(sz_pipeId,iProcessId,sz_inFilename,'E','H',FILEREC_ERR_COPY,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
  		      return FAIL;  
          }
          if(unlink(sz_inFilename))
          {
          	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"the file unlink Err!"); 
            expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
            sprintf(sz_errmsg,"the file (%s) unlink Err!",sz_inFilename);
  	        getCurTime(sz_errtime);
  	        wrlog(sz_pipeId,iProcessId,sz_inFilename,'E','H',FILEREC_ERR_UNLINK,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
  	        return FAIL;
          }
    	  }
    	  else
    	  {
  	      if(rename(sz_inFilename,sz_repeatSchRname)!=0)
	        {
            expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"the file rename Err!"); 
            expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
	        	sprintf(sz_errmsg,"the ErrFile (%s) rename to (%s) Err!",sz_repeatSchOname,sz_repeatSchOname);
  	        getCurTime(sz_errtime);
  	        wrlog(sz_pipeId,iProcessId,sz_repeatSchOname,'E','M',FILEREC_ERR_RENAME,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	      DBConn.Rollback();
            return FAIL;
          }
	      }

	      if(e.c_SerialFlag=='Y')
	      {
	      	if(iCheckSerialFlag>=0)
          {
            UpdateSerialInfo(e.sz_sourceId,sz_serialDay,iTmpSerialNo);
            strcpy(e.sz_nowday,sz_serialDay);
	  	      e.iSerial=iTmpSerialNo;
	  	      e.iSerial++;
	  	      if(e.iSerial>e.iMaxSerialValue) e.iSerial=1;
          }
	      }
	      DBConn.Rollback();

        expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"the file proc end!");
        
	      return REC_FILE_ERR;
      }
    }
    if(c_UzipFlag=='Y')
	  {
		  char sz_buff[255]; 
		  sprintf(sz_unzipFile,"%s.TMP.%s",sz_inFilename,sz_endSuff);
		  copyFile(sz_inFilename,sz_unzipFile);
		  sprintf(sz_buff,"%s %s",sz_shFile,sz_unzipFile);

      if(system(sz_buff))
      {
        sprintf(sz_errmsg,"System: %s err",sz_buff);
    	  getCurTime(sz_errtime);
    	  wrlog(sz_pipeId,iProcessId,"System",'E','M',FILEREC_ERR_COMMAND,errno,sz_errtime,sz_errmsg,(char *)__FILE__,__LINE__);
	  	  DBConn.Rollback();
        return REC_FILE_DROP;
      }
      sz_unzipFile[strlen(sz_unzipFile)-strlen(sz_endSuff)-1]=0;
    }

	  if(e.c_SerialFlag=='Y')
	  {
      expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"Update the SERIALINFO table!");
      if(iCheckSerialFlag>=0)
	      UpdateSerialInfo(e.sz_sourceId,sz_serialDay,iTmpSerialNo);
    }

    char sz_backupFile[255];
    if((cQcBakOnlyFlag!='Y')&&(c_BakFlag=='Y'))
    {
    	char sz_bakFile[255];
    	
    	if(sz_bakType[0]!='N')
    	{
    		char sz_bakYear[6+1],sz_bakDate[2+1];
	      strncpy(sz_bakYear,sz_bakday,6);
	      sz_bakYear[6]=0;
	      if(strlen(sz_bakday)==6)
	      {
	        sprintf(sz_outFilename,"%s%s%s/",e.sz_sourcePath,sz_fileBakPath,sz_bakYear);
	      }
	      else
	      {
	        strncpy(sz_bakDate,sz_bakday+6,2);
          sz_bakDate[2]=0;
	        sprintf(sz_outFilename,"%s%s%s/%s/",e.sz_sourcePath,sz_fileBakPath,sz_bakYear,sz_bakDate);
	      }
	      chkAllDir(sz_outFilename);
	      completeDir(sz_outFilename);
	    }
	    else
	    {
	    	sprintf(sz_outFilename,"%s%s",e.sz_sourcePath,sz_fileBakPath);
	    }
    	
    	if(c_bakFrontFlag=='Y')
    	{
  	    strcat(sz_outFilename,sz_tempFile);
        strcpy(sz_bakFile,sz_inFilename);
 	    }
 	    else
 	    {
 	    	strcat(sz_outFilename,sz_realFileName);
        if(c_UzipFlag=='Y')
        {
        	 strcpy(sz_bakFile,sz_unzipFile);
        }
        else
        {
          strcpy(sz_bakFile,sz_inFilename);
        }
 	    }
 	    strcpy(sz_backupFile,sz_outFilename);
 	    if(copyFile(sz_bakFile,sz_outFilename)!=0)
      {
        expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"the file copy Err!"); 
        expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
      	sprintf(sz_errmsg,"the file (%s) copy to (%s) Err!",sz_bakFile,sz_outFilename);
  	    getCurTime(sz_errtime);
  	    wrlog(sz_pipeId,iProcessId,sz_bakFile,'E','M',FILEREC_ERR_COPY,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
  		  DBConn.Rollback();
  		  return REC_FILE_DROP;
      }
    }

    if(iTimeOutFlag==0)
    {
  	  sprintf(sz_outFilename,"%s%s%s",e.sz_sourcePath,sz_fileOutPath,sz_realFileName);
    }
    else
    {
    	sprintf(sz_outFilename,"%s%s%s.timeout",e.sz_sourcePath,sz_fileErrPath,sz_realFileName);
    }
    
    expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"move the file to outputDir!");

	  if(c_UzipFlag=='Y')
	  {
    	if(c_copyFlag=='Y')
    	{
        if(copyFile(sz_unzipFile,sz_outFilename)!=0)
        {
          expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"the file copy Err!"); 
          expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
      	  sprintf(sz_errmsg,"the file (%s) copy to (%s) Err!",sz_unzipFile,sz_outFilename);
  	      getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,sz_unzipFile,'E','H',FILEREC_ERR_COPY,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
  		    if((cQcBakOnlyFlag!='Y')&&(c_BakFlag=='Y'))
	  	      unlink(sz_backupFile);
	  	    DBConn.Rollback();
  		    return REC_FILE_DROP;  
        }
        if(unlink(sz_unzipFile))
        {
          expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"the file unlink Err!"); 
          expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
          sprintf(sz_errmsg,"the file (%s) unlink Err!",sz_unzipFile);
  	      getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,sz_unzipFile,'E','H',FILEREC_ERR_UNLINK,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
  	      if((cQcBakOnlyFlag!='Y')&&(c_BakFlag=='Y'))
	  	      unlink(sz_backupFile);
	  	    DBConn.Rollback(); 
  	      return REC_FILE_DROP;
        }
    	}
    	else
    	{
	    	if(rename(sz_unzipFile,sz_outFilename)!=0)
        {
        	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"the file rename Err!"); 
          expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
          sprintf(sz_errmsg,"the Recfile (%s) rename to (%s) Err!",sz_unzipFile,sz_outFilename);
  	      getCurTime(sz_errtime);
          wrlog(sz_pipeId,iProcessId,sz_unzipFile,'E','M',FILEREC_ERR_RENAME,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
          if((cQcBakOnlyFlag!='Y')&&(c_BakFlag=='Y'))
	  	      unlink(sz_backupFile);
          DBConn.Rollback();
  	      return REC_FILE_DROP;
  	    }
      }
	  }
	  else
	  {
	  	if(c_copyFlag=='Y')
      {
        if(copyFile(sz_inFilename,sz_outFilename)!=0)
        {
        	expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"the file copy Err!"); 
          expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
      	  sprintf(sz_errmsg,"the file (%s) copy to (%s) Err!",sz_inFilename,sz_outFilename);
  	      getCurTime(sz_errtime);
  	      wrlog(sz_pipeId,iProcessId,sz_inFilename,'E','H',FILEREC_ERR_COPY,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
  		    if((cQcBakOnlyFlag!='Y')&&(c_BakFlag=='Y'))
	  	      unlink(sz_backupFile);
	  	    DBConn.Rollback();
  		    return REC_FILE_DROP;  
        }
    	}
    	else
    	{
	      if(link(sz_inFilename,sz_outFilename)!=0)
  	    {
  	      unlink(sz_outFilename);
  	      if(link(sz_inFilename,sz_outFilename)!=0)
          {
             expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"the file link Err!"); 
             expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
  		       sprintf(sz_errmsg,"the file (%s) rename to (%s) Err!",sz_inFilename,sz_outFilename);
  	         getCurTime(sz_errtime);
  	         wrlog(sz_pipeId,iProcessId,sz_inFilename,'E','M',FILEREC_ERR_RENAME,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
  		       if((cQcBakOnlyFlag!='Y')&&(c_BakFlag=='Y'))
	  	         unlink(sz_backupFile);
	  	       DBConn.Rollback();
  	         return REC_FILE_DROP;
  		    }
  		  } 
  		}
    }
    if(iCheckSerialFlag>=0)
    {
      strcpy(e.sz_nowday,sz_serialDay);
	    e.iSerial=iTmpSerialNo;
     if(e.iSerial>e.iMaxSerialValue) e.iSerial=1;
    }
    
    collectLog(iWorkFlow,iProcessId,e.sz_sourceId,sz_realFileName,iFileSize,sz_nowtime);
    
	  DBConn.Commit();
	  if(unlink(sz_inFilename))
	  {
		  sprintf(sz_errmsg,"the file (%s) unlink Err!",sz_inFilename);
  	  getCurTime(sz_errtime);
      wrlog(sz_pipeId,iProcessId,sz_inFilename,'E','H',FILEREC_ERR_UNLINK,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	      
	  }
    expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"the file proc end!"); 
    expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
  }
  catch(CF_CError e)
  {
  	getCurTime(sz_errtime);
    wrlog(sz_pipeId,iProcessId,"oracleErr",sz_errtime,e);
    DBConn.Rollback();
    return REC_FILE_DROP;
  }
  return SUCC;
}

int CFreceive::InsertQCToFileRec(struct SOURCE_INFO &s,char *pch_filename,int iQcTypeNo)
{
	char sz_openFile[255],sz_temp[255],sz_buff[255],sz_orgSourceId[5+1];
	char sz_duration[255],sz_fee[255],sz_filename[255],sz_nowtime[14+1];
	int iRecord,iFileSize,iNum;
	FILE *fp;
	struct stat buf;
	time_t timer;

  CBindSQL bs(DBConn);

  sprintf(sz_openFile,"%s%s%s",s.sz_sourcePath,sz_fileInPath,pch_filename);
  getCurTime(sz_nowtime);

  time(&timer);
  stat(sz_openFile,&buf);

  if(timer-buf.st_mtime<FILEREC_REC_WAITING_TIME)
  {
  	
  	return SUCC;
  }
  
  try
	{
	  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------start----------------------------------");
    expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"begin to receive the QC file %s",sz_openFile);
    
    if(c_QCtableFlag=='Y')
    {
    	iNum=0;
    	char sz_qcReceiveTime[14+1];
    	bs.Open("SELECT max(RECEIVE_TIME) FROM QC_RECEIVED WHERE QC_FILENAME=:1 and SOURCE_ID=:2 ",SELECT_QUERY);
      bs<<pch_filename<<s.sz_sourceId;
      while(bs>>sz_qcReceiveTime)
      {
      	iNum++;
      }
      bs.Close();
    
      if(iNum!=0)
      {
      	if(sourcelist[iSourceNo].iRereceiveDay!=0)
      	{
          long lTmp;
          time_t timer;
          lTmp=timeStr2Time(sz_qcReceiveTime);
          time(&timer);
          lTmp=(timer-lTmp)/60/60/24;
          if(lTmp<=sourcelist[iSourceNo].iRereceiveDay)
          {
	          bs.Open("INSERT INTO REPEAT_FILE(FILENAME ,REPEAT_TIME,SOURCE_ID) VALUES(:1,:2,:3)",NONSELECT_DML);
            bs<<pch_filename<<sz_nowtime<<s.sz_sourceId;
            bs.Execute();
	          bs.Close();
	          
	          expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"the QC file is repeat file");
	          char sz_repeatYear[6+1],sz_repeatDay[2+1];
	          char sz_repeatRname[255];
	          strncpy(sz_repeatYear,sz_nowtime,6);
	          sz_repeatYear[6]=0;
	          strncpy(sz_repeatDay,sz_nowtime+6,2);
            sz_repeatDay[2]=0;
	          sprintf(sz_repeatRname,"%srepeat_file/%s/%s",s.sz_sourcePath,sz_repeatYear,sz_repeatDay);
	          chkAllDir(sz_repeatRname);
	          completeDir(sz_repeatRname);
	          sprintf(sz_repeatRname,"%s%s.%s",sz_repeatRname,pch_filename,sz_nowtime);

            expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"move QC file to repeat DIR");
            
            if(c_copyFlag=='Y')
            {
      	      if(copyFile(sz_openFile,sz_repeatRname)!=0)
              {
            	  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"copy File Err!");
                expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"----------------------------end----------------------------------");
      	        sprintf(sz_errmsg,"the file (%s) copy to (%s) Err!",sz_openFile,sz_repeatRname);
  	            getCurTime(sz_errtime);
  	            wrlog(sz_pipeId,iProcessId,sz_openFile,'E','H',FILEREC_ERR_COPY,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
	  	          DBConn.Rollback();
  		          return REC_FILE_DROP;
  		        }
  		        if(unlink(sz_openFile))
  		        {
  		        	sprintf(sz_errmsg,"the file (%s) unlink Err!",sz_openFile);
  	            getCurTime(sz_errtime);
  	            wrlog(sz_pipeId,iProcessId,sz_openFile,'E','H',FILEREC_ERR_UNLINK,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
                DBConn.Rollback();
  	            return REC_FILE_DROP;
  		        }
            }
            else
            {
	            if(rename(sz_openFile,sz_repeatRname)!=0)
	            {
	          	  sprintf(sz_errmsg,"the repeatfile (%s) rename to (%s) Err!",sz_openFile,sz_repeatRname);
  	            getCurTime(sz_errtime);
  	            wrlog(sz_pipeId,iProcessId,sz_openFile,'E','H',FILEREC_ERR_RENAME,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  	          DBConn.Rollback();
                return REC_FILE_DROP;
	            }
	          }
	          expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"QC file is repeat file");
	          expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"-----------------------------end-----------------------------------");
	          DBConn.Commit();
	          return SUCC;
	        }
	      }
      }
      InsertQcReceived(pch_filename,s.sz_sourceId);
    }
 
	  if((fp=fopen(sz_openFile,"r"))==NULL)
	  {
	    getCurTime(sz_errtime);
	    sprintf(sz_errmsg,"open the file (%s) Err!",sz_openFile);
    	wrlog(sz_pipeId,iProcessId,pch_filename,'E','M',FILEREC_ERR_OPEN_FILE,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	    return FAIL;
	  }
	  
	  while(1)
  	{
	    if(fgets(sz_temp,600,fp)==NULL) break;
	    if(strncmp(sz_temp,"SOF",3)==0) continue;
	    if(strncmp(sz_temp,"wEND",4)==0) continue;
	    if(strlen(sz_temp)==0) continue;
	    if(sz_temp[strlen(sz_temp)-1]=='\n') sz_temp[strlen(sz_temp)-1]=0;

	    QCRecord.Set_record(sz_temp);

	    QCRecord.Get_Field(QCRecordlist[iQcTypeNo].iRecord[0][0],sz_buff);
	    delSpace(sz_buff,0);
	    strcpy(sz_filename,sz_buff);

	    iFileSize=0;
	    for(int i=0;i<QCRecordlist[iQcTypeNo].iNum[1];i++)
	    {
	      QCRecord.Get_Field(QCRecordlist[iQcTypeNo].iRecord[1][0],sz_buff);
	      delSpace(sz_buff,0);
	      iFileSize=atoi(sz_buff);
	    }
	    
      iRecord=0;
	    for(int i=0;i<QCRecordlist[iQcTypeNo].iNum[2];i++)
	    {
	      QCRecord.Get_Field(QCRecordlist[iQcTypeNo].iRecord[2][0],sz_buff);
	      delSpace(sz_buff,0);
	      iRecord=atoi(sz_buff);
      }

      sz_fee[0]=0;
	    for(int i=0;i<QCRecordlist[iQcTypeNo].iNum[3];i++)
	    {
	      QCRecord.Get_Field(QCRecordlist[iQcTypeNo].iRecord[3][i],sz_buff);
	      delSpace(sz_buff,0);
	      sprintf(sz_fee,"%s%s|",sz_fee,sz_buff);
      }
      sz_fee[strlen(sz_fee)-1]=0;

      sz_duration[0]=0;
      for(int i=0;i<QCRecordlist[iQcTypeNo].iNum[4];i++)
      {
	      QCRecord.Get_Field(QCRecordlist[iQcTypeNo].iRecord[4][i],sz_buff);
	      delSpace(sz_buff,0);
	      sprintf(sz_duration,"%s%s|",sz_duration,sz_buff);
      }
      sz_duration[strlen(sz_duration)-1]=0;

      if(c_OrgSourceFlag=='Y')
	    {
		    strncpy(sz_orgSourceId,sz_filename,5);
		    sz_orgSourceId[5]=0;
	    }
	    else
	    {
	    	strcpy(sz_orgSourceId,s.sz_sourceId);
	    }
      try
      { 
      	char cDealFlag;
      	char cFileType;
      	char sz_realName[255];
      	int  iTmpNum;
      	cDealFlag=0;
      	
      	bs.Open("SELECT COUNT(FILENAME) FROM FILE_RECEIVED WHERE SOURCE_ID=:1 AND ORG_NAME=:2 AND DEAL_FLAG=:3",SELECT_QUERY);
        bs<<s.sz_sourceId<<sz_filename<<'Q';
        bs>>iTmpNum;
        bs.Close();
        
        if(iTmpNum==1)
        {
      	  bs.Open("select deal_flag,filename,file_type,filesize from file_received where source_id=:1 and org_name=:2 AND DEAL_FLAG=:3",SELECT_QUERY);
          bs<<s.sz_sourceId<<sz_filename<<'Q';
          bs>>cDealFlag>>sz_realName>>cFileType;
          bs.Close();
        }
        else if(iTmpNum>1)
        {
      	  bs.Open("select deal_flag,filename,file_type,filesize from file_received where source_id=:1 and org_name=:2 AND DEAL_FLAG=:3 AND FILESIZE=:4",SELECT_QUERY);
          bs<<s.sz_sourceId<<sz_filename<<'Q'<<iFileSize;
          bs>>cDealFlag>>sz_realName>>cFileType;
          bs.Close();
        }
        
        if(cDealFlag=='Q')
        {
        	bs.Open("UPDATE FILE_RECEIVED SET DEAL_FLAG=:1,FILESIZE=:2,RECORDNUM=:3,DURATION=:4,FEE=:5 WHERE SOURCE_ID=:6 AND ORG_NAME=:7 AND DEAL_FLAG=:8 AND FILENAME=:9",NONSELECT_DML);
          bs<<'Y'<<iFileSize<<iRecord<<sz_duration<<sz_fee<<s.sz_sourceId<<sz_filename<<'Q'<<sz_realName;
          bs.Execute();
          bs.Close();
          if(cFileType!='O')
            InsertSchFormat(sz_realName,s.sz_sourceId);
        }
        else if(c_FileType=='D')//分拣
        {
          bs.Open("INSERT INTO FILE_RECEIVED(SOURCE_ID,FILE_TYPE,FILENAME,ORG_NAME,DEAL_FLAG,FILESIZE,RECORDNUM,DURATION,FEE,ROLLBACK_FLAG,ORG_SOURCE_ID,PROCESS_ID,RECEIVE_TIME,FILE_ID) VALUES (:1,:2,:3,:4,:5,:6,:7,:8,:9,:10,:11,:12,:13,:14)",NONSELECT_DML);
          bs<<s.sz_sourceId<<c_FileType<<sz_filename<<sz_filename<<'W'<< iFileSize<<iRecord<<sz_duration<<sz_fee<<'N'<<sz_orgSourceId<<iProcessId<<sz_nowtime<<-1;
          bs.Execute();
          bs.Close();
        }
        else
        {
          bs.Open("INSERT INTO FILE_RECEIVED(SOURCE_ID,FILE_TYPE,FILENAME,ORG_NAME,DEAL_FLAG,FILESIZE,RECORDNUM,DURATION,FEE,ROLLBACK_FLAG,ORG_SOURCE_ID,PROCESS_ID,RECEIVE_TIME,FILE_ID) VALUES (:1,:2,:3,:4,:5,:6,:7,:8,:9,:10,:11,:12,:13,S_FILE_RECEIVED.NEXTVAL)",NONSELECT_DML);
          bs<<s.sz_sourceId<<c_FileType<<sz_filename<<sz_filename<<'W'<< iFileSize<<iRecord<<sz_duration<<sz_fee<<'N'<<sz_orgSourceId<<iProcessId<<sz_nowtime;
          bs.Execute();
          bs.Close();
        }
      }
      catch(CF_CError e)
      {
      	if(e.get_appErrorCode()==1)
      	{
          char sz_rectime[14+1];
          bs.Open("SELECT RECEIVE_TIME FROM FILE_RECEIVED WHERE SOURCE_ID=:1 and ORG_NAME=:2",SELECT_QUERY);
          bs<<s.sz_sourceId<<sz_filename;
          bs>>sz_rectime;
          bs.Close();
          if(sourcelist[iSourceNo].iRereceiveDay>=0)
          {
            long lTmp;
            time_t timer;
            lTmp=timeStr2Time(sz_rectime);
            time(&timer);
            lTmp=(timer-lTmp)/60/60/24;
            if((lTmp>sourcelist[iSourceNo].iRereceiveDay)||(sourcelist[iSourceNo].iRereceiveDay==0))
            {
            	char sz_tempTime[14+1],sz_tmpName[255];
            	getCurTime(sz_tempTime);
            	sprintf(sz_tmpName,"%s.%s",sz_filename,sz_tempTime);
              if(c_FileType=='D')//分拣
              {
                bs.Open("INSERT INTO FILE_RECEIVED(SOURCE_ID,FILE_TYPE,FILENAME,ORG_NAME,DEAL_FLAG,FILESIZE,RECORDNUM,DURATION,FEE,ROLLBACK_FLAG,ORG_SOURCE_ID,PROCESS_ID,RECEIVE_TIME,FILE_ID) VALUES (:1,:2,:3,:4,:5,:6,:7,:8,:9,:10,:11,:12,:13,:14)",NONSELECT_DML);
                bs<<s.sz_sourceId<<c_FileType<<sz_tmpName<<sz_filename<<'W'<< iFileSize<<iRecord<<sz_duration<<sz_fee<<'N'<<sz_orgSourceId<<iProcessId<<sz_nowtime<<-1;
                bs.Execute();
                bs.Close();
              }
              else
              {
                bs.Open("INSERT INTO FILE_RECEIVED(SOURCE_ID,FILE_TYPE,FILENAME,ORG_NAME,DEAL_FLAG,FILESIZE,RECORDNUM,DURATION,FEE,ROLLBACK_FLAG,ORG_SOURCE_ID,PROCESS_ID,RECEIVE_TIME,FILE_ID) VALUES (:1,:2,:3,:4,:5,:6,:7,:8,:9,:10,:11,:12,:13,S_FILE_RECEIVED.NEXTVAL)",NONSELECT_DML);
                bs<<s.sz_sourceId<<c_FileType<<sz_tmpName<<sz_filename<<'W'<< iFileSize<<iRecord<<sz_duration<<sz_fee<<'N'<<sz_orgSourceId<<iProcessId<<sz_nowtime;
                bs.Execute();
                bs.Close();
              }
            }
      		  continue;
      		}
      	}
      	DBConn.Rollback();
        fclose(fp);
        getCurTime(sz_errtime);
  	    wrlog(sz_pipeId,iProcessId,sz_openFile,sz_errtime,e);
        return FAIL;
      }
    }
  }
  catch(CF_CError e)
  {
    fclose(fp);
    getCurTime(sz_errtime);
  	wrlog(sz_pipeId,iProcessId,sz_openFile,sz_errtime,e);
  	try
    {
      DBConn.Rollback();
    }
    catch(...)
    {
    }
    return FAIL;
  }
  fclose(fp);
  
  if(c_BakFlag=='Y')
  {
    char sz_bakday[20];
    if(sz_bakType[0]=='Y')//获取备份方式
    {
      strncpy(sz_bakday,sz_nowtime,6);
      sz_bakday[6]='\0';
      strcat(sz_bakday,(char *)"/");
      strncat(sz_bakday,sz_nowtime+6,2);
      sz_bakday[9]='\0';
      strcat(sz_bakday,(char *)"/");
    }
    else if(sz_bakType[0]=='D')
    {
      strncpy(sz_bakday,sz_nowtime,6);
      sz_bakday[6]='\0';
      strcat(sz_bakday,(char *)"/");
      strncat(sz_bakday,sz_nowtime+6,2);
      sz_bakday[9]='\0';
      strcat(sz_bakday,(char *)"/");
    }
    else if(sz_bakType[0]=='M')
    {
      strncpy(sz_bakday,sz_nowtime,6);
      sz_bakday[6]='\0';
      strcat(sz_bakday,(char *)"/");
    }
    else
    {
      sz_bakday[0]='\0';
    }
    char sz_QCbakFile[255];
    sprintf(sz_QCbakFile,"%s%s%s",s.sz_sourcePath,sz_fileBakPath,sz_bakday);
    chkAllDir(sz_QCbakFile);
	  completeDir(sz_QCbakFile);
    sprintf(sz_QCbakFile,"%s%s%s%s",s.sz_sourcePath,sz_fileBakPath,sz_bakday,pch_filename);

    if(copyFile(sz_openFile,sz_QCbakFile)!=0)
    {
      sprintf(sz_errmsg,"the file (%s) copy to (%s) Err!",sz_openFile,sz_QCbakFile);
  	  getCurTime(sz_errtime);
  	  wrlog(sz_pipeId,iProcessId,sz_openFile,'E','H',FILEREC_ERR_COPY,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
  		DBConn.Rollback();
  		return FAIL;
    }
  }
  DBConn.Commit();

  if(unlink(sz_openFile))
  {
  	sprintf(sz_errmsg,"the file (%s) unlink Err!",sz_openFile);
  	getCurTime(sz_errtime);
  	wrlog(sz_pipeId,iProcessId,sz_openFile,'E','H',FILEREC_ERR_UNLINK,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
  	return FAIL;
  }
  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"QC file received finished");
  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"-----------------------------end-----------------------------------");    
	return SUCC;

}
/***************************************************
description:
  扫描指定目录并返回待发送文件列表
input:
  none;
output:
  filelist 文件列表
return
  FAIL 扫描失败；
  SUCC 扫描成功；
*****************************************************/
int CFreceive::ScanFiles(char *pch_sourcePath,char *pch_fileFilter)
{
  char sz_nowtime[14+1];
	char sz_temp[255],sz_filter[10];;
	char sz_filename[255],sz_scanPath[255];
	time_t timer;
	struct stat str_Buff;

	int i;

  //扫描源文件路径
  CF_CFscan  file_scaner;
  
  sprintf(sz_scanPath,"%s%s",pch_sourcePath,sz_fileInPath);

  if(file_scaner.openDir(sz_scanPath)==-1)
	{
		sprintf(sz_errmsg,"Error in scan the DIR(%s)\n",sz_scanPath);
  	getCurTime(sz_errtime);
  	wrlog(sz_pipeId,iProcessId,sz_scanPath,'E','M',FILEREC_ERR_SCAN_RECDIR,errno,sz_errtime,sz_errmsg,(char *)__FILE__,__LINE__);
  	return FAIL;
	}
	
  i=0;
	while(1)
	{
	  if(file_scaner.getFile(pch_fileFilter,sz_temp)==100) break;
    char sz_Null[255];
    
    dispartFullName(sz_temp,sz_Null,sz_filename);
    if(c_UzipFlag=='Y')
    {
	     if(strcmp(pch_fileFilter,"*.bak")!=0)
	     {
	     	 if((strstr(sz_filename,"*.bak"))!=NULL) continue;
	     }
    }
    if((FileTmpCheck(sz_filename))==FAIL) continue;
    stat(sz_temp,&str_Buff);
    strcpy(filelist[i].sz_orgFname,sz_filename);
    filelist[i].iFileSize=str_Buff.st_size;
    filelist[i].lCreateTime=str_Buff.st_ctime;
    filelist[i].iFlag=1;
    i++;
    if(i>=FILEREC_INIT_FILE_COUNT) break;
  }
  iCountMax=i;
  file_scaner.closeDir();
  
  if(cSortFlag=='S')
  {
    qsort(&filelist[0],iCountMax,sizeof(struct FILE_INFO),_CmpFileSize);
  }
  else if(cSortFlag=='T')
  {
    qsort(&filelist[0],iCountMax,sizeof(struct FILE_INFO),_CmpCreateTime);
  }
  else
  {
    qsort(&filelist[0],iCountMax,sizeof(struct FILE_INFO),_CmpFileName);
  }
  return SUCC;
}


/*
  约定：
      iType的值：
           1：  sourceId;
           2:   batchId;
           3:   orgName;
           4:   YYYYMMDD;(处理日）
           5:   NNNN(序列号）
           6：  自定义；
           7:   YYYYMM;（处理月）
*/
int CFreceive::ParseFileName(char *pch_buff,struct COL_INFO *pch_collist,int &iTmpNum,int iMaxColNum)
{
	char sz_buff[255];
	char *pch_tmp;
	
	strcpy(sz_buff,pch_buff);
	int i=0;
	int iDealFlag=0;
	c_SerialFlag='N';
	while(1)
	{
		if(iDealFlag==1) break;
    if((pch_tmp=strchr(sz_buff,'+'))==NULL) iDealFlag=1;
    int iLen;
    if(iDealFlag==0)
    {
      iLen=strlen(sz_buff)-strlen(pch_tmp);
      strncpy(pch_collist[i].sz_colBuff,sz_buff,iLen);
      pch_collist[i].sz_colBuff[iLen]=0;
      strcpy(sz_buff,pch_tmp+1);
    }
    else
    {
    	strcpy(pch_collist[i].sz_colBuff,sz_buff);
    }
    
    if(strcmp(pch_collist[i].sz_colBuff,"$sourceId")==0)
    {
    	pch_collist[i].iType=1;
    }
    else if(strcmp(pch_collist[i].sz_colBuff,"$batchId")==0)
    {
    	pch_collist[i].iType=2;
    }
    else if(strcmp(pch_collist[i].sz_colBuff,"$orgName")==0)
    {
    	pch_collist[i].iType=3;
    }
    else if(strcmp(pch_collist[i].sz_colBuff,"$YYYYMMDD")==0)
    {
    	pch_collist[i].iType=4;
    }
    else if(strncmp(pch_collist[i].sz_colBuff,"$NNN",4)==0)
    {
    	iSerialLong=strlen(pch_collist[i].sz_colBuff)-1;
    	pch_collist[i].iType=5;
    	iMaxSerialValue=1;
    	for(int j=0;j<iSerialLong;j++)
    	{
    		iMaxSerialValue=iMaxSerialValue*10;
    	}
    	iMaxSerialValue--;
    	c_SerialFlag='Y';
    }
    else if(strcmp(pch_collist[i].sz_colBuff,"$YYYYMM")==0)
    {
    	pch_collist[i].iType=7;
    }
    else if(pch_collist[i].sz_colBuff[0]=='$')
    {
    	int k=0,l=0;
    	char sz_subBuff[10];
    	for(int j=1;j<strlen(pch_collist[i].sz_colBuff);j++)
    	{
    		if(pch_collist[i].sz_colBuff[j]=='\'') continue;//过滤'号
    		if(pch_collist[i].sz_colBuff[j]=='(') continue;
    		if(pch_collist[i].sz_colBuff[j]==' ') continue;
        if(k==0)
        {
        	pch_collist[i].c_colFlag=pch_collist[i].sz_colBuff[j];
        	k++;
        	continue;
        }
        if((pch_collist[i].sz_colBuff[j]==',')||(pch_collist[i].sz_colBuff[j]==')'))
        {
        	sz_subBuff[l]=0;
        	if(k==2)
        	{
        		pch_collist[i].iStartPos=atoi(sz_subBuff);
        	}
        	if(k==3)
        	{
        		pch_collist[i].iEndPos=atoi(sz_subBuff);
        	}
        	if(k==5)
        	{
        		pch_collist[i].iColLen=atoi(sz_subBuff);
        	}
        	l=0;
        	k++;
        	continue;
        }
        sz_subBuff[l]=pch_collist[i].sz_colBuff[j];
        l++;
        if(k==4)
        {
        	pch_collist[i].c_separate=pch_collist[i].sz_colBuff[j];
        }
    	}
    	pch_collist[i].iType=6;
    }
    else
    {
    	char sz_tmp[255];
    	strcpy(sz_tmp,pch_collist[i].sz_colBuff);
    	strncpy(pch_collist[i].sz_colBuff,sz_tmp+1,strlen(sz_tmp)-2);
    	pch_collist[i].sz_colBuff[strlen(sz_tmp)-2]=0;
    	pch_collist[i].iType=0;
    }
    i++;
    if(i>iMaxColNum) return FAIL;
  }
  iTmpNum=i;

	return SUCC;
}
int CFreceive::GetCol(char const *pch_orgName,struct COL_INFO e,char *pch_colBuff)
{
  char sz_buff[255];
  strcpy(sz_buff,pch_orgName);

  if(e.c_colFlag=='B')
  {
  	if(e.iStartPos==0)
  	{ 
  		e.iStartPos=1;
  	}
  	else if(e.iStartPos<0)
  	{
  		e.iStartPos=strlen(sz_buff)+e.iStartPos;
  	}

  	if(e.iEndPos<=0)
  	{
  		e.iEndPos=strlen(sz_buff)+e.iEndPos;
  	}
  	int iLen;
  	iLen=e.iEndPos-e.iStartPos+1;
  	if(iLen>=0)
  	{
  	  strncpy(pch_colBuff,pch_orgName+e.iStartPos-1,iLen);
  	  pch_colBuff[iLen]=0;
  	}
  	else if(iLen<0)
  	  return FAIL;
  }
  if(e.c_colFlag=='S')
  {
  	int iPosB,iPosE;

  	if(e.iStartPos==0)
  	{
  		iPosB=-1;
  	}
  	else if(e.iStartPos<0)
  	{
  	  e.iStartPos=-e.iStartPos;
  		iPosB=strrncspn(sz_buff,e.c_separate,e.iStartPos);
  		if(iPosB<0)
  		{
  			return FAIL;
  		}
  	}
  	else
  	{
  		iPosB=strncspn(sz_buff,e.c_separate,e.iStartPos);
  		if(iPosB<0)
  		{
  			return FAIL;
  		}
   	}

  	if(e.iEndPos>0)
  	{
  		iPosE=strncspn(sz_buff,e.c_separate,e.iEndPos);
  		if(iPosE<0)
  		{
  			return FAIL;
  		}
  	}
  	else if(e.iEndPos<0)
  	{
  		e.iEndPos=-e.iEndPos;
  		iPosE=strrncspn(sz_buff,e.c_separate,e.iEndPos);
  		if(iPosE<0)
  		{
  			return FAIL;
  		}
  	}
  	else if(e.iEndPos==0)
  	{
  		iPosE=strlen(sz_buff);
  		if(iPosE<0)
  		{
  			return FAIL;
  		}
  	}
  	if(iPosE==iPosB)
  	{
  		pch_colBuff[0]=0;
  	}
  	else if(iPosE>iPosB)
  	{
   	  strncpy(pch_colBuff,sz_buff+iPosB+1,iPosE-iPosB-1);
  	  pch_colBuff[iPosE-iPosB-1]=0;
  	}
  	else
  	  return FAIL;
  }
  if(e.iColLen==0)
  {
  	return SUCC;
  }

  int iLen;
  iLen=strlen(pch_colBuff);

  if(iLen>e.iColLen)
  {
  	pch_colBuff[e.iColLen]=0;
  }
  else if(iLen<e.iColLen)
  {
  	int i;

  	for(i=0;i<e.iColLen-iLen;i++)
  	 sz_buff[i]='0';
    sz_buff[i]=0;
    sprintf(sz_buff,"%s%s",sz_buff,pch_colBuff);
    strcpy(pch_colBuff,sz_buff);
  }

  return SUCC;	
}

int CFreceive::GetFileName(char *pch_filename,char* pch_sourceId,char *pch_day,int iBatch,int iSerial,char *pch_orgFile,struct SOURCE_INFO pch_sourceList)
{
	int iLen;
	char sz_serial[10+1],sz_buff[255];
	char sz_batchId[6+1];
	char sz_rateCycle[14+1];
	char sz_orgSourceId[5+1];
	char *pch_tmpA,*pch_tmpB;

	sprintf(sz_batchId,"%05d",iBatch);
	pch_filename[0]=0;
	memset(sz_serial,'0',10);
  sz_serial[10]='\0';
	sprintf(sz_buff,"%d",iSerial);
	iLen=strlen(sz_buff);
	strncpy(sz_serial+iSerialLong-iLen,sz_buff,iLen);
	sz_serial[iSerialLong]='\0';

	for(int i=0;i<pch_sourceList.iColNum;i++)
	{
	  if(pch_sourceList.sFileNameList[i].iType==0)
	    sprintf(pch_filename,"%s%s",pch_filename,pch_sourceList.sFileNameList[i].sz_colBuff);
	  else if(pch_sourceList.sFileNameList[i].iType==1)
	  	sprintf(pch_filename,"%s%s",pch_filename,pch_sourceId);
    else if(pch_sourceList.sFileNameList[i].iType==2)
     	sprintf(pch_filename,"%s%s",pch_filename,sz_batchId);
    else if(pch_sourceList.sFileNameList[i].iType==3)
     	sprintf(pch_filename,"%s%s",pch_filename,pch_orgFile);
    else if(pch_sourceList.sFileNameList[i].iType==4)
     	sprintf(pch_filename,"%s%s",pch_filename,pch_day);
    else if(pch_sourceList.sFileNameList[i].iType==5)
     	sprintf(pch_filename,"%s%s",pch_filename,sz_serial);
    else if(pch_sourceList.sFileNameList[i].iType==6)
    {
    	char sz_buff[255];
    	if(GetCol(pch_orgFile,pch_sourceList.sFileNameList[i],sz_buff)==FAIL)
    	{
    		return FAIL;
    	}
     	sprintf(pch_filename,"%s%s",pch_filename,sz_buff);
    }
    else if(pch_sourceList.sFileNameList[i].iType==7)
    {
    	char sz_month[6+1];
    	strncpy(sz_month,pch_day,6);
    	sz_month[6]='\0';
    }
	}
	return SUCC;
}

int CFreceive::GetBatchId(char *pch_time,int &iBatch)
{
	char sz_tmp[20];
	
	if(c_BatchFlag=='Y')
	{
		iBatch=iBatchVal;
		return SUCC;
	}
	
	strncpy(sz_tmp,pch_time+8,2);
	sz_tmp[2]=0;
	iBatch=atoi(sz_tmp);
	iBatch=iBatch*60;
	strncpy(sz_tmp,pch_time+10,2);
	sz_tmp[2]=0;
	iBatch=iBatch+atoi(sz_tmp);
	iBatch=iBatch/iBatchTime+1;

	return SUCC;
}

int CFreceive::GetFileSize(char *pch_oname)
{
	 char sz_buff[255];
	 struct stat buf;

   stat(pch_oname,&buf);
   return (buf.st_size);
   
}

void CFreceive::MoveToErrPath(struct SOURCE_INFO &e,char *pch_InFile,char *pch_fileName)
{
	char sz_ErrName[255];
	
	sprintf(sz_ErrName,"%s%s%s",e.sz_sourcePath,sz_fileErrPath,pch_fileName);
  expTrace(sz_DebugFlag,(char*)__FILE__,__LINE__,"move the file to ErrDir");
  if(c_copyFlag=='Y')
  {
    if(copyFile(pch_InFile,sz_ErrName)!=0)
    {
      sprintf(sz_errmsg,"the file (%s) copy to (%s) Err!",pch_InFile,sz_ErrName);
  	  getCurTime(sz_errtime);
  	  wrlog(sz_pipeId,iProcessId,pch_InFile,'E','H',FILEREC_ERR_COPY,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
    }
    if(unlink(pch_InFile))
    {
      sprintf(sz_errmsg,"the file (%s) unlink Err!",pch_InFile);
  	  getCurTime(sz_errtime);
  	  wrlog(sz_pipeId,iProcessId,pch_InFile,'E','H',FILEREC_ERR_UNLINK,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);	
    }
  }
  else
  {
    if(rename(pch_InFile,sz_ErrName)!=0)
    {
	    sprintf(sz_errmsg,"the file (%s) rename to (%s) Err!",pch_InFile,sz_ErrName);
  	  getCurTime(sz_errtime);
  	  wrlog(sz_pipeId,iProcessId,pch_fileName,'E','H',FILEREC_ERR_RENAME,errno,sz_errtime,sz_errmsg,(char*)__FILE__,__LINE__);
	  }
	}
}

int CFreceive::GetRunFlag(int &iRunFlag)
{
	CBindSQL bs(DBConn);

  bs.Open("SELECT RUNFLAG FROM PROCESS_CTL WHERE PIPE_ID= :1 and PROCESS_ID=:2",SELECT_QUERY);
  bs<<sz_pipeId<<iProcessId;
  while(bs >>iRunFlag);
  bs.Close();

  return SUCC;
}
/***************************************************
description:
  检查文件是临时文件，是否为临时文件；
input:
  fname 文件名;
output:
  none;
return
  SUCC 不是临时文件；
  FAIL 是临时文件；
*****************************************************/
int CFreceive::FileTmpCheck(char *pch_fname)
{
	char sz_name[100];
	strcpy(sz_name,pch_fname);
	if(sz_name[0]=='~') return FAIL;
	if((strstr(sz_name,".TMP"))!=NULL) return FAIL;
	if((strstr(sz_name,".tmp"))!=NULL) return FAIL;
	if((strstr(sz_name,".Tmp"))!=NULL) return FAIL;
	
	return SUCC;
}


/**********************************************
数据源、文件名、原始文件名、批次号、文件大小、原始数据源、采集日期、压缩时间、接收时间、处理标志、文件类型
**********************************************/
int CFreceive::InsertFileReceive(char *pch_sourceId,char *pch_filename,char *pch_orgname,int iBatch,long lFilesize,char *pch_orgSourceId,char *sz_colDay,char *pch_compressTime,char *pch_RecTime,char cDealFlag,char c_RecFileType)
{
	 char sz_nowtime[15];
	 char sz_CompTime[14+1];
	 CBindSQL bs(DBConn);

	 if(c_FileType=='D')
	 {
	 	 bs.Open("INSERT INTO FILE_RECEIVED(SOURCE_ID,FILENAME,ORG_NAME,BATCH_ID,FILESIZE,ORG_SOURCE_ID,COL_DATE,COMPRESS_TIME,RECEIVE_TIME,DEAL_FLAG,FILE_TYPE,ROLLBACK_FLAG,PROCESS_ID,FILE_ID)VALUES(:1,:2,:3,:4,:5,:6,:7,:8,:9,:10,:11,:12,:13,:14)",NONSELECT_DML);
	   bs<<pch_sourceId<<pch_filename<<pch_orgname<<iBatch<<lFilesize<<pch_orgSourceId<<sz_colDay<<pch_compressTime<<pch_RecTime<<cDealFlag<<c_RecFileType<<'N'<<iProcessId<<-1;
	 }
	 else
	 { 
	 	 bs.Open("INSERT INTO FILE_RECEIVED(SOURCE_ID,FILENAME,ORG_NAME,BATCH_ID,FILESIZE,ORG_SOURCE_ID,COL_DATE,COMPRESS_TIME,RECEIVE_TIME,DEAL_FLAG,FILE_TYPE,ROLLBACK_FLAG,PROCESS_ID,FILE_ID)VALUES(:1,:2,:3,:4,:5,:6,:7,:8,:9,:10,:11,:12,:13,S_FILE_RECEIVED.NEXTVAL)",NONSELECT_DML);
     bs<<pch_sourceId<<pch_filename<<pch_orgname<<iBatch<<lFilesize<<pch_orgSourceId<<sz_colDay<<pch_compressTime<<pch_RecTime<<cDealFlag<<c_RecFileType<<'N'<<iProcessId;
	 }
	 bs.Execute();
   bs.Close();
   
   return SUCC;
}

int CFreceive::UpdateErrFileReceive(char *pch_orgname,char *pch_sourceId)
{
	CBindSQL bs(DBConn);

	bs.Open("update FILE_RECEIVED set DEAL_FLAG='E' where ORG_NAME=:1 and SOURCE_ID=:2",NONSELECT_DML);
  bs<<pch_orgname<<pch_sourceId;
  bs.Execute();
  bs.Close();
  
  return SUCC;
}

int CFreceive::UpdateSerialInfo(char *pch_sourceId,char *pch_time,int iSerial)
{
  CBindSQL bs(DBConn);
	bs.Open("update FILE_SERIAL_INFO set DEAL_TIME =:1,NEW_SERIAL =:2 where SOURCE_ID=:3",NONSELECT_DML);
	bs<<pch_time<<iSerial<<pch_sourceId;
	bs.Execute();
  bs.Close();

	return SUCC;
}

int CFreceive::InsertRepeatFile(char *pch_filename,char *pch_sourceId,char *pch_proTime)
{
	//char sz_nowtime[15];
	//char sz_filename[60];
	CBindSQL bs(DBConn);


	bs.Open("INSERT INTO REPEAT_FILE(FILENAME ,REPEAT_TIME,SOURCE_ID) VALUES(:1,:2,:3)",NONSELECT_DML);
  bs<<pch_filename<<pch_proTime<<pch_sourceId;
  bs.Execute();
	bs.Close();
	
	return SUCC;
}

int CFreceive::InsertQcReceived(char *pch_filename,char *pch_sourceId)
{
	char sz_nowtime[15];
	char sz_filename[60];
	CBindSQL bs(DBConn);

	getCurTime(sz_nowtime);

	bs.Open("INSERT INTO QC_RECEIVED(QC_FILENAME ,RECEIVE_TIME,SOURCE_ID) VALUES(:1,:2,:3)",NONSELECT_DML);
  bs<<pch_filename<<sz_nowtime<<pch_sourceId;
  bs.Execute();
	bs.Close();
	
	return SUCC;
}

int CFreceive::InsertSchFormat(char *pch_filename,char *pch_sourceId)
{
	int iNum;
	char sz_buff[255];

	CBindSQL bs(DBConn);

	sprintf(sz_buff,"SELECT count(FILENAME)FROM %s WHERE FILENAME=:1 and SOURCE_ID=:2",sz_outControlTable);
  bs.Open(sz_buff,SELECT_QUERY);
  bs<<pch_filename<<pch_sourceId;
	bs>>iNum;
	bs.Close();

	if(iNum!=0)
	 return FILEREC_SCH_REPEAT;

  sprintf(sz_buff,"INSERT INTO %s (FILENAME,SOURCE_ID,DEAL_FLAG,VALIDFLAG) VALUES (:A,:B,:C,:D)",sz_outControlTable);
  bs.Open(sz_buff,NONSELECT_DML);
  bs<<pch_filename<<pch_sourceId<<'W'<<'Y';
  bs.Execute();
  bs.Close();

  return SUCC;
}



/************************************************
   长途检测文件重复：
     1：C: 原始文件名重复,在重复时间定义范围之内的即为重复
        R、D：原始文件名重复,压缩时间相同即为重复文件(防止重复传输某个文件)
 *************************************************/
int CFreceive::LDTCheckFileInFileReceive(char *pch_sourceId,char *pch_filename,char *pch_orgName,char *pch_compTime,char cFlag)
{
	char *pch_tmp;
	char sz_buff[255];
	char sz_fileName[255];
	int iNum;
	CBindSQL bs(DBConn);
	time_t timer;

  
  strcpy(sz_fileName,pch_filename);

	if(cFlag=='R'||cFlag=='D')
	{
		bs.Open("SELECT count(FILENAME) FROM FILE_RECEIVED WHERE SOURCE_ID=:1 AND ORG_NAME=:2",SELECT_QUERY);
    bs<<pch_sourceId<<pch_orgName;
	  bs>>iNum;
	  bs.Close();
	  if(iNum==0) return FILEREC_NORMAL_FILE;
	  else
	  { 
	  	char sz_compTime[14+1];
	    bs.Open("SELECT COMPRESS_TIME FROM FILE_RECEIVED WHERE SOURCE_ID=:1 and ORG_NAME=:2",SELECT_QUERY);
	    bs<<pch_sourceId<<pch_orgName;
	    while(bs>>sz_compTime)
	    {
	    	if(strcmp(pch_compTime,sz_compTime)==0)
	      {
	      	bs.Close();
	      	return FILEREC_REPEAT_FILE;
	      }
	    }
	    bs.Close();
	    return FILEREC_NORMAL_FILE;
    }
	}
  else
  {
	  bs.Open("SELECT count(ORG_NAME)FROM FILE_RECEIVED WHERE SOURCE_ID=:1 and ORG_NAME=:2",SELECT_QUERY);
	  bs<<pch_sourceId<<pch_orgName;
	  bs>>iNum;
	  bs.Close();
	  if(iNum==0) 
	    return FILEREC_NORMAL_FILE;
	  else
	  {
      if(sourcelist[iSourceNo].iRereceiveDay>0)
      {
     	  char sz_nowtime[14+1];
	      char sz_rectime[14+1];
	      long lTmp;

        bs.Open("SELECT RECEIVE_TIME FROM FILE_RECEIVED WHERE SOURCE_ID=:1 and ORG_NAME=:2",SELECT_QUERY);
        bs<<pch_sourceId<<pch_orgName;
        bs>>sz_rectime;
        bs.Close();
        lTmp=timeStr2Time(sz_rectime);
        time(&timer);
        if(lTmp==-1) return FILEREC_DROP_FILE;
        lTmp=(timer-lTmp)/60/60/24;

        if(lTmp>sourcelist[iSourceNo].iRereceiveDay) 
          return FILEREC_NORMAL_FILE;
  	    else
	        return FILEREC_REPEAT_FILE;
	    }
	    else if(sourcelist[iSourceNo].iRereceiveDay==0)
	    	return FILEREC_NORMAL_FILE;
	    else
	      return FILEREC_REPEAT_FILE;
	  }
  }
  return SUCC;
}

/*************************************
     短信检测文件重复：
 
     2：正式文件名重复即为重复

 *************************************/ 
int CFreceive::SingleCheckFileInFileReceive(char *pch_sourceId,char *pch_filename)
{
	int iNum;  
  CBindSQL bs(DBConn);
  bs.Open("SELECT count(FILENAME)FROM FILE_RECEIVED WHERE SOURCE_ID=:1 and FILENAME=:2",SELECT_QUERY);
  bs<<pch_sourceId<<pch_filename;
  bs>>iNum;
  bs.Close();
  if(iNum==0) return FILEREC_NORMAL_FILE;
  if(iNum>=1) return FILEREC_REPEAT_FILE;

}
 
/*************************************
     数据业务检测文件重复：
     1：原始文件名匹配，重复文件
     2：正式文件名重复即为重复
     含QC文件的检测：
     1：原文件匹配不存在，放弃该文件的接收
     2：原始文件名匹配，接收时间为空，正常接收
     3：原始文件名匹配，接收时间不为空，重复文件
 *************************************/ 
int CFreceive::CheckFileInFileReceive(char *pch_sourceId,char *pch_filename)
{	
	int iNum;
	long lTmp;
	CBindSQL bs(DBConn);

  time_t timer;


  bs.Open("SELECT count(ORG_NAME)FROM FILE_RECEIVED WHERE SOURCE_ID=:1 and ORG_NAME=:2",SELECT_QUERY);
  bs<<pch_sourceId<<pch_filename;
  bs>>iNum;
  bs.Close();

  if(sourcelist[iSourceNo].c_QcFlag=='Y')
  {
  	if(iNum==0) return  FILEREC_DROP_FILE;
  }
  else
  {
	  if(iNum==0) return FILEREC_NORMAL_FILE;
	}
	
	char sz_nowtime[14+1];
	char sz_rectime[14+1];
	char c_Flag;
  int iWfileNum;
  iWfileNum=0;
  bs.Open("SELECT RECEIVE_TIME,DEAL_FLAG FROM FILE_RECEIVED WHERE SOURCE_ID=:1 and ORG_NAME=:2 order by RECEIVE_TIME",SELECT_QUERY);
  bs<<pch_sourceId<<pch_filename;
  while(bs>>sz_rectime>>c_Flag)
  {
    if(c_Flag=='W') iWfileNum++;
  }
  bs.Close();
  if(iWfileNum==1) return FILEREC_NORMAL_FILE;
    else if(iWfileNum>1) return FILEREC_MANY_QFILE;
  

  if(sourcelist[iSourceNo].iRereceiveDay>0)
  {
    lTmp=timeStr2Time(sz_rectime);
    time(&timer);
    if(lTmp==-1) return FILEREC_DROP_FILE;
    lTmp=(timer-lTmp)/60/60/24;

    if(lTmp>sourcelist[iSourceNo].iRereceiveDay) return FILEREC_NORMAL_FILE;
  	else
	    return FILEREC_REPEAT_FILE;
	}
	else if(sourcelist[iSourceNo].iRereceiveDay==0)
	{
    if(sourcelist[iSourceNo].c_QcFlag=='Y')
    {
  	  return  FILEREC_DROP_FILE;
    }
    else
		  return FILEREC_NORMAL_FILE;	
  }
	else
	  return FILEREC_REPEAT_FILE;
}

int CFreceive::CheckRecollectInfo(char *pch_sourceId,char *pch_fileName,char *pch_DealTime)
{
	int iNum;
	CBindSQL bs(DBConn);
	bs.Open("SELECT count(*)FROM RECOLLECT_FILE_INFO WHERE SOURCE_ID=:1 and ORG_FILENAME=:2 and DEAL_FLAG=:3",SELECT_QUERY);
  bs<<pch_sourceId<<pch_fileName<<'C';
  bs>>iNum;
  bs.Close();
  
  if(iNum==0)
  {
  	return REC_FILE_DROP;
  }
  return SUCC;
}

int CFreceive::GetMaxSerial(char *pch_sourceId,char *pch_date)
{
	char sz_buff[255];
	int  iTmpSerial;
  CBindSQL bs(DBConn);
  
  sprintf(sz_buff,"select MAX(SUBSTR(FILENAME,%d,%d)) from FILE_RECEIVED where SOURCE_ID =:1 and COL_DATE=:2",iSerialPos,iSerialLong),
	bs.Open(sz_buff,SELECT_QUERY);
	bs<<pch_sourceId<<pch_date;
	sz_buff[0]='\0';
	bs>>sz_buff;
  bs.Close();
  if(sz_buff[0]=='\0')  iTmpSerial=0;
  else
    iTmpSerial=atoi(sz_buff);

	return iTmpSerial;
}

int CFreceive::UpdateRecollectInfo(char *pch_sourceId,char *pch_orgName,char *pch_fileName,char *pch_DealTime)
{
	CBindSQL bs(DBConn);

  bs.Open("UPDATE RECOLLECT_FILE_INFO SET COMPLETE_TIME=:1,DEAL_FLAG=:2,AF_FILENAME=:3 WHERE SOURCE_ID=:3 and ORG_FILENAME=:4",NONSELECT_DML);
  bs<<pch_DealTime<<'Y'<<pch_fileName<<pch_sourceId<<pch_orgName;
  bs.Execute();
  bs.Close();

  return SUCC;
}
  
int CFreceive::InsertRecollectInfo(char *pch_sourceId,char *pch_orgName,char *pch_fileName,char *pch_DealTime)
{
	int iNum;
	CBindSQL bs(DBConn);
	bs.Open("SELECT count(*)FROM RECOLLECT_FILE_INFO WHERE SOURCE_ID=:1 and ORG_FILENAME=:2",SELECT_QUERY);
  bs<<pch_sourceId<<pch_orgName;
  bs>>iNum;
  bs.Close();
  
  if(iNum==0)
  {
  	bs.Open("INSERT INTO RECOLLECT_FILE_INFO(SOURCE_ID,ORG_FILENAME,AF_FILENAME,COMPLETE_TIME,DEAL_FLAG) VALUES (:1,:2,:3,:4,:5)",NONSELECT_DML);
    bs<<pch_sourceId<<pch_orgName<<pch_fileName<<pch_DealTime<<'Y';
    bs.Execute();
    bs.Close();
  }
  else
  {
  	bs.Open("UPDATE RECOLLECT_FILE_INFO SET COMPLETE_TIME=:1,DEAL_FLAG=:2,AF_FILENAME=:3 WHERE SOURCE_ID=:4 and ORG_FILENAME=:5",NONSELECT_DML);
    bs<<pch_DealTime<<'Y'<<pch_fileName<<pch_sourceId<<pch_orgName;
    bs.Execute();
    bs.Close();
  }

  return SUCC;
}

int CFreceive::UpdateFileReceive(char *pch_sourceId,char *pch_orgname,char *pch_filename,int iBatch,char *pch_colday,char c_RecFileType,int iFileSize)
{
	char sz_nowtime[15];
	CBindSQL bs(DBConn);
  
  if(iFileSize==0)
  {	
	  getCurTime(sz_nowtime);
	  bs.Open("UPDATE FILE_RECEIVED SET FILENAME=:1,BATCH_ID=:2,RECEIVE_TIME=:3,DEAL_FLAG=:4,COL_DATE=:5, COMPRESS_TIME=:6,FILE_TYPE=:7 WHERE SOURCE_ID=:8 AND ORG_NAME=:9 AND DEAL_FLAG=:10",NONSELECT_DML);
    bs<<pch_filename<<iBatch<<sz_nowtime<<'Y'<<pch_colday<<sz_nowtime<<c_RecFileType<<pch_sourceId<<pch_orgname<<'W';
    bs.Execute();
    bs.Close();
  }
  else
  {
	  getCurTime(sz_nowtime);
	  bs.Open("UPDATE FILE_RECEIVED SET FILENAME=:1,BATCH_ID=:2,RECEIVE_TIME=:3,DEAL_FLAG=:4,COL_DATE=:5, COMPRESS_TIME=:6,FILE_TYPE=:7 WHERE SOURCE_ID=:8 AND ORG_NAME=:9 AND FILESIZE=:10 AND DEAL_FLAG=:11",NONSELECT_DML);
    bs<<pch_filename<<iBatch<<sz_nowtime<<'Y'<<pch_colday<<sz_nowtime<<c_RecFileType<<pch_sourceId<<pch_orgname<<iFileSize<<'W';
    bs.Execute();
    bs.Close();
  }
	return SUCC;
}
/*
int CFreceive::GetFileInfo(char *pch_filename)
{
	int iRecordNum;
	FileRead.Open(pch_fileName);
	iRecordNum=FileRead.Get_recCount();
	while(1)
	{
		if(FileRead.Read(QCRecord)==READ_AT_END) break;
	}
	return SUCC;
	
}
*/
int CFreceive::SelectBillCycle(char *pch_CycleTime,int&  iBillCycle)
{
	int iNum;
	char sz_dbTime[14+1];
	CBindSQL bs(DBConn);
	
	iNum=0;
  strcpy(sz_dbTime,pch_CycleTime);
  sz_dbTime[6]='\0';
  
  bs.Open("SELECT count(BILL_CYCLE_ID) FROM BILLING_CYCLE WHERE YEAR_MONTH=:1",SELECT_QUERY);
  bs<<sz_dbTime;
  bs>>iNum;
  bs.Close();
  
  if(iNum!=1) return FAIL;

 	bs.Open("SELECT BILL_CYCLE_ID FROM BILLING_CYCLE WHERE YEAR_MONTH=:1",SELECT_QUERY);
  bs<<sz_dbTime;
  bs>>iBillCycle;
  bs.Close();

	return SUCC;
}

int CFreceive::SplitBuf(char *pch_Buf,char cWord,char pch_Var[20][100])
{
  char *pch_tmp;
  char sz_buff[400];
  int i;
  
  i=0;
  strcpy(sz_buff,pch_Buf);
  while(1)
  {
    int iLen;
    if((pch_tmp=strchr(sz_buff,cWord))==NULL) break;
    iLen=strlen(sz_buff)-strlen(pch_tmp);
    if(iLen==0) continue;
    strncpy(pch_Var[i],sz_buff,iLen);
    pch_Var[i][iLen]='\0';
    strcpy(sz_buff,pch_tmp+1);
    i++;
  }
  if(strlen(sz_buff)!=0)
  {
    strcpy(pch_Var[i],sz_buff);
    i++;
  }
  return i;
}

void CFreceive::SleepSender()
{
	sleep(iSleepTime);
}

int main(int argc,char *argv[])
{
	printf("*************************************************** \n");
	printf("*        GuangDong Telecom. zhjs System        \n");
	printf("*                                                   \n");
	printf("*            file_rec  module			   \n");
	printf("*	     Version 2.0.2       		   \n");
	printf("*         last updated: 2008-3-6 by zhangguoqiang  \n");
	printf("*		 \n");
	printf("*						   \n");
	printf("***************************************************\n");
	
	if (argc!=4)
	{
		printf("umssage:%s <pipe_id> <process_id> <path of env>\n",argv[0]);
	  exit (FAIL);
	}
	CFreceive ss;
	if(ss.Init(argv[1],argv[2],argv[3],argv[0])==FAIL)
	{
		return FAIL;
	}
  ss.Proc();

	return SUCC;
}