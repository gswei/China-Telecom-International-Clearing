/*
1.20060809 修改了check_rule表结构，增加每种校验方式可配两个范围，V为值域,L为长度范围
2.20070319 增加校验方式m
*/






#include "formatcheck.h"


extern char Field[MAX_FIELD_COUNT][FMT_MAX_FIELD_LEN];


int _CmpCheckEnum(const void *s1,const void *s2)
{
  char *ss1,*ss2;
  int ss3,ss4;
  int i;
  ss3 = ( (SENUMCHK *)s1 )->iFieldIndex;
  ss4 = ( (SENUMCHK *)s2 )->iFieldIndex;
  i=ss3-ss4;
  if(i) return i;

  ss1 = ( (SENUMCHK *)s1 )->szRcdFmt;
  ss2 = ( (SENUMCHK *)s2 )->szRcdFmt;
 
  return ( strcmp(ss1,ss2) ) ;
}

int _CmpCheckRule(const void *s1,const void *s2)
{
  char *ss1,*ss2;
  char ss3,ss4;
  int i;
  ss3 = ( (SCheckRule *)s1 )->chCheckFmt;
  ss4 = ( (SCheckRule *)s2 )->chCheckFmt;
  i=ss3-ss4;
  if(i) return i;

  ss3 = ( (SCheckRule *)s1 )->chCheckType;
  ss4 = ( (SCheckRule *)s2 )->chCheckType;
  i=ss3-ss4;
  if(i) return i;

  ss1 = ( (SCheckRule *)s1 )->szSource_ID;
  ss2 = ( (SCheckRule *)s2 )->szSource_ID;
 
  return ( strcmp(ss1,ss2) ) ;
}

int _CmpFakeHeader(const void *s1,const void *s2)
{
  char *ss1,*ss2;
  char ss3,ss4;
  int i;
  ss1 = ( (SFakeHeader *)s1 )->szSource_ID;
  ss2 = ( (SFakeHeader *)s2 )->szSource_ID;
  i=( strcmp(ss1,ss2) );
  if(i) return i;

  ss1 = ( (SFakeHeader *)s1 )->szFakeHeader;
  ss2 = ( (SFakeHeader *)s2 )->szFakeHeader;
  i=strlen(ss1)-strlen(ss2);
  if(i) return i;
  return ( strcmp(ss1,ss2) ) ;
}

int _CmpCallnoHeaderHeader(const void *s1,const void *s2)
{
  char *ss1,*ss2;
  int i;

  ss1 = ( (SCallnoHeader *)s1 )->szCallnoHeader;
  ss2 = ( (SCallnoHeader *)s2 )->szCallnoHeader;
  i=strlen(ss1)-strlen(ss2);
  if(i) return i;
  return ( strcmp(ss1,ss2) ) ;
}
int _CmpIpNo(const void *s1,const void *s2)
{
  char *ss1,*ss2;
  int i;

  ss1 = ( (SIpNo *)s1 )->szIpNo;
  ss2 = ( (SIpNo *)s2 )->szIpNo;
  return ( strncmp(ss1,ss2,strlen(ss2)) ) ;
  
}
CFormatCheck::CFormatCheck()
{
  pCheckRule=NULL;
  iCheckRule_Num=0;
  pCur_CheckRule=NULL;
  pFakeHeader=NULL;
  iFakeHeader_Num=0;
  pCur_FakeHeader=NULL;
  pCallnoHeader=NULL;
  iCallnoHeader_Num=0;
  pFile=NULL;
  iFile_Num=0;
  pIpNo=NULL;
  iIpNo_Num=0;
  pENumChk=NULL;
  iENumChk_num=0;
}

CFormatCheck::~CFormatCheck()
{
  if(!pCheckRule)
  {
    delete[] pCheckRule;
    pCheckRule=NULL;
  }
  if(!pFakeHeader)
  {
    delete[] pFakeHeader;
    pFakeHeader=NULL;
  }
  if(!pCallnoHeader)
  {
    delete[] pCallnoHeader;
    pCallnoHeader=NULL;
  }
  if(!pIpNo)
  {
    delete[] pIpNo;
    pIpNo=NULL;
  }

  if(!pFile)
  {

    for(int i=0;i<iFile_Num;i++)
    {
      if(pFile[i].iIs_Open) pFile[i].Outfp.Close();
    }
    delete[] pFile;
    pFile=NULL;
  }
  
}



int CFormatCheck::splEnumValue( char *ss,SENUMCHK &TmpOut )

{

 int i;

 char *ss0,*ss1,*ss2;

     i = 0; ss0 = ss;

     while( ( ss1 = strchr( ss0,';' ) ) != NULL ) {

        *ss1 = 0;
        sprintf( TmpOut.szEnumValue[i],"%s",ss0 );
        ss0 = ss1+1;
        *ss1 = ';';
		i++;
		TmpOut.iEnumValue_Num=i;
		if(i>=100) return (0);
     }

     sprintf( TmpOut.szEnumValue[i],"%s",ss0 );
     TmpOut.iEnumValue_Num=i+1;

 return(0);

}

//int CFormatCheck::LoadCheckDataToMem(CDatabase *Database,char *SourceGrp,char *szDebugFlag,char Is_Ann)
int CFormatCheck::LoadCheckDataToMem(DBConnection connection,char *SourceGrp,char *szDebugFlag,char Is_Ann)
{
	iCheckRouteIndex=-1;
	iAnn_Index=0;
	iCalledNo_Index=0;
	iFake_Header_Flag=0;

  sprintf(szDebug_Flag,"%s",szDebugFlag);
  //m_DataBase=Database;
  conn = connection;
  if(LoadCheckRule(SourceGrp)) return -1;
  if(LoadFakeHeader(SourceGrp)) return -1;
  if(LoadCallnoHeader()) return -1;
  if(LoadCheckEnum()) return -1;

//#ifdef SETTLE
if(Is_Ann == 'Y')
{
  if(LoadIpNo()) return -1;
}
//#endif
  return 0;
}


int CFormatCheck::LoadIpNo()
{
  //CBindSQL ds(*m_DataBase);
  //ds.Open("select count(*) from c_ip_no", SELECT_QUERY);
  //ds>>iIpNo_Num;
  //ds.Close();
	
 Statement stmt = conn.createStatement();
 string sql = "select count(*) from c_ip_no";
 stmt.setSQLString(sql);
 stmt.execute();
 stmt>>iIpNo_Num;
 

  expTrace(szDebug_Flag, __FILE__, __LINE__, 
    "iIpNo_Num=%d;", iIpNo_Num);

  if (iIpNo_Num <= 0)
  {
    strcpy(szLogStr, "select count(*) from c_ip_no");
    expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
    return 0;
  }

  pIpNo = new SIpNo[iIpNo_Num];
  if (!pIpNo)
  {
    strcpy(szLogStr, "New pIpNo fail.");
    expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
    return (-1);
  }

  //ds.Open("select ipno,busi_abbr from c_ip_no", SELECT_QUERY );
  sql = "select ipno,busi_abbr from c_ip_no";
  stmt.setSQLString(sql);
  stmt.execute();
   /* Repeat reading all record */
  for (int i=0; i<iIpNo_Num; i++)
  {
    //ds>>pIpNo[i].szIpNo>>pIpNo[i].szBusi_Abbr;
	stmt>>pIpNo[i].szIpNo>>pIpNo[i].szBusi_Abbr;

    delSpace(pIpNo[i].szIpNo,0);
    delSpace(pIpNo[i].szBusi_Abbr,0);
  }
  //ds.Close();

  qsort(&pIpNo[0],iIpNo_Num,sizeof( struct SIpNo ),_CmpIpNo);


  //ds.Open("select count(distinct busi_abbr) from c_ip_no", SELECT_QUERY);
 // ds>>iFile_Num;
  //ds.Close();
  sql = "select count(distinct busi_abbr) from c_ip_no";
  stmt.setSQLString(sql);
  stmt.execute();
  stmt>>iFile_Num;

  expTrace(szDebug_Flag, __FILE__, __LINE__, 
    "iFile_Num=%d;", iFile_Num);

  if (iFile_Num <= 0)
  {
    strcpy(szLogStr, "select count(distinct busi_abbr) from c_ip_no");
    expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
    return 0;
  }
  iFile_Num++;
  pFile = new SFile[iFile_Num];
  if (!pFile)
  {
    strcpy(szLogStr, "New pFile fail.");
    expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
    return (-1);
  }

  //ds.Open("select distinct busi_abbr from c_ip_no", SELECT_QUERY );
  sql = "select distinct busi_abbr from c_ip_no";
  stmt.setSQLString(sql);
  stmt.execute();
   /* Repeat reading all record */
  for (int i=0; i<(iFile_Num-1); i++)
  {
    //ds>>pFile[i].szBusi_Abbr;
	stmt>>pFile[i].szBusi_Abbr;

    delSpace(pFile[i].szBusi_Abbr,0);
    pFile[i].iIs_Open = 0;
//    pFile[i].pOutfp=NULL;
    pFile[i].szdate[0] = 0;
  }
  //ds.Close();
  stmt.close();

  pFile[iFile_Num-1].szBusi_Abbr[0]='0';
  pFile[iFile_Num-1].szBusi_Abbr[1]=0;
  pFile[iFile_Num-1].iIs_Open = 0;
  pFile[iFile_Num-1].szdate[0] = 0;

  return 0;
}


int CFormatCheck::LoadCallnoHeader()
{
  //CBindSQL ds(*m_DataBase);
 // ds.Open("select count(*) from C_CALLNO_HEADER", SELECT_QUERY);
  //ds>>iCallnoHeader_Num;
  //ds.Close();
  Statement stmt = conn.createStatement();
  string sql = "select count(*) from C_CALLNO_HEADER";
  stmt.setSQLString(sql);
  stmt.execute();
  stmt>>iCallnoHeader_Num;


  expTrace(szDebug_Flag, __FILE__, __LINE__, 
    "iCallnoHeader_Num=%d;", iCallnoHeader_Num);

  if (iCallnoHeader_Num <= 0)
  {
    strcpy(szLogStr, "select count(*) from C_CALLNO_HEADER");
    expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
    return 0;
  }

  pCallnoHeader = new SCallnoHeader[iCallnoHeader_Num];
  if (!pCallnoHeader)
  {
    strcpy(szLogStr, "New pCallnoHeader fail.");
    expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
    return (-1);
  }

  //ds.Open("select callno_header from C_CALLNO_HEADER", SELECT_QUERY );
  sql = "select callno_header from C_CALLNO_HEADER";
  stmt.setSQLString(sql);
  stmt.execute();
   /* Repeat reading all record */
  for (int i=0; i<iCallnoHeader_Num; i++)
  {
    //ds>>pCallnoHeader[i].szCallnoHeader;
	stmt>>pCallnoHeader[i].szCallnoHeader;

    delSpace(pCallnoHeader[i].szCallnoHeader,0);
  }
  //ds.Close();
  stmt.close();
  qsort(&pCallnoHeader[0],iCallnoHeader_Num,sizeof( struct SCallnoHeader ),_CmpCallnoHeaderHeader);

  return 0;
}

