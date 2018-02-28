/*******************************************************************************************
*CF_CCdrTransformer.Cpp
*created by tanj 2005.1.4
*20050722 tanj   CFmt_Change��Ŀ������캯�����ڴ�й©������ ����Copy_Record����
*******************************************************************************************/


#include "CF_CCdrTransformer.h"


CF_CCdrTransformer::CF_CCdrTransformer(char *filetype_from, char *filetype_to)
{
	strcpy(CF_CCdrTransformer::Filetype_From, filetype_from);
	strcpy(CF_CCdrTransformer::Filetype_To, filetype_to);
	fieldMap.Init(filetype_to, filetype_from);  //ע�⣺filetype_to���������ļ�����
}
CF_CCdrTransformer::~CF_CCdrTransformer()
{
}

void CF_CCdrTransformer::cdrTransform(char *cdrFrom, CFmt_Change &fmtTo)
{
	CFmt_Change fmtFrom;
	fmtFrom.Init(Filetype_From);
	fmtFrom.Set_record(cdrFrom);
	cdrTransform(fmtFrom, fmtTo);
}
void CF_CCdrTransformer::cdrTransform(char *cdrFrom, char *cdrTo)
{
	CFmt_Change fmtFrom, fmtTo;
	fmtFrom.Init(Filetype_From);
	fmtTo.Init(Filetype_To);
	fmtFrom.Set_record(cdrFrom);
	cdrTransform(fmtFrom, fmtTo);
	fmtTo.Get_record(cdrTo, 1000);
}
void CF_CCdrTransformer::cdrTransform(CFmt_Change &fmtFrom, char *cdrTo)
{
	CFmt_Change fmtTo;
	fmtTo.Init(Filetype_To);
	cdrTransform(fmtFrom, fmtTo);	
	fmtTo.Get_record(cdrTo, 1000);
}

void CF_CCdrTransformer::cdrTransform(CFmt_Change &fmtFrom, CFmt_Change &fmtTo)
{
	if (strcmp(Filetype_From, Filetype_To) == 0)
	{
		//fmtTo = fmtFrom;
		//modified by tanj 20050722  CFmt_Change��Ŀ������캯�����ڴ�й©������
		fmtTo.Copy_Record(fmtFrom);
		return;
	}
	
	//CFmt_Change fmtFrom;
	//CFmt_Change fmtTo;
	//fmtFrom.Init(CF_CCdrTransformer::Filetype_From);
	//fmtTo.Init(CF_CCdrTransformer::Filetype_To);
	
	//fmtFrom.Set_record(cdrFrom);
	
  CBindSQL bs(DBConn);
  long col_index_num = -1;
  long col_index_from;
  char sqltmp[500];
  sprintf(sqltmp, "select count(*)from txtfile_fmt where filetype_id = '%s'",Filetype_To);
  bs.Open(sqltmp, SELECT_QUERY);
  if (!(bs>>col_index_num))
  {
  }
  for (long i = 1; i <= col_index_num; i++)
  {
  	col_index_from = fieldMap.Get_Index(i);
  	if (col_index_from >= 0)
  	{
  		fmtTo.Set_Field(i, fmtFrom.Get_Field(col_index_from));
  	}
  	else
  	{
  		fmtTo.Set_Field(i, "");       //û���ֶ����Ӧ������ַ���
  	}
  }
  
  //fmtTo.Get_record(cdrTo, 1000);
}
  
  	
    












