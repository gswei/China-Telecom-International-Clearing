#include "Infocom.h"

extern int _CmpFileName(const void *p1, const void *p2)
{
    int i;
    struct FILE_INFO *ptr1, *ptr2;
    ptr1 = (struct FILE_INFO *) p1; 
    ptr2 = (struct FILE_INFO *) p2;
 
    i=strcmp(ptr1->sz_orgFname,ptr2->sz_orgFname);
    
    return i;
}

int CInfoCom::Infocom()	//将各分系统的信息点综合，上传给集团
{
	char szCDate[20];	
	char szFilter[20];
	char szFileOrg[100];
	char szInfoPoint[30];
	char szType[10];
	char szRate[10];
	char szSep[10];
	char szJsonFile[300];
	char szFilePath[300];
	char szCopyPath[300];
	char szCopyFile[300];
	char szFileDate[20];
	char buf[250];
	char iRNum[10];
	int j,k;
	int ibpos;
	int iepos;
	int total_num;
	
	expTrace("Y", __FILE__, __LINE__, "endtime:%s",m_szEndTime);
	
	strcpy(m_kpid,"");
	strcpy(szType,"");	
	strcpy(szRate,"");
	getCurDate(szCDate);
	
	if(strlen(m_szEndTime)==0)
	  return -1;
	  
	sprintf(szFilter,"*%s*",m_szEndTime);
	expTrace("Y", __FILE__, __LINE__, "filter:%s",szFilter);
	  	
	if(-1==ScanFiles(m_szCheckPath,szFilter))
	{
		expTrace("Y", __FILE__, __LINE__, "scan file error to filelist");
		return -1;
	}
	  
	if (0==iCountMax)
	{
		expTrace("Y", __FILE__, __LINE__, "scan file num is zero");
		return -1;
	}
	expTrace("Y", __FILE__, __LINE__, "scan file num is %d",iCountMax);

	
	sprintf(szCopyPath,"%sBAK/",m_szCheckPath);
	chkAllDir(szCopyPath);    //检查备份目录是否建好
	expTrace("Y", __FILE__, __LINE__, "check copy dir:%s",szCopyPath);
	FILE *fp;	
	total_num=0;
	ibpos=0;
	iepos=0;
	
	for(j=0;j<iCountMax;j++)
	{
		strcpy(szFileOrg,filelist[j].sz_orgFname);			
		GetSubStr(szFileOrg,9,'.',szSep);
		
		if(strcmp(szSep,"END")==0)
		{
			sprintf(szFilePath,"%s%s",m_szCheckPath,szFileOrg);			
			remove(szFilePath);
			expTrace("Y", __FILE__, __LINE__, "remove end file: %s",szFilePath);
			continue;
		}
		else
		{
			GetSubStr(szFileOrg,3,'_',szInfoPoint);	
			GetSubStr(szFileOrg,4,'_',szType);
			if (strcmp(szType,"50")==0)  //50信息点暂停传送
			{
				sprintf(szFilePath,"%s%s",m_szCheckPath,szFileOrg);			
			  remove(szFilePath);
			  continue;
			}
			GetSubStr(szFileOrg,5,'_',szRate);
			GetSubStr(szFileOrg,1,'.',szJsonFile);
			strcat(szJsonFile,".json");
		}
	
		
		if(strcmp(m_kpid,"")!=0 && (strcmp(m_kpid,szInfoPoint)!=0 || strcmp(m_type,szType)!=0|| strcmp(m_rate,szRate)!=0))
		{
			expTrace("Y", __FILE__, __LINE__, "old kpid: %s",m_kpid);
			expTrace("Y", __FILE__, __LINE__, "new kpid: %s",szInfoPoint);
			
			GetSubStr(filelist[ibpos].sz_orgFname,3,'.',szFileDate);
			if (strcmp(m_rate,"L1")==0 || strcmp(m_rate,"L2")==0)
			{
				strcpy(szFileDate,szCDate);			
			}
				
			szFileDate[8]=0;
			GetSeq(m_kpid,m_type,m_rate,szFileDate);
			
			sprintf(szFilePath, "%sBOSSNM_%s_%s_%s_%s_%s_%s_020_%08d_%03d.json.tmp", m_szOutPath, m_szDrId,m_szNetId,m_kpid,m_type,m_rate, szFileDate, m_iSeqID,m_iOrder);
	
			//sprintf(szFilePath, "%sBOSSNM.%s.%s.%s.%08d.%s.2000.tmp", m_szOutPath, m_kpid,m_szNetId, szFileDate, m_iSeqID,m_szDrId);
		  expTrace("Y", __FILE__, __LINE__, "cout file: %s",szFilePath);
		  expTrace("Y", __FILE__, __LINE__, "begin pos:%d,end pos:%d",ibpos,iepos);
		  	
		  ofstream outInfo;
		  outInfo.open(szFilePath);
		  //outInfo<<"STA/"<<total_num<<"/"<<endl;
		  for (k=ibpos;k<=iepos;k++)
		  {
		    GetSubStr(filelist[k].sz_orgFname,9,'.',szSep);
		    if(strcmp(szSep,"END")==0) continue;
		    sprintf(szFilePath, "%s%s", m_szCheckPath,filelist[k].sz_orgFname);		
		
		    if((fp=fopen(szFilePath,"r"))==NULL)
        {
    	    expTrace("Y", __FILE__, __LINE__, "file open error :%s",szFilePath);    	 
  	       continue;
        }
        expTrace("Y", __FILE__, __LINE__, "deal file :%s",szFilePath); 
 	      while(1)
        {    	 
    	    if(fgets(buf,255,fp)==NULL) break; 
    	    //if(strncmp(buf,"STA",3)==0||strncmp(buf,"END",3)==0)
    	    //  continue;
    	    if(buf[strlen(buf)-1]=='\n')
    	      buf[strlen(buf)-1]=0;
				  outInfo<<buf<<endl;
				    				
		    }		  
			  fclose(fp);
			  remove(szFilePath);
			  
		  }
		  //outInfo<<"END"<<endl;
		  outInfo.close();
		  
		  sprintf(szFilePath, "%sBOSSNM_%s_%s_%s_%s_%s_%s_020_%08d_%03d.json.tmp", m_szOutPath, m_szDrId,m_szNetId,m_kpid,m_type,m_rate, szFileDate, m_iSeqID,m_iOrder);
			
			sprintf(szCopyFile, "%sBOSSNM_%s_%s_%s_%s_%s_%s_020_%08d_%03d.json", szCopyPath, m_szDrId,m_szNetId,m_kpid,m_type,m_rate, szFileDate, m_iSeqID,m_iOrder);
			 	    
		  //sprintf(szCopyFile, "%sBOSSNM.%s.%s.%s.%08d.%s.2000", szCopyPath, m_kpid,m_szNetId, szFileDate, m_iSeqID,m_szDrId);
		  				    
			if (0 != copyFile(szFilePath,szCopyFile))
			{
			  expTrace("Y", __FILE__, __LINE__, "fail to copy file :%s",szFilePath); 
			}
		  
		  sprintf(szCopyFile, "%sBOSSNM_%s_%s_%s_%s_%s_%s_020_%08d_%03d.json", m_szOutPath, m_szDrId,m_szNetId,m_kpid,m_type,m_rate, szFileDate, m_iSeqID,m_iOrder);
			  
		  //sprintf(szCopyFile, "%sBOSSNM.%s.%s.%s.%08d.%s.2000", m_szOutPath, m_kpid,m_szNetId, szFileDate, m_iSeqID,m_szDrId);
		  			    
		     
		  if(rename(szFilePath,szCopyFile)!=0)
	    {
	  	  expTrace("Y", __FILE__, __LINE__, "rename tmp error:from %s to %s;", szFilePath,szCopyFile);
	    }
		    
		  expTrace("Y", __FILE__, __LINE__, " infopoint:%s unit finished",m_kpid);	
		    
		    
		  total_num=0;
		  ibpos=j;
			
			strcpy(m_kpid,szInfoPoint);
			strcpy(m_type,szType);			
			strcpy(m_rate,szRate);
				
			
			expTrace("Y", __FILE__, __LINE__, "init kpid: %s",szInfoPoint);
				
			total_num=0;
			ibpos=j;
			iepos=j;
					
		}
		else if (strcmp(m_kpid,"")==0 )
		{
			expTrace("Y", __FILE__, __LINE__, "init kpid: %s",szInfoPoint);
				
			total_num=0;
			ibpos=j;	
			iepos=j;		
			strcpy(m_kpid,szInfoPoint);
			strcpy(m_type,szType);			
			strcpy(m_rate,szRate);	 
			
		}
		else
		{
			iepos=j;
			
		}
	}	
	
	iepos=iCountMax-1;
	
	if (strcmp(m_kpid,"")!=0)
	{
	GetSubStr(filelist[ibpos].sz_orgFname,3,'.',szFileDate);
	if (strcmp(m_rate,"L1")==0 || strcmp(m_rate,"L2")==0)
	{
		strcpy(szFileDate,szCDate);			
	}
	szFileDate[8]=0;	
	GetSeq(m_kpid,m_type,m_rate,szFileDate);
	//GetSeq(m_kpid,szFileDate);
	sprintf(szFilePath, "%sBOSSNM_%s_%s_%s_%s_%s_%s_020_%08d_%03d.json.tmp", m_szOutPath, m_szDrId,m_szNetId,m_kpid,m_type,m_rate, szFileDate, m_iSeqID,m_iOrder);
			
	//sprintf(szFilePath, "%sBOSSNM.%s.%s.%s.%08d.%s.2000.tmp", m_szOutPath, szInfoPoint,m_szNetId, szFileDate, m_iSeqID,m_szDrId);
	expTrace("Y", __FILE__, __LINE__, "cout file: %s",szFilePath);
		  	
	ofstream outInfo;
	outInfo.open(szFilePath);
	//outInfo<<"STA/"<<total_num<<"/"<<endl;
	for (k=ibpos;k<=iepos;k++)
	{
		 GetSubStr(filelist[k].sz_orgFname,9,'.',szSep);
		 if(strcmp(szSep,"END")==0) continue;
		    		
		 sprintf(szFilePath, "%s%s", m_szCheckPath,filelist[k].sz_orgFname);		
		
		 if((fp=fopen(szFilePath,"r"))==NULL)
     {
    	  expTrace("Y", __FILE__, __LINE__, "file open error :%s",szFilePath);    	 
  	    continue;
     }
      
 	   while(1)
     {    	 
    	  if(fgets(buf,255,fp)==NULL) break; 
    	  if(strncmp(buf,"STA",3)==0||strncmp(buf,"END",3)==0)
    	     continue;
    	  if(buf[strlen(buf)-1]=='\n')
    	     buf[strlen(buf)-1]=0;
				outInfo<<buf<<endl;
				    				
		 }		  
		fclose(fp);
		remove(szFilePath);    
			    
	}
	//outInfo<<"END"<<endl;
	outInfo.close();
	
	sprintf(szFilePath, "%sBOSSNM_%s_%s_%s_%s_%s_%s_020_%08d_%03d.json.tmp", m_szOutPath, m_szDrId,m_szNetId,m_kpid,m_type,m_rate, szFileDate, m_iSeqID,m_iOrder);
	sprintf(szCopyFile, "%sBOSSNM_%s_%s_%s_%s_%s_%s_020_%08d_%03d.json", szCopyPath, m_szDrId,m_szNetId,m_kpid,m_type,m_rate, szFileDate, m_iSeqID,m_iOrder);
	
	//sprintf(szFilePath, "%sBOSSNM.%s.%s.%s.%08d.%s.2000.tmp", m_szOutPath, m_kpid,m_szNetId, szFileDate, m_iSeqID,m_szDrId);
	    
	//sprintf(szCopyFile, "%sBOSSNM.%s.%s.%s.%08d.%s.2000", szCopyPath, m_kpid,m_szNetId, szFileDate, m_iSeqID,m_szDrId);
		  				    
	if (0 != copyFile(szFilePath,szCopyFile))
	{
			expTrace("Y", __FILE__, __LINE__, "fail to copy file :%s",szFilePath); 
	}
	
	sprintf(szCopyFile, "%sBOSSNM_%s_%s_%s_%s_%s_%s_020_%08d_%03d.json", m_szOutPath, m_szDrId,m_szNetId,m_kpid,m_type,m_rate, szFileDate, m_iSeqID,m_iOrder);
		    
	//sprintf(szCopyFile, "%sBOSSNM.%s.%s.%s.%08d.%s.2000", m_szOutPath, m_kpid,m_szNetId, szFileDate, m_iSeqID,m_szDrId);
	  			    
	     
	if(rename(szFilePath,szCopyFile)!=0)
	{
	   expTrace("Y", __FILE__, __LINE__, "rename tmp error:from %s to %s;", szFilePath,szCopyFile);
	}
		    
	expTrace("Y", __FILE__, __LINE__, " infopoint:%s unit finished",m_kpid);
		    
	total_num=0;
	ibpos=j;
	iepos=j;		
	strcpy(m_kpid,szInfoPoint);
	strcpy(m_type,szType);			
	strcpy(m_rate,szRate);	
}
		
	expTrace("Y", __FILE__, __LINE__, "all infopoint unit finished");
	return 0;	
}



