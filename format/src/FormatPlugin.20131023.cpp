/******************************************************************
	Copyright (c) 2012, GUANGDONG YOSON TECHNOLOGY CO., LTD
	All rights reserved.

	Created:		2013/2/25
	Filename: 		FormatPlugin.cpp
	Description:	结算六期系统的格式化文件类
					格式化文件，写话单块

	History:
	<table>
		Revision	Author			Date		Description
		--------	------			----		-----------
		v1.0		hed		 2013/3/22	       完成程序初稿
		v2.0		hed		 2013/3/31		   将格式化操作放到申请话单块之前执行好
		v3.0        hed		 2013/4/9		   支持多进程格式化文件
		v4.0		hed		 2013/4/17		   格式化成功与否都要到数据库中登记信息 
	</table>
*******************************************************************/
/*
84：当前纪录在文件中的记录序号
85：文件名上的某个字符串：根据环境变量FMT_TIME_INFNAME截取，原定为文件名上时间字段
87: 文件名上的某个字符串：根据环境变量FMT_FIELD_INFNAME截取
88: 话单方向：      （MsgDirection）短信网间结算和短信与cp结算系统有效
90: 整个输入话单：  （Record）--经压缩为定长32字节；在输出记录格式中存在字段名为Record的字段时有效，（可在source_env表定义该字段组合）
91: 输入话单长度：  （Record_Len）
92：异常单类型      (Abn_type)
94：假子冠话单标志  (Fake_Header_Flag)
99: 数据源代码      (source_id);
100:本地网缩写      (localnet_abbr);
*/
//时间格式之类的长度不一致。

#include "FormatPlugin.h"
#define THROW_EXCEPTION(iErrorCode, strErrorMsg) {throw jsexcp::CException( iErrorCode, strErrorMsg, __FILE__, __LINE__); }
#define MAX_CHLD_PRC_NUM  10
typedef struct
{
	pid_t pid;
	int m_lPrc_ID;     //增加进程id add by hed 2013-4-9
	char  state;       // PRCST_STARTING PRCST_IDLE PRCST_BUSY PRCST_STOPPING PRCST_INACTV
}CHLD_PRC_INFO;
CHLD_PRC_INFO g_stChldPrcInfo[MAX_CHLD_PRC_NUM];
queue<int> task;

int  g_iChldPrcNum;
bool gbNormal=true;

struct stat fileInfo;

//PacketParser pps;			//??为什么放到成员变量就要挂啊
//ResParser res;

extern char Field[MAX_FIELD_COUNT][FMT_MAX_FIELD_LEN];

FormatPlugin::FormatPlugin()
{
	initflag=0;
	m_bSuspSig=false;
	
	m_enable = false;
	
	memset(fileName,0,sizeof(fileName));
	//memset(file_name,0,sizeof(file_name));
	memset(input_path,0,sizeof(input_path));
	memset(output_path,0,sizeof(output_path));
	memset(timeout_path,0,sizeof(timeout_path));
	memset(other_path,0,sizeof(other_path));
	memset(bak_path,0,sizeof(bak_path));
	memset(err_code,0,sizeof(err_code));
	memset(erro_msg,0,sizeof(erro_msg));
	memset(outFileName,0,sizeof(outFileName));
	memset(sql,0,sizeof(sql));
	//file_num  = 0;
	source_file_num = 0;
}

FormatPlugin::~FormatPlugin()
{
	//m_Shm = NULL; 
	//m_pArgv=NULL;
	//m_pTktBlkBase=NULL;
	//m_DBConn=NULL;
	pCur_TxtFmtDefine=NULL;
	pCur_Input2Output=NULL;
	//if(conn != NULL)  conn.close();
	conn = NULL;

	if(m_enable) 
	{
		int ret = dr_ReleasePlatform();
		if(ret != 0)
		{
			char tmp[100] = {0};
			snprintf(tmp, sizeof(tmp), "释放容灾平台失败,返回值=%d", ret);
			theJSLog<<tmp<<endw;
		}
	}
}

/********格式化插件自带函数*******************************************************/
void FormatPlugin::execute(PacketParser& ps,ResParser& retValue)
{
	char szLogStr[LOG_MSG_LEN+1];							//日志信息
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

	//cout<<"记录序号： "<<iTotalNum<<" 偏移量："<<ps.getOffset()<<"  记录长度："<<strlen(szRecord)<<endl;

    pCur_TxtRecFmt=m_txtformat.Get_CurTxtRecordFmt(szRecord);
    if(!pCur_TxtRecFmt)
    {
	  SetAbnOutCdr(eFmtOther,iErrorType,inrcd,ps,retValue);
//写otherformat
      return ;
    }
    //cout<<"iErrorType1："<<iErrorType<<endl;

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
    //考虑返回字段序号//考虑不校验入口长度
      iErrorType=inrcd->Set_record(szRecord,pCur_TxtRecFmt->iRecord_Len);
    }
    catch(jsexcp::CException e)
    {
      iErrorType=-1;
    }
   // cout<<"iErrorType2："<<iErrorType<<endl;

/*
	if(iErrorType) 
	{
		SetAbnOutCdr(eFmtErr,iErrorType,inrcd,ps,retValue);
		return ;
	}
*/
    Get_TxtCalledNo(inrcd);
//取整张话单，压缩后在转成文本形式存放
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
      transfer( Field[MAX_FIELD_COUNT-11],16,szDulRtn);//压缩后固定16个字节
      sprintf(Field[MAX_FIELD_COUNT-11],"%s",szDulRtn);
      Field[MAX_FIELD_COUNT-11][32]=0;
    }
  

    //***********************************************************
    // 校验字段
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
	      //跳出

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
    //cout<<"iErrorType3："<<iErrorType<<endl;
    //***********************************************************
    // 在这里添加代码对话单进行处理并置iResult的值
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

    iRet=m_txtformat.Set_OutRcd(*inrcd,outrcd);
    if(iRet) iErrorType = iRet;
    //cout<<"iErrorType4："<<iErrorType<<endl;
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
    //cout<<"iErrorType5："<<iErrorType<<endl;
    
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
    //cout<<"到尾部喽：outrcd = "<<outrcd.Get_record()<<endl;
	retValue.m_outRcd.Copy_Record(outrcd);
	return ;

}

//子进程在发送消息时要调用数据库操作
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
	  theJSLog<<"InsTimeBetweenFile时操作数据库失败:"<<e.what()<<endi;
	  throw jsexcp::CException(0, "InsTimeBetweenFile时操作数据库失败", __FILE__, __LINE__);
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
                /*trans_table[i*16+j][0]  = 0x20;大于等于10的数值均置为空格*/
                trans_table[i*16 + j][0] = 0x41 + i - 10; /*updated on 2001.3.23, 更改后BCD与16进制编码相同*/
                if (i >= 10)
                    trans_tablex[i*16 + j][0] = 0x41 + i - 10;
                else
                {
                    trans_table[i*16 + j][0] = 0x30 + i;
                    trans_tablex[i*16 + j][0] = 0x30 + i;
                }
                /*trans_table[i*16+j][0]  = 0x20;大于等于10的数值均置为空格*/
                trans_table[i*16 + j][1] = 0x41 + j - 10; /*updated on 2001.3.23, 更改后BCD与16进制编码相同*/
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

//父进程加载数据源配置信息时会调用数据库操作
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
	memset(Value,0,sizeof(Value));		//2013-07-18必须初始化 否则判断有误

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
		theJSLog<<"getFromSrcEnv抛出异常: "<<e.what()<<endi;
		throw jsexcp::CException(0, "getFromSrcEnv抛出异常：", __FILE__, __LINE__);		
  }
	
	
	delSpace(Value,0);	
	return 0;
}/*end of getFromSrcEnv*/

/**
 *根据数据源组从C_SOURCE_GROUP_DEFINE 找文件类型FILETYPE_ID，
 *根据FILETYPE_ID从表c_filetype_define找出记录类型record_type，记录长度record_len，
 *根据FILETYPE_ID从c_txtfile_fmt找出记录格式的全部信息
 *在根据FILETYPE_ID从表C_SOURCE_GROUP_CONFIG找数据源ID数目
 *根据数据源ID从表C_FILE_RECEIVE_ENV找到过滤规则
 *数据源信息全部放到了Map中变量m_SourceCfg
 *CFmt_Change类变量outrcd也存储了关于许多文件记录类型格式字段等信息
 */
