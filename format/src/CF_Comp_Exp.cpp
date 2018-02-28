
#include "CF_Comp_Exp.h"




extern char Field[MAX_FIELD_COUNT][FMT_MAX_FIELD_LEN];

/*
*	Function Name	: AddVariable
*	Description	:向动态编译器加参数
*	Input param	: 
*		InRcd     记录格式类对象,文本记录格式
*	Returns		: 0:成功;负数:失败
*	complete	:2005/10/18
*/
int Comp_Exp::AddVariable( CFmt_Change &InRcd )
{
    int iInFieldCount=InRcd.Get_fieldcount();
	for( int i=1;i<=iInFieldCount;i++ )
	{
		//对动态编译器增加变量
		m_Compile.DefineVariable( InRcd.Get_FieldName(i),InRcd.Get_Field(i) );
	}
	return ( 0 );
}


/*
*	Function Name	: Comp_Expression
*	Description	:用动态编译器解析表达式
*	Input param	: 
*		szExp    表达式组合字符串 
*	Returns		: 0:成功;负数:失败
*	complete	:2005/10/18
*/
int Comp_Exp::Comp_Expression(char *szExp)
{

	char result[500] = "";
	int  errorno;
	const char *theResult;
//	char ExpMethod[256];
	char errmsg[500];
	
		theResult = m_Compile.Operation( result,sizeof(result)-1, &errorno, szExp );
		if ( errorno )
		{
			sprintf( errmsg,"excute compiler =%d=%s= err",errorno,szExp);
			throw CException(COMPILE_EXCUTE_ERROR,errmsg,__FILE__,__LINE__);
			return -1;
		}
		
		if ( !strcmp( theResult,"true" ) )
		{		
            return -1;
		}
	return 0;
}

int Comp_Exp::Set_FileFmt(char *szFileFmt)
{
  sprintf(m_FileFmt,"%s",szFileFmt);
  return 0;
}

char* Comp_Exp::Get_FileFmt()
{
  return m_FileFmt;
}