int CInfoCom::GetInfoEnv()
{
	char sz_EnvName[31];
	char sz_EnvValue[301];
	
	CBindSQL ds(DBConn);	
	char szSql[100];
	sprintf(szSql, "select ENV_NAME,ENV_VALUE from INFO_PUB_ENV where SERVER_ID='Z'");
	ds.Open(szSql);
	while (ds>>sz_EnvName>>sz_EnvValue)
	{
		if (strcmp(sz_EnvName,"UP_CHECK_PATH")==0)
		{
			strcpy(m_szCheckPath,sz_EnvValue);
		}		
		else if (strcmp(sz_EnvName,"UP_OUT_PATH")==0)
		{
			strcpy(m_szOutPath,sz_EnvValue);			
		}		
		else if (strcmp(sz_EnvName,"UP_NETID")==0)
		{		
			strcpy(m_szNetId,sz_EnvValue);		
		}	
		else if (strcmp(sz_EnvName,"DR_ID")==0)
		{
			strcpy(m_szDrId,sz_EnvValue);
		}	
		else
		{
		}
	}
	ds.Close();	
	if(strlen(m_szCheckPath)==0||strlen(m_szOutPath)==0||strlen(m_szNetId)==0)
		return -1;
	chkAllDir(m_szOutPath);
	chkAllDir(m_szCheckPath);
	return 0;
}

