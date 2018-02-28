/******************************************************************
	Copyright (c) 2012, GUANGDONG YOSON TECHNOLOGY CO., LTD
	All rights reserved.

	Created:		2013/2/25
	Filename: 		FormatPlugin.cpp
	Description:	��������ϵͳ�ĸ�ʽ���ļ���
					��ʽ���ļ���д������

	History:
	<table>
		Revision	Author			Date		Description
		--------	------			----		-----------
		v1.0		hed		 2013/3/22	       ��ɳ������
		v2.0		hed		 2013/3/31		   ����ʽ�������ŵ����뻰����֮ǰִ�к�
		v3.0        hed		 2013/4/9		   ֧�ֶ���̸�ʽ���ļ�
		v4.0		hed		 2013/4/17		   ��ʽ���ɹ����Ҫ�����ݿ��еǼ���Ϣ 
	</table>
*******************************************************************/
/*
84����ǰ��¼���ļ��еļ�¼���
85���ļ����ϵ�ĳ���ַ��������ݻ�������FMT_TIME_INFNAME��ȡ��ԭ��Ϊ�ļ�����ʱ���ֶ�
87: �ļ����ϵ�ĳ���ַ��������ݻ�������FMT_FIELD_INFNAME��ȡ
88: ��������      ��MsgDirection�������������Ͷ�����cp����ϵͳ��Ч
90: �������뻰����  ��Record��--��ѹ��Ϊ����32�ֽڣ��������¼��ʽ�д����ֶ���ΪRecord���ֶ�ʱ��Ч��������source_env������ֶ���ϣ�
91: ���뻰�����ȣ�  ��Record_Len��
92���쳣������      (Abn_type)
94�����ӹڻ�����־  (Fake_Header_Flag)
99: ����Դ����      (source_id);
100:��������д      (localnet_abbr);
*/
//ʱ���ʽ֮��ĳ��Ȳ�һ�¡�

#include "FormatPlugin.h"
#define THROW_EXCEPTION(iErrorCode, strErrorMsg) {throw jsexcp::CException( iErrorCode, strErrorMsg, __FILE__, __LINE__); }
#define MAX_CHLD_PRC_NUM  10
typedef struct
{
	pid_t pid;
	int m_lPrc_ID;     //���ӽ���id add by hed 2013-4-9
	char  state;       // PRCST_STARTING PRCST_IDLE PRCST_BUSY PRCST_STOPPING PRCST_INACTV
}CHLD_PRC_INFO;
CHLD_PRC_INFO g_stChldPrcInfo[MAX_CHLD_PRC_NUM];
queue<int> task;

int  g_iChldPrcNum;
bool gbNormal=true;

struct stat fileInfo;

//PacketParser pps;			//??Ϊʲô�ŵ���Ա������Ҫ�Ұ�
//ResParser res;

SGW_RTInfo	rtinfo;
//extern vector<string> v_sql;

extern char Field[MAX_FIELD_COUNT][FMT_MAX_FIELD_LEN];

FormatPlugin::FormatPlugin()
{
	initflag=0;
	m_bSuspSig=false;
	file_id = 0;
	record_num = 0;
	//source_file_num = 0;
	petri_status = DB_STATUS_ONLINE;
	petri_status_tmp = DB_STATUS_ONLINE;

	memset(fileName,0,sizeof(fileName));
	memset(rate_cycle,0,sizeof(rate_cycle));
	memset(file_time,0,sizeof(file_time));
	//memset(timeout_path,0,sizeof(timeout_path));
	//memset(other_path,0,sizeof(other_path));
	memset(currTime,0,sizeof(currTime));
	memset(err_code,0,sizeof(err_code));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(outFileName,0,sizeof(outFileName));
	memset(sql,0,sizeof(sql));	
}

FormatPlugin::~FormatPlugin()
{
	//m_Shm = NULL; 
	//m_pArgv=NULL;
	//m_pTktBlkBase=NULL;
	//m_DBConn=NULL;
	pCur_TxtFmtDefine=NULL;
	pCur_Input2Output=NULL;
	//conn = NULL;
	
	if(pps)
	{
		delete pps;
	}
	if(res)
	{
		delete res;
	}

	if(mdrParam.m_enable) 
	{
		int ret = dr_ReleasePlatform();
		if(ret != 0)
		{
			char tmp[100] = {0};
			snprintf(tmp, sizeof(tmp), "�ͷ�����ƽ̨ʧ��,����ֵ=%d", ret);
			theJSLog.writeLog(LOG_CODE_DR_RELEASE_ERR,tmp);
		}
		mdrParam.m_enable = false;
	}
}

/********��ʽ������Դ�����*******************************************************/
void FormatPlugin::execute(PacketParser& ps,ResParser& retValue)
{
	char szLogStr[LOG_MSG_LEN+1];							//��־��Ϣ
  	CFmt_Change *inrcd;
    char szRecord[record_len];
    memset(szRecord,0,record_len);
    int iErrorType=0;
    int iTimeOutType=0;
    int iRet=0;
	STxtRecordFmtDefine *pCur_TxtRecFmt=NULL;
	STxtRecordFmtDefine *pLast_TxtRecFmt=NULL;
	
    m_formatcheck.iAnn_Index=0;
    m_formatcheck.iCalledNo_Index=0;
    m_formatcheck.iFake_Header_Flag=0;
    m_formatcheck.szPreCalledNo[0] = 0;
    m_formatcheck.szAftCalledNo[0] = 0;
    m_txtformat.iSign2_Begin=0;
    m_formatcheck.SHQQRcdType=0;
	m_formatcheck.SHQQCalledNoIdx=0;
//	cout<<"ps.getRecord():"<<ps.getRecord()<<endl;

	strcpy(szRecord,ps.getString());
    if(szRecord[strlen(szRecord)-1] == '\n')
    	szRecord[strlen(szRecord)-1] = 0;
    iTotalNum++;
//    cout<<"szRecord:"<<szRecord<<endl;

    sprintf(Field[MAX_FIELD_COUNT-10],"%d",strlen(szRecord));
    sprintf(Field[MAX_FIELD_COUNT-17],"%d",iTotalNum);
    sprintf(Field[MAX_FIELD_COUNT-5],"%d",ps.getOffset());

	//cout<<"��¼��ţ� "<<iTotalNum<<" ƫ������"<<ps.getOffset()<<"  ��¼���ȣ�"<<strlen(szRecord)<<endl;

    pCur_TxtRecFmt=m_txtformat.Get_CurTxtRecordFmt(szRecord);
    if(!pCur_TxtRecFmt)
    {
	  SetAbnOutCdr(eFmtOther,iErrorType,inrcd,ps,retValue);
//дotherformat
      return ;
    }
    //cout<<"iErrorType1��"<<iErrorType<<endl;

    pLast_TxtRecFmt=pCur_TxtRecFmt;
    inrcd=OTxt_Fmt.Get_Fmt(pCur_TxtRecFmt->szRecord_Name);

	pCur_Input2Output=m_txtformat.GetCurInput2Output(inrcd->Get_id(),outrcd.Get_id());
	if(pCur_Input2Output == NULL)
	{
		sprintf(szLogStr, "can't find input:%s ,output:%s in input2output!",inrcd->Get_id(),outrcd.Get_id());
		//throw CException(ERR_SELECT_NONE,szLogStr, __FILE__, __LINE__);
		THROW_EXCEPTION(ERR_SELECT_NONE,szLogStr);
	}

	outrcd.Clear_Field();

    try
    {
    //���Ƿ����ֶ����//���ǲ�У����ڳ���
      iErrorType=inrcd->Set_record(szRecord,pCur_TxtRecFmt->iRecord_Len);
    }
    catch(jsexcp::CException e)
    {
      iErrorType=-1;
    }
   // cout<<"iErrorType2��"<<iErrorType<<endl;

/*
	if(iErrorType) 
	{
		SetAbnOutCdr(eFmtErr,iErrorType,inrcd,ps,retValue);
		return ;
	}
*/
    Get_TxtCalledNo(inrcd);
//ȡ���Ż�����ѹ������ת���ı���ʽ���
    if(iRecord_Index > 0) 
    {
      CF_CHash oHash;
      if(mCur_SourceCfg->second.FieldIndex[0] == 0)
        oHash.getHashStr((unsigned char*)Field[MAX_FIELD_COUNT-11], szRecord, strlen(szRecord));
      else
      {
        char szTmpRecord[record_len+1];
        szTmpRecord[0]=0;
        int tmpFieldIndex=0;
        do
        {
          if(mCur_SourceCfg->second.FieldIndex[tmpFieldIndex]<80)
            sprintf(szTmpRecord,"%s%s;",szTmpRecord,inrcd->Get_Field(mCur_SourceCfg->second.FieldIndex[tmpFieldIndex]));
          else sprintf(szTmpRecord,"%s%s;",szTmpRecord,Field[mCur_SourceCfg->second.FieldIndex[tmpFieldIndex]-1]);
          tmpFieldIndex++;
        }while(mCur_SourceCfg->second.FieldIndex[tmpFieldIndex]!=0);
        oHash.getHashStr((unsigned char*)Field[MAX_FIELD_COUNT-11], szTmpRecord, strlen(szTmpRecord));
      }
      char szDulRtn[record_len+1];
      transfer( Field[MAX_FIELD_COUNT-11],16,szDulRtn);//ѹ����̶�16���ֽ�
      sprintf(Field[MAX_FIELD_COUNT-11],"%s",szDulRtn);
      Field[MAX_FIELD_COUNT-11][32]=0;
    }
  

    //***********************************************************
    // У���ֶ�
    //***********************************************************
  int iInFieldCount=pCur_Input2Output->iCount;
  int iCheckIndex=0;
  int iCheckReturn;

  int iIsC = 0;
  char  DTime[16];
  memset(DTime,0,16);

  for(int i=0;i<iInFieldCount;i++)
  {
      if(pCur_Input2Output->pInputIndex[i]<=0) continue;
      if(pCur_Input2Output->pInputIndex[i]>=inrcd->Get_fieldcount()) continue;
  	  char chCheckFmt=inrcd->Get_FieldFmt(pCur_Input2Output->pInputIndex[i]);
  	  if(chCheckFmt == NONCHECK_STATITC_CH)
  	  {
  	    if(strlen(inrcd->Get_Field(pCur_Input2Output->pInputIndex[i]))<16)
  	    {
  	      strcpy(DTime,inrcd->Get_Field(pCur_Input2Output->pInputIndex[i]));
  	      iIsC=1;
  	    }
  	  }
  	  else if(chCheckFmt!=NONCHECK_CH)
  	  { 
  	    iCheckReturn=m_formatcheck.CheckField(chCheckFmt,inrcd->Get_Field(pCur_Input2Output->pInputIndex[i]),m_szSourceID,
  	    	(pCur_Input2Output->pInputIndex[i]-1),inrcd->Get_id(),inrcd->Get_FieldCheckLen(pCur_Input2Output->pInputIndex[i]));
  	    if(iCheckReturn)
  	    {
  	    	if((iCheckReturn==(-2))&&(iTimeOutType==0))
  	      		iTimeOutType=1;
  	    	else if((iCheckReturn==(-90))&&(iTimeOutType==0))
  	           iTimeOutType=90;
  	    	else if((iCheckReturn==(-9))&&(iTimeOutType==0))
  	           iTimeOutType=9;
  	    	else iErrorType=pCur_Input2Output->pInputIndex[i];
	      //����

  	    }
  	    if(((chCheckFmt==CTIMECHECK_CH)||(chCheckFmt==cTIMECHECK_CH))&&(!iCheckReturn))
  	    {
  	      strcpy(DTime,inrcd->Get_Field(pCur_Input2Output->pInputIndex[i]));
  	      iIsC=1;
  	    }
  	    if(!iIsC)
  	    {
  	      if((chCheckFmt==ZDATECHECK_CH)&&(!iCheckReturn))
  	      {
  	        strncpy(DTime,inrcd->Get_Field(pCur_Input2Output->pInputIndex[i]),8);
  	        DTime[14]=0;
  	      }
  	      if((chCheckFmt==zTIMECHECK_CH)&&(!iCheckReturn))
  	      {
  	        strcpy(DTime+8,inrcd->Get_Field(pCur_Input2Output->pInputIndex[i]));
  	        DTime[14]=0;
  	      }
  	      if((chCheckFmt==sTIMECHECK_CH)&&(!iCheckReturn))
  	      {
  	        strcpy(DTime,inrcd->Get_Field(pCur_Input2Output->pInputIndex[i]));
  	        DTime[14]=0;
  	      }
  	    }

      }
  	}
/*
	if(iErrorType) SetAbnOutCdr(eFmtErr,iErrorType,inrcd,ps,retValue);
	else SetAbnOutCdr(eFmtTimeOut,iErrorType,inrcd,ps,retValue);
	return;
*/
    //cout<<"iErrorType3��"<<iErrorType<<endl;
    //***********************************************************
    // ��������Ӵ���Ի������д�����iResult��ֵ
    //***********************************************************
    if(pCur_TxtRecFmt->szExpression[0]!='@')
    {
      int i=0;
      for(i=0;i<iComp_Exp_Num;i++)
      {
        if(!strcmp(oComp_Exp[i].Get_FileFmt(),inrcd->Get_id()))
        	break;
      }
      if(i == iComp_Exp_Num)
      {
        Comp_Exp Tmp_Obj_Comp;
        oComp_Exp.push_back(Tmp_Obj_Comp);
        oComp_Exp[iComp_Exp_Num].AddVariable(*inrcd);
        oComp_Exp[iComp_Exp_Num].Set_FileFmt(inrcd->Get_id());
        iComp_Exp_Num++;
      }
      for(int j=0;j<pCur_TxtRecFmt->szExp_Num;j++)
      {
        if(oComp_Exp[i].Comp_Expression(pCur_TxtRecFmt->szExp[j]))
        {
          iErrorType=inrcd->Get_fieldcount()+1;
/*          
		  SetAbnOutCdr(eFmtErr,iErrorType,inrcd,ps,retValue);
          return ;
*/
        }
      }
    }
    //
    sprintf(Field[MAX_FIELD_COUNT-7],"%d",m_formatcheck.iFake_Header_Flag);
    sprintf(Field[MAX_FIELD_COUNT-9],"%d",iTimeOutType);
    //
    Route_Change(pCur_TxtFmtDefine,inrcd);

	m_txtformat.SHQQRcdType = m_formatcheck.SHQQRcdType;
	m_txtformat.SHQQCalledNoIdx = m_formatcheck.SHQQCalledNoIdx;

    iRet=m_txtformat.Set_OutRcd(*inrcd,outrcd,file_head,file_tail);
    if(iRet) iErrorType = iRet;
    //cout<<"iErrorType4��"<<iErrorType<<endl;
    if(iErrorType)
	{ 
		SetAbnOutCdr(eFmtErr,iErrorType,inrcd,ps,retValue);
      	return ;
    }
    else
    {
    	if(iTimeOutType)
    	{
    		SetAbnOutCdr(eFmtTimeOut,iErrorType,inrcd,ps,retValue);
    		return ;
    	}
    }
    //cout<<"iErrorType5��"<<iErrorType<<endl;
    
    if(iOraRcdIdx>0) outrcd.Set_Field(iOraRcdIdx,szRecord);

	if(m_chIs_Ann == 'Y')
	{
   		if(!m_formatcheck.Check_Ann_Flag(outrcd)) sprintf(Field[MAX_FIELD_COUNT-8],"%d",1);
   		else sprintf(Field[MAX_FIELD_COUNT-8],"%d",0);
	}


    if(mCur_SourceCfg->second.chIs_Bill_Statics_Route == 'Y')
    {
      char InRoute[16];
      char OutRoute[16];
      outrcd.Get_Field(INROUTE_NAME,InRoute);
      outrcd.Get_Field(OUTROUTE_NAME,OutRoute);
      if(!m_txtformat.CheckBillSFC(*inrcd))
        Bill_Route.AddItem(DTime,InRoute,OutRoute);
    }
    if(mCur_SourceCfg->second.chIs_Bill_Statics_fnTime == 'Y')
    {
      if(!m_txtformat.CheckBillSFC(*inrcd))
        Bill_fnTime.AddItem(Field[MAX_FIELD_COUNT-16]);
    }
    if(!m_txtformat.CheckBillSFC(*inrcd))
      Bill.AddItem(DTime);
    if(strcmp(szEarliestTime,DTime) > 0) sprintf(szEarliestTime,"%s",DTime);
    if(strcmp(szLatestTime,DTime) < 0) sprintf(szLatestTime,"%s",DTime);

//	retValue.setAnaResult(eNormal,"","");
//	cout<<"retValue.m_outRcd.Get_id():"<<retValue.m_outRcd.Get_id()<<"|outrcd.Get_id():"<<outrcd.Get_id()<<endl;
    //cout<<"��β��ඣ�outrcd = "<<outrcd.Get_record()<<endl;
	retValue.m_outRcd.Copy_Record(outrcd);
	return ;

}

//�ӽ����ڷ�����ϢʱҪ�������ݿ����
int FormatPlugin::InsTimeBetweenFile(char *SourceId,char *filename,char *EarlyTime,char *LastTime)
{
  //CBindSQL ds(*m_DBConn);
  char szSqlStr[400];
  memset(szSqlStr,0,sizeof(szSqlStr));
  sprintf(szSqlStr,"insert into D_BILLTIME_BETWEEN_FILE(Source_Id, FileName, EarlyTime, LastTime) values('%s', '%s', '%s', '%s')",SourceId, filename,EarlyTime,LastTime);
  //ds.Open(szSqlStr, NONSELECT_DML);
  //ds.Execute();
  //ds.Close();
  try
  {
	string sql = szSqlStr;
	Statement stmt = conn.createStatement();
	stmt.setSQLString(sql);
	stmt.execute(sql);
	stmt.close();
  }
  catch (SQLException e)
  {
	  theJSLog<<"InsTimeBetweenFileʱ�������ݿ�ʧ��:"<<e.what()<<endi;
	  throw jsexcp::CException(0, "InsTimeBetweenFileʱ�������ݿ�ʧ��", __FILE__, __LINE__);
  }
  
  
  return 0;
}

