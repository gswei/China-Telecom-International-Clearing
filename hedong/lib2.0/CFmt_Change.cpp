//20060913 sprintf( Field[i].field,ss0 );->sprintf( Field[i].field,"%s",ss0 );解决core问题
//20061008 增加长度限制,即原始话单长度不够长时，只取到原始话单长度
//20061010 增加错误类型可配置字段col_errtype_index
//20070827 更新D 方式的set_record。判断每个字段没去填充符前的长度
//20070918 更新C 方式的set_record。判断每个字段没去填充符前的长度
//SELECT  count(*) FROM user_tab_columns WHERE table_name='FILE_HEADEND_DEFINE' and column_name ='HEAD_END_SEPARATOR'
//SELECT  count(*) FROM user_tab_columns WHERE table_name='FILETYPE_DEFINE' and column_name ='CHECK_FIELDNUM'
//增加字段col_reallen
//200803 优化set_record\get_record
//20080417 将原来字段中有填充符挂起，改为替换成*
//20081223 可半定长半不定长读写，写的只限D 模式。
//         用法:C\D的分隔符为NULL的为定长字段，取begin和len用



#include "CFmt_Change.h"
#include <stdio.h>
#include <string.h>
#include <iostream.h>
#include "COracleDB.h"
#include "CF_CError.h"
long tmp_readtmp_count;
long tmp_wttmp_count;

//clock_t checkff=0;
//clock_t checkf1=0;
//clock_t checkf2=0;
//clock_t checkf3=0;
//clock_t checkf4=0;

//EXEC SQL INCLUDE sqlca;
//EXEC SQL INCLUDE oraca;

CFmt_Change::CFmt_Change()
{
	Txt_Fmt = NULL;
	Field = NULL;
}

CFmt_Change::CFmt_Change(const CFmt_Change &right)
{

  if((right.Txt_Fmt==NULL)||(right.Field==NULL))
  {
    Txt_Fmt = NULL;
	Field = NULL;
  }
  else
  { 
	record_type = right.record_type;
	field_count = right.field_count;
	strcpy(filetype_id, right.filetype_id);
    Record_Len=right.Record_Len;
    real_field_count=right.real_field_count;
    strcpy(Tmpss,right.Tmpss);
    strcpy(TMP,right.TMP);
	
   	Txt_Fmt=new TXT_FMT[field_count+2];
	
	int k;
   	for(k = 0; k < field_count+2; k++)
  	{
		Txt_Fmt[k].col_filler = right.Txt_Fmt[k].col_filler;
		Txt_Fmt[k].col_index = right.Txt_Fmt[k].col_index;
		Txt_Fmt[k].col_bigen = right.Txt_Fmt[k].col_bigen;
		Txt_Fmt[k].col_len = right.Txt_Fmt[k].col_len;
		Txt_Fmt[k].col_seperator = right.Txt_Fmt[k].col_seperator;
		Txt_Fmt[k].col_log = right.Txt_Fmt[k].col_log;
		Txt_Fmt[k].col_fmt = right.Txt_Fmt[k].col_fmt;
    	  Txt_Fmt[k].check_len = right.Txt_Fmt[k].check_len;
    	  Txt_Fmt[k].col_reallen = right.Txt_Fmt[k].col_reallen;
		  strcpy(Txt_Fmt[k].colname,right.Txt_Fmt[k].colname);
		  strcpy(Txt_Fmt[k].col_end,right.Txt_Fmt[k].col_end);
	}
	
	Field = new FIELD[field_count+2];
	
	for(k = 0; k < field_count+2; k++)
 		strcpy(Field[k].field, right.Field[k].field);

  }
}


CFmt_Change::~CFmt_Change()
{
	if (Txt_Fmt != NULL)
	{
		delete[] Txt_Fmt;
		Txt_Fmt = NULL;
	}
	if (Field != NULL)
	{
		delete[] Field;
		Field = NULL;
	}
}

