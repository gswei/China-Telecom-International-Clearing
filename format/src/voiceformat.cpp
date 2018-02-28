/*
1.20060824 增加把最早时间和最晚时间入库的功能
2.20061020 qc校验故障修正；
3.20061207 SHQQ被叫号码头为17909(P)、96688(m)的不加号码头
4.20061214 增加字段szError_Len，表示超长时的输入字段序号
*/





#include "voiceformat.h"

char Field[MAX_FIELD_COUNT][FMT_MAX_FIELD_LEN];
#define	ARECORDTYPE					'A'
#define	CRECORDTYPE					'C'

int _CmpTxtRecordFmt(const void *s1,const void *s2)
{
  char *ss1,*ss2;
  int i;
  ss1 = ( (STxtRecordFmtDefine *)s1 )->szRecord_Name;
  ss2 = ( (STxtRecordFmtDefine *)s2 )->szRecord_Name;
  return ( strcmp(ss1,ss2) ) ;
}
int _CmpTxtFileFmt(const void *s1,const void *s2)
{
  char *ss1,*ss2;
  int i;
  ss1 = ( (STxtFileFmtDefine *)s1 )->szFile_Fmt_Name;
  ss2 = ( (STxtFileFmtDefine *)s2 )->szFile_Fmt_Name;
  return ( strcmp(ss1,ss2) ) ;
}

int _CmpTxtSignNode(const void *s1,const void *s2)
{
  int ss1,ss2;

  ss1 = ( (STxtSign_node *)s1 )->iNodePri;
  ss2 = ( (STxtSign_node *)s2 )->iNodePri;
  return (ss1-ss2);
}

int _CmpTxtFmt(const void *s1,const void *s2)
{
  char *ss1,*ss2;
  int i;
  ss1 = ( (STxtFmt *)s1 )->szFileTypeID;
  ss2 = ( (STxtFmt *)s2 )->szFileTypeID;
  return ( strcmp(ss1,ss2) ) ;
}
int _CmpInput2Output(const void *s1,const void *s2)
{
  char *ss1,*ss2;
  int i;
  ss1 = ( (SInput2Output *)s1 )->szInputType;
  ss2 = ( (SInput2Output *)s2 )->szInputType;
  i=strcmp(ss1,ss2);
  if(i) return i;

  ss1 = ( (SInput2Output *)s1 )->szOutputType;
  ss2 = ( (SInput2Output *)s2 )->szOutputType;
  return ( strcmp(ss1,ss2) ) ;
}

CFormat::CFormat()
{
  pTxtFmt=NULL;
  pInput2Output=NULL;
  pCur_TxtFmt=NULL;
  pCur_Input2Output=NULL;
  iTxtFmt_Num=0;
  iInput2Output_Num=0;
  iBCondNum=0;
  iBCond_CurIndex=0;
  memset(szDebug_Flag,0,50);
  iSign2_Begin=0;
  iSign2_Len=0;
  iSign2_Value=0;
}
void CFormat::DelNodeSpace(STxtSign_node *&NNode,int Num)
{
  for(int i=0;i<Num;i++)
  {
    if(NNode[i].pNext_Node == NULL) continue;
    DelNodeSpace(NNode[i].pNext_Node,NNode[i].iNextSign_Num);
    delete[] NNode[i].pNext_Node;
    NNode[i].pNext_Node=NULL;
    NNode[i].iNextSign_Num = 0;
  }
return ;
}

CFormat::~CFormat()
{

  if(pTxtFileFmtDefine!=NULL)
  {
    for(int i=0;i<iTxtFileFmtFefine_Num;i++)
    {
      if(pTxtFileFmtDefine[i].pCheckRoute!=NULL)
      {
        delete[] pTxtFileFmtDefine[i].pCheckRoute;
        pTxtFileFmtDefine[i].pCheckRoute=NULL;
      }

////pRecordFmt      
      if(pTxtFileFmtDefine[i].pTxtRecordFmt!=NULL)
      {
        delete[] pTxtFileFmtDefine[i].pTxtRecordFmt;
        pTxtFileFmtDefine[i].pTxtRecordFmt=NULL;
        pTxtFileFmtDefine[i].iTxtRecordFmt_Num=0;
      }
////pNext_Node
      if(pTxtFileFmtDefine[i].pNext_Node!=NULL)
      {
        DelNodeSpace(pTxtFileFmtDefine[i].pNext_Node,pTxtFileFmtDefine[i].iNextSign_Num);
        delete[] pTxtFileFmtDefine[i].pNext_Node;
        pTxtFileFmtDefine[i].pNext_Node=NULL;
        pTxtFileFmtDefine[i].iNextSign_Num=0;
      }
    }
    
    delete[] pTxtFileFmtDefine;
    pTxtFileFmtDefine=NULL;
    iTxtFileFmtFefine_Num=0;
  }

  
  if(pTxtFmt!=NULL)
  {
    delete[] pTxtFmt;
    pTxtFmt=NULL;
    iTxtFmt_Num=0;
  }

  if(pInput2Output!=NULL)
  {
    for(int i=0;i<iInput2Output_Num;i++)
    {
      if(pInput2Output[i].pInputIndex!=NULL)
      {
        delete[] pInput2Output[i].pInputIndex;
        pInput2Output[i].pInputIndex=NULL;
        pInput2Output[i].iCount=0;
      }
      if(pInput2Output[i].pOutputIndex!=NULL)
      {
        delete[] pInput2Output[i].pOutputIndex;
        pInput2Output[i].pOutputIndex=NULL;
        pInput2Output[i].iCount=0;
      }
    }
    delete[] pInput2Output;
    pInput2Output=NULL;
    iInput2Output_Num=0;
  }


}

//int CFormat::LoadFmtDataToMem(CDatabase *Database,char *szDebugFlag,char *szOutTypeId,char chCheckBillSta_Cond,char *SourceGrp)
int CFormat::LoadFmtDataToMem(DBConnection connection,char *szDebugFlag,char *szOutTypeId,char chCheckBillSta_Cond,char *SourceGrp)
{
  sprintf(szDebug_Flag,"%s",szDebugFlag);
  //m_DataBase=Database;
   conn = connection;
  //if(!(dbConnect(conn)))
  //{
 //	cout<<"CFormat::LoadFmtDataToMem 数据库 connect error."<<endl;
	//	return -1 ;
 // }

  if(LoadTxtFileFmt(szOutTypeId,SourceGrp)) return -1;
  if(LoadTxt_Fmt(SourceGrp)) return -1;
  if(LoadInput2Output()) return -1;

  if(chCheckBillSta_Cond=='Y')
  	GetBillCond();
return 0;
}