void FormatPlugin::Build_trans_table()
{
    int i, j;
    for (i = 0; i < 16; i++)
        for (j = 0; j < 16; j++)
        {
            if (i < 10 && j < 10)
            {
                trans_table[i*16 + j][0] = 0x30 + i;
                trans_tablex[i*16 + j][0] = 0x30 + i;
                trans_table[i*16 + j][1] = 0x30 + j;
                trans_tablex[i*16 + j][1] = 0x30 + j;
            }
            else
            {
                /*trans_table[i*16+j][0]  = 0x20;���ڵ���10����ֵ����Ϊ�ո�*/
                trans_table[i*16 + j][0] = 0x41 + i - 10; /*updated on 2001.3.23, ���ĺ�BCD��16���Ʊ�����ͬ*/
                if (i >= 10)
                    trans_tablex[i*16 + j][0] = 0x41 + i - 10;
                else
                {
                    trans_table[i*16 + j][0] = 0x30 + i;
                    trans_tablex[i*16 + j][0] = 0x30 + i;
                }
                /*trans_table[i*16+j][0]  = 0x20;���ڵ���10����ֵ����Ϊ�ո�*/
                trans_table[i*16 + j][1] = 0x41 + j - 10; /*updated on 2001.3.23, ���ĺ�BCD��16���Ʊ�����ͬ*/
                if (j >= 10)
                    trans_tablex[i*16 + j][1] = 0x41 + j - 10;
                else
                {
                    trans_table[i*16 + j][1] = 0x30 + j;
                    trans_tablex[i*16 + j][1] = 0x30 + j;
                }
            }
        } /*end for i,j*/
}

int FormatPlugin::transfer( char *block_buff, int len,char *szRtn,int flag )
{
    int i, start_pos = 0;
	if(flag)
	{
   	 	for (i = 0;i < len;i++)
   	 	{
        	szRtn[i*2] = trans_table[block_buff[start_pos] & 0x00ff][0]; /*translate the high 4 bits*/
        	szRtn[i*2 + 1] = trans_table[block_buff[start_pos] & 0x00ff][1]; /*low 4 bits*/
        	start_pos++;
  	  	}
	}
	else
	{
    	for (i = 0;i < len;i++)
    	{
        	szRtn[i*2] = trans_tablex[block_buff[start_pos] & 0x00ff][0]; /*translate the high 4 bits*/
        	szRtn[i*2 + 1] = trans_tablex[block_buff[start_pos] & 0x00ff][1]; /*low 4 bits*/
        	start_pos++;
    	}
	}
	szRtn[2*i] = 0;

    return 0;
}
void FormatPlugin::Route_Change(STxtFileFmtDefine *pTmp,CFmt_Change *inrcd)
{
  if(m_formatcheck.iCheckRouteIndex<0) return;
  for(int i=0;i<pTmp->iRoute_Num;i++)
  {
    if((!strcmp(inrcd->Get_Field(m_formatcheck.iCheckRouteIndex+1),pTmp->pCheckRoute[i].szRoute))
  	  &&(!strncmp(m_formatcheck.szTmpCalledNo,pTmp->pCheckRoute[i].szCalledNo_Header,strlen(pTmp->pCheckRoute[i].szCalledNo_Header))))
    {
      sprintf(inrcd->Get_Field(m_formatcheck.iCheckRouteIndex+1),"%s",pTmp->pCheckRoute[i].szOutputRoute);
      return ;
    }
  }

  return ;
}
void FormatPlugin::Get_TxtCalledNo(CFmt_Change *inrcd)
{

 if(m_txtformat.iSign2_Begin==0) return ;
 if((m_txtformat.iSign2_Value ) == 12)
   sprintf(m_formatcheck.szPreCalledNo,"40513A");
 else if((m_txtformat.iSign2_Value ) == 11)
 {
 	  sprintf(m_formatcheck.szPreCalledNo,"A");
 	  sprintf(m_formatcheck.szAftCalledNo,"%s",inrcd->Get_Field(m_txtformat.iSign2_Begin) + m_txtformat.iSign2_Len);
 }
 else if((m_txtformat.iSign2_Value==1402)||(m_txtformat.iSign2_Value==1404)||(m_txtformat.iSign2_Value==2402)||(m_txtformat.iSign2_Value==2404))
 {
   sprintf(m_formatcheck.szPreCalledNo,"%s",inrcd->Get_Field(m_txtformat.iSign2_Begin) + m_txtformat.iSign2_Len);
 }
  return ;
}

int FormatPlugin::GetStrFromFN(char *Res,char *Fn,char Spl,int Index,int Begin,int Len)
{
  int s1,s2;
//  char TmpFn[254];
//  TmpFn[0]=0;

  if(Index<1) return -1;
  
  if(Index == 1)
  {
    s1 = -1;
  }
  else 
  {
    s1 = strncspn(Fn,Spl,Index-1);
    if(s1<0) return -1;
  }
  s2 = strncspn(Fn,Spl,Index);
  if(s2<0) s2 = strlen(Fn);

  if(Begin >= (s2-s1)) return -2;

  strncpy(Res,Fn+s1+1,(s2-s1-1));
  Res[s2-s1-1] = 0;

  if(Begin<1) return -2;
  sprintf(Res,"%s",Res+Begin-1);
  if(Len<1) return -3;

  Res[Len]=0;
  return 0;
}

//�����̼�������Դ������Ϣʱ��������ݿ����
int FormatPlugin::getFromSrcEnv( char *Value, char *Name, char *SourceId,char *szService)
{
	//CBindSQL ds( *m_DBConn );
	//ds.Open("select VAR_VALUE from C_SOURCE_ENV where VARNAME=:a and source_id=:b and SERVICE=:c", SELECT_QUERY );
 	//ds<<Name<<SourceId<<szService;
 	//if( !(ds>>Value) ) 
 	//{   
 	//	ds.Close();
	//	ds.Open("select VAR_VALUE from C_PROCESS_ENV where VARNAME=:a and source_group=:b and SERVICE=:c", SELECT_QUERY );
 	//	ds<<Name<<m_szSrcGrpID<<szService;
	//	if( !(ds>>Value) )
	//	{
 	//		ds.Close();
	//		ds.Open("select VARVALUE from C_GLOBAL_ENV where VARNAME=:a", SELECT_QUERY );
 	//		ds<<Name;
	//		if( !(ds>>Value) )
	//		{
	//			ds.Close();
	//			return -1;
	//		}
	//	}
 	//}
	//ds.Close();
  try
  {
	memset(Value,0,sizeof(Value));		//2013-07-18�����ʼ�� �����ж�����

	Statement stmt  = conn.createStatement();	
	string sql = "select VAR_VALUE from C_SOURCE_ENV where VARNAME=:1 and source_id=:2 and SERVICE=:3";
	stmt.setSQLString(sql);
	stmt<<Name<<SourceId<<szService;
	stmt.execute();
	
	if(!(stmt>>Value))
	{		
			sql = "select VARVALUE from C_GLOBAL_ENV where VARNAME=:1";
			stmt.setSQLString(sql);
			stmt.execute();
			stmt<<Name;
			if(!(stmt>>Value))
			{
				stmt.close();
				return -1;
			}
	}
  }
  catch (SQLException e)
  {
		theJSLog<<"getFromSrcEnv�׳��쳣: "<<e.what()<<endi;
		throw jsexcp::CException(0, "getFromSrcEnv�׳��쳣��", __FILE__, __LINE__);		
  }
	
	
	delSpace(Value,0);	
	return 0;
}/*end of getFromSrcEnv*/

/**
 *��������Դ���C_SOURCE_GROUP_DEFINE ���ļ�����FILETYPE_ID��
 *����FILETYPE_ID�ӱ�c_filetype_define�ҳ���¼����record_type����¼����record_len��
 *����FILETYPE_ID��c_txtfile_fmt�ҳ���¼��ʽ��ȫ����Ϣ
 *�ڸ���FILETYPE_ID�ӱ�C_SOURCE_GROUP_CONFIG������ԴID��Ŀ
 *��������ԴID�ӱ�C_FILE_RECEIVE_ENV�ҵ����˹���
 *����Դ��Ϣȫ���ŵ���Map�б���m_SourceCfg
 *CFmt_Change�����outrcdҲ�洢�˹�������ļ���¼���͸�ʽ�ֶε���Ϣ
 */
int FormatPlugin::LoadSourceCfg()
{
	//CBindSQL ds(*m_DBConn);
	char szSqlStr[400];
	int iSourceCount=0;

//�����߱�TP_BILLING_LINE��ͨ������Դ��SOURCE_GROUPȥ��C_SOURCE_GROUP_DEFINE���ҵ��ļ�����FILETYPE_ID
//��Ϊ��ʽ��������� add by hed  2013-03-11
	//int flow_id = getFlowID();
	//ds.Open("select b.filetype_id from TP_BILLING_LINE  a,C_SOURCE_GROUP_DEFINE b where a.billing_line_id = :1 and a.source_group = b.source_group ")
	//ds<<flow_id;
	//ds.close();

try
{
	
//sourceGrp
	Statement stmt = conn.createStatement();
	string sql = "select FILETYPE_ID from C_SOURCE_GROUP_DEFINE where SOURCE_GROUP=:1";
    //ds.Open("select FILETYPE_ID from C_SOURCE_GROUP_DEFINE where SOURCE_GROUP=:1", SELECT_QUERY);
    //ds<<m_szSrcGrpID;
    //ds>>m_szOutTypeId;
    //ds.Close();
	stmt.setSQLString(sql);
	stmt<<m_szSrcGrpID;
	if(stmt.execute())
	 {
		stmt>>m_szOutTypeId;
	 }
	
	strcpy(mConfParam.szOutputFiletypeId,m_szOutTypeId);
	theJSLog<<"szOutputFiletypeId="<<m_szOutTypeId<<endi;
   	//expTrace(szDebugFlag, __FILE__, __LINE__,"%s;%s",m_szSrcGrpID,m_szOutTypeId);

	//outrcd.Init(m_szOutTypeId,ds); 
	outrcd.Init(m_szOutTypeId); 

 	iRecord_Index = outrcd.Get_FieldIndex(RECORD_NAME);
  	iOraRcdIdx = outrcd.Get_FieldIndex(ORARECORD_NAME);
//source
    //ds.Open("select count(*) from C_SOURCE_GROUP_CONFIG where SOURCE_GROUP=:1", SELECT_QUERY);
    //ds<<m_szSrcGrpID;
    //ds>>iSourceCount;
    //ds.Close();
	sql = "select count(*) from C_SOURCE_GROUP_CONFIG where SOURCE_GROUP=:1";
    stmt.setSQLString(sql);
	stmt<<m_szSrcGrpID;
	if(stmt.execute())
	{
		stmt>>iSourceCount;
	}

	theJSLog<<"iSourceCount="<<iSourceCount<<endi;
    //expTrace(szDebugFlag, __FILE__, __LINE__, "iSourceCount=%d;", iSourceCount);    
	
    //ds.Open("select a.source_id,b.file_fmt,b.source_path,b.TOLLCODE from C_SOURCE_GROUP_CONFIG a,I_SOURCE_DEFINE b where SOURCE_GROUP=:1 and a.source_id=b.source_id", SELECT_QUERY );
    //ds<<m_szSrcGrpID;
	sql = "select a.source_id,b.file_fmt,b.source_path,b.TOLLCODE,b.serv_cat_id from C_SOURCE_GROUP_CONFIG a,I_SOURCE_DEFINE b where SOURCE_GROUP=:1 and a.source_id=b.source_id";
    stmt.setSQLString(sql);
	stmt<<m_szSrcGrpID;
	if(stmt.execute())
	{
		//stmt>>iSourceCount;
		//}
    for (int i=0; i<iSourceCount; i++)
    {
	  SOURCECFG SourceCfg;
	  string strSourceId;
      char TmpVarValue[101];
      //ds>>SourceCfg.szSourceId>>SourceCfg.szInFileFmt>>SourceCfg.szSourcePath>>SourceCfg.szTollCode;
	  stmt>>SourceCfg.szSourceId>>SourceCfg.szInFileFmt>>SourceCfg.szSourcePath>>SourceCfg.szTollCode>>SourceCfg.serverCatID;
      
      strSourceId=SourceCfg.szSourceId;
      
	  if(getSourceFilter(SourceCfg.szSourceId,SourceCfg.filterRule,SourceCfg.file_begin,SourceCfg.file_length))
	  {
			return -1;
			//exit(-1);
	  }

	  completeDir(SourceCfg.szSourcePath);
	  
	  theJSLog<<"szSourceId="<<SourceCfg.szSourceId<<" szSourcePath="<<SourceCfg.szSourcePath<<" serverCatID="<<SourceCfg.serverCatID<<"  szInFileFmt="<<SourceCfg.szInFileFmt
			  <<" filterRule="<<SourceCfg.filterRule<<"  filetime_begin="<<SourceCfg.file_length<<"  filetime_length="<<SourceCfg.file_length<<endi;

	  if(getFileNameFmt(strSourceId) == -1)   return -1;      //��������Դ��ȡ�ļ�����ʽ

	  if(getRecordHTFilter(SourceCfg.szInFileFmt) == -1)  return -1;     //�����ļ����ͻ�ȡ�ļ���¼ͷβ��ʽ���������Դ���ܶ���ͬһ���ļ���ʽ

      if(getFromSrcEnv( TmpVarValue, "DUL_KEYWORDS",SourceCfg.szSourceId,m_szService))
      {
        SourceCfg.FieldIndex[0]=0;
      }
      else
      {

        char *ss0,*ss1;
        int j = 0; ss0 = TmpVarValue;
		ss1 = strtok(ss0, ";");
		while(ss1 != NULL)
		{
			SourceCfg.FieldIndex[j]=atoi(ss1);
			j++;
			if(j>19)
			{
            	expTrace(szDebugFlag, __FILE__, __LINE__,"DUL_KEYWORDS field define in source_env is more than 19");
            	//�˳�
		  	}
		  ss1 = strtok(NULL, ";");
		}
      }

//FMT_TIME_FMT/FMT_ZDATE_FMT/FMT_zTIME_FMT
      if(getFromSrcEnv( TmpVarValue, "FMT_TIME_FMT",SourceCfg.szSourceId,m_szService))
      {
        strcpy(SourceCfg.Fmt_Time_fmt,"YYYYDDMMhhmmss");
      }
      else
      {
        strcpy(SourceCfg.Fmt_Time_fmt,TmpVarValue);
      }
      if(getFromSrcEnv( TmpVarValue, "FMT_ZDATE_FMT",SourceCfg.szSourceId,m_szService))
      {
        strcpy(SourceCfg.szZDate_Fmt,"YYYYDDMM");
      }
      else
      {
        strcpy(SourceCfg.szZDate_Fmt,TmpVarValue);
      }
      if(getFromSrcEnv( TmpVarValue, "FMT_zTIME_FMT",SourceCfg.szSourceId,m_szService))
      {
        strcpy(SourceCfg.szzTime_Fmt,"hhmmss");
      }
      else
      {
        strcpy(SourceCfg.szzTime_Fmt,TmpVarValue);
      }

	  if(getFromSrcEnv( TmpVarValue, "MSGDIRECTION_BEGIN",SourceCfg.szSourceId,m_szService))
      {
        SourceCfg.iMsgDirBegin=-1;
      }
      else
      {
        SourceCfg.iMsgDirBegin=atoi(TmpVarValue);
      }

//���ļ�����ȡ�ֶ�
	  if(getFromSrcEnv( TmpVarValue, "FMT_FIELD_INFNAME",SourceCfg.szSourceId,m_szService))
	  {
    	  SourceCfg.FnSep= '.';
    	  SourceCfg.FnIndex = -1;
     	  SourceCfg.FnBegin = -1;
     	  SourceCfg.FnLen = -1;
 	  }
	  else
	  {

	    char TmpFn[10];
	    int Fns1,Fns2;
	    Fns1 = strncspn(TmpVarValue,';',1);
	    if(Fns1 <= 0) SourceCfg.FnSep= '.';
	    else SourceCfg.FnSep= TmpVarValue[0];
	//�ҵڶ����ָ���
	    Fns2 = strncspn(TmpVarValue,';',2);
	    if(Fns2<0)
	    {
	      if(Fns1<0) SourceCfg.FnIndex = -1;
	      else
	      {
	        SourceCfg.FnIndex = atoi(TmpVarValue+Fns1+1);
	      }
	      SourceCfg.FnBegin = -1;
	      SourceCfg.FnLen = -1;
	    }
	    else
	    {
	      strncpy(TmpFn,TmpVarValue+Fns1+1,Fns2-Fns1-1);
	      TmpFn[Fns2-Fns1-1] = 0;
	      SourceCfg.FnIndex = atoi(TmpFn);
	//�ҵ������ָ���
	      Fns1 = strncspn(TmpVarValue,';',3);
	      if(Fns1 < 0)
	      {
	        SourceCfg.FnBegin = atoi(TmpVarValue+Fns2+1);
	        SourceCfg.FnLen = -1;
	      }
	      else
	      {
	        strncpy(TmpFn,TmpVarValue+Fns2+1,Fns1-Fns2-1);
	        TmpFn[Fns1-Fns2-1] = 0;
	        SourceCfg.FnBegin = atoi(TmpFn);
	        SourceCfg.FnLen = atoi(TmpVarValue+Fns1+1);
	      }
	    }
	  }

//20071026,ȡ�ļ����ϵ�ʱ��,��Ϊ������ͳ����

//���ļ�����ȡ�ֶ�
	  if(getFromSrcEnv( TmpVarValue, "FMT_TIME_INFNAME",SourceCfg.szSourceId,m_szService))
	  {
	      SourceCfg.FntSep= '.';
	      SourceCfg.FntIndex = -1;
	      SourceCfg.FntBegin = -1;
	      SourceCfg.FntLen = -1;
	  }
	  else
	  {
	    char TmpFn[10];
	    int Fns1,Fns2;
	    Fns1 = strncspn(TmpVarValue,';',1);
	    if(Fns1 <= 0) SourceCfg.FntSep= '.';
	    else SourceCfg.FntSep= TmpVarValue[0];
	//�ҵڶ����ָ���
	    Fns2 = strncspn(TmpVarValue,';',2);
	    if(Fns2<0)
	    {
	      if(Fns1<0) SourceCfg.FntIndex = -1;
	      else
	      {
	        SourceCfg.FntIndex = atoi(TmpVarValue+Fns1+1);
	      }
	      SourceCfg.FntBegin = -1;
	      SourceCfg.FntLen = -1;
	    }
	    else
	    {
	      strncpy(TmpFn,TmpVarValue+Fns1+1,Fns2-Fns1-1);
	      TmpFn[Fns2-Fns1-1] = 0;
	      SourceCfg.FntIndex = atoi(TmpFn);
	//�ҵ������ָ���
	      Fns1 = strncspn(TmpVarValue,';',3);
	      if(Fns1 < 0)
	      {
	        SourceCfg.FntBegin = atoi(TmpVarValue+Fns2+1);
	        SourceCfg.FntLen = -1;
	      }
	      else
	      {
	        strncpy(TmpFn,TmpVarValue+Fns2+1,Fns1-Fns2-1);
	        TmpFn[Fns1-Fns2-1] = 0;
	        SourceCfg.FntBegin = atoi(TmpFn);
	        SourceCfg.FntLen = atoi(TmpVarValue+Fns1+1);
	      }
	    }
	  }

//bill_stat
	  if(getFromSrcEnv( TmpVarValue, "IS_BILL_STATICS",SourceCfg.szSourceId,m_szService))
	  {
		SourceCfg.chIs_Bill_Statics='N';
	  }
	  SourceCfg.chIs_Bill_Statics=TmpVarValue[0];

	  if(getFromSrcEnv( TmpVarValue, "IS_BILL_STATICS_ROUTE",SourceCfg.szSourceId,m_szService))
	  {
	    expTrace(szDebugFlag, __FILE__, __LINE__,
	      "set IS_BILL_STATICS_ROUTE = 'N' !");
	  	SourceCfg.chIs_Bill_Statics_Route = 'N';
	  }
	  else SourceCfg.chIs_Bill_Statics_Route=TmpVarValue[0];

	  if(getFromSrcEnv( TmpVarValue, "IS_BILL_STATICS_FNTIME",SourceCfg.szSourceId,m_szService))
	  {
	    expTrace(szDebugFlag, __FILE__, __LINE__,
	      "set IS_BILL_STATICS_FNTIME = 'N' !");
	  	SourceCfg.chIs_Bill_Statics_fnTime = 'N';
	  }
	  else SourceCfg.chIs_Bill_Statics_fnTime=TmpVarValue[0];

	  if(getFromSrcEnv( TmpVarValue, "IS_STAT_TIMEBETWEENFILE",SourceCfg.szSourceId,m_szService))
	  {
	    expTrace(szDebugFlag, __FILE__, __LINE__,
	      "set IS_STAT_TIMEBETWEENFILE = 'N' !");
	  	SourceCfg.chIs_TimeFile = 'N';
	  }
	  else SourceCfg.chIs_TimeFile=TmpVarValue[0];


//����
	  if(getFromSrcEnv( TmpVarValue, "REDO_PROPERTY_BEGIN",SourceCfg.szSourceId,m_szService))
	  	Redo_Begin = 0;
	  else Redo_Begin = atoi(TmpVarValue);

	  if(getFromSrcEnv( Redo_What, "REDO_PROPERTY_BEGIN",SourceCfg.szSourceId,m_szService))
	  {
	  	Redo_What[0] = '#';
	  	Redo_What[1] = 0;
	  }

	  if(getFromSrcEnv( TmpVarValue, "RCD_ARR_DUR",SourceCfg.szSourceId,m_szService))
	  {
	  	expTrace(szDebugFlag, __FILE__, __LINE__,
	      "set RCD_ARR_DUR =-1 !");
	  	SourceCfg.iRcd_Arr_Dur = -1;
	  }
	  else SourceCfg.iRcd_Arr_Dur = atoi(TmpVarValue);
	  if(getFromSrcEnv( TmpVarValue, "EAR_RCD_ARR_DUR",SourceCfg.szSourceId,m_szService))
	  {
	    expTrace(szDebugFlag, __FILE__, __LINE__,
	      "set EAR_RCD_ARR_DUR =-1 !");
	  	SourceCfg.iERcd_Arr_Dur = -1;
	  }
	  else SourceCfg.iERcd_Arr_Dur = atoi(TmpVarValue);

	  if(getFromSrcEnv( TmpVarValue, "FMT_ANN_DIR",SourceCfg.szSourceId,m_szService))
	      sprintf(TmpVarValue,"N");

	  if(strlen(TmpVarValue) < 2) m_chIs_Ann='N';
	  else m_chIs_Ann='Y';
	  if(TmpVarValue[strlen(TmpVarValue)-1] != '/')
	    strcat(TmpVarValue, "/");
	  sprintf(szAnn_Dir,"%s",TmpVarValue);

      m_SourceCfg[strSourceId]=SourceCfg;
    }
    //ds.Close();
    } //if(excute())
  }
   catch (SQLException e)
	{
		cout<<"LoadSourceCfg���ݿ����"<<e.what()<<endl;
		throw jsexcp::CException(0, "LoadSourceCfg�������ݿ����", __FILE__, __LINE__);
	}
}