int FormatPlugin::LoadSourceCfg()
{
	//CBindSQL ds(*m_DBConn);
	char szSqlStr[400];
	int iSourceCount=0;

//生成线表TP_BILLING_LINE中通过数据源组SOURCE_GROUP去表C_SOURCE_GROUP_DEFINE中找到文件类型FILETYPE_ID
//作为格式化输出类型 add by hed  2013-03-11
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

   	expTrace(szDebugFlag, __FILE__, __LINE__,"%s;%s",m_szSrcGrpID,m_szOutTypeId);

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
    expTrace(szDebugFlag, __FILE__, __LINE__, 
      "iSourceCount=%d;", iSourceCount);
    
	
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

	  if(getFileNameFmt(strSourceId) == -1)   return -1;      //根据数据源获取文件名格式

	  if(getRecordHTFilter(SourceCfg.szInFileFmt) == -1)  return -1;     //根据文件类型获取文件记录头尾格式，多个数据源可能对于同一种文件格式

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
            	//退出
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

//从文件名上取字段
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
	//找第二个分隔符
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
	//找第三个分隔符
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

//20071026,取文件名上的时间,作为话务量统计用

//从文件名上取字段
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
	//找第二个分隔符
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
	//找第三个分隔符
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


//回流
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
		cout<<"LoadSourceCfg数据库出错："<<e.what()<<endl;
		throw jsexcp::CException(0, "LoadSourceCfg加载数据库出错：", __FILE__, __LINE__);
	}
}

/******根据数据源获取过滤规则 0没有查到规则，1查到规则了 2013-08-01增加获取文件名上时间的起始位置,和长度*********************/
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
				sprintf(erro_msg,"数据源[%s]没有配置过滤规则或者文件名时间截取规则",source);
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
			sprintf(erro_msg,"数据源[%s]文件名时间截取规则配置规则错误:%s  [如3,8]",source,file_time);
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
			sprintf(erro_msg,"数据源[%s]文件名时间截取规则配置规则错误:%s  [如3,8]",source,file_time);
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
		sprintf(erro_msg,"getSourceFilter 数据库查询异常: %s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);
		return -1 ;
	}
	catch(jsexcp::CException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"getSourceFilter 字段转化出错：%s",e.GetErrMessage());
		theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);
		return -1;
	}
	
	return 0;
}

//获取文件名的格式，根据数据源来获取  0表示咩有配置格式
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
			fmt.index--;	//2013-09-04 从1开始
			vfmt.push_back(fmt);
			count++;
		}
		

		mapFileNameFmt.insert(map< string,vector<FileNameFmt> >::value_type(source,vfmt));

		stmt.close();

	}catch(SQLException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"getFileNameFmt 数据库查询异常: %s [%s]",e.what(),sql);
		theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);

		return -1 ;
	}
	
	return 0; 
}

//获取文件记录的头尾格式，根据文件类型来获取
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
			fmt.index--;	//2013-09-04 从1开始

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
		
		//2013-09-04 考虑该种文件类型是否需要判断已经在map里面了,因为有可能多个数据源对应同一种文件类型?

		//头尾分开保存	
		mapFileRecordHeadFmt.insert(map< string,vector<RecordHTFmt> >::value_type(fileType,vfmta));		
		mapFileRecordTailFmt.insert(map< string,vector<RecordHTFmt> >::value_type(fileType,vfmtb));			

		//mapFileRecordHTFmt.insert(map< string,vector<RecordHTFmt> >::value_type(fileType,vfmt));

		stmt.close();

	}catch(SQLException e)
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"getRecordHTFilter 数据库查询异常: %s [%s]",e.what(),sql);
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

 //拿到子进程中初始化，避免数据库交叉 已修改
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
			//iTotalNum = 0;		//2013-09-08由于文件头校验在前面,所以初始化放在校验处
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
		//加MsgDirection
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
			//加从文件名上取得字段
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
	
	//PS_Process::setSignalPrc();	//设置中断

	if(!drInit())  return false;

/*********获取格式化插件初始化所需要的数据源组,service*** 2013-03-11 add by hed*********************************************/
	if(!(dbConnect(conn)))
	{
		cout<<"数据库 connect error."<<endl;
		return false ;
	}

	int flow_id = getFlowID();
	int module_id = getModuleID();
	char sourceGroup[8],service[8]; 
	memset(sourceGroup,0,sizeof(sourceGroup));
	memset(service,0,sizeof(service));

	try{

		string sql = "select source_group from TP_BILLING_LINE  where billing_line_id = :1";
		Statement stmt = conn.createStatement();
		stmt.setSQLString(sql);
		stmt<<flow_id;
		stmt.execute();
		if(!(stmt>>sourceGroup))
		{
			cout<<"请在tp_billing_line表中配置数据源组!"<<endl;
			return false ;
		}

		sql = "select ext_param from TP_PROCESS where billing_line_id = :1 and module_id = :2";
		stmt.setSQLString(sql);
		stmt<<flow_id<<module_id;
		stmt.execute();
		if(!(stmt>>service))
		{
			cout<<"请在tp_process表中字段ext_param配置service"<<endl;
			return false ;
		}

		sql = "select c.input_path,c.output_path from C_SOURCE_GROUP_DEFINE a,C_SERVICE_FLOW b,C_SERVICE_INTERFACE c where a.source_group=:1" 
                    "and a.workflow_id = b.workflow_id and b.service=:2 and b.input_id = c.interface_id ";
		stmt.setSQLString(sql);
		stmt<<sourceGroup<<service;
		stmt.execute();
		if(!(stmt>>input_path>>output_path))
		{
			cout<<"格式化输入输出文件相对路径没有配置,请在work_flow_id中配置"<<endl;
			return false ;
		}	
		completeDir(input_path);
		completeDir(output_path);

		sql = "select var_value from c_process_env where varname = 'FMT_ERR_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<sourceGroup<<service;
		stmt.execute();
		if(!(stmt>>erro_path))
		{
			cout<<"格式化错误文件路径变量[FMT_ERR_DIR]没有在c_process_env中配置"<<endl;
			return false; 
		}
		completeDir(erro_path);
		
		sql = "select var_value from c_process_env where varname = 'FMT_TIMEOUT_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<sourceGroup<<service;
		stmt.execute();
		if(!(stmt>>timeout_path))
		{
			cout<<"格式化超时文件路径变量[FMT_TIMEOUT_DIR]没有在c_process_env中配置"<<endl;
			return false ;
		}
		completeDir(timeout_path);
	
		sql = "select var_value from c_process_env where varname = 'FMT_OTHER_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<sourceGroup<<service;	
		stmt.execute();
		if(!(stmt>>other_path))
		{
			cout<<"格式化未定义格式话单文件路径[FMT_OTHER_DIR]没有在c_process_env中配置"<<endl;
			return false ;
		}
		completeDir(other_path);

		
		sql = "select var_value from c_process_env where varname = 'FMT_BACKUP_DIR' and source_group=:1 and service=:2 ";
		stmt.setSQLString(sql);
		stmt<<sourceGroup<<service;
		stmt.execute();
		if(!(stmt>>bak_path))
		{
			cout<<"请在表c_process_env配置格式化模块的备份目录:FMT_BACKUP_DIR"<<endl;
			return false;
		}
		completeDir(bak_path);

		//新增SQL路径出错 中文字符乱码

		}catch(SQLException e)
		{
			cout<<"init 数据库查询异常:"<<e.what()<<endl;
			return false ;
		}
	
	
	//新增可以配置每次扫描数据源目录下面指定个数文件后调到下个数据源
	char sParamName[256];
	CString sKeyVal;

	memset(sParamName,0,sizeof(sParamName));
	sprintf(sParamName, "business.source.file.%d.num",getFlowID());
	if(param_cfg.bGetMem(sParamName, sKeyVal))
	{
		source_file_num = sKeyVal.toInteger();
	}
	else
	{	
		cout<<"请在配置文件中配置流水线["<<flow_id<<"]中数据源每次扫描文件的个数,参数名:"<<sParamName<<endl;
		return false ;
	}	


	bool bb = initializeLog(argc,argv,false);  //是否调试模式
	if(!bb)
	{
			return false;
	}
	theJSLog.setLog(szLogPath, szLogLevel,service , sourceGroup, 001);
	
	//日志初始路径： log/run_log/年月/sourceGroup/service 日志文件按天登记
	theJSLog<<"  数据源组:"<<sourceGroup<<"  service:"<<service<<"  相对输入路径:"<<input_path<<" 相对输出路径:"<<output_path<<"  错误文件路径:"<<erro_path<<" 超时文件路径:"<<timeout_path<<" 格式化未定义话单文件路径:"<<other_path<<"备份文件路径:"<<bak_path<<"	日志路径:"<<szLogPath<<"  日志级别:"<<"  SQL存放路径:"<<sql_path<<endi;

	try
	{
			/*调用格式化插件的初始化函数,加载数据源信息*/
			init(sourceGroup,service,0);  
	}
	catch(SQLException e)
	{
		  memset(erro_msg,0,sizeof(erro_msg));
		  sprintf(erro_msg,"格式化插件的初始化init() sql err : %s (%s)",e.what(),sql);
		  theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//sql执行失败

		  conn.close();
		  return false ;
	}
	catch(jsexcp::CException &e)
	{
			memset(erro_msg,0,sizeof(erro_msg));
			sprintf(erro_msg,"格式化插件的初始化init() err : %s",e.GetErrMessage());
			theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		//字段转换出错

			conn.close();
			exit(-1);
			//return false;		//内存没有释放，程序挂死，目前强制退出
	} 
	
	conn.close();

	DIR *dirptr = NULL; 
	char input_dir[512],output_dir[512],erro_dir[512],timeout_dir[512],other_dir[512],bak_dir[512];
	int rett = -1;

	//获取话单文件路径
	for(map<string,SOURCECFG>::const_iterator iter = m_SourceCfg.begin();iter !=m_SourceCfg.end(); ++iter)
	{		   		
			memset(input_dir,0,sizeof(input_dir));
			strcpy(input_dir,iter->second.szSourcePath);
			strcat(input_dir,input_path);
			if((dirptr=opendir(input_dir)) == NULL)
			{
					 memset(erro_msg,0,sizeof(erro_msg));
					 sprintf(erro_msg,"数据源[%s]的输入文件路径[%s]不存在",iter->first,input_dir);
					 theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
					 return false ;
			}else closedir(dirptr);
			
			memset(output_dir,0,sizeof(output_dir));
			strcpy(output_dir,iter->second.szSourcePath);
			strcat(output_dir,output_path);
			if((dirptr=opendir(output_dir)) == NULL)
			{		 
					theJSLog<<"数据源【"<<iter->first<<"】的输出文件路径: "<<output_path<<"不存在，自行创建"<<endw;
					rett = mkdir(output_dir,0755);
					if(rett == -1)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"数据源[%s]的输出话单文件文件路径[%s]不存在，自行创建失败",iter->first,output_path);
						theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
						return false;
					}

			}else closedir(dirptr);

			memset(erro_dir,0,sizeof(erro_dir));
			strcpy(erro_dir,iter->second.szSourcePath);
			strcat(erro_dir,erro_path);
			//struct stat fileStat; if ((stat(dir.c_str(), &fileStat) == 0) && S_ISDIR(fileStat.st_mode))
			if((dirptr=opendir(erro_dir)) == NULL)
			{
					theJSLog<<"数据源【"<<iter->first<<"】的错误文件路径: "<<erro_dir<<"不存在，自行创建"<<endw;
					rett = mkdir(erro_dir,0755);
					if(rett == -1)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"数据源[%s]的错误话单文件文件路径[%s]不存在，自行创建失败",iter->first,erro_dir);
						theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
						return false;
					}
			}else closedir(dirptr);
			
			memset(timeout_dir,0,sizeof(timeout_dir));
			strcpy(timeout_dir,iter->second.szSourcePath);		
			strcat(timeout_dir,timeout_path);
			if((dirptr=opendir(timeout_dir)) == NULL)
			{
					theJSLog<<"数据源【"<<iter->first<<"】的超时文件路径: "<<timeout_dir<<"不存在，自行创建"<<endw;
					rett = mkdir(timeout_dir,0755);
					if(rett == -1)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"数据源[%s]的超时话单文件文件路径[%s]不存在，自行创建失败",iter->first,timeout_dir);
						theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
						return false;
					}
			
			}else  closedir(dirptr);
			
			memset(other_dir,0,sizeof(other_dir));
			strcpy(other_dir,iter->second.szSourcePath);
			strcat(other_dir,other_path);
			if((dirptr=opendir(other_dir)) == NULL)
			{
					theJSLog<<"数据源【"<<iter->first<<"】的未定义格式话单文件路径: "<<other_dir<<"不存在，自行创建"<<endw;
					rett = mkdir(other_dir,0755);
					if(rett == -1)
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"数据源[%s]的未定义格式话单文件文件路径[%s]不存在，自行创建失败",iter->first,other_dir);
						theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
						
						return false;
					}		

			}else  closedir(dirptr);

			memset(bak_dir,0,sizeof(bak_dir));
			strcpy(bak_dir,iter->second.szSourcePath);
			strcat(bak_dir,bak_path);
			if((dirptr=opendir(bak_dir)) == NULL)
			{
				//memset(erro_msg,0,sizeof(erro_msg));
				//sprintf(erro_msg,"数据源[%s]的备份文件路径[%s]不存在",iter->first,bak_dir);
				//theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
				
				theJSLog<<"数据源【"<<iter->first<<"】的话单文件备份路径: "<<bak_dir<<"不存在，自行创建"<<endw;
				rett = mkdir(bak_dir,0755);
				if(rett == -1)
				{
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"数据源[%s]的话单文备份路径[%s]不存在，自行创建失败",iter->first,bak_dir);
					theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
						
					return false;
				}	

			}else  closedir(dirptr);

	}
	

	theJSLog<<"格式化初始化完毕！\n"<<endi;

	return true;
}


