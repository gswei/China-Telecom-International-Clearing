
#include "C_PluginMethod.h"

/*
*	Function Name	: C_PluginMethod
*	Description	:���캯��
*	Input param	:
*	Returns		:
*	complete	:2006/02/08
*/
C_Method::C_Method()
{
	Seq = 0;
	memset(Plugin,0,sizeof(Plugin));
	m_pPluginObj = NULL;	
}

/*
*	Function Name	: Init
*	Description	:��ʼ������
*	Input param	:
*	Returns		:
*		SUCC 0 /FAIL -1
*	complete	:2004/09/02
*/
int C_Method::Init(int seq, char *plugin,char *out_col, VAR_CHAR &var,PluginProxy &pluginp)
{
	Seq = seq;
	strcpy( Plugin,plugin );
	Output_Cnt = SplitBuf( out_col,',',Output_Col );
	
	//add by sunhua at 2004102
	char param[30],temp[30];
	memset( param, 0, sizeof(param) );
	char errmsg[500];

	if ( Plugin[strlen(Plugin)-1] == '\n' )
	{
		Plugin[strlen(Plugin)-1] = 0;
	}
	
	char *aa;
	char *bb;

	aa = strchr(Plugin,'(');
	if ( aa == NULL )
	{
		sprintf( errmsg,"Plugin string define err =%s= \n",Plugin);
		throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_HIG,PLUGIN_STRING_DEFINE_ERR,PLUGIN_STRING_DEFINE_ERR,errmsg,__FILE__,__LINE__);
		return PLUGIN_STRING_DEFINE_ERR;
	}		
	*aa = 0;
	bb = aa+1;

	strcpy(PluginName,Plugin);
	
	m_pPluginObj = pluginp[PluginName];
	if( m_pPluginObj == NULL )
	{
		sprintf( errmsg,"Plugin class =%s= not found!\n",PluginName);
		throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_HIG,CAN_NOT_FIND_PLUGIN_CLASS,CAN_NOT_FIND_PLUGIN_CLASS,errmsg,__FILE__,__LINE__);
		return CAN_NOT_FIND_PLUGIN_CLASS;
	}
		
	aa = strchr( bb,')' );
	if ( aa == NULL )
	{
		sprintf( errmsg,"Plugin string define err =%s= \n",Plugin);
		throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_HIG,PLUGIN_STRING_DEFINE_ERR,PLUGIN_STRING_DEFINE_ERR,errmsg,__FILE__,__LINE__);
		return PLUGIN_STRING_DEFINE_ERR;
	}		
	*aa = ',' ;
	//��ʼ��PacketParser�����
	int j=0;
	while ( *bb != 0 )
	{
		aa = strchr(bb,',');
		if ( aa != NULL )
		{
			*aa = 0;
		}
		strcpy( param,bb );
		if ( param[0] == '$' )
		{
			strcpy( temp, param+1 );
			VAR_CHAR::iterator it = var.find(temp);
			if ( it != var.end())
			{
				pparser.setFieldValue(j,it->second,MAXLENGTH);	
			}
			else
			{
				sprintf(errmsg,"var %s not define!",temp );
				throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_HIG,COMPILE_EXCUTE_ERROR,COMPILE_EXCUTE_ERROR,errmsg,__FILE__,__LINE__);
				return COMPILE_EXCUTE_ERROR;	
			}			
		}
		else
		{
			pparser.setFieldValue(j,bb,MAXLENGTH);
		}
		bb = aa+1 ;
		j++;
	}	
			
	return ( 0 );
}

/*
*	Function Name	: get_Output_Col
*	Description	:ȡ��ԱOutput_Col��ֵ
*	Input param	:
*	Returns		:
*		Output_Col
*	complete	:2004/09/02
*/
int C_Method::get_Output_Col(int i)
{
	return ( Output_Col[i] );
}

/*
*	Function Name	: get_Output_Cnt
*	Description	:ȡ��ԱOutput_Cnt��ֵ
*	Input param	:
*	Returns		:
*		Output_Cnt
*	complete	:2004/09/02
*/
int C_Method::get_Output_Cnt()
{
	return ( Output_Cnt );
}

/*
*	Function Name	: get_Plugin
*	Description	:ȡ��ԱPlugin��ֵ
*	Input param	:
*	Returns		:
*		Plugin
*	complete	:2004/09/02
*/
char* C_Method::get_Plugin()
{
	return ( Plugin );
}

/*
*	Function Name	: ~C_PluginMethod
*	Description	:��������
*	Input param	:
*	Returns		:
*	complete	:2004/09/02
*/
C_Method::~C_Method()
{
}

/*
*	Function Name	: C_PluginMethod
*	Description	:���캯��
*	Input param	:
*	Returns		:
*	complete	:2006/02/08
*/
C_PluginMethod::C_PluginMethod()
{
	Method = NULL;
	MethodCnt = 0;	
}

/*
*	Function Name	: ~C_PluginMethod
*	Description	:��������
*	Input param	:
*	Returns		:
*	complete	:2004/09/02
*/
C_PluginMethod::~C_PluginMethod()
{	
	if ( Method != NULL )
	{
		delete []Method;
		Method = NULL ;
	}
	MethodCnt = 0;
}