/******��������Դ��ȡ���˹��� 0û�в鵽����1�鵽������ 2013-08-01���ӻ�ȡ�ļ�����ʱ�����ʼλ��,�ͳ���*********************/
int FormatPlugin::getSourceFilter(char* source,char* filter,int &index,int &length)
{	
	//CBindSQL ds( *m_DBConn );
	string sql ;
	try
	{	
		string file_time;

		Statement stmt = conn.createStatement();
		sql = "select file_filter,file_time_index_len from C_FILE_RECEIVE_ENV where source_id = :1 ";		
		stmt.setSQLString(sql);
		stmt << source;
		stmt.execute();
		if(!(stmt>>filter>>file_time))
		{
				stmt.close();
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"����Դ[%s]û�����ù��˹�������ļ���ʱ���ȡ����",source);
				theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
				return -1;
		}
		
		char tmp[5];
		memset(tmp,0,sizeof(tmp));
		strcpy(tmp,file_time.c_str());

		vector<string> fileTime;		
		splitString(tmp,",",fileTime,false);
		if(fileTime.size() != 2)
		{
			stmt.close();
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"����Դ[%s]�ļ���ʱ���ȡ�������ù������:%s  [��3,8]",source,file_time);
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
			return -1;
		}
		
		index = atoi(fileTime[0].c_str());
		length = atoi(fileTime[1].c_str());
		
		//cout<<"index = "<<index<<"  length = "<<length<<" file_time = "<<file_time<<endl;
		if(index < 1 || length == 0)
		{
			stmt.close();
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"����Դ[%s]�ļ���ʱ���ȡ�������ù������:%s  [��3,8]",source,file_time);
			theJSLog.writeLog(LOG_CODE_ENV_MISSING,erro_msg);
			return -1;
		}

		index--;

		stmt.close();

		//ds.Open("select file_filter from C_FILE_RECEIVE_ENV where source_id = :1");
		//ds<<source;
		//if(!(ds>>filter))
		//{
		//	ds.Close();
		//	return 0;
		//}
		//ds.Close();
	}
	catch(SQLException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"getSourceFilter ���ݿ��ѯ�쳣: %s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);
		return -1 ;
	}
	catch(jsexcp::CException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"getSourceFilter �ֶ�ת������%s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);
		return -1;
	}
	
	return 0;
}

//��ȡ�ļ����ĸ�ʽ����������Դ����ȡ  0��ʾ�������ø�ʽ
int FormatPlugin::getFileNameFmt(string source)
{
	vector<FileNameFmt> vfmt;
	int count = 0;
	string sql;
	try
	{	
		Statement stmt = conn.createStatement();
	    sql = "select check_no,check_index,check_len,check_type,check_value,err_code from C_FILENAME_CHECK where source_id = :1 order by check_no ";		
		stmt.setSQLString(sql);
		stmt << source;
		if(!stmt.execute())
		{
			stmt.close();
			return 0;	
		}
		
		FileNameFmt fmt;
		while(stmt>>fmt.number>>fmt.index>>fmt.len>>fmt.check_type>>fmt.check_value>>fmt.err_code)
		{
			fmt.index--;	//2013-09-04 ��1��ʼ
			vfmt.push_back(fmt);
			count++;
		}
		

		mapFileNameFmt.insert(map< string,vector<FileNameFmt> >::value_type(source,vfmt));

		stmt.close();

	}catch(SQLException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"getFileNameFmt ���ݿ��ѯ�쳣: %s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);

		return -1 ;
	}
	
	return 0; 
}

//��ȡ�ļ���¼��ͷβ��ʽ�������ļ���������ȡ
int FormatPlugin::getRecordHTFilter(char* fileType)
{
	vector<RecordHTFmt> vfmta,vfmtb;
	int count = 0;
	string sql;
	try
	{	Statement stmt = conn.createStatement();
		sql = "select col_no,col_name,col_index,col_len,col_seperator,check_type,check_value,ht_flag,spec_flag,err_code from C_FILE_HEADEND_CHECK where filetype_id = :1 order by ht_flag,col_no ";		
		stmt.setSQLString(sql);
		stmt << fileType;
		if(!stmt.execute())
		{
			stmt.close();
			return 0;	
		}
		
		RecordHTFmt fmt;
		while(stmt>>fmt.number>>fmt.name>>fmt.index>>fmt.len>>fmt.seperator>>fmt.check_type>>fmt.check_value>>fmt.ht_flag>>fmt.spec_flag>>fmt.err_code)
		{
			fmt.index--;	//2013-09-04 ��1��ʼ

			if(fmt.ht_flag == 'H')
			{
				vfmta.push_back(fmt);
			}

			else if(fmt.ht_flag == 'T')
			{
				vfmtb.push_back(fmt);
			}
			
			else 
			{
					return 0;
			}

			count++;
		}
		
		//2013-09-04 ���Ǹ����ļ������Ƿ���Ҫ�ж��Ѿ���map������,��Ϊ�п��ܶ������Դ��Ӧͬһ���ļ�����?

		//ͷβ�ֿ�����	
		mapFileRecordHeadFmt.insert(map< string,vector<RecordHTFmt> >::value_type(fileType,vfmta));		
		mapFileRecordTailFmt.insert(map< string,vector<RecordHTFmt> >::value_type(fileType,vfmtb));			

		//mapFileRecordHTFmt.insert(map< string,vector<RecordHTFmt> >::value_type(fileType,vfmt));

		stmt.close();

	}catch(SQLException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"getRecordHTFilter ���ݿ��ѯ�쳣: %s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);

		return -1 ;
	}

   return 0; 
}


int FormatPlugin::SetAbnOutCdr(pluginAnaResult anaType,int iErrorType,CFmt_Change *inrcd,PacketParser& ps,ResParser& retValue)
{
	char szErrorCode1[32];
	char szErrorCode2[32];
	
	sprintf(szErrorCode2,"%s_%d",inrcd->Get_id(),iErrorType);
	if(iErrorType<=0)
		sprintf(szErrorCode1,"%s_%d",outrcd.Get_id(),iErrorType);
	else
	{
		int i =0;
		while(i<pCur_Input2Output->iCount)
		{
			if(pCur_Input2Output->pInputIndex[i] == iErrorType)
			{
				sprintf(szErrorCode1,"%s_%d",outrcd.Get_id(),pCur_Input2Output->pOutputIndex[i]);
				break;
			}
			i++;
		}
	}
	retValue.setAnaResult(anaType,szErrorCode1,szErrorCode2);
//	retValue.setAnaResult(anaType,iErrorType,szErrorCode2);
//	cout<<"ps.m_inRcd.Get_id():"<<retValue.m_outRcd.Get_id()<<"|outrcd.Get_id():"<<outrcd.Get_id()<<endl;
    ps.m_inRcd.Copy_Record(outrcd);	
    /*
	switch(anaType)
	{
		case 1:
			retValue.setAnaResult(anaType,szErrorCode1,szErrorCode2);
			break;
		case 2:
			retValue.setAnaResult(anaType,szErrorCode1,szErrorCode2);
		    ps.m_inRcd.Copy_Record(outrcd);
			break;
		case 3:
			retValue.setAnaResult(anaType,szErrorCode1,szErrorCode2);
		    ps.m_inRcd.Copy_Record(outrcd);
		    break;
		default:
			break;
	}
	*/
	return 0;
}

void FormatPlugin::init(char *szSourceGroupID,char *szService,int index)
{
	int ret = 0;
	szDebugFlag[0]='N';
	m_chBillSta_Filter_Cond='Y';
	szDebugFlag[1]=0;
	//m_DBConn=&DBConn;
//	m_Logger=m_log;
	strcpy(m_szSrcGrpID,szSourceGroupID);
	strcpy(m_szService,szService);
	iProcIndex = index;
	Build_trans_table();

	ret = LoadSourceCfg();
	if(ret == -1)	exit(-1);

 //�õ��ӽ����г�ʼ�����������ݿ⽻�� ���޸�
	//if(m_txtformat.LoadFmtDataToMem(m_DBConn,szDebugFlag,m_szOutTypeId,m_chBillSta_Filter_Cond,m_szSrcGrpID))
	if(m_txtformat.LoadFmtDataToMem(conn,szDebugFlag,m_szOutTypeId,m_chBillSta_Filter_Cond,m_szSrcGrpID))
		//throw jsexcp::CException(ERR_SELECT_NONE,"Load Fmt Data To memory err!", __FILE__, __LINE__);
			THROW_EXCEPTION(ERR_SELECT_NONE,"Load Fmt Data To memory err!");
	//if(m_formatcheck.LoadCheckDataToMem(m_DBConn,m_szSrcGrpID,szDebugFlag,m_chIs_Ann))
	if(m_formatcheck.LoadCheckDataToMem(conn,m_szSrcGrpID,szDebugFlag,m_chIs_Ann))
		//throw jsexcp::CException(ERR_SELECT_NONE,"Load Check Data To memory err!", __FILE__, __LINE__);
			THROW_EXCEPTION(ERR_SELECT_NONE,"Load Check Data To memory err!");

	initflag=1;
}

