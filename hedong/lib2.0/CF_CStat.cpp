//公用统计类

#include "CF_CStat.h"

int G_CF_CStat_UpdateCount=0;
char G_CF_CStat_DebugFlag[10];  
//int Update_flag=0;

//删除字符串空格
int CF_CStat::DeleteSpace( char *ss )
{
  if(strlen(ss)!=0)
  	{
  	int i;
  	i = strlen(ss)-1;
  	while ( i>=0 && ss[i] == ' ' ) i--;
  	ss[i+1] = 0;
  	}
  return(0);
}

int CF_CStat::Count_Item(char *string,char seperator)
{
  	delSpace(string,0);
  	char *TmpPoint1=string;
  	char *TmpPoint2=NULL;
  	int iTmpCount=0;
  	while(1)
  	{
    	iTmpCount++;
    	TmpPoint2=strchr(TmpPoint1,seperator);
    	if(TmpPoint2==NULL) break;
    	TmpPoint1=TmpPoint2+1;
  	}  	
  	return(iTmpCount);
}

void CF_CStat::Get_Item(char *string,char seperator,int *item_group,int item_count)
{
    delSpace(string,0);
  	char *TmpPoint1=string;
  	char *TmpPoint2=NULL;
 	int ii=0;
  	for(ii=1;ii<item_count;ii++)
  	{
    	TmpPoint2=strchr(TmpPoint1,seperator);
    	*TmpPoint2=0;
    	item_group[ii-1]=atol(TmpPoint1);
    	TmpPoint1=TmpPoint2+1;
  	}
  	item_group[ii-1]=atol(TmpPoint1);	
}


int Cmp(const void *s1,const void *s2)//比较两个表项内容
{
  char *st1,*st2;
  int ss1,ss2;
  int j=0;
  //cout<<"*********in cmp  ***********"<<((STAT_RECORD *)s1)->NStat_Item_Count <<endl;
  for(int i=0;i<((STAT_RECORD *)s1)->NStat_Item_Count;i++)
  {
    int k=((STAT_RECORD *)s1)->NItem_Type[i];
    //cout<<"NItem_Type--------"<<k<<endl;
    if(k==STAT_SZ_TYPE||k>=KEYWORK_MIN)//字符串类型
    {
      st1=((STAT_RECORD *)s1)->SzStat_Item[i];
      st2=((STAT_RECORD *)s2)->SzStat_Item[i];
      j=strcmp(st1,st2);
	//cout<<st1<<"|"<<st2<<endl;
	//cout<<"strcmp(st1,st2)="<<j<<endl;
      if(j) return (j);
    }
    if(k==STAT_N_TYPE)//整形
    {
      ss1=atol(((STAT_RECORD *)s1)->SzStat_Item[i]);
      ss2=atol(((STAT_RECORD *)s2)->SzStat_Item[i]);
      j=ss1-ss2;
	 //cout<<ss1<<"|"<<ss2<<endl;
	 //cout<<"ss1-ss2="<<j<<endl;
      if(j)  return (j);
    }
    
  }
  return (j);
}
int _CmpFieValue(const void *s1,const void *s2)//比较两个字段值
{
  char *st1,*st2;

  st1=((FIELD_VALUE *)s1)->szField_Value;
  st2=((FIELD_VALUE *)s2)->szField_Value;
  return strcmp(st1,st2);
}


void  CF_CStat::Rqsort(void  *base,  size_t  nel,  size_t  width)//排序
{
  qsort( base,nel,width,Cmp);
}


CF_CStat::CF_CStat()
{
  m_SStat_Table.NStat_Item_Count=0;    
  m_NRecord_Count=0;
  icount_toUpdate=0;
 // istat_item_numb=0;
  //icount_toInsert_table=0;
 // icount_toUpdate_table=0;  
//  Record_toInsert=NULL;
 // Record_toUpdate=NULL;
  m_SStat_Table.cond_field=NULL;  
  m_SStat_Table.cond_field_count=0;
  m_SStat_Table.cond_field_value=NULL;
  first_time_read_record=1;
  cond_variable_in_record_count=0;
  before_endtime_flag=0;
  m_key_record.clear();
  m_key_seq.clear();
/*
  Stat_Record_toUpdate = new STAT_RECORD[MEMORY_APPLY_UNIT];
  if(Stat_Record_toUpdate==NULL)
  {
    throw CF_CError('E','H',ERR_REQU_MEM,0,(char *)"new err.",__FILE__,__LINE__);
  }
  Current_Memorry_Unit=MEMORY_APPLY_UNIT;
  
  m_PsPre_Stat_Record = new STAT_RECORD[maxRcdToCaculate];
  if(m_PsPre_Stat_Record==NULL)
  {
    throw CF_CError('E','H',ERR_REQU_MEM,0,(char *)"new err.",__FILE__,__LINE__);
  }
  m_PsRes_Stat_Record = new STAT_RECORD[maxRcdToCaculate];
  if(m_PsRes_Stat_Record==NULL)
  {
    throw CF_CError('E','H',ERR_REQU_MEM,0,(char *)"new err.",__FILE__,__LINE__);
  }
  */
  Stat_Record_toUpdate=NULL;
  m_PsPre_Stat_Record=NULL;
  m_PsRes_Stat_Record=NULL;
  
  map_table_recordToInsert.clear();
  map_table_recordToUpdate.clear();
  map_table_recordToInsertCount.clear();
  map_table_recordToUpdateCount.clear();
  crr_table_name[0]=0;
  m_SzStat_process_id[0]=0;
}

CF_CStat::~CF_CStat()
{

  if(Stat_Record_toUpdate!=NULL)
  	{
  	 delete[] Stat_Record_toUpdate;
     Stat_Record_toUpdate=NULL;
  	}
  
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
  if(m_SStat_Table.cond_field!=NULL)
  {
    delete[] m_SStat_Table.cond_field;
    m_SStat_Table.cond_field=NULL;
  }
  if(m_SStat_Table.cond_field_value!=NULL)
  {
    delete[] m_SStat_Table.cond_field_value;
    m_SStat_Table.cond_field_value=NULL;
  }
}

//long CF_CStat::statictUpdateCount=0;

//20070208,若para_config_id为负数，则作冲销统计
int CF_CStat::Init(char *Pipe_id,int Process_id,int para_config_id,char *dataType_endTime,int maxRcdNumToUpdat)
{
	sprintf(m_SzStat_process_id,"%d",Process_id);
  //Current_Memorry_Unit=maxRcdNumToUpdat;
   m_maxRcdToUpdate=maxRcdNumToUpdat;
	
  char errmsg[200];
  strcpy(endTime,dataType_endTime);
  
  int config_id=abs(para_config_id);
  CBindSQL ds( DBConn );	//DynamicSQL SqlStr;
  m_NRecord_Count=0;
  char tmpstr[400];

//get schedule
/*Query workflow_id*/
  int workflow_id;
  ds.Open("select workflow_id from pipe where pipe_id=:pipe_id");
  ds<<Pipe_id;
  ds>>workflow_id;
  if (ds.IsEnd())
  {
    sprintf(tmpstr,"select from pipe err:no match pipe_id=%s!",Pipe_id);
    throw CF_CError('E','H',ERR_SELECT,0,tmpstr,__FILE__,__LINE__);
  }
  ds.Close();

/*Query input_interface_id*/
  int input_id;
  ds.Open("select input_id from workflow where workflow_id=:workflow_id and process_id=:process_id ");
  ds<<workflow_id<<Process_id;
  ds>>input_id;
  if (ds.IsEnd()) 
  {
   	sprintf(tmpstr,"select from workflow err:no match "
   	  "workflow_id=%d & process_id=%d!",workflow_id,Process_id);
	throw CF_CError('E','H',ERR_SELECT,0,tmpstr,__FILE__,__LINE__);
  }
  ds.Close();

/*Query ctl_tabname*/
  ds.Open("select ctl_tabname from model_interface where interface_id=:input_id");
  ds<<input_id;
  ds>>m_SzSchedule;
  if (ds.IsEnd())
  {
   	sprintf(tmpstr,"select from model_interface err:no match "
   	  "interface_id=%d!",input_id);
	throw CF_CError('E','H',ERR_SELECT,0,tmpstr,__FILE__,__LINE__);
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
	throw CF_CError('E','H',ERR_SELECT,0,(char *)"select from STAT_TABLE_DEFINE err:"
	    "no match CONFIG_ID!",__FILE__,__LINE__);
    return (-1);
  }  
  
  ds>>m_SStat_Table.ChStat_Type
    >>m_SStat_Table.ChIs_Count>>m_SStat_Table.SzCount_Item_Name;
  if (ds.IsEnd()) 
  {
	throw CF_CError('E','H',ERR_SELECT,0,(char *)"select from STAT_TABLE_DEFINE err:"
	    "no match CONFIG_ID!",__FILE__,__LINE__);
  	return (-1);
  }
  ds.Close();	
  DeleteSpace(m_SStat_Table.SzCount_Item_Name);

  if(para_config_id<0)
  	{
  	m_SStat_Table.ChStat_Type='-';
  	}
  
  //查询统计模式insert or update
  ds.Open("select Stat_Mode from "
    "STAT_TABLE_DEFINE where config_id=:config_id");
  ds<<config_id;
  ds>>m_SStat_Table.CHStat_Mode;
  if (ds.IsEnd()) 
  {
	throw CF_CError('E','H',ERR_SELECT,0,(char *)"select from STAT_TABLE_DEFINE err:"
	    "no match CONFIG_ID!",__FILE__,__LINE__);
    return (-1);
  } 
  if(m_SStat_Table.CHStat_Mode!='I')
  	 m_SStat_Table.CHStat_Mode='U';

  //20080604
    try
  	{
  	if(m_SStat_Table.CHStat_Mode=='I')
	  	m_PsPre_Stat_Record = new STAT_RECORD[maxRcdNumToUpdat];
	else
	  	Stat_Record_toUpdate = new STAT_RECORD[maxRcdNumToUpdat];
	Current_Memorry_Unit=maxRcdNumToUpdat;
  	}
  catch(...)
	{
	throw CF_CError('E','H',ERR_REQU_MEM,0,(char *)"new err.",__FILE__,__LINE__);
	}
  
  //cout<<"m_SStat_Table.CHStat_Mode==="<<m_SStat_Table.CHStat_Mode<<endl;
 //查询统计结果输出方式，初始化结果输出方式及路径
  ds.Open("select Output_Des from "
             "STAT_TABLE_DEFINE where config_id=:config_id");
  ds<<config_id;
  ds>>Result_File_Path;  
  DeleteSpace(Result_File_Path);
  if(strlen(Result_File_Path)==0)
	Result_Output_Des='T';
  else  	
  	{  	
  	Result_Output_Des='F';    	
  	if (Result_File_Path[strlen(Result_File_Path)-1] != '/')
   		strcat(Result_File_Path, "/");        
  	}

  char Out_Record_Type_ID;
  if(Result_Output_Des=='F')
  {
    //读stat_table_define,获取Out_File_type_id 
	ds.Open("select Out_File_Type_ID from STAT_TABLE_DEFINE where config_id=:config_id");
	ds<<config_id;	
	ds>>Out_File_Type_ID;
	DeleteSpace(Out_File_Type_ID);
    if(strlen(Out_File_Type_ID)==0)
    {      
      throw CF_CError('E','H',ERR_FILED_VAL,0,(char *)"table STAT_TABLE_DEFINE error:Out_File_Type_ID is null!",__FILE__,__LINE__);
  	  return (-1);
	}
    Temp_Out_File.Init(Out_File_Type_ID);
    
    //读 filetype_define 表获得out_record_type_id
	ds.Open("select record_type from filetype_define where filetype_id=:id");
	ds<<Out_File_Type_ID;
	if(!(ds>>Out_Record_Type_ID))
		{
		throw CF_CError('E','H',ERR_SELECT,0,(char *)"select from filetype_define:"
	    			"no match filetype_id!",__FILE__,__LINE__);
  	      return (-1);
		}
	Out_Record.Init(Out_File_Type_ID,Out_Record_Type_ID);
	
  }

//查询统计表结构
 // cout<<"config id "<<config_id<<"stat_item:---"<<endl;
  ds.Open("select count(*) from STAT_TABLE_FMT where config_id=:config_id");
  ds<<config_id;
  ds>>m_SStat_Table.NStat_Item_Count;
  if (ds.IsEnd()) 
  {
	throw CF_CError('E','H',ERR_SELECT,0,(char *)"select from STAT_TABLE_FMT err:"
	    "no match CONFIG_ID!",__FILE__,__LINE__);
  	return (-1);
  }
  ds.Close();	
  if(m_SStat_Table.NStat_Item_Count>STAT_ITEM_COUNT)
  {
    throw CF_CError('E','H',ERR_TABLEITEM_OVER,0,(char *)"table item count is over uplimit!",__FILE__,__LINE__);
	return (-1);
  }

  //查询已有统计表
  char sql[200];
  char table_name[200];
  char nameForQuery[200];
  sprintf(nameForQuery,"%s_",m_SStat_Table.SzTable_Name);
  strcat(nameForQuery,"%");
  cout<<"nameForQuery="<<nameForQuery<<endl;
  //sprintf(sql,"SELECT table_name FROM user_tables WHERE table_name like '%s_%'",m_SStat_Table.SzTable_Name);
  ds.Open("SELECT table_name FROM user_tables WHERE table_name like :v");
  ds<<nameForQuery;
  while(ds>>table_name)
  	{
  	m_SStat_Table.v_tableName.push_back(table_name);
  	}
  curr_table_count=m_SStat_Table.v_tableName.size();
  cout<<"v_tableName.size()="<<curr_table_count<<endl;
  
  /* select table format*/
  
  //Row_Or_Col_Flag='R';
  //int istat_item_index=-1;
  //int istat_value_index=-1;
  char CField_Index[STAT_ITEM_COUNT][STAT_ITEM_CONTAINER_LEN];

  int i=0;
  //20080220，动态生成结果表
  //先查询表名关键字
  ds.Open("select count(*) from stat_table_fmt a where a.IS_TABLE_NAME_KEY='Y'  and a.CONFIG_ID=:config_id");
  ds<<config_id;
  ds>>m_SStat_Table.table_name_key_count;
  if(m_SStat_Table.table_name_key_count!=0)
  	{
  	/*
  	if(m_SStat_Table.CHStat_Mode=='I')
  		{
  		throw CF_CError('E','H',ERR_SELECT,0,(char *)"can't create table in insert_mode stat!",__FILE__,__LINE__);
	  	return (-1);
  		}
  		*/
	  ds.Open("select Table_Item,Item_Type,Field_Index,Field_Begin,Field_End,Defv_Or_Func"
	    ",nvl(indexinfield,-1),nvl(sprinfield,' '),nvl(unit,' ') from STAT_TABLE_FMT where "
	    "IS_TABLE_NAME_KEY='Y'  and config_id=:config_id order by Table_Item");
	  ds<<config_id;
	  for( i=0;i<m_SStat_Table.table_name_key_count;i++)
	  	{
	  	ds>>m_SStat_Table.SzStat_Item[i]>>m_SStat_Table.NItem_Type[i]
		      >>CField_Index[i]>>m_SStat_Table.NStat_Item_Begin[i]
		      >>m_SStat_Table.NStat_Item_End[i]>>m_SStat_Table.Predfval_Or_Statid[i]
		      >>m_SStat_Table.NStat_Item_IndexInField[i]>>m_SStat_Table.chStat_Item_SprInField[i]
		      >>m_SStat_Table.ChCount_Item_Unit[i];
		DeleteSpace(m_SStat_Table.SzStat_Item[i]);
		DeleteSpace(m_SStat_Table.Predfval_Or_Statid[i]);
		DeleteSpace(m_SStat_Table.ChCount_Item_Unit[i]);
		if(m_SStat_Table.NStat_Item_Begin[i]<1 && m_SStat_Table.NStat_Item_Begin[i]!=-1)
		  	{
		  	sprintf(errmsg,"%d:%s Field_Begin error:%d",i,m_SStat_Table.SzStat_Item[i],m_SStat_Table.NStat_Item_Begin[i]);
		    	throw CF_CError('E','H',ERR_GET_SUBSTR,0,errmsg,__FILE__,__LINE__);
		  	}
		m_SStat_Table.NField_Index[i]=atol(CField_Index[i]);
	  	}
	 }
  cout<<m_SStat_Table.table_name_key_count<<"table_name_key"<<endl;
  //再查询表结构
  char isTableMainKey;
  i=m_SStat_Table.table_name_key_count;

  ds.Open("select Table_Item,Item_Type,Field_Index,Field_Begin,Field_End,Defv_Or_Func"
    ",nvl(indexinfield,-1),nvl(sprinfield,' '),nvl(unit,' ') ,IS_TABLE_NAME_KEY ,DATA_TYPE,IS_INDEX "
    "from STAT_TABLE_FMT where config_id=:config_id  ");
  ds<<config_id;
  for( int j=0;j<m_SStat_Table.NStat_Item_Count;j++)
  {
    ds>>m_SStat_Table.SzStat_Item[i]>>m_SStat_Table.NItem_Type[i]
      >>CField_Index[i]>>m_SStat_Table.NStat_Item_Begin[i]
      >>m_SStat_Table.NStat_Item_End[i]>>m_SStat_Table.Predfval_Or_Statid[i]
      >>m_SStat_Table.NStat_Item_IndexInField[i]>>m_SStat_Table.chStat_Item_SprInField[i]
      >>m_SStat_Table.ChCount_Item_Unit[i]      
      >>isTableMainKey>>m_SStat_Table.NField_dataType[i]
      >>m_SStat_Table.IsIndex[i];
    if(isTableMainKey=='Y')
    	{
		continue;
    	}
    if(m_SStat_Table.IsIndex[i]!='Y')
		m_SStat_Table.IsIndex[i]='N';
	
    DeleteSpace(m_SStat_Table.SzStat_Item[i]);
    DeleteSpace(m_SStat_Table.Predfval_Or_Statid[i]);
    DeleteSpace(m_SStat_Table.ChCount_Item_Unit[i]);
    if(m_SStat_Table.NStat_Item_Begin[i]<1 && m_SStat_Table.NStat_Item_Begin[i]!=-1)
  	{
    	sprintf(errmsg,"%d:%s Field_Begin error:%d",i,m_SStat_Table.SzStat_Item[i],m_SStat_Table.NStat_Item_Begin[i]);
	throw CF_CError('E','H',ERR_GET_SUBSTR,0,errmsg,__FILE__,__LINE__);
  	}
    
   /* if(strcmp(m_SStat_Table.SzStat_Item[i],"STAT_ITEM")==0)  
    	{
    	Row_Or_Col_Flag='C';//竖表统计标识
    	istat_item_index=i;    	
    	}
    else if(strcmp(m_SStat_Table.SzStat_Item[i],"STAT_VALUE")==0)
    	{
    	istat_value_index=i;
    	}
    else*/
    	m_SStat_Table.NField_Index[i]=atol(CField_Index[i]);
	
	//cout<<m_SStat_Table.SzStat_Item[i]<<"|"<<m_SStat_Table.NField_Index[i]<<endl;
	i++;
  }
  ds.Close();
  
//20080604
  /*
 //若为竖表，填充s_col_stat_item结构体，计算统计项标识个数，
  if (Row_Or_Col_Flag=='C')
  	{
  	char *Temp=NULL;
  	delSpace(CField_Index[istat_item_index],0);
  	char *TmpPoint1=NULL;
  	char *TmpPoint2=NULL;
  	int iTmpCount=0;
  	Temp=CField_Index[istat_item_index];
    //取得存在于话单中的统计标识组的个数
    TmpPoint1=Temp;	
  	while(1)
  		{
	    	iTmpCount++;
	    	TmpPoint2=strchr(TmpPoint1,m_SStat_Table.chStat_Item_SprInField[istat_item_index]);
	    	if(TmpPoint2==NULL) 
	    		break;
	    	TmpPoint1=TmpPoint2+1;
  		}  	
  	s_Col_Stat_Item.iStat_Item_Group_Num=iTmpCount;
  	if(s_Col_Stat_Item.iStat_Item_Group_Num>STAT_GROUP_COUNT)
  		{
          	throw CF_CError('E','H',ERR_STATGROUP_OVER,0,(char *)"stat item group count is more than that is defined",
          			__FILE__,__LINE__);
	      return (-1);
        	}  			    		
  	//填充ID_Group_Filed_Index[]
  	TmpPoint1=Temp;
 	int i=0;
  	for(i=1;i<s_Col_Stat_Item.iStat_Item_Group_Num;i++)
	  	{
	    	TmpPoint2=strchr(TmpPoint1,m_SStat_Table.chStat_Item_SprInField[istat_item_index]);
	    	*TmpPoint2=0;
	    	s_Col_Stat_Item.ID_Group_Filed_Index[i-1]=atol(TmpPoint1);
	    	TmpPoint1=TmpPoint2+1;
	  	}
  	s_Col_Stat_Item.ID_Group_Filed_Index[i-1]=atol(TmpPoint1);	
  	//开始填充Value_Group_Field_Index[]
  	delSpace(CField_Index[istat_value_index],0);
  	TmpPoint1=CField_Index[istat_value_index];
 	int ii=0;
  	for(ii=1;ii<s_Col_Stat_Item.iStat_Item_Group_Num;ii++)
	  	{
	    	TmpPoint2=strchr(TmpPoint1,m_SStat_Table.chStat_Item_SprInField[istat_item_index]);
	    	*TmpPoint2=0;
	    	s_Col_Stat_Item.Value_Group_Field_Index[ii-1]=atol(TmpPoint1);
	    	TmpPoint1=TmpPoint2+1;
	  	}
  	s_Col_Stat_Item.Value_Group_Field_Index[ii-1]=atol(TmpPoint1);	
  	
  	}
  */
/************************************************************************************/
/*                   按条件统计功能改用表达式实现                   					*/
/*************************************************************************************/
	for(int item=0;item<m_SStat_Table.NStat_Item_Count;item++)
		{
		m_SStat_Table.item_cond_index[item]=-1;
		}
    
	ds.Open("select CONDICTION from STAT_TABLE_DEFINE where CONFIG_ID=:id");
	ds<<config_id;
	ds>>m_SStat_Table.ChCondiction;
	
	if (ds.IsEnd()) 
  	{
		throw CF_CError('E','H',ERR_SELECT,0,(char *)"select from STAT_TABLE_FMT err:"
	  	  		"no match CONFIG_ID!",__FILE__,__LINE__);
  		return (-1);
  	}
  	ds.Close();	
	DeleteSpace(m_SStat_Table.ChCondiction);
	
	if(strlen(m_SStat_Table.ChCondiction)!=0)
		{
		if(Read_Cond_field()==-1)
			{
	          	throw CF_CError('E','H',ERR_EXPRESS,0,(char *)"error expression in STAT_TABLE_DEFINE",
	          		__FILE__,__LINE__);
		      	return (-1);
	        	}  
		char temp[250];
	      	sprintf(temp,"  ");
	      	strcat(temp,m_SStat_Table.ChCondiction);	      
	      	strcpy(m_SStat_Table.ChCondiction,temp);		
		
		for(int field=0;field<m_SStat_Table.cond_field_count;field++)
		if(!theCompile.DefineVariable(m_SStat_Table.cond_field[field].szField_Value,
				m_SStat_Table.cond_field_value[field].szField_Value))
    			{
    			sprintf(tmpstr,"Define Variable %s error.",m_SStat_Table.cond_field[field].szField_Value);
    			throw CF_CError('E','H',ERR_EXPRESS,0,tmpstr,__FILE__,__LINE__);
    			return -1;
    			} 
		}
	return 0;
}

