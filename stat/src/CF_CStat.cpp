//公用统计类

#include "CF_CStat.h"
using namespace std;
using namespace tpss;

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
  m_key_record.clear();
  m_key_seq.clear();

  Stat_Record_toUpdate=NULL;
  m_PsPre_Stat_Record=NULL;
  m_PsRes_Stat_Record=NULL;
  
  map_table_recordToInsert.clear();
  map_table_recordToUpdate.clear();
  map_table_recordToMerge.clear();
  map_table_recordToInsertCount.clear();
  map_table_recordToUpdateCount.clear();
  map_table_recordToMergeCount.clear();

  middRcdCount=0;
  statRcdCount=0;

  v_tmpOutFileName.clear();
  v_outFileName.clear();
  strcpy(Temp_Out_File_Name,"-1");
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
int CF_CStat::Init(int para_config_id,int maxRcdNumToUpdat)
{
   //pLog=m_log;
  theJSLog<<"para_config_id="<<para_config_id<<endi;
  DBConnection conn;//数据库连接
  try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			m_maxRcdToUpdate=maxRcdNumToUpdat;	
      char errmsg[200];     
      config_id=abs(para_config_id);   
      m_NRecord_Count=0;
      char tmpstr[400];   
    
    //get Table Define
      string sql = "select Table_Name,Stat_Type,Is_Count,Count_Item_Name from "
        "C_STAT_TABLE_DEFINE where config_id=:config_id";
      stmt.setSQLString(sql);
			stmt << config_id;			
			stmt.execute();
			stmt >> m_SStat_Table.SzTable_Name;

      DeleteSpace(m_SStat_Table.SzTable_Name);
     /* if (ds.IsEnd()) 
      {
        	strcpy(errmsg,"select from STAT_TABLE_FMT err:no match CONFIG_ID!");
           errLog(LEVEL_ERROR, "",  ERR_SELECT ,errmsg,__FILE__,__LINE__);
    	throw CException(ERR_SELECT,errmsg,__FILE__,__LINE__);
        return (-1);
      }  */
      
      stmt>>m_SStat_Table.ChStat_Type>>m_SStat_Table.ChIs_Count>>m_SStat_Table.SzCount_Item_Name;
      /*if (ds.IsEnd()) 
      {
        	strcpy(errmsg,"select from C_STAT_TABLE_FMT err:no match CONFIG_ID!");
           errLog(LEVEL_ERROR, "",  ERR_SELECT ,errmsg,__FILE__,__LINE__);
    	throw CException(ERR_SELECT,errmsg,__FILE__,__LINE__);
      }*/
      DeleteSpace(m_SStat_Table.SzCount_Item_Name);
    
      if(para_config_id<0)
      	{
      	m_SStat_Table.ChStat_Type='-';
    	  theJSLog<<"ChStat_Type="<<m_SStat_Table.ChStat_Type<<"，准备冲销统计..."<<endi;
      	}
      else
      	{
      	theJSLog<<"ChStat_Type="<<m_SStat_Table.ChStat_Type<<endi;
      	}
      
      //查询统计模式insert or update or merge
      sql = "select Stat_Mode from C_STAT_TABLE_DEFINE where config_id=:config_id";
      stmt.setSQLString(sql);
			stmt << config_id;			
			stmt.execute();
			stmt >> m_SStat_Table.CHStat_Mode;
      if(m_SStat_Table.CHStat_Mode!='I' && m_SStat_Table.CHStat_Mode!='T'&& m_SStat_Table.CHStat_Mode!='M'&&m_SStat_Table.CHStat_Mode!='U')
      	 m_SStat_Table.CHStat_Mode!='M';//默认是merge模式
			
			try
    	{
    	if(m_SStat_Table.CHStat_Mode=='I')
  	  	m_PsPre_Stat_Record = new STAT_RECORD[maxRcdNumToUpdat];
  	 else
  		//theJSLog<<"test"<<endi;
  	  	Stat_Record_toUpdate = new STAT_RECORD[maxRcdNumToUpdat];
  	    Current_Memorry_Unit=maxRcdNumToUpdat;
    	}
      catch(...)
    	{
    	  errLog(LEVEL_ERROR, "",  ERR_REQU_MEM ,"new err.",__FILE__,__LINE__);
    	  throw CException(ERR_REQU_MEM,(char *)"new err.",__FILE__,__LINE__);
    	}
    
     //cout<<"m_SStat_Table.CHStat_Mode==="<<m_SStat_Table.CHStat_Mode<<endl;
     //查询统计结果输出方式，初始化结果输出方式及路径
     sql = "select Output_Des from C_STAT_TABLE_DEFINE where config_id=:config_id";
     stmt.setSQLString(sql);
			stmt << config_id;			
			stmt.execute();
			stmt >> Result_File_Path;
 
    DeleteSpace(Result_File_Path);
    if(strlen(Result_File_Path)==0)
    	{
    		Result_Output_Des='T';
    	}
  	
    else  	
    	{  	
    	Result_Output_Des='F';    	
    	if (Result_File_Path[strlen(Result_File_Path)-1] != '/')
     		strcat(Result_File_Path, "/");        
    	}
  
   if(Result_Output_Des=='F' && m_SStat_Table.CHStat_Mode=='U')
   	{
   	sprintf(errmsg,"error configuration in  C_STAT_TABLE_DEFINE:Output_Des=%s,Stat_Mode='U'",Result_File_Path);
         errLog(LEVEL_ERROR, "",  ERR_CONFIGID ,errmsg,__FILE__,__LINE__);
  	throw CException(ERR_CONFIGID,errmsg,__FILE__,__LINE__);
   	}
  
   theJSLog<<"Result_Output_Des="<<Result_Output_Des<<endi;
   theJSLog<<"Stat_Mode="<<m_SStat_Table.CHStat_Mode<<endi;
  
    char Out_Record_Type_ID;
    if(Result_Output_Des=='F')
    {
      //读C_STAT_TABLE_DEFINE,获取Out_File_type_id 
      sql = "select Out_File_Type_ID from C_STAT_TABLE_DEFINE where config_id=:config_id";
      stmt.setSQLString(sql);
			stmt << config_id;			
			stmt.execute();
			stmt >> Out_File_Type_ID;
 
  	  DeleteSpace(Out_File_Type_ID);
      if(strlen(Out_File_Type_ID)==0)
      {      
          strcpy(errmsg,"table C_STAT_TABLE_DEFINE error:Out_File_Type_ID is null!");
         errLog(LEVEL_ERROR, "",  ERR_FILED_VAL ,errmsg,__FILE__,__LINE__);
         throw CException(ERR_FILED_VAL,errmsg,__FILE__,__LINE__);
    	  return (-1);
  	}
      theJSLog<<"Temp_Out_File.Init(Out_File_Type_ID);"<<endd;
      Temp_Out_File.Init(Out_File_Type_ID);
      
      //读 filetype_define 表获得out_record_type_id
      sql = "select record_type from c_filetype_define where filetype_id=:id";
      stmt.setSQLString(sql);
			stmt << Out_File_Type_ID;			
			stmt.execute();

  	if(!(stmt>>Out_Record_Type_ID))
  		{
  		throw CException(ERR_SELECT,(char *)"select from filetype_define:no match filetype_id!",__FILE__,__LINE__);
    	      return (-1);
  		}
  	Out_Record.Init(Out_File_Type_ID,Out_Record_Type_ID); 	
    }		
    //查询统计表结构
    // cout<<"config id "<<config_id<<"stat_item:---"<<endl;
    sql = "select count(*) from C_STAT_TABLE_FMT where config_id=:config_id";
    stmt.setSQLString(sql);
		stmt << config_id;			
		stmt.execute();
    stmt>>m_SStat_Table.NStat_Item_Count;

    if(m_SStat_Table.NStat_Item_Count>STAT_ITEM_COUNT)
    {
     strcpy(errmsg,"table item count is over uplimit!");
     errLog(LEVEL_ERROR, "",  ERR_TABLEITEM_OVER ,errmsg,__FILE__,__LINE__);
      throw CException(ERR_TABLEITEM_OVER,errmsg,__FILE__,__LINE__);
  	return (-1);
    }
  
    //查询已有统计表
    char tmp_sql[200];
    char table_name[200];
    char nameForQuery[200];
    sprintf(nameForQuery,"%s_",m_SStat_Table.SzTable_Name);
  
    cout<<"nameForQuery="<<nameForQuery<<endl;
    sprintf(tmp_sql,"SELECT table_name FROM user_tables WHERE table_name like '%s_%'",m_SStat_Table.SzTable_Name);    
    stmt.setSQLString(tmp_sql);		
		stmt.execute();
		
    //ds.Open("SELECT table_name FROM user_tables WHERE table_name like :v"); 
    //ds<<nameForQuery;
    while(stmt>>table_name)
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
    sql = "select count(*) from C_STAT_TABLE_FMT a where a.IS_TABLE_NAME_KEY='Y'  and a.CONFIG_ID=:config_id";
    stmt.setSQLString(sql);		
    stmt<<config_id;
		stmt.execute();
    stmt>>m_SStat_Table.table_name_key_count;
    if(m_SStat_Table.table_name_key_count!=0)
    	{
    		sql = "select Table_Item,Item_Type,Field_Name,Field_Begin,Field_End,Defv_Or_Func"
  	    ",nvl(indexinfield,-1),nvl(sprinfield,' '),nvl(unit,' ') from C_STAT_TABLE_FMT where "
  	    "IS_TABLE_NAME_KEY='Y'  and config_id=:config_id order by Table_Item";
  	    stmt.setSQLString(sql);		
        stmt<<config_id;
		    stmt.execute();
  	  for( i=0;i<m_SStat_Table.table_name_key_count;i++)
  	  	{
  	  	stmt>>m_SStat_Table.SzStat_Item[i]>>m_SStat_Table.NItem_Type[i]
  		      >>m_SStat_Table.SzStat_FieldName[i]>>m_SStat_Table.NStat_Item_Begin[i]
  		      >>m_SStat_Table.NStat_Item_End[i]>>m_SStat_Table.Predfval_Or_Statid[i]
  		      >>m_SStat_Table.NStat_Item_IndexInField[i]>>m_SStat_Table.chStat_Item_SprInField[i]
  		      >>m_SStat_Table.ChCount_Item_Unit[i];
  		m_SStat_Table.IsIndex[i]='Y';
  		DeleteSpace(m_SStat_Table.SzStat_Item[i]);
  		DeleteSpace(m_SStat_Table.Predfval_Or_Statid[i]);
  		DeleteSpace(m_SStat_Table.ChCount_Item_Unit[i]);
  		if(m_SStat_Table.NStat_Item_Begin[i]<1 && m_SStat_Table.NStat_Item_Begin[i]!=-1)
  		  	{
  		  	sprintf(errmsg,"%d:%s Field_Begin error:%d",i,m_SStat_Table.SzStat_Item[i],m_SStat_Table.NStat_Item_Begin[i]);
  			errLog(LEVEL_ERROR, "",  ERR_GET_SUBSTR ,errmsg,__FILE__,__LINE__);
  			throw CException(ERR_GET_SUBSTR,errmsg,__FILE__,__LINE__);
  		  	}
  		DEBUG_LOG<<m_SStat_Table.SzStat_Item[i]<<"|"<<m_SStat_Table.NField_Index[i]<<endd;
  	  	}
  	 }
    theJSLog<<m_SStat_Table.table_name_key_count<<"个表名关键字"<<endi;
    //再查询表结构
    char isTableMainKey;
    i=m_SStat_Table.table_name_key_count;
  
    sql = "select Table_Item,Item_Type,Field_Name,Field_Begin,Field_End,Defv_Or_Func"
      ",nvl(indexinfield,-1),nvl(sprinfield,' '),nvl(unit,' ') ,IS_TABLE_NAME_KEY ,DATA_TYPE,IS_INDEX "
  //    "from C_STAT_TABLE_FMT where config_id=:config_id and IS_TABLE_NAME_KEY!='Y' ");
  //edit by weiyk 20100710
      "from C_STAT_TABLE_FMT where config_id=:config_id and (IS_TABLE_NAME_KEY!='Y' or IS_TABLE_NAME_KEY is null) order by seq";
     stmt.setSQLString(sql);		
     stmt<<config_id;
		 stmt.execute(); 

    for( ;i<m_SStat_Table.NStat_Item_Count;i++)
    {
      stmt>>m_SStat_Table.SzStat_Item[i]>>m_SStat_Table.NItem_Type[i]
        >>m_SStat_Table.SzStat_FieldName[i]>>m_SStat_Table.NStat_Item_Begin[i]
        >>m_SStat_Table.NStat_Item_End[i]>>m_SStat_Table.Predfval_Or_Statid[i]
        >>m_SStat_Table.NStat_Item_IndexInField[i]>>m_SStat_Table.chStat_Item_SprInField[i]
        >>m_SStat_Table.ChCount_Item_Unit[i]      
        >>isTableMainKey>>m_SStat_Table.NField_dataType[i]
        >>m_SStat_Table.IsIndex[i];
     // if(isTableMainKey=='Y')
     // 	{
     //		continue;
     // 	}
    //  if(m_SStat_Table.IsIndex[i]!='Y')
  		m_SStat_Table.IsIndex[i]='N';
  	
      DeleteSpace(m_SStat_Table.SzStat_Item[i]);
      DeleteSpace(m_SStat_Table.Predfval_Or_Statid[i]);
      DeleteSpace(m_SStat_Table.ChCount_Item_Unit[i]);
      if(m_SStat_Table.NStat_Item_Begin[i]<1 && m_SStat_Table.NStat_Item_Begin[i]!=-1)
    	{
      	sprintf(errmsg,"%d:%s Field_Begin error:%d",i,m_SStat_Table.SzStat_Item[i],m_SStat_Table.NStat_Item_Begin[i]);
    	  errLog(LEVEL_ERROR, "",  ERR_GET_SUBSTR ,errmsg,__FILE__,__LINE__);
  	    throw CException(ERR_GET_SUBSTR,errmsg,__FILE__,__LINE__);
    	}  	
  	    DEBUG_LOG<<m_SStat_Table.SzStat_Item[i]<<"|"<<m_SStat_Table.NField_Index[i]<<endd; 
    }
     /************************************************************************************/
    /*                   按条件统计功能改用表达式实现                   					*/
    /*************************************************************************************/
    	for(int item=0;item<m_SStat_Table.NStat_Item_Count;item++)
    		{
    		m_SStat_Table.item_cond_index[item]=-1;
    		}
      sql = "select CONDICTION from C_STAT_TABLE_DEFINE where CONFIG_ID=:id";
      stmt.setSQLString(sql);		
      stmt<<config_id;
		  stmt.execute();
    	stmt>>m_SStat_Table.ChCondiction;

    	DeleteSpace(m_SStat_Table.ChCondiction);    	
    	if(strlen(m_SStat_Table.ChCondiction)!=0)
    		{
    		if(Read_Cond_field()==-1)
    			{
    			sprintf(tmpstr,"error expression in C_STAT_TABLE_DEFINE!");
    			errLog(LEVEL_ERROR, "",  ERR_EXPRESS ,tmpstr,__FILE__,__LINE__);
    	          	throw CException(ERR_EXPRESS,tmpstr,__FILE__,__LINE__);
    		      	return (-1);
    	        	}  
    		char temp[250];
    	      	sprintf(temp,"  ");
    	      	strcat(temp,m_SStat_Table.ChCondiction);	      
    	      	strcpy(m_SStat_Table.ChCondiction,temp);		
    		int field;
    		for( field=0;field<m_SStat_Table.cond_field_count;field++)
    			{
    			if(!theCompile.DefineVariable(m_SStat_Table.cond_field[field].szParaName,
    					m_SStat_Table.cond_field_value[field].szField_Value))
    	    			{
    	    			sprintf(tmpstr,"Define Variable %s error.",m_SStat_Table.cond_field[field].szParaName);
    				errLog(LEVEL_ERROR, "",  ERR_EXPRESS ,tmpstr,__FILE__,__LINE__);
    	    			throw CException(ERR_EXPRESS,tmpstr,__FILE__,__LINE__);
    	    			return -1;
    	    			} 	
    		    }
    		}   
    
	
	 }else{
	 	   cout<<"connect error."<<endl;
	 	   return -1;
	 }
	     conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "统计CF_CStat::Init 出错" << endi;
  		throw jsexcp::CException(0, "统计CF_CStat::Init 出错", __FILE__, __LINE__);
  		conn.close();
  		return -1;
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
	m_SStat_Table.cond_field=new COND_PARA[m_SStat_Table.cond_field_count];
	m_SStat_Table.cond_field_value=new FIELD_VALUE[m_SStat_Table.cond_field_count];
	if(m_SStat_Table.cond_field==NULL || m_SStat_Table.cond_field_value==NULL)
    	{
    	errLog(LEVEL_ERROR, "",  ERR_REQU_MEM ,"new err.",__FILE__,__LINE__);
    	throw CException(ERR_REQU_MEM,(char *)"new err.",__FILE__,__LINE__);
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
			if(strcmp(tmp_field,m_SStat_Table.cond_field[var].szParaName)==0)
				{
				is_exist=1;
				break;				
				}
			}
		if(is_exist==0)
			{
			strcpy(m_SStat_Table.cond_field[i].szParaName,tmp_field);
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

int CF_CStat::dealRedoRec(CFmt_Change &inrcd,  char *szError_Type)
{
  //theJSLog<<"dealRedoRec---------------"<<endi;
//2005621 
//Exch_Field(InRecord);
  int j;
  //第一次读话单
  if(first_time_read_record==1)
	{
	//去得话单字段名对应的话单字段序号
	int item;
	for(item=0;item<m_SStat_Table.NStat_Item_Count;item++)
		{
		if(strcmp(m_SStat_Table.SzStat_FieldName[item],"0")==0 || strcmp(m_SStat_Table.SzStat_FieldName[item],"-1")==0)
			m_SStat_Table.NField_Index[item]=atoi(m_SStat_Table.SzStat_FieldName[item]);
		else
			m_SStat_Table.NField_Index[item]=inrcd.Get_FieldIndex(m_SStat_Table.SzStat_FieldName[item]);
		}
	
	//区分表达式里变量来自话单或表字段
	 for(j=0;j<m_SStat_Table.cond_field_count;j++)
	  	{
	  	theJSLog<<j<<endd;
	  	m_SStat_Table.cond_field[j].index=inrcd.Get_FieldIndex(m_SStat_Table.cond_field[j].szParaName);
		theJSLog<<"m_SStat_Table.cond_field[j].index="<<m_SStat_Table.cond_field[j].index<<endd;
		theJSLog<<"m_SStat_Table.cond_field[j].szParaName="<<m_SStat_Table.cond_field[j].szParaName<<endd;
		if(m_SStat_Table.cond_field[j].index==-1)
			{
			//表字段
			for(item=0;item<m_SStat_Table.NStat_Item_Count;item++)
				{
//				theJSLog<<"m_SStat_Table.SzStat_Item[item]="<<m_SStat_Table.SzStat_Item[item]<<endd;
				if(strcmp(m_SStat_Table.SzStat_Item[item],m_SStat_Table.cond_field[j].szParaName)==0)
					{
					//统计表的第item个字段对应条件表达式中的第j个变量
					m_SStat_Table.item_cond_index[item]=j;
//					theJSLog<<"m_SStat_Table.item_cond_index["<<item<<"]="<<m_SStat_Table.item_cond_index[item]<<endd;
					break;
					}
				}
			}
		else
			{
			//话单字段
			cond_variable_in_record_count++;
			}
	  	}
	first_time_read_record=0;
	}
//  theJSLog<<"cond_variable_in_record_count="<<cond_variable_in_record_count<<endi;
  //按条件统计，用表达式实现
  //不满足条件则不统计  		
  char TmpLogMsg[400];
  for(j=0;j<m_SStat_Table.cond_field_count;j++)
  	{
  	if(m_SStat_Table.cond_field[j].index!=-1)
  		{
  		strcpy(m_SStat_Table.cond_field_value[j].szField_Value,inrcd.Get_Field(m_SStat_Table.cond_field[j].index)); 
		theJSLog<<m_SStat_Table.cond_field[j].index<<"="<<m_SStat_Table.cond_field_value[j].szField_Value<<endd;
  		} 
  	}  
  if(m_SStat_Table.cond_field_count>0 && cond_variable_in_record_count==m_SStat_Table.cond_field_count)
	{
  	char Result[255] = "";
  	int ErrorNo=0;
  	char curr_condiction[250];
  	//sprintf(curr_condiction,m_SStat_Table.ChCondiction);
//  	theJSLog<<"Result="<<Result<<endd;
//  	theJSLog<<"m_SStat_Table.ChCondiction="<<m_SStat_Table.ChCondiction<<endd;
//  	theJSLog<<"ErrorNo="<<ErrorNo<<endd;
  	theCompile.Operation(Result, sizeof(Result)-1, &ErrorNo, m_SStat_Table.ChCondiction);
//  	theJSLog<<"ErrorNo="<<ErrorNo<<endd;
//  	theJSLog<<"Result="<<Result<<endd;
  	if(ErrorNo==0 && strcmp(Result,"false")==0)
  		{
  		DEBUG_LOG<<"不统计"<<endi;
  		return 0;
  		}
  	else if(ErrorNo!=0)
  		{
  		sprintf(TmpLogMsg,"error condiction expression.ErrorNo[%d]",ErrorNo);
		 errLog(LEVEL_ERROR, "",  ERR_EXPRESS ,TmpLogMsg,__FILE__,__LINE__);
  		throw CException(ERR_EXPRESS,TmpLogMsg,__FILE__,__LINE__);
  		}
	}
//theJSLog<<"to stat ..."<<endd;
char tmp_item_contain[STAT_ITEM_CONTAINER_LEN];
/////////////////////////////////////////////////////////////////////////////////////
//                    统计结果输出到文件
/////////////////////////////////////////////////////////////////////////////////////
if(Result_Output_Des=='F' && m_SStat_Table.CHStat_Mode=='I')
{
  	if(Stat_Record_Insert(inrcd,szError_Type)) return 0;  	
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
  	    theJSLog<<"Temp_Out_File.writeRec(Out_Record) ok!!"<<endd;
  	    } 	
}

/////////////////////////////////////////////////////////////////////////////////////
//                    统计结果输出到表
/////////////////////////////////////////////////////////////////////////////////////
else
{
  //update模式，临时表模式
  if (m_SStat_Table.CHStat_Mode=='U' || m_SStat_Table.CHStat_Mode=='T'|| m_SStat_Table.CHStat_Mode=='M')
  	{
   	  //20080604
  	  //if(Stat_Record_Row(inrcd,IError_Type)) 
 	  //theJSLog<<"Stat_Record_Update(inrcd,szError_Type);	"<<endi;
  	  Stat_Record_Update(inrcd,szError_Type);	
	  return 0;	
  	  }
  //insert 模式，累积到一定数量，做批量插入操作
  else
  	{
  	//横表插入  	
//  	DEBUG_LOG<<"Stat_Record_Insert(inrcd,szError_Type);	"<<endi;
  	  if(Stat_Record_Insert(inrcd,szError_Type)) 
	  	return 0;  	  
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
  		//theJSLog<<"批量插入"<<endl;
  		if(Insert_Stat())
	  		{
			return (-1);
	  		}
		m_NRecord_Count=0;	
	    	}
  	   return 0;
  	}//insert
  }//table
  theJSLog<<"dealRedoRec ok!!"<<endd;
}
  


