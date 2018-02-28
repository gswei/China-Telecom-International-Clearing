//20061213.加一个关键字m_szErrorType2(31),加一个接口int dealInsertRec(CFmt_Change &InRecord,char *ErrorType2,int ErrorType = 0);
//20070118加一个关键字m_OraRcd(32，原始话单) ,加一个接口int dealInsertRec(CFmt_Change &InRecord,char *ErrorType2,char *OraRcd,int ErrorType = 0)；









#include "CF_Error_Table.h"


//删除字符串空格
int CF_CError_Table::DeleteSpace( char *ss )
{
  int i;
  i = strlen(ss)-1;
  while ( i && ss[i] == ' ' ) i--;
  ss[i+1] = 0;
  return(0);
}
/*
int Cmp(const void *s1,const void *s2)//比较两个表项内容
{
  char *st1,*st2;
  int ss1,ss2;
  int j=0;

  for(int i=0;i<((STAT_RECORD *)s1)->NStat_Item_Count;i++)
  {
    int k=((STAT_RECORD *)s1)->NItem_Type[i];
    if(k==STAT_SZ_TYPE)//字符串类型
    {
      st1=((STAT_RECORD *)s1)->SzStat_Item[i];
      st2=((STAT_RECORD *)s2)->SzStat_Item[i];
      j=strcmp(st1,st2);
      if(j) return (j);
    }
    if(k==STAT_N_TYPE)//整形
    {
      ss1=atoi(((STAT_RECORD *)s1)->SzStat_Item[i]);
      ss2=atoi(((STAT_RECORD *)s2)->SzStat_Item[i]);
      j=ss1-ss2;
      if(j) return (j);
    }
  }
  return (j);
}

int _CmpFieldValue(const void *s1,const void *s2)//比较两个字段值
{
  char *st1,*st2;

  st1=((FIELD_VALUE *)s1)->szField_Value;
  st2=((FIELD_VALUE *)s2)->szField_Value;
  return strcmp(st1,st2);
}

void  CF_CError_Table::Rqsort(void  *base,  size_t  nel,  size_t  width)//排序
{
  qsort( base,nel,width,Cmp);
}
*/

CF_CError_Table::CF_CError_Table()//构造函数
{
  m_SStat_Table.NStat_Item_Count=0;
  m_NRecord_Count=0;
  m_PsPre_Stat_Record = new STAT_RECORD[ERROR_RECORD_COUNT+1];
  if( m_PsPre_Stat_Record == NULL )
  {
    throw CF_CError('E','H',-1,0,(char *)"new err.",__FILE__,__LINE__);
  }
  m_PsRes_Stat_Record = new STAT_RECORD[ERROR_RECORD_COUNT+1];
  if(m_PsRes_Stat_Record == NULL)
  {
    throw CF_CError('E','H',-1,0,(char *)"new err.",__FILE__,__LINE__);
  }
  
}

CF_CError_Table::~CF_CError_Table()
{
/*
  for(int i=0;i<m_SStat_Table.NStat_Item_Count;i++)
  {
    if(m_SStat_Table.sStat_Vip[i].pField_Value!=NULL)
    {
      delete[] m_SStat_Table.sStat_Vip[i].pField_Value;
      m_SStat_Table.sStat_Vip[i].pField_Value=NULL;
    }
  }
  */
  m_NRecord_Count=0;	

  if(m_PsPre_Stat_Record!=NULL)
  {
    delete[] m_PsPre_Stat_Record;
    m_PsPre_Stat_Record=NULL;
  }
  
  if(m_PsRes_Stat_Record!=NULL)
  {
    delete[] m_PsRes_Stat_Record;
    m_PsRes_Stat_Record=NULL;
  }

}