//数条件表达式中的变量，存于cond_field 中
int CF_CStat::Read_Cond_field()
{
	char *p_begin,*p_end1,*p_end2,*p_end,*point=m_SStat_Table.ChCondiction;
			
	while((p_begin=strchr(point,'$'))!=NULL)
		{
		p_end1=strchr(p_begin,',');
		p_end2=strchr(p_begin,')');
		if(p_end1!=NULL && p_end2!=NULL)
			p_end=(p_end1 < p_end2 ? p_end1 : p_end2);					
		else if(p_end1!=NULL)
			p_end=p_end1;
		else if(p_end2!=NULL)
			p_end=p_end2;
		m_SStat_Table.cond_field_count++;
		point=p_end;	
		}
	if(m_SStat_Table.cond_field_count==0)
		return -1;
	m_SStat_Table.cond_field=new FIELD_VALUE[m_SStat_Table.cond_field_count];
	m_SStat_Table.cond_field_value=new FIELD_VALUE[m_SStat_Table.cond_field_count];
	if(m_SStat_Table.cond_field==NULL || m_SStat_Table.cond_field_value==NULL)
    	{
    	throw CF_CError('E','H',ERR_REQU_MEM,0,(char *)"new err.",__FILE__,__LINE__);
    	return -1;
    	}
	int i=0;//变量个数
	point=m_SStat_Table.ChCondiction;
	while((p_begin=strchr(point,'$'))!=NULL)
		{
		p_end1=strchr(p_begin,',');
		p_end2=strchr(p_begin,')');
		if(p_end1!=NULL && p_end2!=NULL)
			p_end=(p_end1 < p_end2 ? p_end1 : p_end2);					
		else if(p_end1!=NULL)
			p_end=p_end1;
		else if(p_end2!=NULL)
			p_end=p_end2;		
		char tmp_field[20];
		int is_exist=0;
		strncpy(tmp_field,p_begin+1,p_end-p_begin-1);
		tmp_field[p_end-p_begin-1]=0;
		for(int var=0;var<i;var++)
			{
			if(strcmp(tmp_field,m_SStat_Table.cond_field[var].szField_Value)==0)
				{
				is_exist=1;
				break;				
				}
			}
		if(is_exist==0)
			{
			strcpy(m_SStat_Table.cond_field[i].szField_Value,tmp_field);
			i++;
			}
	  	point=p_end;		  
		}
	m_SStat_Table.cond_field_count=i;
	for(int field=0;field<m_SStat_Table.cond_field_count;field++)
		{
		m_SStat_Table.cond_field[field].index=0;
		}
	return 0;
}

//设置结果输出文件名、临时文件名,并打开临时输出文件
void CF_CStat::Set_TempOutFile(char *today, int group_id)
{	  	
 if(Result_Output_Des=='F')
 	{ 	
	sprintf(Temp_Out_File_Name,"%s%s.%s.%d.stat.tmp",
			  Result_File_Path,m_SStat_Table.SzTable_Name,today,group_id);
	sprintf(Out_File_Name,"%s%s.%s.%d.stat",
		  	Result_File_Path,m_SStat_Table.SzTable_Name,today,group_id);
	Temp_Out_File.Open(Temp_Out_File_Name,'A');
 	}
}


void CF_CStat::Close_Temp_Outfile()
{
	if(Result_Output_Des=='F')
	{ 
    	Temp_Out_File.Close();
	}
}

//删除临时文件
void CF_CStat::Unlink_TemFile()
{
	if(Result_Output_Des=='F')
	{ 
		unlink(Temp_Out_File_Name);
	}
}
void CF_CStat::Unlink_File()
{
	if(Result_Output_Des=='F')
	{ 
		unlink(Out_File_Name);
	}
}

//将临时文件更名为正式文件
int CF_CStat::Rename_OutFile()
{
	if(Result_Output_Des=='F')
	{
		int i=2;
		while(1)
			{
			if(access(Out_File_Name, 0)!=0)
				break;
			else
				{
				Out_File_Name[strlen(Out_File_Name)-5]=0;
				char flag[3];
				sprintf(flag,"_%d.stat",i);
				strcat(Out_File_Name,flag);
				i++;
				}
			}	    	
		return(rename(Temp_Out_File_Name,Out_File_Name));
	}
	else 
		return 0;
}
//20070509,由于效率问题，修改该接口，sourceid和dealstarttime尽可能从外部传入
int CF_CStat::setFileName(char *FileName,char *sourceid,char *dealstarttime)
{
/*
	CBindSQL ds(DBConn);
	char sqltmp[400];
	char tmpstr[400];
	sprintf(m_SzFileName,"%s",FileName);
	memset(m_SzSource_ID,0,STAT_ITEM_NAME_LEN);
	memset(m_SzStat_Time,0,STAT_ITEM_NAME_LEN);
	//update:20060713,+、-统计m_SzStat_Time 统一用输入控制表dealstarttime
	sprintf(sqltmp,"select source_id, dealstarttime from %s where filename='%s'",
	  m_SzSchedule,FileName);
	ds.Open(sqltmp);
	ds>>m_SzSource_ID>>m_SzStat_Time;
	if (ds.IsEnd()) 
	{
		  sprintf(tmpstr,"select from %s err:no match filename:%s!",m_SzSchedule,FileName);
	  throw CF_CError('E','H',ERR_SELECT,0,tmpstr,__FILE__,__LINE__);
	}
	ds.Close();
	DeleteSpace(m_SzSource_ID);
	DeleteSpace(m_SzStat_Time);
*/
	sprintf(m_SzFileName,"%s",FileName);
	sprintf(m_SzSource_ID,"%s",sourceid);
	sprintf(m_SzStat_Time,"%s",dealstarttime);
	
	//判断是否在截止时间以前，20070314
	strncpy(crrdate,m_SzStat_Time,8);
	crrdate[8]=0;
	char time[5];
	int i=0;
	for(i=0;i<4;i++)
		{
		time[i]=m_SzStat_Time[8+i];
		}
	time[4]=0;
	if(atoi(time)<atoi(endTime))
		before_endtime_flag=1;
	char lastdatetime[15];
	get_preday_time(lastdatetime);
	strncpy(lastdate,lastdatetime,8);
	lastdate[8]=0;
	/*
	cout<<"crrTime:"<<m_SzStat_Time<<endl;
	cout<<" dataType_endTime:"<<endTime<<" sub crrTime:"<<time<<endl;;
	cout<<" before_endtime_flag="<<before_endtime_flag<<" lastdate:"<<lastdate;
	cout<<" crrdate="<<crrdate<<endl;
	*/
    return 0;
}
//20070509,sourceid从外部传入
int CF_CStat::setFileName(char *FileName,char *sourceid)
{
	CBindSQL ds(DBConn);
	char sqltmp[400];
	char tmpstr[400];
	sprintf(m_SzFileName,"%s",FileName);
	sprintf(m_SzSource_ID,"%s",sourceid);
	memset(m_SzStat_Time,0,STAT_ITEM_NAME_LEN);
	//update:20060713,+、-统计m_SzStat_Time 统一用输入控制表dealstarttime
	sprintf(sqltmp,"select dealstarttime from %s where source_id='%s' and filename='%s'",
	  m_SzSchedule,sourceid,FileName);
	ds.Open(sqltmp);
	ds>>m_SzStat_Time;
	if (ds.IsEnd()) 
	{
		  sprintf(tmpstr,"err:%s!",sqltmp);
	  	  throw CF_CError('E','H',ERR_SELECT,0,tmpstr,__FILE__,__LINE__);
	}
	ds.Close();	
	DeleteSpace(m_SzStat_Time);
	
	//判断是否在截止时间以前，20070314
	strncpy(crrdate,m_SzStat_Time,8);
	crrdate[8]=0;
	char time[5];
	int i=0;
	for(i=0;i<4;i++)
		{
		time[i]=m_SzStat_Time[8+i];
		}
	time[4]=0;
	if(atoi(time)<atoi(endTime))
		before_endtime_flag=1;
	char lastdatetime[15];
	get_preday_time(lastdatetime);
	strncpy(lastdate,lastdatetime,8);
	lastdate[8]=0;
	/*
	cout<<"crrTime:"<<m_SzStat_Time<<endl;
	cout<<" dataType_endTime:"<<endTime<<" sub crrTime:"<<time<<endl;;
	cout<<" before_endtime_flag="<<before_endtime_flag<<" lastdate:"<<lastdate;
	cout<<" crrdate="<<crrdate<<endl;
	*/
    return 0;
}

//20080720,增加对process_id的支持
int CF_CStat::setFileName(char *FileName,char *sourceid,char *dealstarttime,int process_id)
{
	sprintf(m_SzFileName,"%s",FileName);
	sprintf(m_SzSource_ID,"%s",sourceid);
	sprintf(m_SzStat_Time,"%s",dealstarttime);
	sprintf(m_SzStat_process_id,"%d",process_id);
	//判断是否在截止时间以前，20070314
	strncpy(crrdate,m_SzStat_Time,8);
	crrdate[8]=0;
	char time[5];
	int i=0;
	for(i=0;i<4;i++)
		{
		time[i]=m_SzStat_Time[8+i];
		}
	time[4]=0;
	if(atoi(time)<atoi(endTime))
		before_endtime_flag=1;
	char lastdatetime[15];
	get_preday_time(lastdatetime);
	strncpy(lastdate,lastdatetime,8);
	lastdate[8]=0;

    return 0;
}