//设置结果输出文件名、临时文件名,并打开临时输出文件
void CF_CStat::Set_TempOutFile()
{	  	
 if(Result_Output_Des=='F')
 	{
 	if(m_SStat_Table.ChStat_Type=='-')
 		{
		sprintf(Temp_Out_File_Name,"%s%s.%d.undo.tmp",
				  Result_File_Path,m_SzFileName,config_id);
		sprintf(Out_File_Name,"%s%s.%d.undo",
			  	Result_File_Path,m_SzFileName,config_id);
		sprintf(Out_File_Name_nopath,"%s.%d.undo",
			  	m_SzFileName,config_id);
 		}
	else
		{
		sprintf(Temp_Out_File_Name,"%s%s.%d.tmp",
				  Result_File_Path,m_SzFileName,config_id);
		sprintf(Out_File_Name,"%s%s.%d",
			  	Result_File_Path,m_SzFileName,config_id);
		sprintf(Out_File_Name_nopath,"%s.%d",
			  	m_SzFileName,config_id);
		}
	theJSLog<<"Temp_Out_File_Name:"<<Temp_Out_File_Name<<endi;
	theJSLog<<"Out_File_Name:"<<Out_File_Name<<endi;
	Temp_Out_File.Open(Temp_Out_File_Name);
	
 	}
}