int CF_CError_Table::Init(char *Pipe_id,int Process_id,int config_id)
{
  CBindSQL ds( DBConn );	//DynamicSQL SqlStr;
  m_NRecord_Count=0;
  char tmpstr[400];

  sprintf(m_SzProcessId,"%d",Process_id);
  sprintf(m_SzInvalidFlag,"Y");
  sprintf(m_SzRedoFlag,"Y");
  iSortKey_Num = 0;

//get schedule
/*Query workflow_id*/
  int workflow_id;
  ds.Open("select workflow_id from pipe where pipe_id=:pipe_id");
  ds<<Pipe_id;
  ds>>workflow_id;
  if (ds.IsEnd())
  {
    sprintf(tmpstr,"select from pipe err:no match pipe_id=%s!",Pipe_id);
    throw CF_CError('D','H',-1,0,tmpstr,__FILE__,__LINE__);
  }
  ds.Close();

/*Query input_interface_id*/
  int input_id;
  ds.Open("select input_id from workflow where workflow_id"
    "=:workflow_id and process_id=:process_id ");
  ds<<workflow_id<<Process_id;
  ds>>input_id;
  if (ds.IsEnd()) 
  {
   	sprintf(tmpstr,"select from workflow err:no match "
   	  "workflow_id=%d & process_id=%d!",workflow_id,Process_id);
	throw CF_CError('D','H',-1,0,tmpstr,__FILE__,__LINE__);
  }
  ds.Close();

/*Query ctl_tabname*/
  ds.Open("select ctl_tabname from model_interface where "
    "interface_id=:input_id");
  ds<<input_id;
  ds>>m_SzSchedule;
  if (ds.IsEnd())
  {
   	sprintf(tmpstr,"select from model_interface err:no match "
   	  "interface_id=%d!",input_id);
	throw CF_CError('D','H',-1,0,tmpstr,__FILE__,__LINE__);
  }
  ds.Close();
  DeleteSpace(m_SzSchedule);
//end of get schedule



//get Table Define
  ds.Open("select Table_Name,Stat_Type,Is_Count,Count_Item_Name from "
    "STAT_TABLE_DEFINE where config_id=:config_id");
  ds<<config_id;
  ds>>m_SStat_Table.SzTable_Name;
  DeleteSpace(m_SStat_Table.SzTable_Name);
  if (ds.IsEnd()) 
  {
	throw CF_CError('D','H',-1,0,(char *)"select from STAT_TABLE_DEFINE err:"
	    "no match CONFIG_ID!",__FILE__,__LINE__);
    return (-1);
  }  
  ds>>m_SStat_Table.ChStat_Type
    >>m_SStat_Table.ChIs_Count>>m_SStat_Table.SzCount_Item_Name;
  if (ds.IsEnd()) 
  {
	throw CF_CError('D','H',-1,0,(char *)"select from STAT_TABLE_DEFINE err:"
	    "no match CONFIG_ID!",__FILE__,__LINE__);
  	return (-1);
  }
  ds.Close();	
  DeleteSpace(m_SStat_Table.SzCount_Item_Name);
//end of get table define;

//get Table Struct
/*select NStat_Item_Count*/
	ds.Open("select count(*) from STAT_TABLE_FMT where config_id=:config_id");
  ds<<config_id;
  ds>>m_SStat_Table.NStat_Item_Count;
  if (ds.IsEnd()) 
  {
	throw CF_CError('D','H',-1,0,(char *)"select from STAT_TABLE_FMT err:"
	    "no match CONFIG_ID!",__FILE__,__LINE__);
  	return (-1);
  }
  ds.Close();	
  if(m_SStat_Table.NStat_Item_Count>STAT_ITEM_COUNT)
  {
    throw CF_CError('D','H',-1,0,(char *)"table item count is more than define"
	    " in the source!",__FILE__,__LINE__);
	return (-1);
  }


char TmpIndex[10];

/* select table format*/
  ds.Open("select Table_Item,Item_Type,Field_Index,Field_Begin,Field_End"
    ",nvl(indexinfield,-1),nvl(sprinfield,' ') from STAT_TABLE_FMT where config_id=:config_id");
  ds<<config_id;
  for(int i=0;i<m_SStat_Table.NStat_Item_Count;i++)
  {
    ds>>m_SStat_Table.SzStat_Item[i]>>m_SStat_Table.NItem_Type[i]
      >>TmpIndex>>m_SStat_Table.NStat_Item_Begin[i]
      >>m_SStat_Table.NStat_Item_End[i]>>m_SStat_Table.NStat_Item_IndexInField[i]
      >>m_SStat_Table.chStat_Item_SprInField[i];
    DeleteSpace(m_SStat_Table.SzStat_Item[i]);
    m_SStat_Table.NField_Index[i]=atoi(TmpIndex);
//matching
/*
    m_SStat_Table.sStat_Vip[i].iField_Index=m_SStat_Table.NField_Index[i];
    m_SStat_Table.sStat_Vip[i].iFieldValue_Count=0;
    m_SStat_Table.sStat_Vip[i].pField_Value=NULL;
*/
  }
  ds.Close();
  
//end of get Table Struct

return 0;
}