/*
//竖表读话单，生成N条记录
int CF_CStat::Stat_Record_Col(CFmt_Change &InRecord, int IError_Type)
{  
  char TmpLogMsg[400];  
  STAT_RECORD TMP_RECORD;  
  int statid_array[MAX_STATID_NUM];
  
  int statvalue_array[MAX_STATID_NUM];
  char sGroup1[500];  
  char tmp[100];

  int statid_index;
  int statvalue_index;
  
  //按条件统计，用表达式实现
  //不满足条件则不统计
    if(first_time_read_record==1)
  	{
  	first_time_read_record=0;
  	for(int field=0;field<m_SStat_Table.cond_field_count;field++)
  		{
  		m_SStat_Table.cond_field[field].index=InRecord.Get_FieldIndex(m_SStat_Table.cond_field[field].szField_Value);
		if(m_SStat_Table.cond_field[field].index==-1)
			for(int item=0;item<m_SStat_Table.NStat_Item_Count;item++)
  			{
  			if(strcmp(m_SStat_Table.SzStat_Item[item],m_SStat_Table.cond_field[field].szField_Value)==0)
  				{
  				//统计表的第item个字段对应条件表达式中的第j个变量
  				m_SStat_Table.item_cond_index[item]=field;
  				break;
  				}
  			}
		else cond_variable_in_record_count++;//条件表达式中名为话单字段名的变量个数
  	    }
  	}
  
  for(int j=0;j<m_SStat_Table.cond_field_count;j++)
  	{
  	if(m_SStat_Table.cond_field[j].index!=-1)
  		{
  		sprintf(m_SStat_Table.cond_field_value[j].szField_Value,InRecord.Get_Field(m_SStat_Table.cond_field[j].index)); 
  		} 
  	}  
  if(m_SStat_Table.cond_field_count>0 && cond_variable_in_record_count==m_SStat_Table.cond_field_count)
	{
  	char Result[255] = "";
  	int ErrorNo=0;
  	char curr_condiction[250];
  	//sprintf(curr_condiction,m_SStat_Table.ChCondiction);
  	theCompile.Operation(Result, sizeof(Result)-1, &ErrorNo, m_SStat_Table.ChCondiction);
  	if(ErrorNo==0 && strcmp(Result,"false")==0)
  		return 1;
  	else if(ErrorNo!=0)
  		{
  		sprintf(TmpLogMsg,"error condiction expression.ErrorNo[%d]",ErrorNo);
  		throw CF_CError('E','H',ERR_EXPRESS,0,TmpLogMsg,__FILE__,__LINE__);
  		}
	}

  
  for(int i=0;i<m_SStat_Table.NStat_Item_Count;i++)
  {   
  //检查该字段是否有指定值
        DeleteSpace(m_SStat_Table.Predfval_Or_Statid[i]);
	  	if(strlen(m_SStat_Table.Predfval_Or_Statid[i])!=0)
	  		{
	  		char *container=strchr(m_SStat_Table.Predfval_Or_Statid[i],':');
	  		container++;
	  		if(m_SStat_Table.Predfval_Or_Statid[i][0]=='C')
	  			{
	  			strcpy(TMP_RECORD.SzStat_Item[i],container);
	  			continue;
	  			}
	  		//取某个函数值，暂时没有需要
	  		else
	  			{	  		
	  			
	  			}
	  		}
    //关键字< 11
   	if(m_SStat_Table.NItem_Type[i]<KEYWORK_MIN)
	  	{
	  	//若是统计项
	  	if(strcmp(m_SStat_Table.SzStat_Item[i],"STAT_ITEM")==0 ||strcmp(m_SStat_Table.SzStat_Item[i],"stat_item")==0)
	  		{	
	  			statid_index=i;
	  			
	  			char ToString[2];
  				ToString[0]=m_SStat_Table.chStat_Item_SprInField[i];
  				ToString[1]=0;
  				sprintf(sGroup1,"%s",InRecord.Get_Field(s_Col_Stat_Item.ID_Group_Filed_Index[0])); 				
  				for(int group=1;group<s_Col_Stat_Item.iStat_Item_Group_Num;group++)
  				{  	
  					sprintf(tmp,"%s",InRecord.Get_Field(s_Col_Stat_Item.ID_Group_Filed_Index[group]));
  					delSpace(tmp,0); 
  					strcat(sGroup1,tmp);       
   				 	strcat(sGroup1,ToString);    
  				}
  				if(s_Col_Stat_Item.iStat_Item_Group_Num>1)
  					sGroup1[strlen(sGroup1)]=0;  
  				istat_item_numb=Count_Item(sGroup1,m_SStat_Table.chStat_Item_SprInField[i]);
  				
  				if(istat_item_numb>MAX_STATID_NUM)
  				{
    				throw CF_CError('E','H',ERR_STATIEMT_OVER,0,(char *)"stat item id count is more than definition!",__FILE__,__LINE__);
					return (-1);
  				}    
  				Get_Item(sGroup1,m_SStat_Table.chStat_Item_SprInField[i],statid_array,istat_item_numb);
	  		}
	  	//若是统计值
	  	else if(strcmp(m_SStat_Table.SzStat_Item[i],"STAT_VALUE")==0 ||strcmp(m_SStat_Table.SzStat_Item[i],"stat_value")==0)
	  		{	 
	  			statvalue_index=i;
	  			
	  		    char ToString[2];
  				ToString[0]=m_SStat_Table.chStat_Item_SprInField[i];
  				ToString[1]=0;
  				
	  			sprintf(sGroup1,"%s",InRecord.Get_Field(s_Col_Stat_Item.Value_Group_Field_Index[0]));
  				for(int group=1;group<s_Col_Stat_Item.iStat_Item_Group_Num;group++)
  				{  	
  					sprintf(tmp,"%s",InRecord.Get_Field(s_Col_Stat_Item.Value_Group_Field_Index[group]));
  					delSpace(tmp,0); 
  					strcat(sGroup1,tmp);       
    				strcat(sGroup1,ToString); 
  				}    				
  				if(s_Col_Stat_Item.iStat_Item_Group_Num>1)
    				sGroup1[strlen(sGroup1)]=0;    
  				//int istat_value_numb=Count_Item(sGroup1,m_SStat_Table.chStat_Item_SprInField[i]);  			
  				Get_Item(sGroup1,m_SStat_Table.chStat_Item_SprInField[i],statvalue_array,istat_item_numb);
	  		}
	  	//一般情况
	  	else
	  	 {
	  		if(m_SStat_Table.NStat_Item_Begin[i]==(-1))
	  	    {
			if(m_SStat_Table.NStat_Item_IndexInField[i]==(-1))
			  sprintf(TMP_RECORD.SzStat_Item[i],
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
		      	sprintf(TMP_RECORD.SzStat_Item[i],"%s",TmpPoint1);
            	else
            	{
    			  sprintf(TmpLogMsg,"Field %s Index In Field is less than Define.",m_SStat_Table.SzStat_Item[i]);
             	  throw CF_CError('D','H',ERR_FIELD_INDEX,0,TmpLogMsg,__FILE__,__LINE__);
           	 	}
          	}
          	else
          	   {
           	    *TmpPoint2=0;
           	    sprintf(TMP_RECORD.SzStat_Item[i],
		    	  "%s",TmpPoint1);
          	   }
			}
	  	}
	  	//m_SStat_Table.NStat_Item_Begin[i]!=(-1)
		else 
		{
	    //判断字段是否有规定取的那么长
		int NField_Len = strlen(InRecord.Get_Field(m_SStat_Table.NField_Index[i]));
		if((NField_Len<m_SStat_Table.NStat_Item_End[i])||
		  (m_SStat_Table.NStat_Item_End[i]<m_SStat_Table.NStat_Item_Begin[i]))
			{
		  	sprintf(TmpLogMsg,"Field %s Field_Len is not long enough.",m_SStat_Table.SzStat_Item[i]);
          	throw CF_CError('E','H',ERR_FIELD_INDEX,0,TmpLogMsg,__FILE__,__LINE__);
			}
		NField_Len=m_SStat_Table.NStat_Item_End[i]-m_SStat_Table.NStat_Item_Begin[i]+1;
		sprintf(TMP_RECORD.SzStat_Item[i],"%s",
		  InRecord.Get_Field(m_SStat_Table.NField_Index[i])
		  +m_SStat_Table.NStat_Item_Begin[i]-1);
		TMP_RECORD.SzStat_Item[i][NField_Len]=0;
		}
		if(!strlen(TMP_RECORD.SzStat_Item[i]))
		 {
	 	 sprintf(TmpLogMsg,"Field %s is NULL.",m_SStat_Table.SzStat_Item[i]);
     	 throw CF_CError('D','H',ERR_FIELD_NULL,0,TmpLogMsg,__FILE__,__LINE__);
		 }
	   }
	  }
   	
   	//对通话时长做特别处理，分31个区间
   	else if(m_SStat_Table.NItem_Type[i]==KEYWORK_CDRDURATION)
    {      
	   	int cdr_time=atol(InRecord.Get_Field(m_SStat_Table.NField_Index[i]))/60+1;
	  	if (cdr_time<=30)
	  		sprintf(TMP_RECORD.SzStat_Item[i],"%d",cdr_time);
	  	else
	  		sprintf(TMP_RECORD.SzStat_Item[i],"%s",">30");
	}
   	 else if(m_SStat_Table.NItem_Type[i]==KEYWORK_ERRORTYPE)
  	{
  	sprintf(TMP_RECORD.SzStat_Item[i],"%d",IError_Type);
  	}
   	//其余关键字
    else
    {
    	Set_KeyStatItem(TMP_RECORD,i);
    }    
    
  }  

  //赋值统计表字段名变量
  for(int item=0;item<m_SStat_Table.NStat_Item_Count;item++)
  	{
  	if(item!=statid_index && item!=statvalue_index && m_SStat_Table.item_cond_index[item]!=-1)
  		{
  		sprintf(m_SStat_Table.cond_field_value[m_SStat_Table.item_cond_index[item]].szField_Value,
  		TMP_RECORD.SzStat_Item[item]); 
  		}
  	}
  
 //开始生成icol_stat_insert_numb 条记录
 icol_stat_insert_numb=0;
 
 for(int irecord=0;irecord<istat_item_numb;irecord++)
  	{  	
  	    //////////////////////////////////////////////////////////////////////////////
  		//处理表达式
  			if(m_SStat_Table.item_cond_index[statid_index]!=-1)
  			{
  			sprintf(m_SStat_Table.cond_field_value[m_SStat_Table.item_cond_index[statid_index]].szField_Value,
  				"%d",statid_array[irecord]); 
  			}
  			if(m_SStat_Table.item_cond_index[statvalue_index]!=-1)
  			{
  			sprintf(m_SStat_Table.cond_field_value[m_SStat_Table.item_cond_index[statvalue_index]].szField_Value,
  				"%d",statvalue_array[irecord]); 
  			}
  			if(m_SStat_Table.cond_field_count>0 && cond_variable_in_record_count != m_SStat_Table.cond_field_count)
			{
  			char Result[255] = "";
  			int ErrorNo=0;  			
  			theCompile.Operation(Result, sizeof(Result)-1, &ErrorNo, m_SStat_Table.ChCondiction);
  			if(ErrorNo==0 && strcmp(Result,"false")==0)
  				return 1;
  			else if(ErrorNo!=0)
  				{
  				sprintf(TmpLogMsg,"error condiction expression.ErrorNo[%d]",ErrorNo);
  				throw CF_CError('E','H',ERR_EXPRESS,0,TmpLogMsg,__FILE__,__LINE__);
  				}
			}
  	    //////////////////////////////////////////////////////////////////////////////////
  	    //生成一条记录
  		for(int item=0;item<m_SStat_Table.NStat_Item_Count;item++)
  		{
  			//统计项
  			if(strcmp(m_SStat_Table.SzStat_Item[item],"STAT_ITEM")==0
  				||strcmp(m_SStat_Table.SzStat_Item[item],"stat_item")==0)
  			{
  			sprintf(m_PsPre_Stat_Record[m_NRecord_Count+icol_stat_insert_numb].SzStat_Item[item],
  				"%d", statid_array[irecord]);
  			m_PsPre_Stat_Record[m_NRecord_Count+icol_stat_insert_numb].NItem_Type[item]=m_SStat_Table.NItem_Type[item];
  			}
  			//统计值
  			else if(strcmp(m_SStat_Table.SzStat_Item[item],"STAT_VALUE")==0
  				   ||strcmp(m_SStat_Table.SzStat_Item[item],"stat_value")==0)
  			{
  			sprintf(m_PsPre_Stat_Record[m_NRecord_Count+icol_stat_insert_numb].SzStat_Item[item],
  				  "%d",statvalue_array[irecord]);
  	        m_PsPre_Stat_Record[m_NRecord_Count+icol_stat_insert_numb].NItem_Type[item]=m_SStat_Table.NItem_Type[item];
  			}
  			//普通字段
  			else
  			{
   			strcpy(m_PsPre_Stat_Record[m_NRecord_Count+icol_stat_insert_numb].SzStat_Item[item],
   				  TMP_RECORD.SzStat_Item[item]); 
   		    m_PsPre_Stat_Record[m_NRecord_Count+icol_stat_insert_numb].NItem_Type[item]=m_SStat_Table.NItem_Type[item];
  			}
  		}
  		
  		m_PsPre_Stat_Record[m_NRecord_Count+icol_stat_insert_numb].NStat_Item_Count=m_SStat_Table.NStat_Item_Count;
  		icol_stat_insert_numb++;
  	} 
  
 return 0;
 
}

*/
//横表读话单，生成一条记录，添加通话时长统计
int CF_CStat::Stat_Record_Insert(CFmt_Change &InRecord , int IError_Type)
{
  char TmpLogMsg[400];

  //按条件统计，用表达式实现
  //不满足条件则不统计
  if(first_time_read_record==1)
  	{
  	first_time_read_record=0;
  	for(int field=0;field<m_SStat_Table.cond_field_count;field++)
  		{
  		m_SStat_Table.cond_field[field].index=InRecord.Get_FieldIndex(m_SStat_Table.cond_field[field].szField_Value);
		if(m_SStat_Table.cond_field[field].index==-1)
			for(int item=0;item<m_SStat_Table.NStat_Item_Count;item++)
  			{
  			if(strcmp(m_SStat_Table.SzStat_Item[item],m_SStat_Table.cond_field[field].szField_Value)==0)
  				{
  				//统计表的第item个字段对应条件表达式中的第j个变量
  				m_SStat_Table.item_cond_index[item]=field;
  				break;
  				}
  			}
		else 
			cond_variable_in_record_count++;//条件表达式中名为话单字段名的变量个数
  	    }
  	}
  
  for(int j=0;j<m_SStat_Table.cond_field_count;j++)
  	{
  	if(m_SStat_Table.cond_field[j].index!=-1)
  		{
  		sprintf(m_SStat_Table.cond_field_value[j].szField_Value,InRecord.Get_Field(m_SStat_Table.cond_field[j].index)); 
  		} 
  	}  
  if(m_SStat_Table.cond_field_count>0 && cond_variable_in_record_count==m_SStat_Table.cond_field_count)
	{
  	char Result[255] = "";
  	int ErrorNo=0;
  	char curr_condiction[250];
  	//sprintf(curr_condiction,m_SStat_Table.ChCondiction);
  	theCompile.Operation(Result, sizeof(Result)-1, &ErrorNo, m_SStat_Table.ChCondiction);
  	if(ErrorNo==0 && strcmp(Result,"false")==0)
  		return 1;
  	else if(ErrorNo!=0)
  		{
  		sprintf(TmpLogMsg,"error condiction expression.ErrorNo[%d]",ErrorNo);
  		throw CF_CError('E','H',ERR_EXPRESS,0,TmpLogMsg,__FILE__,__LINE__);
  		}
	}
  
  char crrFieldValue[100];
  crrFieldValue[0]=0;
  //开始统计
  for(int i=0;i<m_SStat_Table.NStat_Item_Count;i++)
  {
    //取字段值
    m_PsPre_Stat_Record[m_NRecord_Count].NItem_Type[i] 
	  = m_SStat_Table.NItem_Type[i];
    m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i][0]=0;
	
    crrFieldValue[0]=0;
    if(m_SStat_Table.NField_Index[i]!=-1 && m_SStat_Table.NField_Index[i]!=0)
		strcpy(crrFieldValue,InRecord.Get_Field(m_SStat_Table.NField_Index[i]));
	
    if(m_SStat_Table.NItem_Type[i]<KEYWORK_MIN)
    {
    	//看是否有设置默认值
    	//DeleteSpace(m_SStat_Table.Predfval_Or_Statid[i]);
    	int has_def=0;
      	if(strlen(m_SStat_Table.Predfval_Or_Statid[i])!=0)
	  		{
	  		char *container=strchr(m_SStat_Table.Predfval_Or_Statid[i],':');
	  		container++;
	  		if(m_SStat_Table.Predfval_Or_Statid[i][0]=='C')
	  			{
	  			has_def=1;
	  			strcpy(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i],container);
	  			}
	  		//取某个函数值，暂时没有需要
	  		else
	  			{
	  			}
	  		}
	else if(m_SStat_Table.NField_Index[i]<=0)
		{
		 sprintf(TmpLogMsg,"error FIELD_INDEX:%d, at Field:%s.",m_SStat_Table.NField_Index[i],m_SStat_Table.SzStat_Item[i]);
     	 	 throw CF_CError ('E','H',ERR_FIELD_NULL,0,TmpLogMsg,__FILE__,__LINE__);
		}
		//(1)若有设置默认值，但field_index不为-1
		//(2)没设默认值，field_index不为-1
		//这种配置下，应该取话单中的值		
      	if(m_SStat_Table.NField_Index[i]!=-1
      		&& (has_def==0||strlen(crrFieldValue)>0))
      		{
	  	    if(m_SStat_Table.NStat_Item_Begin[i]==(-1))
	  			{
				if(m_SStat_Table.NStat_Item_IndexInField[i]==(-1))
			 		 sprintf(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i],
		    			"%s",crrFieldValue);
				else
				  {
		  			sprintf(TmpLogMsg,"%s",crrFieldValue);
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
             	 			throw CF_CError('E','H',ERR_FIELD_INDEX,0,TmpLogMsg,__FILE__,__LINE__);
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
	    		char fieldValue[50];
	    		sprintf(fieldValue,crrFieldValue);
				int NField_Len = strlen(fieldValue);
				if((NField_Len<m_SStat_Table.NStat_Item_End[i])||
				  (m_SStat_Table.NStat_Item_End[i]<m_SStat_Table.NStat_Item_Begin[i]))
				{
		  			sprintf(TmpLogMsg,"Field %s Field_Len is not long enough.",m_SStat_Table.SzStat_Item[i]);
          			throw CF_CError('E','H',ERR_GET_SUBSTR,0,TmpLogMsg,__FILE__,__LINE__);
				}
				NField_Len=m_SStat_Table.NStat_Item_End[i]-m_SStat_Table.NStat_Item_Begin[i]+1;
				sprintf(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i],"%s",
		  			crrFieldValue+m_SStat_Table.NStat_Item_Begin[i]-1);
				m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i][NField_Len]=0;
			}
			if(m_SStat_Table.NItem_Type[i]==STAT_N_COUNT_TYPE 
				&& strcmp(m_SStat_Table.ChCount_Item_Unit[i],"/60")==0)
			{				
				sprintf(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i], "%ld",
					(atol(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i])+59)/60);
			}
			
      	}
		
	if(!strlen(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i]))
		{
	 	 sprintf(TmpLogMsg,"Field %s is NULL and has not set defaul value.",m_SStat_Table.SzStat_Item[i]);
     	 	throw CF_CError ('E','H',ERR_FIELD_NULL,0,TmpLogMsg,__FILE__,__LINE__);
		}
		//cout<<"getfield "<<m_SStat_Table.NField_Index[i]<<"|"<<m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i]<<"|"<<InRecord.Get_Field(m_SStat_Table.NField_Index[i])<<endl;
	} 
  //关键字>= 11
  //对通话时长做特别处理，分31个区间
  else if(m_SStat_Table.NItem_Type[i]==KEYWORK_CDRDURATION)
	  	{
	  	int cdr_time=(atol(crrFieldValue)+59)/60;
	  	if (cdr_time<=30)
	  		sprintf(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i],"%d",cdr_time);
	  	else
	  		sprintf(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i],"%s",">30");
	  	}
  else if(m_SStat_Table.NItem_Type[i]==KEYWORK_ERRORTYPE)
  	{
  	sprintf(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i],"%d",IError_Type);
  	}
//20080604
  /*
  //主叫局号，长途业务中使用
  //由计费号码除去区号和后面三位号码获得
  else if(m_SStat_Table.NItem_Type[i]==KEYWORK_THOUSANDCALLNO)
  	{
  	char chargeNumber[25];
  	chargeNumber[0]=0;
  	char chargeTollCode[10];
  	chargeTollCode[0]=0;
  	sprintf(chargeNumber,"%s",crrFieldValue);
  	//29为长途LDTSB格式清单计费号码所属本地网长途区号字段序号
  	sprintf(chargeTollCode,"%s",InRecord.Get_Field(29));
  	int length_tollcode=strlen(chargeTollCode);
  	if(length_tollcode==0)
  		{
  		 sprintf(TmpLogMsg,"chargeNumber is NULL.");
     	 throw CF_CError ('E','H',ERR_FIELD_NULL,0,TmpLogMsg,__FILE__,__LINE__);
  		}
  	int length_thousandcallno=strlen(chargeNumber)-length_tollcode-3;
  	if(length_thousandcallno<=0)
  		{
  		 sprintf(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i],"0");
  		}
  	else
  		{
		strncpy(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i],chargeNumber+length_tollcode,length_thousandcallno);
		m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i][length_thousandcallno]=0;
  		}
  	}*/
  else if(m_SStat_Table.NItem_Type[i]==KEYWORK_DAYCYCLE)
  	{
  	char cdrBeginTime[15];
  	sprintf(cdrBeginTime,crrFieldValue);
	
	int NField_Len = strlen(cdrBeginTime);
	if((NField_Len<m_SStat_Table.NStat_Item_End[i])||
	  (m_SStat_Table.NStat_Item_End[i]<m_SStat_Table.NStat_Item_Begin[i]))
	{
			sprintf(TmpLogMsg,"Field %s Field_Len is not long enough.",m_SStat_Table.SzStat_Item[i]);
			throw CF_CError('E','H',ERR_GET_SUBSTR,0,TmpLogMsg,__FILE__,__LINE__);
	}
	char keyValue[20];
	NField_Len=m_SStat_Table.NStat_Item_End[i]-m_SStat_Table.NStat_Item_Begin[i]+1;
	sprintf(keyValue,"%s",cdrBeginTime+m_SStat_Table.NStat_Item_Begin[i]-1);
	keyValue[NField_Len]=0;
  	
  	//是昨天且当前时间超过截止时间，或是昨天以前的
	//则需要纠正日帐期为今天
    if((strcmp(lastdate,keyValue)==0 && before_endtime_flag==0)
 		|| (strcmp(lastdate,keyValue)!=0 && strcmp(crrdate,keyValue)!=0))
 		{
 		strcpy(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i],crrdate);
 		}
    else
    	strcpy(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i],keyValue);
  	}
  //余下关键字
  else 
	{
	Set_KeyStatItem(m_PsPre_Stat_Record[m_NRecord_Count],i);			    
    }
  
  if(m_SStat_Table.item_cond_index[i]!=-1)
  	{
  	strcpy(m_SStat_Table.cond_field_value[m_SStat_Table.item_cond_index[i]].szField_Value,
  		m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i]); 
  	}
 
  }

//分析表达式
  if(m_SStat_Table.cond_field_count>0 && cond_variable_in_record_count != m_SStat_Table.cond_field_count)
	{
  	char Result[255] = "";
  	int ErrorNo=0;
  	//char curr_condiction[250];
  	//sprintf(curr_condiction,m_SStat_Table.ChCondiction);
  	theCompile.Operation(Result, sizeof(Result)-1, &ErrorNo, m_SStat_Table.ChCondiction);
  	if(ErrorNo==0 && strcmp(Result,"false")==0)
  		return 1;
  	else if(ErrorNo!=0)
  		{
  		sprintf(TmpLogMsg,"error condiction expression.ErrorNo[%d]",ErrorNo);
  		throw CF_CError('E','H',ERR_EXPRESS,0,TmpLogMsg,__FILE__,__LINE__);
  		}
	}
  
  if(m_SStat_Table.ChIs_Count=='Y')
  	 m_PsPre_Stat_Record[m_NRecord_Count].NBill_Count=1;
  m_PsPre_Stat_Record[m_NRecord_Count].NStat_Item_Count=m_SStat_Table.NStat_Item_Count;
  G_CF_CStat_UpdateCount++;
  if(G_CF_CStat_UpdateCount%500==0)
		expTrace(G_CF_CStat_DebugFlag, __FILE__, __LINE__, "total update count = %d",G_CF_CStat_UpdateCount);  
	 	
  return 0;	

}