int CFormat::LoadTxtFileFmt(char *szOutTypeId,char *SourceGrp)
{
  //CBindSQL ds(*m_DataBase);
  char szSqlStr[400];
  //ds.Open("select count(*) from C_SOURCE_GROUP_CONFIG a,I_SOURCE_DEFINE b where a.SOURCE_GROUP=:szPipeId and a.source_id=b.source_id", SELECT_QUERY);
  //ds<<SourceGrp;
  //ds>>iTxtFileFmtFefine_Num;
  //ds.Close();
 try
 {
	
  Statement stmt = conn.createStatement();
  string sql = "select count(*) from C_SOURCE_GROUP_CONFIG a,I_SOURCE_DEFINE b where a.SOURCE_GROUP=:szPipeId and a.source_id=b.source_id";
  stmt.setSQLString(sql);
  stmt<<SourceGrp;
  stmt.execute();
  stmt>>iTxtFileFmtFefine_Num;

  expTrace(szDebug_Flag, __FILE__, __LINE__, 
    "iTxtFileFmtFefine_Num=%d;", iTxtFileFmtFefine_Num);

  iTxtFileFmtFefine_Num += 1;
  if (iTxtFileFmtFefine_Num <= 0)
  {
    strcpy(szLogStr, "select count(*) from C_SOURCE_GROUP_CONFIG a,I_SOURCE_DEFINE b where a.SOURCE_GROUP=:szPipeId and a.source_id=b.source_id is Zero");
    expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
    pTxtFileFmtDefine=NULL;
    return 0;
  }

  pTxtFileFmtDefine = new STxtFileFmtDefine[iTxtFileFmtFefine_Num];
  if (!pTxtFileFmtDefine)
  {
    strcpy(szLogStr, "New pTxtFileFmtDefine fail.");
    expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
    return (-1);
  }

  //ds.Open("select file_fmt from C_SOURCE_GROUP_CONFIG a,I_SOURCE_DEFINE b where a.SOURCE_GROUP=:szPipeId and a.source_id=b.source_id ", SELECT_QUERY );
  //ds<<SourceGrp;
  sql = "select file_fmt from C_SOURCE_GROUP_CONFIG a,I_SOURCE_DEFINE b where a.SOURCE_GROUP=:szPipeId and a.source_id=b.source_id ";
  stmt.setSQLString(sql);
  stmt<<SourceGrp;
  stmt.execute();
   /* Repeat reading all record */
  for (int i=0; i<(iTxtFileFmtFefine_Num-1); i++)
  {
    //ds>>pTxtFileFmtDefine[i].szFile_Fmt_Name;
	stmt>>pTxtFileFmtDefine[i].szFile_Fmt_Name;

    delSpace(pTxtFileFmtDefine[i].szFile_Fmt_Name,0);
  }
  //ds.Close();

  sprintf(pTxtFileFmtDefine[iTxtFileFmtFefine_Num-1].szFile_Fmt_Name,"%s",szOutTypeId);

/*
  for(int i=0;i<SourceCfg.size(); i++ )
    sprintf(pTxtFileFmtDefine[iTxtFileFmtFefine_Num-1-i].szFile_Fmt_Name,"%s",Param.oRecordform[i].szOutFileTypeId);
*/  
  qsort(&pTxtFileFmtDefine[0],iTxtFileFmtFefine_Num,sizeof( struct STxtFileFmtDefine ),_CmpTxtFileFmt);

  for(int i=0;i<iTxtFileFmtFefine_Num; i++)
  {
      sprintf(szSqlStr,"select record_Type,sign_index,sign_mode,sign_begin,sign_len,seperator from \
      	C_TXTFILE_FMT_DEFINE where file_fmt='%s' and NODEID='%s' ",pTxtFileFmtDefine[i].szFile_Fmt_Name,FIRSTNODE);
      //ds.Open(szSqlStr, SELECT_QUERY);
      //ds>>pTxtFileFmtDefine[i].chRecordType>>pTxtFileFmtDefine[i].iSign_Index
      //	>>pTxtFileFmtDefine[i].iSign_Mode>>pTxtFileFmtDefine[i].iSign_Begin
      //	>>pTxtFileFmtDefine[i].iSign_Len>>pTxtFileFmtDefine[i].szSeperator;
      // ds.Close();
	  sql = szSqlStr;
	  stmt.setSQLString(sql);
	  stmt.execute();
	  stmt>>pTxtFileFmtDefine[i].chRecordType>>pTxtFileFmtDefine[i].iSign_Index
		  >>pTxtFileFmtDefine[i].iSign_Mode>>pTxtFileFmtDefine[i].iSign_Begin
		  >>pTxtFileFmtDefine[i].iSign_Len>>pTxtFileFmtDefine[i].szSeperator;
	
      if(LoadTxtSignNode(pTxtFileFmtDefine[i].szFile_Fmt_Name,FIRSTNODE,pTxtFileFmtDefine[i].iNextSign_Num,pTxtFileFmtDefine[i].pNext_Node)) return -1;
      if(LoadTxtRoute(i)) return -1;
      if(LoadTxtRecordFmt(i)) return -1;
  }
  stmt.close();
 }
 catch (SQLException e)
 {
		cout<<"CFormat::LoadTxtFileFmt 抛出异常: "<<e.what()<<endl;
		//conn.close();
		//stmt.close();
		throw jsexcp::CException(0, "CFormat::LoadTxtFileFmt 抛出异常:", __FILE__, __LINE__);	
		//return -1;
 }

  pCur_TxtFileFmtDefine=&pTxtFileFmtDefine[0];
  return 0;
}


int CFormat::LoadTxtSignNode(char *File_Fmt,char *szNodeId,int &iSonNode_Num,STxtSign_node *&pSonNode)
{

  //CBindSQL ds(*m_DataBase);
  char szSqlStr[400];
  sprintf(szSqlStr,"select count(*) from c_txtrecord_fmt_define where file_fmt='%s' and NODEID='%s' ",File_Fmt,szNodeId);
  //ds.Open(szSqlStr, SELECT_QUERY);
  //ds>>iSonNode_Num;
  //ds.Close();
  string sql = szSqlStr;
try
{
	
  Statement stmt = conn.createStatement();
  stmt.setSQLString(sql);
  stmt.execute();
  stmt>>iSonNode_Num;

  expTrace(szDebug_Flag, __FILE__, __LINE__,
    "NodeID = %s,iSonNode_Num=%d;", szNodeId,iSonNode_Num);

  if (iSonNode_Num <= 0)
  {
    strcpy(szLogStr, "%s is Zero");
    expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr,szSqlStr);
    pSonNode=NULL;
    return 0;
  }

  pSonNode = new STxtSign_node[iSonNode_Num];
  if (!pSonNode)
  {
    strcpy(szLogStr, "New pSonNode fail.");
    expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
    return (-1);
  }
  sprintf(szSqlStr,"select pri,SIGN_VALUE,RECORD_FMT,Leaftype,RECORD_LEN,nvl(expression,'@') from c_txtrecord_fmt_define \
  	where FILE_FMT='%s' and NODEID='%s' ",File_Fmt,szNodeId);
  //ds.Open(szSqlStr, SELECT_QUERY );
  sql = szSqlStr;
  stmt.setSQLString(sql);
  stmt.execute();

   /* Repeat reading all record */
  for (int i=0; i<iSonNode_Num; i++)
  {
    //ds>>pSonNode[i].iNodePri>>pSonNode[i].szPreSignValue
     // >>pSonNode[i].szNodeId>>pSonNode[i].chNodeType
     // >>pSonNode[i].iNodeLen>>pSonNode[i].szExpression;
	 stmt>>pSonNode[i].iNodePri>>pSonNode[i].szPreSignValue
		 >>pSonNode[i].szNodeId>>pSonNode[i].chNodeType
		 >>pSonNode[i].iNodeLen>>pSonNode[i].szExpression;

    delSpace(pSonNode[i].szNodeId,0);
    delSpace(pSonNode[i].szPreSignValue,0);
  }
  //ds.Close();

  qsort(&pSonNode[0],iSonNode_Num,sizeof( struct STxtSign_node ),_CmpTxtSignNode);


  for(int i=0;i<iSonNode_Num; i++)
  {
      if(pSonNode[i].chNodeType == LEAF_FLAG)
      {
        
        pSonNode[i].iSign_Index=-1;
        pSonNode[i].iSign_Mode=-1;
        pSonNode[i].iSign_Begin=-1;
        pSonNode[i].iSign_Len=-1;
        pSonNode[i].chRecordType=0;
        pSonNode[i].szSeperator[0]=0;
        pSonNode[i].iNextSign_Num=0;
        pSonNode[i].pNext_Node=NULL;
        continue;
      }

      sprintf(szSqlStr,"select record_Type,sign_index,sign_mode,sign_begin,sign_len,seperator from \
      	C_TXTFILE_FMT_DEFINE where file_fmt='%s' and NODEID='%s' ",File_Fmt,pSonNode[i].szNodeId);
      //ds.Open(szSqlStr, SELECT_QUERY);
     // ds>>pSonNode[i].chRecordType>>pSonNode[i].iSign_Index
     // 	>>pSonNode[i].iSign_Mode>>pSonNode[i].iSign_Begin
      //	>>pSonNode[i].iSign_Len>>pSonNode[i].szSeperator;
     // ds.Close();
	 sql = szSqlStr;
	 stmt.setSQLString(sql);
	 stmt.execute();
	 stmt>>pSonNode[i].chRecordType>>pSonNode[i].iSign_Index
		 >>pSonNode[i].iSign_Mode>>pSonNode[i].iSign_Begin
		 >>pSonNode[i].iSign_Len>>pSonNode[i].szSeperator;

      pSonNode[i].pNext_Node = NULL;
      if(LoadTxtSignNode(File_Fmt,pSonNode[i].szNodeId,pSonNode[i].iNextSign_Num,pSonNode[i].pNext_Node)) return -1;
  	}
  stmt.close();
 }
 catch (SQLException e)
 {
		cout<<"CFormat::LoadTxtSignNode 抛出异常: "<<e.what()<<endl;		
		throw jsexcp::CException(0, "CFormat::LoadTxtSignNode 抛出异常:", __FILE__, __LINE__);	
 }
  return 0;
}