CFmt_Change & CFmt_Change::operator=(const CFmt_Change &right)
{
if (this == &right) return *this;


	if (Txt_Fmt != NULL)
	{
		delete[] Txt_Fmt;
		Txt_Fmt = NULL;
	}
	if (Field != NULL)
	{
		delete[] Field;
		Field = NULL;
	}



	record_type=right.record_type;
	field_count=right.field_count;
	strcpy(filetype_id,right.filetype_id);
	Record_Len=right.Record_Len;
	real_field_count=right.real_field_count;
	strcpy(Tmpss,right.Tmpss);
	strcpy(TMP,right.TMP);

   	Txt_Fmt=new TXT_FMT[field_count+2];
   	if(!Txt_Fmt)
  		{
	  	throw CF_CError('O','H',ERR_REQ_MEM,0,(char *)"new Txt_Fmt fail.",__FILE__,__LINE__);
  		}
   	for(int k=0;k<field_count+2;k++)
  		{
		  Txt_Fmt[k].col_filler=right.Txt_Fmt[k].col_filler;
		  Txt_Fmt[k].col_index=right.Txt_Fmt[k].col_index;
		  Txt_Fmt[k].col_bigen=right.Txt_Fmt[k].col_bigen;
		  Txt_Fmt[k].col_len=right.Txt_Fmt[k].col_len;
		  Txt_Fmt[k].col_seperator=right.Txt_Fmt[k].col_seperator;
		  Txt_Fmt[k].col_log=right.Txt_Fmt[k].col_log;
    	  Txt_Fmt[k].col_fmt = right.Txt_Fmt[k].col_fmt;
    	  Txt_Fmt[k].check_len = right.Txt_Fmt[k].check_len;
    	  Txt_Fmt[k].col_reallen = right.Txt_Fmt[k].col_reallen;
		  strcpy(Txt_Fmt[k].colname,right.Txt_Fmt[k].colname);
		  strcpy(Txt_Fmt[k].col_end,right.Txt_Fmt[k].col_end);
   		}

	Field=new FIELD[field_count+2];
  	if(!Field)
  		{
	  	throw CF_CError('O','H',ERR_REQ_MEM,0,(char *)"new Field fail.",__FILE__,__LINE__);
  		}
  	for(int k=0;k<field_count+2;k++)
  		{
  		strcpy(Field[k].field,right.Field[k].field);
  		}

  	return *this;
}
int CFmt_Change::Init(char *id,CBindSQL &SqlStr,char type)
{
//	CBindSQL SqlStr( DBConn );	//DynamicSQL SqlStr;
    printf("Begin init rcd_type (%s) ... ...\n",id);

	if (Txt_Fmt != NULL)
	{
		delete[] Txt_Fmt;
		Txt_Fmt = NULL;
	}
	if (Field != NULL)
	{
		delete[] Field;
		Field = NULL;
	}


	if(type=='0')
		{
		SqlStr.Open("select record_type from filetype_define \
			  where filetype_id=:id ", SELECT_QUERY);			
		SqlStr<<id;
			
		if ( !(SqlStr>>type ) ) 
			{
        	sprintf(Tmpss,"select the record_type for filetype_id(%s) err",id);
  	        throw CF_CErrorFile('E','M',ERR_TYPE,0,Tmpss,__FILE__,__LINE__);
			return (-1);
			}
		SqlStr.Close();	
		}


    int tmpCHECK_FIELDNUM=0;
	SqlStr.Open("SELECT  count(*) FROM user_tab_columns WHERE table_name='FILETYPE_DEFINE' and column_name ='CHECK_FIELDNUM' ", SELECT_QUERY);			
    SqlStr>>tmpCHECK_FIELDNUM;
	SqlStr.Close();

    if(tmpCHECK_FIELDNUM)
    {
	  SqlStr.Open("SELECT  CHECK_FIELDNUM FROM FILETYPE_DEFINE WHERE filetype_id=:id ", SELECT_QUERY);			
      SqlStr<<id;
      SqlStr>>check_field_num;
	  SqlStr.Close();
    }else check_field_num='N';

	
	
	if((type!='A')&&(type!='C')&&(type!='D'))
	{
    sprintf(Tmpss,"record_type(%c) err",type);
  	throw CF_CErrorFile('E','M',ERR_TYPE,0,Tmpss,__FILE__,__LINE__);
	return (-1);
	}
   record_type=type;
   strcpy(filetype_id,id);
   
	int _col_index;
	int _col_bigen;
	int _col_len;
	char _colname[20];
   	char _col_seperator;
   	char _col_log;
   	char _col_filler;
   	char _col_fmt;
   	int _check_len;
   	char _col_end[6];

//20061010 add by zhoulh
   	int _col_errtype_index;
	int i=1;
	
	SqlStr.Open("select record_len from filetype_define \
			  where filetype_id=:id ", SELECT_QUERY);			
	SqlStr<<id;
			
	if ( !(SqlStr>>Record_Len ) ) 
		{
        sprintf(Tmpss,"count the id(%s) record_len err.",id);
  	    throw CF_CErrorFile('E','M',ERR_TYPE,0,Tmpss,__FILE__,__LINE__);
		return (-1);
		}
	SqlStr.Close();
	
		
	SqlStr.Open("select count(*) from txtfile_fmt \
			  where filetype_id=:id ", SELECT_QUERY);			
	SqlStr<<id;
			
	if ( !(SqlStr>>field_count ) ) 
		{
        sprintf(Tmpss,"count the id(%s) list err.",id);
  	    throw CF_CErrorFile('E','M',ERR_TYPE,0,Tmpss,__FILE__,__LINE__);
		return (-1);
		}
	SqlStr.Close();
   	Txt_Fmt=new TXT_FMT[field_count+2];
   	if(!Txt_Fmt)
  		{
	  	throw CF_CError('O','H',ERR_REQ_MEM,0,(char *)"new Txt_Fmt fail.",__FILE__,__LINE__);
  		return (-1);
  		}
   	for(int k=0;k<field_count+2;k++)
  		{
		  Txt_Fmt[k].col_filler=' ';
  		}

	Field=new FIELD[field_count+2];
  	if(!Field)
  		{
	  	throw CF_CError('O','H',ERR_REQ_MEM,0,(char *)"new Field fail.",__FILE__,__LINE__);
  		return (-1);
  		}
  	for(int k=0;k<field_count+2;k++)
  		{
  		memset(Field[k].field,0,string_len);
  		}
  	
	SqlStr.Open("select col_index,colname,col_begin,col_len,col_seperator,col_loc,nvl(col_filler,' '),col_fmt,nvl(check_len,-1),col_end,nvl(col_errtype_index,0) from txtfile_fmt \
				  where filetype_id=:id \
				  ORDER BY col_index ", SELECT_QUERY);			
	SqlStr<<id;   
	if(SqlStr.IsError())
		{

        sprintf(Tmpss,"select DB content for id(%s) err.",id);
  	    throw CF_CErrorFile('E','M',ERR_TYPE,0,Tmpss,__FILE__,__LINE__);
		
		return (-1);
		}
	while ( SqlStr>>_col_index>>_colname>>_col_bigen>>_col_len>>_col_seperator>>_col_log>>_col_filler>>_col_fmt>>_check_len>>_col_end>>_col_errtype_index )
		{ if(SqlStr.IsEnd()) break;		
		  Txt_Fmt[i].col_index=_col_index;
		  strcpy(Txt_Fmt[i].colname,_colname);
		  DeleteFiller(Txt_Fmt[i].colname,' ');
		  Txt_Fmt[i].col_bigen=_col_bigen;
		  Txt_Fmt[i].col_len=_col_len;
		  Txt_Fmt[i].col_seperator=_col_seperator;
		  Txt_Fmt[i].col_log=_col_log;
		  Txt_Fmt[i].col_filler=_col_filler;
		  Txt_Fmt[i].col_errtype_index=_col_errtype_index;
		  Txt_Fmt[i].col_fmt=_col_fmt;
		  Txt_Fmt[i].check_len=_check_len;
		  Txt_Fmt[i].col_reallen = 0;
		  strcpy(Txt_Fmt[i].col_end,_col_end);
		  DeleteFiller(Txt_Fmt[i].col_end,' ');
		  if(strlen(Txt_Fmt[i].col_end)==0)
		  {
		    Txt_Fmt[i].col_end[0] = '@';
		    Txt_Fmt[i].col_end[1] = '\0';
		  }
		  i++;
  		}
	if(SqlStr.IsError())
		{
        sprintf(Tmpss,"select DB content for id(%s) err.",id);
  	    throw CF_CErrorFile('E','M',ERR_TYPE,0,Tmpss,__FILE__,__LINE__);
		return (-1);
		}
	SqlStr.Close();
  return 0;

}
int CFmt_Change::Init(char *id,char type)
{


    printf("Begin init rcdtype (%s) ... ...\n",id);

	CBindSQL SqlStr( DBConn );	//DynamicSQL SqlStr;

	if (Txt_Fmt != NULL)
	{
		delete[] Txt_Fmt;
		Txt_Fmt = NULL;
	}
	if (Field != NULL)
	{
		delete[] Field;
		Field = NULL;
	}


	if(type=='0')
		{
		SqlStr.Open("select record_type from filetype_define \
			  where filetype_id=:id ", SELECT_QUERY);			
		SqlStr<<id;
			
		if ( !(SqlStr>>type ) ) 
			{
        	sprintf(Tmpss,"select the record_type for filetype_id(%s) err",id);
  	        throw CF_CErrorFile('E','M',ERR_TYPE,0,Tmpss,__FILE__,__LINE__);
			return (-1);
			}
		SqlStr.Close();	
		}
	
    int tmpCHECK_FIELDNUM=0;
	SqlStr.Open("SELECT  count(*) FROM user_tab_columns WHERE table_name='FILETYPE_DEFINE' and column_name ='CHECK_FIELDNUM' ", SELECT_QUERY);			
    SqlStr>>tmpCHECK_FIELDNUM;
	SqlStr.Close();

    if(tmpCHECK_FIELDNUM)
    {
	  SqlStr.Open("SELECT  CHECK_FIELDNUM FROM FILETYPE_DEFINE WHERE filetype_id=:id ", SELECT_QUERY);			
      SqlStr<<id;
      SqlStr>>check_field_num;
	  SqlStr.Close();
    }else check_field_num='N';

	
	if((type!='A')&&(type!='C')&&(type!='D'))
	{
    sprintf(Tmpss,"record_type(%c) err",type);
  	throw CF_CErrorFile('E','M',ERR_TYPE,0,Tmpss,__FILE__,__LINE__);
	return (-1);
	}
   record_type=type;
   strcpy(filetype_id,id);
   
	int _col_index;
	int _col_bigen;
	int _col_len;
	char _colname[20];
   	char _col_seperator;
   	char _col_log;
   	char _col_filler;
   	char _col_fmt;
   	int _check_len;
   	char _col_end[6];

//20061010 add by zhoulh
   	int _col_errtype_index;
	int i=1;
	
	SqlStr.Open("select record_len from filetype_define \
			  where filetype_id=:id ", SELECT_QUERY);			
	SqlStr<<id;
			
	if ( !(SqlStr>>Record_Len ) ) 
		{
        sprintf(Tmpss,"count the id(%s) record_len err.",id);
  	    throw CF_CErrorFile('E','M',ERR_TYPE,0,Tmpss,__FILE__,__LINE__);
		return (-1);
		}
	SqlStr.Close();
	
		
	SqlStr.Open("select count(*) from txtfile_fmt \
			  where filetype_id=:id ", SELECT_QUERY);			
	SqlStr<<id;
			
	if ( !(SqlStr>>field_count ) ) 
		{
        sprintf(Tmpss,"count the id(%s) list err.",id);
  	    throw CF_CErrorFile('E','M',ERR_TYPE,0,Tmpss,__FILE__,__LINE__);
		return (-1);
		}
	SqlStr.Close();
   	Txt_Fmt=new TXT_FMT[field_count+2];
   	if(!Txt_Fmt)
  		{
	  	throw CF_CError('O','H',ERR_REQ_MEM,0,(char *)"new Txt_Fmt fail.",__FILE__,__LINE__);
  		return (-1);
  		}
   	for(int k=0;k<field_count+2;k++)
  		{
		  Txt_Fmt[k].col_filler=' ';
  		}

	Field=new FIELD[field_count+2];
  	if(!Field)
  		{
	  	throw CF_CError('O','H',ERR_REQ_MEM,0,(char *)"new Field fail.",__FILE__,__LINE__);
  		return (-1);
  		}
  	for(int k=0;k<field_count+2;k++)
  		{
  		memset(Field[k].field,0,string_len);
  		}
  	
	SqlStr.Open("select col_index,colname,col_begin,col_len,col_seperator,col_loc,nvl(col_filler,' '),col_fmt,nvl(check_len,-1),col_end,nvl(col_errtype_index,0) from txtfile_fmt \
				  where filetype_id=:id \
				  ORDER BY col_index ", SELECT_QUERY);			
	SqlStr<<id;   
	if(SqlStr.IsError())
		{

        sprintf(Tmpss,"select DB content for id(%s) err.",id);
  	    throw CF_CErrorFile('E','M',ERR_TYPE,0,Tmpss,__FILE__,__LINE__);
		
		return (-1);
		}
	while ( SqlStr>>_col_index>>_colname>>_col_bigen>>_col_len>>_col_seperator>>_col_log>>_col_filler>>_col_fmt>>_check_len>>_col_end>>_col_errtype_index )
		{ if(SqlStr.IsEnd()) break;		
		  Txt_Fmt[i].col_index=_col_index;
		  strcpy(Txt_Fmt[i].colname,_colname);
		  DeleteFiller(Txt_Fmt[i].colname,' ');
		  Txt_Fmt[i].col_bigen=_col_bigen;
		  Txt_Fmt[i].col_len=_col_len;
		  Txt_Fmt[i].col_seperator=_col_seperator;
		  Txt_Fmt[i].col_log=_col_log;
		  Txt_Fmt[i].col_filler=_col_filler;
		  Txt_Fmt[i].col_errtype_index=_col_errtype_index;
		  Txt_Fmt[i].col_fmt=_col_fmt;
		  Txt_Fmt[i].check_len=_check_len;
		  Txt_Fmt[i].col_reallen = 0;
		  strcpy(Txt_Fmt[i].col_end,_col_end);
		  DeleteFiller(Txt_Fmt[i].col_end,' ');
		  if(strlen(Txt_Fmt[i].col_end)==0)
		  {
		    Txt_Fmt[i].col_end[0] = '@';
		    Txt_Fmt[i].col_end[1] = '\0';
		  }
		  i++;
  		}
	if(SqlStr.IsError())
		{
        sprintf(Tmpss,"select DB content for id(%s) err.",id);
  	    throw CF_CErrorFile('E','M',ERR_TYPE,0,Tmpss,__FILE__,__LINE__);
		return (-1);
		}
	SqlStr.Close();
  return 0;

}