int CF_CStat::Stat_Record_Update(CFmt_Change &InRecord , int IError_Type)
{
  char TmpLogMsg[400];
   STAT_RECORD  crrRecord;
   
  //按条件统计，用表达式实现
  //不满足条件则不统计
  if(first_time_read_record==1)
  	{
  	first_time_read_record=0;
  	for(int field=0;field<m_SStat_Table.cond_field_count;field++)
  		{
  		m_SStat_Table.cond_field[field].index=InRecord.Get_FieldIndex(m_SStat_Table.cond_field[field].szField_Value);
		if(m_SStat_Table.cond_field[field].index==-1)
			for(int item=0;item<m_SStat_Table.NStat_Item_Count;item++)
  			{
  			if(strcmp(m_SStat_Table.SzStat_Item[item],m_SStat_Table.cond_field[field].szField_Value)==0)
  				{
  				//统计表的第item个字段对应条件表达式中的第j个变量
  				m_SStat_Table.item_cond_index[item]=field;
  				break;
  				}
  			}
		else 
			cond_variable_in_record_count++;//条件表达式中名为话单字段名的变量个数
  	    }
  	}
  
  for(int j=0;j<m_SStat_Table.cond_field_count;j++)
  	{
  	if(m_SStat_Table.cond_field[j].index!=-1)
  		{
  		sprintf(m_SStat_Table.cond_field_value[j].szField_Value,InRecord.Get_Field(m_SStat_Table.cond_field[j].index)); 
  		} 
  	}  
  if(m_SStat_Table.cond_field_count>0 && cond_variable_in_record_count==m_SStat_Table.cond_field_count)
	{
  	char Result[255] = "";
  	int ErrorNo=0;
  	char curr_condiction[250];
  	//sprintf(curr_condiction,m_SStat_Table.ChCondiction);
  	theCompile.Operation(Result, sizeof(Result)-1, &ErrorNo, m_SStat_Table.ChCondiction);
  	if(ErrorNo==0 && strcmp(Result,"false")==0)
  		return 1;
  	else if(ErrorNo!=0)
  		{
  		sprintf(TmpLogMsg,"error condiction expression.ErrorNo[%d]",ErrorNo);
  		throw CF_CError('E','H',ERR_EXPRESS,0,TmpLogMsg,__FILE__,__LINE__);
  		}
	}
  
  char crrFieldValue[100];
  crrFieldValue[0]=0;
  //开始统计
  for(int i=0;i<m_SStat_Table.NStat_Item_Count;i++)
  {
    //取字段值
    crrRecord.NItem_Type[i] = m_SStat_Table.NItem_Type[i];
    crrRecord.SzStat_Item[i][0]=0;
	
    crrFieldValue[0]=0;
    if(m_SStat_Table.NField_Index[i]!=-1 && m_SStat_Table.NField_Index[i]!=0)
		strcpy(crrFieldValue,InRecord.Get_Field(m_SStat_Table.NField_Index[i]));
	
    if(m_SStat_Table.NItem_Type[i]<KEYWORK_MIN)
    {
    	//看是否有设置默认值
    	//DeleteSpace(m_SStat_Table.Predfval_Or_Statid[i]);
    	int has_def=0;
      	if(strlen(m_SStat_Table.Predfval_Or_Statid[i])!=0)
	  		{
	  		char *container=strchr(m_SStat_Table.Predfval_Or_Statid[i],':');
	  		container++;
	  		if(m_SStat_Table.Predfval_Or_Statid[i][0]=='C')
	  			{
	  			has_def=1;
	  			strcpy(crrRecord.SzStat_Item[i],container);
	  			}
	  		//取某个函数值，暂时没有需要
	  		else
	  			{
	  			}
	  		}
	else if(m_SStat_Table.NField_Index[i]<=0)
		{
		 sprintf(TmpLogMsg,"error FIELD_INDEX:%d, at Field:%s.",m_SStat_Table.NField_Index[i],m_SStat_Table.SzStat_Item[i]);
     	 	 throw CF_CError ('E','H',ERR_FIELD_NULL,0,TmpLogMsg,__FILE__,__LINE__);
		}
		//(1)若有设置默认值，但field_index不为-1
		//(2)没设默认值，field_index不为-1
		//这种配置下，应该取话单中的值
		
      	if(m_SStat_Table.NField_Index[i]!=-1
      		&& (has_def==0||strlen(crrFieldValue)>0))
      		{
	  	    if(m_SStat_Table.NStat_Item_Begin[i]==(-1))
	  			{
				if(m_SStat_Table.NStat_Item_IndexInField[i]==(-1))
			 		 sprintf(crrRecord.SzStat_Item[i],	"%s",crrFieldValue);
				else
				  {
		  			sprintf(TmpLogMsg,"%s",crrFieldValue);
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
		      				sprintf(crrRecord.SzStat_Item[i],"%s",TmpPoint1);
            			else
            			{
    			 			 sprintf(TmpLogMsg,"Field %d Index In Field is less than Define.",i);
             	 			throw CF_CError('E','H',ERR_FIELD_INDEX,0,TmpLogMsg,__FILE__,__LINE__);
           	 			}
          			}
          			else
          			 {
           				 *TmpPoint2=0;
           				 sprintf(crrRecord.SzStat_Item[i], "%s",TmpPoint1);
          	 		}
				  }
	  			}
			else 
			{
	    		//判断字段是否有规定取的那么长
	    		char fieldValue[50];
	    		sprintf(fieldValue,crrFieldValue);
				int NField_Len = strlen(fieldValue);
				if((NField_Len<m_SStat_Table.NStat_Item_End[i])||
				  (m_SStat_Table.NStat_Item_End[i]<m_SStat_Table.NStat_Item_Begin[i]))
				{
		  			sprintf(TmpLogMsg,"Field %s Field_Len is not long enough.",m_SStat_Table.SzStat_Item[i]);
          			throw CF_CError('E','H',ERR_GET_SUBSTR,0,TmpLogMsg,__FILE__,__LINE__);
				}
				NField_Len=m_SStat_Table.NStat_Item_End[i]-m_SStat_Table.NStat_Item_Begin[i]+1;
				sprintf(crrRecord.SzStat_Item[i],"%s",
		  			crrFieldValue+m_SStat_Table.NStat_Item_Begin[i]-1);
				crrRecord.SzStat_Item[i][NField_Len]=0;
			}
			if(m_SStat_Table.NItem_Type[i]==STAT_N_COUNT_TYPE 
				&& strcmp(m_SStat_Table.ChCount_Item_Unit[i],"/60")==0)
			{				
				sprintf(crrRecord.SzStat_Item[i], "%ld",
					(atol(crrRecord.SzStat_Item[i])+59)/60);
			}
			
      	}
		
		if(!strlen(crrRecord.SzStat_Item[i]) )
		{
	 	 sprintf(TmpLogMsg,"Field %s is NULL and has not set defaul value.",m_SStat_Table.SzStat_Item[i]);
     	 throw CF_CError ('E','H',ERR_FIELD_NULL,0,TmpLogMsg,__FILE__,__LINE__);
		}
		//cout<<"getfield "<<m_SStat_Table.NField_Index[i]<<"|"<<crrRecord.SzStat_Item[i]<<"|"<<InRecord.Get_Field(m_SStat_Table.NField_Index[i])<<endl;
	} 
  //关键字>= 11
  //对通话时长做特别处理，分31个区间
  else if(m_SStat_Table.NItem_Type[i]==KEYWORK_CDRDURATION)
	  	{
	  	int cdr_time=(atol(crrFieldValue)+59)/60;
	  	if (cdr_time<=30)
	  		sprintf(crrRecord.SzStat_Item[i],"%d",cdr_time);
	  	else
	  		sprintf(crrRecord.SzStat_Item[i],"%s",">30");
	  	}
  else if(m_SStat_Table.NItem_Type[i]==KEYWORK_ERRORTYPE)
  	{
  	sprintf(crrRecord.SzStat_Item[i],"%d",IError_Type);
  	}
//20080604,没用，需要了再增加
  /*
  //主叫局号，长途业务中使用
  //由计费号码除去区号和后面三位号码获得
  else if(m_SStat_Table.NItem_Type[i]==KEYWORK_THOUSANDCALLNO)
  	{
  	char chargeNumber[25];
  	chargeNumber[0]=0;
  	char chargeTollCode[10];
  	chargeTollCode[0]=0;
  	sprintf(chargeNumber,"%s",crrFieldValue);
  	//29为长途LDTSB格式清单计费号码所属本地网长途区号字段序号
  	sprintf(chargeTollCode,"%s",InRecord.Get_Field(29));
  	int length_tollcode=strlen(chargeTollCode);
  	if(length_tollcode==0)
  		{
  		 sprintf(TmpLogMsg,"chargeNumber is NULL.");
     	 throw CF_CError ('E','H',ERR_FIELD_NULL,0,TmpLogMsg,__FILE__,__LINE__);
  		}
  	int length_thousandcallno=strlen(chargeNumber)-length_tollcode-3;
  	if(length_thousandcallno<=0)
  		{
  		 sprintf(crrRecord.SzStat_Item[i],"0");
  		}
  	else
  		{
		strncpy(crrRecord.SzStat_Item[i],chargeNumber+length_tollcode,length_thousandcallno);
		crrRecord.SzStat_Item[i][length_thousandcallno]=0;
  		}
  	}*/
  else if(m_SStat_Table.NItem_Type[i]==KEYWORK_DAYCYCLE)
  	{
  	char cdrBeginTime[15];
  	sprintf(cdrBeginTime,crrFieldValue);
	
	int NField_Len = strlen(cdrBeginTime);
	if((NField_Len<m_SStat_Table.NStat_Item_End[i])||
	  (m_SStat_Table.NStat_Item_End[i]<m_SStat_Table.NStat_Item_Begin[i]))
	{
			sprintf(TmpLogMsg,"Field %s Field_Len is not long enough.",m_SStat_Table.SzStat_Item[i]);
			throw CF_CError('E','H',ERR_GET_SUBSTR,0,TmpLogMsg,__FILE__,__LINE__);
	}
	char keyValue[20];
	NField_Len=m_SStat_Table.NStat_Item_End[i]-m_SStat_Table.NStat_Item_Begin[i]+1;
	sprintf(keyValue,"%s",cdrBeginTime+m_SStat_Table.NStat_Item_Begin[i]-1);
	keyValue[NField_Len]=0;
  	
  	//是昨天且当前时间超过截止时间，或是昨天以前的
	//则需要纠正日帐期为今天
    if((strcmp(lastdate,keyValue)==0 && before_endtime_flag==0)
 		|| (strcmp(lastdate,keyValue)!=0 && strcmp(crrdate,keyValue)!=0))
 		{
 		strcpy(crrRecord.SzStat_Item[i],crrdate);
 		}
    else
    	strcpy(crrRecord.SzStat_Item[i],keyValue);
  	}
  //余下关键字
  else 
	{
	Set_KeyStatItem(crrRecord,i);			    
    }
  
  if(m_SStat_Table.item_cond_index[i]!=-1)
  	{
  	strcpy(m_SStat_Table.cond_field_value[m_SStat_Table.item_cond_index[i]].szField_Value,
  		crrRecord.SzStat_Item[i]); 
  	}
 
  }

//分析表达式
  if(m_SStat_Table.cond_field_count>0 && cond_variable_in_record_count != m_SStat_Table.cond_field_count)
	{
  	char Result[255] = "";
  	int ErrorNo=0;
  	//char curr_condiction[250];
  	//sprintf(curr_condiction,m_SStat_Table.ChCondiction);
  	theCompile.Operation(Result, sizeof(Result)-1, &ErrorNo, m_SStat_Table.ChCondiction);
  	if(ErrorNo==0 && strcmp(Result,"false")==0)
  		return 1;
  	else if(ErrorNo!=0)
  		{
  		sprintf(TmpLogMsg,"error condiction expression.ErrorNo[%d]",ErrorNo);
  		throw CF_CError('E','H',ERR_EXPRESS,0,TmpLogMsg,__FILE__,__LINE__);
  		}
	}
  
  if(m_SStat_Table.ChIs_Count=='Y')
  	 crrRecord.NBill_Count=1;
  crrRecord.NStat_Item_Count=m_SStat_Table.NStat_Item_Count;
  AddOneRecord(crrRecord);
  return 0;	

}