int CFormat::LoadTxtRoute(int iSeq)
{

  //CBindSQL ds(*m_DataBase);
  char szSqlStr[400];
  sprintf(szSqlStr,"select count(*) from c_route_change where file_fmt='%s'",pTxtFileFmtDefine[iSeq].szFile_Fmt_Name);
  //ds.Open(szSqlStr, SELECT_QUERY);
  //ds>>(pTxtFileFmtDefine[iSeq].iRoute_Num);
  //ds.Close();
  string sql = szSqlStr;
try
{
  Statement stmt = conn.createStatement();
  stmt.setSQLString(sql);
  stmt.execute();
  stmt>>(pTxtFileFmtDefine[iSeq].iRoute_Num);

  expTrace(szDebug_Flag, __FILE__, __LINE__, 
    "pTxtFileFmtDefine[iSeq].iRoute_Num=%d;", pTxtFileFmtDefine[iSeq].iRoute_Num);

  if (pTxtFileFmtDefine[iSeq].iRoute_Num <= 0)
  {
    strcpy(szLogStr, "select count(*) from c_route_change where file_fmt=%s is Zero");
    expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr,pTxtFileFmtDefine[iSeq].szFile_Fmt_Name);
    pTxtFileFmtDefine[iSeq].pCheckRoute=NULL;
    return 0;
  }

  pTxtFileFmtDefine[iSeq].pCheckRoute = new SCheckRoute[pTxtFileFmtDefine[iSeq].iRoute_Num];
  if (!pTxtFileFmtDefine[iSeq].pCheckRoute)
  {
    strcpy(szLogStr, "New pTxtFileFmtDefine[iSeq].pCheckRoute fail.");
    expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
    return (-1);
  }

  sprintf(szSqlStr,"select calledno_header,route,outputroute from c_route_change \
  	where file_fmt='%s'",pTxtFileFmtDefine[iSeq].szFile_Fmt_Name); 
  //ds.Open(szSqlStr, SELECT_QUERY );
  sql = szSqlStr;
  stmt.setSQLString(sql);
  stmt.execute();

  for (int i=0; i<pTxtFileFmtDefine[iSeq].iRoute_Num; i++)
  {
    //ds>>pTxtFileFmtDefine[iSeq].pCheckRoute[i].szCalledNo_Header
    //  >>pTxtFileFmtDefine[iSeq].pCheckRoute[i].szRoute
    //  >>pTxtFileFmtDefine[iSeq].pCheckRoute[i].szOutputRoute;
	stmt>>pTxtFileFmtDefine[iSeq].pCheckRoute[i].szCalledNo_Header
		>>pTxtFileFmtDefine[iSeq].pCheckRoute[i].szRoute
		>>pTxtFileFmtDefine[iSeq].pCheckRoute[i].szOutputRoute;

    delSpace(pTxtFileFmtDefine[iSeq].pCheckRoute[i].szCalledNo_Header,0);
    delSpace(pTxtFileFmtDefine[iSeq].pCheckRoute[i].szRoute,0);
    delSpace(pTxtFileFmtDefine[iSeq].pCheckRoute[i].szOutputRoute,0);
  }
  //ds.Close();  
  stmt.close();
 }
 catch (SQLException e)
 {
		cout<<"CFormat::LoadTxtRoute 抛出异常: "<<e.what()<<endl;		
		throw jsexcp::CException(0, "CFormat::LoadTxtRoute 抛出异常:", __FILE__, __LINE__);	
 }
  return 0;

}

int CFormat::LoadTxtRecordFmt(int iSeq)
{
  //CBindSQL ds(*m_DataBase);
  char szSqlStr[400];

  sprintf(szSqlStr,"select count(*) from (select distinct RECORD_FMT from c_txtrecord_fmt_define \
  	where file_fmt = '%s' and leaftype = '%c')",pTxtFileFmtDefine[iSeq].szFile_Fmt_Name,LEAF_FLAG);
try
{
   string sql = szSqlStr;
  //ds.Open(szSqlStr, SELECT_QUERY);
  //ds>>(pTxtFileFmtDefine[iSeq].iTxtRecordFmt_Num);
  //ds.Close();
  Statement stmt = conn.createStatement();
  stmt.setSQLString(sql);
  stmt.execute();
  stmt>>(pTxtFileFmtDefine[iSeq].iTxtRecordFmt_Num);

  expTrace(szDebug_Flag, __FILE__, __LINE__, 
    "pTxtFileFmtDefine[iSeq].iTxtRecordFmt_Num=%d;", pTxtFileFmtDefine[iSeq].iTxtRecordFmt_Num);

  if (pTxtFileFmtDefine[iSeq].iTxtRecordFmt_Num <= 0)
  {
    strcpy(szLogStr, "select count(*) from c_Txtrecord_fmt_define where file_fmt=%s is Zero");
    expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr,pTxtFileFmtDefine[iSeq].szFile_Fmt_Name);
    pTxtFileFmtDefine[iSeq].pTxtRecordFmt=NULL;
    return -1;
  }

  pTxtFileFmtDefine[iSeq].pTxtRecordFmt = new STxtRecordFmtDefine[pTxtFileFmtDefine[iSeq].iTxtRecordFmt_Num];
  if (!pTxtFileFmtDefine[iSeq].pTxtRecordFmt)
  {
    strcpy(szLogStr, "New pTxtFileFmtDefine[iSeq].pTxtRecordFmt fail.");
    expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
    return (-1);
  }

  sprintf(szSqlStr,"select distinct RECORD_FMT from c_txtrecord_fmt_define where file_fmt = '%s' and leaftype = '%c'",pTxtFileFmtDefine[iSeq].szFile_Fmt_Name,LEAF_FLAG); 
  //ds.Open(szSqlStr, SELECT_QUERY );
  sql = szSqlStr;
  stmt.setSQLString(sql);
  stmt.execute();

   /* Repeat reading all record */
  for (int i=0; i<pTxtFileFmtDefine[iSeq].iTxtRecordFmt_Num; i++)
  {
    //ds>>pTxtFileFmtDefine[iSeq].pTxtRecordFmt[i].szRecord_Name;
	stmt>>pTxtFileFmtDefine[iSeq].pTxtRecordFmt[i].szRecord_Name;

    pTxtFileFmtDefine[iSeq].pTxtRecordFmt[i].iRecord_Len=0;
    pTxtFileFmtDefine[iSeq].pTxtRecordFmt[i].szExp_Num = 0;

    delSpace(pTxtFileFmtDefine[iSeq].pTxtRecordFmt[i].szRecord_Name,0);
  }
  //ds.Close();
  stmt.close();
 }
 catch (SQLException e)
 {
		cout<<"CFormat::LoadTxtRecordFmt 抛出异常: "<<e.what()<<endl;		
		throw jsexcp::CException(0, "CFormat::LoadTxtRecordFmt 抛出异常:", __FILE__, __LINE__);	
 }

  qsort(pTxtFileFmtDefine[iSeq].pTxtRecordFmt,pTxtFileFmtDefine[iSeq].iTxtRecordFmt_Num,sizeof( struct STxtRecordFmtDefine ),_CmpTxtRecordFmt);

  pTxtFileFmtDefine[iSeq].pCur_TxtRecordFmt=&(pTxtFileFmtDefine[iSeq].pTxtRecordFmt[0]);
  return 0;
}