int CF_CError_Table::Init(char *Pipe_id,int Process_id,int config_id,char *SortKey)
{
  CBindSQL ds( DBConn );	//DynamicSQL SqlStr;
  m_NRecord_Count=0;
  char tmpstr[400];

  sprintf(m_SzProcessId,"%d",Process_id);
  sprintf(m_SzInvalidFlag,"Y");
  sprintf(m_SzRedoFlag,"Y");


  if((SortKey == NULL)||(strlen(SortKey) == 0))
  {
    iSortKey_Num = 0;
  }
  else
  {
    SplitBuf(SortKey,';');
  }

//get schedule
/*Query workflow_id*/
  int workflow_id;
  ds.Open("select workflow_id from pipe where pipe_id=:pipe_id");
  ds<<Pipe_id;
  ds>>workflow_id;
  if (ds.IsEnd())
  {
    sprintf(tmpstr,"select from pipe err:no match pipe_id=%s!",Pipe_id);
    throw CF_CError('D','H',-1,0,tmpstr,__FILE__,__LINE__);
  }
  ds.Close();

/*Query input_interface_id*/
  int input_id;
  ds.Open("select input_id from workflow where workflow_id"
    "=:workflow_id and process_id=:process_id ");
  ds<<workflow_id<<Process_id;
  ds>>input_id;
  if (ds.IsEnd()) 
  {
   	sprintf(tmpstr,"select from workflow err:no match "
   	  "workflow_id=%d & process_id=%d!",workflow_id,Process_id);
	throw CF_CError('D','H',-1,0,tmpstr,__FILE__,__LINE__);
  }
  ds.Close();

/*Query ctl_tabname*/
  ds.Open("select ctl_tabname from model_interface where "
    "interface_id=:input_id");
  ds<<input_id;
  ds>>m_SzSchedule;
  if (ds.IsEnd())
  {
   	sprintf(tmpstr,"select from model_interface err:no match "
   	  "interface_id=%d!",input_id);
	throw CF_CError('D','H',-1,0,tmpstr,__FILE__,__LINE__);
  }
  ds.Close();
  DeleteSpace(m_SzSchedule);
//end of get schedule



//get Table Define
  ds.Open("select Table_Name,Stat_Type,Is_Count,Count_Item_Name from "
    "STAT_TABLE_DEFINE where config_id=:config_id");
  ds<<config_id;
  ds>>m_SStat_Table.SzTable_Name;
  DeleteSpace(m_SStat_Table.SzTable_Name);
  if (ds.IsEnd()) 
  {
	throw CF_CError('D','H',-1,0,(char *)"select from STAT_TABLE_DEFINE err:"
	    "no match CONFIG_ID!",__FILE__,__LINE__);
    return (-1);
  }  
  ds>>m_SStat_Table.ChStat_Type
    >>m_SStat_Table.ChIs_Count>>m_SStat_Table.SzCount_Item_Name;
  if (ds.IsEnd()) 
  {
	throw CF_CError('D','H',-1,0,(char *)"select from STAT_TABLE_DEFINE err:"
	    "no match CONFIG_ID!",__FILE__,__LINE__);
  	return (-1);
  }
  ds.Close();	
  DeleteSpace(m_SStat_Table.SzCount_Item_Name);
//end of get table define;

//get Table Struct
/*select NStat_Item_Count*/
	ds.Open("select count(*) from STAT_TABLE_FMT where config_id=:config_id");
  ds<<config_id;
  ds>>m_SStat_Table.NStat_Item_Count;
  if (ds.IsEnd()) 
  {
	throw CF_CError('D','H',-1,0,(char *)"select from STAT_TABLE_FMT err:"
	    "no match CONFIG_ID!",__FILE__,__LINE__);
  	return (-1);
  }
  ds.Close();	
  if(m_SStat_Table.NStat_Item_Count>STAT_ITEM_COUNT)
  {
    throw CF_CError('D','H',-1,0,(char *)"table item count is more than define"
	    " in the source!",__FILE__,__LINE__);
	return (-1);
  }


char TmpIndex[10];

/* select table format*/
  ds.Open("select Table_Item,Item_Type,Field_Index,Field_Begin,Field_End"
    ",nvl(indexinfield,-1),nvl(sprinfield,' ') from STAT_TABLE_FMT where config_id=:config_id");
  ds<<config_id;
  for(int i=0;i<m_SStat_Table.NStat_Item_Count;i++)
  {
    ds>>m_SStat_Table.SzStat_Item[i]>>m_SStat_Table.NItem_Type[i]
      >>TmpIndex>>m_SStat_Table.NStat_Item_Begin[i]
      >>m_SStat_Table.NStat_Item_End[i]>>m_SStat_Table.NStat_Item_IndexInField[i]
      >>m_SStat_Table.chStat_Item_SprInField[i];
    DeleteSpace(m_SStat_Table.SzStat_Item[i]);
    m_SStat_Table.NField_Index[i]=atoi(TmpIndex);
//matching
/*
    m_SStat_Table.sStat_Vip[i].iField_Index=m_SStat_Table.NField_Index[i];
    m_SStat_Table.sStat_Vip[i].iFieldValue_Count=0;
    m_SStat_Table.sStat_Vip[i].pField_Value=NULL;
*/
  }
  ds.Close();
  
//end of get Table Struct

return 0;
}




int CF_CError_Table::setFileName(char *FileName,char *Source_Id)
{

  sprintf(m_SzFileName,"%s",FileName);
  memset(m_SzStat_Time,0,STAT_ITEM_NAME_LEN);
  getCurTime(m_SzStat_Time);
  if(Source_Id != NULL)
    sprintf(m_SzSource_ID,"%s",Source_Id);
  else
  {
    CBindSQL ds(DBConn);
    char sqltmp[400];
    char tmpstr[400];
    sprintf(sqltmp,"select source_id from %s where filename='%s'",
    m_SzSchedule,FileName);
    ds.Open(sqltmp);
    ds>>m_SzSource_ID;
    if (ds.IsEnd()) 
   	{
  	  sprintf(tmpstr,"select from %s err:no match filename:%s!",m_SzSchedule,FileName);
	  throw CF_CError('D','H',-1,0,tmpstr,__FILE__,__LINE__);
    }
    ds.Close();
    DeleteSpace(m_SzSource_ID);
  }
  return 0;
}