//校验文件是否重复  >=1表示重复 0表示不重复，
int FormatPlugin::checkFileRepeat(char* file)
{
	string sql = "select count(*) from D_SCH_FORMAT where source_id = :1 and filename = :2 ";
	int count = 0;
	Statement stmt = conn.createStatement();
	stmt.setSQLString(sql);
	stmt<<m_szSourceID<<file;
	stmt.execute();
	stmt>>count;
	if(count)	//若从调度表找到后 再查询异常登记表,若再找到则不判断重,否则重了
	{
		sql = "select count(*) from D_ERRFILE_INFO where source_id = :1 and filename = :2 ";
		stmt.setSQLString(sql);
		stmt<<m_szSourceID<<file;
		stmt.execute();
		stmt>>count;

		if(count)  return 0;   
		else	   return 1;
	}

	stmt.close();

	return 0;
}


//校验文件名格式是否正确  0表示校验失败，1表示校验成功
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
		strncpy(tmp,fileName+fmt.index,fmt.len);		//通过截取文件名的位置

		//规则校验 
		switch(fmt.check_type)
		{
			case 'C' :            //常量校验
				  
				  if(strcmp(fmt.check_value,tmp) != 0)
				  {
					  strcpy(err_code,fmt.err_code);
					  sprintf(m_AuditMsg,"%s:%c:%s",tmp,fmt.check_type,fmt.err_code);

					  memset(erro_msg,0,sizeof(erro_msg));
					  sprintf(erro_msg,"文件名校验:常量校验失败,[%s != %s]",tmp,fmt.check_value);
					  return CHECK_FAIL;
				  }

				  break;
		
			case 'Z' :			 //日期格式校验支持6位 8位 10位 12位 14位
				   
				  len = strlen(tmp);
				  if(len != strlen(fmt.check_value))  //要校验的日期字符串截取的长度和设置的日期格式不一样，肯定校验失败
				  {
					    strcpy(err_code,fmt.err_code);
						sprintf(m_AuditMsg,"%s:%c:%s",tmp,fmt.check_type,fmt.err_code);

						memset(erro_msg,0,sizeof(erro_msg));
					    sprintf(erro_msg,"文件名校验:要校验的日期字符串截取的长度和设置的日期格式不一致, [%s != %s]",tmp,fmt.check_value);
						return CHECK_FAIL;
				  }
				  
				  for(int j = 0;j<len;j++)
				  {
						if(tmp[j] > '9' || tmp[j] < '0')
						{
							strcpy(err_code,fmt.err_code);
							sprintf(m_AuditMsg,"%s:%c:%s",tmp,fmt.check_type,fmt.err_code);

							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"文件名校验： 日期校验之日期校验失败, 日期必须全为数字 [%s] ",tmp);
							return CHECK_FAIL;
						}
				  }

				  if(len == 6||len == 8||len ==10||len ==12||len ==14)
				  {
						if(checkDate(tmp) == CHECK_FAIL)
						{
								strcpy(err_code,fmt.err_code);
								sprintf(m_AuditMsg,"%s:%c:%s",tmp,fmt.check_type,fmt.err_code);

								memset(erro_msg,0,sizeof(erro_msg));
								sprintf(erro_msg,"文件名校验:日期时间不合法,[%s]",tmp);
								return CHECK_FAIL;
						}
				  }
				  else 
				  {
					  strcpy(err_code,fmt.err_code);
					  sprintf(m_AuditMsg,"%s:%c:%s",tmp,fmt.check_type,fmt.err_code);

					  memset(erro_msg,0,sizeof(erro_msg));
					  sprintf(erro_msg,"文件名校验:日期格式校验失败,长度必须为6 8 10  12 14,[%s]",tmp);
					  return CHECK_FAIL;
				  }

				  break;

			case 'D' :          //数字校验 
				  
				  for(int j = 0;j<strlen(tmp);j++)
				  {
						if(tmp[j] > '9' || tmp[j] < '0')
						{
							strcpy(err_code,fmt.err_code);
							sprintf(m_AuditMsg,"%s:%c:%s",tmp,fmt.check_type,fmt.err_code);

							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"文件名校验:数字校验失败,[%s]",tmp);
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

//校验文件记录头尾是否正确
int FormatPlugin::checkRecorHT(char* fileType,char* head,char* tail)
{
	cout<<"话单文件类型："<<fileType<<"  头记录："<<head<<"  尾记录："<<tail<<endl;
	int rett = CHECK_SUCCESS,len = 0,flag = 0;
	char tmp[1024],erro_tmp[1024];
	string str;
	str.append(fileType);
	
	RecordHTFmt fmt;
	memset(erro_msg,0,sizeof(erro_msg));
	memset(erro_tmp,0,sizeof(erro_tmp));

	if(head[0] != '\0')						//检验头
	{	
		map< string,vector<RecordHTFmt> >::const_iterator iter1 = mapFileRecordHeadFmt.find(str);
		if(iter1 == mapFileRecordHeadFmt.end())
		{		
			return CHECK_SUCCESS;
		}

		vector<RecordHTFmt> vfmta = iter1->second;
		vector<string> coloum_value_head;
		
		//cout<<"规则表配置的文件记录头的字段个数"<<vfmta.size()<<endl;
		flag = 0;
		if(vfmta[0].seperator != '')
		{
			splitString(head,vfmta[0].seperator,coloum_value_head,false,false);  //不跳过空串
			//cout<<" 分隔符配置取字段 从记录中获取的 头字段个数："<<coloum_value_head.size()<<endl;
			flag = 1;
		}	

		for(int i = 0;i<vfmta.size();i++)
		{
			fmt = vfmta[i] ;
			memset(tmp,0,sizeof(tmp));
		
			if(flag)             //表示通过分隔符获取字段值
			{
				strcpy(tmp,coloum_value_head[vfmta[i].number-1].c_str());
			}
			else
			{
				strncpy(tmp,head+fmt.index,fmt.len);
			}

			//校验类型
			if( checkRule(fmt,tmp) == CHECK_FAIL) 
			{	
				strcpy(erro_tmp,"文件记录头校验:");
				strcat(erro_tmp,erro_msg);
				strcpy(erro_msg,erro_tmp);

				strcpy(err_code,fmt.err_code);		//字段I的值在做某种类型的校验失败
				sprintf(m_AuditMsg,"%s:%c:%s:%s",fmt.name,fmt.check_type,tmp,fmt.err_code);

				return CHECK_FAIL;
			}

		}
	}


	if(tail[0] != '\0')				  //校验尾
	{	
		
		map< string,vector<RecordHTFmt> >::const_iterator iter2 = mapFileRecordTailFmt.find(str);
		if(iter2 == mapFileRecordTailFmt.end())
		{		
			return CHECK_SUCCESS;
		}
		
		vector<RecordHTFmt> vfmtb = iter2->second;
		vector<string> coloum_value_tail;
		
		//cout<<"规则表配置的文件记录尾的字段个数"<<vfmtb.size()<<endl;
		flag = 0;
		if(vfmtb[0].seperator != '')
		{
			splitString(tail,vfmtb[0].seperator,coloum_value_tail,false,false);
			//cout<<" 分隔符配置取字段 从记录中获取的尾字段个数："<<coloum_value_tail.size()<<endl;
			flag = 1;
		}	

		for(int i = 0;i<vfmtb.size();i++)
		{
			fmt = vfmtb[i] ;
			memset(tmp,0,sizeof(tmp));
		
			if(flag)             //表示通过分隔符获取字段值
			{
				strcpy(tmp,coloum_value_tail[vfmtb[i].number-1].c_str());
			}
			else
			{
				strncpy(tmp,tail+fmt.index,fmt.len);
			}

			//校验类型
			if( checkRule(fmt,tmp) == CHECK_FAIL) 
			{
				strcpy(erro_tmp,"文件记录尾校验:");
				strcat(erro_tmp,erro_msg);			
				strcpy(erro_msg,erro_tmp);	
				
				strcpy(err_code,fmt.err_code);		//字段I的值在做某种类型的校验失败
				sprintf(m_AuditMsg,"%s:%c:%s:%s",fmt.name,fmt.check_type,tmp,fmt.err_code);

				return CHECK_FAIL;
			}
		}


	}
		
	return rett;
}


//规则校验 ,将文件名校验和头尾校验规则和在一起
int FormatPlugin::checkRule(RecordHTFmt fmt,char* column_value)
{	
	int rett = 0,len = 0;
	char szbuff[256],szbuff2[256];
		
		//2013-07-26,获取账期，根据特殊标志来判断
		if(strcmp(fmt.spec_flag,"RATE_CYCLE") == 0)
		{
			strcpy(rate_cycle,column_value);
			//cout<<"账期："<<rate_cycle<<endl;
		}

		switch(fmt.check_type)
		{
			case 'C' :            //常量校验
				  
				  if(strcmp(fmt.check_value,column_value) != 0)
				  {
					  //strcpy(err_code,fmt.err_code);
					  memset(erro_msg,0,sizeof(erro_msg));
					  sprintf(erro_msg,"常量校验失败,[%s != %s]",column_value,fmt.check_value);
					  return CHECK_FAIL;
				  }

				  break;
		
			case 'Z' :			 //日期格式校验支持6位 8位 10位 12位 14位
				   
				  len = strlen(column_value);
				  if(len != strlen(fmt.check_value))  //要校验的日期字符串截取的长度和设置的日期格式不一样，肯定校验失败
				  {
					    //strcpy(err_code,fmt.err_code);
					    memset(erro_msg,0,sizeof(erro_msg));
					    sprintf(erro_msg,"要校验的日期字符串截取的长度和设置的日期格式不一样,[%s != %s]",column_value,fmt.check_value);
						return CHECK_FAIL;
				  }
				  
				  for(int j = 0;j<len;j++)
				  {
						if(column_value[j] > '9' || column_value[j] < '0')
						{
							//strcpy(err_code,fmt.err_code);
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"日期校验之数字校验失败,日期必须全为数字,[%s]",column_value);
							return CHECK_FAIL;
						}
				  }

				  if(len == 6||len == 8||len ==10||len ==12||len ==14)
				  {
							if(checkDate(column_value) == CHECK_FAIL)
							{
								//strcpy(err_code,fmt.err_code);
								memset(erro_msg,0,sizeof(erro_msg));
								sprintf(erro_msg,"日期格式校验失败,日期不合法 [%s]",column_value);
								return CHECK_FAIL;
							}
				  }
				  else 
				  {
					  //strcpy(err_code,fmt.err_code);
					  memset(erro_msg,0,sizeof(erro_msg));
					  sprintf(erro_msg,"日期格式校验失败,长度必须为6,8,10,12,14 [%s]",column_value);
					  return CHECK_FAIL;
				  }

				  break;

			case 'D' :          //数字校验 
				  
				  for(int j = 0;j<strlen(column_value);j++)
				  {
						if(column_value[j] > '9' || column_value[j] < '0')
						{
							//strcpy(err_code,fmt.err_code);
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"数字校验失败,必须全为数字,[%s]",column_value);
							return CHECK_FAIL;
						}
				  }
				  break;

			case 'J':          //记录数校验
				  
				  if(atoi(column_value) !=  record_num)
				  {
					    //strcpy(err_code,fmt.err_code);
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"文件记录数校验失败,[%s != %d]",column_value,record_num);
						return CHECK_FAIL;
				  }

				  break;
			case 'F':			 //字符串函数截取-变量校验只支持substring函数 内置变量暂时支持 文件名 数据源，文件类型
				
			/*
				  if(record_num == 0)	//清算系统与国际运营商交换的清单文件名称。空文件时该字段为空。不做校验
				  {
						break;
				  }
				  if(strcmp("substring($FILENAME)",fmt.check_value) == 0)		//校验文件名
				  {
						if(strcmp(column_value,m_szFileName) != 0)
					    {
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"文件名不一致,[%s != %s]",column_value,m_szFileName);
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
						//cout<<"变量名 = "<<varName<<endl;
						if(strcmp(varName,"FILENAME") == 0)
						{
								strcpy(varValue,m_szFileName);					
						}
						else
					    {
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"字符串函数中存在不识别的变量! $%s",varName);
							return CHECK_SUCCESS;
					    }

						memset(varName,0,sizeof(varName));
						strcpy(szbuff2,p+1);						
						p = strchr(szbuff2,',');
						if(p == NULL)						return CHECK_FAIL;
						strncpy(varName,szbuff2,p-szbuff2);
						//cout<<"截取变量的起始位置 = "<<varName<<endl;
						index = atoi(varName);
						if(index <= 0)
						{
							//strcpy(err_code,fmt.err_code);
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"字符串函数[%s]配置不合法 index <= 0 ",fmt.check_value);
							return CHECK_FAIL;
						}
						index--;

						memset(varName,0,sizeof(varName));
						strcpy(szbuff2,p+1);
						p = strchr(szbuff2,')');
						if(p == NULL)						return CHECK_FAIL;
						strncpy(varName,szbuff2,p-szbuff2);
						//cout<<"截取变量的长度 = "<<varName<<endl;
						len = atoi(varName);
						if(len <= 0)
						{
							//strcpy(err_code,fmt.err_code);
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"字符串函数[%s]配置不合法 length <= 0 ",fmt.check_value);
							return CHECK_FAIL;
						}

						memset(szbuff,0,sizeof(szbuff));
						strncpy(szbuff,varValue+index,len);
						if(strcmp(column_value,szbuff)  != 0)
						{
							//strcpy(err_code,fmt.err_code);
							memset(erro_msg,0,sizeof(erro_msg));
							sprintf(erro_msg,"字符串函数截取变量校验失败 %s != %s",column_value,fmt.check_value);
							return CHECK_FAIL;
						}					
						
				  }
				  else
				  {
						theJSLog<<"字符串校验函数表达式配置不合法，此时跳过校验"<<fmt.check_value<<endw;
				  }

				  break ;

			default :           //默认不校验 S
				  break;

		}	
		
		return CHECK_SUCCESS;
}


//校验日期是否合法
//对如下格式进行校验,不判断是否闰年
// 长度 6 yyyymm
// 长度 8 yyyymmdd
// 长度 10 yymmddhh
// 长度 12 yymmddhhmi
// 长度 14 yymmddhhmiss

int FormatPlugin::checkDate(char* date)
{
	//num =sscanf(buf, "%4d%2d2%d2%d2%2d%2s", &year, &month, &day, &hour, &minute, &second);
	int iTemp,iTemp2;
	char chTemp[10];
	memset(chTemp,0,sizeof(chTemp));
	
	int len = strlen(date);

	//校验年份2000-2999
	strncpy(chTemp,date,4);
	chTemp[4] = 0;
	iTemp = atoi(chTemp);
	if(iTemp < 2000 || iTemp > 2999)
	{			
			return CHECK_FAIL;
	}
	
	//校验月份1-12
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
		//校验日
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
		
		//校验时
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
		
		//校验分
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

		//校验秒
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
	int ret = 0;
	char tmp[512],filter[256],szFiletypeIn[10],inputFilePath[512];
	memset(tmp,0,sizeof(tmp));
	
	CF_CFscan scan;

	if(gbExitSig || !gbNormal)  //主进程退出或者子进程异常退出
	{
		//if(gbExitSig) PS_Process::writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "应用程序收到退出信号");
		theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR,"应用程序收到退出信号");
		prcExit();
		return;
	}

	if(drStatus == 1)  //备系统接收到内容后才进行处理
	{
			//检查trigger触发文件是否存在
			 if(!CheckTriggerFile())
			 {
				sleep(1);
				return ;
			 }
			
			memset(m_SerialString,0,sizeof(m_SerialString));
			ret = drVarGetSet(m_SerialString);
			if(ret)
			{
				theJSLog<<"同步失败..."<<endw;
				return ;
			}
			
			//保证主备系统的file_id同时增加
			if(!(dbConnect(conn)))
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"dealFile() 连接数据库失败 connect error");
				theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
				return ;
			}

			memset(sql,0,sizeof(sql));
			strcpy(sql,"select S_FILE_RECEIVED.NEXTVAL from dual");		//从数据库求得FileID
			Statement stmt = conn.createStatement();
			stmt.setSQLString(sql);
			stmt.execute();					
			stmt>>file_id;				
			stmt.close();

			memset(m_AuditMsg,0,sizeof(m_AuditMsg));
			//获取同步变量
			vector<string> data;		
			splitString(m_SerialString,";",data,false,false);  //发送的字符串数据源ID,文件名,sqlFile

			it = m_SourceCfg.find(data[0]);
			if(it == m_SourceCfg.end())
			{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"没有找到该数据源信息[%s]",data[0]);		//环境变量未设置
				theJSLog.writeLog(LOG_CODE_SOURCE_NOT_FIND,erro_msg);

				dr_AbortIDX();
				return ;
			}
			
			memset(inputFilePath,0,sizeof(inputFilePath));	
			strcpy(inputFilePath,it->second.szSourcePath);
			strcat(inputFilePath,input_path); 

			memset(fileName,0,sizeof(fileName));
			strcpy(fileName,inputFilePath);
			strcat(fileName,data[1].c_str());
			strcpy(m_szFileName,data[1].c_str());
			
			file_id = atol(data[2].c_str());		//file_id由主系统获取

			//int dr_GetAuditMode()1表示同步，2表示跟随, 其它为失败，-1是配置错误，-2配置文件读取出现问题
			int iStatus = dr_GetAuditMode(module_name);
			if(iStatus == 1)		//同步模式,只能是单边情况
			{	
				bool flag = false;
				int times = 1;
				while(times < 31)
				{
					if(access(fileName,F_OK|R_OK))
					{
						theJSLog<<"查找了"<<times<<"次文件"<<endi;
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
					theJSLog<<"无法查询到文件:"<<fileName<<endw;
					dr_AbortIDX();		
					return ;
				}
			}
			else if(iStatus == 2)		//跟随模式,默认300s
			{
				//设置中断
				if(gbExitSig)
				{
					dr_AbortIDX();

					theJSLog.writeLog(LOG_CODE_APP_SEM_EXIT_ERR, "应用程序收到退出信号");
					PS_Process::prcExit();
					return;
				}

				while(1)
				{
					//设置中断
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
				snprintf(tmp, sizeof(tmp), "容灾平台dr_GetAuditMode函数配置错误，返回值[%d]", iStatus);
				theJSLog<<tmp<<endw;
				return ;
			}
			
			theJSLog<<"查询到文件:"<<fileName<<endi;

			setSQLFileName(data[1].c_str());
			ret = dealFile();						//处理文件	
			if(ret == -11)
			{
				theJSLog<<"系统出错"<<endw;
				dr_AbortIDX();
				return;
			}
			
			conn.close();

			dealAuditResult(ret);
	}
	else			//主系统,非容灾系统
	{	
			int counter = 0;

			//依次扫描各个数据源
			for(it = m_SourceCfg.begin();it != m_SourceCfg.end();++it)        //先获取数据源信息
			{	
				memset(inputFilePath,0,sizeof(inputFilePath));				
				strcpy(inputFilePath,it->second.szSourcePath);
				strcat(inputFilePath,input_path); 
			
				memset(filter,0,sizeof(filter));
				strcpy(filter,it->second.filterRule);		  //过滤条件
			
				if(scan.openDir(inputFilePath))
				{
					sprintf(erro_msg,"打开话单文件目录[%s]失败",inputFilePath);
					theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg); //打开目录出错
					//errLog(4,"",FILEREC_ERR_SCAN_RECDIR,sz_errmsg,__FILE__,__LINE__);
					sleep(2);
					return ;	
				}		
						
				//循环读取目录，扫描文件夹，获取文件
				int rett = -1 ;
				counter = 0;
				while(1) 
				{	 
					memset(fileName,0,sizeof(fileName));
					rett = scan.getFile(filter,fileName);  				
					if(rett == 100)
					{		
						break ;  
					}
					if(rett == -1)
					{
						continue;			//表示获取文件信息失败
					}

					counter++;				//扫描一次文件计数器+1	
					
					if(counter > source_file_num)
					{					
						break;				//cout<<"扫描10次后，跳到下个数据源"<<endl;
					}

					/*过滤文件*.tmp,*TMP,~* */			
					char* p = strrchr(fileName,'/');
					memset(tmp,0,sizeof(tmp));
					strcpy(tmp,p+1);
					if(tmp[0] == '~' ) continue;
					if(strlen(tmp) > 2)
					{		
						int pos = strlen(tmp)-3;
						//cout<<"后缀名为："<<tmp+pos<<endl;
						if((strcmp(tmp+pos,"tmp") && strcmp(tmp+pos,"TMP")) == 0) 
						{
							//cout<<"扫描到临时文件，舍弃"<<fileName<<endl;
							continue;
						}
					}		
					
					theJSLog<<"扫描到文件:"<<fileName<<endi;

					strcpy(m_szFileName,p+1);  //复制文件名,去路径					
					
					if(!(dbConnect(conn)))
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"dealFile() 连接数据库失败 connect error");
						theJSLog.writeLog(LOG_CODE_DB_CONNECT_ERR,erro_msg);		//连接数据库失败
						return ;
					}

					memset(sql,0,sizeof(sql));
					strcpy(sql,"select S_FILE_RECEIVED.NEXTVAL from dual");		//从数据库求得FileID
					Statement stmt = conn.createStatement();
					stmt.setSQLString(sql);
					stmt.execute();					
					stmt>>file_id;				
					stmt.close();

					//同步变量数据源ID,文件名,sql文件名
					memset(m_SerialString,0,sizeof(m_SerialString));
					sprintf(m_SerialString,"%s;%s;%ld",it->first,m_szFileName,file_id);
					ret = drVarGetSet(m_SerialString);
					if(ret)
					{
						theJSLog<<"同步失败...."<<endw;
						return ;
					}

					setSQLFileName(m_szFileName);

					ret = dealFile();			//处理文件
					if(ret == -11)
					{
						conn.close();
						theJSLog<<"系统出错"<<endw;
						dr_AbortIDX();
						continue;
					}
					
					conn.close();

					dealAuditResult(ret);
				
				}//while(1)

				scan.closeDir();

			}//for

		//sleep(2);		
	}
}