void FormatPlugin::message(MessageParser&  pMessage)
{
	char szLogStr[400];
	switch(pMessage.getMessageType())
	{
		case MESSAGE_NEW_BATCH:
			szLastAnn_Dir[0]=0;
			szLastAnn_Abbr[0]=0;
			break;
		case MESSAGE_END_BATCH_END_FILES:
			if(m_formatcheck.CommitAnnFile())
            {
				m_formatcheck.RollbackAnnFile();
	            expTrace(szDebugFlag,__FILE__, __LINE__, 
	              	  "rename Ann err!");
				//throw jsexcp::CException(ERR_ANNFILE,"COMMIT ANN err!", __FILE__, __LINE__);
				THROW_EXCEPTION(ERR_ANNFILE,"COMMIT ANN err!");
            }			
			break;
		case MESSAGE_END_BATCH_END_DATA:
			break;
		case MESSAGE_BREAK_BATCH:
			m_formatcheck.RollbackAnnFile();
			break;
		case MESSAGE_PROGRAM_QUIT:
			break;
		case MESSAGE_NEW_FILE:
			//iTotalNum = 0;		//2013-09-08�����ļ�ͷУ����ǰ��,���Գ�ʼ������У�鴦
			iRightNum = 0;
			iLackNum = 0;
			iErrorNum = 0;
			iPickNum = 0;
			iOtherNum = 0;
			strcpy(m_szSourceID,pMessage.getSourceId());
		    sprintf(Field[MAX_FIELD_COUNT-2],"%s",m_szSourceID);
			strcpy(m_szFileName,pMessage.getFileName());
			mCur_SourceCfg=m_SourceCfg.find((string)m_szSourceID);
			if(mCur_SourceCfg == m_SourceCfg.end())
			{
		      	sprintf(szLogStr, "can't find source_id:%s in source_group:%s!",m_szSourceID,m_szSrcGrpID);
				//throw jsexcp::CException(ERR_SELECT_NONE,szLogStr, __FILE__, __LINE__);
				THROW_EXCEPTION(ERR_SELECT_NONE,szLogStr);
			}
			DEBUG_LOG<<"FormatPlugin-m_szSourceID:"<<m_szSourceID<<endd;
			iErrorBack_Flag=0;
	        strcpy((mCur_SourceCfg->second).szFile_Fmt,(mCur_SourceCfg->second).szInFileFmt);
	        DEBUG_LOG<<"FormatPlugin-szInFileFmt:"<<(mCur_SourceCfg->second).szInFileFmt<<endd;
			DEBUG_LOG<<"FormatPlugin-szFile_Fmt:"<<(mCur_SourceCfg->second).szFile_Fmt<<endd;

	        if((Redo_Begin>0)&&(Redo_What[0] != '#'))
	        if(!strncmp( m_szFileName+Redo_Begin-1,Redo_What,strlen(Redo_What)))
	  	  	{
	  	  	  iErrorBack_Flag=1;
	  	  	  DEBUG_LOG<<"FormatPlugin-Redo_Begin:"<<Redo_Begin<<endd;
	  	  	  DEBUG_LOG<<"FormatPlugin-Redo_What:"<<Redo_What<<endd;
	  	  	  DEBUG_LOG<<"FormatPlugin-m_szOutTypeId:"<<m_szOutTypeId<<endd;
	  	  	  strcpy((mCur_SourceCfg->second).szFile_Fmt,m_szOutTypeId);
	  	  	}
		//��MsgDirection
			if((mCur_SourceCfg->second).iMsgDirBegin > 0)
		  	{
			    char *s1,*s2;
			    s1 = m_szFileName+(mCur_SourceCfg->second).iMsgDirBegin;
			    if((s2 = strchr(s1,'.')) == NULL)
			    {
			      sprintf(szLogStr,"get MsgDirection Err in filename:%s;offset:%d.",m_szFileName,(mCur_SourceCfg->second).iMsgDirBegin);
			      //throw jsexcp::CException(ERR_GET_MsgDirection,szLogStr,__FILE__,__LINE__);
				  THROW_EXCEPTION(ERR_GET_MsgDirection,szLogStr);
			    }
			    *s2 = 0;
			    sprintf(Field[MAX_FIELD_COUNT-13],"%s",s1);
			    *s2 = '.';
			}
			//�Ӵ��ļ�����ȡ���ֶ�
			if((mCur_SourceCfg->second).FnIndex > 0)
			{
			    GetStrFromFN(Field[MAX_FIELD_COUNT-14],m_szFileName,(mCur_SourceCfg->second).FnSep,
			    	(mCur_SourceCfg->second).FnIndex,(mCur_SourceCfg->second).FnBegin,(mCur_SourceCfg->second).FnLen);
			}
			if((mCur_SourceCfg->second).FntIndex > 0)
			{
			    GetStrFromFN(Field[MAX_FIELD_COUNT-16],m_szFileName,(mCur_SourceCfg->second).FntSep,
			    	(mCur_SourceCfg->second).FntIndex,(mCur_SourceCfg->second).FntBegin,(mCur_SourceCfg->second).FntLen);
			}
			if(m_chIs_Ann == 'Y')
			{
				   char szAnnDir[256];
				   sprintf(szAnnDir,"%s%s",(mCur_SourceCfg->second).szSourcePath,szAnn_Dir);
				   chkDir(szAnnDir);
				   if(strcmp(szAnnDir,szLastAnn_Dir)||strcmp((mCur_SourceCfg->second).szTollCode,szLastAnn_Abbr))
				   {
				   		strcpy(szLastAnn_Dir,szAnnDir);
				   		strcpy(szLastAnn_Abbr,(mCur_SourceCfg->second).szTollCode);
				  		m_formatcheck.CommitAnnFile();
				  		m_formatcheck.Init_Ann_Dir(szAnnDir,(mCur_SourceCfg->second).szTollCode,iProcIndex);
				   }
			}
			
			DEBUG_LOG<<"FormatPlugin-szFile_Fmt:"<<(mCur_SourceCfg->second).szFile_Fmt<<endd;
			  
			pCur_TxtFmtDefine=m_txtformat.Get_CurTxtFileFmt((mCur_SourceCfg->second).szFile_Fmt);
			if(!pCur_TxtFmtDefine)
			{
			    sprintf(szLogStr,"can not find TxtFile_Fmt: %s !",(mCur_SourceCfg->second).szFile_Fmt);
			    //throw jsexcp::CException(ERR_FILEFMT,szLogStr,__FILE__,__LINE__);
				THROW_EXCEPTION(ERR_FILEFMT,szLogStr);
			}
			Bill_Route.Init(m_szSourceID,iProcIndex);
			Bill_fnTime.Init(m_szSourceID,iProcIndex);
			Bill.Init(m_szSourceID,iProcIndex);
			  
			strcpy(m_formatcheck.szTime_Fmt,(mCur_SourceCfg->second).Fmt_Time_fmt);
			m_formatcheck.iTime_FmtLen=strlen(m_formatcheck.szTime_Fmt);
			strcpy(m_formatcheck.szzTime_Fmt,(mCur_SourceCfg->second).szzTime_Fmt);
			m_formatcheck.izTime_FmtLen=strlen(m_formatcheck.szzTime_Fmt);
			strcpy(m_formatcheck.szZDate_Fmt,(mCur_SourceCfg->second).szzTime_Fmt);
			m_formatcheck.iZDate_FmtLen=strlen(m_formatcheck.szZDate_Fmt);
			m_formatcheck.iRcd_Arr_Dur =(mCur_SourceCfg->second).iRcd_Arr_Dur;
			m_formatcheck.iERcd_Arr_Dur =(mCur_SourceCfg->second).iERcd_Arr_Dur;
 			memset(szEarliestTime,'9',14);
  			memset(szLatestTime,'0',14);
  			sprintf(Field[MAX_FIELD_COUNT-4],"%ld",pMessage.getFileId());
			sprintf(Field[MAX_FIELD_COUNT-3],"%s",rate_cycle);		//2013-07-26
			sprintf(Field[MAX_FIELD_COUNT-6],"%s",file_time);		//2013-08-01
			sprintf(Field[MAX_FIELD_COUNT-18],"%s",it->second.serverCatID);		//2013-09-09
			break;
		case MESSAGE_END_FILE:

			  if(mCur_SourceCfg->second.chIs_Bill_Statics == 'Y')
			  //if(Bill.UpdateDB(szLogStr,*m_DBConn))
			  if(Bill.UpdateDB(szLogStr,conn))
			  {
			    //throw jsexcp::CException(ERR_UPDATE_STATICS,"update BILL_STATICS ERR",__FILE__,__LINE__);
				THROW_EXCEPTION(ERR_UPDATE_STATICS,"update BILL_STATICS ERR");
			  }

			  if(mCur_SourceCfg->second.chIs_Bill_Statics_fnTime == 'Y')
			  //if(Bill_fnTime.UpdateDB(szLogStr,*m_DBConn))
			  if(Bill_fnTime.UpdateDB(szLogStr,conn))
			  {
			    //throw jsexcp::CException(ERR_UPDATE_STATICS,"update BILL_STATICS_ROUTE_FNTIME ERR",__FILE__,__LINE__);
				THROW_EXCEPTION(ERR_UPDATE_STATICS,"update BILL_STATICS_ROUTE_FNTIME ERR");
			  }
			  if(mCur_SourceCfg->second.chIs_Bill_Statics_Route == 'Y')
			  //if(Bill_Route.UpdateDB(szLogStr,*m_DBConn))
			  if(Bill_Route.UpdateDB(szLogStr,conn))
			  {
			    //throw jsexcp::CException(ERR_UPDATE_STATICS,"update BILL_STATICS_ROUTE ERR",__FILE__,__LINE__);
				THROW_EXCEPTION(ERR_UPDATE_STATICS,"update BILL_STATICS_ROUTE ERR");
			  }
			if(mCur_SourceCfg->second.chIs_TimeFile == 'Y')
			    InsTimeBetweenFile(m_szSourceID,m_szFileName,szEarliestTime,szLatestTime);
			break;
		default:
			break;
	}


}

void expTrace(const char *szDebugFlag, const char *szFileName,
  int lineno, const char *msg, ...)
{
  va_list argp;
  char ss0[1024];
  char *ss1, *ss2;
  char *spara;
  int ipara;
  char sline[6];
  char cur_time[15];

  strncpy(ss0, msg, 1024);
  ss0[1023]='\0';
  ss1=ss0;
  sprintf(sline, "%05d", lineno);
//  getCurTime(cur_time);
//  cout<<"["<<cur_time<<"] ";
  //cout<<szFileName<<":"<<sline<<":"<<endl;
  //cout<<szFileName<<":"<<sline<<":";
  va_start( argp, msg );
  while (( ss2 = strchr( ss1, '%') ) != NULL) {
    *ss2 = 0;
    theJSLog<<ss1;
    switch (ss2[1])
    {
      case 'd':
        ipara=va_arg(argp, int);
        theJSLog<<ipara;
        ss1=ss2+2;
        break;
      case 's':
        spara=va_arg(argp, char *);
        theJSLog<<spara;
        ss1=ss2+2;
        break;
      default:
        theJSLog<<"%";
        ss1=ss2+1;
        break;
    }
  }

  va_end( argp );
  theJSLog<<ss1;
  theJSLog<<endi;
  
  return;
}


bool FormatPlugin::init(int argc, char *argv[])
{
	if( !PS_Process::init(argc, argv) ) return false;
	
/*********��ȡ��ʽ�������ʼ������Ҫ������Դ��,service*** 2013-03-11 add by hed*********************************************/
	if(!(dbConnect(conn)))
	{
		cout<<"���ݿ� connect error."<<endl;
		return false ;
	}
    
	mConfParam.iflowID = getFlowID();
	mConfParam.iModuleId = getModuleID();

	//char sourceGroup[8],service[8]; 
	//memset(sourceGroup,0,sizeof(sourceGroup));
	//memset(service,0,sizeof(service));

	try{

		string sql = "select source_group from TP_BILLING_LINE  where billing_line_id = :1";
		Statement stmt = conn.createStatement();
		stmt.setSQLString(sql);
		stmt<<mConfParam.iflowID;
		stmt.execute();
		if(!(stmt>>mConfParam.szSrcGrpID))
		{
			cout<<"����tp_billing_line����������ˮ��["<<mConfParam.iflowID<<"]������Դ��"<<endl;
			return false ;
		}

		sql = "select ext_param from TP_PROCESS where billing_line_id = :1 and module_id = :2";
		stmt.setSQLString(sql);
		stmt<<mConfParam.iflowID<<mConfParam.iModuleId;
		stmt.execute();
		if(!(stmt>>mConfParam.szService))
		{
			cout<<"����tp_process���ֶ�ext_param������ģ��["<<mConfParam.iModuleId<<"]service"<<endl;
			return false ;
		}
		
		theJSLog.setLog(szLogPath,szLogLevel,mConfParam.szService, mConfParam.szSrcGrpID, 001);
		
		theJSLog<<"szLogPath="<<szLogPath<<"	szLogLevel="<<szLogLevel<<endi;
		theJSLog<<"flowID="<<mConfParam.iflowID<<"	ModuleId="<<mConfParam.iModuleId<<endi;
		theJSLog<<"szSrcGrpID="<<mConfParam.szSrcGrpID<<"	szService="<<mConfParam.szService<<endi;
		 
		sql = "select a.workflow_id,c.input_path,c.output_path,b.input_id,b.output_id,c.log_tabname from C_SOURCE_GROUP_DEFINE a,C_SERVICE_FLOW b,C_SERVICE_INTERFACE c where a.source_group=:1" 
                    "and a.workflow_id = b.workflow_id and b.service=:2 and b.input_id = c.interface_id ";
		stmt.setSQLString(sql);
		stmt<<mConfParam.szSrcGrpID<<mConfParam.szService;
		stmt.execute();
		if(!(stmt>>mConfParam.iWorkflowId>>mConfParam.szInPath>>mConfParam.szOutPath>>mConfParam.iInputId>>mConfParam.iOutputId>>mConfParam.szSchCtlTabname))
		{
			theJSLog<<"C_SOURCE_GROUP_DEFINE,C_SERVICE_INTERFACE,C_SERVICE_INTERFACE������ѯʧ��:"<<sql<<endw;
			return false ;
		}	
		completeDir(mConfParam.szInPath);
		completeDir(mConfParam.szOutPath);
		
		theJSLog<<"WorkflowId="<<mConfParam.iWorkflowId<<"	InputId="<<mConfParam.iInputId
				<<"	 OutputId="<<mConfParam.iOutputId<<"	sch_table="<<mConfParam.szSchCtlTabname<<endi;

		sql = "select var_value from c_process_env where varname = 'FMT_ERR_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<mConfParam.szSrcGrpID<<mConfParam.szService;
		stmt.execute();
		if(!(stmt>>mConfParam.szErroPath))
		{
			theJSLog<<"��ʽ�������ļ�·������[FMT_ERR_DIR]û����c_process_env������"<<endw;
			return false; 
		}
		completeDir(mConfParam.szErroPath);
		
		sql = "select var_value from c_process_env where varname = 'FMT_TIMEOUT_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<mConfParam.szSrcGrpID<<mConfParam.szService;
		stmt.execute();
		if(!(stmt>>mConfParam.szTimeoutPath))
		{
			theJSLog<<"��ʽ����ʱ�ļ�·������[FMT_TIMEOUT_DIR]û����c_process_env������"<<endw;
			return false ;
		}
		completeDir(mConfParam.szTimeoutPath);
	
		sql = "select var_value from c_process_env where varname = 'FMT_OTHER_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<mConfParam.szSrcGrpID<<mConfParam.szService;	
		stmt.execute();
		if(!(stmt>>mConfParam.szOtherPath))
		{
			theJSLog<<"��ʽ��δ�����ʽ�����ļ�·��[FMT_OTHER_DIR]û����c_process_env������"<<endw;
			return false ;
		}
		completeDir(mConfParam.szOtherPath);
		
		sql = "select var_value from c_process_env where varname = 'FMT_BACKUP_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<mConfParam.szSrcGrpID<<mConfParam.szService;
		stmt.execute();
		if(!(stmt>>mConfParam.szBakPath))
		{
			theJSLog<<"���ڱ�c_process_env���ø�ʽ��ģ��ı���Ŀ¼:FMT_BACKUP_DIR"<<endw;
			return false;
		}
		completeDir(mConfParam.szBakPath);
		
		theJSLog<<"szInPath="<<mConfParam.szInPath<<" szOutPath="<<mConfParam.szOutPath<<"  szErroPath="<<mConfParam.szErroPath
				<<"  szTimeoutPath="<<mConfParam.szTimeoutPath<<"  szOtherPath="<<mConfParam.szOtherPath<<endi;

		//����SQL·������ �����ַ�����

		}catch(SQLException e)
		{
			theJSLog<<"init ���ݿ��ѯ�쳣:"<<e.what()<<endw;
			return false ;
		}
	
	
	//������������ÿ��ɨ������ԴĿ¼����ָ�������ļ�������¸�����Դ
	char sParamName[256];
	CString sKeyVal;

	memset(sParamName,0,sizeof(sParamName));
	sprintf(sParamName, "business.source.file.%d.num",getFlowID());
	if(param_cfg.bGetMem(sParamName, sKeyVal))
	{
		mConfParam.source_file_num = sKeyVal.toInteger();
		theJSLog<<sParamName<<"="<<mConfParam.source_file_num<<endi;
	}
	else
	{	
		theJSLog<<"���������ļ���������ˮ��["<<mConfParam.iflowID<<"]������Դÿ��ɨ���ļ��ĸ���,������:"<<sParamName<<endw;
		return false ;
	}	


	bool bb = initializeLog(argc,argv,false);  //�Ƿ����ģʽ
	if(!bb)
	{
			return false;
	}

	bool flag = true;
	for(int i=1;i<argc;i++)
	{
		if(strcmp(argv[i],"-k") == 0)
		{
			theJSLog<<"ģ��["<<module_name<<"]����������..."<<endi;
			flag = false;
			mdrParam.m_enable = false;
			break;
		}	
	}
	
	if(flag)
	{
		if(!drInit())  return false;
	}
	if(!(rtinfo.connect()))
	{
		theJSLog<<"��������ʱ�ڴ���ʧ��"<<endw;
		return false;
	}	
	rtinfo.getDBSysMode(petri_status);
	petri_status_tmp = petri_status;
	theJSLog<<"petri status:"<<petri_status<<endi;

	//theJSLog.setLog(szLogPath, szLogLevel,service , sourceGroup, 001);	
	//��־��ʼ·���� log/run_log/����/sourceGroup/service ��־�ļ�����Ǽ�
	//theJSLog<<"  ����Դ��:"<<sourceGroup<<"  service:"<<service<<"  �������·��:"<<input_path<<" ������·��:"<<output_path<<"  �����ļ�·��:"<<erro_path<<" ��ʱ�ļ�·��:"<<timeout_path<<" ��ʽ��δ���廰���ļ�·��:"<<other_path<<"�����ļ�·��:"<<bak_path<<"	��־·��:"<<szLogPath<<"  ��־����:"<<"  SQL���·��:"<<sql_path<<endi;

	try
	{
			/*���ø�ʽ������ĳ�ʼ������,��������Դ��Ϣ*/
			init(mConfParam.szSrcGrpID,mConfParam.szService,0);
			pps = new PacketParser();
			res = new ResParser();
	}
	catch(SQLException e)
	{
		  memset(erro_msg,0,sizeof(erro_msg));
		  sprintf(erro_msg,"��ʽ������ĳ�ʼ��init() sql err : %s (%s)",e.what(),sql);
		  theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//sqlִ��ʧ��

		  conn.close();
		  return false ;
	}
	catch(jsexcp::CException &e)
	{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"��ʽ������ĳ�ʼ��init() err : %s",e.GetErrMessage());
			theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		//�ֶ�ת������

			conn.close();
			exit(-1);
			//return false;		//�ڴ�û���ͷţ����������Ŀǰǿ���˳�
	} 
	
	conn.close();

	DIR *dirptr = NULL; 
	char input_dir[JS_MAX_FILEFULLPATH_LEN],output_dir[JS_MAX_FILEFULLPATH_LEN],erro_dir[JS_MAX_FILEFULLPATH_LEN],timeout_dir[JS_MAX_FILEFULLPATH_LEN],other_dir[JS_MAX_FILEFULLPATH_LEN],bak_dir[JS_MAX_FILEFULLPATH_LEN];
	int rett = -1;

	//��ȡ�����ļ�·��
	for(map<string,SOURCECFG>::const_iterator iter = m_SourceCfg.begin();iter !=m_SourceCfg.end(); ++iter)
	{		   		
			memset(input_dir,0,sizeof(input_dir));
			strcpy(input_dir,iter->second.szSourcePath);
			strcat(input_dir,mConfParam.szInPath);
			if((dirptr=opendir(input_dir)) == NULL)
			{
					 memset(erro_msg,0,sizeof(erro_msg));
					 sprintf(erro_msg,"����Դ[%s]�������ļ�·��[%s]������",iter->first,input_dir);
					 theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����
					 return false ;
			}else closedir(dirptr);
			
			memset(output_dir,0,sizeof(output_dir));
			strcpy(output_dir,iter->second.szSourcePath);
			strcat(output_dir,mConfParam.szOutPath);
			if((dirptr=opendir(output_dir)) == NULL)
			{		 
					theJSLog<<"����Դ��"<<iter->first<<"��������ļ�·��: "<<output_dir<<"�����ڣ����д���"<<endw;
					rett = mkdir(output_dir,0755);
					if(rett == -1)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"����Դ[%s]����������ļ��ļ�·��[%s]�����ڣ����д���ʧ��",iter->first,output_dir);
						theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����
						return false;
					}

			}else closedir(dirptr);

			memset(erro_dir,0,sizeof(erro_dir));
			strcpy(erro_dir,iter->second.szSourcePath);
			strcat(erro_dir,mConfParam.szErroPath);
			//struct stat fileStat; if ((stat(dir.c_str(), &fileStat) == 0) && S_ISDIR(fileStat.st_mode))
			if((dirptr=opendir(erro_dir)) == NULL)
			{
					theJSLog<<"����Դ��"<<iter->first<<"���Ĵ����ļ�·��: "<<erro_dir<<"�����ڣ����д���"<<endw;
					rett = mkdir(erro_dir,0755);
					if(rett == -1)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"����Դ[%s]�Ĵ��󻰵��ļ��ļ�·��[%s]�����ڣ����д���ʧ��",iter->first,erro_dir);
						theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����
						return false;
					}
			}else closedir(dirptr);
			
			memset(timeout_dir,0,sizeof(timeout_dir));
			strcpy(timeout_dir,iter->second.szSourcePath);		
			strcat(timeout_dir,mConfParam.szTimeoutPath);
			if((dirptr=opendir(timeout_dir)) == NULL)
			{
					theJSLog<<"����Դ��"<<iter->first<<"���ĳ�ʱ�ļ�·��: "<<timeout_dir<<"�����ڣ����д���"<<endw;
					rett = mkdir(timeout_dir,0755);
					if(rett == -1)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"����Դ[%s]�ĳ�ʱ�����ļ��ļ�·��[%s]�����ڣ����д���ʧ��",iter->first,timeout_dir);
						theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����
						return false;
					}
			
			}else  closedir(dirptr);
			
			memset(other_dir,0,sizeof(other_dir));
			strcpy(other_dir,iter->second.szSourcePath);
			strcat(other_dir,mConfParam.szOtherPath);
			if((dirptr=opendir(other_dir)) == NULL)
			{
					theJSLog<<"����Դ��"<<iter->first<<"����δ�����ʽ�����ļ�·��: "<<other_dir<<"�����ڣ����д���"<<endw;
					rett = mkdir(other_dir,0755);
					if(rett == -1)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"����Դ[%s]��δ�����ʽ�����ļ��ļ�·��[%s]�����ڣ����д���ʧ��",iter->first,other_dir);
						theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����
						
						return false;
					}		

			}else  closedir(dirptr);

			memset(bak_dir,0,sizeof(bak_dir));
			strcpy(bak_dir,iter->second.szSourcePath);
			strcat(bak_dir,mConfParam.szBakPath);
			if((dirptr=opendir(bak_dir)) == NULL)
			{
				//memset(erro_msg,0,sizeof(erro_msg));
				//sprintf(erro_msg,"����Դ[%s]�ı����ļ�·��[%s]������",iter->first,bak_dir);
				//theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����
				
				theJSLog<<"����Դ��"<<iter->first<<"���Ļ����ļ�����·��: "<<bak_dir<<"�����ڣ����д���"<<endw;
				rett = mkdir(bak_dir,0755);
				if(rett == -1)
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"����Դ[%s]�Ļ����ı���·��[%s]�����ڣ����д���ʧ��",iter->first,bak_dir);
					theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����
						
					return false;
				}	

			}else  closedir(dirptr);

	}
	
	it = m_SourceCfg.begin();

	theJSLog<<"��ʽ����ʼ����ϣ�... \n"<<endi;

	return true;
}