int CFormatCheck::LoadCheckEnum()
{
  //CBindSQL ds(*m_DataBase);
  char Tmp_EnumValue[512];
  //ds.Open("select count(*) from c_check_enum", SELECT_QUERY);
  //ds>>iENumChk_num;
  //ds.Close();
  string sql = "select count(*) from c_check_enum";
  Statement stmt = conn.createStatement();
  stmt.setSQLString(sql);
  stmt.execute();
  stmt>>iENumChk_num;

  expTrace(szDebug_Flag, __FILE__, __LINE__, 
    "iENumChk_num=%d;", iENumChk_num);

  if (iENumChk_num <= 0)
  {
    strcpy(szLogStr, "select count(*) from c_check_enum");
    expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
    return 0;
  }

  pENumChk = new SENUMCHK[iENumChk_num];
  if (!pENumChk)
  {
    strcpy(szLogStr, "New pENumChk fail.");
    expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
    return (-1);
  }

  //ds.Open("select record_fmt,field_index,enum_value from c_check_enum", SELECT_QUERY );
	sql = "select record_fmt,field_index,enum_value from c_check_enum";
	stmt.setSQLString(sql);
	stmt.execute();
   /* Repeat reading all record */
  for (int i=0; i<iENumChk_num; i++)
  {
    //ds>>pENumChk[i].szRcdFmt>>pENumChk[i].iFieldIndex>>Tmp_EnumValue;
	stmt>>pENumChk[i].szRcdFmt>>pENumChk[i].iFieldIndex>>Tmp_EnumValue;

    delSpace(Tmp_EnumValue,0);
    splEnumValue(Tmp_EnumValue,pENumChk[i]);
  }
  //ds.Close();
  stmt.close();

  qsort(&pENumChk[0],iENumChk_num,sizeof( struct SENUMCHK ),_CmpCheckEnum);

  pCur_ENumChk=&pENumChk[0];
  return 0;

}

int CFormatCheck::LoadCheckRule(char *SourceGrp)
{
  //CBindSQL ds(*m_DataBase);
 // ds.Open("select count(*) from C_SOURCE_GROUP_CONFIG a,c_check_rule b where a.SOURCE_GROUP=:szPipeId and a.source_id=b.source_id", SELECT_QUERY);
  //ds<<SourceGrp;
  //ds>>iCheckRule_Num;
 // ds.Close();
  string sql = "select count(*) from C_SOURCE_GROUP_CONFIG a,c_check_rule b where a.SOURCE_GROUP=:szPipeId and a.source_id=b.source_id";
  Statement stmt = conn.createStatement();
  stmt.setSQLString(sql);
  stmt<<SourceGrp;
  stmt.execute();
  stmt>>iCheckRule_Num;

  expTrace(szDebug_Flag, __FILE__, __LINE__, 
    "iCheckRule_Num=%d;", iCheckRule_Num);

  if (iCheckRule_Num <= 0)
  {
    strcpy(szLogStr, "select count(*) from C_SOURCE_GROUP_CONFIG a,c_check_rule b where a.SOURCE_GROUP=:szPipeId and a.source_id=b.source_id");
    expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
    return 0;
  }

  pCheckRule = new SCheckRule[iCheckRule_Num];
  if (!pCheckRule)
  {
    strcpy(szLogStr, "New pCheckRule fail.");
    expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
    return (-1);
  }

  //ds.Open("select b.source_id,b.check_fmt,nvl(b.check_type,'V'),b.up_limit,b.down_limit from C_SOURCE_GROUP_CONFIG a,c_check_rule b \
  //	where a.SOURCE_GROUP=:szPipeId and a.source_id=b.source_id", SELECT_QUERY );
  //ds<<SourceGrp;
  sql = "select b.source_id,b.check_fmt,nvl(b.check_type,'V'),b.up_limit,b.down_limit from C_SOURCE_GROUP_CONFIG a,c_check_rule b "
		"where a.SOURCE_GROUP=:szPipeId and a.source_id=b.source_id";
  stmt.setSQLString(sql);
  stmt<<SourceGrp;
  stmt.execute();

   /* Repeat reading all record */
  for (int i=0; i<iCheckRule_Num; i++)
  {
    //ds>>pCheckRule[i].szSource_ID>>pCheckRule[i].chCheckFmt>>pCheckRule[i].chCheckType
    //  >>pCheckRule[i].szUp_Limit>>pCheckRule[i].szLow_Limit;
	stmt>>pCheckRule[i].szSource_ID>>pCheckRule[i].chCheckFmt>>pCheckRule[i].chCheckType>>pCheckRule[i].szUp_Limit>>pCheckRule[i].szLow_Limit;

    delSpace(pCheckRule[i].szUp_Limit,0);
    delSpace(pCheckRule[i].szLow_Limit,0);
  }
  //ds.Close();
  stmt.close();

  qsort(&pCheckRule[0],iCheckRule_Num,sizeof( struct SCheckRule ),_CmpCheckRule);

  pCur_CheckRule=&pCheckRule[0];
  return 0;

}
int CFormatCheck::LoadFakeHeader(char *SourceGrp)
{
  //CBindSQL ds(*m_DataBase);
  //ds.Open("select count(*) from C_SOURCE_GROUP_CONFIG a,c_fakeheader b where a.SOURCE_GROUP=:szPipeId and a.source_id=b.source_id", SELECT_QUERY);
  //ds<<SourceGrp;
  //ds>>iFakeHeader_Num;
  //ds.Close();
  string sql = "select count(*) from C_SOURCE_GROUP_CONFIG a,c_fakeheader b where a.SOURCE_GROUP=:szPipeId and a.source_id=b.source_id";
  Statement stmt = conn.createStatement();
  stmt.setSQLString(sql);
  stmt<<SourceGrp;
  stmt.execute();
  stmt>>iFakeHeader_Num;

  expTrace(szDebug_Flag, __FILE__, __LINE__, 
    "iFakeHeader_Num=%d;", iFakeHeader_Num);

  if (iFakeHeader_Num <= 0)
  {
    strcpy(szLogStr, "select count(*) from C_SOURCE_GROUP_CONFIG a,c_fakeheader b where a.SOURCE_GROUP=:szPipeId and a.source_id=b.source_id");
    expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
    return 0;
  }

  pFakeHeader = new SFakeHeader[iFakeHeader_Num];
  if (!pFakeHeader)
  {
    strcpy(szLogStr, "New pFakeHeader fail.");
    expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
    return (-1);
  }

  //ds.Open("select b.source_id,b.fake_head,b.delete_flag from C_SOURCE_GROUP_CONFIG a,c_fakeheader b \
  //	where a.SOURCE_GROUP=:szPipeId and a.source_id=b.source_id", SELECT_QUERY );
  //ds<<SourceGrp;
	sql = "select b.source_id,b.fake_head,b.delete_flag from C_SOURCE_GROUP_CONFIG a,c_fakeheader b"
		  "where a.SOURCE_GROUP=:szPipeId and a.source_id=b.source_id";
	stmt.setSQLString(sql);
	stmt<<SourceGrp;
	stmt.execute();

   /* Repeat reading all record */
  for (int i=0; i<iFakeHeader_Num; i++)
  {
    //ds>>pFakeHeader[i].szSource_ID>>pFakeHeader[i].szFakeHeader
    //  >>pFakeHeader[i].chDelFlag;
	stmt>>pFakeHeader[i].szSource_ID>>pFakeHeader[i].szFakeHeader>>pFakeHeader[i].chDelFlag;

    delSpace(pFakeHeader[i].szFakeHeader,0);
  }
  //ds.Close();
  stmt.close();

  qsort(&pFakeHeader[0],iFakeHeader_Num,sizeof( struct SFakeHeader ),_CmpFakeHeader);

  pCur_FakeHeader=&pFakeHeader[0];
  return 0;

}

int CFormatCheck::Init_Ann_Dir(char *szAnn_Dir,char *szlocalnet,int iProcIndex)
{
char szTmpDate[9];
getCurDate(szTmpDate);
//  for(int i=0;i<iFile_Num;i++)
  for(int i=0;i<iFile_Num-1;i++)
  {
//    if(!strcmp(szTmpDate,pFile[i].szdate)) continue;
    sprintf(pFile[i].szFileName,"%s%s/",szAnn_Dir,pFile[i].szBusi_Abbr);
    chkDir(pFile[i].szFileName);
    sprintf(pFile[i].szFileName,"%s%s%s%s_%d.ann",pFile[i].szFileName,szlocalnet,pFile[i].szBusi_Abbr,szTmpDate,iProcIndex);
    sprintf(pFile[i].TmpFileName,"%s.tmp",pFile[i].szFileName);
    sprintf(pFile[i].szdate,"%s",szTmpDate);
  }
return 0;
}