void CF_CStat::Close_Temp_Outfile()
{
	theJSLog<<"Temp_Out_File_Name="<<Temp_Out_File_Name<<endi;
	DBConnection conn;//数据库连接
	if(Result_Output_Des=='F' && strcmp(Temp_Out_File_Name,"-1")!=0)
	{ 
    	Temp_Out_File.Close();
	if(middRcdCount!=0)
		{
		v_tmpOutFileName.push_back(Temp_Out_File_Name);
		v_outFileName.push_back(Out_File_Name);
		strcpy(Temp_Out_File_Name,"-1");
		try
			{
				if (dbConnect(conn))
    	 {
    			Statement stmt = conn.createStatement();
    			char dealtime[16];	    	
    		    	getCurTime(dealtime); 
    			char sql[200];
    			strcpy(sql,"insert into C_STAT_FILE_REG(CONFIG_ID,SOURCE_ID,FILE_NAME,OUT_PATH,STAT_COUNT,OUT_COUNT,INSERT_TIME,DEAL_FLAG) values(:v1,:v2,:v3,:v4,:v5,:v6,:v7,:v8)");
    			stmt.setSQLString(sql);
    			stmt <<config_id<<m_SzSource_ID<<Out_File_Name_nopath<<Result_File_Path<<statRcdCount<<middRcdCount<<dealtime <<'W';		
    			stmt.execute();
    			conn.commit();
	
    	 }else{
    	 	   cout<<"connect error."<<endl;
    	 	   //return false;
    	 }
    	     conn.close();		
			}
		catch (CException e)
		      {
		      //20070419,增强了错误信息输出
		      char szLogStr[700];
		      char DBerrmsg[500];
		      if(strlen(e.GetErrMessage())<499)
		      		{
		      		strcpy(DBerrmsg,e.GetErrMessage());
		      		}
		      else
		      		{
			      strncpy(DBerrmsg,e.GetErrMessage(),499);
			      DBerrmsg[499]=0;
		      		}
		      sprintf(szLogStr,"登记文件到表C_STAT_FILE_REG失败. ErrorMessage:%s",DBerrmsg);
			errLog(LEVEL_ERROR, "", ERR_INSERT_TABLE,szLogStr ,__FILE__,__LINE__);
		      throw CException(ERR_INSERT_TABLE,szLogStr,__FILE__,__LINE__);
		      }
	        catch(...)
			{
			 char szLogStr[100];	
			strcpy(szLogStr,"insert C_STAT_FILE_REG fail, unknow error!!! ");
			errLog(LEVEL_ERROR, "", ERR_INSERT_TABLE,szLogStr ,__FILE__,__LINE__);
			   throw CException(ERR_INSERT_TABLE,szLogStr,__FILE__,__LINE__);
			}
		}
	else
		{
		unlink(Temp_Out_File_Name);
		}
	middRcdCount=0;
	statRcdCount=0;
	}
}

//删除临时文件
void CF_CStat::Unlink_TemFile()
{
	if(Result_Output_Des=='F')
	{ 
		int fileCount=v_tmpOutFileName.size();
		theJSLog<<"unlike tmp fileCount="<<fileCount<<endi;
		for(int i=0;i<fileCount;i++)
			unlink(v_tmpOutFileName[i].c_str());
	}
}
void CF_CStat::Unlink_File()
{
	if(Result_Output_Des=='F')
	{
		int fileCount=v_outFileName.size();
		theJSLog<<"unlike fileCount="<<fileCount<<endi;
		for(int i=0;i<fileCount;i++)
			unlink(v_outFileName[i].c_str());
	}
}

//将临时文件更名为正式文件
int CF_CStat::Rename_AllOutFile()
{
	if(Result_Output_Des=='F')
		{
		int fileCount=v_outFileName.size();
		theJSLog<<"rename fileCount="<<fileCount<<endi;
		for(int i=0;i<fileCount;i++)
			{
			theJSLog<<"tmp:"<<v_tmpOutFileName[i].c_str()<<endi;
			theJSLog<<"end:"<<v_outFileName[i].c_str()<<endi;
			if(rename(v_tmpOutFileName[i].c_str(),v_outFileName[i].c_str()))
				{
				char szLogStr[200];
				sprintf(szLogStr,"rename %s to %s error",v_tmpOutFileName[i].c_str(),v_outFileName[i].c_str());
				errLog(LEVEL_ERROR, "", ERR_RENAME,szLogStr ,__FILE__,__LINE__);
      				//throw CException(ERR_RENAME,szLogStr,__FILE__,__LINE__);
      				exit(1);
				}
			}
		}
	v_tmpOutFileName.clear();
	v_outFileName.clear();
	return 0;
}
//20070509,由于效率问题，修改该接口，sourceid和dealstarttime尽可能从外部传入
int CF_CStat::setFileName(char *FileName,char *sourceid,char *dealstarttime)
{

	sprintf(m_SzFileName,"%s",FileName);
	sprintf(m_SzSource_ID,"%s",sourceid);
	sprintf(m_SzStat_Time,"%s",dealstarttime);

    return 0;
}
/*
//20070509,sourceid从外部传入
int CF_CStat::setFileName(char *FileName,char *sourceid)
{
	CBindSQL ds( DBConn);
	char sqltmp[400];
	char tmpstr[400];
	sprintf(m_SzFileName,"%s",FileName);
	sprintf(m_SzSource_ID,"%s",sourceid);

//----------------------------------------------------------------------------------------------------
//处理时间：为了冲销，现在是从输入控制表里取的，3.0从哪里取
//----------------------------------------------------------------------------------------------------
	memset(m_SzStat_Time,0,STAT_ITEM_NAME_LEN);
	//update:20060713,+、-统计m_SzStat_Time 统一用输入控制表dealstarttime
	sprintf(sqltmp,"select dealstarttime from %s where source_id='%s' and filename='%s'",
	  m_SzSchedule,sourceid,FileName);
	ds.Open(sqltmp);
	ds>>m_SzStat_Time;
	if (ds.IsEnd()) 
	{
		  sprintf(tmpstr,"err:%s!",sqltmp);
		  errLog(LEVEL_ERROR, "",  ERR_SELECT ,tmpstr,__FILE__,__LINE__);
	  	  throw CException(ERR_SELECT,tmpstr,__FILE__,__LINE__);
	}
	ds.Close();	
	DeleteSpace(m_SzStat_Time);

    return 0;
}
*/