int CFmt_Change::strlen(const char *str)
{   
  assert(str);
  const char *p=str;
  while(*p++);
  return p-str-1;
}


char* CFmt_Change::Get_id()
{
return filetype_id;
}


int CFmt_Change::Copy_Record(CFmt_Change &dst_record)
{
 if(strcmp(filetype_id,dst_record.Get_id()))
	{
    sprintf(Tmpss,"the filetype_id %s(input) doesn't equal %s.",dst_record.Get_id(),filetype_id);
  	throw CF_CErrorFile('E','M',ERR_FILETYPEID,0,Tmpss,__FILE__,__LINE__);
	return (-1);
	}
 for(int i=0;i<field_count;i++)
 {
 	strcpy(Field[i].field,dst_record.Get_Field(i+1));
 	Txt_Fmt[i+1].col_reallen = dst_record.Get_RealLen(i+1);
 }
 
 return 0;	
}


int CFmt_Change::Set_record(char *buf ,int len)
{

int buflen=0;
char *ss2;
char *sstmp;
for(int k=0;k<field_count;k++)
{
   Field[k].field[0]='\0';
}

if(len==1) return 0;

if(len) Record_Len = len;
buflen=Record_Len;
strcpy(TMP,buf);



//if(buf[strlen(buf)-1]==0x0D)
int tmplen = strlen(buf)-1;
if(buf[tmplen]=='\n')
{
  	buf[tmplen]='\0';
  	tmplen--;
}
if(buf[tmplen]==0x0D)
{
  buf[tmplen]='\0';
  tmplen--;
}

int flag=0;
if((tmplen+1)>buflen)
{
	flag=1;
}

if(record_type=='C')
{
	if(SplitBuf( buf)) 
	{
		sprintf(Tmpss,"the buffer field count is %d less than %d (define) in filetype_id %s.",real_field_count,field_count,filetype_id);
	  	throw CF_CErrorFile('E','M',ERR_COUNT_LESS,0,Tmpss,__FILE__,__LINE__);
		return(-1);
	}
      int field_len_chk=0;
  	  for(int k=0;k<field_count;k++)
  	  {
//        int tmpfieldlen=strlen(Field[k].field);
//  	    if(tmpfieldlen>Txt_Fmt[k+1].col_len)
        int tmpk=k+1;
  	    if(Txt_Fmt[tmpk].col_reallen>Txt_Fmt[tmpk].col_len)
  	      field_len_chk=tmpk;
  	   	if(Txt_Fmt[tmpk].col_log=='L')
  	   	  Txt_Fmt[tmpk].col_reallen=DeleteFiller(Field[k].field,Txt_Fmt[tmpk].col_filler,Txt_Fmt[tmpk].col_reallen);
  	   	else if(Txt_Fmt[tmpk].col_log=='M')
  	   	{
  	   	  Txt_Fmt[tmpk].col_reallen=DeleteFiller(Field[k].field,Txt_Fmt[tmpk].col_filler,Txt_Fmt[tmpk].col_reallen);
		  Txt_Fmt[tmpk].col_reallen=DeleteHFiller(Field[k].field,Txt_Fmt[tmpk].col_filler,Txt_Fmt[tmpk].col_reallen);
  	   	}
  	   	else Txt_Fmt[tmpk].col_reallen=DeleteHFiller(Field[k].field,Txt_Fmt[tmpk].col_filler,Txt_Fmt[tmpk].col_reallen);
	  }
      if(field_len_chk)
      {
		sprintf(Tmpss,"the field %dth is too large in filetype_id %s.",field_len_chk,filetype_id);
	  	throw CF_CErrorFile('E','M',ERR_INPUTLENGTH_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
		return(-1);
      }

  	if(flag) 
  		{
  		sprintf(Tmpss,"the buffer length is %d longer than %d (define) in filetype_id %s.",strlen(buf),buflen,filetype_id);
  		throw CF_CErrorFile('E','M',ERR_LENGTH_LONG,0,Tmpss,__FILE__,__LINE__);
  		return (-1);
  		}

	return 0;
}
if(record_type=='A')
{
//20061008增加长度限制,即原始话单长度不够长时，只取到原始话单长度
    int bufreallen=tmplen+1;
	if(bufreallen != buflen)
	{
	  flag=1;
	}
	for(int i=0;i<field_count;i++)
	{
	    if(bufreallen<Txt_Fmt[i+1].col_bigen) break;
        strncpy(Field[i].field,buf+Txt_Fmt[i+1].col_bigen-1,Txt_Fmt[i+1].col_len);
        Field[i].field[Txt_Fmt[i+1].col_len]='\0';
        Txt_Fmt[i+1].col_reallen = Txt_Fmt[i+1].col_len;
        if(Txt_Fmt[i+1].col_end[0] != '@')
        {
          if((ss2=strstr(Field[i].field,Txt_Fmt[i+1].col_end)) != NULL)
          {
            *ss2 = '\0';
            Txt_Fmt[i+1].col_reallen=ss2-Field[i].field;
          }
        }
	}
  	for(int k=0;k<field_count;k++)
  	{
  	    int tmpk=k+1;
  	   	if(Txt_Fmt[tmpk].col_log=='L')
  	   	  Txt_Fmt[tmpk].col_reallen=DeleteFiller(Field[k].field,Txt_Fmt[tmpk].col_filler,Txt_Fmt[tmpk].col_reallen);
  	   	else if(Txt_Fmt[tmpk].col_log=='M')
  	   	{
  	   	  Txt_Fmt[tmpk].col_reallen=DeleteFiller(Field[k].field,Txt_Fmt[tmpk].col_filler,Txt_Fmt[tmpk].col_reallen);
		  Txt_Fmt[tmpk].col_reallen=DeleteHFiller(Field[k].field,Txt_Fmt[tmpk].col_filler,Txt_Fmt[tmpk].col_reallen);
  	   	}
  	   	else Txt_Fmt[tmpk].col_reallen=DeleteHFiller(Field[k].field,Txt_Fmt[tmpk].col_filler,Txt_Fmt[tmpk].col_reallen);
	}
  	if(flag) 
  	{
  		sprintf(Tmpss,"the buffer length is %d longer than %d (define) int filetype_id %s",strlen(buf),buflen,filetype_id);
  		throw CF_CErrorFile('E','M',ERR_LENGTH_LONG,0,Tmpss,__FILE__,__LINE__);
  		return (-1);
  	}
	return 0;
}
if(record_type=='D')
{

    int bufreallen=tmplen+1;
	if(bufreallen != buflen)
	{
	  flag=1;
	}
	sstmp=buf;
	for(int i=0;i<field_count;i++)
	{
	    if(bufreallen<Txt_Fmt[i+1].col_bigen) break;
        if((Txt_Fmt[i+1].col_seperator == Txt_Fmt[i+1].col_filler)||(Txt_Fmt[i+1].col_seperator == NULL))
        {
          strncpy(Field[i].field,buf+Txt_Fmt[i+1].col_bigen-1,Txt_Fmt[i+1].col_len);
          Field[i].field[Txt_Fmt[i+1].col_len]='\0';
          Txt_Fmt[i+1].col_reallen = Txt_Fmt[i+1].col_len;
//          if(*(buf+Txt_Fmt[i+1].col_bigen-1+Txt_Fmt[i+1].col_len)!=Txt_Fmt[i+1].col_seperator) flag =1;
        }
        else
        {
          char *ss1;
          if(sstmp<buf+Txt_Fmt[i+1].col_bigen-1) sstmp=buf+Txt_Fmt[i+1].col_bigen-1;
          if( ( ss1 = strchr( sstmp,Txt_Fmt[i+1].col_seperator ) ) != NULL )
          {
            *ss1='\0';
            sprintf( Field[i].field,"%s",sstmp );
            *ss1=Txt_Fmt[i+1].col_seperator;
            sstmp=ss1+1;
            int tmpfieldlen=strlen(Field[i].field);
            Txt_Fmt[i+1].col_reallen=tmpfieldlen;
            if(tmpfieldlen!=Txt_Fmt[i+1].col_len) flag =1;
          }
          else
          {
            sprintf( Field[i].field,"%s",sstmp );
            int tmpfieldlen=strlen(Field[i].field);
            Txt_Fmt[i+1].col_reallen=tmpfieldlen;
            if(tmpfieldlen!=Txt_Fmt[i+1].col_len) flag =1;
            if(i<field_count-1)
            {
		      sprintf(Tmpss,"the buffer field count is %d less than %d (define) in filetype_id %s.",i+1,field_count,filetype_id);
	  	      throw CF_CErrorFile('E','M',ERR_COUNT_LESS,0,Tmpss,__FILE__,__LINE__);
            }
          }
        }

        if(Txt_Fmt[i+1].col_end[0] != '@')
        {
          if((ss2=strstr(Field[i].field,Txt_Fmt[i+1].col_end)) != NULL)
          {
            *ss2 = '\0';
            Txt_Fmt[i+1].col_reallen=ss2-Field[i].field;
          }
        }
	}

  	  for(int k=0;k<field_count;k++)
  	  {
  	    int tmpk=k+1;
  	   	if(Txt_Fmt[tmpk].col_log=='L')
  	   	  Txt_Fmt[tmpk].col_reallen=DeleteFiller(Field[k].field,Txt_Fmt[tmpk].col_filler,Txt_Fmt[tmpk].col_reallen);
  	   	else if(Txt_Fmt[tmpk].col_log=='M')
  	   	{
  	   	  Txt_Fmt[tmpk].col_reallen=DeleteFiller(Field[k].field,Txt_Fmt[tmpk].col_filler,Txt_Fmt[tmpk].col_reallen);
		  Txt_Fmt[tmpk].col_reallen=DeleteHFiller(Field[k].field,Txt_Fmt[tmpk].col_filler,Txt_Fmt[tmpk].col_reallen);
  	   	}
  	   	else Txt_Fmt[tmpk].col_reallen=DeleteHFiller(Field[k].field,Txt_Fmt[tmpk].col_filler,Txt_Fmt[tmpk].col_reallen);
	  }

  	if(flag) 
  	{
  		sprintf(Tmpss,"the buffer length is %d longer than %d (define) in filetype_id %s.",strlen(buf),buflen,filetype_id);
  		throw CF_CErrorFile('E','M',ERR_LENGTH_LONG,0,Tmpss,__FILE__,__LINE__);
  		return (-1);
  	}

	return 0;
}







return (-1);
}

int CFmt_Change::Check_FieldLen(int count)
{
	if(strlen(Field[count-1].field)>Txt_Fmt[count].col_len)
	{
	return (-1);
	}
	return 0;
}
int CFmt_Change::Get_FieldIndex(char *cname)
{
for(int ii=1;ii<=field_count;ii++)
	{
	if(!strcmp(cname,Txt_Fmt[ii].colname))
		{return ii;}
	}
return (-1);
}

int CFmt_Change::Get_FieldLen(int count)
{
if((count>field_count)||(count<=0))
	{
	sprintf(Tmpss,"the array count %dth in filetype_id %s over uplimit",count,filetype_id);
  	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
	return (-1);
	}
	 return Txt_Fmt[count].col_len;
}
int CFmt_Change::Get_Field(int count, char *array)
{
if((count>field_count)||(count<=0))
	{
	sprintf(Tmpss,"the array count %dth in filetype_id %s over uplimit",count,filetype_id);
  	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
	return (-1);
	}

	 strcpy(array,Field[count-1].field);

return 0;
}
int CFmt_Change::Get_Field(char *cname,char *array)
{int count;

	count=Get_FieldIndex(cname);
	
    if((count>field_count)||(count<=0))
	{
	sprintf(Tmpss,"the array count %dth in filetype_id %s over uplimit",count,filetype_id);
  	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
	return (-1);
	}
		 strcpy(array,Field[count-1].field);

	 return 0;
}

char* CFmt_Change::Get_Field(int count)
{
  if((count>field_count)||(count<=0))
  {
	sprintf(Tmpss,"the array count %dth in filetype_id %s over uplimit",count,filetype_id);
  	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
	return NULL;
  }
  return Field[count-1].field;
}

char* CFmt_Change::Get_FieldName(int count)
{
if((count>field_count)||(count<=0))
	{
	sprintf(Tmpss,"the array count %dth in filetype_id %s over uplimit",count,filetype_id);
  	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
	return NULL;
	}

	 return Txt_Fmt[count].colname;
}

char* CFmt_Change::Get_Fillerfield(int count)
{
 char temp[string_len];
 char tmpstr[string_len];
 
 if((count>field_count)||(count<=0))
	{
	sprintf(Tmpss,"the array count %dth in filetype_id %s over uplimit",count,filetype_id);
  	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
	return NULL;
	}
 	 if(Txt_Fmt[count].col_log=='R')
 		{
 		int num=Txt_Fmt[count].col_len-strlen(Field[count-1].field);
 		memset(tmpstr,Txt_Fmt[count].col_filler,num);
 		tmpstr[num]='\0';
 		sprintf(temp,"%s%s",tmpstr,Field[count-1].field);
 	 	}
 	 else {
 		int num=Txt_Fmt[count].col_len-strlen(Field[count-1].field);
 		memset(tmpstr,Txt_Fmt[count].col_filler,num);
 		tmpstr[num]='\0';
 		sprintf(temp,"%s%s",Field[count-1].field,tmpstr);
 	 	}
 	 return temp;
}



int  CFmt_Change::Get_Fillerfield(int count,char *temp,int len)
{

 char tmpstr[string_len];

 if((count>field_count)||(count<=0))
	{
	sprintf(Tmpss,"the array count %dth in filetype_id %s over uplimit",count,filetype_id);
  	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
	return (-1);
	}
 if(Txt_Fmt[count].col_len>len)
 	{
	sprintf(Tmpss,"the array count %dth in filetype_id %s inputlen(%d) less than fieldlen(%d)",count,filetype_id,len,Txt_Fmt[count].col_len);
  	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
	return (-1);
 	}
 
 	 if(Txt_Fmt[count].col_log=='R')
 		{
 		int num=Txt_Fmt[count].col_len-strlen(Field[count-1].field);
 		memset(tmpstr,Txt_Fmt[count].col_filler,num);
 		tmpstr[num]='\0';
 		sprintf(temp,"%s%s",tmpstr,Field[count-1].field);
 	 	}
 	 else {
 		int num=Txt_Fmt[count].col_len-strlen(Field[count-1].field);
 		memset(tmpstr,Txt_Fmt[count].col_filler,num);
 		tmpstr[num]='\0';
 		sprintf(temp,"%s%s",Field[count-1].field,tmpstr);
 	 	}
 	 return 0;
}
//added by tanj 20050908
int  CFmt_Change::Get_Fillerfield(int count,char *temp,int len, char filler)
{

 char tmpstr[string_len];

 if((count>field_count)||(count<=0))
	{
  	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,(char *)"the array count over uplimit.",__FILE__,__LINE__);
	return (-1);
	}
 if(Txt_Fmt[count].col_len>len)
 	{
 	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,(char *)"the array count over uplimit.",__FILE__,__LINE__);
	return (-1);
 	}
 
 	 if(Txt_Fmt[count].col_log=='R')
 		{
 		int num=Txt_Fmt[count].col_len-strlen(Field[count-1].field);
 		memset(tmpstr,filler,num);
 		tmpstr[num]='\0';
 		sprintf(temp,"%s%s",tmpstr,Field[count-1].field);
 	 	}
 	 else {
 		int num=Txt_Fmt[count].col_len-strlen(Field[count-1].field);
 		memset(tmpstr,filler,num);
 		tmpstr[num]='\0';
 		sprintf(temp,"%s%s",Field[count-1].field,tmpstr);
 	 	}
 	 return 0;
}
//end of addition