int CFormatCheck::Check_Ann_Flag(CFmt_Change &outrcd)
{
  if(iAnn_Index==0) return 1;
  if(Field[iAnn_Index][0] != '0') return 1;
  SIpNo OTmp;
  SIpNo *pTmp=NULL;
  SFile  *pFileTmp=NULL;
  strncpy(OTmp.szIpNo,Field[iCalledNo_Index],10);
  OTmp.szIpNo[10]=0;
  pTmp = (SIpNo *)(bsearch(&OTmp,pIpNo,iIpNo_Num,sizeof(struct SIpNo),_CmpIpNo));
//  if(!pTmp) pFileTmp = &pFile[iFile_Num-1];
  if(!pTmp) return 0;
    else
    {
      for(int i=0;i<iFile_Num;i++)
      {
        if(!strcmp(pTmp->szBusi_Abbr,pFile[i].szBusi_Abbr))
        {
          pFileTmp = &pFile[i];
          break;
        }
      }
    }

  if(!(pFileTmp->iIs_Open))
  {
    FILE *AnnFile;
    if((AnnFile=fopen(pFileTmp->szFileName,"r"))!=NULL)
    {
      fclose(AnnFile);
      copyFile(pFileTmp->szFileName,pFileTmp->TmpFileName);
    }
    else 
    {
      if((AnnFile=fopen(pFileTmp->TmpFileName,"r"))!=NULL)
      {
        fclose(AnnFile);
        unlink(pFileTmp->TmpFileName);
      }
    }
    pFileTmp->Outfp.Open(pFileTmp->TmpFileName,'A');
    pFileTmp->iIs_Open=1;
  }

    pFileTmp->Outfp.writeRec(outrcd);

  return 0;
}

int CFormatCheck::CommitAnnFile()
{

  for(int i=0;i<iFile_Num;i++)
  {
    if(pFile[i].iIs_Open)
    {
      pFile[i].Outfp.Close();
      if(rename(pFile[i].TmpFileName,pFile[i].szFileName)) return -1;
      pFile[i].iIs_Open = 0;
    }
  }

return 0;

}

int CFormatCheck::RollbackAnnFile()
{

  for(int i=0;i<iFile_Num;i++)
  {
    if(pFile[i].iIs_Open)
    {
      pFile[i].Outfp.Close();
      unlink(pFile[i].TmpFileName);
      pFile[i].iIs_Open = 0;
    }
  }

return 0;

}



int CFormatCheck::FHCHECK_CH(char *str,char *szSourceID,int iCheckLen)
{
  int length=strlen(str);
//非空
  if(length==0) return -1;
  if(iCheckLen>0) 
  {
    if(length>iCheckLen) length=iCheckLen;
  }
//16进制  
  for(int i = 0;i<length;i++)
  {  
    if(str[i]=='\n') break;
    if((str[i]>='A')&&(str[i]<='F'))
      continue;
    if(((str[i]&0x00ff)<47)||((str[i]&0x00ff)>58))//如果包含非数字字符
    {
      return -1;
    }
  }
  return 0;
}

int CFormatCheck::FCTIMECHECK_CH(char *str,char *szSourceID,int iCheckLen)
{
  char szYear[5];
  char szMonth[3];
  char szDay[3];
  char szHour[3];
  char szMinu[3];
  char szSec[3];
  char cur_Time[15];
  int length=strlen(str);
  char szTmpDateTime[9];
  szTmpDateTime[0]='\0';
//时间格式限定为14位
  if(length==12)
  {
    char Tmp[15];
    sprintf(Tmp,"20%s",str);
    sprintf(str,"%s",Tmp);
  }
  
  length=strlen(str);
  if(length!=14)
  {
    if(length!=iTime_FmtLen)
  	  return -1;
  //有定义的格式转换成YYYYMMDDhhmmss
    char *pTime_Fmt;
  //Year;
    pTime_Fmt=strstr(szTime_Fmt,YEARSTRING);
    if(pTime_Fmt)
    {
      strncpy(szYear,str+(pTime_Fmt-szTime_Fmt),4);
      szYear[4]=0;
    }else {
            pTime_Fmt=strstr(szTime_Fmt,YEARSTRING2);
            if(pTime_Fmt)
            {
              strncpy(szMonth,str+(pTime_Fmt-szTime_Fmt),2);
              szMonth[2]=0;
              sprintf(szYear,"20%s",szMonth);
            }else{
                   getCurDate(szTmpDateTime);
                   strncpy(szYear,szTmpDateTime,4);
                   szYear[4]=0;
                 }
           }
    //Month
    pTime_Fmt=strstr(szTime_Fmt,MONTHSTRING);
    if(!pTime_Fmt) return -1;
    strncpy(szMonth,str+(pTime_Fmt-szTime_Fmt),2);
    szMonth[2]=0;
    
    if((szTmpDateTime[0]!='\0')&&(strncmp(szMonth,szTmpDateTime+4,2)>0))
    {
      sprintf(szYear,"%d",atoi(szYear)-1);
    }
    //Day
    pTime_Fmt=strstr(szTime_Fmt,DAYSTRING);
    if(!pTime_Fmt) return -1;
    strncpy(szDay,str+(pTime_Fmt-szTime_Fmt),2);
    szDay[2]=0;
    //Hour
    pTime_Fmt=strstr(szTime_Fmt,HOURSTRING);
    if(!pTime_Fmt) return -1;
    strncpy(szHour,str+(pTime_Fmt-szTime_Fmt),2);
    szHour[2]=0;
    //Minu
    pTime_Fmt=strstr(szTime_Fmt,MINUSTRING);
    if(!pTime_Fmt) return -1;
    strncpy(szMinu,str+(pTime_Fmt-szTime_Fmt),2);
    szMinu[2]=0;
    //Sec
    pTime_Fmt=strstr(szTime_Fmt,SECSTRING);
    if(pTime_Fmt)
    {
      strncpy(szSec,str+(pTime_Fmt-szTime_Fmt),2);
      szSec[2]=0;
    }else{
           pTime_Fmt=strstr(szTime_Fmt,SECSTRING2);
           if(!pTime_Fmt) return -1;
           szSec[0]=*(str+(pTime_Fmt-szTime_Fmt));
           sprintf(szSec,"%d",(szSec[0]-48)*6);
         }
    
    sprintf(str,"%s%s%s%s%s%s",szYear,szMonth,szDay,szHour,szMinu,szSec);
  }
  
//校验时间的合法性  
  char szDate[9];
  char szTime[7];
  strncpy(szDate,str,8);
  szDate[8]=0;
  if(CheckDate(szDate)==false) return -1;
  strncpy(szTime,str+8,6);
  szTime[6]=0;
  if(CheckTime(szTime)==false) return -1;

  if(iCheckRule_Num)
  {
    SCheckRule sCheckRule;
    sprintf(sCheckRule.szSource_ID,"%s",szSourceID);
    sCheckRule.chCheckFmt=CTIMECHECK_CH;
    sCheckRule.chCheckType=CHECK_VALUE_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    if((strcmp(str,pCur_CheckRule->szUp_Limit)>0)||(strcmp(str,pCur_CheckRule->szLow_Limit)<0))
  	return -2;


    sCheckRule.chCheckType=CHECK_LENTH_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    {
      if(strlen(str)>atoi(pCur_CheckRule->szUp_Limit))
      {
        str[atoi(pCur_CheckRule->szUp_Limit)]=0;
        return -1;
      }
      if(strlen(str)<atoi(pCur_CheckRule->szLow_Limit))
  	  return -1;
    }
  }

  if(iRcd_Arr_Dur >= 0)
  {
    getCurTime(cur_Time);
    if(Check_Rcd_Arr_Dur(cur_Time,str)) return -90;
  }
  return 0;
}




int CFormatCheck::FcTIMECHECK_CH(char *str,char *szSourceID,int iCheckLen)
{
  char szYear[5];
  char szMonth[3];
  char szDay[3];
  char szHour[3];
  char szMinu[3];
  char szSec[3];
  char cur_Time[15];
  int length=strlen(str);
  char szTmpDateTime[9];
  szTmpDateTime[0]='\0';
//时间格式限定为14位

  length=strlen(str);
  if(length!=14)
  {
    if(length!=iTime_FmtLen)
  	  return -1;
  //有定义的格式转换成YYYYMMDDhhmmss
    char *pTime_Fmt;
  //Year;
    pTime_Fmt=strstr(szTime_Fmt,YEARSTRING);
    if(pTime_Fmt)
    {
      strncpy(szYear,str+(pTime_Fmt-szTime_Fmt),4);
      szYear[4]=0;
    }else {
            pTime_Fmt=strstr(szTime_Fmt,YEARSTRING2);
            if(pTime_Fmt)
            {
              strncpy(szMonth,str+(pTime_Fmt-szTime_Fmt),2);
              szMonth[2]=0;
              sprintf(szYear,"20%s",szMonth);
            }else{
                   getCurDate(szTmpDateTime);
                   strncpy(szYear,szTmpDateTime,4);
                   szYear[4]=0;
                 }
           }
    //Month
    pTime_Fmt=strstr(szTime_Fmt,MONTHSTRING);
    if(!pTime_Fmt) return -1;
    strncpy(szMonth,str+(pTime_Fmt-szTime_Fmt),2);
    szMonth[2]=0;
    
    if((szTmpDateTime[0]!='\0')&&(strncmp(szMonth,szTmpDateTime+4,2)>0))
    {
      sprintf(szYear,"%d",atoi(szYear)-1);
    }
    //Day
    pTime_Fmt=strstr(szTime_Fmt,DAYSTRING);
    if(!pTime_Fmt) return -1;
    strncpy(szDay,str+(pTime_Fmt-szTime_Fmt),2);
    szDay[2]=0;
    //Hour
    pTime_Fmt=strstr(szTime_Fmt,HOURSTRING);
    if(!pTime_Fmt) return -1;
    strncpy(szHour,str+(pTime_Fmt-szTime_Fmt),2);
    szHour[2]=0;
    //Minu
    pTime_Fmt=strstr(szTime_Fmt,MINUSTRING);
    if(!pTime_Fmt) return -1;
    strncpy(szMinu,str+(pTime_Fmt-szTime_Fmt),2);
    szMinu[2]=0;
    //Sec
    pTime_Fmt=strstr(szTime_Fmt,SECSTRING);
    if(pTime_Fmt)
    {
      strncpy(szSec,str+(pTime_Fmt-szTime_Fmt),2);
      szSec[2]=0;
    }else{
           pTime_Fmt=strstr(szTime_Fmt,SECSTRING2);
           if(!pTime_Fmt) return -1;
           szSec[0]=*(str+(pTime_Fmt-szTime_Fmt));
           sprintf(szSec,"%d",(szSec[0]-48)*6);
         }
    
    sprintf(str,"%s%s%s%s%s%s",szYear,szMonth,szDay,szHour,szMinu,szSec);
  }
  
//校验时间的合法性  
  char szDate[9];
  char szTime[7];
  strncpy(szDate,str,8);
  szDate[8]=0;
  if(CheckDate(szDate)==false) return -1;
  strncpy(szTime,str+8,6);
  szTime[6]=0;
  if(CheckTime(szTime)==false) return -1;

  if(iCheckRule_Num)
  {
    SCheckRule sCheckRule;
    sprintf(sCheckRule.szSource_ID,"%s",szSourceID);
    sCheckRule.chCheckFmt=cTIMECHECK_CH;
    sCheckRule.chCheckType=CHECK_VALUE_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    if((strcmp(str,pCur_CheckRule->szUp_Limit)>0)||(strcmp(str,pCur_CheckRule->szLow_Limit)<0))
  	return -2;


    sCheckRule.chCheckType=CHECK_LENTH_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    {
      if(strlen(str)>atoi(pCur_CheckRule->szUp_Limit))
      {
        str[atoi(pCur_CheckRule->szUp_Limit)]=0;
        return -1;
      }
      if(strlen(str)<atoi(pCur_CheckRule->szLow_Limit))
  	  return -1;
    }
  }

  if(iRcd_Arr_Dur >= 0)
  {
    getCurTime(cur_Time);
    if(Check_Rcd_Arr_Dur(cur_Time,str)) return -90;
  }
  return 0;
}





