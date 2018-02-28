
#include "ProcessRec_Mgr.h"

/*
*	Function Name	: Input2Output
*	Description	:���캯��
*	Input param	:
*	Returns		:
*	complete	:2004/09/02
*/
Input2Output::Input2Output(int in,int out)
{
	In_Idx = in;
	Out_Idx = out;
}

/*
*	Function Name	: ~Input2Output
*	Description	:��������
*	Input param	:
*	Returns		:
*	complete	:2004/09/02
*/
Input2Output::~Input2Output()
{
}

/*
*	Function Name	: ProcessRec_Mgr
*	Description	:���캯��
*	Input param	:
*	Returns		:
*	complete	:2004/09/02
*/
ProcessRec_Mgr::ProcessRec_Mgr()
{
	m_Method = NULL;
	m_Method_cnt = 0;
}

/*
*	Function Name	: ~ProcessRec_Mgr
*	Description	:��������
*	Input param	:
*	Returns		:
*	complete	:2006/02/08
*/
ProcessRec_Mgr::~ProcessRec_Mgr()
{
	/*�رղ����*/
	pluginp.CloseSLFile();
	
	/*���m_ClFruleʵ��*/
	for( vector<ClassifyRule*>::iterator it = m_ClFrule.begin(); it != m_ClFrule.end() ; it++ )
		delete (*it);
	m_ClFrule.clear();
	
	/*���ClRuleʵ��*/
	for( vector<ClassifyRule*>::iterator it = m_Clrule.begin(); it != m_Clrule.end() ; it++ )
		delete (*it);
	m_Clrule.clear();
	
	/*���In2Outʵ��*/
	for( vector<Input2Output*>::iterator it = FileType_Chg.begin(); it != FileType_Chg.end() ; it++ )
		delete (*it);
	FileType_Chg.clear();	
	
	/*�ͷŴ�Ų���Ŀռ�*/
	if (m_Method != NULL )
	{
		delete []m_Method;
		m_Method = NULL;
	}
	m_Method_cnt = 0;		
}

/*
*	Function Name	: Init
*	Description	:��ĳ�ʼ������
*	Input param	: 
*		WorkFlowID  :����ģ��ID
*		ProcessID	:���̱�ʶID
*	Returns		: 0:�ɹ�;<0:ʧ��
*	complete	:2006/02/08
*/
int ProcessRec_Mgr::Init( int WorkFlowID, int ProcessID, char *slfile )
{
	WorkFlow_ID = WorkFlowID;
	Process_ID = ProcessID;
	
	int ret ;
	//��ʼ�����������ʽ��Ӧ��ϵ
	ret = Init_In2Out( WorkFlowID, ProcessID );	
	if ( ret < 0 )
	{
		return ret;
	}
	
	//��̬�������Ӳ���
	ret = AddVariable( WorkFlowID, ProcessID );	
	if ( ret < 0 )
	{
		return ret;
	}
	
	//��ʼ���ּ���
	ret = InitClRule( WorkFlowID, ProcessID );	
	if ( ret < 0 )
	{
		return ret;
	}
	
	//��ʼ������������
	ret = InitPrcM( WorkFlowID, ProcessID, slfile );
	if ( ret < 0 )
	{
		return ret;
	}
	
	return ( 0 );
}
	