//横表读话单，生成一条记录，添加通话时长统计
int CF_CStat::Stat_Record_Insert(CFmt_Change &InRecord , char *szError_Type)
{
  char TmpLogMsg[400];
  char crrFieldValue[100];
  crrFieldValue[0]=0;
  
  //开始统计
  for(int i=0;i<m_SStat_Table.NStat_Item_Count;i++)
  {
    //取字段值
    m_PsPre_Stat_Record[m_NRecord_Count].NItem_Type[i] 
	  = m_SStat_Table.NItem_Type[i];
    m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i][0]=0;
	//theJSLog<<"字段值 "<<m_SStat_Table.NItem_Type[i]<<endi;
    crrFieldValue[0]=0;
    if(m_SStat_Table.NField_Index[i]!=-1 && m_SStat_Table.NField_Index[i]!=0)
	{
	strcpy(crrFieldValue,InRecord.Get_Field(m_SStat_Table.NField_Index[i])); 
	}
	
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
		errLog(LEVEL_ERROR, "",  ERR_FIELD_NULL ,TmpLogMsg,__FILE__,__LINE__);
		throw CException (ERR_FIELD_NULL,TmpLogMsg,__FILE__,__LINE__);
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
							errLog(LEVEL_ERROR, "",  ERR_FIELD_INDEX ,TmpLogMsg,__FILE__,__LINE__);
		             	 			throw CException(ERR_FIELD_INDEX,TmpLogMsg,__FILE__,__LINE__);
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
						 errLog(LEVEL_ERROR, "",  ERR_GET_SUBSTR ,TmpLogMsg,__FILE__,__LINE__);
						throw CException(ERR_GET_SUBSTR,TmpLogMsg,__FILE__,__LINE__);
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
		 errLog(LEVEL_ERROR, "",  ERR_FIELD_NULL ,TmpLogMsg,__FILE__,__LINE__);
     	 	throw CException (ERR_FIELD_NULL,TmpLogMsg,__FILE__,__LINE__);
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
  	sprintf(m_PsPre_Stat_Record[m_NRecord_Count].SzStat_Item[i],"%s",szError_Type);
  	}
  else if(m_SStat_Table.NItem_Type[i]==KEYWORK_DAYCYCLE)
  	{
  	char cdrBeginTime[15];
  	sprintf(cdrBeginTime,crrFieldValue);
	
	int NField_Len = strlen(cdrBeginTime);
	if((NField_Len<m_SStat_Table.NStat_Item_End[i])||
	  (m_SStat_Table.NStat_Item_End[i]<m_SStat_Table.NStat_Item_Begin[i]))
		{
		sprintf(TmpLogMsg,"Field %s Field_Len is not long enough.",m_SStat_Table.SzStat_Item[i]);
		errLog(LEVEL_ERROR, "",  ERR_GET_SUBSTR ,TmpLogMsg,__FILE__,__LINE__);
		throw CException(ERR_GET_SUBSTR,TmpLogMsg,__FILE__,__LINE__);
		}
	char keyValue[20];
	NField_Len=m_SStat_Table.NStat_Item_End[i]-m_SStat_Table.NStat_Item_Begin[i]+1;
	sprintf(keyValue,"%s",cdrBeginTime+m_SStat_Table.NStat_Item_Begin[i]-1);
	keyValue[NField_Len]=0;
  	
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
		errLog(LEVEL_ERROR, "",  ERR_EXPRESS ,TmpLogMsg,__FILE__,__LINE__);
  		throw CException(ERR_EXPRESS,TmpLogMsg,__FILE__,__LINE__);
  		}
	}
  
  if(m_SStat_Table.ChIs_Count=='Y')
  	 m_PsPre_Stat_Record[m_NRecord_Count].NBill_Count=1;
  m_PsPre_Stat_Record[m_NRecord_Count].NStat_Item_Count=m_SStat_Table.NStat_Item_Count;
  G_CF_CStat_UpdateCount++;
  if(G_CF_CStat_UpdateCount%500==0)
  	{
		sprintf(TmpLogMsg,"total update count = %d",G_CF_CStat_UpdateCount);
		runLog(LEVEL_DEBUG, TmpLogMsg);
  	}
	 	
  return 0;	

}

int CF_CStat::Stat_Record_Update(CFmt_Change &InRecord, char *szError_Type)
{
  char TmpLogMsg[400];
   STAT_RECORD  crrRecord;

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
    	{
    	strcpy(crrFieldValue,InRecord.Get_Field(m_SStat_Table.NField_Index[i])); 
    	//theJSLog<<"字段值 "<<InRecord.Get_Field(m_SStat_Table.NField_Index[i])<<endi;
    	}
    
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
		errLog(LEVEL_ERROR, "",  ERR_FIELD_NULL ,TmpLogMsg,__FILE__,__LINE__);
		throw CException (ERR_FIELD_NULL,TmpLogMsg,__FILE__,__LINE__);
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
						errLog(LEVEL_ERROR, "",  ERR_FIELD_INDEX ,TmpLogMsg,__FILE__,__LINE__);
						throw CException(ERR_FIELD_INDEX,TmpLogMsg,__FILE__,__LINE__);
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
					errLog(LEVEL_ERROR, "",  ERR_GET_SUBSTR ,TmpLogMsg,__FILE__,__LINE__);
          			throw CException(ERR_GET_SUBSTR,TmpLogMsg,__FILE__,__LINE__);
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
		 errLog(LEVEL_ERROR, "",  ERR_FIELD_NULL ,TmpLogMsg,__FILE__,__LINE__);
     	 	throw CException (ERR_FIELD_NULL,TmpLogMsg,__FILE__,__LINE__);
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
  	sprintf(crrRecord.SzStat_Item[i],"%s",szError_Type);
  	}
  else if(m_SStat_Table.NItem_Type[i]==KEYWORK_DAYCYCLE)
  	{
  	char cdrBeginTime[15];
  	sprintf(cdrBeginTime,crrFieldValue);
	
	int NField_Len = strlen(cdrBeginTime);
	if((NField_Len<m_SStat_Table.NStat_Item_End[i])||
	  (m_SStat_Table.NStat_Item_End[i]<m_SStat_Table.NStat_Item_Begin[i]))
		{
		sprintf(TmpLogMsg,"Field %s Field_Len is not long enough.",m_SStat_Table.SzStat_Item[i]);
		errLog(LEVEL_ERROR, "",  ERR_GET_SUBSTR ,TmpLogMsg,__FILE__,__LINE__);
		throw CException(ERR_GET_SUBSTR,TmpLogMsg,__FILE__,__LINE__);
		}
	char keyValue[20];
	NField_Len=m_SStat_Table.NStat_Item_End[i]-m_SStat_Table.NStat_Item_Begin[i]+1;
	sprintf(keyValue,"%s",cdrBeginTime+m_SStat_Table.NStat_Item_Begin[i]-1);
	keyValue[NField_Len]=0;
  	
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
	//theJSLog<<i<<"="<<crrRecord.SzStat_Item[i]<<endd;
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
  		{
  		theJSLog<<"不统计"<<endi;
  		return 1;
  		}
  	else if(ErrorNo!=0)
  		{
  		sprintf(TmpLogMsg,"error condiction expression.ErrorNo[%d]",ErrorNo);
		errLog(LEVEL_ERROR, "",  ERR_EXPRESS ,TmpLogMsg,__FILE__,__LINE__);
  		throw CException(ERR_EXPRESS,TmpLogMsg,__FILE__,__LINE__);
  		}
	}
  
  if(m_SStat_Table.ChIs_Count=='Y')
  	 crrRecord.NBill_Count=1;
  crrRecord.NStat_Item_Count=m_SStat_Table.NStat_Item_Count;
  AddOneRecord(crrRecord);
  //theJSLog<<"add ok"<<endi;
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
		//theJSLog<<"it->second->NBill_Count="<<it->second->NBill_Count<<endi;
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
		//theJSLog<<"icount_toUpdate="<<icount_toUpdate<<endi;
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
				errLog(LEVEL_ERROR, "",  ERR_REQU_MEM ,"new err.",__FILE__,__LINE__);
				throw CException(ERR_REQU_MEM,(char *)"new err.",__FILE__,__LINE__);
				}
			memcpy(tmp, Stat_Record_toUpdate,icount_toUpdate*sizeof(STAT_RECORD));
			map<CRecordKey,STAT_RECORD*>::iterator it_change=m_key_record.begin();
			for(;it_change!=m_key_record.end();it_change++)
				{
				it_change->second=&tmp[m_key_seq[it_change->first]];
				//theJSLog<<"结果 "<<it_change->second<<endi;
				}
			delete[] Stat_Record_toUpdate;
        		Stat_Record_toUpdate = tmp;
			}
		
		G_CF_CStat_UpdateCount++;
		if(G_CF_CStat_UpdateCount%500==0)
			{
			char TmpLogMsg[100];
			sprintf(TmpLogMsg,"total update count = %d",G_CF_CStat_UpdateCount);
			theJSLog<<TmpLogMsg<<endi;
  			}
	 	} 
	 statRcdCount++;
     return 0;
}

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
	char msg[200];
	sprintf(msg, "Field_Len is not long enough:%s",Key_Container);
       throw CException(ERR_GET_SUBSTR,msg,__FILE__,__LINE__);
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

  default:
  	errLog(LEVEL_ERROR, "",  ERR_KEY_ITEM ,"Can not find Such Key Stat Item.",__FILE__,__LINE__);
  	throw CException(ERR_KEY_ITEM,"Can not find Such Key Stat Item.",__FILE__,__LINE__);
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
  strcat(sqltmp," for update");
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

/*merge into D_DXJS_BASE_RESULT p1
   using (select colume1,colume2 from dual) p2
   on (p1.filename =p2.filename)
   when matched then
   update 
   set p1.dealstarttime='2011121111'
   when not matched then
     insert (source_id,deal_flag,validflag) values (p2.source_id,p2.deal_flag,p2.validflag)
*/
int CF_CStat::Make_Merge_Sql(char *sqltmp,const char *table_name)
{
    char c_item[20];
	int i,flag=0;
	sprintf(sqltmp,"merge into ",sqltmp);
    sprintf(sqltmp,"%s%s p1 using (select ",sqltmp,table_name);
	for(i=m_SStat_Table.table_name_key_count;i<m_SStat_Table.NStat_Item_Count;i++)
	  {
	    sprintf(c_item,":%d ",flag);
 	    strcat(sqltmp,c_item);	   
 	    flag++;
 	    strcat(sqltmp,m_SStat_Table.SzStat_Item[i]);
 	    strcat(sqltmp,", ");      
	   }
 	 if(m_SStat_Table.ChIs_Count=='Y')
	  {
	    sprintf(sqltmp,"%s :%d ",sqltmp,flag);
    	strcat(sqltmp,m_SStat_Table.SzCount_Item_Name);
    	sqltmp[strlen(sqltmp)]  = 0;
	  }
 	 else
	  {
      	sqltmp[strlen(sqltmp)-1] = 0;
	  } 
 	 
 	  sprintf(sqltmp,"%s from dual) p2 on ",sqltmp);
 	  sprintf(sqltmp,"%s (",sqltmp);
	  int colum=m_SStat_Table.NStat_Item_Count;
	  if(colum!=0)
	  	{
	  	//theJSLog<<"colum="<<colum<<endi;
	  	for(int item=m_SStat_Table.table_name_key_count;item<colum;item++)
  		{ 
  		//theJSLog<<"item="<<item<<endi;
  		if(m_SStat_Table.NItem_Type[item]!=STAT_N_COUNT_TYPE
			&& m_SStat_Table.NItem_Type[item]!=STAT_D_COUNT_TYPE)
  			{
  			sprintf(sqltmp,"%s p1.",sqltmp);   //添加两个别名p1 p2
  			strcat(sqltmp,m_SStat_Table.SzStat_Item[item]);
  			strcat(sqltmp,"=");
  			sprintf(sqltmp,"%s p2.",sqltmp);   //添加两个别名p1 p2
  			strcat(sqltmp,m_SStat_Table.SzStat_Item[item]);
  			sprintf(sqltmp,"%s and ",sqltmp);
  			}
  	 	if(m_SStat_Table.NItem_Type[item]==KEYWORK_UPDATETIME)
  			strcat(sqltmp,c_item);
  			
  	     } 	
	  	   sqltmp[strlen(sqltmp)-5] = 0;
	  	   
	  	}
	  else
	  	{
	  	  sprintf(sqltmp,"%s 1=0 ",sqltmp);
	  	}
 	 	
  	 flag=i+1; 
 	 sprintf(sqltmp,"%s ) when matched then update set ",sqltmp);   //update条件
	 for(int j=m_SStat_Table.table_name_key_count;j<m_SStat_Table.NStat_Item_Count;j++)
	  {	  
	  if(m_SStat_Table.NItem_Type[j]==STAT_N_COUNT_TYPE
	  	|| m_SStat_Table.NItem_Type[j]==STAT_D_COUNT_TYPE)
	    {
	     strcat(sqltmp,m_SStat_Table.SzStat_Item[j]);
	     strcat(sqltmp,"=p1.");
	     strcat(sqltmp,m_SStat_Table.SzStat_Item[j]);
	     strcat(sqltmp,"+");
	     sprintf(c_item,":%d,",flag);  //按顺序递增变量
 	     strcat(sqltmp,c_item);	  
 	     flag++;
	    }
	  }

	 //统计项
	  if(m_SStat_Table.ChIs_Count=='Y')
	  {
	  	 strcat(sqltmp,m_SStat_Table.SzCount_Item_Name);
	     strcat(sqltmp,"=p1.");
	     strcat(sqltmp,m_SStat_Table.SzCount_Item_Name);
	     strcat(sqltmp,"+");
	     sprintf(c_item,":%d,",flag);  //按顺序递增变量
 	     strcat(sqltmp,c_item);	  
 	     flag++;
	  } 
	sqltmp[strlen(sqltmp)-1] = 0;
	sprintf(sqltmp,"%s when not matched then insert (",sqltmp);   //insert条件
	for(i=m_SStat_Table.table_name_key_count;i<m_SStat_Table.NStat_Item_Count;i++)
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
	sprintf(sqltmp,"%s ) values ( ",sqltmp);
	
	if(m_SStat_Table.ChIs_Count=='Y')  
  		colum++;
	
    for(i=m_SStat_Table.table_name_key_count;i<m_SStat_Table.NStat_Item_Count;i++)
  		{ 
  			sprintf(sqltmp,"%s p2.",sqltmp);   //添加两个别名p1 p2
  			strcat(sqltmp,m_SStat_Table.SzStat_Item[i]);
  			sprintf(sqltmp,"%s ,",sqltmp);
  	 		if(m_SStat_Table.NItem_Type[i]==KEYWORK_UPDATETIME)
  			strcat(sqltmp,c_item);
  		}
 	 if(m_SStat_Table.ChIs_Count=='Y')
	  {
	    sprintf(sqltmp,"%s p2.",sqltmp);
    	strcat(sqltmp,m_SStat_Table.SzCount_Item_Name);
    	sqltmp[strlen(sqltmp)]  = 0;
	  }
 	 else
	  {
      	sqltmp[strlen(sqltmp)-1] = 0;
	  } 
 	 sprintf(sqltmp,"%s )",sqltmp);
	theJSLog<<"输出sql语句 "<<sqltmp<<endi;
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
	//theJSLog<<"输出sql语句 "<<sqltmp<<endi;
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
  	//theJSLog<<"输出sql语句 "<<sqltmp<<endi;
	return 0;
}

