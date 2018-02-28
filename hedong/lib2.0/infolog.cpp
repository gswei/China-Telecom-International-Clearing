/****************************************
   信息点文件：
   配置信息点文件： /rootpath/configinfo/yyyymmdd/hhmm_workflow_processid.log
****************************************/

#include "infolog.h"

char sz_InfoLogEnvPath[255];
char sz_InfoLogPath[255];
char sz_BusinessName[10];
char sz_LocalIp[255],sz_HostName[255];
int  iInfoLogFlag=0;

int infoLog_setEnvPath(char *pch_EnvPath)
{
  if(pch_EnvPath!=NULL)
  {
    strcpy(sz_InfoLogEnvPath,pch_EnvPath); 
    int iLen;
    iLen =strlen(sz_InfoLogEnvPath);
    if(sz_InfoLogEnvPath[iLen-1] == '/') 
      sz_InfoLogEnvPath[iLen] = 0;
  }
  
  FILE *fp;
  int iFlag,iLen;
  char ss[5][255];
  char sz_InfoLogEnvFile[255],sz_Buff[255];
  
  ss[0][0]=0;
  iFlag=0;
  sprintf(sz_InfoLogEnvFile,"%s/zhjs.env",sz_InfoLogEnvPath);
  if((fp=fopen(sz_InfoLogEnvFile,"r"))==NULL)
   	return INFOLOG_OPEN_ENV_ERROR; 
  
 	while(1)
  {
    if(fgets(sz_Buff,255,fp)==NULL) break;
    sscanf(sz_Buff,"%s %s %s",ss[0],ss[1],ss[2]);
    
    if( strcmp(ss[0],INFOLOG)==0)
    {	 
    	strcpy(sz_InfoLogPath,ss[1]);
      iLen=strlen(sz_InfoLogPath);
  	  if (sz_InfoLogPath[iLen-1] == '/') sz_InfoLogPath[iLen-1]='\0';
     	iInfoLogFlag=1;
    }
    
    //if( strcmp(ss[0],BUSINESSNAME)==0)
    //{	 
    	//strcpy(sz_BusinessName,ss[0]);
     	//iFlag++;
    //}
  }//end while	
	//if(iFlag!=2)   
 	//return INFOLOG_GET_ENV_ERROR;
 	fclose(fp);

 	struct in_addr addr;
 	/*取得机器名称*/
	if(gethostname(sz_HostName,sizeof(sz_HostName))!=-1)
	{
		/*取得给定机器的信息*/
		struct hostent* phost=gethostbyname(sz_HostName);
		for(int i=0;phost!= NULL&&phost->h_addr_list[i]!=NULL;i++)
		{
      memcpy(&addr, phost->h_addr_list[i],sizeof(struct in_addr));
    }
    strcpy(sz_LocalIp,inet_ntoa(addr));
	}
  else
    return INFOLOG_GET_ENV_ERROR;
  return 0;
}

int configInfoLog(int iWorkFlow,int iProcessId,char *pch_ProgramName,char *pch_ProgramPath,char *pch_ConfigName,char *pch_ConfigPath,char *pch_InputPath)
{
	char sz_buff[INFOLOG_MAX_LEN],sz_InfoFile[255];
	char sz_NowTime[14+1],sz_NowDate[8+1];
	FILE *fp;
	
	if(iInfoLogFlag==0)  return 0;
		 
	getCurTime(sz_NowTime);
	strncpy(sz_NowDate,sz_NowTime,8);
	sz_NowDate[8]=0;
	strncpy(sz_buff,sz_NowTime+8,4);
	sz_buff[4]=0;
	sprintf(sz_InfoFile,"%s/configinfo/%s/",sz_InfoLogPath,sz_NowDate);
	
  if(chkAllDir(sz_InfoFile)!=0)
  {
  	return INFOLOG_MDIR_ERROR;
  }
  
  
  sprintf(sz_InfoFile,"%s%s_%d_%d.log",sz_InfoFile,sz_buff,iWorkFlow,iProcessId);
  
	if((fp=fopen(sz_InfoFile,"a+"))==NULL)
	{
	  return  INFOLOG_OPENFILE_ERROR;
	}
	sprintf(sz_buff,"%s|%s|%s|%s|%s|%s|%s\n",sz_NowTime,pch_ProgramName,sz_LocalIp,pch_ProgramPath,pch_ConfigName,pch_ConfigPath,pch_InputPath);
  
  errno=0;
  
  fprintf(fp,"%s",sz_buff);
  
  if(errno!=0)
  {
  	return INFOLOG_WRITEFILE_ERROR;
  }
  
  fclose(fp);

	return 0;
}