/*
*	Function Name	: Init_In2Out
*	Description	:��ʼ�����������ʽ��Ӧ��ϵ
*	Input param	: 
*		WorkFlowID  :����ģ��ID
*		ProcessID	:���̱�ʶID
*	Returns		: 0:�ɹ�;����:ʧ��
*	complete	:2004/09/02
*/
int ProcessRec_Mgr::Init_In2Out( int WorkFlowID, int ProcessID )
{
	int in_index,out_index;
	int i = 0;
	int ret;
	CBindSQL SqlStr( DBConn );
	char errmsg[500];
	
	//ȡ����ļ���ʽID
	SqlStr.Open("select a.FILETYPE_ID from MODEL_INTERFACE a,WORKFLOW b "
			" where b.WORKFLOW_ID = :WORKFLOW_ID and b.PROCESS_ID = :PROCESS_ID "
			" and a.INTERFACE_ID = b.OUTPUT_ID ", SELECT_QUERY );
	SqlStr<<WorkFlowID<<ProcessID;
	if ( !(SqlStr>>OutFileTypeID) )
	{
		sprintf( errmsg,"select output filetype_id from MODEL_INTERFACE err,workflowid=%s,process_id=%d",WorkFlowID,ProcessID );
		throw CF_CError(ERR_TYPE_DB,ERR_LEVEL_HIG,SELECT_ERROR_FROM_DB,SELECT_ERROR_FROM_DB,errmsg,__FILE__,__LINE__);
		return SELECT_ERROR_FROM_DB;
	}
	SqlStr.Close();

	//ȡ�����ļ���ʽID
	SqlStr.Open("select a.FILETYPE_ID from MODEL_INTERFACE a,WORKFLOW b "
			" where b.WORKFLOW_ID = :WORKFLOW_ID and b.PROCESS_ID = :PROCESS_ID "
			" and a.INTERFACE_ID = b.INPUT_ID " , SELECT_QUERY);
	SqlStr<<WorkFlowID<<ProcessID;
	if ( !(SqlStr>>InFileTypeID) )
	{
		sprintf( errmsg,"select input filetype_id from MODEL_INTERFACE err,workflowid=%s,process_id=%d",WorkFlowID,ProcessID );
		throw CF_CError(ERR_TYPE_DB,ERR_LEVEL_HIG,SELECT_ERROR_FROM_DB,SELECT_ERROR_FROM_DB,errmsg,__FILE__,__LINE__);
		return SELECT_ERROR_FROM_DB;
	}
	SqlStr.Close();
	
	/*�����������ļ��ĸ�ʽһ�������TXTFILE_FMT�л���ֶζ�Ӧ��ϵ 
	 * ���򣬴�INPUT2OUTPUT�л���ֶζ�Ӧ��ϵ
	 */		 
	if( 0 == strcmp(InFileTypeID, OutFileTypeID) )
	{
		int iCount = 0;
		SqlStr.Open("select count(*) from TXTFILE_FMT where FILETYPE_ID = :INPUT_ID", SELECT_QUERY);
		SqlStr<<InFileTypeID;
		SqlStr>>iCount;
		for( int index = 1; index <= iCount; index++ )
		{
			Input2Output *tmp;
			tmp = NULL;
			tmp = new Input2Output(index,index);
			if( tmp == NULL ) 
			{
				sprintf( errmsg, "not enough memory to apply Input2Output");
				throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_HIG,NOT_ENOUGH_MEMORY_TO_APPLY,NOT_ENOUGH_MEMORY_TO_APPLY,errmsg,__FILE__,__LINE__);
				return NOT_ENOUGH_MEMORY_TO_APPLY ;
			}		
			FileType_Chg.push_back(tmp);	
		}
		if(SqlStr.IsError()) 
		{
			sprintf( errmsg,"select from INPUT2OUTPUT err,in=%d,out=%d",InFileTypeID,OutFileTypeID );
			throw CF_CError(ERR_TYPE_DB,ERR_LEVEL_HIG,SELECT_ERROR_FROM_DB,SELECT_ERROR_FROM_DB,errmsg,__FILE__,__LINE__);
			return SELECT_ERROR_FROM_DB;
		}
		SqlStr.Close();				
	}
	else
	{
		//������������ļ���ʽIDȡ�ֶζ�Ӧ��ϵ
		SqlStr.Open("select INPUT_INDEX,OUTPUT_INDEX from INPUT2OUTPUT "
				" where INPUT_ID = :INPUT_ID and OUTPUT_ID = :OUTPUT_ID ", SELECT_QUERY);
		SqlStr<<InFileTypeID<<OutFileTypeID;

		while( SqlStr>>in_index>>out_index )
		{
			Input2Output *tmp;
			tmp = NULL;
			tmp = new Input2Output(in_index,out_index);
			if( tmp == NULL ) 
			{
				sprintf( errmsg, "not enough memory to apply Input2Output");
				throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_HIG,NOT_ENOUGH_MEMORY_TO_APPLY,NOT_ENOUGH_MEMORY_TO_APPLY,errmsg,__FILE__,__LINE__);
				return NOT_ENOUGH_MEMORY_TO_APPLY ;
			}		
			FileType_Chg.push_back(tmp);
		}	
		if(SqlStr.IsError()) 
		{
			sprintf( errmsg,"select from INPUT2OUTPUT err,in=%d,out=%d",InFileTypeID,OutFileTypeID );
			throw CF_CError(ERR_TYPE_DB,ERR_LEVEL_HIG,SELECT_ERROR_FROM_DB,SELECT_ERROR_FROM_DB,errmsg,__FILE__,__LINE__);
			return SELECT_ERROR_FROM_DB;
		}
		SqlStr.Close();
	}
	return ( 0 );
}