//У���ļ��Ƿ��ظ�  >=1��ʾ�ظ� 0��ʾ���ظ���
int FormatPlugin::checkFileRepeat(char* file)
{
	int count = 0;
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select count(*) from %s where source_id = '%s' and filename = '%s' ",mConfParam.szSchCtlTabname,m_szSourceID,file);	
	Statement stmt = conn.createStatement();
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>count;
	if(count)	//���ӵ��ȱ��ҵ��� �ٲ�ѯ�쳣�ǼǱ�,�����ҵ����ж���,��������
	{
		memset(sql,0,sizeof(sql));
		sprintf(sql,"select count(*) from D_ERRFILE_INFO where source_id = '%s' and filename='%s'" ,m_szSourceID,file);
		stmt.setSQLString(sql);
		stmt.execute();
		stmt>>count;

		if(count)  return 0;   
		else	   return 1;
	}

	stmt.close();

	return 0;
}


//У���ļ�����ʽ�Ƿ���ȷ  0��ʾУ��ʧ�ܣ�1��ʾУ��ɹ�
int FormatPlugin::checkFileName(string source,char* fileName)
{ 
	int rett = CHECK_SUCCESS,len = 0;
	char tmp[1024];
	map< string,vector<FileNameFmt> >::const_iterator iter = mapFileNameFmt.find(source);
	if(iter == mapFileNameFmt.end())
	{		
		return rett;
	}

	memset(erro_msg,0,sizeof(erro_msg));

	FileNameFmt fmt;
	vector<FileNameFmt> vfmt = iter->second;
	for(int i = 0;i<vfmt.size();i++)
	{
		fmt = vfmt[i] ;
		memset(tmp,0,sizeof(tmp));
		strncpy(tmp,fileName+fmt.index,fmt.len);		//ͨ����ȡ�ļ�����λ��

		//����У�� 
		switch(fmt.check_type)
		{
			case 'C' :            //����У��
				  
				  if(strcmp(fmt.check_value,tmp) != 0)
				  {
					  strcpy(err_code,fmt.err_code);
					  sprintf(mdrParam.m_AuditMsg,"%s:%c:%s",tmp,fmt.check_type,fmt.err_code);

					  memset(erro_msg,0,sizeof(erro_msg));
					  sprintf(erro_msg,"[%s]�ļ���У��:����У��ʧ��,[%s != %s]",m_szFileName,tmp,fmt.check_value);
					  return CHECK_FAIL;
				  }

				  break;

			case 'R' :
					
				  strncpy(rate_cycle,tmp,6);
				  rate_cycle[6] = '\0';
		
			case 'Z' :			 //���ڸ�ʽУ��֧��6λ 8λ 10λ 12λ 14λ
				   
				  len = strlen(tmp);
				  if(len != strlen(fmt.check_value))  //ҪУ��������ַ�����ȡ�ĳ��Ⱥ����õ����ڸ�ʽ��һ�����϶�У��ʧ��
				  {
					    strcpy(err_code,fmt.err_code);
						sprintf(mdrParam.m_AuditMsg,"%s:%c:%s",tmp,fmt.check_type,fmt.err_code);

						memset(erro_msg,0,sizeof(erro_msg));
					    sprintf(erro_msg,"[%s]�ļ���У��:ҪУ��������ַ�����ȡ�ĳ��Ⱥ����õ����ڸ�ʽ��һ��, [%s != %s]",m_szFileName,tmp,fmt.check_value);
						return CHECK_FAIL;
				  }
				  
				  for(int j = 0;j<len;j++)
				  {
						if(tmp[j] > '9' || tmp[j] < '0')
						{
							strcpy(err_code,fmt.err_code);
							sprintf(mdrParam.m_AuditMsg,"%s:%c:%s",tmp,fmt.check_type,fmt.err_code);

							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"[%s]�ļ���У�飺 ����У��֮����У��ʧ��, ���ڱ���ȫΪ���� [%s] ",m_szFileName,tmp);
							return CHECK_FAIL;
						}
				  }

				  if(len == 6||len == 8||len ==10||len ==12||len ==14)
				  {
						if(checkDate(tmp) == CHECK_FAIL)
						{
								strcpy(err_code,fmt.err_code);
								sprintf(mdrParam.m_AuditMsg,"%s:%c:%s",tmp,fmt.check_type,fmt.err_code);

								memset(erro_msg,0,sizeof(erro_msg));
								sprintf(erro_msg,"[%s]�ļ���У��:����ʱ�䲻�Ϸ�,[%s]",m_szFileName,tmp);
								return CHECK_FAIL;
						}
				  }
				  else 
				  {
					  strcpy(err_code,fmt.err_code);
					  sprintf(mdrParam.m_AuditMsg,"%s:%c:%s",tmp,fmt.check_type,fmt.err_code);

					  memset(erro_msg,0,sizeof(erro_msg));
					  sprintf(erro_msg,"[%s]�ļ���У��:���ڸ�ʽУ��ʧ��,���ȱ���Ϊ6 8 10  12 14,[%s]",m_szFileName,tmp);
					  return CHECK_FAIL;
				  }

				  break;

			case 'D' :          //����У�� 
				  
				  for(int j = 0;j<strlen(tmp);j++)
				  {
						if(tmp[j] > '9' || tmp[j] < '0')
						{
							strcpy(err_code,fmt.err_code);
							sprintf(mdrParam.m_AuditMsg,"%s:%c:%s",tmp,fmt.check_type,fmt.err_code);

							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"[%s]�ļ���У��:����У��ʧ��,[%s]",m_szFileName,tmp);
							return CHECK_FAIL;
						}
				  }
				  break;

			default :
				  break;

		}
	}
	
		
	return rett ;
}

//У���ļ���¼ͷβ�Ƿ���ȷ
int FormatPlugin::checkRecorHT(char* fileType,char* head,char* tail)
{
	theJSLog<<"�����ļ����ͣ�"<<fileType<<"  ͷ��¼��"<<head<<"  β��¼��"<<tail<<endi;
	int rett = CHECK_SUCCESS,len = 0,flag = 0;
	char tmp[1024],erro_tmp[1024];
	string str;
	str.append(fileType);
	
	RecordHTFmt fmt;
	memset(erro_msg,0,sizeof(erro_msg));
	memset(erro_tmp,0,sizeof(erro_tmp));

	if(head[0] != '\0')						//����ͷ
	{	
		memset(tmp,0,sizeof(tmp));
		strcpy(tmp,head);
		theJSLog<<"���ļ�ͷ����У��..."<<endi;

		map< string,vector<RecordHTFmt> >::const_iterator iter1 = mapFileRecordHeadFmt.find(str);
		if(iter1 == mapFileRecordHeadFmt.end())
		{	
			theJSLog<<"û���ҵ��ļ���ʽ["<<str<<"]��ͷ��¼�ֶ�����"<<endw;
			return CHECK_SUCCESS;
		}

		vector<RecordHTFmt> vfmta = iter1->second;
		//vector<string> coloum_value_head;
		
		flag = 0;
		if(vfmta[0].seperator != '')
		{
			file_head.clear();
			splitString(tmp,vfmta[0].seperator,file_head,false,false);  //�������մ�
			flag = 1;
		
			//file_head = coloum_value_head;
		}	

		for(int i = 0;i<vfmta.size();i++)
		{
			fmt = vfmta[i] ;
			memset(tmp,0,sizeof(tmp));
		
			if(flag)             //��ʾͨ���ָ�����ȡ�ֶ�ֵ
			{
				strcpy(tmp,file_head[vfmta[i].number-1].c_str());
			}
			else
			{
				strncpy(tmp,head+fmt.index,fmt.len);
			}

			//У������
			if( checkRule(fmt,tmp) == CHECK_FAIL) 
			{	
				sprintf(erro_tmp,"[%s]�ļ���¼ͷУ��:",m_szFileName);
				strcat(erro_tmp,erro_msg);
				strcpy(erro_msg,erro_tmp);

				strcpy(err_code,fmt.err_code);		//�ֶ�I��ֵ����ĳ�����͵�У��ʧ��
				sprintf(mdrParam.m_AuditMsg,"%s:%c:%s:%s",fmt.name,fmt.check_type,tmp,fmt.err_code);

				return CHECK_FAIL;
			}

		}
	}


	if(tail[0] != '\0')				  //У��β
	{	
		memset(tmp,0,sizeof(tmp));
		strcpy(tmp,tail);
		theJSLog<<"���ļ�β����У��..."<<endi;

		map< string,vector<RecordHTFmt> >::const_iterator iter2 = mapFileRecordTailFmt.find(str);
		if(iter2 == mapFileRecordTailFmt.end())
		{	
			theJSLog<<"û���ҵ��ļ���ʽ["<<str<<"]��β��¼�ֶ�����"<<endw;
			return CHECK_SUCCESS;
		}
		
		vector<RecordHTFmt> vfmtb = iter2->second;
		//vector<string> coloum_value_tail;
		
		flag = 0;
		if(vfmtb[0].seperator != '')
		{
			file_tail.clear();
			splitString(tmp,vfmtb[0].seperator,file_tail,false,false);
			flag = 1;
		
			//file_tail = coloum_value_tail;
		}	

		for(int i = 0;i<vfmtb.size();i++)
		{
			fmt = vfmtb[i] ;
			memset(tmp,0,sizeof(tmp));
		
			if(flag)             //��ʾͨ���ָ�����ȡ�ֶ�ֵ
			{
				strcpy(tmp,file_tail[vfmtb[i].number-1].c_str());
			}
			else
			{
				strncpy(tmp,tail+fmt.index,fmt.len);
			}

			//У������
			if( checkRule(fmt,tmp) == CHECK_FAIL) 
			{
				sprintf(erro_tmp,"[%s]�ļ���¼βУ��:",m_szFileName);
				strcat(erro_tmp,erro_msg);			
				strcpy(erro_msg,erro_tmp);	
				
				strcpy(err_code,fmt.err_code);		//�ֶ�I��ֵ����ĳ�����͵�У��ʧ��
				sprintf(mdrParam.m_AuditMsg,"%s:%c:%s:%s",fmt.name,fmt.check_type,tmp,fmt.err_code);

				return CHECK_FAIL;
			}
		}


	}
		
	return rett;
}


//����У�� ,���ļ���У���ͷβУ��������һ��
int FormatPlugin::checkRule(RecordHTFmt fmt,char* column_value)
{	
	int rett = 0,len = 0;
	char szbuff[256],szbuff2[256];
		
		//2013-07-26,��ȡ���ڣ����������־���ж�
		//if(strcmp(fmt.spec_flag,"RATE_CYCLE") == 0)
		//{
		//	strcpy(rate_cycle,column_value);
			//cout<<"���ڣ�"<<rate_cycle<<endl;
		//}

		len = strlen(column_value);
		switch(fmt.check_type)
		{
			case 'C' :            //����У��
				  
				  if(strcmp(fmt.check_value,column_value) != 0)
				  {
					  //strcpy(err_code,fmt.err_code);
					  memset(erro_msg,0,sizeof(erro_msg));
					  sprintf(erro_msg,"����У��ʧ��,[%s != %s]",column_value,fmt.check_value);
					  return CHECK_FAIL;
				  }

				  break;
		
			case 'Z' :			 //���ڸ�ʽУ��֧��6λ 8λ 10λ 12λ 14λ
				   
				  if(len != strlen(fmt.check_value))  //ҪУ��������ַ�����ȡ�ĳ��Ⱥ����õ����ڸ�ʽ��һ�����϶�У��ʧ��
				  {
					    //strcpy(err_code,fmt.err_code);
					    memset(erro_msg,0,sizeof(erro_msg));
					    sprintf(erro_msg,"ҪУ��������ַ�����ȡ�ĳ��Ⱥ����õ����ڸ�ʽ��һ��,[%s != %s]",column_value,fmt.check_value);
						return CHECK_FAIL;
				  }
				  
				  for(int j = 0;j<len;j++)
				  {
						if(column_value[j] > '9' || column_value[j] < '0')
						{
							//strcpy(err_code,fmt.err_code);
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"����У��֮����У��ʧ��,���ڱ���ȫΪ����,[%s]",column_value);
							return CHECK_FAIL;
						}
				  }

				  if(len == 6||len == 8||len ==10||len ==12||len ==14)
				  {
							if(checkDate(column_value) == CHECK_FAIL)
							{
								//strcpy(err_code,fmt.err_code);
								memset(erro_msg,0,sizeof(erro_msg));
								sprintf(erro_msg,"���ڸ�ʽУ��ʧ��,���ڲ��Ϸ� [%s]",column_value);
								return CHECK_FAIL;
							}
				  }
				  else 
				  {
					  //strcpy(err_code,fmt.err_code);
					  memset(erro_msg,0,sizeof(erro_msg));
					  sprintf(erro_msg,"���ڸ�ʽУ��ʧ��,���ȱ���Ϊ6,8,10,12,14 [%s]",column_value);
					  return CHECK_FAIL;
				  }

				  break;

			case 'D' :          //����У��,��Ϊ��ֵУ��
				  
				  for(int j = 0;j<len;j++)
				  {
					    if((j == 0) && column_value[j] == '-')		//�������
					    {
							continue;
						}
						
						//if((j != 0 || j != (len-1)) && column_value[j] == '.')	//С����� .36, 45. ����Ҳ�Ϸ�
						//{
						//	continue;
						//}

						if(column_value[j] > '9' || column_value[j] < '0')
						{
							//strcpy(err_code,fmt.err_code);
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"����У��ʧ��,����Ϊ��ֵ,[%s]",column_value);
							return CHECK_FAIL;
						}
				  }
				  break;

			case 'J':          //��¼��У��
				  
				  if(atoi(column_value) !=  record_num)
				  {
					    //strcpy(err_code,fmt.err_code);
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"�ļ���¼��У��ʧ��,[%s != %d]",column_value,record_num);
						return CHECK_FAIL;
				  }

				  break;
			case 'F':			 //�ַ���������ȡ-����У��ֻ֧��substring���� ���ñ�����ʱ֧�� �ļ��� ����Դ���ļ�����
				
			/*
				  if(record_num == 0)	//����ϵͳ�������Ӫ�̽������嵥�ļ����ơ����ļ�ʱ���ֶ�Ϊ�ա�����У��
				  {
						break;
				  }
				  if(strcmp("substring($FILENAME)",fmt.check_value) == 0)		//У���ļ���
				  {
						if(strcmp(column_value,m_szFileName) != 0)
					    {
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"�ļ�����һ��,[%s != %s]",column_value,m_szFileName);
							return CHECK_FAIL;
					    }
						else
						{
							return CHECK_SUCCESS;
						}
				  }
			*/
				  if(strncmp(fmt.check_value,"substring($",11) == 0)  //substring($FILENAME,4,6)
				  {
					    memset(szbuff,0,sizeof(szbuff));
						memset(szbuff2,0,sizeof(szbuff2));
						strcpy(szbuff,fmt.check_value+11);   	 
						char varName[256],varValue[256],*p;
						int index,len;
						memset(varName,0,sizeof(varName));
						memset(varValue,0,sizeof(varValue));

						p = strchr(szbuff,',');
						if(p == NULL)						return CHECK_FAIL;  
						strncpy(varName,szbuff,p-szbuff);
						//cout<<"������ = "<<varName<<endl;
						if(strcmp(varName,"FILENAME") == 0)
						{
								strcpy(varValue,m_szFileName);					
						}
						else
					    {
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"�ַ��������д��ڲ�ʶ��ı���! $%s",varName);
							return CHECK_SUCCESS;
					    }

						memset(varName,0,sizeof(varName));
						strcpy(szbuff2,p+1);						
						p = strchr(szbuff2,',');
						if(p == NULL)						return CHECK_FAIL;
						strncpy(varName,szbuff2,p-szbuff2);
						//cout<<"��ȡ��������ʼλ�� = "<<varName<<endl;
						index = atoi(varName);
						if(index <= 0)
						{
							//strcpy(err_code,fmt.err_code);
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"�ַ�������[%s]���ò��Ϸ� index <= 0 ",fmt.check_value);
							return CHECK_FAIL;
						}
						index--;

						memset(varName,0,sizeof(varName));
						strcpy(szbuff2,p+1);
						p = strchr(szbuff2,')');
						if(p == NULL)						return CHECK_FAIL;
						strncpy(varName,szbuff2,p-szbuff2);
						//cout<<"��ȡ�����ĳ��� = "<<varName<<endl;
						len = atoi(varName);
						if(len <= 0)
						{
							//strcpy(err_code,fmt.err_code);
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"�ַ�������[%s]���ò��Ϸ� length <= 0 ",fmt.check_value);
							return CHECK_FAIL;
						}

						memset(szbuff,0,sizeof(szbuff));
						strncpy(szbuff,varValue+index,len);
						if(strcmp(column_value,szbuff)  != 0)
						{
							//strcpy(err_code,fmt.err_code);
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"�ַ���������ȡ����У��ʧ�� %s != %s",column_value,fmt.check_value);
							return CHECK_FAIL;
						}					
						
				  }
				  else
				  {
						theJSLog<<"�ַ���У�麯�����ʽ���ò��Ϸ�����ʱ����У��"<<fmt.check_value<<endw;
				  }

				  break ;

			default :           //Ĭ�ϲ�У�� S
				  break;

		}	
		
		return CHECK_SUCCESS;
}