int CF_CStat::AddOneRecord(STAT_RECORD& addRcd)
{
	map<CRecordKey,STAT_RECORD*>::iterator it;
  
	CRecordKey crrKey;
	 for(int k=0;k<m_SStat_Table.NStat_Item_Count;k++)
		  {
		  if(m_SStat_Table.NItem_Type[k] != STAT_N_COUNT_TYPE 
		  	&& m_SStat_Table.NItem_Type[k] != STAT_D_COUNT_TYPE )//20070530,符点型累计项
		    	{
		    	strcpy(crrKey.SzStat_Item[crrKey.NStat_Item_Count],addRcd.SzStat_Item[k]);
			crrKey.NStat_Item_Count++;
		    	}
		  }
	 it=m_key_record.find(crrKey);
	 if(it!=m_key_record.end())
	 	{
	 	//累加
	 	for(int k=0;k<m_SStat_Table.NStat_Item_Count;k++)
			{
			//符点
			if(m_SStat_Table.NItem_Type[k] == STAT_D_COUNT_TYPE )
				{
				sprintf(it->second->SzStat_Item[k],"%lf",
			 		 atof(it->second->SzStat_Item[k])
			 		 +atof(addRcd.SzStat_Item[k]));				
				}
			//整形
			else if(m_SStat_Table.NItem_Type[k] == STAT_N_COUNT_TYPE )
				{
				sprintf(it->second->SzStat_Item[k],"%ld",
			 		 atol(it->second->SzStat_Item[k])
			 		 +atol(addRcd.SzStat_Item[k]));
				}
			}
		it->second->NBill_Count++;
		//cout<<"it->second->NBill_Count="<<it->second->NBill_Count<<endl;
	 	}
	 else
	 	{
	 	//新增，	mReport.insert(MSTATKD::value_type(key, value));
	 	//Stat_Record_toUpdate增加一条记录
	 	for(int j=0; j<m_SStat_Table.NStat_Item_Count; j++)
	    		{
	    		strcpy(Stat_Record_toUpdate[icount_toUpdate].SzStat_Item[j],addRcd.SzStat_Item[j]);
			Stat_Record_toUpdate[icount_toUpdate].NItem_Type[j]=addRcd.NItem_Type[j];			 
	    	 	}
    	       Stat_Record_toUpdate[icount_toUpdate].NBill_Count=addRcd.NBill_Count;
	       Stat_Record_toUpdate[icount_toUpdate].NStat_Item_Count=addRcd.NStat_Item_Count;	         
	 	m_key_record.insert(map<CRecordKey,STAT_RECORD*>::value_type(crrKey,&Stat_Record_toUpdate[icount_toUpdate]));
		m_key_seq.insert(map<CRecordKey,int>::value_type(crrKey,icount_toUpdate));
		icount_toUpdate++;	
		
		if(icount_toUpdate>=Current_Memorry_Unit)
			{
			//cout<<"icount_toUpdate="<<icount_toUpdate<<",Current_Memorry_Unit="<<Current_Memorry_Unit<<endl;
			Current_Memorry_Unit=Current_Memorry_Unit+MEMORY_APPLY_UNIT;
			STAT_RECORD *tmp=NULL;
			try
				{
				tmp=new STAT_RECORD[Current_Memorry_Unit];
				}
			catch(...)
				{
				throw CF_CError('E','H',ERR_REQU_MEM,0,(char *)"new err.",__FILE__,__LINE__);
				}
			memcpy(tmp, Stat_Record_toUpdate,icount_toUpdate*sizeof(STAT_RECORD));
			map<CRecordKey,STAT_RECORD*>::iterator it_change=m_key_record.begin();
			for(;it_change!=m_key_record.end();it_change++)
				{
				it_change->second=&tmp[m_key_seq[it_change->first]];
				}
			delete[] Stat_Record_toUpdate;
        		Stat_Record_toUpdate = tmp;
			}
		
		G_CF_CStat_UpdateCount++;
		if(G_CF_CStat_UpdateCount%500==0)
			 expTrace(G_CF_CStat_DebugFlag, __FILE__, __LINE__, "total update count = %d",G_CF_CStat_UpdateCount);  
	 	} 
     return 0;
}
/*
int CF_CStat::Update_Stat_File_Map()
{
//printf("Update_Stat_File\n");
 expTrace(G_CF_CStat_DebugFlag, __FILE__, __LINE__, "Update_Stat_File_Map..");  
  ///start,20080604
  map<CRecordKey,STAT_RECORD*>::iterator it;
  	
  int icount_Current_Stat_Record=0;
  //cout<<"m_NRecord_Count="<<m_NRecord_Count<<endl;
   for(int i=0;i<m_NRecord_Count;i++)
	{
	CRecordKey crrKey;
	 for(int k=0;k<m_SStat_Table.NStat_Item_Count;k++)
		  {
		  if(m_SStat_Table.NItem_Type[k] != STAT_N_COUNT_TYPE 
		  	&& m_SStat_Table.NItem_Type[k] != STAT_D_COUNT_TYPE )//20070530,符点型累计项
		    	{
		    	strcpy(crrKey.SzStat_Item[crrKey.NStat_Item_Count],m_PsPre_Stat_Record[i].SzStat_Item[k]);
			crrKey.NStat_Item_Count++;
		    	}
		  }
	 it=m_key_record.find(crrKey);
	 if(it!=m_key_record.end())
	 	{
	 	//累加
	 	for(int k=0;k<m_SStat_Table.NStat_Item_Count;k++)
			{
			//符点
			if(m_SStat_Table.NItem_Type[k] == STAT_D_COUNT_TYPE )
				{
				sprintf(it->second->SzStat_Item[k],"%lf",
			 		 atof(it->second->SzStat_Item[k])
			 		 +atof(m_PsPre_Stat_Record[i].SzStat_Item[k]));				
				}
			//整形
			else if(m_SStat_Table.NItem_Type[k] == STAT_N_COUNT_TYPE )
				{
				sprintf(it->second->SzStat_Item[k],"%ld",
			 		 atol(it->second->SzStat_Item[k])
			 		 +atol(m_PsPre_Stat_Record[i].SzStat_Item[k]));
				}
			}
		it->second->NBill_Count++;
		//cout<<"it->second->NBill_Count="<<it->second->NBill_Count<<endl;
	 	}
	 else
	 	{
	 	//新增，	mReport.insert(MSTATKD::value_type(key, value));
	 	m_key_record.insert(map<CRecordKey,STAT_RECORD*>::value_type(crrKey,&m_PsPre_Stat_Record[i]));
	 	}
   	}
  
   for(it=m_key_record.begin();it!=m_key_record.end();it++)
   	{
	for(int j=0; j<m_SStat_Table.NStat_Item_Count; j++)
    		{
    		strcpy(m_PsRes_Stat_Record[icount_Current_Stat_Record].SzStat_Item[j],it->second->SzStat_Item[j]);
		m_PsRes_Stat_Record[icount_Current_Stat_Record].NItem_Type[j]=it->second->NItem_Type[j];			 
    	 	}
    	  m_PsRes_Stat_Record[icount_Current_Stat_Record].NBill_Count=it->second->NBill_Count;
	  m_PsRes_Stat_Record[icount_Current_Stat_Record].NStat_Item_Count=it->second->NStat_Item_Count;
	  icount_Current_Stat_Record++;
   	}
   Rqsort( &m_PsRes_Stat_Record[0],icount_Current_Stat_Record,sizeof( struct STAT_RECORD ));  
     return 0;
   //print(m_PsRes_Stat_Record,icount_Current_Stat_Record);
   //把MAP里的记录写到m_PsRes_Stat_Record
   ///end,20080604

}

int CF_CStat::Update_Stat_File()
{
//printf("Update_Stat_File\n");
 expTrace(G_CF_CStat_DebugFlag, __FILE__, __LINE__, "Update_Stat_File..");  

 clock_t checka=clock();
 clock_t checkb=0;
 clock_t checkc=0;
 long cmptime=0;
 long updatetime=0;
 long addtime=0;
  if(m_NRecord_Count==0)
    {
    return 0;
    }
 
  Rqsort( &m_PsPre_Stat_Record[0],m_NRecord_Count,sizeof( struct STAT_RECORD ));  
  checkb=clock();
  printf("\nqsort:%d",checkb-checka);  
  expTrace(G_CF_CStat_DebugFlag, __FILE__, __LINE__, "Rqsort finised in stat file.");  
expTrace("Y", __FILE__, __LINE__, "Rqsort finised in stat file.");  
  int icount_Current_Stat_Record,i;
  STAT_RECORD Cpm_Stat_Record;
  for(i=0;i<m_SStat_Table.NStat_Item_Count;i++)
	  {
	  sprintf(Cpm_Stat_Record.SzStat_Item[i],"@@@");
	  }
  icount_Current_Stat_Record = 0; 
  
  for( i=0;i<m_NRecord_Count;i++ ) 
	  {
	  m_PsCur_Stat_Record = &m_PsPre_Stat_Record[i];
	  int Diff_flag=0;
//比较统计项
	checka=clock();
	  for(int k=0;k<m_SStat_Table.NStat_Item_Count;k++)
		  {
		  if(m_SStat_Table.NItem_Type[k] != STAT_N_COUNT_TYPE 
		  	&& m_SStat_Table.NItem_Type[k] != STAT_D_COUNT_TYPE )//20070530,符点型累计项
		    {
		    if(strcmp(Cpm_Stat_Record.SzStat_Item[k],m_PsCur_Stat_Record->SzStat_Item[k]))
			    {
			    Diff_flag=1;
			    break;
			    }
			}
		  }
	  checkb=clock();
	  cmptime+=checkb-checka;
	  
	  if(Diff_flag)
		  {
		   for(int k=0;k<m_SStat_Table.NStat_Item_Count;k++)
			  {
			  strcpy(m_PsRes_Stat_Record[icount_Current_Stat_Record].SzStat_Item[k],m_PsCur_Stat_Record->SzStat_Item[k]);
			//20080508
			  m_PsRes_Stat_Record[icount_Current_Stat_Record].NItem_Type[k]=m_PsCur_Stat_Record->NItem_Type[k];
			  }
		   m_PsRes_Stat_Record[icount_Current_Stat_Record].NStat_Item_Count=m_PsCur_Stat_Record->NStat_Item_Count;
		   m_PsRes_Stat_Record[icount_Current_Stat_Record].NBill_Count=1;
		   icount_Current_Stat_Record++;
 	
   		  for(int k=0;k<m_SStat_Table.NStat_Item_Count;k++)
			{
			strcpy(Cpm_Stat_Record.SzStat_Item[k],m_PsCur_Stat_Record->SzStat_Item[k]);
			}
			
		continue;
		}
	checkc=clock();
	addtime+=checkc-checkb;
//相同则累加
	for(int k=0;k<m_SStat_Table.NStat_Item_Count;k++)
			{
			if(m_SStat_Table.NItem_Type[k]==STAT_N_COUNT_TYPE)
				{
				sprintf(m_PsRes_Stat_Record[icount_Current_Stat_Record-1].SzStat_Item[k],"%ld",
			 		 atol(m_PsRes_Stat_Record[icount_Current_Stat_Record-1].SzStat_Item[k])+atol(m_PsCur_Stat_Record->SzStat_Item[k]));
				}
			if(m_SStat_Table.NItem_Type[k]==STAT_D_COUNT_TYPE)
				{
				sprintf(m_PsRes_Stat_Record[icount_Current_Stat_Record-1].SzStat_Item[k],"%lf",
			 		 atof(m_PsRes_Stat_Record[icount_Current_Stat_Record-1].SzStat_Item[k])+atof(m_PsCur_Stat_Record->SzStat_Item[k]));
				}
			}
	m_PsRes_Stat_Record[icount_Current_Stat_Record-1].NBill_Count+=1;
	checka=clock();
	updatetime+=checka-checkc;
	}
expTrace(G_CF_CStat_DebugFlag, __FILE__, __LINE__, "cmptime=%d,addtime=%d,updatetime=%d",cmptime,addtime,updatetime);  
  if(Connect(icount_Current_Stat_Record)==0)  
   //checkc=clock();
 //printf("\nwhole stattime:%ld",checkc-checka); 
  return 0;

}

//在内存中做累计操作，结果记录存放在Stat_Record_toUpdate[] 数组中
//20080508，函数算法，只扫描一次Stat_Record_toUpdate[]数组
int CF_CStat::Connect(int Add_Count)
{
   //20080508，改进算法	，先做一下排序
   expTrace(G_CF_CStat_DebugFlag, __FILE__, __LINE__, "%d to Rqsort,%d to add",icount_toUpdate,Add_Count);  
   Rqsort( Stat_Record_toUpdate,icount_toUpdate,sizeof( struct STAT_RECORD ));  
   //print(Stat_Record_toUpdate,icount_toUpdate);
    expTrace(G_CF_CStat_DebugFlag, __FILE__, __LINE__, "Rqsort finished");  
   
    if(icount_toUpdate+Add_Count>Current_Memorry_Unit)
    	{
    	while(icount_toUpdate+Add_Count>Current_Memorry_Unit)
    		{
    		Current_Memorry_Unit = Current_Memorry_Unit+MEMORY_APPLY_UNIT;
    		}
	expTrace(G_CF_CStat_DebugFlag, __FILE__, __LINE__, "to new STAT_RECORD:%d ",Current_Memorry_Unit); 
	
	STAT_RECORD *TEMPORARY=NULL;
	try
		{
    		TEMPORARY = new STAT_RECORD[Current_Memorry_Unit];
		}
	catch(...)
		{
		throw CF_CError('E','H',ERR_REQU_MEM,0,(char *)"new err.",__FILE__,__LINE__);
		}
2
	//20080508
	memcpy(TEMPORARY, Stat_Record_toUpdate,icount_toUpdate*sizeof(STAT_RECORD));
    	delete[] Stat_Record_toUpdate;
        Stat_Record_toUpdate = TEMPORARY;
    	}    
    
	int icountcpy = icount_toUpdate;
//20080508	
//j为Stat_Record_toUpdate数组的下标
    int j=0; 
    int strcmp_result=0;
    for(int i=0; i<Add_Count; i++)
    	{    	
	int equalflag=0;
	int lessflag=0;
	//每次从j开始扫描
    	for(; j<icountcpy; j++)
    		{
    		int difflag=0;
    		for(int k=0;k<m_SStat_Table.NStat_Item_Count;k++)
    			{
    			if(m_SStat_Table.NItem_Type[k]!=STAT_N_COUNT_TYPE
    				&& m_SStat_Table.NItem_Type[k]!=STAT_D_COUNT_TYPE)
    				{
    				//20080526,对整形字段应该用相减来判断，与排序比较函数一致
    				if(m_SStat_Table.NItem_Type[k]==STAT_N_TYPE)
					strcmp_result=atol(m_PsRes_Stat_Record[i].SzStat_Item[k])-atol(Stat_Record_toUpdate[j].SzStat_Item[k]);
				else
    					strcmp_result=strcmp(m_PsRes_Stat_Record[i].SzStat_Item[k],Stat_Record_toUpdate[j].SzStat_Item[k]);
    				if(strcmp_result!=0)
    					{
    					if(strcmp_result<0)
						{
						lessflag=1;
    						}
    					difflag=1;
    					break;
    					}
    				}
    			}    		  		
    		//相同，相应项累加
    		if(difflag==0)
    			{
    			equalflag=1;
    			for(int h=0; h<m_SStat_Table.NStat_Item_Count; h++)
    				{
    				if(m_SStat_Table.NItem_Type[h]==STAT_N_COUNT_TYPE)
    				{
    					sprintf(Stat_Record_toUpdate[j].SzStat_Item[h],"%ld",
    						atol(Stat_Record_toUpdate[j].SzStat_Item[h])+atol(m_PsRes_Stat_Record[i].SzStat_Item[h]));
    				}
    				if(m_SStat_Table.NItem_Type[h]==STAT_D_COUNT_TYPE)
						{
						sprintf(Stat_Record_toUpdate[j].SzStat_Item[h],"%lf",
    						atof(Stat_Record_toUpdate[j].SzStat_Item[h])
    						+atof(m_PsRes_Stat_Record[i].SzStat_Item[h]));
    					}
    				}
    			Stat_Record_toUpdate[j].NBill_Count+=m_PsRes_Stat_Record[i].NBill_Count;
    			break;
    			}
		//20080508
		//Stat_Record_toUpdate[]已排序
		//在Stat_Record_toUpdate[]中碰到比当前记录大的记录
		//说明Stat_Record_toUpdate[]中不会有相同条件记录
		//不用再比较下去
		if(lessflag==1)
			break;
    		}
	//不同，往Stat_Record_toUpdate 新添一记录
    	if(equalflag==0)
    		{
    		for(int h=0; h<m_SStat_Table.NStat_Item_Count; h++)
    			{
    			strcpy(Stat_Record_toUpdate[icount_toUpdate].SzStat_Item[h],m_PsRes_Stat_Record[i].SzStat_Item[h]);
			//20080508
			Stat_Record_toUpdate[icount_toUpdate].NItem_Type[h]=m_PsRes_Stat_Record[i].NItem_Type[h];
			Stat_Record_toUpdate[icount_toUpdate].NStat_Item_Count=m_PsRes_Stat_Record[i].NStat_Item_Count;
    			}
    		Stat_Record_toUpdate[icount_toUpdate].NBill_Count=m_PsRes_Stat_Record[i].NBill_Count;
    		icount_toUpdate++;    	
		G_CF_CStat_UpdateCount++;
    		}
    	}
    expTrace(G_CF_CStat_DebugFlag, __FILE__, __LINE__, 
		"totoal table UpdateCount=%d,current table UpdateCount=%d",
		G_CF_CStat_UpdateCount,icount_toUpdate);  
   // print(Stat_Record_toUpdate,icount_toUpdate);
    return 0;    
}

*/
int CF_CStat::Set_KeyWord(STAT_RECORD &Cur,int i,char *Key_Container)
{
  if(m_SStat_Table.NStat_Item_Begin[i]==(-1)) 
  {
    sprintf(Cur.SzStat_Item[i],"%s",Key_Container);
  }

  else 
  {
    //判断字段是否有规定取的那么长
    int NField_Len = strlen(Key_Container);
	if((NField_Len<m_SStat_Table.NStat_Item_End[i])||
      (m_SStat_Table.NStat_Item_End[i]<m_SStat_Table.NStat_Item_Begin[i]))
	{
       throw CF_CError('E','H',ERR_GET_SUBSTR,0,"Field_Len is not long enough!",__FILE__,__LINE__);
    }
    NField_Len=m_SStat_Table.NStat_Item_End[i]-m_SStat_Table.NStat_Item_Begin[i]+1;
    sprintf(Cur.SzStat_Item[i],"%s",
      Key_Container+m_SStat_Table.NStat_Item_Begin[i]-1);
    Cur.SzStat_Item[i][NField_Len]=0;
  }
  return 0;			
}


int CF_CStat::Set_KeyStatItem(STAT_RECORD &Cur,int i)
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
  case 14:
  	char UpdateTime[DATETIME_LEN+1];	    	
    getCurTime(UpdateTime);   
    Set_KeyWord(Cur,i,UpdateTime);
    break;
   case 17://KEYWORK_PROCESSID
            Set_KeyWord(Cur,i,m_SzStat_process_id);
	     break;
  default:
  	throw CF_CError('E','H',ERR_KEY_ITEM,0,"Can not find Such Key Stat Item.",__FILE__,__LINE__);
  }
  return 0;
}

int CF_CStat::Make_Select_Sql(STAT_RECORD& UPdate_Stat_Record,char *sqltmp)
{
  //printf("Make_Select_Sql\n");
  sprintf(sqltmp,"select ");
  for(int i = m_SStat_Table.table_name_key_count;i<m_SStat_Table.NStat_Item_Count;i++)
  {
    if(m_SStat_Table.NItem_Type[i]==STAT_N_COUNT_TYPE 
		|| m_SStat_Table.NItem_Type[i]==STAT_D_COUNT_TYPE)
    {
      strcat(sqltmp,m_SStat_Table.SzStat_Item[i]);
      strcat(sqltmp,",");
     }
  }
  
  if(m_SStat_Table.ChIs_Count=='Y')
  {
    sprintf(sqltmp,"%s%s",sqltmp,m_SStat_Table.SzCount_Item_Name);    	
    sqltmp[strlen(sqltmp)] = 0;
  }
  else
  {
    sqltmp[strlen(sqltmp)-1] = 0;
  }	 
  sprintf(sqltmp,"%s from %s where ",sqltmp,crr_table_name);

  for(int i=m_SStat_Table.table_name_key_count;i<m_SStat_Table.NStat_Item_Count;i++)
  {
    if(m_SStat_Table.NItem_Type[i]==STAT_N_TYPE)
    {
      strcat(sqltmp,m_SStat_Table.SzStat_Item[i]);
      strcat(sqltmp,"=");
      strcat(sqltmp,UPdate_Stat_Record.SzStat_Item[i]);
      strcat(sqltmp," and ");
    }
    else if((m_SStat_Table.NItem_Type[i]==STAT_SZ_TYPE)||(m_SStat_Table.NItem_Type[i]>=KEYWORK_MIN))
    {
      strcat(sqltmp,m_SStat_Table.SzStat_Item[i]);
      strcat(sqltmp,"='");
      strcat(sqltmp,UPdate_Stat_Record.SzStat_Item[i]);
      strcat(sqltmp,"' and ");
    }
    
  }
  sqltmp[strlen(sqltmp)-5] = 0;	 
  return 0;
}

int CF_CStat::Make_Delete_Sql(STAT_RECORD& UPdate_Stat_Record,char *sqltmp)
{
	sprintf(sqltmp,"delete from %s where ",crr_table_name);
	for(int i=m_SStat_Table.table_name_key_count;i<m_SStat_Table.NStat_Item_Count;i++)
	  {
	  if(m_SStat_Table.NItem_Type[i]==STAT_N_TYPE)
	    {
	    strcat(sqltmp,m_SStat_Table.SzStat_Item[i]);
	    strcat(sqltmp,"=");
	    strcat(sqltmp,UPdate_Stat_Record.SzStat_Item[i]);
	    strcat(sqltmp," and ");
	    }
	  else if((m_SStat_Table.NItem_Type[i]==STAT_SZ_TYPE)||(m_SStat_Table.NItem_Type[i]>=KEYWORK_MIN))
	    {
	    strcat(sqltmp,m_SStat_Table.SzStat_Item[i]);
	    strcat(sqltmp,"='");
	    strcat(sqltmp,UPdate_Stat_Record.SzStat_Item[i]);
	    strcat(sqltmp,"' and ");
	    }	  
	  }
    sqltmp[strlen(sqltmp)-5] = 0;	 
	
	return 0;
}


int CF_CStat::Make_Insert_Sql(char *sqltmp,const char *table_name)
{
	sprintf(sqltmp,"insert into %s( ",table_name);
	for(int i=m_SStat_Table.table_name_key_count;i<m_SStat_Table.NStat_Item_Count;i++)
	  {
	    strcat(sqltmp,m_SStat_Table.SzStat_Item[i]);
	    strcat(sqltmp,",");
	  }
 	 if(m_SStat_Table.ChIs_Count=='Y')
	  {
    	strcat(sqltmp,m_SStat_Table.SzCount_Item_Name);
    	sqltmp[strlen(sqltmp)]     = 0;
	  }
 	 else
	  {
      	sqltmp[strlen(sqltmp)-1] = 0;
	  }	 
	sprintf(sqltmp,"%s) values ( ",sqltmp);
	
	char c_item[50];
	int colum=m_SStat_Table.NStat_Item_Count;
	if(m_SStat_Table.ChIs_Count=='Y')  
  		colum++;
	for(int item=m_SStat_Table.table_name_key_count;item<colum;item++)
  	{  	 
  	 	if(m_SStat_Table.NItem_Type[item]==KEYWORK_UPDATETIME)
    		sprintf(c_item,"to_date(:%d,'YYYYMMDDHH24MISS'),",item);
  	 	else
  	 	  sprintf(c_item,":%d,",item);    	
  		strcat(sqltmp,c_item);
  	}  	  	  	
	sqltmp[strlen(sqltmp)-1]=')';  
	
	return 0;
}

int CF_CStat::Make_Update_Sql(char *sqltmp,const char *table_name)
{
	//printf("Make_Update_Sql\n");

    char c_item[20];
	sprintf(sqltmp,"update %s set ",table_name);
	int i,flag=0;
	for(i = 0;i<m_SStat_Table.NStat_Item_Count;i++)
	  {
	  if(m_SStat_Table.NItem_Type[i]==STAT_N_COUNT_TYPE
	  	|| m_SStat_Table.NItem_Type[i]==STAT_D_COUNT_TYPE)
	    {
	    strcat(sqltmp,m_SStat_Table.SzStat_Item[i]);
	    strcat(sqltmp,"=");
	    sprintf(c_item,":%d,",flag);
 	    strcat(sqltmp,c_item);	   
 	    flag++;
	    }
	  }
  	if(m_SStat_Table.ChIs_Count=='Y')
	  {
	    strcat(sqltmp,m_SStat_Table.SzCount_Item_Name);
	    strcat(sqltmp,"=");
	    sprintf(c_item,":%d,",flag);
 	    strcat(sqltmp,c_item);
 	    flag++;
	  }
  	sqltmp[strlen(sqltmp)-1] = 0;	  	
	sprintf(sqltmp,"%s where ",sqltmp);
	int colum=m_SStat_Table.NStat_Item_Count;
	if(m_SStat_Table.ChIs_Count=='Y')  
  		colum++;	
	for(int item=m_SStat_Table.table_name_key_count;item<colum;item++)
  		{ 
  		if(m_SStat_Table.NItem_Type[item]!=STAT_N_COUNT_TYPE
			&& m_SStat_Table.NItem_Type[item]!=STAT_D_COUNT_TYPE)
  			{
  			strcat(sqltmp,m_SStat_Table.SzStat_Item[item]);
  			strcat(sqltmp,"=");
  	 		if(m_SStat_Table.NItem_Type[item]==KEYWORK_UPDATETIME)
    			sprintf(c_item,"to_date(:%d,'YYYYMMDDHH24MISS') and ,",flag);
  	 		else
  	 		  sprintf(c_item,":%d and ",flag);    	
  			strcat(sqltmp,c_item);
  			flag++;
  			}
  		}  	  	  		 
  	sqltmp[strlen(sqltmp)-5] = 0;	 	
	return 0;
}