int CFormat::LoadTxt_Fmt(char *SourceGrp)
{
  //CBindSQL ds(*m_DataBase);
  //ds.Open("select count(*) from C_SOURCE_GROUP_CONFIG a,I_SOURCE_DEFINE b where a.SOURCE_GROUP=:szPipeId and a.source_id=b.source_id ", SELECT_QUERY);
  //ds<<SourceGrp;
  //ds>>iTxtFmt_Num;
  //ds.Close();
 try
 {
  Statement stmt = conn.createStatement();
  string sql = "select count(*) from C_SOURCE_GROUP_CONFIG a,I_SOURCE_DEFINE b where a.SOURCE_GROUP=:szPipeId and a.source_id=b.source_id ";
  stmt.setSQLString(sql);
  stmt<<SourceGrp;
  stmt.execute();
  stmt>>iTxtFmt_Num;

  expTrace(szDebug_Flag, __FILE__, __LINE__, 
    "iTxtFmt_Num=%d;", iTxtFmt_Num);

  if (iTxtFmt_Num <= 0)
  {
    strcpy(szLogStr, "select count(*) from C_SOURCE_GROUP_CONFIG a,I_SOURCE_DEFINE b where a.SOURCE_GROUP=:szPipeId and a.source_id=b.source_id is Zero");
    expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
    pTxtFmt=NULL;
	return -1;
  }

  pTxtFmt = new STxtFmt[iTxtFmt_Num];
  if (!pTxtFmt)
  {
    strcpy(szLogStr, "New pTxtFmt fail.");
    expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
    return (-1);
  }

  //ds.Open("select file_fmt from C_SOURCE_GROUP_CONFIG a,I_SOURCE_DEFINE b where a.SOURCE_GROUP=:szPipeId and a.source_id=b.source_id", SELECT_QUERY );
  //ds<<SourceGrp;
  sql = "select file_fmt from C_SOURCE_GROUP_CONFIG a,I_SOURCE_DEFINE b where a.SOURCE_GROUP=:szPipeId and a.source_id=b.source_id";
  stmt.setSQLString(sql);
  stmt<<SourceGrp;

   /* Repeat reading all record */
  for (int i=0; i<iTxtFmt_Num; i++)
  {
    //ds>>pTxtFmt[i].szFileTypeID;
	stmt>>pTxtFmt[i].szFileTypeID;

    delSpace(pTxtFmt[i].szFileTypeID,0);
  }
  //ds.Close();

  qsort(&pTxtFmt[0],iTxtFmt_Num,sizeof( struct STxtFmt ),_CmpTxtFmt);

  for(int i=0;i<iTxtFmt_Num; i++)
  {
      //ds.Open("select RECORD_TYPE from C_FILETYPE_DEFINE \
      //	where FILETYPE_ID=:szPipeId ", SELECT_QUERY);
      //ds<<pTxtFmt[i].szFileTypeID;
      //ds>>pTxtFmt[i].chRecordType;
     // ds.Close();
	 sql = "select RECORD_TYPE from C_FILETYPE_DEFINE where FILETYPE_ID=:szPipeId ";
	 stmt<<pTxtFmt[i].szFileTypeID;
	 stmt.execute();
	 stmt>>pTxtFmt[i].chRecordType;
  }

 stmt.close();
 }
 catch (SQLException e)
 {
		cout<<"CFormat::LoadTxt_Fmt 抛出异常: "<<e.what()<<endl;		
		throw jsexcp::CException(0, "CFormat::LoadTxt_Fmt 抛出异常:", __FILE__, __LINE__);	
 }

  pCur_TxtFmt=&pTxtFmt[0];
  return 0;
}

char CFormat::Get_Txtfmt_RecordType(char *FileTypeID)
{
  if(!iTxtFmt_Num) return ' ';
  if(!strcmp(pCur_TxtFmt->szFileTypeID,FileTypeID))
  	return pCur_TxtFmt->chRecordType;
  STxtFmt  sTxtFmt;
  strcpy(sTxtFmt.szFileTypeID,FileTypeID);
  pCur_TxtFmt=((STxtFmt *)(bsearch(&sTxtFmt,pTxtFmt,iTxtFmt_Num,sizeof(struct STxtFmt),_CmpTxtFmt)));
  if(!pCur_TxtFmt)
  	{
  	pCur_TxtFmt=&pTxtFmt[0];
  	return ' ';
  	}
  return pCur_TxtFmt->chRecordType;
}