int FormatPlugin::dealFile()
{			
	    //long file_id = 0;
		int ret = 0,deal_flag = 0; //格式化处理正确标志
		char dir[512],erro_dir[512],timeout_dir[512],other_dir[512],szFiletypeIn[10];
		char szBuff[1012],head[1024],tail[1024];
		char outFilePath[256],inputFilePath[256];		
		char tmp[512],state = 'W';
		ofstream ff ;
		
		pps = new PacketParser();
		res = new ResParser();

		memset(dir,0,sizeof(dir));
		strcpy(dir,it->second.szSourcePath);

		memset(outFilePath,0,sizeof(outFilePath));  
		strcpy(outFilePath,it->second.szSourcePath);
		strcat(outFilePath,output_path);	
		
		memset(szFiletypeIn,0,sizeof(szFiletypeIn));  
		strcpy(szFiletypeIn,it->second.szInFileFmt);	//当前数据源的输入格式
		strcpy(m_szSourceID,it->first.c_str());			//当前数据源ID
		
		memset(mServCatId,0,sizeof(mServCatId));
		strcpy(mServCatId,it->second.serverCatID);

		memset(tmp,0,sizeof(tmp));
		strcpy(tmp,fileName);
		strcat(tmp,".tmp");
		if(rename(fileName,tmp))			//文件改名成临时文件, 【考虑发生异常后如何修改回源文件】
		{
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"文件改名失败: %s",strerror(errno));
				theJSLog.writeLog(LOG_CODE_FILE_RENAME_ERR,erro_msg); // 文件改名出错

				return -11 ;
		}	

		theJSLog<<"准备对文件 ["<<m_szFileName<<"] 进行处理"<<endi;
		getCurTime(currTime);
		
		memset(m_AuditMsg,0,sizeof(m_AuditMsg));

		//发送消息，处理新文件	1111 = FILE_ID 在ORACLE里建一个SEQUENCE，比如SEQ_FILEID，用select的方式得到
		try
		{	
		
					memset(outFileName,0,sizeof(outFileName));
					sprintf(outFileName,"%s%s",outFilePath,m_szFileName);		//输出文件名,全路径			
				
					//校验是否文件重复*******重复则放入错误目录***********************************************				
					if(checkFileRepeat(m_szFileName))
					{
							strcpy(err_code,"F104");
							sprintf(erro_msg,"文件重复,调度表中已经存在,F104");					
							theJSLog.writeLog(LOG_CODE_FILE_NAME_REPEAT ,erro_msg);			
							
							memset(sql,0,sizeof(sql));
							getCurTime(currTime);    //获取当前文件时间
							//state = 'E';
							//sprintf(sql,"update D_SCH_FORMAT set DEAL_FLAG='%c',DEAL_TIME='%s' where SOURCE_ID='%s' and FILENAME='%s' ",state,currTime,m_szSourceID,m_szFileName);
							//writeSQL(sql);
							//memset(sql,0,sizeof(sql));
							//state = 'W'; 
							sprintf(sql,"insert into D_ERRFILE_INFO(FILENAME,SOURCE_ID,DEAL_TIME,ERR_MSG,ERR_CODE,ERR_COL,ERR_LINE,ERR_SEQ,STATE) values('%s','%s','%s','%c','%s',%d,%d,%ld,'%c')",m_szFileName,m_szSourceID ,currTime,'F',err_code,0,0,file_id,state);		
							writeSQL(sql);

							memset(erro_dir,0,sizeof(erro_dir));
							strcpy(erro_dir,dir);
							strcat(erro_dir,erro_path);
							rename(tmp,strcat(erro_dir,m_szFileName));
							
							sprintf(m_AuditMsg,"%s:%s",m_szFileName,"F104");
							return -1;
					}
							
					//取文件名上的时间插入到调度表并带到话单里面2013-08-01
					memset(file_time,0,sizeof(file_time));
					strncpy(file_time,m_szFileName+it->second.file_begin,it->second.file_length);
					file_time[8] = '\0';

					//扫描到文件后将其注册
					memset(sql,0,sizeof(sql));
					sprintf(sql,"insert into D_SCH_FORMAT(SOURCE_ID,FILENAME,RECIEVE_TIME,DEAL_FLAG,RECORD_COUNT,FILE_ID,FILE_TIME,SERV_CAT_ID) values('%s','%s','%s','%c',%d,%ld,'%s','%s')",m_szSourceID,m_szFileName,currTime,state,0,file_id,file_time,mServCatId);					
					writeSQL(sql);

					//对文件名进行校验******************************************************
					theJSLog<<"对文件名进行校验..."<<endi;
					if(checkFileName(m_szSourceID,m_szFileName) == CHECK_FAIL)
					{
							//strcpy(err_code,"F101");
							theJSLog.writeLog(LOG_CODE_FILE_NAME_CHECK_VALID,erro_msg);		
										
							getCurTime(currTime);    //获取当前文件时间
							state = 'E';
							sprintf(sql,"update D_SCH_FORMAT set DEAL_FLAG='%c',DEAL_TIME='%s' where SOURCE_ID='%s' and FILENAME='%s' and DEAL_FLAG = 'W' ",state,currTime,m_szSourceID,m_szFileName);
							writeSQL(sql);
							memset(sql,0,sizeof(sql));
							state = 'W';
							sprintf(sql,"insert into D_ERRFILE_INFO(FILENAME,SOURCE_ID,DEAL_TIME,ERR_MSG,ERR_CODE,ERR_COL,ERR_LINE,ERR_SEQ,STATE) values('%s','%s','%s','%c','%s',%d,%d,%ld,'%c')",m_szFileName,m_szSourceID ,currTime,'F',err_code,0,0,file_id,state);
							writeSQL(sql);
							
							memset(erro_dir,0,sizeof(erro_dir));
							strcpy(erro_dir,dir);
							strcat(erro_dir,erro_path);
							rename(tmp,strcat(erro_dir,m_szFileName)); 
							
							//sprintf(m_AuditMsg,"F101");
							return -1;

					}
					
					//2013-10-17 新增文件打开错误 错误码F103
					ifstream in(tmp,ios::nocreate);
					if(!in)
					{					
						strcpy(err_code,"F103");
						sprintf(erro_msg,"文件打开错误,F103");					
						theJSLog.writeLog(LOG_CODE_FILE_NAME_REPEAT ,erro_msg);			
						
						memset(sql,0,sizeof(sql));
						getCurTime(currTime);    //获取当前文件时间
						state = 'E';
						sprintf(sql,"update D_SCH_FORMAT set DEAL_FLAG='%c',DEAL_TIME='%s' where SOURCE_ID='%s' and FILENAME='%s' and DEAL_FLAG = 'W' ",state,currTime,m_szSourceID,m_szFileName);
						writeSQL(sql);
						memset(sql,0,sizeof(sql));
						state = 'W';
						sprintf(sql,"insert into D_ERRFILE_INFO(FILENAME,SOURCE_ID,DEAL_TIME,ERR_MSG,ERR_CODE,ERR_COL,ERR_LINE,ERR_SEQ,STATE) values('%s','%s','%s','%c','%s',%d,%d,%ld,'%c')",m_szFileName,m_szSourceID ,currTime,'F',err_code,0,0,file_id,state);
						writeSQL(sql);

						memset(erro_dir,0,sizeof(erro_dir));
						strcpy(erro_dir,dir);
						strcat(erro_dir,erro_path);
						rename(tmp,strcat(erro_dir,m_szFileName));
							
						sprintf(m_AuditMsg,"%s:%s",m_szFileName,"F103");
						return -1;
					}
					in.close();

					CF_MemFileI _infile;
					_infile.Init(szFiletypeIn); //输入格式			
					_infile.Open(tmp);
			
					//对文件头尾进行校验****************************************************
					memset(head,0,sizeof(head));
					memset(tail,0,sizeof(tail));
					_infile.GetHead(head,MAX_LINE_LENGTH);
					_infile.GetEnd(tail,MAX_LINE_LENGTH);
					
					record_num  = _infile.Get_recCount();  //求出文件记录条数,供校验

					iTotalNum = 0; 
					if(head[0] != '\0') iTotalNum++	;					//2013-07-15	有头记录行号+1
					
					memset(rate_cycle,0,sizeof(rate_cycle));			//每次校验头尾时将账期置空
					
					theJSLog<<"对文件头尾进行校验..."<<endi;
					if(checkRecorHT(szFiletypeIn,head,tail) == CHECK_FAIL)
					{					
							theJSLog.writeLog(LOG_CODE_FILE_HEAD_TAIL_VALID,erro_msg);		//错误码有待更正

							getCurTime(currTime);    //获取当前文件时间
							memset(sql,0,sizeof(sql));
							state = 'E';
							sprintf(sql,"update D_SCH_FORMAT set DEAL_FLAG='%c',DEAL_TIME='%s' where SOURCE_ID='%s' and FILENAME='%s' and DEAL_FLAG = 'W' ",state,currTime,m_szSourceID,m_szFileName);
							writeSQL(sql);
							memset(sql,0,sizeof(sql));
							state = 'W';
							sprintf(sql,"insert into D_ERRFILE_INFO(FILENAME,SOURCE_ID,DEAL_TIME,ERR_MSG,ERR_CODE,ERR_COL,ERR_LINE,ERR_SEQ,STATE) values('%s','%s','%s','%c','%s',%d,%d,%ld,'%c')",m_szFileName,m_szSourceID ,currTime,'F',err_code,0,0,file_id,state);
							writeSQL(sql);
							
							memset(erro_dir,0,sizeof(erro_dir));
							strcpy(erro_dir,dir);
							strcat(erro_dir,erro_path);
							rename(tmp,strcat(erro_dir,m_szFileName));

							return -1;
					}					
					
					if(record_num == 0)
					{
							theJSLog<<"该文件空文件..."<<endw;
							conn.close();
							sprintf(m_AuditMsg,"%s;%s,%d",m_szSourceID,m_szFileName,record_num);
							return ret;
					}

					//发送消息处理文件
					MessageParser  pMessage; 
					pMessage.setMessage(MESSAGE_NEW_BATCH, it->first.c_str(),m_szFileName,file_id);
					message(pMessage);
					pMessage.setMessage(MESSAGE_NEW_FILE, it->first.c_str(),m_szFileName,file_id);			
					message(pMessage);

					pps->clearAll();
					res->clearAll();		

					record_num = 0;
					deal_flag = 0;							//默认文件处理结果为正常的
					memset(szBuff, 0, sizeof(szBuff));      //将数据清空		
					memset(erro_msg,0,sizeof(erro_msg));  //先清空缓存错误消息
	
					//循环扫描文件记录
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
							//theJSLog<<"格式化记录出错单！数据源："<<m_szSourceID<<"  原始文件名："<<m_szFileName<<" 分割后的文件名"<<file_name<<" 记录位置："<<record_num<<"	状态："<<res->getAnaType()<<ende;
							deal_flag = 1;
							
						}
																						
						switch(res->getAnaType())   //根据格式后的类型判断记录的处理方式
						{
							case eNormal:			//正常
								 break;

							case eFmtErr:			//格式化错单
								 memset(erro_msg,0,sizeof(erro_msg));
								 sprintf(erro_msg,"格式化错单:%d[%s|%s]",record_num,res->getRuleType(),res->getReason());
								 theJSLog.writeLog(LOG_CODE_FORMAT_ERR_RCD ,erro_msg);	
							
								 sprintf(m_AuditMsg,"format err rcd:%d[%s|%s]",record_num,res->getRuleType(),res->getReason());

								 memset(erro_dir,0,sizeof(erro_dir));
								 strcpy(erro_dir,dir);							 
								 strcat(erro_dir,erro_path);
								 rename(tmp,strcat(erro_dir,m_szFileName)); //将该错误文件移动到错误目录下
								 break;

							case eFmtTimeOut:		//格式化超时单，入timeout文件
								 memset(erro_msg,0,sizeof(erro_msg));
								 sprintf(erro_msg,"格式化超时单:%d[%s|%s]",record_num,res->getRuleType(),res->getReason());
								 theJSLog.writeLog(LOG_CODE_FORMAT_TIMEOUT_RCD,erro_msg);

								 sprintf(m_AuditMsg,"format timeout rcd:%d[%s|%s]",record_num,res->getRuleType(),res->getReason());

								 memset(timeout_dir,0,sizeof(timeout_dir));
								 strcpy(timeout_dir,dir);			
								 strcat(timeout_dir,timeout_path);
								 rename(tmp,strcat(timeout_dir,m_szFileName)); 
								 break;

							case eFmtOther:			//格式化未定义格式话单，入otherformat文件
								 memset(erro_msg,0,sizeof(erro_msg));
								 sprintf(erro_msg,"未定义的格式化单:%d[%s|%s]",record_num,res->getRuleType(),res->getReason());
								 theJSLog.writeLog(LOG_CODE_FORMAT_OTHER_RCD,erro_msg);
								
								 sprintf(m_AuditMsg,"format other rcd:%d[%s|%s]",record_num,res->getRuleType(),res->getReason());

								 memset(other_dir,0,sizeof(other_dir));
								 strcpy(other_dir,dir);
								 strcat(other_dir,other_path);
								 rename(tmp,strcat(other_dir,m_szFileName)); 
								 break;

							default :
								theJSLog.writeLog(0,"格式化未知的单");
								sprintf(m_AuditMsg,"format undefine rcd ");
								break;
						}

						if(deal_flag == 1) //有一条记录出错则整个文件不要,退出读文件
						{
							break;  
						}

						strcpy(szBuff,res->m_outRcd.Get_record());

						record_array.push_back(szBuff);
						memset(szBuff, 0, sizeof(szBuff) );					//将数据清空
					}
			
					_infile.Close();

					if(deal_flag == 1)								//格式化文件出了错，记录不要
					{
						record_array.clear();		
						
						//写错误登记表 更新调度表
						getCurTime(currTime);    //获取当前文件时间
						memset(sql,0,sizeof(sql));
						state = 'E';
						sprintf(sql,"update D_SCH_FORMAT set DEAL_FLAG='%c',DEAL_TIME='%s',RECORD_COUNT=%d where SOURCE_ID='%s' and FILENAME='%s' and DEAL_FLAG = 'W' ",state,currTime,record_num,m_szSourceID,m_szFileName);
						writeSQL(sql);
						if(res->getAnaType() == eFmtErr)		
						{
							memset(sql,0,sizeof(sql));
							state = 'W';
							sprintf(sql,"insert into D_ERRFILE_INFO(FILENAME,SOURCE_ID,DEAL_TIME,ERR_MSG,ERR_CODE,ERR_COL,ERR_LINE,ERR_SEQ,STATE) values('%s','%s','%s','%c','%s',%d,%d,%ld,'%c')",m_szFileName,m_szSourceID ,currTime,'F',"F110",0,iTotalNum,file_id,'W');
							writeSQL(sql);
						}
			
						return -1;
					}			
					
					//写文件完成后清空私有内存,先写临时文件,等仲裁成功后再写正式文件
					ret = writeFile(outFileName);
					if(ret)
					{
						theJSLog<<"写文件失败["<<outFileName<<"]"<<endi;
						state = 'E';
					}
					record_array.clear();     								
					
					//struct stat fileInfo;
					memset(tmp,0,sizeof(tmp));
					strcpy(tmp,outFileName);
					strcat(tmp,".tmp");
					stat(tmp,&fileInfo); //获取文件大小	
					
					sprintf(m_AuditMsg,"%s;%s,%d,%d",m_szSourceID,m_szFileName,record_num,fileInfo.st_size);

				//发送消息，结束文件处理文件
				pMessage.setMessage(MESSAGE_END_FILE, it->first.c_str(), m_szFileName,file_id);			
				message(pMessage);	
				pMessage.setMessage(MESSAGE_END_BATCH_END_FILES, it->first.c_str(), m_szFileName,file_id);
				message(pMessage);

		}catch(jsexcp::CException e)			//抛异常时文件怎么处理,暂时放在源目录下面,需要手工干预
		{			
				rollBackSQL();
				//conn.close();

				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"dealFile() 格式化出错 %s",e.GetErrMessage());
				theJSLog.writeLog(LOG_CODE_FIELD_CONVERT_ERR,erro_msg);		//字段转换出错
					
				//sprintf(m_AuditMsg,"%s;%s",m_AuditMsg,e.GetErrMessage());
				return -11;
		} 
		catch(util_1_0::db::SQLException e)
		{ 
				rollBackSQL();
				//conn.close();
				memset(erro_msg,0,sizeof(erro_msg));
				sprintf(erro_msg,"dealFile() 数据库操作异常 %s (%s)",e.what(),sql);
				theJSLog.writeLog(LOG_CODE_DB_EXECUTE_ERR,erro_msg);		//发生sql执行异常	
					
				//sprintf(m_AuditMsg,"%s;%s",m_AuditMsg,e.what());
				return -11;
		}
				  
		return ret ;
}