/*
*	Function Name	: AddVar
*	Description	:���ϲ���̬�������Ӳ���
*	Input param	: 
*		varname  :��������
*		varvalue	:����ֵ
*	Returns		: 0:�ɹ�;����:ʧ��
*	complete	:2004/09/02
*/

int ProcessRec_Mgr::AddVar( const char *varname, char *varvalue )
{
	m_Compile.DefineVariable( varname,varvalue );
	var_char.insert(VAR_CHAR::value_type(varname,varvalue));
	return ( 0 );
}

/*
*	Function Name	: AddVariable
*	Description	:��̬�������Ӳ���
*	Input param	: 
*		WorkFlowID  :����ģ��ID
*		ProcessID	:���̱�ʶID
*	Returns		: 0:�ɹ�;����:ʧ��
*	complete	:2004/09/02
*/
int ProcessRec_Mgr::AddVariable( int WorkFlowID, int ProcessID )
{
	CBindSQL SqlStr( DBConn );
	char ColName[50];
	int	 ColIndex;
	char Col_Fmt[21];
	char GetValue[RECORD_LENGTH];
	char RecordType;
	char errmsg[500];

	//���������ʽIDȡ��ʽ����
	SqlStr.Open("select RECORD_TYPE from FILETYPE_DEFINE "
			" where FILETYPE_ID = :FILETYPE_ID ", SELECT_QUERY );
	SqlStr<<OutFileTypeID;
	if ( !(SqlStr>>RecordType) )
	{
		sprintf( errmsg,"select RECORD_TYPE from FILETYPE_DEFINE err,FILETYPE_ID=%s",OutFileTypeID);
		throw CF_CError(ERR_TYPE_DB,ERR_LEVEL_HIG,SELECT_ERROR_FROM_DB,SELECT_ERROR_FROM_DB,errmsg,__FILE__,__LINE__);
		return SELECT_ERROR_FROM_DB;
	}
	SqlStr.Close();
	
	//��ʼ�������ʽ��
	if ( OutRec.Init(OutFileTypeID, RecordType) )
	{
		sprintf( errmsg,"init our record err");
		throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_HIG,INIT_CFMT_CHANGE_ERROR,INIT_CFMT_CHANGE_ERROR,errmsg,__FILE__,__LINE__);
		return INIT_CFMT_CHANGE_ERROR;
	}

	//ȡ�����ʽ���ֶ����������š��ֶ�����
	SqlStr.Open("select COLNAME,COL_INDEX, COL_FMT from TXTFILE_FMT "
			" where FILETYPE_ID = :FILETYPE_ID ", SELECT_QUERY );			
	SqlStr<<OutFileTypeID;
		
	while ( SqlStr>>ColName>>ColIndex>>Col_Fmt )
	{
		//�Զ�̬���������ӱ���
		m_Compile.DefineVariable( ColName,OutRec.Get_Field(ColIndex) );
		
		var_char.insert(VAR_CHAR::value_type(ColName,OutRec.Get_Field(ColIndex)));
		//��һ������ı������ڱȽϴ�������ʱ��
		if ( !strncmp( Col_Fmt,"C",1 ) )
		{
			m_Compile.DefineVariable( "CheckCondDateTime",OutRec.Get_Field(ColIndex) );
		}
	}
	if(SqlStr.IsError()) 
	{
		sprintf( errmsg,"select record from TXTFILE_FMT err,FILETYPE_ID=%s",OutFileTypeID);
		throw CF_CError(ERR_TYPE_DB,ERR_LEVEL_HIG,SELECT_ERROR_FROM_DB,SELECT_ERROR_FROM_DB,errmsg,__FILE__,__LINE__);
		return SELECT_ERROR_FROM_DB;
	}
  SqlStr.Close();
	return ( 0 );
}