int CFormat::LoadInput2Output()
{
  //CBindSQL ds(*m_DataBase);
  //ds.Open("select count(*) from (select distinct INPUT_ID,output_id from c_input2output)", SELECT_QUERY);
//  ds.Open("select count(*) from (select distinct INPUT_ID,output_id from input2output where input_id<>output_id)", SELECT_QUERY);
  //ds>>iInput2Output_Num;
  //ds.Close();
 try
 {
  Statement stmt = conn.createStatement();
  string sql = "select count(*) from (select distinct INPUT_ID,output_id from c_input2output)";
  stmt.setSQLString(sql);  
  stmt.execute();
  stmt>>iInput2Output_Num;

  expTrace(szDebug_Flag, __FILE__, __LINE__, 
    "iInput2Output_Num=%d;", iInput2Output_Num);

  if (iInput2Output_Num <= 0)
  {
    strcpy(szLogStr, "select count(*) from (select distinct INPUT_ID,output_id from c_input2output where input_id<>output_id) is Zero");
    expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
    pInput2Output=NULL;
    return -1;
  }

  pInput2Output = new SInput2Output[iInput2Output_Num];
  if (!pInput2Output)
  {
    strcpy(szLogStr, "New pInput2Output fail.");
    expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
    return (-1);
  }

  //ds.Open("select distinct input_id,output_id from input2output where input_id <>output_id", SELECT_QUERY );
  //ds.Open("select distinct input_id,output_id from c_input2output", SELECT_QUERY );
   sql = "select distinct input_id,output_id from c_input2output";
   stmt.setSQLString(sql);
   stmt.execute();

   /* Repeat reading all record */
  for (int i=0; i<iInput2Output_Num; i++)
    //ds>>pInput2Output[i].szInputType>>pInput2Output[i].szOutputType;
     stmt>>pInput2Output[i].szInputType>>pInput2Output[i].szOutputType;
  //ds.Close();

  qsort(&pInput2Output[0],iInput2Output_Num,sizeof( struct SInput2Output ),_CmpInput2Output);

  for(int i=0;i<iInput2Output_Num; i++)
  {
    //ds.Open("select count(*) from c_input2output where \
    //  input_id=:input and output_id=:output ", SELECT_QUERY);
    //ds<<pInput2Output[i].szInputType<<pInput2Output[i].szOutputType;
    //ds>>pInput2Output[i].iCount;
    //ds.Close();
	sql = "select count(*) from c_input2output where input_id=:input and output_id=:output ";
	stmt.setSQLString(sql);
	stmt<<pInput2Output[i].szInputType<<pInput2Output[i].szOutputType;
	stmt.execute();
	stmt>>pInput2Output[i].iCount;
    
    if (pInput2Output[i].iCount <= 0)
    {
      strcpy(szLogStr, "select count(*) from c_input2output where \
        input_id=:input and output_id=:output is Zero");
      expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
      return -1;
    }

    pInput2Output[i].pInputIndex = new int[pInput2Output[i].iCount];
    if (!pInput2Output[i].pInputIndex)
    {
      strcpy(szLogStr, "New pInput2Output[i].pInputIndex fail.");
      expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
      return (-1);
    }
    pInput2Output[i].pOutputIndex = new int[pInput2Output[i].iCount];
    if (!pInput2Output[i].pOutputIndex)
    {
      strcpy(szLogStr, "New pInput2Output[i].pOutputIndex fail.");
      expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
      return (-1);
    }
    pInput2Output[i].pOutIdx_Index = new int[pInput2Output[i].iCount];
    if (!pInput2Output[i].pOutIdx_Index)
    {
      strcpy(szLogStr, "New pInput2Output[i].pOutIdx_Index fail.");
      expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
      return (-1);
    }
    pInput2Output[i].psubstrInputlen = new int[pInput2Output[i].iCount];
    if (!pInput2Output[i].psubstrInputlen)
    {
      strcpy(szLogStr, "New pInput2Output[i].psubstrInputlen fail.");
      expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
      return (-1);
    }
    
    pInput2Output[i].pszInputValue = new BILLSFC_GROUP[pInput2Output[i].iCount];
    if (!pInput2Output[i].pszInputValue)
    {
      strcpy(szLogStr, "New pInput2Output[i].pszInputValue fail.");
      expTrace(szDebug_Flag, __FILE__, __LINE__, szLogStr);
      return (-1);
    }

    
    //ds.Open("select input_index,output_index,nvl(outfield_index,1),nvl(subinputlen,0),nvl(inputvalue,'') from c_input2output where \
    //  input_id=:input and output_id=:output order by output_index,outfield_index", SELECT_QUERY);
    //ds<<pInput2Output[i].szInputType<<pInput2Output[i].szOutputType;
    sql = "select input_index,output_index,nvl(outfield_index,1),nvl(subinputlen,0),nvl(inputvalue,'') from c_input2output where "
		  "input_id=:input and output_id=:output order by output_index,outfield_index";
	stmt.setSQLString(sql);
	stmt<<pInput2Output[i].szInputType<<pInput2Output[i].szOutputType;
	stmt.execute();

    for (int j=0; j<pInput2Output[i].iCount; j++)
      //ds>>pInput2Output[i].pInputIndex[j]>>pInput2Output[i].pOutputIndex[j]>>pInput2Output[i].pOutIdx_Index[j]
      //	>>pInput2Output[i].psubstrInputlen[j]>>pInput2Output[i].pszInputValue[j].szValue;
	  stmt>>pInput2Output[i].pInputIndex[j]>>pInput2Output[i].pOutputIndex[j]>>pInput2Output[i].pOutIdx_Index[j]
		  >>pInput2Output[i].psubstrInputlen[j]>>pInput2Output[i].pszInputValue[j].szValue;
    //ds.Close();
    
  }
  stmt.close();
 }
 catch (SQLException e)
 {
		cout<<"CFormat::LoadInput2Output 抛出异常: "<<e.what()<<endl;		
		throw jsexcp::CException(0, "CFormat::LoadInput2Output 抛出异常:", __FILE__, __LINE__);	
 }

  pCur_Input2Output=&pInput2Output[0];
  return 0;

}
SInput2Output* CFormat::GetCurInput2Output(char *szInType,char *szOutType)
{

  SInput2Output sInput2Output;
  sprintf(sInput2Output.szInputType,"%s",szInType);
  sprintf(sInput2Output.szOutputType,"%s",szOutType);
  if(strcmp(pCur_Input2Output->szInputType,sInput2Output.szInputType)||strcmp(pCur_Input2Output->szOutputType,sInput2Output.szOutputType))
  {
    pCur_Input2Output=((SInput2Output *)(bsearch(&sInput2Output,pInput2Output,iInput2Output_Num,sizeof(struct SInput2Output),_CmpInput2Output)));
  }
  if(!pCur_Input2Output)
  {
    pCur_Input2Output=&pInput2Output[0];
  	return NULL;
  }
  return pCur_Input2Output;
}

 //2013-10-27 增加将头尾字段塞到话单记录里面