void CF_CStat::Create_Table(char *table_name)
{
	//CBindSQL ds(DBConn);
	DBConnection conn;//数据库连接
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
		if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			stmt.setSQLString(sqlCreateTable);		
			stmt.execute();
			conn.commit();		
			//cout<<"sqlCreateIndex:-----"<<endl<<sqlCreateIndex<<endl;
  		if(index_field_count!=0 && m_SStat_Table.CHStat_Mode=='U')
  			{
  				stmt.setSQLString(sqlCreateIndex);		
			    stmt.execute();
			    conn.commit();
  			}
  		//cout<<"sqlPrimaryKey:-----"<<endl<<sqlPrimaryKey<<endl;
  		if(m_SStat_Table.CHStat_Mode=='U')
  			{
  				stmt.setSQLString(sqlPrimaryKey);		
			    stmt.execute();
			    conn.commit();
  			}
	
	 }else{
	 	   cout<<"connect error."<<endl;
	 	   //return false;
	 }
	     conn.close();	     		
	}
	catch (SQLException e)
	{
		  cout<<e.what()<<endl;
  		theJSLog << "Create_Table 出错" << endi;
  		throw jsexcp::CException(0, "Create_Table 出错", __FILE__, __LINE__);
  		conn.close();
		  //cout<<"Error number: "<<  ex.GetErrorCode() << endl;
		  //cout<<"Error Message: "<<ex.GetErrorMsg() << endl;    
//		errLog(LEVEL_ERROR, "",  ex.GetErrorCode() ,ex.GetErrorMsg().c_str() ,__FILE__,__LINE__);
		  //throw(ex);
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
  theJSLog<<"int Prepare_Rec ================="<<endi;
	
  //先算出涉及多少个表，每个表有多少条记录，不存在的表先建好
   int cnt=0;
  
  map_table_recordCount.clear();
  map_rcdSeq_tableName.clear();
  
  map<string,int>::iterator it_count;
  char tmp_table_name[STAT_ITEM_CONTAINER_LEN] ;
  string str_tmp_table_name;
  

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
  if(m_SStat_Table.CHStat_Mode=='U')
  	Prepare_Rec_Update();
  else if(m_SStat_Table.CHStat_Mode=='T')
  	Prepare_Rec_TmpStat();
  else if(m_SStat_Table.CHStat_Mode=='M')
  	Prepare_Rec_Merge();
  	//Prepare_Rec_Update();

   theJSLog<<"Prepare_Rec ok================="<<endi;
  /*
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
					      errLog(LEVEL_ERROR, "", ERR_SELECT_UNDO,sqltmp ,__FILE__,__LINE__);
			  		       throw CException(ERR_SELECT_UNDO,sqltmp,__FILE__,__LINE__);
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
				errLog(LEVEL_ERROR, "", ERR_SELECT_UNDO,sqltmp ,__FILE__,__LINE__);
				 throw CException(ERR_SELECT_UNDO,sqltmp,__FILE__,__LINE__);
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
		  	{
		  	errLog(LEVEL_ERROR, "", ERR_BILLCOUNT_NEGATIVE,"BILL COUNT IS NEGATIVE" ,__FILE__,__LINE__);
		    	throw CException(ERR_BILLCOUNT_NEGATIVE,"BILL COUNT IS NEGATIVE",__FILE__,__LINE__);
		  	}
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
  */
  return 0;		
}


int CF_CStat::Prepare_Rec_Merge()
{	
	//CBindSQL ds(DBConn);
	//DBConnection conn;//数据库连接
	char sqltmp[SQL_LENGTH];
	STAT_RECORD crrRcord;
	strcpy(crr_table_name,m_SStat_Table.SzTable_Name);
	theJSLog<<"Prepare_Rec_Merge:"<<crr_table_name<<endi;
	
	map<string,v_pRcd>::iterator it;
	for(  int cnt=0;cnt<icount_toUpdate;cnt++)
		{
		STAT_RECORD &UPdate_Stat_Record=Stat_Record_toUpdate[cnt];
		//如果是冲销，把度量变为负的
		if(m_SStat_Table.ChStat_Type=='-')
			{
			for(int item=m_SStat_Table.table_name_key_count;item<m_SStat_Table.NStat_Item_Count;item++)
			{	  		    
			if(m_SStat_Table.NItem_Type[item]==STAT_N_COUNT_TYPE)
				{
				long tmp=-atol(UPdate_Stat_Record.SzStat_Item[item]);
				sprintf(UPdate_Stat_Record.SzStat_Item[item],"%ld",tmp);
				}
			else if(m_SStat_Table.NItem_Type[item]==STAT_D_COUNT_TYPE)
				{
				double tmp=-atof(UPdate_Stat_Record.SzStat_Item[item]);
				sprintf(UPdate_Stat_Record.SzStat_Item[item],"%lf",tmp);
				}
			}
			if(m_SStat_Table.ChIs_Count=='Y')  
			 	UPdate_Stat_Record.NBill_Count=-UPdate_Stat_Record.NBill_Count;
			}

		if(m_SStat_Table.table_name_key_count!=0)
			strcpy(crr_table_name,map_rcdSeq_tableName[cnt].c_str());

		it= map_table_recordToMerge.find(crr_table_name);
		if(it==map_table_recordToMerge.end())
			{
			theJSLog<<"map_table_recordCount[crr_table_name]="<<map_table_recordCount[crr_table_name]<<endi;
			v_pRcd v_pToRecord(map_table_recordCount[crr_table_name],NULL);
			map_table_recordToMerge[crr_table_name]=v_pToRecord;
			map_table_recordToMergeCount[crr_table_name]=0;
			it= map_table_recordToMerge.find(crr_table_name);
			}
		int &currRrdCount=map_table_recordToMergeCount[crr_table_name];
		(it->second)[currRrdCount]=&UPdate_Stat_Record;
		currRrdCount++;	
		}
	map_rcdSeq_tableName.clear();
	map_table_recordCount.clear();
	return 0;	
}


int CF_CStat::Prepare_Rec_Update()
{
  theJSLog<<"Prepare_Rec_Update....."<<endi;
  //CBindSQL ds(DBConn);
  DBConnection conn;//数据库连接
  char sqltmp[SQL_LENGTH];
  STAT_RECORD crrRcord;
  strcpy(crr_table_name,m_SStat_Table.SzTable_Name);
  
  try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			 map<string,v_pRcd>::iterator it;
      for(  int cnt=0;cnt<icount_toUpdate;cnt++)
      	{
      	STAT_RECORD &UPdate_Stat_Record=Stat_Record_toUpdate[cnt];
    
    	if(m_SStat_Table.table_name_key_count!=0)
    	 	strcpy(crr_table_name,map_rcdSeq_tableName[cnt].c_str());
    	  Make_Select_Sql(UPdate_Stat_Record,sqltmp);
    	  stmt.setSQLString(sqltmp);		
		    //stmt.execute();

    	  int i;
    	  for( i=0;i<m_SStat_Table.NStat_Item_Count;i++)
    		  {
    		  if(m_SStat_Table.NItem_Type[i]==STAT_N_COUNT_TYPE
    		  	||m_SStat_Table.NItem_Type[i]==STAT_D_COUNT_TYPE)
    		    {
    		    //stmt>>crrRcord.SzStat_Item[i];
    		    if(!(stmt.execute()))
    			    {			    
    			        //找不到且为'-' 
    			     	  if(m_SStat_Table.ChStat_Type=='-')
    					      {
    					      errLog(LEVEL_ERROR, "", ERR_SELECT_UNDO,sqltmp ,__FILE__,__LINE__);
    			  		       throw CException(ERR_SELECT_UNDO,sqltmp,__FILE__,__LINE__);
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
    						//theJSLog<<"insert_num:"<<currRrdCount<<endi;
    					       break;
    					      }
    			    }
    			    stmt>>crrRcord.SzStat_Item[i];
    		    }		 
    		  }
    	    //没有找到相同条件记录，已经插入map_table_recordToInsert，处理下一条记录
    	    if(i!=m_SStat_Table.NStat_Item_Count)
    	  	      continue;
    
    	    //走到这里，找到相同条件记录或表只统计通话次数	
    	    //处理方法与上相似
    
    	    if(m_SStat_Table.ChIs_Count=='Y') 
    		  {
    		  //stmt>>crrRcord.NBill_Count;
    		  if(!(stmt.execute()))
    			  {			  
    			  //找不到且为-　
    		     	  if(m_SStat_Table.ChStat_Type=='-')
    				{
    				errLog(LEVEL_ERROR, "", ERR_SELECT_UNDO,sqltmp ,__FILE__,__LINE__);
    				 throw CException(ERR_SELECT_UNDO,sqltmp,__FILE__,__LINE__);
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
    				//theJSLog<<"insert_num:"<<currRrdCount<<endi;
    				//本条记录处理完毕	   
    				continue;  		
    				}  
    		  }
    		    stmt>>crrRcord.NBill_Count;
    		}
    	 //ds.Close();
    
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
    		  	{
    		  	errLog(LEVEL_ERROR, "", ERR_BILLCOUNT_NEGATIVE,"BILL COUNT IS NEGATIVE" ,__FILE__,__LINE__);
    		    	throw CException(ERR_BILLCOUNT_NEGATIVE,"BILL COUNT IS NEGATIVE",__FILE__,__LINE__);
    		  	}
    		  if(NTmp_Bill_Count==0) 
    			  {
    			  //做删除动作,然后返回
    			  memset(sqltmp,0,SQL_LENGTH);
    			  Make_Delete_Sql(UPdate_Stat_Record,sqltmp);
    			  stmt.setSQLString(sqltmp);		
			      stmt.execute();
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
    	//theJSLog<<"update_num:"<<currRrdCount<<endi;
    
      	}
		
	 }else{
	 	   cout<<"connect error."<<endl;
	 	   return -1;
	 }
	     conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "Prepare_Rec_Update 出错" << endi;
  		throw jsexcp::CException(0, "Prepare_Rec_Update 出错", __FILE__, __LINE__);
  		conn.close();
  		return -1;
  } 
  
  map_rcdSeq_tableName.clear();
  map_table_recordCount.clear();
  theJSLog<<"prepare record ok!!"<<endi;
  return 0;	
}