/*
*	Function Name	: InitClRule
*	Description	:��ʼ���ּ���
*	Input param	: 
*		WorkFlowID  :����ģ��ID
*		ProcessID	:���̱�ʶID
*	Returns		: 0:�ɹ�;����:ʧ��
*	complete	:2004/09/02
*/
int ProcessRec_Mgr::InitClRule( int WorkFlowID, int ProcessID )
{
  CBindSQL SqlStr( DBConn );
	int  Priority;
	char ExpMethod[METHODLENTH],tmp[METHODLENTH];
	char PickPath[PATHLENTH];
	char PickFlag[2];
	char errmsg[500];
	char result[500] = "";
	int  errorno;
	const char *theResult;
	int m_iPickRuleNo; 
	m_iPickRuleNo = 0;
	
	//ȡ���Ϸ���ǰ�ķּ����CLASSIFY_LOCATION=F
	SqlStr.Open("select PRIORITY,EXP_METHOD,PICKPATH,PICKFLAG from CLASSIFY_RULE  "
			" where WORKFLOW_ID = :WORKFLOW_ID and PROCESS_ID = :PROCESS_ID and CLASSIFY_LOCATION='F'"
			" order by PRIORITY " , SELECT_QUERY );			
	SqlStr<<WorkFlowID<<ProcessID;
	
	while ( SqlStr>>Priority>>tmp>>PickPath>>PickFlag )
	{
		//��̬��������ǰ�����2���ո�
		sprintf( ExpMethod,"\t\t%s",tmp);
		theResult = m_Compile.Operation( result,sizeof(result)-1, &errorno, ExpMethod );
		if ( errorno )
		{
			sprintf(errmsg,"excute a compiler =%s= err",ExpMethod);
			throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_HIG,COMPILE_EXCUTE_ERROR,COMPILE_EXCUTE_ERROR,errmsg,__FILE__,__LINE__);
			return COMPILE_EXCUTE_ERROR;
		}
		ClassifyRule* NewRule;
		NewRule = NULL;
		NewRule = new ClassifyRule( m_iPickRuleNo,ExpMethod,PickPath,PickFlag );
		if( NewRule == NULL ) 
		{
			sprintf( errmsg, "not enough memory to apply ClassifyRule");
			throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_HIG,NOT_ENOUGH_MEMORY_TO_APPLY,NOT_ENOUGH_MEMORY_TO_APPLY,errmsg,__FILE__,__LINE__);
			return NOT_ENOUGH_MEMORY_TO_APPLY ;
		}				
		m_ClFrule.push_back( NewRule );
		m_iPickRuleNo++;
	}
	if(SqlStr.IsError())
	{
		sprintf( errmsg,"select record from CLASSIFY_RULE err,WORKFLOW_ID=%d,PROCESS_ID=%d,CLASSIFY_LOCATION='F'",WorkFlowID,ProcessID);
		throw CF_CError(ERR_TYPE_DB,ERR_LEVEL_HIG,SELECT_ERROR_FROM_DB,SELECT_ERROR_FROM_DB,errmsg,__FILE__,__LINE__);
		return SELECT_ERROR_FROM_DB;
	}
	SqlStr.Close();  
	
	//ȡ�ּ����
	//2005-06-13
	SqlStr.Open("select PRIORITY,EXP_METHOD,PICKPATH,PICKFLAG from CLASSIFY_RULE  "
			" where WORKFLOW_ID = :WORKFLOW_ID and PROCESS_ID = :PROCESS_ID and CLASSIFY_LOCATION='N'"
			" order by PRIORITY " , SELECT_QUERY );			
	SqlStr<<WorkFlowID<<ProcessID;
	
	while ( SqlStr>>Priority>>tmp>>PickPath>>PickFlag )
	{
		//��̬��������ǰ�����2���ո�
		sprintf( ExpMethod,"\t\t%s",tmp);
		//printf("init classify,=%s=\n",ExpMethod);
		theResult = m_Compile.Operation( result,sizeof(result)-1, &errorno, ExpMethod );
		if ( errorno )
		{
			sprintf(errmsg,"excute a compiler =%s= err",ExpMethod);
			throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_HIG,COMPILE_EXCUTE_ERROR,COMPILE_EXCUTE_ERROR,errmsg,__FILE__,__LINE__);
			return COMPILE_EXCUTE_ERROR;
		}
		
		ClassifyRule* NewRule;
		NewRule = NULL;
		NewRule = new ClassifyRule( m_iPickRuleNo,ExpMethod,PickPath,PickFlag );
		if( NewRule == NULL ) 
		{
			sprintf( errmsg, "not enough memory to apply ClassifyRule");
			throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_HIG,NOT_ENOUGH_MEMORY_TO_APPLY,NOT_ENOUGH_MEMORY_TO_APPLY,errmsg,__FILE__,__LINE__);
			return NOT_ENOUGH_MEMORY_TO_APPLY ;
		}		
		m_Clrule.push_back( NewRule );
		m_iPickRuleNo++;
	}
	if(SqlStr.IsError())
	{
		sprintf( errmsg,"select record from CLASSIFY_RULE err,WORKFLOW_ID=%d,PROCESS_ID=%d,CLASSIFY_LOCATION='N'",WorkFlowID,ProcessID);
		throw CF_CError(ERR_TYPE_DB,ERR_LEVEL_HIG,SELECT_ERROR_FROM_DB,SELECT_ERROR_FROM_DB,errmsg,__FILE__,__LINE__);
		return SELECT_ERROR_FROM_DB;
	}
 	SqlStr.Close();  
  
	return ( 0 );
}