int CFormat::Set_OutRcd(CFmt_Change &inrcd,CFmt_Change &outrcd,vector<string> vhead,vector<string> vtail)
{
  int iIsError=0;

//  if(GetCurInput2Output(inrcd.Get_id(),outrcd.Get_id())==NULL) return -1;
  
  int iInput_Count=inrcd.Get_fieldcount();
  
  for(int i=0;i<iInput_Count;i++)
  {
	sprintf(Field[i],"%s",inrcd.Get_Field(i+1));

  }

//市化亲情被叫加号码头
///3.20061207 SHQQ被叫号码头为17909(P)、96688(m)的不加号码头
    if(SHQQCalledNoIdx!=0)
    {
      char TmpField[1024];
      if(SHQQRcdType=='P')
      {
        if(strncmp(Field[SHQQCalledNoIdx],"17909",5))
        {
          sprintf(TmpField,"17909%s",Field[SHQQCalledNoIdx]);
          sprintf(Field[SHQQCalledNoIdx],"%s",TmpField);
        }
      }
      if(SHQQRcdType=='m')
      {
        if(strncmp(Field[SHQQCalledNoIdx],"96688",5))
        {
          sprintf(TmpField,"96688%s",Field[SHQQCalledNoIdx]);
          sprintf(Field[SHQQCalledNoIdx],"%s",TmpField);
        }
      }
    }
    SHQQRcdType=0;
    SHQQCalledNoIdx=0;


  char temp[256];
  for(int i=0;i<pCur_Input2Output->iCount;i++)
  {
	  //头尾时的截取没实现
	  if(pCur_Input2Output->pInputIndex[i] > 1000)
	  { 	   
			memset(temp,0,sizeof(temp));

			if(pCur_Input2Output->pInputIndex[i] < 9000)
			{
				sprintf(temp,"%s",vhead[(pCur_Input2Output->pInputIndex[i])%100-1]);
				//theJSLog<<"head "<<pCur_Input2Output->pInputIndex[i]%100<<"="<<temp<<endi;
			}
			else
		    {	
				sprintf(temp,"%s",vtail[(pCur_Input2Output->pInputIndex[i])%100-1]);
				//theJSLog<<"tail "<<pCur_Input2Output->pInputIndex[i]%100<<"="<<temp<<endi;
			}

			if((strlen(temp) > outrcd.Get_FieldLen(pCur_Input2Output->pOutputIndex[i])) &&(pCur_Input2Output->psubstrInputlen[i]> outrcd.Get_FieldLen(pCur_Input2Output->pOutputIndex[i])))
			{
				temp[outrcd.Get_FieldLen(pCur_Input2Output->pOutputIndex[i])] = 0;
	   			iIsError = pCur_Input2Output->pInputIndex[i];
			}
			else
			{
				if(pCur_Input2Output->psubstrInputlen[i] > 0)
				{
	   				temp[pCur_Input2Output->psubstrInputlen[i]] = 0;
      			}
			}
			
			if(pCur_Input2Output->pOutIdx_Index[i]>1)
			{
				try
				{
					outrcd.Field_Add(pCur_Input2Output->pOutputIndex[i],temp);
				}
				catch(CException e)
				{
					iIsError=pCur_Input2Output->pInputIndex[i];
				}
			}
			else
			{
				try
				{
					outrcd.Set_Field(pCur_Input2Output->pOutputIndex[i],temp);
				}
				catch(CException e)
				{
					iIsError=pCur_Input2Output->pInputIndex[i];
				}
			}

			continue;
		}
	 
	  if(pCur_Input2Output->pInputIndex[i] == 0)
	  {
	      if(pCur_Input2Output->pOutIdx_Index[i]>1)
	      {
	        try
	        {
	          outrcd.Field_Add(pCur_Input2Output->pOutputIndex[i],pCur_Input2Output->pszInputValue[i].szValue);
	        }
	        catch(CException e)
	        {
	          iIsError=pCur_Input2Output->pInputIndex[i];
	        }
	      }
	      else
	      {
	        try
	        {
	          outrcd.Set_Field(pCur_Input2Output->pOutputIndex[i],pCur_Input2Output->pszInputValue[i].szValue);
	        }
	        catch(CException e)
	        {
	          iIsError=pCur_Input2Output->pInputIndex[i];
	        }
		  }
	      continue;
	  }

  
      if((strlen(Field[pCur_Input2Output->pInputIndex[i]-1]) > outrcd.Get_FieldLen(pCur_Input2Output->pOutputIndex[i]))
      	&&(pCur_Input2Output->psubstrInputlen[i]> outrcd.Get_FieldLen(pCur_Input2Output->pOutputIndex[i])))
      {
	   	  Field[pCur_Input2Output->pInputIndex[i]-1][outrcd.Get_FieldLen(pCur_Input2Output->pOutputIndex[i])] = 0;
	   	  iIsError = pCur_Input2Output->pInputIndex[i];
      }
      else
      {
      	if(pCur_Input2Output->psubstrInputlen[i] > 0)
      	{
	   	  Field[pCur_Input2Output->pInputIndex[i]-1][pCur_Input2Output->psubstrInputlen[i]] = 0;
      	}
      }
      if(pCur_Input2Output->pOutIdx_Index[i]>1)
      {
        try
        {
          outrcd.Field_Add(pCur_Input2Output->pOutputIndex[i],Field[pCur_Input2Output->pInputIndex[i]-1]);
        }
        catch(CException e)
        {
          iIsError=pCur_Input2Output->pInputIndex[i];
        }
      }
      else
      {
        try
        {
          outrcd.Set_Field(pCur_Input2Output->pOutputIndex[i],Field[pCur_Input2Output->pInputIndex[i]-1]);
        }
        catch(CException e)
        {
          iIsError=pCur_Input2Output->pInputIndex[i];
        }
	  }
  }
  
  return iIsError;
  
}

STxtFileFmtDefine* CFormat::Get_CurTxtFileFmt(char *szFile_Fmt)
{
  if(!iTxtFileFmtFefine_Num) return NULL;
  STxtFileFmtDefine sTxtFileFmtDefine;
  sprintf(sTxtFileFmtDefine.szFile_Fmt_Name,"%s",szFile_Fmt);
  if(strcmp(sTxtFileFmtDefine.szFile_Fmt_Name,pCur_TxtFileFmtDefine->szFile_Fmt_Name))
  {
    pCur_TxtFileFmtDefine=(STxtFileFmtDefine *)(bsearch(&sTxtFileFmtDefine,pTxtFileFmtDefine,iTxtFileFmtFefine_Num,sizeof(struct STxtFileFmtDefine),_CmpTxtFileFmt));
  }
  if(!pCur_TxtFileFmtDefine)
  {
    pCur_TxtFileFmtDefine=&pTxtFileFmtDefine[0];
    return NULL;
  }
  
  return pCur_TxtFileFmtDefine;
}

STxtRecordFmtDefine* CFormat::Get_CurTxtRecordFmt(char* szRecord)
{
  if(!iTxtFileFmtFefine_Num) return NULL;
  if(!pCur_TxtFileFmtDefine->iTxtRecordFmt_Num) return NULL;
  STxtRecordFmtDefine sTxtRecordFmt;
  STxtSign_node *pRecordFmt_Node;
  char szSign[11];
  Get_TxtSign_Value(szRecord,pCur_TxtFileFmtDefine->chRecordType,pCur_TxtFileFmtDefine->iSign_Index,pCur_TxtFileFmtDefine->iSign_Mode,pCur_TxtFileFmtDefine->iSign_Begin,pCur_TxtFileFmtDefine->iSign_Len,pCur_TxtFileFmtDefine->szSeperator,szSign);

  pRecordFmt_Node=Get_TxtNode(szRecord,pCur_TxtFileFmtDefine->iNextSign_Num,pCur_TxtFileFmtDefine->pNext_Node,szSign);
  if(!pRecordFmt_Node) return NULL;

  if(strcmp(pRecordFmt_Node->szNodeId,pCur_TxtFileFmtDefine->pCur_TxtRecordFmt->szRecord_Name))
  {
    sprintf(sTxtRecordFmt.szRecord_Name,"%s",pRecordFmt_Node->szNodeId);
    pCur_TxtFileFmtDefine->pCur_TxtRecordFmt=(STxtRecordFmtDefine *)(bsearch(&sTxtRecordFmt,pCur_TxtFileFmtDefine->pTxtRecordFmt,pCur_TxtFileFmtDefine->iTxtRecordFmt_Num,sizeof(struct STxtRecordFmtDefine),_CmpTxtRecordFmt));
  }
  if(!pCur_TxtFileFmtDefine->pCur_TxtRecordFmt)
  {
    pCur_TxtFileFmtDefine->pCur_TxtRecordFmt=&pCur_TxtFileFmtDefine->pTxtRecordFmt[0];
    return NULL;
  }
  pCur_TxtFileFmtDefine->pCur_TxtRecordFmt->iRecord_Len=pRecordFmt_Node->iNodeLen;

  strcpy(pCur_TxtFileFmtDefine->pCur_TxtRecordFmt->szExpression,pRecordFmt_Node->szExpression);

  if(pCur_TxtFileFmtDefine->pCur_TxtRecordFmt->szExp_Num == 0)
  {
//20061010 修改expression字段定义
//分解szExpression
    char *s1,*s2,*s3;
    s1 = pCur_TxtFileFmtDefine->pCur_TxtRecordFmt->szExpression;
    while(1)
    {
      s2 = strchr(s1,EXP_SPLIT);
      if(s2 == NULL)
      {
        sprintf(pCur_TxtFileFmtDefine->pCur_TxtRecordFmt->szExp[pCur_TxtFileFmtDefine->pCur_TxtRecordFmt->szExp_Num],"\t\t%s",s1);
//20061010 修改expression字段定义
//add by zhoulh
        s3 = strchr(pCur_TxtFileFmtDefine->pCur_TxtRecordFmt->szExp[pCur_TxtFileFmtDefine->pCur_TxtRecordFmt->szExp_Num],EXP_ERR_SPLIT);
        if(s3 == NULL) pCur_TxtFileFmtDefine->pCur_TxtRecordFmt->iExp_Errtype_Index[pCur_TxtFileFmtDefine->pCur_TxtRecordFmt->szExp_Num] = 0;
        else
        {
          *s3=0;
          s3++;
          pCur_TxtFileFmtDefine->pCur_TxtRecordFmt->iExp_Errtype_Index[pCur_TxtFileFmtDefine->pCur_TxtRecordFmt->szExp_Num] = atoi(s3);
        }
//end of added by zhoulh
        pCur_TxtFileFmtDefine->pCur_TxtRecordFmt->szExp_Num++;
        break;
      }
      *s2 = 0;
      sprintf(pCur_TxtFileFmtDefine->pCur_TxtRecordFmt->szExp[pCur_TxtFileFmtDefine->pCur_TxtRecordFmt->szExp_Num],"\t\t%s",s1);
//20061010 修改expression字段定义
//add by zhoulh
        s3 = strchr(pCur_TxtFileFmtDefine->pCur_TxtRecordFmt->szExp[pCur_TxtFileFmtDefine->pCur_TxtRecordFmt->szExp_Num],EXP_ERR_SPLIT);
        if(s3 == NULL) pCur_TxtFileFmtDefine->pCur_TxtRecordFmt->iExp_Errtype_Index[pCur_TxtFileFmtDefine->pCur_TxtRecordFmt->szExp_Num] = 0;
        else
        {
          *s3=0;
          s3++;
          pCur_TxtFileFmtDefine->pCur_TxtRecordFmt->iExp_Errtype_Index[pCur_TxtFileFmtDefine->pCur_TxtRecordFmt->szExp_Num] = atoi(s3);
        }
//end of added by zhoulh
      pCur_TxtFileFmtDefine->pCur_TxtRecordFmt->szExp_Num++;
      *s2 = EXP_SPLIT;
      s1 = s2 + 1;
    }
  }
  return pCur_TxtFileFmtDefine->pCur_TxtRecordFmt;
}