int CF_CStat::Prepare_Rec_TmpStat()
{
	
	//CBindSQL ds(DBConn);
	char sqltmp[SQL_LENGTH];
	STAT_RECORD crrRcord;
	strcpy(crr_table_name,m_SStat_Table.SzTable_Name);
	theJSLog<<"Prepare_Rec_TmpStat:"<<crr_table_name<<endi;
	
	map<string,v_pRcd>::iterator it;
	for(  int cnt=0;cnt<icount_toUpdate;cnt++)
		{
		STAT_RECORD &UPdate_Stat_Record=Stat_Record_toUpdate[cnt];
		//如果是冲销，把度量变为负的
		if(m_SStat_Table.ChStat_Type=='-')
			{
			for(int item=m_SStat_Table.table_name_key_count;item<m_SStat_Table.NStat_Item_Count;item++)
			{	  		    
			if(m_SStat_Table.NItem_Type[item]==STAT_N_COUNT_TYPE)
				{
				//theJSLog<<item<<"|"<<UPdate_Stat_Record.SzStat_Item[item]<<endi;
				long tmp=-atol(UPdate_Stat_Record.SzStat_Item[item]);
				sprintf(UPdate_Stat_Record.SzStat_Item[item],"%ld",tmp);
				//theJSLog<<item<<"|"<<UPdate_Stat_Record.SzStat_Item[item]<<endi;
				}
			else if(m_SStat_Table.NItem_Type[item]==STAT_D_COUNT_TYPE)
				{
				//theJSLog<<item<<"|"<<UPdate_Stat_Record.SzStat_Item[item]<<endi;
				double tmp=-atof(UPdate_Stat_Record.SzStat_Item[item]);
				sprintf(UPdate_Stat_Record.SzStat_Item[item],"%lf",tmp);
				//theJSLog<<item<<"|"<<UPdate_Stat_Record.SzStat_Item[item]<<endi;
				}
			}
			if(m_SStat_Table.ChIs_Count=='Y')  
			 	UPdate_Stat_Record.NBill_Count=-UPdate_Stat_Record.NBill_Count;
			}

		if(m_SStat_Table.table_name_key_count!=0)
			strcpy(crr_table_name,map_rcdSeq_tableName[cnt].c_str());

		it= map_table_recordToInsert.find(crr_table_name);
		if(it==map_table_recordToInsert.end())
			{
			theJSLog<<"map_table_recordCount[crr_table_name]="<<map_table_recordCount[crr_table_name]<<endi;
			v_pRcd v_pToRecord(map_table_recordCount[crr_table_name],NULL);
			map_table_recordToInsert[crr_table_name]=v_pToRecord;
			map_table_recordToInsertCount[crr_table_name]=0;
			it= map_table_recordToInsert.find(crr_table_name);
			}
		int &currRrdCount=map_table_recordToInsertCount[crr_table_name];
		(it->second)[currRrdCount]=&UPdate_Stat_Record;
		currRrdCount++;	
		//theJSLog<<"currRrdCount="<<currRrdCount<<endi;
		//theJSLog<<"map_table_recordToInsertCount[crr_table_name]="<<map_table_recordToInsertCount[crr_table_name]<<endi;
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
				cout<<"map_table_recordCount[str_tmp_table_name]="<<map_table_recordCount[str_tmp_table_name]<<endl;
				}
			else
				{
				map_tableName_rcdSeq[str_tmp_table_name].push_back(cnt);
				
				//map_table_recordCount[str_tmp_table_name]++;				
				cout<<"map_table_recordCount[str_tmp_table_name]="<<map_table_recordCount[str_tmp_table_name]<<endl;
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

	//CBindSQL ds( DBConn );
	DBConnection conn;//数据库连接
  try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			int i,j,seq_IN_m_PsPre_Stat_Record=0;
    	char sqltmp[SQL_LENGTH];	
    	for(it=map_tableName_rcdSeq.begin();it!=map_tableName_rcdSeq.end();it++)	
    	{
    		//Make_Insert_Sql(sqltmp,m_SStat_Table.SzTable_Name);
    		Make_Insert_Sql(sqltmp,it->first.c_str());	
    		int insert_num=STAT_INSERT_CONNT;	
    		stmt.setSQLString(sqltmp);	
			  //stmt.execute();
			  //conn.commit();
    		
    		//ds.Open(sqltmp,NONSELECT_DML);
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
    
    				//如果是冲销，把度量变为负的
    				if(m_SStat_Table.ChStat_Type=='-')
    					{
    					for(int item=m_SStat_Table.table_name_key_count;item<m_SStat_Table.NStat_Item_Count;item++)
    						{	  		    
    						if(m_SStat_Table.NItem_Type[item]==STAT_N_COUNT_TYPE)
    							{
    							sprintf(m_PsPre_Stat_Record[seq_IN_m_PsPre_Stat_Record].SzStat_Item[item],"-%s",
    								m_PsPre_Stat_Record[seq_IN_m_PsPre_Stat_Record].SzStat_Item[item]);
    							}
    						else if(m_SStat_Table.NItem_Type[item]==STAT_D_COUNT_TYPE)
    							{
    							sprintf(m_PsPre_Stat_Record[seq_IN_m_PsPre_Stat_Record].SzStat_Item[item],"-%s",
    								m_PsPre_Stat_Record[seq_IN_m_PsPre_Stat_Record].SzStat_Item[item]);
    							}
    						}
    				 	 if(m_SStat_Table.ChIs_Count=='Y')  
    				 	 	{
    				 		m_PsPre_Stat_Record[seq_IN_m_PsPre_Stat_Record].NBill_Count=-m_PsPre_Stat_Record[seq_IN_m_PsPre_Stat_Record].NBill_Count;
    				 	 	}
    					}
    				
    
    		  		for(int item=m_SStat_Table.table_name_key_count;item<m_SStat_Table.NStat_Item_Count;item++)
    		  		{
    		  			
    		  			DeleteSpace(m_PsPre_Stat_Record[seq_IN_m_PsPre_Stat_Record].SzStat_Item[item]);
    		  			if(m_SStat_Table.NItem_Type[item]==STAT_N_TYPE
    		  	  			|| m_SStat_Table.NItem_Type[item]==STAT_N_COUNT_TYPE)
    		  				stmt<<atol(m_PsPre_Stat_Record[seq_IN_m_PsPre_Stat_Record].SzStat_Item[item]);	
    					else if(m_SStat_Table.NItem_Type[item]==STAT_D_COUNT_TYPE)
    						stmt<<atof(m_PsPre_Stat_Record[seq_IN_m_PsPre_Stat_Record].SzStat_Item[item]);	
    		  			else
    		  				stmt<<m_PsPre_Stat_Record[seq_IN_m_PsPre_Stat_Record].SzStat_Item[item];
    					
    		  		}
    		  		if(m_SStat_Table.ChIs_Count=='Y')  
    		  			stmt<<m_PsPre_Stat_Record[seq_IN_m_PsPre_Stat_Record].NBill_Count; 	
    				
    			}
    			//ds.Execute();
    			stmt.execute();
    		   		 	
    		 	i=i+insert_num-1;
    		}

    	}			
	
	 }else{
	 	   cout<<"connect error."<<endl;
	 	   return -1;
	 }
	     conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "insert 模式统计中往表中插记录 出错" << endi;
  		throw jsexcp::CException(0, "insert 模式统计中往表中插记录 出错", __FILE__, __LINE__);
  		conn.close();
  		return -1;
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
	//CBindSQL ds( DBConn );
	DBConnection conn;//数据库连接
  try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			int i,j;
    	char sqltmp[SQL_LENGTH];
    	map<string,v_pRcd>::iterator it_table;
    		
    	for(it_table=map_table_recordToInsert.begin();it_table!=map_table_recordToInsert.end();it_table++)
    	{
    		//cout<<"insert to table:"<<it_table->first<<endl;
    		
    		Make_Insert_Sql(sqltmp,it_table->first.c_str());
    		//theJSLog<<"SQL "<<sqltmp<<endi;
    		int insert_num=STAT_INSERT_CONNT;
    		stmt.setSQLString(sqltmp);
			  //stmt << procID;			
			  //stmt.execute();
			  //stmt >> ext_param;
			
    		//ds.Open(sqltmp,NONSELECT_DML);
    		int icount_toInsert_table=map_table_recordToInsertCount[it_table->first];
    		theJSLog<<"插表:"<<it_table->first.c_str()<<":"<<icount_toInsert_table<<endi;
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
    		  				stmt<<atol((it_table->second)[i+j]->SzStat_Item[item]);
    					else if(m_SStat_Table.NItem_Type[item]==STAT_D_COUNT_TYPE)
    						stmt<<atof((it_table->second)[i+j]->SzStat_Item[item]);
    		  			else
    		  				stmt<<(it_table->second)[i+j]->SzStat_Item[item];
    		  			//theJSLog<<"sql 输出值为 "<<(it_table->second)[i+j]->SzStat_Item[item]<<endi;
    		  		}
    		 		
    		  		if(m_SStat_Table.ChIs_Count=='Y')  
    		  			stmt<<(it_table->second)[i+j]->NBill_Count; 		
    			}
    			//theJSLog<<"SQL "<<sqltmp<<endi;
    			stmt.execute();
	 	 	
    		 	i=i+insert_num-1;
    		} 
    		/*
    		if(it_table->second!=NULL)
    			{
    			delete[] it_table->second;
    			it_table->second=NULL;
    			}
    		*/
    		//ds.Close();	
    	}

	 }else{
	 	   cout<<"connect error."<<endl;
	 	   return -1;
	 }
	     conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "统计Insert_Table 出错" << endi;
  		throw jsexcp::CException(0, "统计Insert_Table 出错", __FILE__, __LINE__);
  		conn.close();
  		return -1;
  }	
	map_table_recordToInsert.clear();
	map_table_recordToInsertCount.clear();
	return 0;
}