void CF_CStat::Create_Table(char *table_name)
{
	CBindSQL ds(DBConn);
	char sqlCreateTable[SQL_LENGTH];
	int i=0;
	char sqlAlter[1000];
	char tmpFieldDef[50];
	
	sprintf(sqlCreateTable,"CREATE TABLE %s (",table_name);
	for( i=m_SStat_Table.table_name_key_count;i<m_SStat_Table.NStat_Item_Count;i++)
	  {
	    strcat(sqlCreateTable,m_SStat_Table.SzStat_Item[i]);
	    strcat(sqlCreateTable," ");
	    //为不同字段指定不同类型	  
	    strcat(sqlCreateTable,m_SStat_Table.NField_dataType[i]);
	    strcat(sqlCreateTable,",");	    
	  }
	//如果要统计通话次数
 	if(m_SStat_Table.ChIs_Count=='Y')
	  {
    	   strcat(sqlCreateTable,m_SStat_Table.SzCount_Item_Name);
	   strcat(sqlCreateTable," NUMBER,");	    
	  }
	sqlCreateTable[strlen(sqlCreateTable)-1]=')';
	
	char sqlCreateIndex[SQL_LENGTH];
	int index_field_count=0;
	sprintf(sqlCreateIndex,"CREATE UNIQUE INDEX INDEX_%s ON %s (",table_name,table_name);
	for( i=m_SStat_Table.table_name_key_count;i<m_SStat_Table.NStat_Item_Count;i++)
	  {
	    if(m_SStat_Table.IsIndex[i]=='N')
			continue;
	    strcat(sqlCreateIndex,m_SStat_Table.SzStat_Item[i]);
	    strcat(sqlCreateIndex,",");
	    index_field_count++;
	  }
	sqlCreateIndex[strlen(sqlCreateIndex)-1]=')';
	
	 char sqlPrimaryKey[SQL_LENGTH];
	 sprintf(sqlPrimaryKey,"ALTER TABLE %s ADD CONSTRAINT PK_%s PRIMARY KEY (",	table_name,table_name);
	 for(i=m_SStat_Table.table_name_key_count;i<m_SStat_Table.NStat_Item_Count;i++)
	  {
	  	if(m_SStat_Table.NItem_Type[i]!=STAT_N_COUNT_TYPE && m_SStat_Table.NItem_Type[i]!=STAT_D_COUNT_TYPE)
	  		{
	  		strcat(sqlPrimaryKey,m_SStat_Table.SzStat_Item[i]);
	    		strcat(sqlPrimaryKey,",");
	  		}	   
	 }
	 sqlPrimaryKey[strlen(sqlPrimaryKey)-1]=')';	 

	try
	{
		/*
		sprintf(sqlCreateTable,"CREATE TABLE %s AS SELECT * FROM %s WHERE 1=2",
		    table_name,m_SStat_Table.SzTable_Name);
		ds.Open(sqlCreateTable, SQL_DDL);
		ds.Execute();
		ds.Close();
		*/
		//cout<<"create sql-----"<<endl<<sqlCreateTable<<endl;
		ds.Open(sqlCreateTable, SQL_DDL);
		ds.Execute();
		ds.Close();
		//cout<<"sqlCreateIndex:-----"<<endl<<sqlCreateIndex<<endl;
		if(index_field_count!=0 && m_SStat_Table.CHStat_Mode=='U')
			{
			ds.Open(sqlCreateIndex, SQL_DDL);
			ds.Execute();
			ds.Close();
			}
		//cout<<"sqlPrimaryKey:-----"<<endl<<sqlPrimaryKey<<endl;
		if(m_SStat_Table.CHStat_Mode=='U')
			{
			ds.Open(sqlPrimaryKey, SQL_DDL);
			ds.Execute();
			ds.Close();
			}
	}
	catch (CDBException ex)
	{
		cout<<"Error number: "<<  ex.GetErrorCode() << endl;
		cout<<"Error Message: "<<ex.GetErrorMsg() << endl;    
		throw(ex);
	}
/*
CREATE TABLE STAT_TEST_CREATE
(
  CDRDAY    VARCHAR2(20 BYTE),
  DAYCYCLE  NUMBER,
  SOURCEID  VARCHAR2(10 BYTE),
  FEE       NUMBER,
  FILENAME  VARCHAR2(250 BYTE)
);

CREATE UNIQUE INDEX PK_STAT_TEST2 ON STAT_TEST11
(LOCAL_NET, CDRDAY, DAYCYCLE, SOURCEID, FILENAME);

ALTER TABLE STAT_TABLE_DEFINE ADD (  CONSTRAINT PK_STAT_TABLE_DEFINE PRIMARY KEY (CONFIG_ID) );
*/
}

/******************************************************************************************
 功能: 
 将输入记录与结果表中记录做对比，根据对比结果做以下一种操作:
 1、向map_table_recordToInsert或map_table_recordToUpdate中插记录
 2、删除记录
 3、报错
********************************************************************************************/
int CF_CStat::Prepare_Rec()
{
  //cout<<"int prepare ================="<<endl;
	STAT_RECORD crrRcord;
  //先算出涉及多少个表，每个表有多少条记录，不存在的表先建好
   int cnt=0;
  map<string,int> map_table_recordCount;//每张表涉及到的记录数
  map_table_recordCount.clear();

  map<string,int>::iterator it_count;
  char tmp_table_name[STAT_ITEM_CONTAINER_LEN] ;
  string str_tmp_table_name;
  map<int,string> map_rcdSeq_tableName;
  map_rcdSeq_tableName.clear();
  //静态表
  if(m_SStat_Table.table_name_key_count==0)
  	{
  	map_table_recordCount[m_SStat_Table.SzTable_Name]=icount_toUpdate;
  	}
  //动态建表
  else
  	{
	   for(cnt=0;cnt<icount_toUpdate;cnt++)
	  	{
	  	STAT_RECORD &UPdate_Stat_Record=Stat_Record_toUpdate[cnt];

		//获得表名
		//"STAT_TABLE_DEFINE.table_name"+"_"+"关键字1" +"_"+"关键字2"+…+"关键字n"
		 
		strcpy(tmp_table_name,m_SStat_Table.SzTable_Name);
		for(int key_cnt=0;key_cnt<m_SStat_Table.table_name_key_count;key_cnt++)
			{
			strcat(tmp_table_name,"_");
			strcat(tmp_table_name,UPdate_Stat_Record.SzStat_Item[key_cnt]);
			}
		str_tmp_table_name=tmp_table_name;
		map_rcdSeq_tableName[cnt]=str_tmp_table_name;
	
		
		it_count=map_table_recordCount.find(str_tmp_table_name);
		if(it_count==map_table_recordCount.end())
			{
			map_table_recordCount[str_tmp_table_name]=1;
			//cout<<"map_table_recordCount[str_tmp_table_name]=1;"<<endl;
			}
		else
			{
			map_table_recordCount[str_tmp_table_name]++;
			//cout<<"map_table_recordCount[str_tmp_table_name]="<<map_table_recordCount[str_tmp_table_name]<<endl;
			}
		
		//检查该表是否存在
		int file_cnt;
		for(file_cnt=curr_table_count-1;file_cnt>=0;file_cnt--)
			{
			if(strcmp(m_SStat_Table.v_tableName[file_cnt].c_str(),tmp_table_name)==0)
				break;
			}
		//没找到，需建表
		if(file_cnt==-1)
			{
			//cout<<"CREATE TABLE:-------"<<str_tmp_table_name<<endl;
			Create_Table(tmp_table_name);
			m_SStat_Table.v_tableName.push_back(tmp_table_name);			
			curr_table_count++;
			}
		
	   	}
	}
  CBindSQL ds(DBConn);
  char sqltmp[SQL_LENGTH];
  
  strcpy(crr_table_name,m_SStat_Table.SzTable_Name);
  
  map<string,v_pRcd>::iterator it;
  for(cnt=0;cnt<icount_toUpdate;cnt++)
  	{
  	STAT_RECORD &UPdate_Stat_Record=Stat_Record_toUpdate[cnt];

	if(m_SStat_Table.table_name_key_count!=0)
	 	strcpy(crr_table_name,map_rcdSeq_tableName[cnt].c_str());
	
	  //cout<<"crr_table_name="<<crr_table_name<<endl;
	  Make_Select_Sql(UPdate_Stat_Record,sqltmp);
	  //printf("%s\n",sqltmp);
	  ds.Open(sqltmp);
	  int i;
	  for( i=0;i<m_SStat_Table.NStat_Item_Count;i++)
		  {
		  if(m_SStat_Table.NItem_Type[i]==STAT_N_COUNT_TYPE
		  	||m_SStat_Table.NItem_Type[i]==STAT_D_COUNT_TYPE)
		    {
		    ds>>crrRcord.SzStat_Item[i];
		    if(ds.IsEnd())
			    {			    
			        //找不到且为'-' 
			     	  if(m_SStat_Table.ChStat_Type=='-')
					      {
			  		       throw CF_CError('D','H',ERR_SELECT,0,sqltmp,__FILE__,__LINE__);
					      }
			     	//找不到，为"+" ，并且不统计话单张数, 在此 向Record_toInsert[]添加一记录
			     	  else if(m_SStat_Table.ChIs_Count=='N') 
					      {	
					 
						it= map_table_recordToInsert.find(crr_table_name);
						if(it==map_table_recordToInsert.end())
							{
							
							v_pRcd v_pToRecord(map_table_recordCount[crr_table_name],NULL);
							map_table_recordToInsert[crr_table_name]=v_pToRecord;
							map_table_recordToInsertCount[crr_table_name]=0;
							it= map_table_recordToInsert.find(crr_table_name);
							}
						int &currRrdCount=map_table_recordToInsertCount[crr_table_name];

			    	  		(it->second)[currRrdCount]=&UPdate_Stat_Record;
					       currRrdCount++;						   
						
					       break;
					      }
			    }
		    }		 
		  }
	    //没有找到相同条件记录，已经插入map_table_recordToInsert，处理下一条记录
	    if(i!=m_SStat_Table.NStat_Item_Count)
	  	      continue;

	    //走到这里，找到相同条件记录或表只统计通话次数	
	    //处理方法与上相似

	    if(m_SStat_Table.ChIs_Count=='Y') 
		  {
		  ds>>crrRcord.NBill_Count;
		  if(ds.IsEnd())
			  {			  
			  //找不到且为-　
		     	  if(m_SStat_Table.ChStat_Type=='-')
				{
				 throw CF_CError('D','H',ERR_SELECT,0,sqltmp,__FILE__,__LINE__);
				}
		     	  //向Record_toInsert[] 添一记录
		     	  else
		     	  	{
				it= map_table_recordToInsert.find(crr_table_name);
				if(it==map_table_recordToInsert.end())
					{
			
					v_pRcd v_pToRecord(map_table_recordCount[crr_table_name],NULL);
					map_table_recordToInsert[crr_table_name]=v_pToRecord;
					map_table_recordToInsertCount[crr_table_name]=0;
					it= map_table_recordToInsert.find(crr_table_name);
					}
				int &currRrdCount=map_table_recordToInsertCount[crr_table_name];

				(it->second)[currRrdCount]=&UPdate_Stat_Record;
				
				currRrdCount++;
				//本条记录处理完毕	   
				continue;  		
				}  
		         }
		}
	 ds.Close();

	  //走到这里，说明找到相应记录，
	  //如果需统计话单数，计算话单数，为0则删除，小于0报错
	  	    //cout<<"org:"<<endl;
		    //printOneRecord(UPdate_Stat_Record);
		    //cout<<"table:"<<endl;
		    //printOneRecord(crrRcord);
	  if(m_SStat_Table.ChIs_Count=='Y') 
		  {
		  int NTmp_Bill_Count;
		  if(m_SStat_Table.ChStat_Type=='-')
		  	{
		  	NTmp_Bill_Count = crrRcord.NBill_Count-UPdate_Stat_Record.NBill_Count;
		    	}
		  else
		      {
		  	  NTmp_Bill_Count = crrRcord.NBill_Count+UPdate_Stat_Record.NBill_Count;
		      }
		  UPdate_Stat_Record.NBill_Count = NTmp_Bill_Count;
		  if(NTmp_Bill_Count<0)
		    	throw CF_CError('E','H',BILL_COUNT_IS_NEGATIVE,0,"BILL COUNT IS NEGATIVE",__FILE__,__LINE__);
		  if(NTmp_Bill_Count==0) 
			  {
			  //做删除动作,然后返回
			  memset(sqltmp,0,SQL_LENGTH);
			  Make_Delete_Sql(UPdate_Stat_Record,sqltmp);
			  ds.Open(sqltmp,NONSELECT_DML);
			  ds.Execute();
			  if(ds.IsError())
				  {
				  ds.Close();
				  return (-1);
				  }
			  ds.Close();
			  //本条记录处理完毕	   
			  continue;  	
			  }
		  }
		  
	//计算累计项
	  if(m_SStat_Table.ChStat_Type=='-')
	    {
		  for(int i = m_SStat_Table.table_name_key_count;i<m_SStat_Table.NStat_Item_Count;i++)
		    {
		    if(m_SStat_Table.NItem_Type[i]==STAT_N_COUNT_TYPE)
		      {
		      sprintf(UPdate_Stat_Record.SzStat_Item[i],"%ld",
		        atol(crrRcord.SzStat_Item[i])
		        -atol(UPdate_Stat_Record.SzStat_Item[i]));
		      }
			else if(m_SStat_Table.NItem_Type[i]==STAT_D_COUNT_TYPE)
		      {
		      sprintf(UPdate_Stat_Record.SzStat_Item[i],"%lf",
		        atof(crrRcord.SzStat_Item[i])
		        -atof(UPdate_Stat_Record.SzStat_Item[i]));
		      }
		    }
		  }
	  else
		    {
		    for(int i = m_SStat_Table.table_name_key_count;i<m_SStat_Table.NStat_Item_Count;i++)
		      {
		      if(m_SStat_Table.NItem_Type[i]==STAT_N_COUNT_TYPE)
		        {
		        sprintf(UPdate_Stat_Record.SzStat_Item[i],"%ld",
		          atol(crrRcord.SzStat_Item[i])
		          +atol(UPdate_Stat_Record.SzStat_Item[i]));
		        }
			  else if(m_SStat_Table.NItem_Type[i]==STAT_D_COUNT_TYPE)
		        {
		        sprintf(UPdate_Stat_Record.SzStat_Item[i],"%lf",
		          atof(crrRcord.SzStat_Item[i])
		          +atof(UPdate_Stat_Record.SzStat_Item[i]));
		        }
		      }
		    }  
	//往Record_toUpdate[]数组添加一记录
	it= map_table_recordToUpdate.find(crr_table_name);
	if(it==map_table_recordToUpdate.end())
		{
		v_pRcd v_pToRecord(map_table_recordCount[crr_table_name],NULL);
		map_table_recordToUpdate[crr_table_name]=v_pToRecord;
		map_table_recordToUpdateCount[crr_table_name]=0;
		it= map_table_recordToUpdate.find(crr_table_name);
		}
	int &currRrdCount=map_table_recordToUpdateCount[crr_table_name];

	(it->second)[currRrdCount]=&UPdate_Stat_Record;

	//cout<<"in vector:"<<endl;
	//printOneRecord((it->second)[currRrdCount]);
	currRrdCount++;				

  	}
  map_rcdSeq_tableName.clear();
  map_table_recordCount.clear();
  return 0;		
}

//insert 模式统计中往表中插记录
int CF_CStat::Insert_Stat()
{
	//clock_t checka=clock();	
	//clock_t checkb=0;
	
	//先算出涉及多少个表，每个表有多少条记录，不存在的表先建好
	int cnt=0;
	
	char tmp_table_name[STAT_ITEM_CONTAINER_LEN] ;
	string str_tmp_table_name;

	map<string,v_recoredSeq> map_tableName_rcdSeq;
	map_tableName_rcdSeq.clear();
	map<string,v_recoredSeq> ::iterator it;
	//cout<<"m_NRecord_Count="<<m_NRecord_Count<<endl;
	//静态表
	if(m_SStat_Table.table_name_key_count==0)
		{
		v_recoredSeq crr_v_recoredSeq;
		crr_v_recoredSeq.clear();
		for(cnt=0;cnt<m_NRecord_Count;cnt++)
		  	{
		  	crr_v_recoredSeq.push_back(cnt);
			}
		map_tableName_rcdSeq[m_SStat_Table.SzTable_Name]=crr_v_recoredSeq;
		
		//map_table_recordCount[m_SStat_Table.SzTable_Name]=m_NRecord_Count;
		}
	//动态建表
	else
		{
		   for(cnt=0;cnt<m_NRecord_Count;cnt++)
		  	{
			//获得表名
			//"STAT_TABLE_DEFINE.table_name"+"_"+"关键字1" +"_"+"关键字2"+…+"关键字n"		 
			strcpy(tmp_table_name,m_SStat_Table.SzTable_Name);
			for(int key_cnt=0;key_cnt<m_SStat_Table.table_name_key_count;key_cnt++)
				{
				strcat(tmp_table_name,"_");
				strcat(tmp_table_name,m_PsPre_Stat_Record[cnt].SzStat_Item[key_cnt]);
				}
			str_tmp_table_name=tmp_table_name;
			//map_rcdSeq_tableName[cnt]=str_tmp_table_name;
			
			it=map_tableName_rcdSeq.find(str_tmp_table_name);
			if(it==map_tableName_rcdSeq.end())
				{
				v_recoredSeq crr_v_recoredSeq;
				crr_v_recoredSeq.clear();
				crr_v_recoredSeq.push_back(cnt);
				map_tableName_rcdSeq[str_tmp_table_name]=crr_v_recoredSeq;
				
				//map_tableName_rcdSeq[str_tmp_table_name]=1;
				//cout<<"map_table_recordCount[str_tmp_table_name]=1;"<<endl;
				}
			else
				{
				map_tableName_rcdSeq[str_tmp_table_name].push_back(cnt);
				
				//map_table_recordCount[str_tmp_table_name]++;				
				//cout<<"map_table_recordCount[str_tmp_table_name]="<<map_table_recordCount[str_tmp_table_name]<<endl;
				}
			
			//检查该表是否存在
			int file_cnt;
			for(file_cnt=curr_table_count-1;file_cnt>=0;file_cnt--)
				{
				if(strcmp(m_SStat_Table.v_tableName[file_cnt].c_str(),tmp_table_name)==0)
					break;
				}
			//没找到，需建表
			if(file_cnt==-1)
				{
				//cout<<"CREATE TABLE:-------"<<str_tmp_table_name<<endl;
				Create_Table(tmp_table_name);
				m_SStat_Table.v_tableName.push_back(tmp_table_name);			
				curr_table_count++;
				}
		
	   		}
	}

	CBindSQL ds( DBConn );
	int i,j,seq_IN_m_PsPre_Stat_Record=0;
	char sqltmp[SQL_LENGTH];	
	for(it=map_tableName_rcdSeq.begin();it!=map_tableName_rcdSeq.end();it++)	
	{
		//Make_Insert_Sql(sqltmp,m_SStat_Table.SzTable_Name);
		Make_Insert_Sql(sqltmp,it->first.c_str());	
		int insert_num=STAT_INSERT_CONNT;	
		ds.Open(sqltmp,NONSELECT_DML);
		int record_count=it->second.size();
		//for(i=0;i<m_NRecord_Count;i++)		
		for(i=0;i<record_count;i++)
		{				
		    if(record_count-i<STAT_INSERT_CONNT)
			   insert_num=record_count-i;	
			
			for(j=0;j<insert_num;j++)
			{	
		 		/*
		 		for(int item=0;item<m_SStat_Table.NStat_Item_Count;item++)
		  		{
		  			DeleteSpace(m_PsPre_Stat_Record[i+j].SzStat_Item[item]);
		  			if(m_SStat_Table.NItem_Type[item]==STAT_N_TYPE
		  	  			|| m_SStat_Table.NItem_Type[item]==STAT_N_COUNT_TYPE)
		  				ds<<atol(m_PsPre_Stat_Record[i+j].SzStat_Item[item]);	
					else if(m_SStat_Table.NItem_Type[item]==STAT_D_COUNT_TYPE)
						ds<<atof(m_PsPre_Stat_Record[i+j].SzStat_Item[item]);	
		  			else
		  				ds<<m_PsPre_Stat_Record[i+j].SzStat_Item[item];
		  		}
		  		if(m_SStat_Table.ChIs_Count=='Y')  
		  			ds<<m_PsPre_Stat_Record[i+j].NBill_Count; 	
		  		*/	
		  		seq_IN_m_PsPre_Stat_Record=it->second[i+j];

		  		for(int item=m_SStat_Table.table_name_key_count;item<m_SStat_Table.NStat_Item_Count;item++)
		  		{
		  			
		  			DeleteSpace(m_PsPre_Stat_Record[seq_IN_m_PsPre_Stat_Record].SzStat_Item[item]);
		  			if(m_SStat_Table.NItem_Type[item]==STAT_N_TYPE
		  	  			|| m_SStat_Table.NItem_Type[item]==STAT_N_COUNT_TYPE)
		  				ds<<atol(m_PsPre_Stat_Record[seq_IN_m_PsPre_Stat_Record].SzStat_Item[item]);	
					else if(m_SStat_Table.NItem_Type[item]==STAT_D_COUNT_TYPE)
						ds<<atof(m_PsPre_Stat_Record[seq_IN_m_PsPre_Stat_Record].SzStat_Item[item]);	
		  			else
		  				ds<<m_PsPre_Stat_Record[seq_IN_m_PsPre_Stat_Record].SzStat_Item[item];
					
		  		}
		  		if(m_SStat_Table.ChIs_Count=='Y')  
		  			ds<<m_PsPre_Stat_Record[seq_IN_m_PsPre_Stat_Record].NBill_Count; 	
				
			}
			ds.Execute();
		  	if(ds.IsError())
		    	{
		     	 ds.Close();
		     	 return (-1);
		    	}	 		 	
		 	i=i+insert_num-1;
		}
		ds.Close();	
	}
	//checkb=clock();	
	//printf("insert time:%ld\n",checkb-checka);
	return 0;
}