//将格式化成功的记录 先写临时文件
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
			sprintf(erro_msg,"writeFile 文件%s打开出错",filename);
			theJSLog.writeLog(LOG_CODE_FILE_OPEN_ERR,erro_msg);		//打开文件失败

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
		
		//在这儿仲裁.....
		if(!IsAuditSuccess(m_AuditMsg))		//仲裁不成功调度表置E,文件移到错误目录,生成的文件删除
		{
				theJSLog<<"仲裁失败..."<<endw;
				char erro_dir[1024];
				rollBackSQL();

				if(result != -1)		//正常格式化文件,但仲裁失败
				{
					memset(tmp,0,sizeof(tmp));
					strcpy(tmp,fileName);
					strcat(tmp,".tmp");
					memset(erro_dir,0,sizeof(erro_dir));
					strcpy(erro_dir,it->second.szSourcePath);			 
					strcat(erro_dir,erro_path);
					if(rename(tmp,strcat(erro_dir,m_szFileName)))
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"移动源文件到错误目录[%s]失败: %s",erro_path,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);

					}
				}

				if(record_num)
				{
					if(remove(strcat(outFileName,".tmp")))
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"临时文件[%s]删除失败: %s",outFileName,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
						ret = -1;
					}
				}

				//需要将原始文件移到错误目录吗????
		}
		else									//仲裁成功,提交sql 临时改正式,
		{		
				if(result)						//格式化出错的情况
				{
					commitSQLFile();
					return  ret;
				}
				else
				{
					state = 'Y';
				}
				
				if(record_num)  //非空文件,不输出
				{
					memset(tmp,0,sizeof(tmp));
					strcpy(tmp,outFileName);
					strcat(tmp,".tmp");
					stat(tmp,&fileInfo); //获取文件大小	

					if(rename(tmp,outFileName))
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"临时文件[%s]改名正式失败: %s",tmp,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_RENAME_ERR,erro_msg);
						return -1;
					}
				}
				else
				{
					fileInfo.st_size = 0;
				}

				//仲裁成功更新调度表
				memset(sql,0,sizeof(sql));
				getCurTime(currTime);				//获取当前文件时间
				sprintf(sql,"update D_SCH_FORMAT set DEAL_FLAG='%c',DEAL_TIME='%s',RECORD_COUNT=%d,FILESIZE=%ld where SOURCE_ID='%s' and FILENAME='%s' and DEAL_FLAG = 'W'",state,currTime,record_num,fileInfo.st_size,m_szSourceID,m_szFileName);
				writeSQL(sql);
				commitSQLFile();


				//要备份,2013-07-16目录根据YYYYMM/DD	
				memset(tmp,0,sizeof(tmp));
				strcpy(tmp,fileName);
				strcat(tmp,".tmp");
				
				char bak_dir[512];
				memset(bak_dir,0,sizeof(bak_dir));
				strcpy(bak_dir,it->second.szSourcePath);
				strcat(bak_dir,bak_path);

				strncat(bak_dir,currTime,6);
				strcat(bak_dir,"/");
				strncat(bak_dir,currTime+6,2);
				strcat(bak_dir,"/");
				if(chkAllDir(bak_dir) == 0)
				{	
					theJSLog<<"备份文件到目录["<<bak_dir<<"]"<<endi;
					strcat(bak_dir,m_szFileName);
					if(rename(tmp,bak_dir))
					{
						memset(erro_msg,0,sizeof(erro_msg));
						sprintf(erro_msg,"备份源文件[%s]失败: %s",m_szFileName,strerror(errno));
						theJSLog.writeLog(LOG_CODE_FILE_MOVE_ERR,erro_msg);
						ret = -1;
					}
				}
				else
				{	
					memset(erro_msg,0,sizeof(erro_msg));
					sprintf(erro_msg,"备份路径[%s]不存在，且无法创建",bak_dir);
					theJSLog.writeLog(LOG_CODE_DIR_OPEN_ERR,erro_msg);		//打开目录出错

					ret = -1;
				}		
		}
					
	
	return ret;
}