STxtSign_node* CFormat::Get_TxtNode(char *szRecord,int iSign_Num,STxtSign_node *pNode,char *iSign)
{
  for(int i=0;i<iSign_Num;i++)
  {
    if(strcmp(pNode[i].szPreSignValue ,iSign)) continue;
    if(pNode[i].chNodeType==LEAF_FLAG) return &pNode[i];
    char szNodeSign[11];
    Get_TxtSign_Value(szRecord,pNode[i].chRecordType,pNode[i].iSign_Index,pNode[i].iSign_Mode,pNode[i].iSign_Begin,pNode[i].iSign_Len,pNode[i].szSeperator,szNodeSign);
    if(pNode[i].chNodeType==OPRATION_FLAG)
    {
    iSign2_Begin=pNode[i].iSign_Index;
    iSign2_Len=strlen(szNodeSign);
    iSign2_Value=atoi(szNodeSign);
    }
    STxtSign_node *pRecordFmt_Node;
    pRecordFmt_Node=Get_TxtNode(szRecord,pNode[i].iNextSign_Num,pNode[i].pNext_Node,szNodeSign);
    if(pRecordFmt_Node != NULL) return pRecordFmt_Node;
  }

  
  iSign[0]='D';
  iSign[1]=0;
  for(int i=0;i<iSign_Num;i++)
  {
    if(strcmp(pNode[i].szPreSignValue ,iSign)) continue;
    if(pNode[i].chNodeType==LEAF_FLAG) return &pNode[i];
    char szNodeSign[11];
    Get_TxtSign_Value(szRecord,pNode[i].chRecordType,pNode[i].iSign_Index,pNode[i].iSign_Mode,pNode[i].iSign_Begin,pNode[i].iSign_Len,pNode[i].szSeperator,szNodeSign);
    if(pNode[i].chNodeType==OPRATION_FLAG)
    {
    iSign2_Begin=pNode[i].iSign_Index;
    iSign2_Len=strlen(szNodeSign);
    iSign2_Value=atoi(szNodeSign);
    }
    STxtSign_node *pRecordFmt_Node;
    pRecordFmt_Node=Get_TxtNode(szRecord,pNode[i].iNextSign_Num,pNode[i].pNext_Node,szNodeSign);
    if(pRecordFmt_Node != NULL) return pRecordFmt_Node;
  }
  
return NULL;
}

int CFormat::GetBillCond()
{
	//CBindSQL ds(*m_DataBase);
    int tmpCHECK_FIELDNUM=0;
	//ds.Open("SELECT  count(*) FROM user_tab_columns WHERE table_name='C_BILLSTA_FILT_COND'", SELECT_QUERY);			
    //ds>>tmpCHECK_FIELDNUM;
	//ds.Close();
try
{
	 Statement stmt = conn.createStatement();
	 string sql = "SELECT  count(*) FROM user_tab_columns WHERE table_name='C_BILLSTA_FILT_COND'";
	 stmt.setSQLString(sql);
	 stmt>>tmpCHECK_FIELDNUM;

	if(tmpCHECK_FIELDNUM==0)
    {
		iBCondNum=0;
		iBCond_CurIndex=0;
		return 0;
    }

  	//ds.Open("select count(*) from (select distinct inrcdfmt from C_BILLSTA_FILT_COND)", SELECT_QUERY);
	//ds>>iBCondNum;
  	//ds.Close();
	sql = "select count(*) from (select distinct inrcdfmt from C_BILLSTA_FILT_COND)";
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>iBCondNum;

  	expTrace(szDebug_Flag, __FILE__, __LINE__, 
    	"iBCondNum=%d;", iBCondNum);
  	if(iBCondNum <= 0)
  	{
		iBCondNum=0;
		iBCond_CurIndex=0;
		return 0;
 	}

  	//ds.Open("select distinct inrcdfmt from C_BILLSTA_FILT_COND", SELECT_QUERY);
	sql = "select distinct inrcdfmt from C_BILLSTA_FILT_COND";
	stmt.setSQLString(sql);
	stmt.execute();

  	for (int i=0; i<iBCondNum; i++)
  	{
  		BILLSTA_FILT_COND tmpBCond;
    	//ds>>tmpBCond.szinRcdFmt;
		stmt>>tmpBCond.szinRcdFmt;

    	GetBillSFC_RcdFmt(tmpBCond);
    	mbCond.push_back(tmpBCond);
  	}
  	//ds.Close();
    stmt.close();
 }
 catch (SQLException e)
 {
		cout<<"CFormat::GetBillCond 抛出异常: "<<e.what()<<endl;		
		throw jsexcp::CException(0, "CFormat::GetBillCond 抛出异常:", __FILE__, __LINE__);	
 }
	iBCond_CurIndex=0;
	return 0;

}