int configInfoLog(int iProcess_Id,char *pch_ProgramName,char *pch_ProgramPath,char *pch_ConfigName,char *pch_ConfigPath,char *pch_InputPath,char *pch_CollectIp)
{
	char sz_buff[INFOLOG_MAX_LEN],sz_InfoFile[255];
	char sz_NowTime[14+1],sz_NowDate[8+1];
	FILE *fp;
	
  if(iInfoLogFlag==0)  return 0;

	getCurTime(sz_NowTime);
	strncpy(sz_NowDate,sz_NowTime,8);
	sz_NowDate[8]=0;
	strncpy(sz_buff,sz_NowTime+8,4);
	sz_buff[4]=0;
	sprintf(sz_InfoFile,"%s/transinfo/%s/",sz_InfoLogPath,sz_NowDate);
	
  if(chkAllDir(sz_InfoFile)!=0)
  {
  	return INFOLOG_MDIR_ERROR;
  }
  
  
  sprintf(sz_InfoFile,"%s%s_-1.%d.log",sz_InfoFile,sz_buff,iProcess_Id);
  
	if((fp=fopen(sz_InfoFile,"a+"))==NULL)
	{
	  return  INFOLOG_OPENFILE_ERROR;
	}
	sprintf(sz_buff,"%s|%s|%s|%s|%s|%s|%s|%s\n",sz_NowTime,pch_ProgramName,sz_LocalIp,pch_ProgramPath,pch_ConfigName,pch_ConfigPath,pch_InputPath,pch_CollectIp);
  
  errno=0;
  
  fprintf(fp,"%s",sz_buff);
  
  if(errno!=0)
  {
  	return INFOLOG_WRITEFILE_ERROR;
  }
  
  fclose(fp);

	return 0;
}

int dataInfoLog(int iWorkFlow,int iProcessId,char *pch_FileName,char *pch_StartDealTime,char *pch_EndDealTime,int iFileSize,int iInputCount,int iMainCount,int iPickCout,int iErrCount,int iLackCount,int iCopyCount,int iInputDuration,int iOutputDuration,int iInputFee,int iOutputFee)
{
	
  if(iInfoLogFlag==0)  return 0;
	
	CBindSQL bs(DBConn);
	bs.Open("INSERT INTO info_log(WORKFLOW,PROCESS_ID,FILENAME,DEALSTARTTIME,DEALENDTIME,FILESIZE,INPUT_COUNT,MAINFLOW_COUNT,PICK_COUNT,ERROR_COUNT,LACKINFO_COUNT,OTHER_COUNT,INPUT_DURATION,MAINFLOW_DURATION,INPUT_FEE,MAINFLOW_FEE) VALUES (:1,:2,:3,:4,:5,:6,:7,:8,:9,:10,:11,:12,:13,:14,:15,:16)",NONSELECT_DML);
  bs<<iWorkFlow<<iProcessId<<pch_FileName<<pch_StartDealTime<<pch_EndDealTime<<iFileSize<<iInputCount<<iMainCount<<iPickCout<<iErrCount<<iLackCount<<iCopyCount<<iInputDuration<<iOutputDuration<<iInputFee<<iOutputFee;
  bs.Execute();
  bs.Close();

  return 0;
}


int dataInfoLog(int iWorkFlow,int iProcessId,char *pch_FileName,char *pch_StartDealTime,char *pch_EndDealTime,int iFileSize,int iInputCount,int iMainCount,int iPickCout,int iErrCount,int iLackCount,int iCopyCount,int iInputDuration,int iOutputDuration,int iInputFee,int iOutputFee,CBindSQL &bs)
{
	
  if(iInfoLogFlag==0)  return 0;
	
	//CBindSQL bs(DBConn);
	bs.Open("INSERT INTO info_log(WORKFLOW,PROCESS_ID,FILENAME,DEALSTARTTIME,DEALENDTIME,FILESIZE,INPUT_COUNT,MAINFLOW_COUNT,PICK_COUNT,ERROR_COUNT,LACKINFO_COUNT,OTHER_COUNT,INPUT_DURATION,MAINFLOW_DURATION,INPUT_FEE,MAINFLOW_FEE) VALUES (:1,:2,:3,:4,:5,:6,:7,:8,:9,:10,:11,:12,:13,:14,:15,:16)",NONSELECT_DML);
  bs<<iWorkFlow<<iProcessId<<pch_FileName<<pch_StartDealTime<<pch_EndDealTime<<iFileSize<<iInputCount<<iMainCount<<iPickCout<<iErrCount<<iLackCount<<iCopyCount<<iInputDuration<<iOutputDuration<<iInputFee<<iOutputFee;
  bs.Execute();
  bs.Close();

  return 0;
}