//У�������Ƿ�Ϸ�
//�����¸�ʽ����У��,���ж��Ƿ�����
// ���� 6 yyyymm
// ���� 8 yyyymmdd
// ���� 10 yymmddhh
// ���� 12 yymmddhhmi
// ���� 14 yymmddhhmiss

int FormatPlugin::checkDate(char* date)
{
	//num =sscanf(buf, "%4d%2d2%d2%d2%2d%2s", &year, &month, &day, &hour, &minute, &second);
	int iTemp,iTemp2;
	char chTemp[10];
	memset(chTemp,0,sizeof(chTemp));
	
	int len = strlen(date);

	//У�����2000-2999
	strncpy(chTemp,date,4);
	chTemp[4] = 0;
	iTemp = atoi(chTemp);
	if(iTemp < 2000 || iTemp > 2999)
	{			
			return CHECK_FAIL;
	}
	
	//У���·�1-12
	date += 4;
	strncpy(chTemp,date,2);
	chTemp[2] = 0;
	iTemp = atoi(chTemp);
	if((iTemp < 1) || (iTemp > 12))
	{
		return CHECK_FAIL;
	}
		
	if(len > 6)
	{	
		//У����
		date += 2;
		strncpy(chTemp, date, 2);
		chTemp[2] = 0;
		iTemp2 = atoi(chTemp);
		switch(iTemp)
		{
			case 1:
			case 3:
			case 5:
			case 7:
			case 8:
			case 10:
			case 12:
				 if ( (iTemp2 < 1) || (iTemp2 > 31) ) return CHECK_FAIL;
				 break;
			case 4:
			case 6:
			case 9:
			case 11:
				if ( (iTemp2 < 1) || (iTemp2 > 30) )  return CHECK_FAIL;
				break;
			default:
				if ( (iTemp2 < 1) || (iTemp2 > 29) )  return CHECK_FAIL;
	     }
		
		//У��ʱ
		if(len > 8)
		{
			date += 2;
			strncpy(chTemp, date, 2);
			chTemp[2] = 0;
			iTemp = atoi(chTemp);
			if((iTemp < 0) || (iTemp > 23))
			{
					return CHECK_FAIL;

			}
		}
		
		//У���
		if(len > 10)
		{
			date += 2;
			strncpy(chTemp, date, 2);
			chTemp[2] = 0;
			iTemp = atoi(chTemp);
			if((iTemp < 0) || (iTemp > 59))
			{
					return CHECK_FAIL;

			}
		}

		//У����
		if(len > 12)
		{
			date += 2;
			strncpy(chTemp, date, 2);
			chTemp[2] = 0;
			iTemp = atoi(chTemp);
			if((iTemp < 0) || (iTemp > 59))
			{
					return CHECK_FAIL;

			}
		}
		
	}
	
	return CHECK_SUCCESS;
}

/***************************************/
void FormatPlugin::run()
{
	short db_status = 0;
	int ret = 0,event_sn, event_type;
	long param1, param2, src_id;

	char tmp[512],filter[50],inputFilePath[512];
	
while(1)
{	
	if(gbExitSig)						//�������˳�
	{
		theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR,"Ӧ�ó����յ��˳��ź�");
		prcExit();
		return;
	}

	ret=getCmd(event_sn, event_type, param1, param2, src_id);
	if(ret == 1)
	{
		if(event_type == EVT_CMD_STOP)
		{
			theJSLog<<"***********���յ��˳�����**********************\n"<<endw;
			prcExit();
		}
	}

	theJSLog.reSetLog();
	
	if(mdrParam.drStatus == 1)  //��ϵͳ���յ����ݺ�Ž��д���
	{
			rtinfo.getDBSysMode(db_status);
			if(db_status != petri_status_tmp)
			{
				theJSLog<<"���ݿ�״̬�л�... "<<petri_status_tmp<<"->"<<db_status<<endw;
				int cmd_sn = 0;
				if( !putEvt(cmd_sn, EVT_RPT_DBSTATUS, 0, db_status, DSPCH_PRC_ID) )
				{
					theJSLog<<"�������ݿ����״̬ʧ�ܣ�\n"<<endw;
					continue;
				}
				petri_status_tmp = db_status;
			}

			//���trigger�����ļ��Ƿ����
			 if(!CheckTriggerFile())
			 {
				sleep(1);
				continue ;
			 }
			
			memset(mdrParam.m_SerialString,0,sizeof(mdrParam.m_SerialString));
			ret = drVarGetSet(mdrParam.m_SerialString);
			if(ret)
			{
				theJSLog<<"ͬ��ʧ��..."<<endw;
				continue ;
			}
			
			if(!(dbConnect(conn)))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"run() �������ݿ�ʧ��,dr_AbortIDX() ");
				theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��

				dr_AbortIDX();
				continue ;
			}
			
			memset(sql,0,sizeof(sql));
			strcpy(sql,"select S_FILE_RECEIVED.NEXTVAL from dual");		//�����ݿ��FileID,��֤���߶��õ�һ��,���������л�
			Statement stmt = conn.createStatement();
			stmt.setSQLString(sql);
			stmt.execute();					
			stmt>>file_id;				
			stmt.close();

			//��ȡͬ������
			vector<string> data;		
			splitString(mdrParam.m_SerialString,";",data,false,false);  //���͵��ַ�������ԴID,�ļ���,sqlFile

			it = m_SourceCfg.find(data[0]);
			if(it == m_SourceCfg.end())
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"û���ҵ�������Դ��Ϣ[%s],dr_AbortIDX()",data[0]);		//��������δ����
				theJSLog.writeLog(LOG_CODE_SOURCE_NOT_FIND,erro_msg);

				dr_AbortIDX();
				continue ;
			}
			
			memset(inputFilePath,0,sizeof(inputFilePath));	
			strcpy(inputFilePath,it->second.szSourcePath);
			strcat(inputFilePath,mConfParam.szInPath); 

			memset(fileName,0,sizeof(fileName));
			strcpy(fileName,inputFilePath);
			strcat(fileName,data[1].c_str());
			strcpy(m_szFileName,data[1].c_str());
			
			file_id = atol(data[2].c_str());			//file_id����ϵͳ��ȡ

			if(petri_status != atoi(data[3].c_str()))
			{
				theJSLog<<"��ϵͳ�����ݿ�״̬�������л�..."<<endw;
			}
			petri_status = atoi(data[3].c_str());		//��ϵͳ��״̬������ϵͳ����	

			//int dr_GetAuditMode()1��ʾͬ����2��ʾ����, ����Ϊʧ�ܣ�-1�����ô���-2�����ļ���ȡ��������
			int iStatus = dr_GetAuditMode(module_name);
			if(iStatus == 1)		//ͬ��ģʽ,ֻ���ǵ������
			{	
				bool flag = false;
				int times = 1;
				while(times < 31)
				{
					if(access(fileName,F_OK|R_OK))
					{
						theJSLog<<"������"<<times<<"���ļ�"<<endi;
						times++;
						sleep(10);
					}
					else
					{
						flag = true;
						break;
					}
				}

				if(!flag)
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"�޷���ѯ���ļ�:%s,dr_AbortIDX() ",fileName);
					theJSLog.writeLog(LOG_CODE_FILE_MISSING,erro_msg);

					dr_AbortIDX();		
					continue ;
				}
			}
			else if(iStatus == 2)		//����ģʽ,Ĭ��300s
			{
				//�����ж�
				if(gbExitSig)
				{
					dr_AbortIDX();

					theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "Ӧ�ó����յ��˳��ź�");
					prcExit();
					return;
				}

				while(1)
				{
					//�����ж�
					if(access(fileName,F_OK|R_OK))
					{
						sleep(10);
					}
					else
					{
						break;
					}
				}
			}
			else
			{
				char tmp[50] = {0};
				snprintf(tmp, sizeof(tmp), "����ƽ̨dr_GetAuditMode�������ô��󣬷���ֵ[%d]", iStatus);
				theJSLog<<tmp<<endw;
				continue ;
			}
			
			theJSLog<<"######## start deal file "<<fileName<<" ########"<<endi;

			setSQLFileName(data[1].c_str());
			ret = dealFile();						//�����ļ�	
			dealAuditResult(ret);
			conn.close();
			theJSLog<<"######## end deal file ########\n"<<endi;
			
	}
	else			//��ϵͳ,������ϵͳ
	{			
			if(it == m_SourceCfg.end())
			{
				it = m_SourceCfg.begin();
			}

			int counter = 0;
			//CF_CFscan scan;

			//����ɨ���������Դ
			//for(it = m_SourceCfg.begin();it != m_SourceCfg.end();++it)        //�Ȼ�ȡ����Դ��Ϣ
			//{	
				memset(inputFilePath,0,sizeof(inputFilePath));				
				strcpy(inputFilePath,it->second.szSourcePath);
				strcat(inputFilePath,mConfParam.szInPath); 
			
				memset(filter,0,sizeof(filter));
				strcpy(filter,it->second.filterRule);		  //��������
			
				if(scan.openDir(inputFilePath))
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"�򿪻����ļ�Ŀ¼[%s]ʧ��",inputFilePath);
					theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //��Ŀ¼����
					//errLog(4,"",FILEREC_ERR_SCAN_RECDIR,sz_errmsg,__FILE__,__LINE__);
					sleep(2);
					continue ;	
				}		
				
				//ѭ����ȡĿ¼��ɨ���ļ��У���ȡ�ļ�
				int rett = -1 ;
				counter = 0;
				while(1) 
				{	
					ret=getCmd(event_sn, event_type, param1, param2, src_id);
					if(ret == 1)
					{
						if(event_type == EVT_CMD_STOP)
						{
							scan.closeDir();
							theJSLog<<"***********���յ��˳�����**********************\n"<<endw;
							prcExit();
						}
					}
					
					rtinfo.getDBSysMode(db_status);
					if(db_status != petri_status)
					{
						theJSLog<<"���ݿ�״̬�л�... "<<petri_status<<"->"<<db_status<<endw;
						int cmd_sn = 0;
						if( !putEvt(cmd_sn, EVT_RPT_DBSTATUS, 0, db_status, DSPCH_PRC_ID) )
						{
							theJSLog<<"�������ݿ����״̬ʧ�ܣ�\n"<<endw;
							continue;
						}
						petri_status = db_status;
						petri_status_tmp = db_status;
					}

					memset(fileName,0,sizeof(fileName));
					rett = scan.getFile(filter,fileName);  				
					if(rett == 100)
					{		
						break ;  
					}
					else if(rett == -1)
					{
						theJSLog<<"��ȡ�ļ���Ϣʧ��:"<<strerror(errno)<<endw;
						continue;			//��ʾ��ȡ�ļ���Ϣʧ��
					}

					counter++;				//ɨ��һ���ļ�������+1	
					
					if(counter > mConfParam.source_file_num)
					{	
						sleep(5);
						break;				//cout<<"ɨ��10�κ������¸�����Դ"<<endl;
					}

					/*�����ļ�*.tmp,*TMP,~* */			
					char* p = strrchr(fileName,'/');
					memset(tmp,0,sizeof(tmp));
					strcpy(tmp,p+1);
					if(tmp[0] == '~' ) continue;
					if(strlen(tmp) > 2)
					{		
						int pos = strlen(tmp)-3;
						//cout<<"��׺��Ϊ��"<<tmp+pos<<endl;
						if((strcmp(tmp+pos,"tmp") && strcmp(tmp+pos,"TMP")) == 0) 
						{
							//cout<<"ɨ�赽��ʱ�ļ�������"<<fileName<<endl;
							continue;
						}
					}		
					
					if(!(dbConnect(conn)))
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"run() �������ݿ�ʧ��!");
						//theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//�������ݿ�ʧ��
						theJSLog<<erro_msg<<endw;
						sleep(30);
						continue ;
					}

					theJSLog<<"######## start deal file "<<fileName<<" ########"<<endi;

					strcpy(m_szFileName,p+1);  //�����ļ���,ȥ·��					
					
					memset(sql,0,sizeof(sql));
					strcpy(sql,"select S_FILE_RECEIVED.NEXTVAL from dual");		//�����ݿ����FileID
					Statement stmt = conn.createStatement();
					stmt.setSQLString(sql);
					stmt.execute();					
					stmt>>file_id;				
					stmt.close();

					//ͬ����������ԴID,�ļ���,sql�ļ���
					memset(mdrParam.m_SerialString,0,sizeof(mdrParam.m_SerialString));
					sprintf(mdrParam.m_SerialString,"%s;%s;%ld;%d",it->first,m_szFileName,file_id,petri_status);
					ret = drVarGetSet(mdrParam.m_SerialString);
					if(ret)
					{
						theJSLog<<"ͬ��ʧ��...."<<endw;
						break ;
					}

					setSQLFileName(m_szFileName);

					ret = dealFile();			//�����ļ�			
					dealAuditResult(ret);
					theJSLog<<"######## end deal file ########\n"<<endi;
					
					conn.close();

				}//while(1)
				
				scan.closeDir();
				
				++it;
			//}//for

		//sleep(2);		
	}

	sleep(5);
 }//while(1)

}