int CFormatCheck::FsTIMECHECK_CH(char *str,char *szSourceID,int iCheckLen)
{

  char cur_Time[15];
  int length=strlen(str);
//非空
  if(length==0) return -1;
  if(iCheckLen>0) 
  {
    if(length>iCheckLen) length=iCheckLen;
  }
//数字
  for(int i = 0;i<length;i++)
  {  
    if(str[i]=='\n') break;
    if(((str[i]&0x00ff)<47)||((str[i]&0x00ff)>58))//如果包含非数字字符
    {
      return -1;
    }
  }
     time_t in_sec=atoi(str);
     struct tm tm_time=*localtime(&in_sec);
     tm_time.tm_mon++;
     sprintf(str,"%04d%02d%02d%02d%02d%02d",
     	tm_time.tm_year+1900,tm_time.tm_mon,tm_time.tm_mday,
     	tm_time.tm_hour,tm_time.tm_min,tm_time.tm_sec);

//校验时间的合法性  
  char szDate[9];
  char szTime[7];
  strncpy(szDate,str,8);
  szDate[8]=0;
  if(CheckDate(szDate)==false) return -1;
  strncpy(szTime,str+8,6);
  szTime[6]=0;
  if(CheckTime(szTime)==false) return -1;

  if(iCheckRule_Num)
  {
    SCheckRule sCheckRule;
    sprintf(sCheckRule.szSource_ID,"%s",szSourceID);
    sCheckRule.chCheckFmt=cTIMECHECK_CH;
    sCheckRule.chCheckType=CHECK_VALUE_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    if((strcmp(str,pCur_CheckRule->szUp_Limit)>0)||(strcmp(str,pCur_CheckRule->szLow_Limit)<0))
  	return -2;


    sCheckRule.chCheckType=CHECK_LENTH_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    {
      if(strlen(str)>atoi(pCur_CheckRule->szUp_Limit))
      {
        str[atoi(pCur_CheckRule->szUp_Limit)]=0;
        return -1;
      }
      if(strlen(str)<atoi(pCur_CheckRule->szLow_Limit))
  	  return -1;
    }
  }

  if(iRcd_Arr_Dur >= 0)
  {
    getCurTime(cur_Time);
    if(Check_Rcd_Arr_Dur(cur_Time,str)) return -90;
  }
  return 0;
}


int CFormatCheck::FDTIMECHECK_CH(char *str,char *szSourceID,int iCheckLen)
{
  char szYear[5];
  char szMonth[3];
  char szDay[3];
  char szHour[3];
  char szMinu[3];
  char szSec[3];
  char cur_Time[15];
  int length=strlen(str);
  char szTmpDateTime[9];
  szTmpDateTime[0]='\0';
//时间格式限定为14位
  if(length==12)
  {
    char Tmp[15];
    sprintf(Tmp,"20%s",str);
    sprintf(str,"%s",Tmp);
  }
  length=strlen(str);
    if(length!=14)
  {
    if(length!=iTime_FmtLen)
  	  return -1;

    
  //有定义的格式转换成YYYYMMDDhhmmss
    char *pTime_Fmt;
  //Year;
    pTime_Fmt=strstr(szTime_Fmt,YEARSTRING);
    if(pTime_Fmt)
    {
      strncpy(szYear,str+(pTime_Fmt-szTime_Fmt),4);
      szYear[4]=0;
    }else {
            pTime_Fmt=strstr(szTime_Fmt,YEARSTRING2);
            if(pTime_Fmt)
            {
              strncpy(szMonth,str+(pTime_Fmt-szTime_Fmt),2);
              szMonth[2]=0;
              sprintf(szYear,"20%s",szMonth);
            }else{
                   getCurDate(szTmpDateTime);
                   strncpy(szYear,szTmpDateTime,4);
                   szYear[4]=0;
                 }
           }
    //Month
    pTime_Fmt=strstr(szTime_Fmt,MONTHSTRING);
    if(!pTime_Fmt) return -1;
    strncpy(szMonth,str+(pTime_Fmt-szTime_Fmt),2);
    szMonth[2]=0;
    
    if((szTmpDateTime[0]!='\0')&&(strncmp(szMonth,szTmpDateTime+4,2)>0))
    {
      sprintf(szYear,"%d",atoi(szYear)-1);
    }
    //Day
    pTime_Fmt=strstr(szTime_Fmt,DAYSTRING);
    if(!pTime_Fmt) return -1;
    strncpy(szDay,str+(pTime_Fmt-szTime_Fmt),2);
    szDay[2]=0;
    //Hour
    pTime_Fmt=strstr(szTime_Fmt,HOURSTRING);
    if(!pTime_Fmt) return -1;
    strncpy(szHour,str+(pTime_Fmt-szTime_Fmt),2);
    szHour[2]=0;
    //Minu
    pTime_Fmt=strstr(szTime_Fmt,MINUSTRING);
    if(!pTime_Fmt) return -1;
    strncpy(szMinu,str+(pTime_Fmt-szTime_Fmt),2);
    szMinu[2]=0;
    //Sec
    pTime_Fmt=strstr(szTime_Fmt,SECSTRING);
    if(pTime_Fmt)
    {
      strncpy(szSec,str+(pTime_Fmt-szTime_Fmt),2);
      szSec[2]=0;
    }else{
           pTime_Fmt=strstr(szTime_Fmt,SECSTRING2);
           if(!pTime_Fmt) return -1;
           szSec[0]=*(str+(pTime_Fmt-szTime_Fmt));
           sprintf(szSec,"%d",(szSec[0]-48)*6);
         }
    
    sprintf(str,"%s%s%s%s%s%s",szYear,szMonth,szDay,szHour,szMinu,szSec);
  }
  
//校验时间的合法性  
  char szDate[9];
  char szTime[7];
  strncpy(szDate,str,8);
  szDate[8]=0;
  if(CheckDate(szDate)==false) return -1;
  strncpy(szTime,str+8,6);
  szTime[6]=0;
  if(CheckTime(szTime)==false) return -1;

//  getCurTime(cur_Time);
//  if(Check_Rcd_Arr_Dur(cur_Time,str)) return -4;

  return 0;
}



int CFormatCheck::FdTIMECHECK_CH(char *str,char *szSourceID,int iCheckLen)
{
  char szYear[5];
  char szMonth[3];
  char szDay[3];
  char szHour[3];
  char szMinu[3];
  char szSec[3];
  char cur_Time[15];
  int length=strlen(str);
  char szTmpDateTime[9];
  szTmpDateTime[0]='\0';
//时间格式限定为14位

  length=strlen(str);
    if(length!=14)
  {
    if(length!=iTime_FmtLen)
  	  return -1;

    
  //有定义的格式转换成YYYYMMDDhhmmss
    char *pTime_Fmt;
  //Year;
    pTime_Fmt=strstr(szTime_Fmt,YEARSTRING);
    if(pTime_Fmt)
    {
      strncpy(szYear,str+(pTime_Fmt-szTime_Fmt),4);
      szYear[4]=0;
    }else {
            pTime_Fmt=strstr(szTime_Fmt,YEARSTRING2);
            if(pTime_Fmt)
            {
              strncpy(szMonth,str+(pTime_Fmt-szTime_Fmt),2);
              szMonth[2]=0;
              sprintf(szYear,"20%s",szMonth);
            }else{
                   getCurDate(szTmpDateTime);
                   strncpy(szYear,szTmpDateTime,4);
                   szYear[4]=0;
                 }
           }
    //Month
    pTime_Fmt=strstr(szTime_Fmt,MONTHSTRING);
    if(!pTime_Fmt) return -1;
    strncpy(szMonth,str+(pTime_Fmt-szTime_Fmt),2);
    szMonth[2]=0;
    
    if((szTmpDateTime[0]!='\0')&&(strncmp(szMonth,szTmpDateTime+4,2)>0))
    {
      sprintf(szYear,"%d",atoi(szYear)-1);
    }
    //Day
    pTime_Fmt=strstr(szTime_Fmt,DAYSTRING);
    if(!pTime_Fmt) return -1;
    strncpy(szDay,str+(pTime_Fmt-szTime_Fmt),2);
    szDay[2]=0;
    //Hour
    pTime_Fmt=strstr(szTime_Fmt,HOURSTRING);
    if(!pTime_Fmt) return -1;
    strncpy(szHour,str+(pTime_Fmt-szTime_Fmt),2);
    szHour[2]=0;
    //Minu
    pTime_Fmt=strstr(szTime_Fmt,MINUSTRING);
    if(!pTime_Fmt) return -1;
    strncpy(szMinu,str+(pTime_Fmt-szTime_Fmt),2);
    szMinu[2]=0;
    //Sec
    pTime_Fmt=strstr(szTime_Fmt,SECSTRING);
    if(pTime_Fmt)
    {
      strncpy(szSec,str+(pTime_Fmt-szTime_Fmt),2);
      szSec[2]=0;
    }else{
           pTime_Fmt=strstr(szTime_Fmt,SECSTRING2);
           if(!pTime_Fmt) return -1;
           szSec[0]=*(str+(pTime_Fmt-szTime_Fmt));
           sprintf(szSec,"%d",(szSec[0]-48)*6);
         }
    
    sprintf(str,"%s%s%s%s%s%s",szYear,szMonth,szDay,szHour,szMinu,szSec);
  }
  
//校验时间的合法性  
  char szDate[9];
  char szTime[7];
  strncpy(szDate,str,8);
  szDate[8]=0;
  if(CheckDate(szDate)==false) return -1;
  strncpy(szTime,str+8,6);
  szTime[6]=0;
  if(CheckTime(szTime)==false) return -1;

//  getCurTime(cur_Time);
//  if(Check_Rcd_Arr_Dur(cur_Time,str)) return -4;

  return 0;
}