int CF_CError_Table::setFlag(char *InvalidFlag,char *RedoFlag)
{
  sprintf(m_SzInvalidFlag,InvalidFlag);
  sprintf(m_SzRedoFlag,RedoFlag);
  return 0;
}

int CF_CError_Table::Stat_Record(CFmt_Change &InRecord)
{
  char TmpLogMsg[400];
  for(int i=0;i<m_SStat_Table.NStat_Item_Count;i++)
  {
    //取字段值
    m_PsPre_Stat_Record[m_NRecord_Count].NItem_Type[i] 
	  = m_SStat_Table.NItem_Type[i];
    if(m_SStat_Table.NItem_Type[i]>=KEYWORK_MIN)
	{
	  Set_KeyStatItem(m_PsPre_Stat_Record[m_NRecord_Count],i);
	  continue;
	}
    if(m_SStat_Table.NItem_Type[i]<KEYWORK_MIN)
    {
	  if(m_SStat_Table.NStat_Item_Begin[i]==(-1))
	  {
		if(m_SStat_Table.NStat_Item_IndexInField[i]==(-1))
		  sprintf(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i],
		    "%s",InRecord.Get_Field(m_SStat_Table.NField_Index[i]));
		else
		{
		  sprintf(TmpLogMsg,"%s",InRecord.Get_Field(m_SStat_Table.NField_Index[i]));
		  char *TmpPoint1;
		  char *TmpPoint2;
		  char *TmpPoint3;
          int kk;
		  TmpPoint3=TmpLogMsg;
		  for(kk=0;kk<m_SStat_Table.NStat_Item_IndexInField[i];kk++)
		  {
		    TmpPoint1=TmpPoint3;
		    TmpPoint2=strchr(TmpPoint1,m_SStat_Table.chStat_Item_SprInField[i]);
		    if(TmpPoint2==NULL) break;
		    TmpPoint3=TmpPoint2+1;
		  }
          if(TmpPoint2==NULL)
          {
            if(kk==(m_SStat_Table.NStat_Item_IndexInField[i]-1))
		      sprintf(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i],
		        "%s",TmpPoint1);
            else
            {
    		  sprintf(TmpLogMsg,"Field %d Index In Field is less than Define.",i);
              throw CF_CError('D','H',ERR_FIND_ITEM_NONE,0,TmpLogMsg,__FILE__,__LINE__);
            }
          }
          else
          {
            *TmpPoint2=0;
            sprintf(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i],
		      "%s",TmpPoint1);
          }
		}
	  }
	else 
	{
	    //判断字段是否有规定取的那么长
		int NField_Len = strlen(InRecord.Get_Field(m_SStat_Table.NField_Index[i]));
		if(//(NField_Len<m_SStat_Table.NStat_Item_End[i])||
		  (m_SStat_Table.NStat_Item_End[i]<m_SStat_Table.NStat_Item_Begin[i]))
		{
		  sprintf(TmpLogMsg,"Field %d Field_Len is not long enough.",i);
          throw CF_CError('D','H',ERR_FIND_ITEM_NONE,0,TmpLogMsg,__FILE__,__LINE__);
		}
		NField_Len=m_SStat_Table.NStat_Item_End[i]-m_SStat_Table.NStat_Item_Begin[i]+1;
		sprintf(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i],"%s",
		  InRecord.Get_Field(m_SStat_Table.NField_Index[i])
		  +m_SStat_Table.NStat_Item_Begin[i]-1);
		m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i][NField_Len]=0;
	}
  }
  }
  m_PsPre_Stat_Record[m_NRecord_Count].NStat_Item_Count=m_SStat_Table.NStat_Item_Count;

return 0;	
}

int CF_CError_Table::Get_OutRcd(CFmt_Change &OutRcd,int Index)
{
  for(int i=0;i<m_SStat_Table.NStat_Item_Count;i++)
  {
    if(m_SStat_Table.NField_Index[i] > 0)
  	OutRcd.Set_Field(m_SStat_Table.NField_Index[i],m_PsRes_Stat_Record[Index].SzStat_Item[i]);
  }
  return 0;

}


int CF_CError_Table::Set_KeyWord(STAT_RECORD &Cur,int i,char *Key_Container)
{
  if(m_SStat_Table.NStat_Item_Begin[i]==(-1))
  {
    sprintf(Cur.SzStat_Item[i],"%s",Key_Container);
  }
  else 
  {
    //判断字段是否有规定取的那么长
    int NField_Len = strlen(Key_Container);
	if(//(NField_Len<m_SStat_Table.NStat_Item_End[i])||
      (m_SStat_Table.NStat_Item_End[i]<m_SStat_Table.NStat_Item_Begin[i]))
	{
       throw CF_CError('D','H',ERR_FIND_ITEM_NONE,0,"Field_Len is not long enough.",__FILE__,__LINE__);
    }
    NField_Len=m_SStat_Table.NStat_Item_End[i]-m_SStat_Table.NStat_Item_Begin[i]+1;
    sprintf(Cur.SzStat_Item[i],"%s",
      Key_Container+m_SStat_Table.NStat_Item_Begin[i]-1);
    Cur.SzStat_Item[i][NField_Len]=0;
  }
  return 0;			
}