/*
*	Function Name	: InitPrcM
*	Description	:����������
*	Input param	: 
*		WorkFlowID  :����ģ��ID
*		ProcessID	:���̱�ʶID
*	Returns		: 0:�ɹ�;����:ʧ��
*	complete	:2006/02/08
*/
int ProcessRec_Mgr::InitPrcM( int WorkFlowID, int ProcessID, char *slfile )
{
	CBindSQL SqlStr( DBConn );
	int ret;
	struct _process{
		int MethodID;
		char Condition[METHODLENTH];
		char StartTime[15];
		char EndTime[15];
	};
	char errmsg[500];
	
	char result[500] = "";
	int  errorno;
	const char *theResult;
	
	strcpy( SL_fname, slfile );
	
	//�Ƿ��ܴ򿪲����
	if (!pluginp.Initialize(SL_fname,"CreateEntryPoint"))
	{
		sprintf( errmsg, "init slfile %s err",SL_fname);
		throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_HIG,OPEN_PLUGIN_ERROR,OPEN_PLUGIN_ERROR,errmsg,__FILE__,__LINE__);
		return OPEN_PLUGIN_ERROR;
	}
	
	//���ù��̹������ִ�����
	SqlStr.Open("select count(*) from PROCESS "
			" where WORKFLOW_ID = :WORKFLOW_ID and PROCESS_ID = :PROCESS_ID ", SELECT_QUERY);			
	SqlStr<<WorkFlowID<<ProcessID;
			
	if ( !(SqlStr>>m_Method_cnt ) ) 
	{
		sprintf( errmsg, "select count(*) from PROCESS err,WORKFLOW_ID =%d,PROCESS_ID =%d",WorkFlowID,ProcessID);
		throw CF_CError(ERR_TYPE_DB,ERR_LEVEL_HIG,SELECT_ERROR_FROM_DB,SELECT_ERROR_FROM_DB,errmsg,__FILE__,__LINE__);
		return SELECT_ERROR_FROM_DB;
	}
	SqlStr.Close();
	if ( m_Method_cnt>0 ) 
	{
		m_Method = new C_PluginMethod[m_Method_cnt];	
		if ( m_Method == NULL ) 
		{
			sprintf( errmsg, "not enough memory to apply C_PluginMethod");
			throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_HIG,NOT_ENOUGH_MEMORY_TO_APPLY,NOT_ENOUGH_MEMORY_TO_APPLY,errmsg,__FILE__,__LINE__);
			return NOT_ENOUGH_MEMORY_TO_APPLY ;
		}	
		
		/*�����ŷ�������ʱ�ռ�*/
		struct _process *p_process;
		p_process = NULL;
		p_process = new struct _process[m_Method_cnt];
		if ( p_process == NULL ) 
		{
			sprintf( errmsg, "not enough memory to apply _process");
			throw CF_CError(ERR_TYPE_OS,ERR_LEVEL_HIG,NOT_ENOUGH_MEMORY_TO_APPLY,NOT_ENOUGH_MEMORY_TO_APPLY,errmsg,__FILE__,__LINE__);
			return NOT_ENOUGH_MEMORY_TO_APPLY ;
		}	
	
		//��ʼ��������
		SqlStr.Open("select METHOD_ID,CONDITION,START_TIME,END_TIME from PROCESS"
				" where WORKFLOW_ID = :WORKFLOW_ID and PROCESS_ID = :PROCESS_ID "
				" order by PROC_CONTENT_ID " , SELECT_QUERY);
		SqlStr<<WorkFlowID<<ProcessID;
		
		int i = 0;
		while( SqlStr>>p_process[i].MethodID>>p_process[i].Condition>>p_process[i].StartTime>>p_process[i].EndTime )
		{
			i++;
		}
		if(SqlStr.IsError())
		{
			sprintf( errmsg, "select record from PROCESS err,WORKFLOW_ID =%d,PROCESS_ID =%d",WorkFlowID,ProcessID);
	    throw CF_CError(ERR_TYPE_DB,ERR_LEVEL_HIG,SELECT_ERROR_FROM_DB,SELECT_ERROR_FROM_DB,errmsg,__FILE__,__LINE__);	
			return SELECT_ERROR_FROM_DB;
		}
		SqlStr.Close();
		for ( int i=0; i<m_Method_cnt ; i++ )
		{
			ret = m_Method[i].Init( p_process[i].MethodID,p_process[i].Condition,p_process[i].StartTime,p_process[i].EndTime, var_char ,pluginp );
			if( ret < 0 )
				return ret;
		}
		
		/*�ͷŴ�ŷ�������ʱ�ռ�*/
		if( p_process != NULL )
		{
			delete []p_process;
			p_process = NULL;
		}
	}
	
	return ( 0 );		
}