/***************************************************
description:
  扫描指定目录并返回待发送文件列表

output:
  filelist 文件列表
return
  -1 扫描失败；
  0 扫描成功；
*****************************************************/
int CInfoCom::ScanFiles(char *pch_sourcePath,char *pch_fileFilter)
{
  char sz_nowtime[14+1];
	char sz_temp[255],sz_filter[10];
	char sz_filename[255],sz_scanPath[255];
	time_t timer;
	struct stat str_Buff;

	int i;
	
	
	filelist = new FILE_INFO[INFO_FILE_REC+1];

  //扫描源文件路径
  CF_CFscan  file_scaner;
  
  strcpy(sz_scanPath,pch_sourcePath);

  if(file_scaner.openDir(sz_scanPath)==-1)
	{
		sprintf(sz_errmsg,"Error in scan the DIR(%s)\n",sz_scanPath);
  	getCurTime(sz_errtime);
  	wrlog("infomgr","0","",'E','H',1003,002,sz_errtime,sz_errmsg,__FILE__,__LINE__);	
  	return -1;
	}
	
  i=0;
	while(1)
	{
	  if(file_scaner.getFile(pch_fileFilter,sz_temp)==100) break;
    char sz_Null[255];
    
    dispartFullName(sz_temp,sz_Null,sz_filename);
    
    stat(sz_temp,&str_Buff);
    
    strcpy(filelist[i].sz_orgFname,sz_filename);
    filelist[i].iFileSize=str_Buff.st_size;
    filelist[i].lCreateTime=str_Buff.st_ctime;
    filelist[i].iFlag=1;
    i++;
    if(i>=INFO_FILE_REC) break;
  }
  iCountMax=i;
  file_scaner.closeDir();
  
  
  qsort(&filelist[0],iCountMax,sizeof(struct FILE_INFO),_CmpFileName);
  
  return 0;
}