int CF_CError_Table::Set_KeyStatItem(STAT_RECORD &Cur,int i)
{
  switch(m_SStat_Table.NItem_Type[i])
  {
  case 11:
    Set_KeyWord(Cur,i,m_SzSource_ID);
    break;
  case 12:
    Set_KeyWord(Cur,i,m_SzStat_Time);
    break;
  case 13:
    Set_KeyWord(Cur,i,m_SzFileName);
    break;
  case 16:
    Set_KeyWord(Cur,i,m_SzErrorType);
    break;
  case 17:
    Set_KeyWord(Cur,i,m_SzProcessId);
    break;
  case 18:
    Set_KeyWord(Cur,i,m_SzInvalidFlag);
    break;
  case 19:
    Set_KeyWord(Cur,i,m_SzRedoFlag);
    break;
  case 31:
    Set_KeyWord(Cur,i,m_szErrorType2);
    break;
  case 32:
  	break;
  default:
    char szErrMsg[100];
  	sprintf(szErrMsg, "Can not find Such Key Stat Item of [%d]", m_SStat_Table.NItem_Type[i]);
  	throw CF_CError('D','H',ERR_FIND_ITEM_NONE,0, szErrMsg,__FILE__,__LINE__);
//  	throw CF_CError('D','H',ERR_FIND_ITEM_NONE,0,"Can not find Such Key Stat Item.",__FILE__,__LINE__);
  }
  return 0;
}

int CF_CError_Table::Make_Select_Count_Sql(STAT_RECORD& UPdate_Stat_Record,char *sqltmp)
{
  sprintf(sqltmp,"select count(*) ");

  sprintf(sqltmp,"%s from %s where ",sqltmp,m_SStat_Table.SzTable_Name);

  for(int i=0;i<m_SStat_Table.NStat_Item_Count;i++)
  {
    if((m_SStat_Table.NItem_Type[i]==STAT_SZ_TYPE)||(m_SStat_Table.NItem_Type[i]>=KEYWORK_MIN))
    {
      strcat(sqltmp,m_SStat_Table.SzStat_Item[i]);
      strcat(sqltmp,"='");
      strcat(sqltmp,UPdate_Stat_Record.SzStat_Item[i]);
      strcat(sqltmp,"' and ");
    }
    if(m_SStat_Table.NItem_Type[i]==STAT_N_TYPE)
    {
      strcat(sqltmp,m_SStat_Table.SzStat_Item[i]);
      strcat(sqltmp,"=");
      strcat(sqltmp,UPdate_Stat_Record.SzStat_Item[i]);
      strcat(sqltmp," and ");
    }
  }
  sqltmp[strlen(sqltmp)-5] = 0;	 
  return 0;
}

int CF_CError_Table::Make_Select_Sql(STAT_RECORD& UPdate_Stat_Record,char *sqltmp)
{
  sprintf(sqltmp,"select ");
  for(int i = 0;i<m_SStat_Table.NStat_Item_Count;i++)
  {
//    if(m_SStat_Table.NItem_Type[i]==STAT_N_COUNT_TYPE)
//    {
      strcat(sqltmp,m_SStat_Table.SzStat_Item[i]);
      strcat(sqltmp,",");
//     }
  }
  
  if(m_SStat_Table.ChIs_Count=='Y')
  {
    sprintf(sqltmp,"%s%s",sqltmp,m_SStat_Table.SzCount_Item_Name);    	
    sqltmp[strlen(sqltmp)]     = 0;
  }
  else
  {
    sqltmp[strlen(sqltmp)-1] = 0;
  }	 
  sprintf(sqltmp,"%s from %s where ",sqltmp,m_SStat_Table.SzTable_Name);

  for(int i=0;i<m_SStat_Table.NStat_Item_Count;i++)
  {
    if((m_SStat_Table.NItem_Type[i]==STAT_SZ_TYPE)||(m_SStat_Table.NItem_Type[i]>=KEYWORK_MIN))
    {
      strcat(sqltmp,m_SStat_Table.SzStat_Item[i]);
      strcat(sqltmp,"='");
      strcat(sqltmp,UPdate_Stat_Record.SzStat_Item[i]);
      strcat(sqltmp,"' and ");
    }
    if(m_SStat_Table.NItem_Type[i]==STAT_N_TYPE)
    {
      strcat(sqltmp,m_SStat_Table.SzStat_Item[i]);
      strcat(sqltmp,"=");
      strcat(sqltmp,UPdate_Stat_Record.SzStat_Item[i]);
      strcat(sqltmp," and ");
    }
  }
  sqltmp[strlen(sqltmp)-5] = 0;	 

  if(iSortKey_Num == 0) return 0;
  
  sprintf(sqltmp,"%s order by ",sqltmp);


  for(int i=0;i<iSortKey_Num;i++)
  {
    strcat(sqltmp,szSortKeyStr[i]);
    strcat(sqltmp,",");
  }
  
  sqltmp[strlen(sqltmp)-1] = 0;	 

  return 0;
}