int CFmt_Change::Set_Field(int count, char *array)
{
	if((count>field_count)||(count<=0))
	{
	sprintf(Tmpss,"the array count %dth in filetype_id %s over uplimit",count,filetype_id);
  	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
	return (-1);
	}

	Field[count-1].field[0]='\0';
	int tmplen=strlen(array);
	if(tmplen>Txt_Fmt[count].col_len)
	{
	sprintf(Tmpss,"the array %dth in filetype_id %s length(%d) is longer than %d(define)",count,filetype_id,strlen(array),Txt_Fmt[count].col_len);
  	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
	return (-1);
	}
	 strcpy(Field[count-1].field,array);
	 Txt_Fmt[count].col_reallen = tmplen;
	 char *ss2;
	 if(Txt_Fmt[count].col_end[0] != '@')
     {
       if((ss2=strstr(Field[count-1].field,Txt_Fmt[count].col_end)) != NULL)
       {
         *ss2 = '\0';
         Txt_Fmt[count].col_reallen=ss2-Field[count-1].field;
       }
     }

   	if(Txt_Fmt[count].col_log=='L')
   	  Txt_Fmt[count].col_reallen=DeleteFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);
   	else if(Txt_Fmt[count].col_log=='M')
   	{
   	  Txt_Fmt[count].col_reallen=DeleteFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);
      Txt_Fmt[count].col_reallen=DeleteHFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);
   	}
   	else Txt_Fmt[count].col_reallen=DeleteHFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);



	char *ss;