//容灾初始化
bool FormatPlugin::drInit()
{
		//传入模块名和实例ID
		char tmp[32];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%ld",getPrcID());

		theJSLog << "初始化容灾平台,模块名:"<< module_name<<" 实例名:"<<tmp<<endi;

		int ret = dr_InitPlatform(module_name,tmp);
		if(ret != 0)
		{
			theJSLog << "容灾平台初始化失败,返回值=" << ret<<endw;
			return false;
		}
		else
		{
			theJSLog<<"dr_InitPlatform ok."<<endi;
		}

		m_enable = true;

		drStatus = _dr_GetSystemState();	//获取主备系统状态
		if(drStatus < 0)
		{
			theJSLog<<"获取容灾平台状态出错: 返回值="<<drStatus<<endw;
			return false;
		}
		
		if(drStatus == 0)		theJSLog<<"当前系统配置为主系统"<<endi;
		else if(drStatus == 1)	theJSLog<<"当前系统配置为备系统"<<endi;
		else if(drStatus == 2)	theJSLog<<"当前系统配置非容灾系统"<<endi;

		return true;
}

//主系统发送同步变量,备系统获取同步变量
int FormatPlugin::drVarGetSet(char* serialString)
{
		int ret ;
		char tmpVar[5000] = {0};

		//检查容灾平台的切换锁
		ret = dr_CheckSwitchLock();   
		if(ret != 0)  
		{  
			theJSLog<<"检查容灾切换锁失败,返回值="<<ret<<endw;
			return -1;  
		} 
		//初始化index  
		ret = dr_InitIDX();  
		if(ret != 0)  
		{  
			theJSLog<<"初始化index失败,返回值=" <<ret<<endw;
			dr_AbortIDX();
			return -1;  
		}

/*
		//主系统传递文件所在路径和文件名 只有容灾平台可以感知,备系统无法识别
		if(drStatus != 1)
		{
			snprintf(tmpVar, sizeof(tmpVar), "%s%s", it->second.szSourcePath,input_path);
			ret = dr_SyncIdxVar("@@CHECKPATH", tmpVar,SYNC_SINGLE);  
			if(ret != 0)
			{
				theJSLog<<"传输文件所在路径失败,文件路径["<<input_path<<"]"<<endw;
				dr_AbortIDX();
				return -1;
			}
			
			snprintf(tmpVar, sizeof(tmpVar), "%s", m_szFileName);
			ret = dr_SyncIdxVar("@@CHECKFILE", tmpVar,SYNC_SINGLE);  
			if(ret != 0)
			{
				theJSLog<<"传输文件失败,文件名["<<m_szFileName<<"]"<<endw;
				dr_AbortIDX();
				return -1;
			}

			cout<<"传输文件路径:"<<input_path<<" 文件名:"<<m_szFileName<<endl;
		}
*/
		snprintf(tmpVar, sizeof(tmpVar), "%s", serialString);
		//主系统把要同步的index “键值对”写入容灾平台维护的index 文件中
		//备系统调用该函数的结果是，var获得和主系统一样的随机变量的值。	SYNC_SINGLE表示注册单一的随机变量
		ret = dr_SyncIdxVar("serialString", tmpVar, SYNC_SINGLE);		
		if (ret != 0)
		{
			theJSLog<<"传序列串时失败，序列名：["<<serialString<<"]"<<endw;
			dr_AbortIDX();
			return -1;
		}
		//serialString = tmpVar;			//同步索引字符串,主系统是赋值，备系统是取值
		strcpy(serialString,tmpVar);
		//m_AuditMsg = tmpVar;			//要仲裁的字符串

		// <5> 传输实例名  用于主系统注册此IDX的模块调用参数。
		//备系统的index manager检查IDX条件满足后，把使用该函数注册的随机变量作为模块的调用参数trigger相应的进程
		snprintf(tmpVar, sizeof(tmpVar), "%d", getPrcID());
		ret = dr_SyncIdxVar("@@ARG", tmpVar,SYNC_SINGLE);  
		if(ret !=0)
		{
			theJSLog<<"传输实例名失败："<<tmpVar<<endw;
			dr_AbortIDX();  
			return -1;
		}
		
		
		// <6> 预提交index  此关键字用于将平台当前内存中的随机变量写入磁盘
		ret = dr_SyncIdxVar("@@FLUSH","SUCCESS",SYNC_SINGLE);  
		if (ret != 0 )
		{
			theJSLog<<"预提交index失败"<<endw;
			dr_AbortIDX();
			return -1;
		}
		
		
		// <7> 提交index  	提交Index。在index文件中设置完成标志
		ret = dr_CommitIDX();  
		if(ret != 0)  
		{  
			theJSLog<<"提交index失败,返回值="<<ret<<endw;
			dr_AbortIDX();  
			return -1;  
		}

		//备系统搜索目录
		//if(!m_syncDr.isMaster())thelog<<"备系统SerialString："<<m_SerialString<<endi;

		theJSLog<<"本次的同步串serialString:"<<serialString<<endi;//for test

		return ret;

}