int CF_CError_Table::Make_Insert_Sql(STAT_RECORD& UPdate_Stat_Record,char *sqltmp)
{
	sprintf(sqltmp,"insert into %s( ",m_SStat_Table.SzTable_Name);
	for(int i=0;i<m_SStat_Table.NStat_Item_Count;i++)
	  {
	    strcat(sqltmp,m_SStat_Table.SzStat_Item[i]);
	    strcat(sqltmp,",");
	  }
  if(m_SStat_Table.ChIs_Count=='Y')
	  {
    strcat(sqltmp,m_SStat_Table.SzCount_Item_Name);
    sqltmp[strlen(sqltmp)]     = 0;
	  }else
	    {
      sqltmp[strlen(sqltmp)-1] = 0;
		  }	 
	sprintf(sqltmp,"%s) values ( ",sqltmp);
/*	
	for(int i=0;i<m_SStat_Table.NStat_Item_Count;i++)
	  {
	  if((m_SStat_Table.NItem_Type[i]==STAT_SZ_TYPE)||(m_SStat_Table.NItem_Type[i]>=KEYWORK_MIN))
	    {
	    strcat(sqltmp,"'");
	    strcat(sqltmp,UPdate_Stat_Record.SzStat_Item[i]);
	    strcat(sqltmp,"',");
	    }
	  if((m_SStat_Table.NItem_Type[i]==STAT_N_TYPE)||(m_SStat_Table.NItem_Type[i]==STAT_N_COUNT_TYPE))
	    {
	    if(strlen(UPdate_Stat_Record.SzStat_Item[i]) == 0)
	    strcat(sqltmp,"NULL");
	    else strcat(sqltmp,UPdate_Stat_Record.SzStat_Item[i]);
	    strcat(sqltmp,",");
	    }
	  }
  if(m_SStat_Table.ChIs_Count=='Y')
  {
    sprintf(sqltmp,"%s%d",sqltmp,UPdate_Stat_Record.NBill_Count);
    strcat(sqltmp,")");
    sqltmp[strlen(sqltmp)]     = 0;
  }else
    {
      sqltmp[strlen(sqltmp)-1] = 0;
      strcat(sqltmp,")");
      sqltmp[strlen(sqltmp)]   = 0;		 
     }	 
	
*/
char c_item[5];
  	for(int i=0;i<m_SStat_Table.NStat_Item_Count;i++)
	  {
  	 	  sprintf(c_item,":%d,",i);
  		strcat(sqltmp,c_item);
	  }
  if(m_SStat_Table.ChIs_Count=='Y')
  {
    sprintf(sqltmp,"%s:%d",sqltmp,m_SStat_Table.NStat_Item_Count);
    strcat(sqltmp,")");
    sqltmp[strlen(sqltmp)]     = 0;
  }else
    {
      sqltmp[strlen(sqltmp)-1] = 0;
      strcat(sqltmp,")");
      sqltmp[strlen(sqltmp)]   = 0;		 
     }	 

  return 0;
}

int CF_CError_Table::Insert_Error()
{
  if(m_NRecord_Count==0)
    {
    return 0;
    }
  
  CBindSQL ds(DBConn);
  char sqltmp[4000];
/*
  for(int i=0;i<m_NRecord_Count;i++ ) 
  {
    memset(sqltmp,0,600);
    Make_Insert_Sql(m_PsPre_Stat_Record[i],sqltmp);
    ds.Open(sqltmp,NONSELECT_DML);

    ds.Execute();
    if(ds.IsError())
    {
      ds.Close();
      return (-1);
    }
    ds.Close();

  }
*/

    memset(sqltmp,0,600);
    Make_Insert_Sql(m_PsPre_Stat_Record[0],sqltmp);
    ds.Open(sqltmp,NONSELECT_DML);
    for(int i=0;i<m_NRecord_Count;i++ ) 
		{	
	 		for(int item=0;item<m_SStat_Table.NStat_Item_Count;item++)
	  		{
	  			DeleteSpace(m_PsPre_Stat_Record[i].SzStat_Item[item]);
	  			if(m_SStat_Table.NItem_Type[item]==STAT_N_TYPE||
	  	  			m_SStat_Table.NItem_Type[item]==STAT_N_COUNT_TYPE)
	  				ds<<atoi(m_PsPre_Stat_Record[i].SzStat_Item[item]);
	  			else if(m_SStat_Table.NItem_Type[item]==KEYWORK_ORARCD)
	  				ds<<m_szOraRcd[i];
	  			else
	  				ds<<m_PsPre_Stat_Record[i].SzStat_Item[item];
	  		}
	  		if(m_SStat_Table.ChIs_Count=='Y')  
	  			ds<<m_PsPre_Stat_Record[i].NBill_Count;
		}
    
    ds.Execute();
    if(ds.IsError())
    {
      ds.Close();
      return (-1);
    }
    ds.Close();


  return (0);
}