int CFormatCheck::FNUMCHECK_CH(char *str,char *szSourceID,int iCheckLen)
{
  int length=strlen(str);
//非空
  if(length==0) return -1;
  if(iCheckLen>0) 
  {
    if(length>iCheckLen) length=iCheckLen;
  }
//数字
  for(int i = 0;i<length;i++)
  {  
    if(str[i]=='\n') break;
    if(((str[i]&0x00ff)<47)||((str[i]&0x00ff)>58))//如果包含非数字字符
    {
      return -1;
    }
  }


  if(iCheckRule_Num)
  {
    SCheckRule sCheckRule;
    sprintf(sCheckRule.szSource_ID,"%s",szSourceID);
    sCheckRule.chCheckFmt=NUMCHECK_CH;
    sCheckRule.chCheckType=CHECK_VALUE_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    if((atoi(str)>atoi(pCur_CheckRule->szUp_Limit))||(atoi(str)<atoi(pCur_CheckRule->szLow_Limit)))
  	return -9;


    sCheckRule.chCheckType=CHECK_LENTH_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    {
      if(strlen(str)>atoi(pCur_CheckRule->szUp_Limit))
      {
        str[atoi(pCur_CheckRule->szUp_Limit)]=0;
        return -1;
      }
      if(strlen(str)<atoi(pCur_CheckRule->szLow_Limit))
  	  return -1;
    }
  }


  return 0;

}

int CFormatCheck::FINTCHECK_CH(char *str,char *szSourceID,int iCheckLen)
{
  int length=strlen(str);
//非空
  if(length==0) return -1;
  if(iCheckLen>0) 
  {
    if(length>iCheckLen) length=iCheckLen;
  }
//数字
  int i=0;

  if(str[0] == '-')
  {
    i=1;
  }

  for(;i<length;i++)
  {  
    if(str[i]=='\n') break;
    if(((str[i]&0x00ff)<47)||((str[i]&0x00ff)>58))//如果包含非数字字符
    {
      return -1;
    }
  }

  if(iCheckRule_Num)
  {
    SCheckRule sCheckRule;
    sprintf(sCheckRule.szSource_ID,"%s",szSourceID);
    sCheckRule.chCheckFmt=INTCHECK_CH;
    sCheckRule.chCheckType=CHECK_VALUE_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    if((atoi(str)>atoi(pCur_CheckRule->szUp_Limit))||(atoi(str)<atoi(pCur_CheckRule->szLow_Limit)))
  	return -1;

    sCheckRule.chCheckType=CHECK_LENTH_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    {
      if(strlen(str)>atoi(pCur_CheckRule->szUp_Limit))
      {
        str[atoi(pCur_CheckRule->szUp_Limit)]=0;
        return -1;
      }
      if(strlen(str)<atoi(pCur_CheckRule->szLow_Limit))
  	  return -1;
    }
  }

  return 0;

}






int CFormatCheck::FENUMCHECK_CH(char *str,char *szSourceID,int index,char *szFile_Fmt)
{
  if(!iENumChk_num)
  	return -1;
  if(strcmp(pCur_ENumChk->szRcdFmt,szFile_Fmt)||pCur_ENumChk->iFieldIndex!=index+1)
  {
    SENUMCHK sTmpEnumchk;
    sprintf(sTmpEnumchk.szRcdFmt,"%s",szFile_Fmt);
    sTmpEnumchk.iFieldIndex=index+1;
    pCur_ENumChk=((SENUMCHK *)(bsearch(&sTmpEnumchk,pENumChk,iENumChk_num,sizeof(struct SENUMCHK),_CmpCheckEnum)));
    if(!pCur_ENumChk)
    {
      pCur_ENumChk=&pENumChk[0];
      return -1;
    }
  }

  for(int i=0;i<pCur_ENumChk->iEnumValue_Num;i++)
  {
    if((strcmp(str,pCur_ENumChk->szEnumValue[i])==0))
      return 0;
  }

  return -1;

}


int CFormatCheck::FTELNUMCHECK_CH(char *str,char *szSourceID,int iCheckLen)
{
  return 0;
}
int CFormatCheck::FLENCHECK_CH(char *str,char *szSourceID,int iCheckLen)
{

  if(iCheckRule_Num)
  {
    SCheckRule sCheckRule;
    sprintf(sCheckRule.szSource_ID,"%s",szSourceID);
    sCheckRule.chCheckFmt=LENCHECK_CH;
    sCheckRule.chCheckType=CHECK_VALUE_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    if((strlen(str)>atoi(pCur_CheckRule->szUp_Limit))||(strlen(str)<atoi(pCur_CheckRule->szLow_Limit)))
  	return -1;

    sCheckRule.chCheckType=CHECK_LENTH_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    {
      if(strlen(str)>atoi(pCur_CheckRule->szUp_Limit))
      {
        str[atoi(pCur_CheckRule->szUp_Limit)]=0;
        return -1;
      }
      if(strlen(str)<atoi(pCur_CheckRule->szLow_Limit))
  	  return -1;
    }
  }

  
  return 0;
}
int CFormatCheck::FNULLCHECK_CH(char *str,char *szSourceID,int iCheckLen)
{
  int length=strlen(str);
//非空
  if(length==0) return -1;
  return 0;
}

int CFormatCheck::FCALLINGCHECK_CH(char *str,char *szSourceID,int iCheckLen,char checkType)
{
  int length=strlen(str);
  int iIs_All_Zore=0;
//非空
//  if(length==0) return 0;
//去填充符


  if(checkType==CALLINGCHECKa_CH)
  {
    for(int i = 0;i<length;i++)
   {  
     if(((str[i]&0x00ff)>47)&&((str[i]&0x00ff)<58))//如果包含非数字字符
     {
       sprintf(str,"%s",str+i);
       break;
     }
   }
  }
/*  else
  {
    for(int i = length-1;i>=0;i--)
    {  
//      if(((str[i]&0x00ff)>47)&&((str[i]&0x00ff)<58))//如果包含非数字字符
      if(((str[i]&0x00ff)!=69)&&((str[i]&0x00ff)!=70)&&((str[i])!=' ')) //非E、F
      {
        str[i+1]=0;
        break;
      }
    }
  }
*///08机以FF为结束符
/*char *pFF;
  pFF=strstr(str,"FF");
  if(pFF) *pFF=0;
*/
  Remove_connector(str);
  
  length=strlen(str);
  if(iCheckLen>0) 
  {
    if(length>iCheckLen) length=iCheckLen;
  }

//数字
  for(int i = length-1;i>=0;i--)
  {  
    if(((str[i]&0x00ff)<47)||((str[i]&0x00ff)>58))//如果包含非数字字符
    {
      return -20;
    }
  }


  length=strlen(str);
  
  if(length>8)
  {
    int iCallHeader_Index;
    for(iCallHeader_Index=0;iCallHeader_Index<iCallnoHeader_Num;iCallHeader_Index++)
    {
      if(!strncmp(str,pCallnoHeader[iCallHeader_Index].szCallnoHeader,strlen(pCallnoHeader[iCallHeader_Index].szCallnoHeader)))
    	break;
    }

    if(iCallHeader_Index == iCallnoHeader_Num)
    {
      char Tmpstr[100];
      sprintf(Tmpstr,"0%s",str);
      sprintf(str,"%s",Tmpstr);
    }
  }
/*
  if(!strcmp(str,"8631828"))
  {
   printf("here\n");
    }
*/
/*
  for(iIs_All_Zore = length-1;iIs_All_Zore>=0;iIs_All_Zore--)
  {
    if(str[iIs_All_Zore] != '0')//如果包含非数字字符
    {
      break;
    }
  }
  if(iIs_All_Zore == (-1)) str[0]=0;
*/


  if(iCheckRule_Num)
  {
    SCheckRule sCheckRule;
    sprintf(sCheckRule.szSource_ID,"%s",szSourceID);
    sCheckRule.chCheckFmt=CALLINGCHECK_CH;
    sCheckRule.chCheckType=CHECK_VALUE_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    if((strcmp(str,pCur_CheckRule->szUp_Limit)>0)||(strcmp(str,pCur_CheckRule->szLow_Limit)<0))
  	return -1;

    sCheckRule.chCheckType=CHECK_LENTH_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    {
      if(strlen(str)>atoi(pCur_CheckRule->szUp_Limit))
      {
        str[atoi(pCur_CheckRule->szUp_Limit)]=0;
        return -1;
      }
      if(strlen(str)<atoi(pCur_CheckRule->szLow_Limit))
  	  return -1;
    }
  }

  return 0;
}