int CInfoCom::GetSeq(char *kpid,char *sType,char *szRate,char *szDate)
{
	char szCurDate[20];
	char sz_errtime[16];
	
	strcpy(m_kpid,kpid);
	try
	{
		getCurDate(szCurDate);
	  strncpy(m_szToday, szCurDate, 8);
	  m_szToday[8] = '\0';
	
	  int idoseq=0;
	  int iCount=0;
	  CBindSQL ds(DBConn);	
	  char szSql[300];
	  
	  sprintf(szSql, "select count(*),nvl(max(seq_id), 0) from info_pub_seq where dotime = '%s' and AuditId='%s' and TypeId='%s' and rate='%s' and ENDTIME='%s'", szDate,kpid,sType,szRate,m_szEndTime);
	   expTrace("Y", __FILE__, __LINE__, "SQL;%s", szSql);
	  ds.Open(szSql);
	  ds>>iCount>>idoseq;
	  ds.Close();
	  
	  if (0==iCount)
	  {
	  	m_iOrder=0;
	  }
	  else
	  {
	  	m_iOrder=iCount;
	  	m_iSeqID=idoseq;
	  }
    
    if (0==m_iOrder)
    {
	    sprintf(szSql, "select nvl(max(seq_id), 0) from info_pub_seq where dotime = '%s' and AuditId='%s' and TypeId='%s' and rate='%s'", szDate,kpid,sType,szRate);
	    expTrace("Y", __FILE__, __LINE__, "SQL;%s", szSql);
	    ds.Open(szSql);
	    if (ds>>idoseq)
	    {
		    m_iSeqID = idoseq+1;
	    }
	    ds.Close();
	  }
	  
   
	 
		sprintf(m_outfile, "%sBOSSNM_%s_%s_%s_%s_%s_%s_020_%08d_%03d.json", m_szOutPath, m_szDrId,m_szNetId,m_kpid,sType,szRate, szDate, m_iSeqID,m_iOrder);
	
		  //sprintf(m_outfile, "%sBOSSNM.%s.%s.%s.%08d.%s.2000", m_szOutPath, m_kpid,m_szNetId, szDate, m_iSeqID,m_szDrId);	    
		  
		
		sprintf(szSql, "insert into info_pub_seq(seq_id, dotime, outfile,auditid,typeid,rate,iorder,endtime) "
			  "values(%d, '%s','%s','%s','%s','%s',%d,'%s')", m_iSeqID, szDate, m_outfile,m_kpid,sType,szRate,m_iOrder,m_szEndTime);
		expTrace("Y", __FILE__, __LINE__, "SQL;%s", szSql);
		ds.Open(szSql);
		ds.Execute();
		ds.Close();	
		DBConn.Commit();
	  
	}
	catch(...)
	{
		getCurTime(sz_errtime);
    wrlog("infomgr","0","",'E','H',1001,002,sz_errtime,"Incom:Connect to database failed!",__FILE__,__LINE__);			
		exit(1);
		return -1;
	}
	return 0;
}