//merge模式统计中merge表中记录
int CF_CStat::Merge_Table()
{	
	DBConnection conn;//数据库连接
  try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			int i;
    	char sqltmp[SQL_LENGTH];
    	STAT_RECORD* crrRcd=NULL;
    	map<string,v_pRcd>::iterator it_table;
    	for(it_table=map_table_recordToMerge.begin();it_table!=map_table_recordToMerge.end();it_table++)
    	{	
    		Make_Merge_Sql(sqltmp,it_table->first.c_str());
    		int merge_num=STAT_INSERT_CONNT;
    		stmt.setSQLString(sqltmp);
    		//ds.Open(sqltmp,NONSELECT_DML);
    		int icount_toMerge_table=map_table_recordToMergeCount[it_table->first];
    		theJSLog<<"Merge 表:"<<it_table->first.c_str()<<":"<<icount_toMerge_table<<endi;
    		for(i=0;i<icount_toMerge_table;i++)
    		{		
    		    if(icount_toMerge_table-i<STAT_INSERT_CONNT)
    			   merge_num=icount_toMerge_table-i;
    			for(int j=0;j<merge_num;j++)
    			{	
    		    	crrRcd=(it_table->second)[i+j];
    		    	 //将所有值输出
    		 		for(int item=m_SStat_Table.table_name_key_count;item<m_SStat_Table.NStat_Item_Count;item++)
    		  		{	  		    
    		  			if(m_SStat_Table.NItem_Type[item]==STAT_N_TYPE||
    		  	  			m_SStat_Table.NItem_Type[item]==STAT_N_COUNT_TYPE)
    		  				stmt<<atol((it_table->second)[i+j]->SzStat_Item[item]);
    					else if(m_SStat_Table.NItem_Type[item]==STAT_D_COUNT_TYPE)
    						stmt<<atof((it_table->second)[i+j]->SzStat_Item[item]);
    		  			else
    		  				stmt<<crrRcd->SzStat_Item[item];
    		  			//theJSLog<<"sql 输出值为 "<<(it_table->second)[i+j]->SzStat_Item[item]<<endi;
    		  		}
    		  		if(m_SStat_Table.ChIs_Count=='Y')  
    		  			stmt<<crrRcd->NBill_Count; 
    		  		//theJSLog<<"sql 输出值为 "<<crrRcd->NBill_Count<<endi;
    		  		//添加统计值
    		  		for(int item=m_SStat_Table.table_name_key_count;item<m_SStat_Table.NStat_Item_Count;item++)
    		  		{	  		    
    		  			if(m_SStat_Table.NItem_Type[item]==STAT_N_COUNT_TYPE)
    		  				{stmt<<atol((it_table->second)[i+j]->SzStat_Item[item]);
    		  			//theJSLog<<"输出值为 "<<(it_table->second)[i+j]->SzStat_Item[item]<<endi;
    		  				}
    					else if(m_SStat_Table.NItem_Type[item]==STAT_D_COUNT_TYPE)
    						{stmt<<atof((it_table->second)[i+j]->SzStat_Item[item]);
    					//theJSLog<<"输出值为 "<<(it_table->second)[i+j]->SzStat_Item[item]<<endi;
    		  				}
    		  		}
    		  		if(m_SStat_Table.ChIs_Count=='Y')  
    		  			{stmt<<crrRcd->NBill_Count;
    				 // theJSLog<<"输出值为 "<<crrRcd->NBill_Count<<endi;
    		  				}
    			}	 
    			stmt.execute();     	 	
    		 	i=i+merge_num-1;
    		 	//theJSLog<<"i= "<<i<<endi;
    		} 
    		//ds.Close();	
    	}
	
	 }else{
	 	   cout<<"connect error."<<endl;
	 	   return -1;
	 }
	     conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "统计Merge_Table 出错" << endi;
  		throw jsexcp::CException(0, "统计Merge_Table 出错", __FILE__, __LINE__);
  		conn.close();
  		return -1;
  } 
	
	map_table_recordToMerge.clear();
	map_table_recordToMergeCount.clear();
	return 0;
			
}
//update模式统计中update 表中记录
int CF_CStat::Update_Table()
{
	//cout<<"int Update_Table ---------------"<<endl;
	//CBindSQL ds( DBConn );
	DBConnection conn;//数据库连接
  try{			
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
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
    		//ds.Open(sqltmp,NONSELECT_DML);
    		stmt.setSQLString(sqltmp);
    		int icount_toUpdate_table=map_table_recordToUpdateCount[it_table->first];
    		theJSLog<<"更新表:"<<it_table->first.c_str()<<":"<<icount_toUpdate_table<<endi;
    		for(i=0;i<icount_toUpdate_table;i++)
    		{		
    		    if(icount_toUpdate_table-i<STAT_INSERT_CONNT)
    			   update_num=icount_toUpdate_table-i;
    		   
    		    for(j=0;j<update_num;j++)
    			{		
    				//cout<<i+j<<endl;
    				
    			 	crrRcd=(it_table->second)[i+j];
    		    		//printOneRecord(crrRcd);
    
     	
    		 		for(int item=m_SStat_Table.table_name_key_count;item<m_SStat_Table.NStat_Item_Count;item++)
    		  		{
    		  			if(m_SStat_Table.NItem_Type[item]==STAT_N_COUNT_TYPE)
    		  				stmt<<atol(crrRcd->SzStat_Item[item]); 	
    					
    					if(m_SStat_Table.NItem_Type[item]==STAT_D_COUNT_TYPE)
    		  				stmt<<atof(crrRcd->SzStat_Item[item]); 
    		  		}
    		  		if(m_SStat_Table.ChIs_Count=='Y')  
    		  			stmt<<crrRcd->NBill_Count; 		
    			
     	
    				for(int item=m_SStat_Table.table_name_key_count;item<m_SStat_Table.NStat_Item_Count;item++)
    				{
    					if(m_SStat_Table.NItem_Type[item]==STAT_N_TYPE)
    						stmt<<atol(crrRcd->SzStat_Item[item]);
    					else if(m_SStat_Table.NItem_Type[item]==STAT_SZ_TYPE 
    						    || m_SStat_Table.NItem_Type[item]>=KEYWORK_MIN) 
    						stmt<<crrRcd->SzStat_Item[item]; 		
    				}
    			}
    
    	        //theJSLog<<"SQL "<<sqltmp<<endi;
    			//ds.Execute();	
    			stmt.execute();					 	
    			i=i+update_num-1;
    		}  
    		//ds.Close();	
    		/*
    		if(it_table->second!=NULL)
    			{
    			delete[] it_table->second;
    			it_table->second=NULL;
    			}
    		*/
    	}
			
				
	 }else{
	 	   cout<<"connect error."<<endl;
	 	   return -1;
	 }
	     conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "Update_Table 出错" << endi;
  		throw jsexcp::CException(0, "Update_Table 出错", __FILE__, __LINE__);
  		conn.close();
  		return -1;
  } 
	
	map_table_recordToUpdate.clear();
	map_table_recordToUpdateCount.clear();
	return 0;
}
    			

void CF_CStat::update_commit()
{ 
    theJSLog<<"统计结果输出到表 或文件... "<<endi;

   /*
   if(Result_Output_Des!='T')
   	{
	 return;
   	}
   */
   
   //cout<<"update_commit:m_SStat_Table.CHStat_Mode==="<<m_SStat_Table.CHStat_Mode<<endl;
   if (m_SStat_Table.CHStat_Mode=='U' ||m_SStat_Table.CHStat_Mode=='T'||m_SStat_Table.CHStat_Mode=='M')
   	{  	
   	//20080604
  	//Update_Stat_File(); 
	if(icount_toUpdate==0)
		{
		return;
		}
	
	char debInfo[100];
	sprintf(debInfo,"共%d 条统计结果",icount_toUpdate);
      theJSLog<<debInfo<<endi;
	  
//  	expTrace(G_CF_CStat_DebugFlag, __FILE__, __LINE__, "%d records to update table ",icount_toUpdate);  
 	Prepare_Rec();		

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
   	if(Result_Output_Des=='T')
   		{
   		if(m_SStat_Table.CHStat_Mode=='U')
   		  {
	   	 //将map_table_recordToUpdate中记录插入表  
		sprintf(debInfo,"to update table:%s ",m_SStat_Table.SzTable_Name);
		theJSLog<<debInfo<<endi;
		
	        Update_Table();	  

		sprintf(debInfo,"to INSERT table:%s ",m_SStat_Table.SzTable_Name);
		theJSLog<<debInfo<<endi;
		
	       Insert_Table();
   			}
   		else if(m_SStat_Table.CHStat_Mode=='M')
   		{
   		sprintf(debInfo,"to merge table:%s ",m_SStat_Table.SzTable_Name);
		theJSLog<<debInfo<<endi;
		
	       Merge_Table();
   		}
   		}
  
	else
		{	       
		sprintf(debInfo,"output file:%s ",Temp_Out_File_Name);
		theJSLog<<debInfo<<endi;
		STAT_RECORD* crrRcd=NULL;
		map<string,v_pRcd>::iterator it_table;
		theJSLog<<"map_table_recordToInsert.size()="<<map_table_recordToInsert.size()<<endi;
		for(it_table=map_table_recordToInsert.begin();it_table!=map_table_recordToInsert.end();it_table++)
			{
			
			int icount_toUpdate_table=map_table_recordToInsertCount[it_table->first];
			theJSLog<<"result table="<<it_table->first<<",icount_toUpdate_table="<<icount_toUpdate_table<<endi;
			for(int i=0;i<icount_toUpdate_table;i++)
				{
				crrRcd=(it_table->second)[i];
				int j;
				for(j=1;j<m_SStat_Table.NStat_Item_Count+1;j++)
					{
					Out_Record.Set_Field(j,crrRcd->SzStat_Item[j-1]);	
					//theJSLog<<j<<":"<<crrRcd->SzStat_Item[j-1]<<endi;
					}
				if(m_SStat_Table.ChIs_Count=='Y')  
					{
					char c_count[10];
					sprintf(c_count,"%d",crrRcd->NBill_Count);
					Out_Record.Set_Field(j,c_count);	
					}
				char strtmp[200];
				Out_Record.Get_record(strtmp,200);
				//theJSLog<<"Out_Record:"<<strtmp<<endi;
				Temp_Out_File.writeRec(Out_Record);
				middRcdCount++;   
				}
			}      
		map_table_recordToInsert.clear();
		map_table_recordToInsertCount.clear();
		}
   	}
   catch (CException e)
      {
      //20070419,增强了错误信息输出
      char szLogStr[700];
      char DBerrmsg[500];
      if(strlen(e.GetErrMessage())<499)
      		{
      		strcpy(DBerrmsg,e.GetErrMessage());
      		}
      else
      		{
	      strncpy(DBerrmsg,e.GetErrMessage(),499);
	      DBerrmsg[499]=0;
      		}
      sprintf(szLogStr,"fail to update table. ErrorMessage:%s",DBerrmsg);
	errLog(LEVEL_ERROR, "", ERR_INSERT_TABLE,szLogStr ,__FILE__,__LINE__);
      throw CException(ERR_INSERT_TABLE,szLogStr,__FILE__,__LINE__);
      }
   catch(...)
	{
	 char szLogStr[100];	
	strcpy(szLogStr,"update table,unknow error!!! ");
	errLog(LEVEL_ERROR, "", ERR_INSERT_TABLE,szLogStr ,__FILE__,__LINE__);
       throw CException(ERR_INSERT_TABLE,szLogStr,__FILE__,__LINE__);
	}
    checkb=clock();

    if(Current_Memorry_Unit>m_maxRcdToUpdate 
		&&(m_SStat_Table.CHStat_Mode=='U'||m_SStat_Table.CHStat_Mode=='T'||m_SStat_Table.CHStat_Mode=='M'))
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
		errLog(LEVEL_ERROR, "", ERR_REQU_MEM,"new err." ,__FILE__,__LINE__);
		throw CException(ERR_REQU_MEM,(char *)"new err.",__FILE__,__LINE__);
		}
	}
	
   }
  else
  	{
  	try
  		{
	  	Insert_Stat();
  		}
  	catch(SQLException e)
  		{
  		//cout<<"Error number: "<<  ex.GetErrorCode() << endl;
		  //cout<<"Error Message: "<<ex.GetErrorMsg() << endl;    
//		errLog(LEVEL_ERROR, "", ex.GetErrorCode() ,ex.GetErrorMsg().c_str() ,__FILE__,__LINE__);
      cout<<e.what()<<endl;
  		theJSLog << "update_commit 出错" << endi;
  		throw jsexcp::CException(0, "update_commit 出错", __FILE__, __LINE__);
  		//conn.close();
		  //throw(ex);
  		}
	catch(...)
  		{
  		errLog(LEVEL_ERROR, "", ERR_INSERT_TABLE,"fail to insert record into table." ,__FILE__,__LINE__);
  		throw CException(ERR_INSERT_TABLE,(char *)"fail to insert record into table.",__FILE__,__LINE__);
  		}
  	m_NRecord_Count=0;
  	}
  	char dealtime[DATETIME_LEN+1];	    	
    	getCurTime(dealtime);  
	sprintf(m_SzStat_Time,"%s",dealtime);
       runLog(LEVEL_DEBUG, "update table finish! ");