int FormatPlugin::dealFile()
{			
	    //long file_id = 0;
		int ret = 0,deal_flag = 0; //��ʽ��������ȷ��־
		char dir[512],szFiletypeIn[10];
		char szBuff[1012],head[1024],tail[1024];
		char outFilePath[256],inputFilePath[256];		
		char tmp[512],state = 'W';
		ofstream ff ;
		
		//pps = new PacketParser();
		//res = new ResParser();

		memset(dir,0,sizeof(dir));
		strcpy(dir,it->second.szSourcePath);

		memset(outFilePath,0,sizeof(outFilePath));  
		strcpy(outFilePath,it->second.szSourcePath);
		strcat(outFilePath,mConfParam.szOutPath);	
		
		memset(szFiletypeIn,0,sizeof(szFiletypeIn));  
		strcpy(szFiletypeIn,it->second.szInFileFmt);	//��ǰ����Դ�������ʽ
		strcpy(m_szSourceID,it->first.c_str());			//��ǰ����ԴID
		
		memset(mServCatId,0,sizeof(mServCatId));
		strcpy(mServCatId,it->second.serverCatID);
/*
		memset(tmp,0,sizeof(tmp));
		strcpy(tmp,fileName);
		strcat(tmp,".tmp");
		if(rename(fileName,tmp))			//�ļ���������ʱ�ļ�, �����Ƿ����쳣������޸Ļ�Դ�ļ���
		{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"�ļ�[%s]����ʧ��: %s",fileName,strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_RENAME_ERR,erro_msg); // �ļ���������

				return -11 ;
		}	
*/
		theJSLog<<"׼�����ļ� ["<<m_szFileName<<"] ���д���"<<endi;
		getCurTime(currTime);

		memset(mdrParam.m_AuditMsg,0,sizeof(mdrParam.m_AuditMsg));

		//������Ϣ���������ļ�	1111 = FILE_ID ��ORACLE�ｨһ��SEQUENCE������SEQ_FILEID����select�ķ�ʽ�õ�
		try
		{	
		
					memset(outFileName,0,sizeof(outFileName));
					sprintf(outFileName,"%s%s",outFilePath,m_szFileName);		//����ļ���,ȫ·��			
				
					//У���Ƿ��ļ��ظ�*******�ظ���������Ŀ¼***********************************************				
					if(checkFileRepeat(m_szFileName))
					{
							strcpy(err_code,"F104");
							sprintf(erro_msg,"�ļ�[%s]�ظ�,���ȱ����Ѿ�����,F104",m_szFileName);					
							theJSLog.writeLog(LOG_CODE_FILE_NAME_REPEAT ,erro_msg);			
							
							memset(sql,0,sizeof(sql));
							getCurTime(currTime);    //��ȡ��ǰ�ļ�ʱ��
							sprintf(sql,"insert into D_ERRFILE_INFO(FILENAME,SOURCE_ID,DEAL_TIME,ERR_MSG,ERR_CODE,ERR_COL,ERR_LINE,ERR_SEQ,STATE) values('%s','%s','%s','%c','%s',%d,%d,%ld,'%c')",m_szFileName,m_szSourceID ,currTime,'F',err_code,0,0,file_id,state);		
							//writeSQL(sql);
							m_vsql.push_back(sql);

							//memset(erro_dir,0,sizeof(erro_dir));
							//strcpy(erro_dir,dir);
							//strcat(erro_dir,erro_path);
							//rename(tmp,strcat(erro_dir,m_szFileName));
							
							sprintf(mdrParam.m_AuditMsg,"%s:%s",m_szFileName,"F104");
							return -1;
					}
							
					//ȡ�ļ����ϵ�ʱ����뵽���ȱ�������������2013-08-01
					memset(file_time,0,sizeof(file_time));
					strncpy(file_time,m_szFileName+it->second.file_begin,it->second.file_length);
					file_time[8] = '\0';

					//ɨ�赽�ļ�����ע��
					memset(sql,0,sizeof(sql));
					sprintf(sql,"insert into %s (SOURCE_ID,FILENAME,RECIEVE_TIME,DEAL_FLAG,RECORD_COUNT,FILE_ID,FILE_TIME,SERV_CAT_ID) values('%s','%s','%s','%c',%d,%ld,'%s','%s')",mConfParam.szSchCtlTabname,m_szSourceID,m_szFileName,currTime,state,0,file_id,file_time,mServCatId);					
					//writeSQL(sql);
					m_vsql.push_back(sql);

					//���ļ�������У��******************************************************
					theJSLog<<"���ļ�������У��..."<<endi;
					if(checkFileName(m_szSourceID,m_szFileName) == CHECK_FAIL)
					{
							//strcpy(err_code,"F101");
							theJSLog.writeLog(LOG_CODE_FILE_NAME_CHECK_VALID,erro_msg);		
							
							memset(sql,0,sizeof(sql));		
							getCurTime(currTime);    //��ȡ��ǰ�ļ�ʱ��
							state = 'E';
							sprintf(sql,"update %s set DEAL_FLAG='%c',DEAL_TIME='%s' where SOURCE_ID='%s' and FILENAME='%s' and DEAL_FLAG = 'W' ",mConfParam.szSchCtlTabname,state,currTime,m_szSourceID,m_szFileName);
							//writeSQL(sql);
							m_vsql.push_back(sql);

							memset(sql,0,sizeof(sql));
							state = 'W';
							sprintf(sql,"insert into D_ERRFILE_INFO(FILENAME,SOURCE_ID,DEAL_TIME,ERR_MSG,ERR_CODE,ERR_COL,ERR_LINE,ERR_SEQ,STATE) values('%s','%s','%s','%c','%s',%d,%d,%ld,'%c')",m_szFileName,m_szSourceID ,currTime,'F',err_code,0,0,file_id,state);
							//writeSQL(sql);
							m_vsql.push_back(sql);
							
							//memset(erro_dir,0,sizeof(erro_dir));
							//strcpy(erro_dir,dir);
							//strcat(erro_dir,erro_path);
							//rename(tmp,strcat(erro_dir,m_szFileName)); 
							
							//sprintf(m_AuditMsg,"F101");
							return -1;
					}
					
					//2013-10-24 �޸Ļ�ȡ���ڷ�ʽ D_CHECK_FILE_DETAIL
					char rate_cycle_tmp[6+1];
					memset(rate_cycle_tmp,0,sizeof(rate_cycle_tmp));
					memset(sql,0,sizeof(sql));
					sprintf(sql,"select rate_cycle from D_CHECK_FILE_DETAIL where source_id = '%s' and check_type = 'AUD' and file_time = '%s' and content = '%s'",m_szSourceID,file_time,m_szFileName);
					Statement stmt = conn.createStatement();
					stmt.setSQLString(sql);
					stmt.execute();
					stmt>>rate_cycle_tmp;
					if(rate_cycle_tmp[0] == '\0')
					{
						theJSLog<<"��D_CHECK_FILE_DETAILû�и��ļ���¼,��ʱ���ļ��������ȡ����"<<endi;
					}
					else
					{
						memset(rate_cycle,0,sizeof(rate_cycle));
						strcpy(rate_cycle,rate_cycle_tmp);
					}

					char cycle_flag;
					memset(sql,0,sizeof(sql));
					sprintf(sql,"select cycle_flag from C_RATE_CYCLE where source_id = '%s' and rate_cycle = '%s'",m_szSourceID,rate_cycle);
					stmt.setSQLString(sql);
					stmt.execute();
					stmt>>cycle_flag;
					if(cycle_flag == 'Y')			//�����ļ�
					{
						strcpy(err_code,"F120");
						sprintf(erro_msg,"[%s]�����ļ���־,F120",m_szFileName);					
						theJSLog.writeLog(LOG_CODE_FILE_NAME_REPEAT ,erro_msg);	
						
						memset(sql,0,sizeof(sql));
						getCurTime(currTime);    //��ȡ��ǰ�ļ�ʱ��
						state = 'E';
						sprintf(sql,"update %s set DEAL_FLAG='%c',DEAL_TIME='%s' where SOURCE_ID='%s' and FILENAME='%s' and DEAL_FLAG = 'W' ",mConfParam.szSchCtlTabname,state,currTime,m_szSourceID,m_szFileName);
						//writeSQL(sql);
						m_vsql.push_back(sql);

						memset(sql,0,sizeof(sql));
						state = 'W';
						sprintf(sql,"insert into D_ERRFILE_INFO(FILENAME,SOURCE_ID,DEAL_TIME,ERR_MSG,ERR_CODE,ERR_COL,ERR_LINE,ERR_SEQ,STATE) values('%s','%s','%s','%c','%s',%d,%d,%ld,'%c')",m_szFileName,m_szSourceID ,currTime,'F',err_code,0,0,file_id,state);
						//writeSQL(sql);
						m_vsql.push_back(sql);

						//memset(erro_dir,0,sizeof(erro_dir));
						//strcpy(erro_dir,dir);
						//strcat(erro_dir,erro_path);
						//rename(tmp,strcat(erro_dir,m_szFileName));

						sprintf(mdrParam.m_AuditMsg,"%s:%s",m_szFileName,"F120");
						return -1;
					}

					//2013-10-17 �����ļ��򿪴��� ������F103
					ifstream in(fileName,ios::nocreate);
					if(!in)
					{					
						strcpy(err_code,"F103");
						sprintf(erro_msg,"�ļ�[%s]�򿪴���:%s,F103",m_szFileName,strerror(errno));					
						theJSLog.writeLog(LOG_CODE_FILE_NAME_REPEAT ,erro_msg);			
						
						memset(sql,0,sizeof(sql));
						getCurTime(currTime);    //��ȡ��ǰ�ļ�ʱ��
						state = 'E';
						sprintf(sql,"update %s set DEAL_FLAG='%c',DEAL_TIME='%s' where SOURCE_ID='%s' and FILENAME='%s' and DEAL_FLAG = 'W' ",mConfParam.szSchCtlTabname,state,currTime,m_szSourceID,m_szFileName);
						//writeSQL(sql);
						m_vsql.push_back(sql);

						memset(sql,0,sizeof(sql));
						state = 'W';
						sprintf(sql,"insert into D_ERRFILE_INFO(FILENAME,SOURCE_ID,DEAL_TIME,ERR_MSG,ERR_CODE,ERR_COL,ERR_LINE,ERR_SEQ,STATE) values('%s','%s','%s','%c','%s',%d,%d,%ld,'%c')",m_szFileName,m_szSourceID ,currTime,'F',err_code,0,0,file_id,state);
						//writeSQL(sql);
						m_vsql.push_back(sql);

						//memset(erro_dir,0,sizeof(erro_dir));
						//strcpy(erro_dir,dir);
						//strcat(erro_dir,erro_path);
						//rename(tmp,strcat(erro_dir,m_szFileName));
							
						sprintf(mdrParam.m_AuditMsg,"%s:%s",m_szFileName,"F103");
						return -1;
					}
					in.close();

					CF_MemFileI _infile;
					_infile.Init(szFiletypeIn); //�����ʽ			
					_infile.Open(fileName);
			
					//���ļ�ͷβ����У��****************************************************
					memset(head,0,sizeof(head));
					memset(tail,0,sizeof(tail));				
					_infile.GetHead(head,MAX_LINE_LENGTH);
					_infile.GetEnd(tail,MAX_LINE_LENGTH);			
		
					record_num  = _infile.Get_recCount();  //����ļ���¼����,��У��

					iTotalNum = 0; 
					if(head[0] != '\0') iTotalNum++	;					//2013-07-15	��ͷ��¼�к�+1
					
					//memset(rate_cycle,0,sizeof(rate_cycle));			//ÿ��У��ͷβʱ�������ÿ�
					
					//theJSLog<<"���ļ�ͷβ����У��..."<<endi;
					if(checkRecorHT(szFiletypeIn,head,tail) == CHECK_FAIL)
					{					
							theJSLog.writeLog(LOG_CODE_FILE_HEAD_TAIL_VALID,erro_msg);		//�������д�����

							getCurTime(currTime);    //��ȡ��ǰ�ļ�ʱ��
							memset(sql,0,sizeof(sql));
							state = 'E';
							sprintf(sql,"update %s set DEAL_FLAG='%c',DEAL_TIME='%s' where SOURCE_ID='%s' and FILENAME='%s' and DEAL_FLAG = 'W' ",mConfParam.szSchCtlTabname,state,currTime,m_szSourceID,m_szFileName);
							//writeSQL(sql);
							m_vsql.push_back(sql);

							memset(sql,0,sizeof(sql));
							state = 'W';
							sprintf(sql,"insert into D_ERRFILE_INFO(FILENAME,SOURCE_ID,DEAL_TIME,ERR_MSG,ERR_CODE,ERR_COL,ERR_LINE,ERR_SEQ,STATE) values('%s','%s','%s','%c','%s',%d,%d,%ld,'%c')",m_szFileName,m_szSourceID ,currTime,'F',err_code,0,0,file_id,state);
							//writeSQL(sql);
							m_vsql.push_back(sql);
							
							//memset(erro_dir,0,sizeof(erro_dir));
							//strcpy(erro_dir,dir);
							//strcat(erro_dir,erro_path);
							//rename(tmp,strcat(erro_dir,m_szFileName));

							return -1;
					}					
					
					if(record_num == 0)
					{
							theJSLog<<"["<<m_szFileName<<"]���ļ����ļ�..."<<endi;
							sprintf(mdrParam.m_AuditMsg,"%s;%s,%d",m_szSourceID,m_szFileName,record_num);
							return ret;
					}

					//������Ϣ�����ļ�
					MessageParser  pMessage; 
					pMessage.setMessage(MESSAGE_NEW_BATCH, it->first.c_str(),m_szFileName,file_id);
					message(pMessage);
					pMessage.setMessage(MESSAGE_NEW_FILE, it->first.c_str(),m_szFileName,file_id);			
					message(pMessage);
				
					pps->clearAll();
					res->clearAll();		

					record_num = 0;
					deal_flag = 0;							//Ĭ���ļ�������Ϊ������
					memset(szBuff, 0, sizeof(szBuff));      //���������		
					memset(erro_msg,0,sizeof(erro_msg));  //����ջ��������Ϣ
	
					//ѭ��ɨ���ļ���¼
				    while(1)  
				    {		
						pps->clear();
						res->clear();

						int iPos = _infile.GetPos();
						pps->setOffset(iPos);

						if( _infile.readRec(szBuff, MAX_LINE_LENGTH) == READ_AT_END )
						{	
							break;									
						}
						record_num++;
		
						pps->setRecord(szBuff);	
						pps->setRcd(outrcd);
						res->setRcd(outrcd);
						res->resetAnaResult();
					
						execute(*pps,*res);

						if(res->getAnaType() != eNormal) 
						{	
							//theJSLog<<"��ʽ����¼����������Դ��"<<m_szSourceID<<"  ԭʼ�ļ�����"<<m_szFileName<<" �ָ����ļ���"<<file_name<<" ��¼λ�ã�"<<record_num<<"	״̬��"<<res->getAnaType()<<ende;
							deal_flag = 1;		
						}
																						
						switch(res->getAnaType())   //���ݸ�ʽ��������жϼ�¼�Ĵ���ʽ
						{
							case eNormal:			//����
								 break;

							case eFmtErr:			//��ʽ����
								 memset(erro_msg,0,sizeof(erro_msg));
								 sprintf(erro_msg,"[%s]��ʽ����:%d[%s|%s]",m_szFileName,record_num,res->getRuleType(),res->getReason());
								 theJSLog.writeLog(LOG_CODE_FORMAT_ERR_RCD ,erro_msg);	
							
								 sprintf(mdrParam.m_AuditMsg,"format err rcd:%d[%s|%s]",record_num,res->getRuleType(),res->getReason());

								 //memset(erro_dir,0,sizeof(erro_dir));
								 //strcpy(erro_dir,dir);							 
								 //strcat(erro_dir,erro_path);
								 //rename(tmp,strcat(erro_dir,m_szFileName)); //���ô����ļ��ƶ�������Ŀ¼��
								 break;

							case eFmtTimeOut:		//��ʽ����ʱ������timeout�ļ�
								 memset(erro_msg,0,sizeof(erro_msg));
								 sprintf(erro_msg,"[%s]��ʽ����ʱ��:%d[%s|%s]",m_szFileName,record_num,res->getRuleType(),res->getReason());
								 theJSLog.writeLog(LOG_CODE_FORMAT_TIMEOUT_RCD,erro_msg);

								 sprintf(mdrParam.m_AuditMsg,"format timeout rcd:%d[%s|%s]",record_num,res->getRuleType(),res->getReason());
								 
								 char timeout_dir[512];
								 memset(timeout_dir,0,sizeof(timeout_dir));
								 strcpy(timeout_dir,dir);			
								 strcat(timeout_dir,mConfParam.szTimeoutPath);
								 rename(fileName,strcat(timeout_dir,m_szFileName)); 
								 break;

							case eFmtOther:			//��ʽ��δ�����ʽ��������otherformat�ļ�
								 memset(erro_msg,0,sizeof(erro_msg));
								 sprintf(erro_msg,"[%s]δ����ĸ�ʽ����:%d[%s|%s]",m_szFileName,record_num,res->getRuleType(),res->getReason());
								 theJSLog.writeLog(LOG_CODE_FORMAT_OTHER_RCD,erro_msg);
								
								 sprintf(mdrParam.m_AuditMsg,"format other rcd:%d[%s|%s]",record_num,res->getRuleType(),res->getReason());
								 
								 char other_dir[512];
								 memset(other_dir,0,sizeof(other_dir));
								 strcpy(other_dir,dir);
								 strcat(other_dir,mConfParam.szOtherPath);
								 rename(fileName,strcat(other_dir,m_szFileName)); 
								 break;

							default :
								theJSLog.writeLog(0,"��ʽ��δ֪�ĵ�");
								sprintf(mdrParam.m_AuditMsg,"format undefine rcd ");
								break;
						}

						if(deal_flag == 1) //��һ����¼�����������ļ���Ҫ,�˳����ļ�
						{
							break;  
						}

						strcpy(szBuff,res->m_outRcd.Get_record());
						record_array.push_back(szBuff);
						memset(szBuff, 0, sizeof(szBuff) );					//���������
					}		
						
					_infile.Close();
						
					if(deal_flag == 1)								//��ʽ���ļ����˴���¼��Ҫ
					{
						record_array.clear();		
						
						//д����ǼǱ� ���µ��ȱ�
						getCurTime(currTime);    //��ȡ��ǰ�ļ�ʱ��
						memset(sql,0,sizeof(sql));
						state = 'E';
						sprintf(sql,"update %s set DEAL_FLAG='%c',DEAL_TIME='%s',RECORD_COUNT=%d where SOURCE_ID='%s' and FILENAME='%s' and DEAL_FLAG = 'W' ",mConfParam.szSchCtlTabname,state,currTime,record_num,m_szSourceID,m_szFileName);
						//writeSQL(sql);
						m_vsql.push_back(sql);

						if(res->getAnaType() == eFmtErr)		
						{
							memset(sql,0,sizeof(sql));
							state = 'W';
							sprintf(sql,"insert into D_ERRFILE_INFO(FILENAME,SOURCE_ID,DEAL_TIME,ERR_MSG,ERR_CODE,ERR_COL,ERR_LINE,ERR_SEQ,STATE) values('%s','%s','%s','%c','%s',%d,%d,%ld,'%c')",m_szFileName,m_szSourceID ,currTime,'F',"F110",0,iTotalNum,file_id,'W');
							//writeSQL(sql);
							m_vsql.push_back(sql);
						}
			
						return -1;
					}			
					
					//д�ļ���ɺ����˽���ڴ�,��д��ʱ�ļ�,���ٲóɹ�����д��ʽ�ļ�
					ret = writeFile(outFileName);
					if(ret)
					{
						theJSLog<<"д�ļ�ʧ��["<<outFileName<<"]"<<endw;
						state = 'E';
					}
					record_array.clear();     								
					
					//struct stat fileInfo;
					memset(tmp,0,sizeof(tmp));
					strcpy(tmp,outFileName);
					strcat(tmp,".tmp");
					stat(tmp,&fileInfo); //��ȡ�ļ���С	
					
					sprintf(mdrParam.m_AuditMsg,"%s;%s,%d,%d",m_szSourceID,m_szFileName,record_num,fileInfo.st_size);

				//������Ϣ�������ļ������ļ�
				pMessage.setMessage(MESSAGE_END_FILE, it->first.c_str(), m_szFileName,file_id);			
				message(pMessage);	
				pMessage.setMessage(MESSAGE_END_BATCH_END_FILES, it->first.c_str(), m_szFileName,file_id);
				message(pMessage);

		}catch(jsexcp::CException e)			
		{			
				rollBackSQL();
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"[%s]dealFile() ��ʽ������ %s",m_szFileName,e.GetErrMessage());
				theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		//�ֶ�ת������
					
				sprintf(mdrParam.m_AuditMsg,"%s;%s",mdrParam.m_AuditMsg,e.GetErrMessage());
				return -11;
		} 
		catch(util_1_0::db::SQLException e)
		{ 
				rollBackSQL();
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"[%s]dealFile() ���ݿ�����쳣:%s (%s)",m_szFileName,e.what(),sql);
				theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣	
					
				sprintf(mdrParam.m_AuditMsg,"%s;%s",mdrParam.m_AuditMsg,e.what());
				return -11;
		}
				  
		return ret ;
}