int CFormatCheck::FCALLEDCHECK_CH(char *str,char *szSourceID,int iCheckLen,char checkType)
{
  int length=strlen(str);
  int iIs_All_Zore=0;

//非空
  if(length==0) return -1;
//去填充符

  if(checkType==CALLEDCHECKb_CH)
  {
    for(int i = 0;i<length;i++)
   {  
     if(((str[i]&0x00ff)>47)&&((str[i]&0x00ff)<58))//如果包含非数字字符
     {
       sprintf(str,"%s",str+i);
       break;
     }
   }
  }
/*
  else
  {
    for(int i = length-1;i>=0;i--)
    {  
//      if(((str[i]&0x00ff)>47)&&((str[i]&0x00ff)<58))//如果包含非数字字符
      if(((str[i]&0x00ff)!=69)&&((str[i]&0x00ff)!=70)&&((str[i])!=' '))
      {
        str[i+1]=0;
        break;
      }
    }
  }
*//*
//08机以FF为结束符
char *pFF;
  pFF=strstr(str,"FF");
  if(pFF) *pFF=0;
*/
  Remove_connector(str);


  length=strlen(str);
  if(iCheckLen>0) 
  {
    if(length>iCheckLen) length=iCheckLen;
  }

//数字
  for(int i = length-1;i>=0;i--)
  {  
    if(((str[i]&0x00ff)<47)||((str[i]&0x00ff)>58))//如果包含非数字字符
    {
      return -3;
    }
  }
length=strlen(str);
  
/*
  if(strlen(str)>8)
  {
    int iCallHeader_Index;
    for(iCallHeader_Index=0;iCallHeader_Index<iCallnoHeader_Num;iCallHeader_Index++)
    {
      if(!strncmp(str,pCallnoHeader[iCallHeader_Index].szCallnoHeader,strlen(pCallnoHeader[iCallHeader_Index].szCallnoHeader)))
    	break;
    }

    if(iCallHeader_Index == iCallnoHeader_Num)
    {
      char Tmpstr[100];
      sprintf(Tmpstr,"0%s",str);
      sprintf(str,"%s",Tmpstr);
    }
  }
*/



/*
  for(iIs_All_Zore = length-1;iIs_All_Zore>=0;iIs_All_Zore--)
  {
    if(str[iIs_All_Zore] != '0')//如果包含非数字字符
    {
      break;
    }
  }
  if(iIs_All_Zore == (-1)) str[0]=0;
*/



  if(iFakeHeader_Num)
  {
    int iSour_Index=iFakeHeader_Num-1;
    for(;iSour_Index>=0;iSour_Index--)
    {
      if(!strcmp(pFakeHeader[iSour_Index].szSource_ID,szSourceID)) break;
    }
    for(;iSour_Index>=0;iSour_Index--)
    {
      if(strcmp(pFakeHeader[iSour_Index].szSource_ID,szSourceID)) break;
      if(!strncmp(str,pFakeHeader[iSour_Index].szFakeHeader,strlen(pFakeHeader[iSour_Index].szFakeHeader)))
   	  {
   	    if(pFakeHeader[iSour_Index].chDelFlag=='Y') return -1;
   	    else
   	  	{
   	  	  sprintf(str,"%s",str+strlen(pFakeHeader[iSour_Index].szFakeHeader));
          iFake_Header_Flag = 1;
   	  	  break;
   	  	}
      }
    
    }
  }

  if(iCheckRule_Num)
  {
    SCheckRule sCheckRule;
    sprintf(sCheckRule.szSource_ID,"%s",szSourceID);
    sCheckRule.chCheckFmt=CALLEDCHECK_CH;
    sCheckRule.chCheckType=CHECK_VALUE_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    if((strcmp(str,pCur_CheckRule->szUp_Limit)>0)||(strcmp(str,pCur_CheckRule->szLow_Limit)<0))
  	return -1;

    sCheckRule.chCheckType=CHECK_LENTH_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    {
      if(strlen(str)>atoi(pCur_CheckRule->szUp_Limit))
      {
        str[atoi(pCur_CheckRule->szUp_Limit)]=0;
        return -1;
      }
      if(strlen(str)<atoi(pCur_CheckRule->szLow_Limit))
  	  return -1;
    }
  }


  sprintf(szTmpCalledNo,"%s%s%s",szPreCalledNo,str,szAftCalledNo);
  sprintf(str,"%s",szTmpCalledNo);
  return 0;
}
int CFormatCheck::FCALLED30CHECK_CH(char *str,char *szSourceID,int iCheckLen,char checkType)
{
  int length=strlen(str);
  int iIs_All_Zore=0;

//非空
  if(length==0) return -1;
//去填充符

  if(checkType==CALLED30CHECKb_CH)
  {
    for(int i = 0;i<length;i++)
   {  
     if(((str[i]&0x00ff)>47)&&((str[i]&0x00ff)<58))//如果包含非数字字符
     {
       sprintf(str,"%s",str+i);
       break;
     }
   }
  }

  Remove_connector(str);
  length=strlen(str);
  if(iCheckLen>0) 
  {
    if(length>iCheckLen) length=iCheckLen;
  }

//数字
  for(int i = length-1;i>=0;i--)
  {  
    if(((str[i]&0x00ff)<47)||((str[i]&0x00ff)>58))//如果包含非数字字符
    {
      return -3;
    }
  }
length=strlen(str);
  

  if(iFakeHeader_Num)
  {
    int iSour_Index=iFakeHeader_Num-1;
    for(;iSour_Index>=0;iSour_Index--)
    {
      if(!strcmp(pFakeHeader[iSour_Index].szSource_ID,szSourceID)) break;
    }
    for(;iSour_Index>=0;iSour_Index--)
    {
      if(strcmp(pFakeHeader[iSour_Index].szSource_ID,szSourceID)) break;
      if(!strncmp(str,pFakeHeader[iSour_Index].szFakeHeader,strlen(pFakeHeader[iSour_Index].szFakeHeader)))
   	  {
   	    if(pFakeHeader[iSour_Index].chDelFlag=='Y') return -1;
   	    else
   	  	{
   	  	  sprintf(str,"%s",str+strlen(pFakeHeader[iSour_Index].szFakeHeader));
          iFake_Header_Flag = 1;
   	  	  break;
   	  	}
      }
    
    }
  }

  if(iCheckRule_Num)
  {
    SCheckRule sCheckRule;
    sprintf(sCheckRule.szSource_ID,"%s",szSourceID);
    sCheckRule.chCheckFmt=CALLEDCHECK_CH;
    sCheckRule.chCheckType=CHECK_VALUE_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    if((strcmp(str,pCur_CheckRule->szUp_Limit)>0)||(strcmp(str,pCur_CheckRule->szLow_Limit)<0))
  	return -1;

    sCheckRule.chCheckType=CHECK_LENTH_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    {
      if(strlen(str)>atoi(pCur_CheckRule->szUp_Limit))
      {
        str[atoi(pCur_CheckRule->szUp_Limit)]=0;
        return -1;
      }
      if(strlen(str)<atoi(pCur_CheckRule->szLow_Limit))
  	  return -1;
    }
  }

  sprintf(szTmpCalledNo,"%s%s%s",szPreCalledNo,str,szAftCalledNo);
  sprintf(str,"%s",szTmpCalledNo);

  if(strlen(str)>30) str[30]=0;
  return 0;
}

int CFormatCheck::FDURCHECK_CH(char *str,char *szSourceID,int iCheckLen)
{
  int length=strlen(str);
//非空
  if(length==0) return -1;

  if(iCheckLen>0) 
  {
    if(length>iCheckLen) length=iCheckLen;
  }
//数字
  for(int i = 0;i<length;i++)
  {  
    if(str[i]=='\n') break;
    if(((str[i]&0x00ff)<47)||((str[i]&0x00ff)>58))//如果包含非数字字符
    {
      return -1;
    }
  }
  if(atoi(str) == 0) return -1;
  
  if(iCheckRule_Num)
  {
    SCheckRule sCheckRule;
    sprintf(sCheckRule.szSource_ID,"%s",szSourceID);
    sCheckRule.chCheckFmt=DURCHECK_CH;
    sCheckRule.chCheckType=CHECK_VALUE_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    if((atoi(str)>atoi(pCur_CheckRule->szUp_Limit))||(atoi(str)<atoi(pCur_CheckRule->szLow_Limit)))
  	return -1;

    sCheckRule.chCheckType=CHECK_LENTH_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    {
      if(strlen(str)>atoi(pCur_CheckRule->szUp_Limit))
      {
        str[atoi(pCur_CheckRule->szUp_Limit)]=0;
        return -1;
      }
      if(strlen(str)<atoi(pCur_CheckRule->szLow_Limit))
  	  return -1;
    }
 }

  sprintf(str,"%d",atoi(str));



  return 0;
}




int CFormatCheck::FMDURCHECK_CH(char *str,char *szSourceID,int iCheckLen)
{
  int length=strlen(str);
//非空
  if(length==0) return -1;

  if(iCheckLen>0) 
  {
    if(length>iCheckLen) length=iCheckLen;
  }
//数字
  for(int i = 0;i<length;i++)
  {  
    if(str[i]=='\n') break;
    if(((str[i]&0x00ff)<47)||((str[i]&0x00ff)>58))//如果包含非数字字符
    {
      return -1;
    }
  }
  //转为秒
  int tmp_duration=atoi(str);
  if((tmp_duration%10)>0) tmp_duration = tmp_duration/10+1;
    else tmp_duration = tmp_duration/10;

  sprintf(str,"%d",tmp_duration);
  
  if(iCheckRule_Num)
  {
    SCheckRule sCheckRule;
    sprintf(sCheckRule.szSource_ID,"%s",szSourceID);
    sCheckRule.chCheckFmt=MDURCHECK_CH;
    sCheckRule.chCheckType=CHECK_VALUE_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    if((atoi(str)>atoi(pCur_CheckRule->szUp_Limit))||(atoi(str)<atoi(pCur_CheckRule->szLow_Limit)))
  	return -1;

    sCheckRule.chCheckType=CHECK_LENTH_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    {
      if(strlen(str)>atoi(pCur_CheckRule->szUp_Limit))
      {
        str[atoi(pCur_CheckRule->szUp_Limit)]=0;
        return -1;
      }
      if(strlen(str)<atoi(pCur_CheckRule->szLow_Limit))
  	  return -1;
    }
 }




  return 0;
}



int CFormatCheck::FFEECHECK_CH(char *str,char *szSourceID,int iCheckLen)
{
  int length=strlen(str);
//非空
  if(length==0) 
  {
    str[0] = '0';
    str[1] = 0 ;
    length = 1;
//    return -1;
  }
  if(iCheckLen>0) 
  {
    if(length>iCheckLen) length=iCheckLen;
  }
//数字
  for(int i = 0;i<length;i++)
  {  
    if(str[i]=='\n') break;
    if(((str[i]&0x00ff)<47)||((str[i]&0x00ff)>58))//如果包含非数字字符
    {
      return -1;
    }
  }
//  if(atoi(str) == 0) return -1;
  
  if(iCheckRule_Num)
  {
    SCheckRule sCheckRule;
    sprintf(sCheckRule.szSource_ID,"%s",szSourceID);
    sCheckRule.chCheckFmt=FEECHECK_CH;
    sCheckRule.chCheckType=CHECK_VALUE_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    if((atoi(str)>atoi(pCur_CheckRule->szUp_Limit))||(atoi(str)<atoi(pCur_CheckRule->szLow_Limit)))
  	return -1;

    sCheckRule.chCheckType=CHECK_LENTH_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    {
      if(strlen(str)>atoi(pCur_CheckRule->szUp_Limit))
      {
        str[atoi(pCur_CheckRule->szUp_Limit)]=0;
        return -1;
      }
      if(strlen(str)<atoi(pCur_CheckRule->szLow_Limit))
  	  return -1;
    }
  }

  return 0;
}