int collectLog(int iWorkFlow,int iProcessId,char *pch_RegionId,char *pch_fileName,int iFileSize,char *pch_CollectTime)
{
	if(iInfoLogFlag==0)  return 0;
	
  char sz_buff[INFOLOG_MAX_LEN],sz_InfoFile[255];
	char sz_NowTime[14+1],sz_NowDate[8+1];
	FILE *fp;
	
  getCurTime(sz_NowTime);
	strncpy(sz_NowDate,sz_NowTime,8);
	sz_NowDate[8]=0;
	strncpy(sz_buff,sz_NowTime+8,4);
	sz_buff[4]=0;
	sprintf(sz_InfoFile,"%s/collectinfo/%s/",sz_InfoLogPath,sz_NowDate);
	
  if(chkAllDir(sz_InfoFile)!=0)
  {
  	return INFOLOG_MDIR_ERROR;
  }
  
  sprintf(sz_InfoFile,"%s%s_Collect_%d_%d.log",sz_InfoFile,sz_buff,iWorkFlow,iProcessId);
  
	if((fp=fopen(sz_InfoFile,"a+"))==NULL)
	{
	  return  INFOLOG_OPENFILE_ERROR;
	}
	sprintf(sz_buff,"%s|%s|%d|%s\n",pch_RegionId,pch_fileName,iFileSize,pch_CollectTime);
  
  errno=0;
  
  fprintf(fp,"%s",sz_buff);
  
  if(errno!=0)
  {
  	return INFOLOG_WRITEFILE_ERROR;
  }

  fclose(fp);
  
  return  0;
}


int transInfoLog(char *pch_TransSign,int iProcessId,char *pch_RegionId,char *pch_fileName,int iFileSize,char *pch_TransTime)
{
	
	if(iInfoLogFlag==0)  return 0;
	
	char sz_buff[INFOLOG_MAX_LEN],sz_InfoFile[255];
	char sz_NowTime[14+1],sz_NowDate[8+1];
	FILE *fp;
	
	getCurTime(sz_NowTime);
	strncpy(sz_NowDate,sz_NowTime,8);
	sz_NowDate[8]=0;
	strncpy(sz_buff,sz_NowTime+8,4);
	sz_buff[4]=0;
	sprintf(sz_InfoFile,"%s/transinfo/%s/",sz_InfoLogPath,sz_NowDate);
	
  if(chkAllDir(sz_InfoFile)!=0)
  {
  	return INFOLOG_MDIR_ERROR;
  }
  
  sprintf(sz_InfoFile,"%s%s_Trans_%s_%d.log",sz_InfoFile,sz_buff,pch_TransSign,iProcessId);
  
	if((fp=fopen(sz_InfoFile,"a+"))==NULL)
	{
	  return  INFOLOG_OPENFILE_ERROR;
	}
	sprintf(sz_buff,"%s|%s|%d|%s\n",pch_RegionId,pch_fileName,iFileSize,pch_TransTime);
  
  errno=0;
  
  fprintf(fp,"%s",sz_buff);
  
  if(errno!=0)
  {
  	return INFOLOG_WRITEFILE_ERROR;
  }

  fclose(fp);
  
  return  0;
}
/*
int main()
{
	infoLog_setEnvPath("/home/zhjs/data/work/infoLog");
	configInfoLog(1,1,"file_rec","/home/zhjs/data/datgw/bin","zhjs.env","/home/zhjs/data/datgw/env/","/home/zhjs/data/GZ/GZ101/revfile/");
  configInfoLog(1,"file_patch","/home/zhjs/data/datgw/bin","get_GZ101.ini;zhjs.env","/home/zhjs/data/datgw/env/;/home/zhjs/data/datgw/env/","/home/zhjs/source/","132.112.10.17");
  
 
  connectDB("/home/zhjs/data/work/infoLog/zhjs.env",DBConn);
  for(int i=0;i<50;i++)
  {   
  	  char sz_nowTime[14+1];
  	  char sz_fileName[255];
  	  getCurTime(sz_nowTime);
  	  sprintf(sz_fileName,"GZ101.00001.20071017.%06d.dat.GZ101.%08d.dat",i,i);
      dataInfoLog(1,1,sz_fileName,sz_nowTime,sz_nowTime,100000,50000,49999,0,1,0,0,0,0,0,0);
      dataInfoLog(1,2,sz_fileName,sz_nowTime,sz_nowTime,100000,49999,49998,0,1,0,0,0,0,0,0);
      dataInfoLog(1,3,sz_fileName,sz_nowTime,sz_nowTime,100000,49998,49988,0,10,0,0,0,0,0,0);
      dataInfoLog(1,4,sz_fileName,sz_nowTime,sz_nowTime,100000,49988,49987,0,1,0,0,0,0,0,0);
      dataInfoLog(1,5,sz_fileName,sz_nowTime,sz_nowTime,100000,49987,49982,0,5,0,0,0,0,0,0);
      dataInfoLog(1,6,sz_fileName,sz_nowTime,sz_nowTime,100000,49982,49972,0,10,0,0,0,0,0,0);
      dataInfoLog(1,7,sz_fileName,sz_nowTime,sz_nowTime,100000,49972,49969,0,3,0,0,0,0,0,0);
      dataInfoLog(1,8,sz_fileName,sz_nowTime,sz_nowTime,100000,49969,49969,0,0,0,0,0,0,0,0);
      collectLog(1,1,"GZ101",sz_fileName,10240000,sz_nowTime);
      transInfoLog("GZ101",1,"GZ101",sz_fileName,10240000,sz_nowTime);


      DBConn.Commit();
  }
  DBConn.Commit();
	return 0;
}*/