/*
*	Function Name	: Set_InRec
*	Description	:�������뵽����Ŀ���
*	Input param	: 
*		inrec	: ���뻰����
*	Output param:
*	Returns		: 0:�ɹ�;����:ʧ��
*	complete	:2004/09/02
*/
int ProcessRec_Mgr::Set_InRec( CFmt_Change &inrec )
{
	char tmpField[RECORD_LENGTH];
	
	//�����뻰��д�������������	
	for( In2Out::iterator it=FileType_Chg.begin(); it!=FileType_Chg.end(); it++ )
	{
		inrec.Get_Field((*it)->get_In_Idx(),tmpField);
		OutRec.Set_Field((*it)->get_Out_Idx(),tmpField);
	}

	return ( 0 );
}

/*
*	Function Name	: Process_Classify
*	Description	:����ּ�
*	Input param	: 
*		inrec	: ���뻰����
*	Output param:
*		retid	���ּ�·��ID���ö��ż��ÿ��ID
*		retcnt	���ּ�·������
*	Returns		: 0:�ɹ�;����:ʧ��
*	complete	:2004/09/02
*/
int ProcessRec_Mgr::Process_Classify( char* retid, int& retcnt )
{
	char result[500] = "";
	int  errorno;
	const char *theResult;
	char errmsg[500];
	
	retid[0] = 0;
	retcnt = 0;
	
	for( vector<ClassifyRule*>::iterator it = m_Clrule.begin(); it != m_Clrule.end() ; it++ )
	{
		theResult = m_Compile.Operation( result,sizeof(result)-1, &errorno, (*it)->get_ExpMethod() );
		if ( errorno )
		{
			sprintf( errmsg,"excute compiler =%s= err",(*it)->get_ExpMethod());
			throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_HIG,COMPILE_EXCUTE_ERROR,COMPILE_EXCUTE_ERROR,errmsg,__FILE__,__LINE__);
			return COMPILE_EXCUTE_ERROR;
		}
		
		if ( !memcmp( theResult,"true",4 ) )
		{		
			char tmp[50];
			sprintf( tmp,"%s%d,",retid,(*it)->get_Priority() );
			strcpy( retid,tmp );
			retcnt++;
			sprintf( tmp, "%s", (*it)->get_PickFlag() );
			if( tmp[0] == 'M' )
				break;		
		}
	}

	return ( 0 );
}

/*
*	Function Name	: Process
*	Description	:ҵ����
*	Input param	: 
*	Output param:
*	Returns		: 0:�ɹ�;����:ʧ��
*	complete	:2006/02/08
*/
int ProcessRec_Mgr::Process(int &rres)
{
        
  int ret=0;
	int islack=0;
	rres = 0;
	for ( int i=0; i<m_Method_cnt ; i++ )
	{
		ret = m_Method[i].Excute(m_Compile, OutRec, pluginp);
		/*������㴦����̣����������Ϻ󣬽����������û���*/
		if ( ret != 0 ) 
		{  
			islack++;
			rres = rres + ret;		  
		}
	}	
	if ( islack > 0 )
	{
		return ( 2 );//�����ϻ�������2
	}
	else 
		return ( 0 );
	
}