/*	if(( ss = strchr( Field[count-1].field,Txt_Fmt[count].col_seperator ) ) != NULL)
	{
	sprintf(Tmpss,"the array %dth in filetype_id %s contains seperator %c",count,filetype_id,Txt_Fmt[count].col_seperator);
  	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
	}
*/
  if(Txt_Fmt[count].col_seperator==NULL) return 0;

  while(1)
  {
	if(( ss = strchr( Field[count-1].field,Txt_Fmt[count].col_seperator ) ) != NULL)
	{
      *ss=SEPERATOR_INSTEAD;
	}
	else break;
  }



  
return 0;

}

int CFmt_Change::Set_Field(char *cname,char *array)
{int count;

	count=Get_FieldIndex(cname);
	
	if((count>field_count)||(count<=0))
	{
	sprintf(Tmpss,"the array count %dth in filetype_id %s over uplimit",count,filetype_id);
  	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
	return (-1);
	}
	
	
	Field[count-1].field[0]='\0';
	int tmplen=strlen(array);
	if(tmplen>Txt_Fmt[count].col_len)
	{
	sprintf(Tmpss,"the array %dth in filetype_id %s length(%d) is longer than %d(define)",count,filetype_id,strlen(array),Txt_Fmt[count].col_len);
  	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
	return (-1);
	}
	 strcpy(Field[count-1].field,array);
	 Txt_Fmt[count].col_reallen = tmplen;
	 char *ss2;
	 if(Txt_Fmt[count].col_end[0] != '@')
     {
       if((ss2=strstr(Field[count-1].field,Txt_Fmt[count].col_end)) != NULL)
       {
         *ss2 = '\0';
         Txt_Fmt[count].col_reallen=ss2-Field[count-1].field;
       }
     }
	 
  	   	if(Txt_Fmt[count].col_log=='L')
  	   	  Txt_Fmt[count].col_reallen=DeleteFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);
  	   	else if(Txt_Fmt[count].col_log=='M')
  	   	{
  	   	  Txt_Fmt[count].col_reallen=DeleteFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);
		  Txt_Fmt[count].col_reallen=DeleteHFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);
  	   	}
  	   	else Txt_Fmt[count].col_reallen=DeleteHFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);

