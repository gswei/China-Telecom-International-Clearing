#include <time.h>
#include <assert.h>
#include "COracleDB.h"

#ifndef _CFmt_Change_h_
#define _CFmt_Change_h_ 1
#define string_len					400
#define record_len					2000

#define MAX_FMT_TYPE				50
#define ADD_NIL_DEFAULT				'-'
#define SEPERATOR_INSTEAD    		'*'
//clock_t testr_head,testr_part1,testr_part2,testr_part3,testr_part4;
//clock_t testr_all,testr_all1;

struct FIELD
{
char field[string_len];
};

struct TXT_FMT
{
 int col_index;
 int col_bigen;
 int col_len;
 int col_reallen;
 char colname[21];
 char col_seperator;
 char col_log;
 char col_filler;
 char col_fmt;
 char col_end[6];
 int   check_len;
 //20061010 add by zhoulh
 int col_errtype_index;
};



class CFmt_Change
{
private:


TXT_FMT *Txt_Fmt;
FIELD *Field;
char record_type;
char check_field_num;
int Record_Len;
int field_count;
char filetype_id[6];



char Tmpss[1024];
int real_field_count;

int strlen(const char *str);
char* strchrdul( char *ss ,char sep);
int SplitBuf( char *ss);
int DeleteHFiller( char *ss,char filler,int len );
int DeleteFiller( char *ss ,char filler,int len =0);
char TMP[20000];
public:
	CFmt_Change();
	CFmt_Change(const CFmt_Change &right);
	~CFmt_Change();

CFmt_Change & operator=(const CFmt_Change &right);



int Init(char *id,CBindSQL &SqlStr,char type='0');
int Init(char *id,char type='0');
char* Get_id();
int Copy_Record(CFmt_Change & dt_record);
int Set_record(char *buf,int len=0);
int Check_FieldLen(int count);
int Get_FieldIndex(char *);
int Get_FieldLen(int count);
char* Get_Field(int count);
char* Get_FieldName(int count);
char* Get_Fillerfield(int count);
int  Get_Fillerfield(int count,char *temp,int len);
int  Get_Fillerfield(int count,char *temp,int len, char filler);//added by tanj 20050908
int Get_Field(char *cname,char *array);

//added by tanj 20060915
int Get_Field(const char *cname,char *array);
char* Get_Field(const char *cname);
int Set_Field(int count, const char *array);
int Set_Field(char *cname,const char *array);
//end of added by tanj 20060915

int Get_Field(int count, char *array);
int Set_Field(int count, char *array);
int Set_Field(char *cname,char *array);
int Field_Add(int count,char *array,char Sep);
int Field_Add(int count,char *array);
int Get_record(char *record,int len);
int Get_fieldcount();
//20061010 add by zhoulh
int Get_FieldErrtypeIdx(int count);
//end of added
char Get_FieldFmt(int count);
int Get_FieldCheckLen(int count);
int Get_RealLen(int count);
char Get_FieldSep(int count);
int Clear_Field();
char* Get_ChecktimeField();
int Get_Time();
int  Get_Fee();




char* Get_record();
int Get_Orecord(char *record,int len);
char* Get_Orecord();
};


class Txt_Fmt
{
private:
  CFmt_Change *pCFmt_Change;
  CFmt_Change *pCurCFmt_Change;
  int iMax_Fmt_Count;
  int iCur_Fmt_Count;



public:
  Txt_Fmt();
  ~Txt_Fmt();
  CFmt_Change *Get_Fmt(char *FileTypeId);
  int Add_Fmt(char *FileTypeId);

};


#endif