//����ʽ���ɹ��ļ�¼ ��д��ʱ�ļ�
int FormatPlugin::writeFile(char* filename)
{
		int ret = -1;
		char tmp[1024];
		memset(tmp,0,sizeof(tmp));
		strcpy(tmp,filename);
		strcat(tmp,".tmp");

		ofstream out(tmp);
		if(!out)  
		{	
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"writeFile �ļ�[%s]�򿪳���:%s ",filename,strerror(errno));
			theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//���ļ�ʧ��

			return ret;
		}
		for(int i = 0;i<record_array.size();i++)
		{
			out<<record_array[i]<<"\n";

		}

		out.close();
		//rename(tmp,fileName);

		ret = 0;
		return ret ;
}

int FormatPlugin::dealAuditResult(int result)
{
		int ret = 0;
		char state,tmp[512];
		
		//theJSLog<<"wait dr_audit() ..."<<endi;
		ret = IsAuditSuccess(mdrParam.m_AuditMsg);
		if(ret)										//�ع����ݿ�,�ļ��Ƶ�����Ŀ¼,���ɵ��ļ�ɾ��
		{
			//rollBackSQL();
			m_vsql.clear();

			if(ret != 3)							//2013-11-07�ٲó�ʱ���ƶ��ļ�
			{
				char erro_dir[512];						
				//memset(tmp,0,sizeof(tmp));
				//strcpy(tmp,fileName);
				//strcat(tmp,".tmp");
				memset(erro_dir,0,sizeof(erro_dir));
				strcpy(erro_dir,it->second.szSourcePath);			 
				strcat(erro_dir,mConfParam.szErroPath);

				theJSLog<<"���ļ��Ƶ�ʧ��Ŀ¼:"<<erro_dir<<endi;
				if(rename(fileName,strcat(erro_dir,m_szFileName)))
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"�ƶ�Դ�ļ�������Ŀ¼[%s]ʧ��: %s",erro_dir,strerror(errno));
					theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
				}
			}

			memset(tmp,0,sizeof(tmp));					//ɾ�����ܴ��ڵ������ʱ�ļ�
			strcpy(tmp,outFileName);
			strcat(tmp,".tmp");
			if(access(tmp,F_OK) == 0)
			{
				if(remove(tmp))
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"��ʱ�ļ�[%s]ɾ��ʧ��: %s",outFileName,strerror(errno));
					theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
					ret = -1;
				}
			}		
		}
		else							//�ٲóɹ�,�ύsql ��ʱ����ʽ,�ļ��Ƶ�ʧ�ܻ��Ǳ��ݿ�������
		{		
				if(result)				//���쳣2013-11-01,ɾ����ʱ�ļ�,Ҳ�п������������ʱ�ļ�,û�п����ļ�����ʧ�����
				{
					char erro_dir[512];
					//memset(tmp,0,sizeof(tmp));
					//strcpy(tmp,fileName);
					//strcat(tmp,".tmp");
					memset(erro_dir,0,sizeof(erro_dir));
					strcpy(erro_dir,it->second.szSourcePath);			 
					strcat(erro_dir,mConfParam.szErroPath);
					if(rename(fileName,strcat(erro_dir,m_szFileName)))
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"�ƶ�Դ�ļ�������Ŀ¼[%s]ʧ��: %s",erro_dir,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
					}
					
					memset(tmp,0,sizeof(tmp));
					strcpy(tmp,outFileName);
					strcat(tmp,".tmp");
					if(access(tmp,F_OK) == 0)
					{
						if(remove(tmp))
						{
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"��ʱ�ļ�[%s]ɾ��ʧ��: %s",outFileName,strerror(errno));
							theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
							ret = -1;
						}
					}
					
					if(petri_status == DB_STATUS_ONLINE)
					{
						//m_vsql = getvSQL();
						theJSLog<<"�������ݿ�ǼǱ�״̬..."<<endi;
						updateDB();
					}
					else
					{
						theJSLog<<"���ݿ�״̬Ϊֻ��̬,��ʱдsql�ļ�"<<endi;
						string ssql;
						for(vector<string>::iterator iter = m_vsql.begin();iter != m_vsql.end();++iter)
						{
							ssql = *iter;
							writeSQL(ssql.c_str());
						}
						commitSQLFile();
						m_vsql.clear();
 					}

					return ret;
				}
				else						//��ʽ���ɹ������
				{
					state = 'Y';
					if(record_num)			//�ǿ��ļ�,�����
					{
						memset(tmp,0,sizeof(tmp));
						strcpy(tmp,outFileName);
						strcat(tmp,".tmp");
						stat(tmp,&fileInfo); //��ȡ�ļ���С	

						if(rename(tmp,outFileName))
						{
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"��ʱ�ļ�[%s]������ʽʧ��: %s",tmp,strerror(errno));
							theJSLog.writeLog(LOG_CODE_FILE_RENAME_ERR,erro_msg);
							return -1;
						}
					}
					else
					{
						fileInfo.st_size = 0;
					}
				}
				
				//�ٲóɹ����µ��ȱ�
				memset(sql,0,sizeof(sql));
				getCurTime(currTime);				//��ȡ��ǰ�ļ�ʱ��
				sprintf(sql,"update %s set DEAL_FLAG='%c',DEAL_TIME='%s',RECORD_COUNT=%d,FILESIZE=%ld where SOURCE_ID='%s' and FILENAME='%s' and DEAL_FLAG = 'W'",mConfParam.szSchCtlTabname,state,currTime,record_num,fileInfo.st_size,m_szSourceID,m_szFileName);
				//writeSQL(sql);
				m_vsql.push_back(sql);
				
				if(petri_status == DB_STATUS_ONLINE)
				{
					//m_vsql = getvSQL();
					theJSLog<<"�������ݿ�ǼǱ�״̬..."<<endi;
					updateDB();		
				}
				else
				{
					theJSLog<<"���ݿ�״̬Ϊֻ��̬,��ʱдsql�ļ�"<<endi;
					string ssql;
					for(vector<string>::iterator iter = m_vsql.begin();iter != m_vsql.end();++iter)
					{
						ssql = *iter;
						writeSQL(ssql.c_str());
					}
					commitSQLFile();
					m_vsql.clear();
 				}
		
				//Ҫ����,2013-07-16Ŀ¼����YYYYMM/DD	
				//memset(tmp,0,sizeof(tmp));
				//strcpy(tmp,fileName);
				//strcat(tmp,".tmp");
				
				char bak_dir[512];
				memset(bak_dir,0,sizeof(bak_dir));
				strcpy(bak_dir,it->second.szSourcePath);
				strcat(bak_dir,mConfParam.szBakPath);

				strncat(bak_dir,currTime,6);
				strcat(bak_dir,"/");
				strncat(bak_dir,currTime+6,2);
				strcat(bak_dir,"/");
				if(chkAllDir(bak_dir) == 0)
				{	
					theJSLog<<"�����ļ���Ŀ¼["<<bak_dir<<"]"<<endi;
					strcat(bak_dir,m_szFileName);
					if(rename(fileName,bak_dir))
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"����Դ�ļ�[%s]ʧ��: %s",m_szFileName,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
						ret = -1;
					}
				}
				else
				{	
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"����·��[%s]�����ڣ����޷�����",bak_dir);
					theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg);		//��Ŀ¼����

					ret = -1;
				}		
		}
						
	return ret;
}

//2013-10-23 �����ύsql,��֤һ������������
int FormatPlugin::updateDB()
{	
	int ret = 0;
	Statement stmt;
	char ssql[1024];
    try
    {	
		stmt = conn.createStatement();

		for(vector<string>::iterator iter = m_vsql.begin();iter != m_vsql.end();++iter)
		{	
			memset(ssql,0,sizeof(ssql));
			strcpy(ssql,(*iter).c_str());
			stmt.setSQLString(ssql);
			ret = stmt.execute();
		}

		stmt.close();
	}
	catch(util_1_0::db::SQLException e)
	{ 
		stmt.rollback();
		stmt.close();

		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"[%s] updateDB ���ݿ����:%s (%s),��ʱдsql�ļ�",m_szFileName,e.what(),ssql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//����sqlִ���쳣
	
		ret =  -1;
	}

	m_vsql.clear();
	
	return ret ;
}

//2013-10-11 �����˳�����
void FormatPlugin::prcExit()
{
	int ret = 0;
	if(mdrParam.m_enable) 
	{
		ret = dr_ReleasePlatform();
		if(ret != 0)
		{
			char tmp[100] = {0};
			snprintf(tmp, sizeof(tmp), "�ͷ�����ƽ̨ʧ��,����ֵ=%d", ret);
			theJSLog.writeLog(LOG_CODE_DR_RELEASE_ERR,tmp);
		}
		
		mdrParam.m_enable = false;
	}
	
	pCur_TxtFmtDefine=NULL;
	pCur_Input2Output=NULL;	
	if(pps)
	{
		delete pps;
	}
	if(res)
	{
		delete res;
	}

	PS_Process::prcExit();
}

//���ֳ�ʼ��
bool FormatPlugin::drInit()
{
		//����ģ������ʵ��ID
		char tmp[32];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%ld",getPrcID());

		theJSLog << "��ʼ������ƽ̨,ģ����:"<< module_name<<" ʵ����:"<<tmp<<endi;

		int ret = dr_InitPlatform(module_name,tmp);
		if(ret != 0)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"����ƽ̨��ʼ��ʧ��,����ֵ=%d",ret);
			theJSLog.writeLog(LOG_CODE_DR_INIT_ERR,erro_msg);

			return false;
		}
		else
		{
			theJSLog<<"dr_InitPlatform ok."<<endi;
		}

		mdrParam.m_enable = true;

		mdrParam.drStatus = _dr_GetSystemState();	//��ȡ����ϵͳ״̬
		if(mdrParam.drStatus < 0)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"��ȡ����ƽ̨״̬����,����ֵ=%d",mdrParam.drStatus);
			theJSLog.writeLog(LOG_CODE_DR_GETSTATE_ERR,erro_msg);

			return false;
		}
		
		if(mdrParam.drStatus == 0)		theJSLog<<"��ǰϵͳ����Ϊ��ϵͳ"<<endi;
		else if(mdrParam.drStatus == 1)	theJSLog<<"��ǰϵͳ����Ϊ��ϵͳ"<<endi;
		else if(mdrParam.drStatus == 2)	theJSLog<<"��ǰϵͳ���÷�����ϵͳ"<<endi;

		return true;
}

//��ϵͳ����ͬ������,��ϵͳ��ȡͬ������
int FormatPlugin::drVarGetSet(char* serialString)
{
		int ret  = 0;
		char tmpVar[5000] = {0};
		
		if(!mdrParam.m_enable)
		{
			return ret;
		}

		//�������ƽ̨���л���
		ret = dr_CheckSwitchLock();   
		if(ret != 0)  
		{  
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"��������л���ʧ��,����ֵ=%d",ret);
			theJSLog.writeLog(LOG_CODE_DR_CHECK_LOCK_ERR,erro_msg);

			return -1;  
		} 
		//��ʼ��index  
		ret = dr_InitIDX();  
		if(ret != 0)  
		{  
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"��ʼ��indexʧ��,����ֵ=%d",ret);
			theJSLog.writeLog(LOG_CODE_DR_INIT_IDX_ERR,erro_msg);

			//dr_AbortIDX();
			return -1;  
		}

/*
		//��ϵͳ�����ļ�����·�����ļ��� ֻ������ƽ̨���Ը�֪,��ϵͳ�޷�ʶ��
		if(drStatus != 1)
		{
			snprintf(tmpVar, sizeof(tmpVar), "%s%s", it->second.szSourcePath,input_path);
			ret = dr_SyncIdxVar("@@CHECKPATH", tmpVar,SYNC_SINGLE);  
			if(ret != 0)
			{
				theJSLog<<"�����ļ�����·��ʧ��,�ļ�·��["<<input_path<<"]"<<endw;
				dr_AbortIDX();
				return -1;
			}
			
			snprintf(tmpVar, sizeof(tmpVar), "%s", m_szFileName);
			ret = dr_SyncIdxVar("@@CHECKFILE", tmpVar,SYNC_SINGLE);  
			if(ret != 0)
			{
				theJSLog<<"�����ļ�ʧ��,�ļ���["<<m_szFileName<<"]"<<endw;
				dr_AbortIDX();
				return -1;
			}

			cout<<"�����ļ�·��:"<<input_path<<" �ļ���:"<<m_szFileName<<endl;
		}
*/
		snprintf(tmpVar, sizeof(tmpVar), "%s", serialString);
		//��ϵͳ��Ҫͬ����index ����ֵ�ԡ�д������ƽ̨ά����index �ļ���
		//��ϵͳ���øú����Ľ���ǣ�var��ú���ϵͳһ�������������ֵ��	SYNC_SINGLE��ʾע�ᵥһ���������
		ret = dr_SyncIdxVar("serialString", tmpVar, SYNC_SINGLE);		
		if (ret != 0)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"�����д�ʱʧ�ܣ�������:%s",serialString);
			theJSLog.writeLog(LOG_CODE_DR_SYSC_IDXVAR_ERR,erro_msg);

			dr_AbortIDX();
			return -1;
		}
		if(mdrParam.drStatus == 1)
		{
			//serialString = tmpVar;			//ͬ�������ַ���,��ϵͳ�Ǹ�ֵ����ϵͳ��ȡֵ
			strcpy(serialString,tmpVar);
			//m_AuditMsg = tmpVar;			//Ҫ�ٲõ��ַ���
		}

		theJSLog<<"���ε�ͬ����serialString:"<<serialString<<endi;//for test

		// <5> ����ʵ����  ������ϵͳע���IDX��ģ����ò�����
		//��ϵͳ��index manager���IDX��������󣬰�ʹ�øú���ע������������Ϊģ��ĵ��ò���trigger��Ӧ�Ľ���
		snprintf(tmpVar, sizeof(tmpVar), "%d", getPrcID());
		ret = dr_SyncIdxVar("@@ARG", tmpVar,SYNC_SINGLE);  
		if(ret !=0)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"����ʵ����ʧ��:%s",tmpVar);
			theJSLog.writeLog(LOG_CODE_DR_SYSC_IDXVAR_ERR,erro_msg);

			dr_AbortIDX();  
			return -1;
		}
		
		
		// <6> Ԥ�ύindex  �˹ؼ������ڽ�ƽ̨��ǰ�ڴ��е��������д�����
		ret = dr_SyncIdxVar("@@FLUSH","SUCCESS",SYNC_SINGLE);  
		if (ret != 0 )
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"Ԥ�ύindexʧ��");
			theJSLog.writeLog(LOG_CODE_DR_SYSC_IDXVAR_ERR,erro_msg);

			dr_AbortIDX();
			return -1;
		}
		
		
		// <7> �ύindex  	�ύIndex����index�ļ���������ɱ�־
		ret = dr_CommitIDX();  
		if(ret != 0)  
		{  
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"�ύindexʧ��,����ֵ=%d",ret);
			theJSLog.writeLog(LOG_CODE_DR_COMMIT_IDX_ERR,erro_msg);

			dr_AbortIDX();  
			return -1;  
		}

		//��ϵͳ����Ŀ¼
		//if(!m_syncDr.isMaster())thelog<<"��ϵͳSerialString��"<<m_SerialString<<endi;

		return ret;

}

//�ٲ��ַ���
 int FormatPlugin::IsAuditSuccess(const char* dealresult)
 {
		int auitStatus = 0, ret = 0;

		if(!mdrParam.m_enable)
		{
			return ret;
		}
		
		theJSLog<<"wait dr_audit() ..."<<endi;
		ret = dr_Audit(dealresult);
		if(2 == ret )
		{
			//theJSLog << "�����ٲ�ʧ��,���:" << ret <<"���ˣ�"<<dealresult<< endw;
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"�����ٲ�ʧ��,���:%d,����:%s",ret,dealresult);
			theJSLog.writeLog(LOG_CODE_DR_AUDIT_ERR,erro_msg);

			dr_AbortIDX();
		}
		else if (3 == ret)
		{
			theJSLog<<"�����ٲó�ʱ..."<<endw;
			dr_AbortIDX();
		}
		else if(4 == ret)
		{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"�Զ�idx�쳣��ֹ");
			theJSLog.writeLog(LOG_CODE_DR_IDX_STOP_ERR,erro_msg);

			dr_AbortIDX();
		}
		else if(1 == ret)
		{
			ret = dr_CommitSuccess();
			if(ret != 0)
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"ҵ��ȫ���ύʧ��(����ƽ̨),����ֵ=%d",ret);
				theJSLog.writeLog(LOG_CODE_DR_COMMIT_SUCCESS_ERR,erro_msg);

				//dr_AbortIDX();
			}
			theJSLog<<"ret = "<<ret<<"�ٲóɹ�...\n�ٲ����ݣ�"<<dealresult<<endi;
		}
		else
		{
			theJSLog<<"δ֪ret="<<ret<<"	�ٲ����ݣ�"<<dealresult<<endw;
			dr_AbortIDX();
		}
	
	return ret;
 }

bool FormatPlugin::CheckTriggerFile()
{
	int ret = 0;
	if(access(m_triggerFile.c_str(),F_OK) != 0)	return false;

	theJSLog<< "��鵽trigger�ļ�����ɾ��"<< m_triggerFile <<endi;

	ret = remove(m_triggerFile.c_str());	
	if(ret) 
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"ɾ��trigger�ļ�[%s]ʧ��: %s",m_triggerFile,strerror(errno));
		theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
		return false;
	}
	
	return true;
}