char *ss;
/*if(( ss = strchr( Field[count-1].field,Txt_Fmt[count].col_seperator ) ) != NULL)
{
	sprintf(Tmpss,"the array %dth in filetype_id %s contains seperator %c",count,filetype_id,Txt_Fmt[count].col_seperator);
  	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
}
*/
  if(Txt_Fmt[count].col_seperator==NULL) return 0;

  while(1)
  {
	if(( ss = strchr( Field[count-1].field,Txt_Fmt[count].col_seperator ) ) != NULL)
	{
      *ss=SEPERATOR_INSTEAD;
	}
	else break;
  }

  
  return 0;
}

int CFmt_Change::Field_Add(int count,char *array,char Sep)
{
    if((count>field_count)||(count<=0))
	{
	sprintf(Tmpss,"the array count %dth in filetype_id %s over uplimit",count,filetype_id);
  	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
	return (-1);
	}

//Field[count-1].field[0]=0;
    int Field_Len=0;
    int array_Len=0;
    Field_Len=Txt_Fmt[count].col_reallen;//strlen(Field[count-1].field);
    array_Len=strlen(array);

    if(Field_Len)
    {
      if((array_Len+Field_Len+1)>Txt_Fmt[count].col_len)
	  {
	    sprintf(Tmpss,"the array %dth in filetype_id %s length(%d) is longer than %d(define)",count,filetype_id,strlen(array)+strlen(Field[count-1].field)+1,Txt_Fmt[count].col_len);
  	    throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
	    return (-1);
	   }
       if(array_Len)
       {
         sprintf(Field[count-1].field,"%s%c%s",Field[count-1].field,Sep,array);
         Txt_Fmt[count].col_reallen += 1+array_Len;
  	   	 if(Txt_Fmt[count].col_log=='L')
  	   	   Txt_Fmt[count].col_reallen=DeleteFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);
  	   	 else if(Txt_Fmt[count].col_log=='M')
  	   	 {
  	   	   Txt_Fmt[count].col_reallen=DeleteFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);
		   Txt_Fmt[count].col_reallen=DeleteHFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);
  	   	 }
  	   	 else Txt_Fmt[count].col_reallen=DeleteHFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);
       }
       else
       {
         sprintf(Field[count-1].field,"%s%c%c",Field[count-1].field,Sep,ADD_NIL_DEFAULT);
         Txt_Fmt[count].col_reallen += 2;
       }	   
	}//	 DeleteHFiller(Field[count-1].field,Txt_Fmt[count].col_filler);
	else
	{
       if((array_Len)>Txt_Fmt[count].col_len)
	   {
	    sprintf(Tmpss,"the array %dth in filetype_id %s length(%d) is longer than %d(define)",count,filetype_id,strlen(array),Txt_Fmt[count].col_len);
  	    throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
	    return (-1);
	   }
       if(array_Len)
       {
	     sprintf(Field[count-1].field,"%s",array);
	     Txt_Fmt[count].col_reallen = array_Len;
  	   	 if(Txt_Fmt[count].col_log=='L')
  	   	   Txt_Fmt[count].col_reallen=DeleteFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);
  	   	 else if(Txt_Fmt[count].col_log=='M')
  	   	 {
  	   	   Txt_Fmt[count].col_reallen=DeleteFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);
		   Txt_Fmt[count].col_reallen=DeleteHFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);
  	   	 }
  	   	 else Txt_Fmt[count].col_reallen=DeleteHFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);

       }
       else
       {
         Field[count-1].field[0]=ADD_NIL_DEFAULT;
         Field[count-1].field[1]='\0';
         Txt_Fmt[count].col_reallen = 1;
       }
     }

	 char *ss2;
	 if(Txt_Fmt[count].col_end[0] != '@')
     {
       if((ss2=strstr(Field[count-1].field,Txt_Fmt[count].col_end)) != NULL)
       {
         *ss2 = '\0';
         Txt_Fmt[count].col_reallen=ss2-Field[count-1].field;
       }
     }

char *ss;
/*if(( ss = strchr( Field[count-1].field,Txt_Fmt[count].col_seperator ) ) != NULL)
{
	sprintf(Tmpss,"the array %dth in filetype_id %s contains seperator %c",count,filetype_id,Txt_Fmt[count].col_seperator);
  	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
}
*/
  if(Txt_Fmt[count].col_seperator==NULL) return 0;

  while(1)
  {
	if(( ss = strchr( Field[count-1].field,Txt_Fmt[count].col_seperator ) ) != NULL)
	{
      *ss=SEPERATOR_INSTEAD;
	}
	else break;
  }


return 0;

}


int CFmt_Change::Field_Add(int count,char *array)
{
    if((count>field_count)||(count<=0))
	{
	sprintf(Tmpss,"the array count %dth in filetype_id %s over uplimit",count,filetype_id);
  	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
	return (-1);
	}

    int Field_Len=0;
    int array_Len=0;
    Field_Len=Txt_Fmt[count].col_reallen;//strlen(Field[count-1].field);
    array_Len=strlen(array);

    if((array_Len+Field_Len)>Txt_Fmt[count].col_len)
	{
	  sprintf(Tmpss,"the array %dth in filetype_id %s length(%d) is longer than %d(define)",count,filetype_id,array_Len+Field_Len,Txt_Fmt[count].col_len);
  	  throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
	  return (-1);
	}
    sprintf(Field[count-1].field,"%s%s",Field[count-1].field,array);
    Txt_Fmt[count].col_reallen += array_Len;
    
  	if(Txt_Fmt[count].col_log=='L')
  	  Txt_Fmt[count].col_reallen=DeleteFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);
  	else if(Txt_Fmt[count].col_log=='M')
  	{
  	  Txt_Fmt[count].col_reallen=DeleteFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);
	  Txt_Fmt[count].col_reallen=DeleteHFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);
  	}
  	else Txt_Fmt[count].col_reallen=DeleteHFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);

         
	char *ss2;
	if(Txt_Fmt[count].col_end[0] != '@')
    {
      if((ss2=strstr(Field[count-1].field,Txt_Fmt[count].col_end)) != NULL)
      {
        *ss2 = '\0';
        Txt_Fmt[count].col_reallen=ss2-Field[count-1].field;
      }
    }


char *ss;
/*if(( ss = strchr( Field[count-1].field,Txt_Fmt[count].col_seperator ) ) != NULL)
{
	sprintf(Tmpss,"the array %dth in filetype_id %s contains seperator %c",count,filetype_id,Txt_Fmt[count].col_seperator);
  	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
}
*/

  if(Txt_Fmt[count].col_seperator==NULL) return 0;
  while(1)
  {
	if(( ss = strchr( Field[count-1].field,Txt_Fmt[count].col_seperator ) ) != NULL)
	{
      *ss=SEPERATOR_INSTEAD;
	}
	else break;
  }
	
return 0;

}


int CFmt_Change::Get_Orecord(char *record,int len)
{
	int k=strlen(TMP);
	TMP[k]='\0';
	if(k>len)
		{
    	sprintf(Tmpss,"the inputlen(%d) less than recordlen(%d) in filetype_id %s ",len,k,filetype_id);
  	    throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
		return (-1);
		}	
strcpy(record,TMP);
return 0;	
}

char* CFmt_Change::Get_Orecord()
{
	return TMP;
}

char* CFmt_Change::Get_record()
{ 
  char tmp[record_len];
  Get_record(tmp,record_len);
  return tmp;
}