//	expTrace(G_CF_CStat_DebugFlag, __FILE__, __LINE__, "update table finish! "); 
}

void CF_CStat::rollback()
{
  m_key_record.clear();
  m_key_seq.clear();
  m_NRecord_Count=0;

  icount_toUpdate = 0;  

   if(Current_Memorry_Unit>m_maxRcdToUpdate 
   	&&(m_SStat_Table.CHStat_Mode=='U'||m_SStat_Table.CHStat_Mode=='T'))
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
		 errLog(LEVEL_ERROR, "",ERR_REQU_MEM,"new err.",__FILE__,__LINE__);
		throw CException(ERR_REQU_MEM,(char *)"new err.",__FILE__,__LINE__);
		}
   	}
   
   map<string,v_pRcd>::iterator it_table;		
   for(it_table=map_table_recordToInsert.begin();it_table!=map_table_recordToInsert.end();it_table++)
	{
	it_table->second.clear();
	}
  map_table_recordToInsert.clear();
	
   for(it_table=map_table_recordToUpdate.begin();it_table!=map_table_recordToUpdate.end();it_table++)
	{
	it_table->second.clear();
	}
  map_table_recordToUpdate.clear();
  
  if(Result_Output_Des=='F')
	{
	theJSLog<<"stat rollback, delete tmp file..."<<endi;
	Unlink_TemFile();
	//Unlink_File();
  	}
  v_tmpOutFileName.clear();
  v_outFileName.clear();
  strcpy(Temp_Out_File_Name,"-1");
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

char * CF_DUP_CStat:: getVersion()
{
	return("3.0.0");
}

int CF_DUP_CStat::Init(char *configid)
{ 
 //cout<<"int CF_DUP_CStat::Init(char *configid)"<<endl;
   	//DynamicSQL SqlStr;
	//CBindSQL ds(DBConn);	
	DBConnection conn;//数据库连接
  try{			
  	if (dbConnect(conn))
  	 {
  			Statement stmt = conn.createStatement();
  			char szTemp[100];
        theJSLog<<"configid="<<configid<<endi;
       int maxRcdNumToUpdat=STAT_MAXNUM_UPATE;
       
      sprintf(szTemp,configid);
    
      if(szTemp[strlen(szTemp)-1]!=ConfigIDSPILT)
      	sprintf(szTemp,"%s%c",szTemp,ConfigIDSPILT);
    
    
      char *TmpPoint1;
      char *TmpPoint2;
      int iTmpCount=0;
      vector <int> v_configid;
      v_configid.clear();
      TmpPoint1=szTemp;
      while(1)
      {
        TmpPoint2=strchr(TmpPoint1,ConfigIDSPILT);
        if(TmpPoint2==NULL) 
    		break;
        *TmpPoint2=0;
        v_configid.push_back(atol(TmpPoint1));
        TmpPoint1=TmpPoint2+1;
      }
      iStatArr_Count=v_configid.size();
    
      if(iStatArr_Count==0)
      	{
      	  sprintf(szTemp,"未定义正确的STAT_CONFIG_ID!");
    	  errLog(LEVEL_ERROR, "",ERR_CONFIGID,szTemp,__FILE__,__LINE__);
       	  throw CException(ERR_CONFIGID,szTemp,__FILE__,__LINE__);
      	}
      pStatArr=new CF_CStat[iStatArr_Count];
      if(pStatArr==NULL)
      {
        sprintf(szTemp,"new CF_CStat pStatArr err !");
          errLog(LEVEL_ERROR, "",ERR_REQ_MEM,szTemp,__FILE__,__LINE__);
        throw CException(ERR_REQ_MEM,szTemp,__FILE__,__LINE__);
      }
      
    
    	int table_cnt=0;
    	char valid_flag;
    	int true_id=0;
    	
      for(int i=0;i<iStatArr_Count;i++)
      {
      	  if(v_configid[i]<0)
    	  	true_id=-v_configid[i];
    	  else 
    	  	true_id= v_configid[i];
  
    	  string sql = "select VALID_FLAG from C_STAT_TABLE_DEFINE where CONFIG_ID=:id";
    	  stmt.setSQLString(sql);
			  stmt << true_id;			
			  stmt.execute();
			  stmt >> valid_flag;
			
    	  if(valid_flag=='Y')
    	  	{
    	      pStatArr[table_cnt].Init(v_configid[i],maxRcdNumToUpdat);
    	      table_cnt++;
    	      }
      }  
      iStatArr_Count=table_cnt;
      
      theJSLog<<"iStatArr_Count="<<iStatArr_Count<<endi;
    
    //  expTrace("Y", __FILE__, __LINE__,"iStatArr_Count=%d;", iStatArr_Count);
    
      if(iStatArr_Count==0)
      	{
      	//
      	 sprintf(szTemp,"no valid CONFIG_ID in C_STAT_TABLE_DEFINE !");
    	  errLog(LEVEL_ERROR, "",ERR_CONFIGID,szTemp,__FILE__,__LINE__);
       	 throw CException(ERR_CONFIGID,szTemp,__FILE__,__LINE__);
      	}
  			
  				
  	 }else{
  	 	   cout<<"connect error."<<endl;
  	 	   return -1;
  	 }
  	     conn.close();
  	 } catch( SQLException e ) {
    		cout<<e.what()<<endl;
    		theJSLog << "CF_DUP_CStat::Init 出错" << endi;
    		throw jsexcp::CException(0, "CF_DUP_CStat::Init 出错", __FILE__, __LINE__);
    		conn.close();
    		return -1;
    } 

  return 0;
}

/*
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
*/

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


//设置临时统计结果输出文件
void CF_DUP_CStat::Set_TempOutFile()
{
  int i;
  for(i=0;i<iStatArr_Count;i++)
  {
    pStatArr[i].Set_TempOutFile();
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
int CF_DUP_CStat::Rename_AllOutFile()
{
  int i;
  for(i=0;i<iStatArr_Count;i++)
  {
    if(pStatArr[i].Rename_AllOutFile())		
		return 1;
  }  
  return 0;
}

int CF_DUP_CStat::dealRedoRec(CFmt_Change &inrcd, char *szError_Type)
{
    //cout<<"iStatArr_Count="<<iStatArr_Count<<endl;

	
     for(int i=0;i<iStatArr_Count;i++)
	  {
	    try
	    {
	      pStatArr[i].dealRedoRec(inrcd,szError_Type);
	      total_update_count=G_CF_CStat_UpdateCount;
	    }
	    catch(CException e)
	    {

		 throw e;
	    }
	  }
 	return 0;
}

/*
int CF_DUP_CStat::dealRedoRec(PacketParser& ps, char *szError_Type)
{
//cout<<"iStatArr_Count="<<iStatArr_Count<<endl;

	int fieldCount=ps.getFieldCount();
	vector <string> v_inrcd;
	v_inrcd.clear();
	 int i;
	 //int PacketParser::getValue (const int index, char buffer[])const
	 char tmpFieldValue[256];
	 tmpFieldValue[0]=0;
	for(i=0;i<fieldCount;i++)
		{
		int ret=ps.getValue(i,tmpFieldValue);
		if(ret ==-1 )
			{
			char errmsg[500];
			sprintf(errmsg,"总字段数为:%d,  获取字段值失败, 字段序号: %d  !!", fieldCount, i );
			throw CException(ERR_GET_FIELD_VALUE,errmsg,__FILE__,__LINE__);
			return -1;
			}
		v_inrcd.push_back(tmpFieldValue);
		}

	  for(i=0;i<iStatArr_Count;i++)
		  {
		    try
			    {
			      pStatArr[i].dealRedoRec(ps,szError_Type);
			      total_update_count=G_CF_CStat_UpdateCount;
			    }
		    catch(CException e)
			    {

				 throw e;
			    }
		  }
	 return 0;
}
*/
//供冲销使用，判断统计结果是否输出到文件
//若是，则将临时结果文件改成正式文件
//成功返回0，失败返回1
int CF_DUP_CStat::Neg_Commit()
{
	return(Rename_AllOutFile());
}

void CF_DUP_CStat::update_commit()
{
	cout<<"iStatArr_Count="<<iStatArr_Count<<endl;
	theJSLog<<"iStatArr_Count="<<iStatArr_Count<<endi;
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