int CFormat::GetBillSFC_RcdFmt(BILLSTA_FILT_COND &tmpBCond)
{

 try
 {	
	//CBindSQL ds(*m_DataBase);
	//CBindSQL bs(*m_DataBase);
    int tmpCHECK_FIELDNUM=0;
    char szSqlStr[400];
    char szSqlStr1[400];
    int inGrp_FIELDNUM=0;
  	sprintf(szSqlStr,"select count(*) from (select distinct cond_group from C_BILLSTA_FILT_COND where inrcdfmt = '%s')",tmpBCond.szinRcdFmt);
  	//ds.Open(szSqlStr, SELECT_QUERY);
	//ds>>tmpCHECK_FIELDNUM;
  	//ds.Close();
	Statement stmt = conn.createStatement();
	Statement stmt2 = conn.createStatement();
    string sql = szSqlStr;
	stmt.setSQLString(sql);
	stmt.execute();
	stmt>>tmpCHECK_FIELDNUM;

  	sprintf(szSqlStr,"select distinct cond_group from C_BILLSTA_FILT_COND where inrcdfmt = '%s' ",tmpBCond.szinRcdFmt);
  	//ds.Open(szSqlStr, SELECT_QUERY);
	sql = szSqlStr;
	stmt.setSQLString(sql);
	stmt.execute();

  	for (int i=0; i<tmpCHECK_FIELDNUM; i++)
  	{
  		BILLSFC_RCDFmt tmpBCond_rcdfmt;
    	//ds>>tmpBCond_rcdfmt.iGroup;
		stmt>>tmpBCond_rcdfmt.iGroup;

//read BILLSFC_GROUP
  		sprintf(szSqlStr1,"select count(*) from (select colindex,value from C_BILLSTA_FILT_COND where inrcdfmt = '%s' and cond_group = '%d')",tmpBCond.szinRcdFmt,tmpBCond_rcdfmt.iGroup);
  		//bs.Open(szSqlStr1, SELECT_QUERY);
		//bs>>inGrp_FIELDNUM;
  		//bs.Close();
		sql = szSqlStr1;
		stmt2.setSQLString(sql);
		stmt2.execute();
		stmt2>>inGrp_FIELDNUM;

  		sprintf(szSqlStr1,"select colindex,value from C_BILLSTA_FILT_COND where inrcdfmt = '%s' and cond_group = '%d'",tmpBCond.szinRcdFmt,tmpBCond_rcdfmt.iGroup);
  		//bs.Open(szSqlStr1, SELECT_QUERY);
		sql = szSqlStr1;
		stmt2.setSQLString(sql);
		stmt2.execute();

	  	for(int j=0;j<inGrp_FIELDNUM; j++)
	  	{
			BILLSFC_GROUP tmpBCond_Group;
			//bs>>tmpBCond_Group.iIndex>>tmpBCond_Group.szValue;
			stmt2>>tmpBCond_Group.iIndex>>tmpBCond_Group.szValue;

			tmpBCond_rcdfmt.mBillSFC_Group.push_back(tmpBCond_Group);
	  	}
		//bs.Close();
//
    	tmpBCond.mBillSFC_RcdFmt.push_back(tmpBCond_rcdfmt);
  	}
  	//ds.Close();
   stmt2.close();
   stmt.close();
 }
 catch (SQLException e)
 {
		cout<<"CFormat::GetBillSFC_RcdFmt 抛出异常: "<<e.what()<<endl;		
		throw jsexcp::CException(0, "CFormat::GetBillSFC_RcdFmt 抛出异常:", __FILE__, __LINE__);	
 }
	return 0;

}
int CFormat::CheckBillSFC(CFmt_Change &inrcd)
{
  if(!iBCondNum) return 0;
  char InRcdFmt[6];
  strcpy(InRcdFmt,inrcd.Get_id());
  if(strcmp(mbCond[iBCond_CurIndex].szinRcdFmt,InRcdFmt))
  {
    for(iBCond_CurIndex=0;iBCond_CurIndex<iBCondNum;iBCond_CurIndex++)
    {
      if(!strcmp(mbCond[iBCond_CurIndex].szinRcdFmt,InRcdFmt)) break;
    }
  }

  if(iBCond_CurIndex==iBCondNum) 
  {
    iBCond_CurIndex=0;
    return 0;
  }

  for(int i=0;i<mbCond[iBCond_CurIndex].mBillSFC_RcdFmt.size();i++)
  {
    int j=0;
    for(j=0;j<mbCond[iBCond_CurIndex].mBillSFC_RcdFmt[i].mBillSFC_Group.size();j++)
    {
      if(strcmp(mbCond[iBCond_CurIndex].mBillSFC_RcdFmt[i].mBillSFC_Group[j].szValue,inrcd.Get_Field(mbCond[iBCond_CurIndex].mBillSFC_RcdFmt[i].mBillSFC_Group[j].iIndex))) break;
    }
	if(j==mbCond[iBCond_CurIndex].mBillSFC_RcdFmt[i].mBillSFC_Group.size()) return 1;
  }

  return 0;

}

static void Get_TxtSign_Value(char *szRecord,char chRecordType,int iSign_Index,int iSign_Mode,int iSign_Begin,
	int iSign_Len,char *szSeperator,char *szTxtSign_Value)
{

  if(chRecordType == ARECORDTYPE)
  {
    switch(iSign_Mode)
    {
      case 1:
      	if(iSign_Len > 10) iSign_Len=10;
        strncpy(szTxtSign_Value,szRecord+iSign_Begin-1,iSign_Len);
        szTxtSign_Value[iSign_Len]=0;
        break;
      default:
      	szTxtSign_Value[0]='D';
      	szTxtSign_Value[1]=0;
      	break;
      	
    } 
    return;
  }

  char *szField_Begin,*szField_End;
  int i;
  int iTmp_Len;
  szField_Begin=szRecord;
  char szTmp_Field[string_len];
  
  if(chRecordType == CRECORDTYPE)
  {
    switch(iSign_Mode)
    {
////1    
      case 1:
      	if(iSign_Len > 10) iSign_Len=10;
        //取标志位所在字段
      	if((szField_End=strchr(szField_Begin,szSeperator[0]))==NULL)
      	{
      	  if(iSign_Index!=1) 
      	  {
      	    szTxtSign_Value[0]='D';
      	    szTxtSign_Value[1]=0;
      	    return;
      	  }
      	}
      	
      	for(i=1;i<iSign_Index;i++)
      	{
      	  szField_Begin=szField_End+1;
      	  if((szField_End=strchr(szField_Begin,szSeperator[i]))==NULL)
      	  {
        	if(iSign_Index!=i+1) 
        	{
      	      szTxtSign_Value[0]='D';
      	      szTxtSign_Value[1]=0;
      	      return;
        	}
      	  }
      	}
      	if(szField_End) *szField_End=0;
      	sprintf(szTmp_Field,"%s",szField_Begin);
      	if(szField_End) *szField_End =szSeperator[i-1];
        iTmp_Len = strlen(szTmp_Field);
        //从标志位字段里取标志
      	if(iSign_Begin == (-1))
      	{
      	  if(iTmp_Len <= 10) 
      	  {
      	  	sprintf(szTxtSign_Value,"%s",szTmp_Field);
      	    break;
      	  }
      	  strncpy(szTxtSign_Value,szTmp_Field,10);
      	  szTxtSign_Value[10]=0;
      	  break;
      	}
        if(iTmp_Len < (iSign_Len+iSign_Begin-1))
        {
            sprintf(szTxtSign_Value,"%s",szTmp_Field+iSign_Begin-1);
      	    break;
        }
        strncpy(szTxtSign_Value,szTmp_Field+iSign_Begin-1,iSign_Len);
        szTxtSign_Value[iSign_Len]=0;
        break;
////
      default:
      	szTxtSign_Value[0]='D';
      	szTxtSign_Value[1]=0;
      	break;
      	
    } 
    return;
  }

  szTxtSign_Value[0]='D';
  szTxtSign_Value[1]=0;
  return;
}