int CInfoCom::Init(int iMax)
{
	m_iSeqID = 1;
	filelist=NULL;
	iMaxRec=iMax;


  if(GetInfoEnv() != 0) 
	{
		getCurTime(sz_errtime);
    wrlog("infomgr","0","",'E','H',1000,001,sz_errtime,"get env error",__FILE__,__LINE__);	
		return -1;
	}
	else
	{
		expTrace("Y", __FILE__, __LINE__, "get env success!");
	}
	
	return 0;

}

int CInfoCom::Check()//检测信息点检测目录下，END文件是否都生成了exp by hlp
{
	char szFileName[100];
	char szTemp[100];
	char szSep[10];
	
	struct dirent *dir;
	struct stat sbuf;
	DIR *dp;
	int k,i;
	

	
	expTrace("Y", __FILE__, __LINE__, "begin to check infocom");
	m_vecEndTime.clear();
	if ((dp = opendir(m_szCheckPath)) == NULL)
	{
		char szLog[300];
		sprintf(szLog,  "Can't open directory: ", m_szCheckPath);
		Init(iMaxRec);
   	expTrace("Y", __FILE__, __LINE__, szLog);
	}
	
	while ((dir = readdir(dp)) != NULL)	//在检测目录下
	{
		if ( !strcmp(dir->d_name,".") || !strcmp(dir->d_name,"..") ) 
			continue;
		strcpy(szFileName,dir->d_name); 
    GetSubStr(szFileName,8,'.',m_szEndTime);
    
    if(m_szEndTime[0]=='.') continue;
    
    k=0;
    for(i=0;i<m_vecEndTime.size(); i++)
    {
    	if(strcmp(m_szEndTime,m_vecEndTime[i].c_str())==0)
    	{
    		k++;
    	}
    }
    
    if(0==k)
    {
    	expTrace("Y", __FILE__, __LINE__, "insert time :%s",m_szEndTime);
    	m_vecEndTime.push_back(m_szEndTime);
    }
    else if (k>50)
    {
    	break;
    }
    
  }
  
  closedir(dp);
   
  strcpy(m_szEndTime,"");

  
  //检测扫描目录下各系统的批次结束文件是否到齐
  
 
	
	expTrace("Y", __FILE__, __LINE__, "vec size :%d",m_vecEndTime.size());
	expTrace("Y", __FILE__, __LINE__, "max rec :%d",iMaxRec);
	
	
   for(i=0;i<m_vecEndTime.size(); i++)
   {
   	  k=0;
   	  strcpy(m_szEndTime,m_vecEndTime[i].c_str());
   	  expTrace("Y", __FILE__, __LINE__, "current endtime :%s",m_szEndTime);
   	  
   	   if ((dp = opendir(m_szCheckPath)) == NULL)
	     {
		     char szLog[300];
		     sprintf(szLog,  "Can't open directory: ", m_szCheckPath);
   	     expTrace("Y", __FILE__, __LINE__, szLog);
	     }
   	  
    	while ((dir = readdir(dp)) != NULL)	//在检测目录下
	    {
		    if ( !strcmp(dir->d_name,".") || !strcmp(dir->d_name,"..") ) 
			     continue;
		    strcpy(szFileName,dir->d_name);
        GetSubStr(szFileName,8,'.',szTemp);
        GetSubStr(szFileName,9,'.',szSep);
        
        if('.'==szTemp[0]) continue;
        
        //expTrace("Y", __FILE__, __LINE__, "FileName :%s",szFileName);
        
        if(strcmp(m_szEndTime,szTemp)==0 && strcmp(szSep,"END")==0)
        {
        	k++;
        	
        }
        
        if(k>=iMaxRec)
        {
        	return 0;
        	expTrace("Y", __FILE__, __LINE__, "time :%s,num:%d",m_szEndTime,k);
        }
       
      }
      closedir(dp);
  }
  strcpy(m_szEndTime,"");
  return -1;
  
}

char* CInfoCom::GetSubStr(char* szSrcStr, int nIndex, char cSeparator, char* szDest)
{
	sprintf(szDest, "%c", cSeparator);
	
	while( nIndex > 1 )
	{
		char* pSep = NULL;
		pSep = strchr( szSrcStr, cSeparator );
		if( pSep == NULL )	
			break;
		
		szSrcStr = pSep+1;
		nIndex--;
	}
	
	if(nIndex == 1)
	{
		char* pSep = NULL;
		pSep = strchr( szSrcStr, cSeparator );			
		if( pSep != NULL )	
		{
			strncpy( szDest, szSrcStr, pSep-szSrcStr );
			szDest[pSep-szSrcStr] = '\0';
		}
		else
			strcpy( szDest, szSrcStr);
	}
	
	return szDest;	

}