int CFmt_Change::Get_record(char *record,int len)
{ 
  char tmp[record_len];  		
  int k;
 if(record_type=='C')
 {

    int nLen;
    char *pbuf=tmp;
	for(int i=0;i<field_count;i++)
	 {
        nLen = Txt_Fmt[i+1].col_reallen;
        memcpy( pbuf, Field[i].field, nLen);
        pbuf += nLen;
        if(Txt_Fmt[i+1].col_seperator==NULL)
        {
          *pbuf=' ';
        }
        else *pbuf=Txt_Fmt[i+1].col_seperator;
        pbuf++;
	}
	*(pbuf-1) = '\0';
	
	k=strlen(tmp);
//	tmp[k]='\0';
	if(k>len)
	{
    	sprintf(Tmpss,"the inputlen(%d) less than recordlen(%d) in filetype_id %s ",len,k,filetype_id);
  	    throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
		return (-1);
	}
//	sprintf(record,"%s",tmp);
	memcpy(record,tmp,k+1);
	return 0;
 }
 if(record_type=='A')
 {
   char temp[string_len];
   char tmpstr[string_len];
   int   num;

   memset(tmp,0,record_len);
   for(int i=0;i<field_count;i++)
   {
     num=Txt_Fmt[i+1].col_len-strlen(Field[i].field);
 	 memset(tmpstr,Txt_Fmt[i+1].col_filler,num);
 	 tmpstr[num]='\0';
 	 if(Txt_Fmt[i+1].col_log=='R')
 	   sprintf(tmp,"%s%s%s",tmp,tmpstr,Field[i].field);
     else sprintf(tmp,"%s%s%s",tmp,Field[i].field,tmpstr);
    }
    
	k=strlen(tmp);
	tmp[k]='\0';
	if(k>len)
	{
    	sprintf(Tmpss,"the inputlen(%d) less than recordlen(%d) in filetype_id %s ",len,k,filetype_id);
  	    throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
		return (-1);
	}
	sprintf(record,"%s",tmp);
	return 0;
 }

 if(record_type=='D')
 {
   char temp[string_len];
   char tmpstr[string_len];
   int   num;

   memset(tmp,0,record_len);
   for(int i=0;i<(field_count-1);i++)
   {
     num=Txt_Fmt[i+1].col_len-strlen(Field[i].field);
 	 memset(tmpstr,Txt_Fmt[i+1].col_filler,num);
 	 tmpstr[num]='\0';
 	 if(Txt_Fmt[i+1].col_log=='R')
 	 {
 	   if(Txt_Fmt[i+1].col_seperator==NULL)
 	   	 sprintf(tmp,"%s%s%s",tmp,tmpstr,Field[i].field);
 	   else sprintf(tmp,"%s%s%s%c",tmp,tmpstr,Field[i].field,Txt_Fmt[i+1].col_seperator);
 	 }
     else
     {
 	   if(Txt_Fmt[i+1].col_seperator==NULL)
 	   	 sprintf(tmp,"%s%s%s",tmp,Field[i].field,tmpstr);
 	   else sprintf(tmp,"%s%s%s%c",tmp,Field[i].field,tmpstr,Txt_Fmt[i+1].col_seperator);
 	 }
    }

     num=Txt_Fmt[field_count].col_len-strlen(Field[field_count-1].field);
 	 memset(tmpstr,Txt_Fmt[field_count].col_filler,num);
 	 tmpstr[num]='\0';
 	 if(Txt_Fmt[field_count].col_log=='R')
 	   sprintf(tmp,"%s%s%s",tmp,tmpstr,Field[field_count-1].field);
     else sprintf(tmp,"%s%s%s",tmp,Field[field_count-1].field,tmpstr);

    
	k=strlen(tmp);
	tmp[k]='\0';
	if(k>len)
	{
    	sprintf(Tmpss,"the inputlen(%d) less than recordlen(%d) in filetype_id %s ",len,k,filetype_id);
  	    throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
		return (-1);
	}
	sprintf(record,"%s",tmp);
	return 0;
 }

return (-1);

}




int CFmt_Change::DeleteFiller( char *ss ,char filler,int len)

{

 int i;

    if(len) i = len -1;
    else i = strlen(ss)-1;


    //非数字字符
    if(filler=='*')
    {
      while ( (i>=0) && (((ss[i]&0x00ff)<48)||((ss[i]&0x00ff)>57)) ) i--;
      ss[i+1] = '\0';
      return(i+1);
    }
//默认ss必须有值否则会保留一个空格
    while ( (i>=0) && ss[i] == filler ) i--;
    ss[i+1] = '\0';
    return(i+1);

}



int CFmt_Change::DeleteHFiller( char *ss,char filler,int len )
{

 int i;

 char ss1[10000];

    strcpy( ss1,ss ); i = 0;


    //非数字字符
    if(filler=='*')
    {
      while ( (ss1[i]!=0) && (((ss1[i]&0x00ff)<48)||((ss1[i]&0x00ff)>57)) ) i++;
      strcpy( ss,ss1+i );
      return((len-i));
    }
    
    while( ss1[i] == filler ) i++;
    strcpy( ss,ss1+i );
    return(len-i);

}
//去除中文中含有分隔符
char* CFmt_Change::strchrdul( char *ss ,char sep)
{
  char *ss0;
  ss0=ss;
  while(1)
  {
    if(*ss0==0) return NULL;
    if(*ss0==sep) return ss0;
	if((*ss0<0)||(*ss0>127) )
	{
      ss0=ss0+2;
      continue;
	}
	ss0++;
  }
  return NULL;
}


int CFmt_Change::SplitBuf( char *ss )

{

  int i;

  char *ss0,*ss1,*ss2;

  i = 0; ss0 = ss;
  int isslen=strlen(ss);
  char *ss3=NULL;
     while( 1 ) {

		int tmpi=i+1; 

        if(Txt_Fmt[tmpi].col_seperator==NULL)
        {
//          ss0=ss+Txt_Fmt[tmpi].col_bigen-1;
          if(((ss0-ss)+Txt_Fmt[tmpi].col_len)>=isslen) break;
          ss1=ss0+Txt_Fmt[tmpi].col_len;
        }
        else if(( ss1 = strchrdul( ss0,Txt_Fmt[tmpi].col_seperator ) ) == NULL) break;

		Txt_Fmt[tmpi].col_reallen=ss1-ss0;
        memcpy(Field[i].field,ss0,Txt_Fmt[tmpi].col_reallen);
        Field[i].field[Txt_Fmt[tmpi].col_reallen]='\0';

        if(Txt_Fmt[tmpi].col_end[0] != '@')
        {
          if((ss2=strstr(Field[i].field,Txt_Fmt[tmpi].col_end)) != NULL)
          {
            *ss2 = '\0';
            Txt_Fmt[tmpi].col_reallen=ss2-Field[i].field;
          }
        }

        if(Txt_Fmt[tmpi].col_seperator==NULL)
          ss0=ss1;
        else ss0 = ss1+1;

		i++;
		if(i>=field_count) 
		{
		  if(check_field_num != 'N')
		  {
		    real_field_count=tmpi;
		    return (-1);
		  }
		  return (0);
		}
     }

     sprintf( Field[i].field,"%s",ss0 ); 
     int tmplen=strlen(Field[i].field);
     
     if(Field[i].field[tmplen-1]=='\r')
     {
     	Field[i].field[tmplen-1]='\0';
     	Txt_Fmt[i+1].col_reallen=tmplen-1;
     }
     else Txt_Fmt[i+1].col_reallen=tmplen;
     
     if(Txt_Fmt[i+1].col_end[0] != '@')
     {
       if((ss2=strstr(Field[i].field,Txt_Fmt[i+1].col_end)) != NULL)
       {
         *ss2 = '\0';
         Txt_Fmt[i+1].col_reallen=ss2-Field[i].field;
       }
     }
if(i==(field_count-1))
 return(0);

real_field_count=i+1;

return (-1);

}

int CFmt_Change::Get_fieldcount()
{
	return field_count;
}