void CF_CStat::print(STAT_RECORD * rcd,int count)
{
	cout<<count<<" records to print------------"<<endl;
	for(int i =0 ;i<count;i++)
		{
		  for(int k=0;k<m_SStat_Table.NStat_Item_Count;k++)
		  {
		  if(m_SStat_Table.NItem_Type[k] != STAT_N_COUNT_TYPE 
		  	&& m_SStat_Table.NItem_Type[k] != STAT_D_COUNT_TYPE )
		  	cout<<rcd[i].SzStat_Item[k]<<"|";
		  }
		cout<<rcd[i].NBill_Count<<endl;

		}
	cout<<" records  print end------------"<<endl;
}

void CF_CStat::printOneRecord(STAT_RECORD * rcd)
{

	for(int k=0;k<m_SStat_Table.NStat_Item_Count;k++)
	{
		if(m_SStat_Table.NItem_Type[k] != STAT_N_COUNT_TYPE 
			&& m_SStat_Table.NItem_Type[k] != STAT_D_COUNT_TYPE )
			cout<<m_SStat_Table.SzStat_Item[k]<<":"<<rcd->SzStat_Item[k]<<"|";
	}
	cout<<rcd->NBill_Count;
	cout<<endl;
		
}

void CF_CStat::printOneRecord(STAT_RECORD &rcd)
{

	for(int k=0;k<m_SStat_Table.NStat_Item_Count;k++)
	{
		if(m_SStat_Table.NItem_Type[k] != STAT_N_COUNT_TYPE 
			&& m_SStat_Table.NItem_Type[k] != STAT_D_COUNT_TYPE )
			cout<<m_SStat_Table.SzStat_Item[k]<<":"<<rcd.SzStat_Item[k]<<"|";
	
	}
	cout<<rcd.NBill_Count;
	cout<<endl;
		
}

//update模式统计中往表中插入记录
int CF_CStat::Insert_Table()
{
	CBindSQL ds( DBConn );
	int i,j;
	char sqltmp[SQL_LENGTH];
	map<string,v_pRcd>::iterator it_table;
		
	for(it_table=map_table_recordToInsert.begin();it_table!=map_table_recordToInsert.end();it_table++)
	{
		//cout<<"insert to table:"<<it_table->first<<endl;
		Make_Insert_Sql(sqltmp,it_table->first.c_str());
		int insert_num=STAT_INSERT_CONNT;
		ds.Open(sqltmp,NONSELECT_DML);
		int icount_toInsert_table=map_table_recordToInsertCount[it_table->first];
		//cout<<"icount_toInsert_table:"<<icount_toInsert_table<<endl;
		for(i=0;i<icount_toInsert_table;i++)
		{		
		    if(icount_toInsert_table-i<STAT_INSERT_CONNT)
			   insert_num=icount_toInsert_table-i;
			  //insert_num=1;
			for(j=0;j<insert_num;j++)
			{		
		 		for(int item=m_SStat_Table.table_name_key_count;item<m_SStat_Table.NStat_Item_Count;item++)
		  		{	  		    
		  			if(m_SStat_Table.NItem_Type[item]==STAT_N_TYPE||
		  	  			m_SStat_Table.NItem_Type[item]==STAT_N_COUNT_TYPE)
		  				ds<<atol((it_table->second)[i+j]->SzStat_Item[item]);
					else if(m_SStat_Table.NItem_Type[item]==STAT_D_COUNT_TYPE)
						ds<<atof((it_table->second)[i+j]->SzStat_Item[item]);
		  			else
		  				ds<<(it_table->second)[i+j]->SzStat_Item[item];
		  		}
		  		if(m_SStat_Table.ChIs_Count=='Y')  
		  			ds<<(it_table->second)[i+j]->NBill_Count; 		
			}		
			ds.Execute();
		  	if(ds.IsError())
		    	{
		     	 ds.Close();
		     	 return (-1);
		    	}	 	 	
		 	i=i+insert_num-1;
		} 
		/*
		if(it_table->second!=NULL)
			{
			delete[] it_table->second;
			it_table->second=NULL;
			}
		*/
		ds.Close();	
	}
	map_table_recordToInsert.clear();
	map_table_recordToInsertCount.clear();
	return 0;
}

//update模式统计中update 表中记录
int CF_CStat::Update_Table()
{
	//cout<<"int Update_Table ---------------"<<endl;
	CBindSQL ds( DBConn );
	int i,j;
	char sqltmp[SQL_LENGTH];
	STAT_RECORD* crrRcd=NULL;
	
	//map<string,STAT_RECORD *> map_table_recordToUpdate;
	map<string,v_pRcd>::iterator it_table;
	for(it_table=map_table_recordToUpdate.begin();it_table!=map_table_recordToUpdate.end();it_table++)
	{
		//printf("Update_Table %s\n",it_table->first.c_str());	
		Make_Update_Sql(sqltmp,it_table->first.c_str());
		//cout<<"update:"<<sqltmp<<endl;
		int update_num=STAT_INSERT_CONNT;
		ds.Open(sqltmp,NONSELECT_DML);
		int icount_toUpdate_table=map_table_recordToUpdateCount[it_table->first];
		for(i=0;i<icount_toUpdate_table;i++)
		{		
		    if(icount_toUpdate_table-i<STAT_INSERT_CONNT)
			   update_num=icount_toUpdate_table-i;
		   
		    for(j=0;j<update_num;j++)
			{		
				//cout<<i+j<<endl;
				
			 	crrRcd=(it_table->second)[i+j];
		    		//printOneRecord(crrRcd);
		    		 //expTrace(G_CF_CStat_DebugFlag, __FILE__, __LINE__, " set ");  
 	
		 		for(int item=m_SStat_Table.table_name_key_count;item<m_SStat_Table.NStat_Item_Count;item++)
		  		{
		  			if(m_SStat_Table.NItem_Type[item]==STAT_N_COUNT_TYPE)
		  				ds<<atol(crrRcd->SzStat_Item[item]); 	
					
					if(m_SStat_Table.NItem_Type[item]==STAT_D_COUNT_TYPE)
		  				ds<<atof(crrRcd->SzStat_Item[item]); 
		  		}
		  		if(m_SStat_Table.ChIs_Count=='Y')  
		  			ds<<crrRcd->NBill_Count; 		
				 //expTrace(G_CF_CStat_DebugFlag, __FILE__, __LINE__, " UPDATE ");  
 	
				for(int item=m_SStat_Table.table_name_key_count;item<m_SStat_Table.NStat_Item_Count;item++)
				{
					if(m_SStat_Table.NItem_Type[item]==STAT_N_TYPE)
						ds<<atol(crrRcd->SzStat_Item[item]);
					else if(m_SStat_Table.NItem_Type[item]==STAT_SZ_TYPE 
						    || m_SStat_Table.NItem_Type[item]>=KEYWORK_MIN) 
						ds<<crrRcd->SzStat_Item[item]; 		
				}
			}
			//expTrace(G_CF_CStat_DebugFlag, __FILE__, __LINE__, " Execute 1 ");  
 
			ds.Execute();			
			//expTrace(G_CF_CStat_DebugFlag, __FILE__, __LINE__, " Execute 2 ");  
			
			if(ds.IsError())
		  	{
		   	 ds.Close();
		   	 return (-1);
		 	 }			 	
			i=i+update_num-1;
		}  
		ds.Close();	
		/*
		if(it_table->second!=NULL)
			{
			delete[] it_table->second;
			it_table->second=NULL;
			}
		*/
	}
	map_table_recordToUpdate.clear();
	map_table_recordToUpdateCount.clear();
	return 0;
}
    			
int CF_CStat::dealRedoRec(CFmt_Change &InRecord, int IError_Type)
{
//2005621 
Exch_Field(InRecord);
char tmp_item_contain[STAT_ITEM_CONTAINER_LEN];
/////////////////////////////////////////////////////////////////////////////////////
//                    统计结果输出到文件
/////////////////////////////////////////////////////////////////////////////////////
if(Result_Output_Des=='F')
{
  	if(Stat_Record_Insert(InRecord,IError_Type)) return 0;  	
  	int i;
  	for(i=1;i<m_SStat_Table.NStat_Item_Count+1;i++)
  		{
  		 if(m_SStat_Table.ChStat_Type=='-' && m_SStat_Table.NItem_Type[i-1]==STAT_N_COUNT_TYPE)
  		 	{
  	  		sprintf(tmp_item_contain,"%ld",-atol(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i-1]));
  		    sprintf(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i-1],"%s",tmp_item_contain);
  		 	}
		 else if(m_SStat_Table.ChStat_Type=='-' && m_SStat_Table.NItem_Type[i-1]==STAT_D_COUNT_TYPE)
  		 	{
  	  		sprintf(tmp_item_contain,"%lf",-atof(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i-1]));
  		    sprintf(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i-1],"%s",tmp_item_contain);
  		 	}
  		Out_Record.Set_Field(i,m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i-1]);	
  		}
  	if(m_SStat_Table.ChIs_Count=='Y')
  		{
  		char c_count[10];
  		if(m_SStat_Table.ChStat_Type=='+')
  			sprintf(c_count,"%d",m_PsPre_Stat_Record[m_NRecord_Count].NBill_Count);
  		else
  			sprintf(c_count,"%d",-m_PsPre_Stat_Record[m_NRecord_Count].NBill_Count);  	
  		Out_Record.Set_Field(i,c_count); 
  	    Temp_Out_File.writeRec(Out_Record);
  	    } 	
}

/////////////////////////////////////////////////////////////////////////////////////
//                    统计结果输出到表
/////////////////////////////////////////////////////////////////////////////////////
else
{
  //update模式
  if (m_SStat_Table.CHStat_Mode=='U')
  	{
   	  //20080604
  	  //if(Stat_Record_Row(InRecord,IError_Type)) 
  	  Stat_Record_Update(InRecord,IError_Type);	
	  return 0;	
  	  }
  //insert 模式，累积到一定数量，做批量插入操作
  else
  	{
  	//横表插入  	
  	  if(Stat_Record_Insert(InRecord,IError_Type)) return 0;  	  
  	   //若为冲销统计
  	  if(m_SStat_Table.ChStat_Type=='-')
  	  	{
  	  	for(int i=0;i<m_SStat_Table.NStat_Item_Count;i++)
  	  		{
  	  		if(m_SStat_Table.NItem_Type[i]==STAT_N_COUNT_TYPE)
  	  			{
  	  			sprintf(tmp_item_contain,"%ld",
	                  -atol(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i]));
  	  			sprintf(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i],"%s",tmp_item_contain);
  	  			}
			if(m_SStat_Table.NItem_Type[i]==STAT_D_COUNT_TYPE)
  	  			{
  	  			sprintf(tmp_item_contain,"%lf",
	                  			-atof(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i]));
  	  			sprintf(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i],"%s",tmp_item_contain);
  	  			}
  	  		}
  	  	}
  	  m_NRecord_Count++;  	 

	    if(m_NRecord_Count>=m_maxRcdToUpdate )
  		{
  		//批量插入
  		//cout<<"批量插入"<<endl;
  		if(Insert_Stat())
	  		{
			return (-1);
	  		}
		m_NRecord_Count=0;	
	    	}
  	   return 0;
  	}//insert
  }//table
}
  

void CF_CStat::update_commit()
{ 
   //expTrace(G_CF_CStat_DebugFlag, __FILE__, __LINE__, "to update table .. "); 
   if(Result_Output_Des!='T')
   	{
	 return;
   	}
   //cout<<"update_commit:m_SStat_Table.CHStat_Mode==="<<m_SStat_Table.CHStat_Mode<<endl;
   if (m_SStat_Table.CHStat_Mode=='U')
   	{  	
   	//20080604
  	//Update_Stat_File(); 
	if(icount_toUpdate==0)
		{
		return;
		}
	/*
    	//为Record_toInsert[]、Record_toUpdate[]申请 icount_toUpdate_table个单位空间  
  	Record_toInsert=new STAT_RECORD[icount_toUpdate];
  	Record_toUpdate=new STAT_RECORD[icount_toUpdate];
  	if(Record_toInsert==NULL||Record_toUpdate==NULL)
	    {
	    throw CF_CError('E','H',ERR_REQU_MEM,0,(char *)"new err.",__FILE__,__LINE__);
	    }
  	*/
  	expTrace(G_CF_CStat_DebugFlag, __FILE__, __LINE__, "%d records to update table ",icount_toUpdate);  
 	Prepare_Rec();		
	//20080513,一开始已经申请足够空间
    	 /*
 	if(Stat_Record_toUpdate!=NULL)
 		{
 		delete[] Stat_Record_toUpdate;
 		Stat_Record_toUpdate=NULL;
 		}
  	Stat_Record_toUpdate = new STAT_RECORD[MEMORY_APPLY_UNIT];
  	if(Stat_Record_toUpdate==NULL)
       	{
	    	throw CF_CError('E','H',ERR_REQU_MEM,0,(char *)"new err.",__FILE__,__LINE__);
	    	}
  	Current_Memorry_Unit=MEMORY_APPLY_UNIT;
	*/
  	icount_toUpdate = 0;
	m_key_record.clear();
	m_key_seq.clear();
  	m_NRecord_Count=0;

    clock_t checka=0;
    clock_t checkb=0;
    checka=clock();

    
   //根据Record_toUpdate[]中记录更新表    
   try
   	{
   	 //将map_table_recordToUpdate中记录插入表  
   	expTrace(G_CF_CStat_DebugFlag, __FILE__, __LINE__, " to update table:%s ",m_SStat_Table.SzTable_Name);  
 	
        Update_Table();	  
   	//printf("icount_toInsert_table=%d\n",icount_toInsert_table);
   	expTrace(G_CF_CStat_DebugFlag, __FILE__, __LINE__, " to INSERT table:%s ",m_SStat_Table.SzTable_Name);  
       Insert_Table();
   	}
   catch (CF_CError e)
      {
      //20070419,增强了错误信息输出
      char szLogStr[700];
      char DBerrmsg[500];
      if(strlen(e.get_errMessage())<499)
      		{
      		strcpy(DBerrmsg,e.get_errMessage());
      		}
      else
      		{
	      strncpy(DBerrmsg,e.get_errMessage(),499);
	      DBerrmsg[499]=0;
      		}
      sprintf(szLogStr,"fail to update table. ErrorMessage:%s",DBerrmsg);
      throw CF_CError('D','H',ERR_INSERT_TABLE,0,szLogStr,__FILE__,__LINE__);
      }
   catch(...)
	{
	 char szLogStr[100];	
	strcpy(szLogStr,"update table,unknow error!!! ");
	expTrace("Y", __FILE__, __LINE__, "update table,unknow error!!! ");  
       throw CF_CError('D','H',ERR_INSERT_TABLE,0,szLogStr,__FILE__,__LINE__);
	}
    checkb=clock();

    if(Current_Memorry_Unit>m_maxRcdToUpdate &&m_SStat_Table.CHStat_Mode=='U')
	{
	try
		{
		 if(Stat_Record_toUpdate!=NULL)
			delete[] Stat_Record_toUpdate;
		Stat_Record_toUpdate = new STAT_RECORD[m_maxRcdToUpdate];
		Current_Memorry_Unit=m_maxRcdToUpdate;
		}
	catch(...)
		{
		throw CF_CError('E','H',ERR_REQU_MEM,0,(char *)"new err.",__FILE__,__LINE__);
		}
	}
	
   }
  else
  	{
  	try
  		{
	  	Insert_Stat();
  		}
  	catch(CDBException ex)
  		{
  		cout<<"Error number: "<<  ex.GetErrorCode() << endl;
		cout<<"Error Message: "<<ex.GetErrorMsg() << endl;    
		throw(ex);
  		}
	catch(...)
  		{
  		throw CF_CError('E','H',ERR_INSERT_TABLE,0,(char *)"fail to insert record into table.",__FILE__,__LINE__);
  		}
  	m_NRecord_Count=0;
  	}
	expTrace(G_CF_CStat_DebugFlag, __FILE__, __LINE__, "update table finish! "); 
}

void CF_CStat::rollback()
{
  m_key_record.clear();
  m_key_seq.clear();
  m_NRecord_Count=0;
  /*
  if(Stat_Record_toUpdate!=NULL)
  	{
  	delete[] Stat_Record_toUpdate;
  	Stat_Record_toUpdate=NULL;
  	}
  Stat_Record_toUpdate = new STAT_RECORD[MEMORY_APPLY_UNIT];
  if(Stat_Record_toUpdate==NULL)
    throw CF_CError('E','H',ERR_REQU_MEM,0,(char *)"new err.",__FILE__,__LINE__);		
  Current_Memorry_Unit=MEMORY_APPLY_UNIT;  
  */
  icount_toUpdate = 0;  
  /*
  icount_toInsert_table=0;
  icount_toUpdate_table=0;

  if(Record_toInsert!=NULL)
  	{
    delete[] Record_toInsert;
    Record_toInsert=NULL;
  	}
  if(Record_toUpdate!=NULL)
  	{
    delete[] Record_toUpdate;
    Record_toUpdate=NULL;
  	}
  */
   if(Current_Memorry_Unit>m_maxRcdToUpdate &&m_SStat_Table.CHStat_Mode=='U')
   	{
	 try
	  	{
	  	
		 if(Stat_Record_toUpdate!=NULL)
				delete[] Stat_Record_toUpdate;
		Stat_Record_toUpdate = new STAT_RECORD[m_maxRcdToUpdate];
		Current_Memorry_Unit=m_maxRcdToUpdate;
	  	}
	  catch(...)
		{
		throw CF_CError('E','H',ERR_REQU_MEM,0,(char *)"new err.",__FILE__,__LINE__);
		}
   	}
   
   map<string,v_pRcd>::iterator it_table;		
   for(it_table=map_table_recordToInsert.begin();it_table!=map_table_recordToInsert.end();it_table++)
	{
	/*
	if(it_table->second!=NULL)
		{
		delete[] it_table->second;
		it_table->second=NULL;		
		}
	*/
	it_table->second.clear();
	}
  map_table_recordToInsert.clear();
	
   for(it_table=map_table_recordToUpdate.begin();it_table!=map_table_recordToUpdate.end();it_table++)
	{
	/*
	if(it_table->second!=NULL)
		{
		delete[] it_table->second;
		it_table->second=NULL;
		}
	*/
	it_table->second.clear();
	}
  map_table_recordToUpdate.clear();
  
  if(Result_Output_Des=='F')
	{
	Unlink_TemFile();
	Unlink_File();
  	}
}