int CFormatCheck::FFREECHECK_CH(char *str,char *szSourceID,int iCheckLen)
{
  int length=strlen(str);
//非空
  if(length==0) return -1;
//  if(str[0]=='0') return -2;

  if(iCheckRule_Num)
  {
    SCheckRule sCheckRule;
    sprintf(sCheckRule.szSource_ID,"%s",szSourceID);
    sCheckRule.chCheckFmt=FREECHECK_CH;
    sCheckRule.chCheckType=CHECK_VALUE_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    if((atoi(str)>atoi(pCur_CheckRule->szUp_Limit))||(atoi(str)<atoi(pCur_CheckRule->szLow_Limit)))
  	return -1;

    sCheckRule.chCheckType=CHECK_LENTH_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    {
      if(strlen(str)>atoi(pCur_CheckRule->szUp_Limit))
      {
        str[atoi(pCur_CheckRule->szUp_Limit)]=0;
        return -1;
      }
      if(strlen(str)<atoi(pCur_CheckRule->szLow_Limit))
  	  return -1;
    }
  }
  
  sprintf(str,"%d",atoi(str));

  return 0;
}
int CFormatCheck::FROUTECHECK_CH(char *str,int index)
{
  int length=strlen(str);
//非空
  if(length==0) return -1;

iCheckRouteIndex=index;
return 0;
}


int CFormatCheck::FZDATECHECK_CH(char *str,char *szSourceID,int iCheckLen)
{
  char szYear[5];
  char szMonth[3];
  char szDay[3];
  char szHour[3];
  char szMinu[3];
  char szSec[3];
  char cur_Time[15];
  int length=strlen(str);
//日期格式限定为8位
  if(length==6)
  {
    char Tmp[15];
    sprintf(Tmp,"20%s",str);
    sprintf(str,"%s",Tmp);
  }
  
  length=strlen(str);
  if(length!=8)
  {
    if(length!=iZDate_FmtLen)
  	  return -1;
  //有定义的格式转换成YYYYMMDD
    char *pTime_Fmt;
  //Year;
    pTime_Fmt=strstr(szZDate_Fmt,YEARSTRING);
    if(pTime_Fmt)
    {
      strncpy(szYear,str+(pTime_Fmt-szZDate_Fmt),4);
      szYear[4]=0;
    }else {
            pTime_Fmt=strstr(szZDate_Fmt,YEARSTRING2);
            if(pTime_Fmt)
            {
              strncpy(szMonth,str+(pTime_Fmt-szZDate_Fmt),2);
              szMonth[2]=0;
              sprintf(szYear,"20%s",szMonth);
            }else{
                   char szTmpDateTime[9];
                   getCurDate(szTmpDateTime);
                   strncpy(szYear,szTmpDateTime,4);
                   szYear[4]=0;
                 }
           }
    //Month
    pTime_Fmt=strstr(szZDate_Fmt,MONTHSTRING);
    if(!pTime_Fmt) return -1;
    strncpy(szMonth,str+(pTime_Fmt-szZDate_Fmt),2);
    szMonth[2]=0;
    //Day
    pTime_Fmt=strstr(szZDate_Fmt,DAYSTRING);
    if(!pTime_Fmt) return -1;
    strncpy(szDay,str+(pTime_Fmt-szZDate_Fmt),2);
    szDay[2]=0;

    sprintf(str,"%s%s%s",szYear,szMonth,szDay);
  }
  
//校验时间的合法性  
  char szDate[9];
  char szTime[7];
  strncpy(szDate,str,8);
  szDate[8]=0;
  if(CheckDate(szDate)==false) return -1;


  if(iCheckRule_Num)
  {
    SCheckRule sCheckRule;
    sprintf(sCheckRule.szSource_ID,"%s",szSourceID);
    sCheckRule.chCheckFmt=ZDATECHECK_CH;
    sCheckRule.chCheckType=CHECK_VALUE_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    if((strcmp(str,pCur_CheckRule->szUp_Limit)>0)||(strcmp(str,pCur_CheckRule->szLow_Limit)<0))
  	return -2;

    sCheckRule.chCheckType=CHECK_LENTH_TYPE_CH;
    pCur_CheckRule=((SCheckRule *)(bsearch(&sCheckRule,pCheckRule,iCheckRule_Num,sizeof(struct SCheckRule),_CmpCheckRule)));
    if(pCur_CheckRule)
    {
      if(strlen(str)>atoi(pCur_CheckRule->szUp_Limit))
      {
        str[atoi(pCur_CheckRule->szUp_Limit)]=0;
        return -1;
      }
      if(strlen(str)<atoi(pCur_CheckRule->szLow_Limit))
  	  return -1;
    }
  }



  return 0;
}


int CFormatCheck::FzTIMECHECK_CH(char *str,char *szSourceID,int iCheckLen)
{
  char szYear[5];
  char szMonth[3];
  char szDay[3];
  char szHour[3];
  char szMinu[3];
  char szSec[3];
  char cur_Time[15];
  int length=strlen(str);
//时间格式限定为6位
  if(length!=6)
  {
    if(length!=izTime_FmtLen)
  	  return -1;
  //有定义的格式转换成hhmmss
    char *pTime_Fmt;
    //Hour
    pTime_Fmt=strstr(szzTime_Fmt,HOURSTRING);
    if(!pTime_Fmt) return -1;
    strncpy(szHour,str+(pTime_Fmt-szzTime_Fmt),2);
    szHour[2]=0;
    //Minu
    pTime_Fmt=strstr(szzTime_Fmt,MINUSTRING);
    if(!pTime_Fmt) return -1;
    strncpy(szMinu,str+(pTime_Fmt-szzTime_Fmt),2);
    szMinu[2]=0;
    //Sec
    pTime_Fmt=strstr(szzTime_Fmt,SECSTRING);
    if(pTime_Fmt)
    {
      strncpy(szSec,str+(pTime_Fmt-szzTime_Fmt),2);
      szSec[2]=0;
    }else{
           pTime_Fmt=strstr(szzTime_Fmt,SECSTRING2);
           if(!pTime_Fmt) return -1;
           szSec[0]=*(str+(pTime_Fmt-szzTime_Fmt));
           sprintf(szSec,"%d",(szSec[0]-48)*6);
         }
    
    sprintf(str,"%s%s%s",szHour,szMinu,szSec);
  }
  
//校验时间的合法性  
  char szTime[7];
  strncpy(szTime,str,6);
  szTime[6]=0;
  if(CheckTime(szTime)==false) return -1;

  return 0;
}

/*
int CFormatCheck::FYCalledIdxCHECK_CH(char *str,char *szSourceID,int iCheckLen)
{
  char TmpField[1024];
  if(SHQQRcdType == 0)
  {
    return 0;
  }

  if(SHQQRcdType=='P')
  {
    sprintf(TmpField,"17909%s",str);
    sprintf(str,"%s",TmpField);
    return 0;
  }
  if(SHQQRcdType=='m')
  {
    sprintf(TmpField,"96688%s",str);
    sprintf(str,"%s",TmpField);
  }
  
  return 0;
}

int CFormatCheck::FyRcdTypeCHECK_CH(char *str,char *szSourceID,int iCheckLen)
{
  char TmpField[1024];
  SHQQRcdType = str[0];
  return 0;

  if(str[0]=='P')
  {
    sprintf(TmpField,"17909%s",Field[SHQQCalledNoIdx]);
    sprintf(Field[SHQQCalledNoIdx],"%s",TmpField);
    return 0;
  }
  if(str[0]=='m')
  {
    sprintf(TmpField,"96688%s",Field[SHQQCalledNoIdx]);
    sprintf(Field[SHQQCalledNoIdx],"%s",TmpField);
  }
  
  return 0;
}

*/