char CFmt_Change::Get_FieldFmt(int count)
{
return Txt_Fmt[count].col_fmt;
}

//20061010 add by zhoulh
int CFmt_Change::Get_FieldErrtypeIdx(int count)
{
return Txt_Fmt[count].col_errtype_index;
}

int CFmt_Change::Get_FieldCheckLen(int count)
{
return Txt_Fmt[count].check_len;
}
int CFmt_Change::Get_RealLen(int count)
{
return Txt_Fmt[count].col_reallen;
}

char CFmt_Change::Get_FieldSep(int count)
{
return Txt_Fmt[count].col_seperator;
}

int CFmt_Change::Clear_Field()
{
  for(int i=0;i<field_count;i++)
  {
	memset(Field[i].field,0,Txt_Fmt[i+1].col_len);
	Txt_Fmt[i+1].col_reallen=0;
  }
  return 0;
}

char* CFmt_Change::Get_ChecktimeField()
{
	for(int i=1;i<=field_count;i++)
	{
	if(Txt_Fmt[i].col_fmt=='C')
		return Field[i-1].field;
	}
	return NULL;
}


//20071015 增加字段标志表示时长（T），费用（F），增加接口取得时长和费用


//add by weixy 20071015
int CFmt_Change::Get_Time()
{
	int Duration=0;
for(int i=1;i<=field_count;i++)
	{
	if(Txt_Fmt[i].col_fmt=='T')
		Duration=atoi(Field[i-1].field);
	}
return Duration;
}


int  CFmt_Change::Get_Fee()
{
	int Fee=0;
for(int i=1;i<=field_count;i++)
	{
	if(Txt_Fmt[i].col_fmt=='F')
		Fee+=atoi(Field[i-1].field);
	}
	
 return Fee;

}
//end add by weixy 20071015






Txt_Fmt::Txt_Fmt()
{
  pCFmt_Change=NULL;
  iMax_Fmt_Count=MAX_FMT_TYPE;
  iCur_Fmt_Count=0;
  pCFmt_Change = new CFmt_Change[iMax_Fmt_Count];
    
}

Txt_Fmt::~Txt_Fmt()
{
  if(pCFmt_Change)
  	delete[] pCFmt_Change;
  pCFmt_Change=NULL;
}

CFmt_Change* Txt_Fmt::Get_Fmt(char *FileTypeId)
{
  int Found=0;
  int i=0;
  if(iCur_Fmt_Count)
  {
    if(!strcmp(FileTypeId,pCurCFmt_Change->Get_id()))
    {
      return pCurCFmt_Change;
    }
  	
    for( i = 0;i<iCur_Fmt_Count;i++)
    {
      if(!strcmp(FileTypeId,pCFmt_Change[i].Get_id()))
      {
        pCurCFmt_Change = &pCFmt_Change[i];
        return pCurCFmt_Change;
      }
    }
  }
  if(Add_Fmt(FileTypeId)) return NULL;
  pCurCFmt_Change = &pCFmt_Change[i];
  return pCurCFmt_Change;
}

int Txt_Fmt::Add_Fmt(char *FileTypeId)
{
  if(iCur_Fmt_Count == iMax_Fmt_Count)
  {
    CFmt_Change *Tmp_Fmt_Point;
    iMax_Fmt_Count += MAX_FMT_TYPE; 
    Tmp_Fmt_Point = new CFmt_Change[iMax_Fmt_Count];
    if(!Tmp_Fmt_Point) return -1;
    for(int i = 0;i<iCur_Fmt_Count;i++)
    {
      Tmp_Fmt_Point[i] = pCFmt_Change[i];
    }
    delete []pCFmt_Change;
    pCFmt_Change = Tmp_Fmt_Point;
    Tmp_Fmt_Point = NULL;
  }

  pCFmt_Change[iCur_Fmt_Count].Init(FileTypeId);
  iCur_Fmt_Count++;
  
  return 0;
}


//added by tanj 20060915
int CFmt_Change::Get_Field(const char *cname,char *array)
{int count;

	count=Get_FieldIndex((char *)cname);
	
    if((count>field_count)||(count<=0))
	{
	sprintf(Tmpss,"the array count %dth in filetype_id %s over uplimit",count,filetype_id);
  	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
	return (-1);
	}
		 strcpy(array,Field[count-1].field);

	 return 0;
}

char* CFmt_Change::Get_Field(const char *cname)
{
	int count = Get_FieldIndex((char *)cname);
	return Get_Field(count);
}



int CFmt_Change::Set_Field(int count,const char *array)
{
if((count>field_count)||(count<=0))
	{
	sprintf(Tmpss,"the array count %dth in filetype_id %s over uplimit",count,filetype_id);
  	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
	return (-1);
	}

	Field[count-1].field[0]='\0';
	int tmplen=strlen(array);
	if(tmplen>Txt_Fmt[count].col_len)
	{
	sprintf(Tmpss,"the array %dth in filetype_id %s length(%d) is longer than %d(define)",count,filetype_id,strlen(array),Txt_Fmt[count].col_len);
  	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
	return (-1);
	}
	 strcpy(Field[count-1].field,array);
	 Txt_Fmt[count].col_reallen = tmplen;
	 char *ss2;
	 if(Txt_Fmt[count].col_end[0] != '@')
     {
       if((ss2=strstr(Field[count-1].field,Txt_Fmt[count].col_end)) != NULL)
       {
         *ss2 = '\0';
         Txt_Fmt[count].col_reallen=ss2-Field[count-1].field;
       }
     }

  if(Txt_Fmt[count].col_log=='L')
    Txt_Fmt[count].col_reallen=DeleteFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);
  else if(Txt_Fmt[count].col_log=='M')
  {
    Txt_Fmt[count].col_reallen=DeleteFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);
	Txt_Fmt[count].col_reallen=DeleteHFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);
  }
  else Txt_Fmt[count].col_reallen=DeleteHFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);
 
return 0;

}

int CFmt_Change::Set_Field(char *cname,const char *array)
{int count;

	count=Get_FieldIndex((char *)cname);
	
if((count>field_count)||(count<=0))
	{
	sprintf(Tmpss,"the array count %dth in filetype_id %s over uplimit",count,filetype_id);
  	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
	return (-1);
	}
	
	
  Field[count-1].field[0]='\0';
  int tmplen=strlen(array);
  if(tmplen>Txt_Fmt[count].col_len)
	{
	sprintf(Tmpss,"the array %dth in filetype_id %s length(%d) is longer than %d(define)",count,filetype_id,strlen(array),Txt_Fmt[count].col_len);
  	throw CF_CErrorFile('E','M',ERR_INPUTCOUNT_OVER_UPLIMIT,0,Tmpss,__FILE__,__LINE__);
	return (-1);
	}
	 strcpy(Field[count-1].field,array);
	 Txt_Fmt[count].col_reallen=tmplen;
	 char *ss2;
	 if(Txt_Fmt[count].col_end[0] != '@')
     {
       if((ss2=strstr(Field[count-1].field,Txt_Fmt[count].col_end)) != NULL)
       {
         *ss2 = '\0';
         Txt_Fmt[count].col_reallen=ss2-Field[count-1].field;
       }
     }
	 
  if(Txt_Fmt[count].col_log=='L')
    Txt_Fmt[count].col_reallen=DeleteFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);
  else if(Txt_Fmt[count].col_log=='M')
  {
    Txt_Fmt[count].col_reallen=DeleteFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);
	Txt_Fmt[count].col_reallen=DeleteHFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);
  }
  else Txt_Fmt[count].col_reallen=DeleteHFiller(Field[count-1].field,Txt_Fmt[count].col_filler,Txt_Fmt[count].col_reallen);
 
  return 0;
}

//end of added by tanj 20060915