/*
*	Function Name	: Init
*	Description	:��ĳ�ʼ������
*	Input param	: 
*		methodid  :������ID
*		condition :ִ������
*		starttime :��Ч��ʼʱ��
*		endtime	  :��Ч����ʱ��
*	Returns		: 0:�ɹ�;<0:ʧ��
*	complete	:2004/09/02
*/
int C_PluginMethod::Init(  int methodid,char *condition,char *starttime,char *endtime ,VAR_CHAR &var, PluginProxy &pluginp )
{
	int seq;
	char plugin[METHODLENTH];
	char out_col[200];
	char errmsg[500];
	
	MethodID = methodid;
	
	strcpy( StartTime,starttime );
	strcpy( EndTime,endtime );
	
	//һ�������ж��ٲ���	
	CBindSQL SqlStr(DBConn);
	
	SqlStr.Open("select count(*) from PROC_STEP \
			 where METHOD_ID = :METHOD_ID", SELECT_QUERY );
			
	SqlStr<<methodid;
	if ( !(SqlStr>>MethodCnt) ) 
	{
		sprintf(errmsg,"select count(*) from PROC_STEP where METHOD_ID = %d err",methodid);
		throw CF_CError(ERR_TYPE_DB,ERR_LEVEL_HIG,SELECT_ERROR_FROM_DB,SELECT_ERROR_FROM_DB,errmsg,__FILE__,__LINE__);
		return SELECT_ERROR_FROM_DB;
	}
	SqlStr.Close();	
	
	if ( MethodCnt>0 )
	{
		Method = new C_Method[MethodCnt];		
		if ( Method == NULL )
		{
			sprintf(errmsg,"create C_Method err,apply memory err");
			throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_HIG,NOT_ENOUGH_MEMORY_TO_APPLY,NOT_ENOUGH_MEMORY_TO_APPLY,errmsg,__FILE__,__LINE__);
			return NOT_ENOUGH_MEMORY_TO_APPLY;
		}
		//ȡÿ������ķ����ͽ������ֶ�
		SqlStr.Open("select PROCESS_SEQ,PROCESS_PLUGIN,OUTPUT_COL from PROC_STEP \
				 where METHOD_ID = :METHOD_ID order by PROCESS_SEQ", SELECT_QUERY );
				
		SqlStr<<methodid;
		
		int i = 0;
		while( SqlStr>>seq>>plugin>>out_col )
		{
			Method[i].Init(seq,plugin,out_col,var,pluginp);
			i++;
		}
		if(SqlStr.IsError()) 
		{
			sprintf(errmsg,"select record from PROC_STEP where METHOD_ID = %d err",methodid);
			throw CF_CError(ERR_TYPE_DB,ERR_LEVEL_HIG,SELECT_ERROR_FROM_DB,SELECT_ERROR_FROM_DB,errmsg,__FILE__,__LINE__);
			return SELECT_ERROR_FROM_DB;
		}
		SqlStr.Close();
	}
	sprintf( m_szCondition, "(%s) and (between($CheckCondDateTime,'%s','%s'))", condition, StartTime, EndTime );
	
	return ( 0 );
}

/*
*	Function Name	: Excute
*	Description	:ִ�к���
*	Input param	: 
*		p_Compile  :��̬�������ָ��
*		p_OutRec :���������ָ��
*	Returns		: 0:�ɹ�;<0:ʧ��
*	complete	:2004/09/02
*/
int C_PluginMethod::Excute(C_Compile &p_Compile,CFmt_Change &p_OutRec, PluginProxy& pluginp)
{
	char result[255];
	int  errorno;
	const char *theResult;
	char param[30][30];
	char errmsg[500];
	char temp[50],temp1[50];
	int  ret;
	int l_iOutputCnt;
	
	
	//�ж��Ƿ���Ҫִ�и÷���	
	memset(result, 0, sizeof(result) );
	theResult = p_Compile.Operation( result,sizeof(result)-1, &errorno, m_szCondition );
	if ( errorno )
	{
		sprintf( errmsg, "excute compiler =%s= err",m_szCondition);
		throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_HIG,COMPILE_EXCUTE_ERROR,errorno,errmsg,__FILE__,__LINE__);
		return COMPILE_EXCUTE_ERROR;
	}
	if ( !memcmp( theResult,"false",5 ) )
	{
		return ( 0 );
	}
	
	for ( int i=0; i<MethodCnt ; i++ )
	{

		//ִ����Ӧ��derived_plugin�����
		ResParser reval;
		ret = Method[i].m_pPluginObj->execute(Method[i].pparser,reval);
		l_iOutputCnt = Method[i].get_Output_Cnt();
		for ( int j=0 ; j<l_iOutputCnt; j++ )
		{
			if ( Method[i].get_Output_Col(j) != 0 )
			{
				p_OutRec.Set_Field(Method[i].get_Output_Col(j),reval.getFieldValue(j));
			}
		}
		if( ret != 0)
			return ret ;
	}	
	
	return ( 0 );
	
}

/*
*	Function Name	: set_Condition
*	Description	:�������������
*	Input param	: 
*		cond :����������
*	Returns		: 0:�ɹ�;<0:ʧ��
*	complete	:2004/09/02
*/
int C_PluginMethod::set_Condition( char *cond )
{
	strcpy( m_szCondition, cond );
	return ( 0 );
}

int DeleteSpace( char *ss )
{
	int i;
	i = strlen(ss)-1;
	while ( i && ss[i] == ' ' )
		i--;
	ss[i+1] = 0;
	return(0);
}

int SplitBuf( char *ss,char flag, int *buf )
{
	int i;
	char *ss0,*ss1;
	char tmp[10];
	i = 0; ss0 = ss;
	while( ( ss1 = strchr( ss0,flag ) ) != NULL ) 
	{
		*ss1 = 0; strcpy( tmp,ss0 ); ss0 = ss1+1; *ss1 = flag;
		buf[i++] = atoi ( tmp );
	}
	strcpy( tmp ,ss0 );
	buf[i++] = atoi( tmp );
	return(i);
}