int CFormatCheck::CheckField(char chCheckFmt,char *szField,char *szSourceID,int index,char *szFile_Fmt,int iCheckLen)
{
  int iErrType;


  switch(chCheckFmt)
  {
    case HCHECK_CH:
      if(FHCHECK_CH(szField,szSourceID,iCheckLen)) return -1;
      return 0;
    case CTIMECHECK_CH:
      iErrType = FCTIMECHECK_CH(szField,szSourceID,iCheckLen);
      if(iErrType ==-1) return -1;//错单
      if(iErrType ==-2) return -2;//超期话单
      if(iErrType ==-90) return -90;//时间异常话单
      return 0;
    case sTIMECHECK_CH:
      iErrType = FsTIMECHECK_CH(szField,szSourceID,iCheckLen);
      if(iErrType ==-1) return -1;//错单
      if(iErrType ==-2) return -2;//超期话单
      if(iErrType ==-90) return -90;//时间异常话单
      return 0;
    case cTIMECHECK_CH:
      iErrType = FcTIMECHECK_CH(szField,szSourceID,iCheckLen);
      if(iErrType ==-1) return -1;//错单
      if(iErrType ==-2) return -2;//超期话单
      if(iErrType ==-90) return -90;//时间异常话单
      return 0;
    case zTIMECHECK_CH:
      iErrType = FzTIMECHECK_CH(szField,szSourceID,iCheckLen);
      if(iErrType ==-1) return -1;//错单
      return 0;
    case ZDATECHECK_CH:
      iErrType = FZDATECHECK_CH(szField,szSourceID,iCheckLen);
      if(iErrType ==-1) return -1;//错单
      if(iErrType ==-2) return -2;//超期话单
      return 0;
    case DTIMECHECK_CH:
      if(FDTIMECHECK_CH(szField,szSourceID,iCheckLen)) return -1;
      return 0;
    case dTIMECHECK_CH:
      if(FdTIMECHECK_CH(szField,szSourceID,iCheckLen)) return -1;
      return 0;
    case NUMCHECK_CH:
      iErrType=FNUMCHECK_CH(szField,szSourceID,iCheckLen);
      if(iErrType ==-1) return -1;//错单
      if(iErrType ==-9) return -9;//超长超短话单
      return 0;
    case LENCHECK_CH:
      if(FLENCHECK_CH(szField,szSourceID,iCheckLen)) return -1;
      return 0;
    case TELNUMCHECK_CH:
      if(FTELNUMCHECK_CH(szField,szSourceID,iCheckLen)) return -1;
      return 0;
    case NULLCHECK_CH:
      if(FNULLCHECK_CH(szField,szSourceID,iCheckLen)) return -1;
      return 0;
    case CALLINGCHECK_CH:
      iErrType=FCALLINGCHECK_CH(szField,szSourceID,iCheckLen);
      if(iErrType ==-1) return -1;//错单
      if(iErrType ==-20) return -20;//错单加异常单
      return 0;
    case CALLEDCHECK_CH:
      iErrType=FCALLEDCHECK_CH(szField,szSourceID,iCheckLen);
      if(iErrType ==-1) return -1;//错单
      if(iErrType ==-3) return -3;//错单加异常单
      iCalledNo_Index=index;
      SHQQCalledNoIdx=index;
      return 0;
    case DURCHECK_CH:
      iErrType =FDURCHECK_CH(szField,szSourceID,iCheckLen);
      if(iErrType ==-1) return -1;//错单
      if(iErrType ==-9) return -9;//超长超短话单
      return 0;
    case MDURCHECK_CH:
      iErrType =FMDURCHECK_CH(szField,szSourceID,iCheckLen);
      if(iErrType ==-1) return -1;//错单
      if(iErrType ==-9) return -9;//超长超短话单
      return 0;
      
    case FEECHECK_CH:
      if(FFEECHECK_CH(szField,szSourceID,iCheckLen)) return -1;
      return 0;
    case FREECHECK_CH:
      if(FFREECHECK_CH(szField,szSourceID,iCheckLen)) return -1;
      iAnn_Index=index;
      return 0;
    case ROUTECHECK_CH:
      if(FROUTECHECK_CH(szField,index)) return -1;
      return 0;
    case INTCHECK_CH:
      if(FINTCHECK_CH(szField,szSourceID,iCheckLen)) return -1;
      return 0;
    case CALLINGCHECKa_CH:
      iErrType=FCALLINGCHECK_CH(szField,szSourceID,iCheckLen,chCheckFmt);
      if(iErrType ==-1) return -1;//错单
      if(iErrType ==-20) return -20;//错单加异常单
      return 0;
     case CALLEDCHECKb_CH:
      iErrType=FCALLEDCHECK_CH(szField,szSourceID,iCheckLen,chCheckFmt);
      if(iErrType ==-1) return -1;//错单
      if(iErrType ==-3) return -3;//错单加异常单
      iCalledNo_Index=index;
      SHQQCalledNoIdx=index;
      return 0;
    case ENUMCHECK_CH:
      iErrType=FENUMCHECK_CH(szField,szSourceID,index,szFile_Fmt);
      if(iErrType ==-1) return -1;//错单
      return 0;
      
    case YCallIdxCHECK_CH:
//      if(FYCalledIdxCHECK_CH(szField,szSourceID,iCheckLen,chCheckFmt)) return -1;
      SHQQCalledNoIdx=index;
      return 0;
     case yRcdTypeCHECK_CH:
//      if(FyRcdTypeCHECK_CH(szField,szSourceID,iCheckLen,chCheckFmt)) return -1;
      SHQQRcdType = szField[0];
      return 0;
//新增被叫校验方式J\j，30字节上的截掉
    case CALLED30CHECK_CH:
      iErrType=FCALLED30CHECK_CH(szField,szSourceID,iCheckLen);
      if(iErrType ==-1) return -1;//错单
      if(iErrType ==-3) return -3;//错单加异常单
      iCalledNo_Index=index;
      SHQQCalledNoIdx=index;
      return 0;
    case CALLED30CHECKb_CH:
      iErrType=FCALLED30CHECK_CH(szField,szSourceID,iCheckLen,chCheckFmt);
      if(iErrType ==-1) return -1;//错单
      if(iErrType ==-3) return -3;//错单加异常单
      iCalledNo_Index=index;
      SHQQCalledNoIdx=index;
      return 0;
    default:
    	return -1;
    
  }

  
}




/*
*	Function Name:	CheckDate
*	Description:	校验日期正确性
*	Input Param:
*		pchString -------> 输入字符串(格式yyyymmdd或者yymmdd)
*	Returns:
*		成功，返回true
*		失败，返回false
*	complete:	2002/03/13
*/
bool CFormatCheck::CheckDate(const char *pchString)
{
	int num = 0,iLen;
	char tmp[9];
	int iYear = 0,iMonth = 0,i;

	iLen = strlen(pchString);
	if ((iLen != 6) && (iLen != 8))
	{
		return false;
	}

	for (i=0; i<iLen; i++)
	{
		if ((pchString[i] < '0') || (pchString[i] > '9'))
		{
			return false;
		}
	}

	if (iLen == 6)
	{
		//校验年份
		strncpy(tmp, pchString, 2);
		tmp[2] = 0;
		num = atoi(tmp);
		iYear = num + 2000;
//		if ((num < 0) || (num > 99))
		if ((num <0))
		{
			return false;
		}
		//校验月份
		strncpy(tmp, pchString + 2, 2);
		tmp[2] = 0;
		num = atoi(tmp);
		iMonth = num;
		if ((num < 1) || (num > 12))
		{
			return false;
		}

		//校验天
		strncpy(tmp, pchString + 4, 2);
		tmp[2] = 0;
		num = atoi(tmp);
		if ((num < 1) || (num > 31))
		{
			return false;
		}
	}
	else if (iLen == 8)
	{
		strncpy(tmp, pchString, 4);
		tmp[4] = '\0';
		num = atoi(tmp);
		iYear = num;
		num = num - 2000;
//		if ((num <0) || (num > 99))
		if ((num <0))
		{
			return false;
		}
		//校验月份
		strncpy(tmp, pchString + 4, 4);
		tmp[2] = 0;
		num = atoi(tmp);
		iMonth = num;
		if ((num < 1) || (num > 12))
		{
			return false;
		}
		//校验天
		strncpy(tmp, pchString + 6, 2);
		tmp[2] = 0;
		num = atoi(tmp);
		if ((num < 1) || (num > 31))
		{
			return false;
		}
	}

	switch(iMonth)
	{
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			if ((num < 1) || (num > 31))
			{
				return false;
			}
			break;
		case 2:
			//判断闰年
			if (((iYear%4 == 0) && (iYear%100 != 0)) || (iYear%400 == 0))
			{
				if ((num < 1) || (num > 29))
				{
					return false;
				}
			}
			else
			{
				if ((num < 1) || (num > 28))
				{
					return false;
				}
			}
			break;
		default :
			if((num < 1) || (num > 30))
			{
				return false;
			}
	}

	return true;
}



/*
*	Function Name:	CheckTime
*	Description:	校验时间正确性
*	Input Param:
*		pchString -------> 输入字符串(格式hhmmss或者为hhmmsss最后一位是十分之一秒)
*	Returns:
*		成功，返回true
*		失败，返回false
*	Complete:	2002/03/13
*	Update:		2003/01/25
*/
bool CFormatCheck::CheckTime(const char *pchString)
{
	int num,i;
	char tmp[7];

	if ((strlen(pchString) != 6) && (strlen(pchString) != 7))
	{
		return false;
	}

	int iLen = strlen(pchString);
	for (i=0; i<iLen; i++)
	{
		if ((pchString[i] < '0') || (pchString[i] > '9'))
		{
			return false;
		}
	}

	//校验小时
	strncpy(tmp, pchString, 2);
	tmp[2] = 0;
	num = atoi(tmp);
	if ((num < 0) || (num > 23))
	{
		return false;
	}

	//校验分钟
	strncpy(tmp, pchString + 2, 2);
	tmp[2] = 0;
	num = atoi(tmp);
	if ((num < 0) || (num > 59))
	{
		return false;
	}

	//校验秒
	strncpy(tmp, pchString + 4, 2);
	tmp[2] = 0;
	num = atoi(tmp);
	if ((num < 0) || (num > 59))
	{
		return false;
	}
	//校验十分之一秒
	if(strlen(pchString) == 7)
	{
		strncpy(tmp, pchString + 6, 1);
		tmp[1] = 0;
		num = atoi(tmp);
		if((num < 0) || (num > 9))
		{
			return false;
		}
	}
	return true;
}


int CFormatCheck::Check_Rcd_Arr_Dur(char *nowertime, char *timeStr )
{

  char tmpstr1[9];
  char tmpstr2[9];
  memcpy(tmpstr1, timeStr,  8);  tmpstr1[8] =0;
  memcpy(tmpstr2, nowertime,8); tmpstr2[8] =0;

  if(strcmp(tmpstr1,tmpstr2))
  {
    long li=timeStr2Time(timeStr);
    long lj=timeStr2Time(nowertime);

    if((lj-li) > iRcd_Arr_Dur) return -1;
    if((li-lj) > iERcd_Arr_Dur) return -1;
    
    return 0;
  }
  
  int  i, j, h, m, s;
  char tmpstr[8];
  memcpy(tmpstr, timeStr + 8,  2);  tmpstr[2] =0;	 h = atoi(tmpstr);
  memcpy(tmpstr, timeStr + 10, 2);  tmpstr[2] =0;	 m = atoi(tmpstr);
  memcpy(tmpstr, timeStr + 12, 2);  tmpstr[2] =0;	 s = atoi(tmpstr);
  i = h*3600+m*60+s;
  memcpy(tmpstr, nowertime + 8,  2);  tmpstr[2] =0;	 h = atoi(tmpstr);
  memcpy(tmpstr, nowertime + 10, 2);  tmpstr[2] =0;	 m = atoi(tmpstr);
  memcpy(tmpstr, nowertime + 12, 2);  tmpstr[2] =0;	 s = atoi(tmpstr);
  j = h*3600+m*60+s;

  if((j-i) > iRcd_Arr_Dur) return -1;
  if((i-j) > iERcd_Arr_Dur) return -1;
  return 0;
}



/*remove the connector '-' in the calling_no or called_no*/
static void Remove_connector(char* str)
{
    int i, j;
    char tmp_no[50];
    int strLen;

        if (strstr(str, "-") != NULL)
        {
          strLen=strlen(str);
          j = 0;
          for (i = 0;i <strLen;i++)
          {
            if (str[i] != '-')
            {
              tmp_no[j] = str[i];
              j++;
            }
          }
          tmp_no[j]=0;
          sprintf(str,"%s",tmp_no);
        }
   return ;
}