//2005621， 整合业务出现名为Fee_A、Fee_B的字段，对此业务不交换字段值
int CF_CStat::Exch_Field(CFmt_Change &inrcd)
{
	if(strcmp(inrcd.Get_id(),"TYSTD")==0)
		return 0;
    int iInfee_Index=0,iOutfee_Index=0;
    int iTmpfee=0,iInFee=0,iOut_Fee=0;
    char szTmpFee[10];
    iInfee_Index=inrcd.Get_FieldIndex((char *)"Fee_A");
    if(iInfee_Index!=(-1))
    {
      iInFee=atol(inrcd.Get_Field(iInfee_Index));
//    sprintf(tmpss,"can not fine the COLNAME:Fee_A for %s from TXTFILE_FMT.",Param.szInputFiletypeId);
//    throw CF_CError('E','M',ERR_COLNAME_NOT_FINE,errno,tmpss,__FILE__,__LINE__);
    }
    iOutfee_Index=inrcd.Get_FieldIndex((char *)"Fee_B");
    if(iOutfee_Index!=(-1))
    {
      iOut_Fee=atol(inrcd.Get_Field(iOutfee_Index));
    }
    if((iInFee<0)||(iOut_Fee>0))
    {
      sprintf(szTmpFee,"%d",iInFee);
      inrcd.Set_Field(iOutfee_Index,szTmpFee);
      sprintf(szTmpFee,"%d",iOut_Fee);
      inrcd.Set_Field(iInfee_Index,szTmpFee);
    }

    return 0;
}

void CF_CStat::get_preday_time(char* curtime)
/*取得前一天的时间 */
{
  	time_t		time1;
	struct tm	*time2;
	time(&time1);
	time_t timepre = time1-86400;
	time2 = localtime(&timepre);	
	sprintf(curtime, "%4d%02d%02d%02d%02d%02d", time2->tm_year+1900, 
		time2->tm_mon+1, time2->tm_mday, time2->tm_hour, time2->tm_min, time2->tm_sec);

}



CF_DUP_CStat::CF_DUP_CStat()
{
  iStatArr_Count=0;
  //iStatArr_Valid_Count=0;
  pStatArr=NULL;
  total_update_count=0;
}

CF_DUP_CStat::~CF_DUP_CStat()
{
  if(pStatArr!=NULL)
  {
    delete[] pStatArr;
    pStatArr=NULL;
    iStatArr_Count=0; 
  }  
}


int CF_DUP_CStat::Init(char *Pipe_id,int Process_id, char *configid,char statfieldnullpass)
{
  CBindSQL ds( DBConn );	//DynamicSQL SqlStr;
  char szTemp[100];
  
  char c_maxRcdUpdat[10];
  c_maxRcdUpdat[0]=0;
  int maxRcdNumToUpdat=0;
  if(getEnvFromDB(DBConn,Pipe_id, Process_id, "STAT_MAXNUM_UPATE", c_maxRcdUpdat)<0)
    	maxRcdNumToUpdat=MEMORY_APPLY_UNIT;
  else
    	maxRcdNumToUpdat=atoi(c_maxRcdUpdat);   
   
  
  if(statfieldnullpass!='0')
  	chFieldNULLPass=statfieldnullpass;
  else
  	{
  	if(getEnvFromDB(DBConn,Pipe_id, Process_id, "STAT_FIELDNULLPASS", szTemp)<0)
  	{
    	sprintf(szTemp,"has not defined variable: STAT_FIELDNULLPASS !");
    	throw CF_CError('D','H',ERR_GETENV,0,szTemp,__FILE__,__LINE__);
  	}
  	chFieldNULLPass=szTemp[0];
  	}
  if(configid!=NULL)
  	sprintf(szTemp,configid);
  else
  	{
  	if(getEnvFromDB(DBConn,Pipe_id, Process_id, "STAT_CONFIGID", szTemp)<0)
  	{
   	 sprintf(szTemp,"has not defined variable: STAT_CONFIGID !");
    	throw CF_CError('D','H',ERR_GETENV,0,szTemp,__FILE__,__LINE__);
 	 }
  	delSpace(szTemp,0);
  	expTrace("Y", __FILE__, __LINE__,"STAT_CONFIGID=%s;", szTemp);
  	}

    if(getEnvFromDB(DBConn,Pipe_id,Process_id,"DEBUG_FLAG",G_CF_CStat_DebugFlag)<0)
    {
      sprintf(G_CF_CStat_DebugFlag, "%s", "N");
    }    
    
    //日帐分单时间点，20070314
	char endTime[6];
	if(getEnvFromDB( DBConn, Pipe_id, Process_id, "STAT_END_TIME", endTime )<0)
		{	
		//throw CF_CError('D','H',ERR_GETENV,0,szTemp,__FILE__,__LINE__);
		strcpy(endTime,"00:00");
		expTrace("Y", __FILE__, __LINE__,
     		"variable STAT_END_TIME has not been defined! set as %s;", endTime);		
		}
	else
		{
		expTrace("Y", __FILE__, __LINE__,"STAT_END_TIME=%s;", endTime);
		}
	int j=0;
	char dataType_endTime[6];
	cout<<"set endtime:"<<endTime<<endl;
	for(int i=0;i<strlen(endTime);i++)
		{
		if(endTime[i]!=':')
			{
			dataType_endTime[j]=endTime[i];
			j++;
			}
		}
	dataType_endTime[j]=0;
	cout<<"dataType_endTime="<<dataType_endTime<<endl;
  char *TmpPoint1;
  char *TmpPoint2;
  int iTmpCount=0;
  TmpPoint1=szTemp;
  while(1)
  {
    iTmpCount++;
    TmpPoint2=strchr(TmpPoint1,ConfigIDSPILT);
    if(TmpPoint2==NULL) break;
    TmpPoint1=TmpPoint2+1;
  }
  iStatArr_Count=iTmpCount;

  pStatArr=new CF_CStat[iStatArr_Count];
  if(pStatArr==NULL)
  {
    sprintf(szTemp,"new CF_CStat pStatArr err !");
    throw CF_CError('E','H',ERR_REQ_MEM,0,szTemp,__FILE__,__LINE__);
  }
  int *pConfigID = new int[iStatArr_Count];
  if(pConfigID==NULL)
  {
    sprintf(szTemp,"new int pConfigID err !");
    throw CF_CError('E','H',ERR_REQ_MEM,0,szTemp,__FILE__,__LINE__);
  }
  TmpPoint1=szTemp;
  int i=0;
  for(i=1;i<iStatArr_Count;i++)
  {
    TmpPoint2=strchr(TmpPoint1,ConfigIDSPILT);
    *TmpPoint2=0;
    pConfigID[i-1]=atol(TmpPoint1);
    TmpPoint1=TmpPoint2+1;
  }
  pConfigID[i-1]=atol(TmpPoint1);

	int table_cnt=0;
	char valid_flag;
	int true_id=0;
	
  for(i=0;i<iStatArr_Count;i++)
  {
  	  if(pConfigID[i]<0)
	  	true_id=-pConfigID[i];
	  else 
	  	true_id= pConfigID[i];
	  
	  ds.Open("select VALID_FLAG from stat_table_define where CONFIG_ID=:id");
	  ds<<true_id;
	  ds>>valid_flag;
	  ds.Close();
	  if(valid_flag=='Y')
	  	{
	      pStatArr[table_cnt].Init(Pipe_id,Process_id,pConfigID[i],dataType_endTime,maxRcdNumToUpdat);
	      table_cnt++;
	      }
  }  
  iStatArr_Count=table_cnt;
  expTrace("Y", __FILE__, __LINE__,"iStatArr_Count=%d;", iStatArr_Count);
  if(iStatArr_Count==0)
  	{
  	//
  	 sprintf(szTemp,"no valid CONFIG_ID in stat_table_define !");
	expTrace("Y", __FILE__, __LINE__,szTemp);	
   	 throw CF_CError('E','H',ERR_VALID_CONFIGID,0,szTemp,__FILE__,__LINE__);
  	}
  if(pConfigID!=NULL)
  	{
  	delete[] pConfigID;
  	pConfigID=NULL;
  	}
  
  return 0;
}

/*
int CF_DUP_CStat::Init(char *Pipe_id,int Process_id, char *configid,char statfieldnullpass)
{
  CBindSQL ds( DBConn );	//DynamicSQL SqlStr;
  char szTemp[100];
  
  if(statfieldnullpass!='0')
  	chFieldNULLPass=statfieldnullpass;
  else
  	{
	  	if(getEnvFromDB(DBConn,Pipe_id, Process_id, "STAT_FIELDNULLPASS", szTemp)<0)
	  	{
	    	sprintf(szTemp,"has not defined variable: STAT_FIELDNULLPASS !");
	    	throw CF_CError('D','H',ERR_GETENV,0,szTemp,__FILE__,__LINE__);
	  	}
  	chFieldNULLPass=szTemp[0];
  	}
  if(configid!=NULL)
  	sprintf(szTemp,configid);
  else
  	{
  	if(getEnvFromDB(DBConn,Pipe_id, Process_id, "STAT_CONFIGID", szTemp)<0)
	  	{
	   	 sprintf(szTemp,"has not defined variable: STAT_CONFIGID !");
	    	throw CF_CError('D','H',ERR_GETENV,0,szTemp,__FILE__,__LINE__);
	 	 }
  	delSpace(szTemp,0);
  	expTrace("Y", __FILE__, __LINE__,"STAT_CONFIGID=%s;", szTemp);
  	}

    //日帐分单时间点，20070314
	char endTime[6];
	if(getEnvFromDB( DBConn, Pipe_id, Process_id, "STAT_END_TIME", endTime )<0)
		{	
		//throw CF_CError('D','H',ERR_GETENV,0,szTemp,__FILE__,__LINE__);
		strcpy(endTime,"00:00");
		expTrace("Y", __FILE__, __LINE__,
     		"variable STAT_END_TIME has not been defined! set as %s;", endTime);		
		}
	else
		{
		expTrace("Y", __FILE__, __LINE__,"STAT_END_TIME=%s;", endTime);
		}
	int j=0;
	char dataType_endTime[6];
	cout<<"set endtime:"<<endTime<<endl;
	for(int i=0;i<strlen(endTime);i++)
		{
		if(endTime[i]!=':')
			{
			dataType_endTime[j]=endTime[i];
			j++;
			}
		}
	dataType_endTime[j]=0;
	cout<<"dataType_endTime="<<dataType_endTime<<endl;
  char *TmpPoint1;
  char *TmpPoint2;
  int iTmpCount=0;
  TmpPoint1=szTemp;
  while(1)
  {
    iTmpCount++;
    TmpPoint2=strchr(TmpPoint1,ConfigIDSPILT);
    if(TmpPoint2==NULL) break;
    TmpPoint1=TmpPoint2+1;
  }
  iStatArr_Count=iTmpCount;
  
  int i=0;
  int *pConfigID = new int[iStatArr_Count];
  if(pConfigID==NULL)
  {
    sprintf(szTemp,"new int pConfigID err !");
    throw CF_CError('E','H',ERR_REQ_MEM,0,szTemp,__FILE__,__LINE__);
  }
  
  TmpPoint1=szTemp;
  for(i=1;i<iStatArr_Count;i++)
  {
    TmpPoint2=strchr(TmpPoint1,ConfigIDSPILT);
    *TmpPoint2=0;
    pConfigID[i-1]=atol(TmpPoint1);
    TmpPoint1=TmpPoint2+1;
  }
  pConfigID[i-1]=atol(TmpPoint1);

  cout<<"config_id in pipe_env---"<<endl;
  for(i=0;i<iStatArr_Count;i++)
  	{
  	cout<<i<<"|"<<pConfigID[i]<<endl;
  	}
  
  //检查stat_table_define里开关是否打开
  char valid_flag;
  for(i=0;i<iStatArr_Count;i++)
  {
	  ds.Open("select VALID_FLAG from stat_table_define where CONFIG_ID=:id");
	  ds<<pConfigID[i];
	  ds>>valid_flag;
	  cout<<"pConfigID[i]="<<pConfigID[i]<<"|valid_flag="<<valid_flag<<endl;
	  if(valid_flag=='Y')
	  	{
	  	iStatArr_Valid_Count++;
	  	}
	  else
	  	{
	  	pConfigID[i]=-1;
	  	}
  }
  cout<<"iStatArr_Valid_Count="<<iStatArr_Valid_Count<<endl;
   cout<<"config_id later ---"<<endl;
  for(i=0;i<iStatArr_Count;i++)
  	{
  	cout<<i<<"|"<<pConfigID[i]<<endl;
  	}
  
  pStatArr=new CF_CStat[iStatArr_Valid_Count];
  if(pStatArr==NULL)
  {
    sprintf(szTemp,"new CF_CStat pStatArr err !");
    throw CF_CError('E','H',ERR_REQ_MEM,0,szTemp,__FILE__,__LINE__);
  }

  int table_count=0;
  for(i=0;i<iStatArr_Count;i++)
  {
  	if(pConfigID[i]!=-1)
  		{
    		pStatArr[table_count].Init(Pipe_id,Process_id,pConfigID[i],dataType_endTime);
	       table_count++;
  		}
  }  
  cout<<"iStatArr_Valid_Count="<<iStatArr_Valid_Count<<"|table_count="<<table_count<<endl;
  
  if(pConfigID!=NULL)
  	{
  	delete[] pConfigID;
  	pConfigID=NULL;
  	}
  
  return 0;
}

*/
/*
int CF_DUP_CStat::setFileName(char *FileName)
{
  int i;
  for(i=0;i<iStatArr_Count;i++)
  {
    pStatArr[i].setFileName(FileName);
  }
  
  return 0;

}
*/

//20070509,从外部传入sourceid
int CF_DUP_CStat::setFileName(char *FileName,char *sourceid)
{
//cout<<"iStatArr_Count="<<iStatArr_Count<<endl;
  int i;
  for(i=0;i<iStatArr_Count;i++)
  {
    pStatArr[i].setFileName(FileName,sourceid);
  }
  
  return 0;

}
//20070509,从外部传入sourceid,dealstarttime
int CF_DUP_CStat::setFileName(char *FileName,char *sourceid,char *dealstarttime)
{
//cout<<"iStatArr_Count="<<iStatArr_Count<<endl;
  int i;
  for(i=0;i<iStatArr_Count;i++)
  {
    pStatArr[i].setFileName(FileName,sourceid,dealstarttime);
  }
  
  return 0;

}
//20080710,从外部传入process_id
int CF_DUP_CStat::setFileName(char *FileName,char *sourceid,char *dealstarttime,int process_id)
{
//cout<<"iStatArr_Count="<<iStatArr_Count<<endl;
  int i;
  for(i=0;i<iStatArr_Count;i++)
  {
    pStatArr[i].setFileName(FileName,sourceid,dealstarttime,process_id);
  }
  
  return 0;

}

//设置临时统计结果输出文件
void CF_DUP_CStat::Set_TempOutFile(char * today, int group_id )
{
  int i;
  for(i=0;i<iStatArr_Count;i++)
  {
    pStatArr[i].Set_TempOutFile(today,group_id);
  }  
}

void CF_DUP_CStat::Close_Temp_Outfile()
{
  int i;
  for(i=0;i<iStatArr_Count;i++)
  {
    pStatArr[i].Close_Temp_Outfile();
  }  
}


//将临时文件更名为正式文件,不成功返回1，否则返回0
int CF_DUP_CStat::Rename_OutFile()
{
  int i;
  for(i=0;i<iStatArr_Count;i++)
  {
    if(pStatArr[i].Rename_OutFile())		
		return 1;
  }  
  return 0;
}

int CF_DUP_CStat::dealRedoRec(CFmt_Change &inrcd, int IError_Type)
{
//cout<<"iStatArr_Count="<<iStatArr_Count<<endl;
  int iNull_Count;
  iNull_Count=0;
  int i;
  for(i=0;i<iStatArr_Count;i++)
  {
    try
    {
      pStatArr[i].dealRedoRec(inrcd,IError_Type);
      total_update_count=G_CF_CStat_UpdateCount;
    }
    catch(CF_CError e)
    {
     int iErrorNo;
     iErrorNo = e.get_appErrorCode();
     expTrace("Y", __FILE__, __LINE__,"iErrorNo=%d;", iErrorNo);
     expTrace("Y",__FILE__, __LINE__, "%s", e.get_errMessage());
     if(iErrorNo != ERR_FIELD_NULL)
     	throw e;
     if(chFieldNULLPass == NONULLPASS)
     	throw e;
     iNull_Count++;
    }
  }
  if(iNull_Count != iStatArr_Count) 
  	return 0;
  else if(chFieldNULLPass == ALLNULLPASS) 
  	return 0;
  else{
		char szTemp[100];
  		sprintf(szTemp,"There is NULL Field in All STAT_Group !");
  		throw CF_CError('E','H',ERR_ALL_ITEM_NULL,0,szTemp,__FILE__,__LINE__);
		return 0;
  	 }
}

//供冲销使用，判断统计结果是否输出到文件
//若是，则将临时结果文件改成正式文件
//成功返回0，失败返回1
int CF_DUP_CStat::Neg_Commit()
{
	return(Rename_OutFile());
}

void CF_DUP_CStat::update_commit()
{
   /*int i=0, j=0;
  while(1)
  	{
  	 if(i==0)
  	 	for(i=0;i<iStatArr_Count;i++)
  		{
    	//pStatArr[i].update_commit();
    	//在此启动多个线程，调用thread_commit(pStatArr[i])， 做置commit_flag 为1操作
  		}
  	 for(j=0;j<iStatArr_Count;j++)
  	 	{
  	 	if(pStatArr[i].commit_flag==0)
  	 		break;
  	 	}
  	 if(j==iStatArr_Count)
  	 	break;
  	}*/
  	for(int i=0;i<iStatArr_Count;i++)
  		{
  		pStatArr[i].update_commit();
  		}
	
	 G_CF_CStat_UpdateCount=0;
  return;
}



void CF_DUP_CStat::rollback()
{
  int i;
  for(i=0;i<iStatArr_Count;i++)
  {
    pStatArr[i].rollback();
  }  

  G_CF_CStat_UpdateCount=0;
  return;
}

void CF_DUP_CStat::print_iStatArr_Count()
{
cout<<"iStatArr_Count="<<iStatArr_Count<<endl;
}