int CF_CError_Table::Select_Error()
{

  CBindSQL ds(DBConn);
  char sqltmp[4000];
  int Error_Rcd_Num;

  Make_Select_Count_Sql(m_PsPre_Stat_Record[m_NRecord_Count],sqltmp);
  ds.Open(sqltmp);
  ds>>Error_Rcd_Num;
  ds.Close();

  if(Error_Rcd_Num == 0) return 0;
  if((Error_Rcd_Num/ERROR_RECORD_COUNT) > 0)
  {
    if(m_PsRes_Stat_Record!=NULL)
    {
      delete[] m_PsRes_Stat_Record;
      m_PsRes_Stat_Record=NULL;
    }

    m_PsRes_Stat_Record = new STAT_RECORD[Error_Rcd_Num+1];
    if(m_PsRes_Stat_Record==NULL)
    {
      throw CF_CError('E','H',-1,0,(char *)"new err.",__FILE__,__LINE__);
    }
  }

  Make_Select_Sql(m_PsPre_Stat_Record[m_NRecord_Count],sqltmp);
  ds.Open(sqltmp);
  for(int j=0;j<Error_Rcd_Num;j++)
  {
    for(int i=0;i<m_SStat_Table.NStat_Item_Count;i++)
	{
//	  if(m_SStat_Table.NItem_Type[i]==STAT_N_COUNT_TYPE)
//	  {
	    ds>>m_PsRes_Stat_Record[j].SzStat_Item[i];
	    if(ds.IsEnd())
		{
		  ds.Close();
		  return -1;
	 	}
//	  }
	}
  }
  ds.Close();

  return Error_Rcd_Num;
}

int CF_CError_Table::Update_Flag(char Invalid_Flag,char Redo_Flag)
{

  CBindSQL ds(DBConn);
  char sqltmp[4000];
  int Error_Rcd_Num;
  sprintf(sqltmp,"update %s set ",m_SStat_Table.SzTable_Name);
  for(int i = 0;i<m_SStat_Table.NStat_Item_Count;i++)
  {
	  if(m_SStat_Table.NItem_Type[i]==KEYWORK_INVALIDFLAG)
	    {
//	    strcat(sqltmp,m_SStat_Table.SzStat_Item[i]);
//	    strcat(sqltmp,"='N',");
        sprintf(sqltmp,"%s%s='%c',",sqltmp,m_SStat_Table.SzStat_Item[i],Invalid_Flag);
	    }
	  if(m_SStat_Table.NItem_Type[i]==KEYWORK_REDOFLAG)
	    {
//	    strcat(sqltmp,m_SStat_Table.SzStat_Item[i]);
//	    strcat(sqltmp,"='N',");
        sprintf(sqltmp,"%s%s='%c',",sqltmp,m_SStat_Table.SzStat_Item[i],Redo_Flag);
	    }
  }
  
  sqltmp[strlen(sqltmp)-1] = 0;
    
  sprintf(sqltmp,"%s where ",sqltmp);

  for(int i=0;i<m_SStat_Table.NStat_Item_Count;i++)
  {
    if((m_SStat_Table.NItem_Type[i]==STAT_SZ_TYPE)||(m_SStat_Table.NItem_Type[i]>=KEYWORK_MIN))
    {
      strcat(sqltmp,m_SStat_Table.SzStat_Item[i]);
      strcat(sqltmp,"='");
      strcat(sqltmp,m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i]);
      strcat(sqltmp,"' and ");
    }
    if(m_SStat_Table.NItem_Type[i]==STAT_N_TYPE)
    {
      strcat(sqltmp,m_SStat_Table.SzStat_Item[i]);
      strcat(sqltmp,"=");
      strcat(sqltmp,m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i]);
      strcat(sqltmp," and ");
    }
  }
  sqltmp[strlen(sqltmp)-5] = 0;
//  Make_Select_Sql(m_PsPre_Stat_Record[m_NRecord_Count],sqltmp);
  ds.Open(sqltmp);
  ds.Execute();
  if(ds.IsError())
  {
    ds.Close();
   	return (-1);
  }	
  ds.Close();
  return 0;
}
 					
    			
int CF_CError_Table::dealInsertRec(CFmt_Change &InRecord,int ErrorType)
{
  sprintf(m_SzErrorType,"%d",ErrorType);
  if(Stat_Record(InRecord)) return 0;
  m_NRecord_Count+=1;
  if(m_NRecord_Count>=ERROR_RECORD_COUNT)
	  {
	  if(Insert_Error())
		  {
			return (-1);
		  }
		m_NRecord_Count=0;	
	  }
  return 0;
}

int CF_CError_Table::dealInsertRec(CFmt_Change &InRecord,char *ErrorType2,int ErrorType)
{
  sprintf(m_SzErrorType,"%d",ErrorType);
  if(strlen(ErrorType2)>100) ErrorType2[99]=0;
  sprintf(m_szErrorType2,"%s",ErrorType2);

  if(Stat_Record(InRecord)) return 0;
  m_NRecord_Count+=1;
  if(m_NRecord_Count>=ERROR_RECORD_COUNT)
	  {
	  if(Insert_Error())
		  {
			return (-1);
		  }
		m_NRecord_Count=0;	
	  }
  return 0;
}