/*
*	Function Name	: Get_OutRec
*	Description	:���ϲ�����������
*	Input param	: 
*		outr	:�����������
*		len		:������������С
*	Output param:
*	Returns		: 0:�ɹ�;����:ʧ��
*	complete	:2004/09/02
*/
int ProcessRec_Mgr::Get_OutRec( char *outr, int len )
{
	OutRec.Get_record(outr,len);
	return ( 0 );
}

/*
*	Function Name	: Get_OutRec
*	Description	:���ϲ�����������
*	Input param	: 
*		outr	:CFmt_Change��ʽ�����
*	Output param:
*	Returns		: 0:�ɹ�;����:ʧ��
*	complete	:2004/09/02
*/
int ProcessRec_Mgr::Get_OutRec( CFmt_Change &outr )
{
	outr.Copy_Record( OutRec );

	return ( 0 );
}


/*
*	Function Name	: ClassifyRule
*	Description	:���캯��
*	Input param	:
*	Returns		:
*	complete	:2004/09/02
*/	
ClassifyRule::ClassifyRule(int priority,char *expmth,char *pickpath, char *pickflag)
{
	Priority = priority;
	strcpy( ExpMethod, expmth );
	strcpy( PickPath, pickpath );
	strcpy( PickFlag, pickflag );
}

/*
*	Function Name	: ClassifyRule
*	Description	:��������
*	Input param	:
*	Returns		:
*	complete	:2004/09/02
*/	
ClassifyRule::~ClassifyRule()
{
}

/*
*	Function Name	: ClassifyRule
*	Description	:��������ķּ�����
*	Input param	:
* 		exp	: �����ķּ�����
*	Returns		: 0:�ɹ�;����:ʧ��
*	complete	:2004/09/02
*/	
int ClassifyRule::set_ExpMethod(char *exp )
{
	strcpy( ExpMethod, exp );
	return ( 0 );
}

/*
*	Function Name	: Process_FClassify
*	Description	:����ּ�
*	Input param	: 
*		inrec	: ���뻰����
*	Output param:
*		retid	���ּ�·��ID���ö��ż��ÿ��ID
*		retcnt	���ּ�·������
*	Returns		: 0:�ɹ�;����:ʧ��
*	complete	:2005/06/13
*/
int ProcessRec_Mgr::Process_FClassify( char* retid, int& retcnt )
{
	char result[500] = "";
	int  errorno;
	const char *theResult;	
	char errmsg[500];
	
	retid[0] = 0;
	retcnt = 0;
		
	for( vector<ClassifyRule*>::iterator it = m_ClFrule.begin(); it != m_ClFrule.end() ; it++ )
	{
		theResult = m_Compile.Operation( result,sizeof(result)-1, &errorno, (*it)->get_ExpMethod() );
		if ( errorno )
		{
			sprintf( errmsg,"excute compiler =%s= err",(*it)->get_ExpMethod());
			throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_HIG,COMPILE_EXCUTE_ERROR,COMPILE_EXCUTE_ERROR,errmsg,__FILE__,__LINE__);
			return COMPILE_EXCUTE_ERROR;
		}
		
		if ( !memcmp( theResult,"true",4 ) )
		{	
			char tmp[50];	
			sprintf( tmp,"%s%d,",retid,(*it)->get_Priority() );
			strcpy( retid,tmp );
			retcnt++;
			sprintf( tmp, "%s", (*it)->get_PickFlag() );
			if( tmp[0] == 'M' )
				break;			
		}
	}

	return ( 0 );
}

int ProcessRec_Mgr::PrintSLNo(char *SLName)	
{
	PacketParser pps;
	ResParser res;
	pluginp[SLName]->execute(pps,res);
	return 0;
}

int ProcessRec_Mgr::LoadOrUpdateMem(char *SLName, char* ShmPath)	
{
	PacketParser pps;
	ResParser res;
	pps.setFieldValue( 0, ShmPath, strlen(ShmPath));
	pluginp[SLName]->execute(pps,res);
	return 0;
}				

/****************************The End!*********************************/	