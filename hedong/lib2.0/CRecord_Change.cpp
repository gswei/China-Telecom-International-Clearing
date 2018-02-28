
#include "CRecord_Change.h"

/*
*	Function Name	: Input2Output
*	Description	:构造函数
*	Input param	:
*	Returns		:
*	complete	:2004/09/02
*/
CInput2Output::CInput2Output(int in,int out)
{
	In_Idx = in;
	Out_Idx = out;
}

/*
*	Function Name	: ~Input2Output
*	Description	:析构函数
*	Input param	:
*	Returns		:
*	complete	:2004/09/02
*/
CInput2Output::~CInput2Output()
{
}

int CRecord_Change::Init( char *in, char *out )
{
	int in_index,out_index;
	char errmsg[500];
	
	strcpy( InFileTypeID, in );
	strcpy( OutFileTypeID, out );
	
	CBindSQL SqlStr(DBConn);
	//根据输入输出文件格式ID取字段对应关系
	SqlStr.Open("select INPUT_INDEX,OUTPUT_INDEX from INPUT2OUTPUT \
			  where INPUT_ID = :INPUT_ID and OUTPUT_ID = :OUTPUT_ID ", SELECT_QUERY);
	SqlStr<<InFileTypeID<<OutFileTypeID;

	while( SqlStr>>in_index>>out_index )
	{
		//Input2Output *tmp = new Input2Output(in_index,out_index);
		CInput2Output tmp(in_index,out_index);
		FileType_Chg.push_back(tmp);
	}

	if(SqlStr.IsError()) 
	{
		sprintf( errmsg,"select from INPUT2OUTPUT err,in=%d,out=%d",InFileTypeID,OutFileTypeID );
		throw CF_CError(ERR_TYPE_DB,ERR_LEVEL_HIG,SELECT_ERROR_FROM_DB,SELECT_ERROR_FROM_DB,errmsg,__FILE__,__LINE__);
		return SELECT_ERROR_FROM_DB;
	}
	SqlStr.Close();
	return ( 0 );
}

int CRecord_Change::Rec_Change( CFmt_Change &inR, CFmt_Change &outR )
{
	char errmsg[500];
	/*********Modified by wulf 2005-09-09********/
	if (strcmp( InFileTypeID,inR.Get_id() ) )
	{
		sprintf( errmsg,"In_FileType_id %s not match the input Filetype_id %s ",InFileTypeID,inR.Get_id() );
		throw CF_CError(ERR_TYPE_DB,ERR_LEVEL_HIG,ERR_FILETYPE_MATCH,ERR_FILETYPE_MATCH,errmsg,__FILE__,__LINE__);
		return ERR_FILETYPE_MATCH;
	}

	if (strcmp( OutFileTypeID,outR.Get_id() ) )
	{
		sprintf( errmsg,"Out_FileType_id %s not match the output Filetype_id %s ",OutFileTypeID,outR.Get_id() );
		throw CF_CError(ERR_TYPE_DB,ERR_LEVEL_HIG,ERR_FILETYPE_MATCH,ERR_FILETYPE_MATCH,errmsg,__FILE__,__LINE__);
		return ERR_FILETYPE_MATCH;
	}
	
	VIn2Out::iterator it;
	/*********End of modified by wulf 2005-09-09**********/
	char tmpField[1001];
	
	//把输入话单写到输出话单类中
	
	for( it=FileType_Chg.begin(); it!=FileType_Chg.end(); it++ )
	{
		inR.Get_Field(it->get_In_Idx(),tmpField);
		outR.Set_Field(it->get_Out_Idx(),tmpField);
	}

	return ( 0 );
}		