int CF_CError_Table::dealInsertRec(CFmt_Change &InRecord,char *ErrorType2,char *OraRcd,int ErrorType)
{
  sprintf(m_SzErrorType,"%d",ErrorType);
  if(strlen(ErrorType2)>100) ErrorType2[99]=0;
  sprintf(m_szErrorType2,"%s",ErrorType2);
  if(strlen(OraRcd)>1000) OraRcd[999]=0;
  sprintf(m_szOraRcd[m_NRecord_Count],"%s",OraRcd);

  if(Stat_Record(InRecord)) return 0;
  m_NRecord_Count+=1;
  if(m_NRecord_Count>=ERROR_RECORD_COUNT)
	  {
	  if(Insert_Error())
		  {
			return (-1);
		  }
		m_NRecord_Count=0;	
	  }
  return 0;
}

int CF_CError_Table::dealSelectRec(CFmt_Change &InRecord,int ErrorType)
{

  m_NRecord_Count=0;	
  sprintf(m_SzErrorType,"%d",ErrorType);
  if(Stat_Record(InRecord)) return 0;
  m_NRecord_Count = Select_Error();
  return m_NRecord_Count;
}
int CF_CError_Table::dealUpdateFlag(CFmt_Change &InRecord,char Invalid_Flag,char Redo_Flag)
{

  m_NRecord_Count=0;
//  sprintf(m_SzErrorType,"%d",ErrorType);
  if(Stat_Record(InRecord)) return 0;
  if(Update_Flag(Invalid_Flag,Redo_Flag)) 
  	throw CF_CError('D','H',-1,0,(char *)"Update Err.",__FILE__,__LINE__);
  return 0;
}

int CF_CError_Table::Select_All()
{

  CBindSQL ds(DBConn);
  char sqltmp[4000];
  int Error_Rcd_Num;

  sprintf(sqltmp,"select count(*) from %s",m_SStat_Table.SzTable_Name);

  ds.Open(sqltmp);
  ds>>Error_Rcd_Num;
  ds.Close();

  if(Error_Rcd_Num == 0) return 0;
  if((Error_Rcd_Num/ERROR_RECORD_COUNT) > 0)
  {
    if(m_PsRes_Stat_Record!=NULL)
    {
      delete[] m_PsRes_Stat_Record;
      m_PsRes_Stat_Record=NULL;
    }

    m_PsRes_Stat_Record = new STAT_RECORD[Error_Rcd_Num+1];
    if(m_PsRes_Stat_Record==NULL)
    {
      throw CF_CError('E','H',-1,0,(char *)"new err.",__FILE__,__LINE__);
    }
  }
  
  sprintf(sqltmp,"select ");
  for(int i = 0;i<m_SStat_Table.NStat_Item_Count;i++)
  {
//    if(m_SStat_Table.NItem_Type[i]==STAT_N_COUNT_TYPE)
//    {
      strcat(sqltmp,m_SStat_Table.SzStat_Item[i]);
      strcat(sqltmp,",");
//     }
  }
  
  if(m_SStat_Table.ChIs_Count=='Y')
  {
    sprintf(sqltmp,"%s%s",sqltmp,m_SStat_Table.SzCount_Item_Name);    	
    sqltmp[strlen(sqltmp)]     = 0;
  }
  else
  {
    sqltmp[strlen(sqltmp)-1] = 0;
  }	 
  sprintf(sqltmp,"%s from %s ",sqltmp,m_SStat_Table.SzTable_Name);
  ds.Open(sqltmp);
  for(int j=0;j<Error_Rcd_Num;j++)
  {
    for(int i=0;i<m_SStat_Table.NStat_Item_Count;i++)
	{
//	  if(m_SStat_Table.NItem_Type[i]==STAT_N_COUNT_TYPE)
//	  {
//       try
//       {
	      ds>>m_PsRes_Stat_Record[j].SzStat_Item[i];
//        }
//        catch(...)
//        {
//          ds>>tmpNum;
//          sprintf(m_PsRes_Stat_Record[j].SzStat_Item[i],"%d",tmpNum);
//        }
	    if(ds.IsEnd())
		{
		  ds.Close();
		  return -1;
	 	}
//	  }
	}
  }

  ds.Close();

  return Error_Rcd_Num;
}

int CF_CError_Table::dealBackupRec()
{

  m_NRecord_Count=0;	
  m_NRecord_Count = Select_All();
  return m_NRecord_Count;
}


void CF_CError_Table::commit()
{
  Insert_Error();
  m_NRecord_Count=0;
}

void CF_CError_Table::rollback()
{
  m_NRecord_Count=0;
  DBConn.Rollback();
}

int CF_CError_Table::SplitBuf( char *ss,char c)

{

 int i;

 char *ss0,*ss1;

     i = 0; ss0 = ss;

     while( ( ss1 = strchr( ss0,c ) ) != NULL ) {

        *ss1 = 0;
        sprintf( szSortKeyStr[i],"%s",ss0 );
        ss0 = ss1+1;
        *ss1 = c;
		i++;
		if(i>=10) 
		{
		  iSortKey_Num = i;
		  return (0);
		}
     }

     sprintf( szSortKeyStr[i],"%s",ss0 );
     i++;
     iSortKey_Num = i;

 return(0);
}