//仲裁字符串
 bool FormatPlugin::IsAuditSuccess(const char* dealresult)
 {
		int auitStatus = 0, ret = 0;
		ret = dr_Audit(dealresult);
		if(2 == ret )
		{
			theJSLog << "容灾仲裁失败,结果:" << ret <<"本端："<<dealresult<< endw;
			dr_AbortIDX();
			return false;
		}
		else if (3 == ret)
		{
			theJSLog<<"容灾仲裁超时..."<<endw;
			dr_AbortIDX();
			return false;
		}
		else if(4 == ret)
		{
			theJSLog<<"对端idx异常终止..."<<endw;
			dr_AbortIDX();
			return false;
		}
		else if(1 == ret)
		{
			ret = dr_CommitSuccess();
			if(ret != 0)
			{
				theJSLog << "业务全部提交失败(容灾平台)" << endw;
				dr_AbortIDX();
				return false;
			}
			theJSLog<<"ret = "<<ret<<"仲裁成功...\n仲裁内容："<<dealresult<<endi;
			return true;
		}
		else
		{
			theJSLog<<"未知ret="<<ret<<"	仲裁内容："<<dealresult<<endw;
			dr_AbortIDX();
			return false;
		}
	
	return true;
 }

bool FormatPlugin::CheckTriggerFile()
{
	int ret = 0;
	if(access(m_triggerFile.c_str(),F_OK) != 0)	return false;

	theJSLog<< "检查到trigger文件，并删除"<< m_triggerFile <<endi;

	ret = remove(m_triggerFile.c_str());	
	if(ret) 
	{
		memset(erro_msg,0,sizeof(erro_msg));
		sprintf(erro_msg,"删除trigger文件[%s]失败: %s",m_triggerFile,strerror(errno));
		theJSLog.writeLog(LOG_CODE_FILE_DELETE_ERR,erro_msg);
		return false;
	}
	
	return true;
}
