#include "interpreter.h"

static char errmsg[512];
//static CBindSQL ds( DBConn );

char Interpreter::m_szPipeId[100]={0};
char Interpreter::m_szVarname[100]={0};


//add by weixy 20080707
int Interpreter::clear()
{
	for (vector<Expression *>::iterator it = m_SyntaxTree.begin(); it != m_SyntaxTree.end(); delete *it++ )
	{};
	m_SyntaxTree.clear();
	m_SyntaxTree.push_back(NULL); 

	return 0;
}
//end add by weixy 20080707

	
Interpreter::~Interpreter()
{
	for (vector<Expression *>::iterator it = m_SyntaxTree.begin(); it != m_SyntaxTree.end(); delete *it++ )
	{}
}

const char *Interpreter::CompileExp(const char *Context, Expression ** SyntaxTree, int &ErrorNo)
{
	Expression *LeftExp = NULL;
	const char *pPosition = NULL;
	
	pPosition = ValueExp(Context, &LeftExp, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
		goto error_exit;

	for (;;)
	{
		//and ���ʽ���� token = " and ",andǰ������пհ��ַ�
		if ( (strncmp(pPosition, "and", 3) == 0) &&
			( (*(pPosition - 1) == ' ') || (*(pPosition - 1) == '\t') ) &&
			( (*(pPosition + 3) == ' ') || (*(pPosition + 3) == '\t') ) 
				)
		{
			AndExp *TempExp = new AndExp();
			Expression *RightExp = NULL;
			
			pPosition += 3;
			
			pPosition = ValueExp(pPosition, &RightExp, ErrorNo);
			if (ErrorNo != COMPILE_NOERROR)
			{
				delete TempExp;
				goto error_exit;
			}
			TempExp->AddLeftExp(LeftExp);
			TempExp->AddRightExp(RightExp);
			//��ݹ�
			LeftExp = TempExp;
		}
		//or ���ʽ���� token = " or ",orǰ������пհ��ַ�
		else if ( (strncmp(pPosition, "or", 2) == 0) &&
				( (*(pPosition - 1) == ' ') || (*(pPosition - 1) == '\t') ) &&
				( (*(pPosition + 2) == ' ') || (*(pPosition + 2) == '\t') ) 
				)
		{
			OrExp *TempExp = new OrExp();
			Expression *RightExp = NULL;
			
			pPosition += 2;
			
			pPosition = ValueExp(pPosition, &RightExp, ErrorNo);
			if (ErrorNo != COMPILE_NOERROR)
			{
				delete TempExp;
				goto error_exit;
			}
			TempExp->AddLeftExp(LeftExp);
			TempExp->AddRightExp(RightExp);
			LeftExp = TempExp;
		}
		//�﷨��������(ע:��Ҫ�ڷ�����ɺ����Ƿ���ʣ���������)
		else
		{
			goto normal_exit;
		}
	}

normal_exit:
	*SyntaxTree = LeftExp;
	return pPosition;
error_exit:
	delete LeftExp;
	return pPosition;
}

const char *Interpreter::ValueExp(const char *Context, Expression ** SyntaxTree, int &ErrorNo)
{
	const char *pPosition = Context;
	char Token[255] = "";
	char *pToken;
	map<string, const char *>::iterator it;
	
	//�ȳԵ�ǰ��Ŀհ�
	while ( (*pPosition != 0) && strchr(" \t\r\n", *pPosition) )
		++pPosition;
	
	switch(*pPosition)
	{
		//�������ʽ
		case '$':
			++pPosition;
			pToken = Token;
			while ( (*pPosition != 0) && (strchr(" \t,()\r\n", *pPosition) == NULL))
				*pToken++ = *pPosition++;
			*pToken = 0;
			
			it = m_VaribleList.find( Token );
			if (it == m_VaribleList.end())
			{
				ErrorNo = COMPILE_ERROR_VARIABLE_NOT_DEFINE;
				//m_ErrorInfo = "$" + string(Token) + " not define";
				//���ݳ���λ��
				return pPosition - strlen(Token) - 1;
			}
			*SyntaxTree = new VariableExp( it->second );
			break;
		//�ַ������ʽ,Ŀǰ������ת���ַ�\',���治��Ƕ�� '''
		case '\'':
			++pPosition;
			pToken = Token;
			while ( (*pPosition != 0) && (*pPosition != '\'') )
				*pToken++ = *pPosition++;
			*pToken = 0;
			if (*pPosition != '\'')
			{
				ErrorNo = COMPILE_ERROR_MISSING_SINGLE_QUOTATION_MARK;
				//m_ErrorInfo = "'" + string(Token) ;
				return pPosition;
			}
			*SyntaxTree = new LiteralExp(Token);
			pPosition++;
			break;
		//����,����������
		case '-'://��Ϊ����
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			pToken = Token;
			*pToken++ = *pPosition++;
			if (*(pToken - 1) == '-') //ȥ��'-'����Ŀհ� ��: - 1234
			{
				while ( (*pPosition != 0) && strchr(" \t", *pPosition) )
					pPosition++;
			}
			//ȡ����
			while ( (*pPosition != 0) && (*pPosition >= '0') && (*pPosition <= '9') )
				*pToken++ = *pPosition++;
			
			*pToken = 0;
			//���������" )\t\r\n"�ַ���β�������,��: 1234rtad , 1234
			if ( (strcmp(Token, "-") == 0) || 
				( (*pPosition != 0) && !strchr(" \t,)\r\n", *pPosition) )  )
			{
				ErrorNo = COMPILE_ERROR_INVALID_NUMBER;
				//m_ErrorInfo = Token ;
				return pPosition;
			}
			*SyntaxTree = new LiteralExp(Token);
			break;
		//����,������ʽ�����ȼ�
		case '(':
			++pPosition;
			pPosition = CompileExp(pPosition, SyntaxTree, ErrorNo);
			if ( *pPosition != ')' )
			{
				ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
				return pPosition;
			}
			pPosition++;
			break;
		default:
			Expression *pExp = NULL;
			//�������ʽ
			if (strncmp(pPosition, "set(", 4) == 0)
			{
				pExp = new SetExp();
			}
			else if (strncmp(pPosition, "case(", 5) == 0)
			{
				pExp = new CaseExp();
			}
			else if (strncmp(pPosition, "like(", 5) == 0)
			{
				pExp = new LikeExp();
			}
			else if (strncmp(pPosition, "in(", 3) == 0)
			{
				pExp = new InExp();
			}
			else if (strncmp(pPosition, "substring(", 10) == 0)
			{
				pExp = new SubStringExp();
			}
			else if (strncmp(pPosition, "connect(", 8) == 0)
			{
				pExp = new ConnectExp();
			}
			else if (strncmp(pPosition, "if(", 3) == 0)
			{
				pExp = new IfExp();
			}
			else if (strncmp(pPosition, "not(", 4) == 0)
			{
				pExp = new NotExp();
			}
			else if (strncmp(pPosition, "strabove(", 9) == 0)
			{
				pExp = new StrCmpExp(true); //�ַ������ڱȽ�
			}
			else if (strncmp(pPosition, "strbelow(", 9) == 0)
			{
				pExp = new StrCmpExp(false); //�ַ���С�ڱȽ�
			}
			else if (strncmp(pPosition, "numabove(", 9) == 0)
			{
				pExp = new NumCmpExp(true); //���ִ��ڱȽ�
			}
			else if (strncmp(pPosition, "numbelow(", 9) == 0)
			{
				pExp = new NumCmpExp(false); //����С�ڱȽ�
			}
			else if (strncmp(pPosition, "isdatetime(", 11) == 0)
			{
				pExp = new IsDateTimeExp();
			}
			else if (strncmp(pPosition, "length(", 7) == 0)
			{
				pExp = new LengthExp();
			}
			else if (strncmp(pPosition, "decode(", 7) == 0)
			{
				pExp = new DecodeExp();
			}
			else if (strncmp(pPosition, "comma(", 6) == 0)
			{
				pExp = new CommaExp();
			}
			else if (strncmp(pPosition, "sum(", 4) == 0)
			{
				pExp = new SumExp();
			}
			else if (strncmp(pPosition, "list(", 5) == 0)
			{
				pExp = new ListExp();
			}
			else if (strncmp(pPosition, "plugin(", 7) == 0)
			{
				pExp = new PluginExp();
			}
			else if (strncmp(pPosition, "between(", 8) == 0)
			{
				pExp = new BetweenExp();
			}
			else if (strncmp(pPosition, "likein(", 7) == 0)
			{
				pExp = new LikeInExp();
			}
			else if (strncmp(pPosition, "adddays(", 8) == 0)
			{
				pExp = new AddDaysExp();
			}
			else if (strncmp(pPosition, "minusdays(", 10) == 0)
			{
				pExp = new MinusDaysExp();
			}
			else if (strncmp(pPosition, "addtime(", 8) == 0)
			{
				pExp = new AddTimeExp();
			}
			else if (strncmp(pPosition, "minustime(", 10) == 0)
			{
				pExp = new MinusTimeExp();
			}
			//add by weixy 20081201
			else if (strncmp(pPosition, "difftime(", 9) == 0)
			{
				pExp = new DiffTimeExp();
			}
			//end add by weixy 20081201
			//����δԤ�ڵ��ַ���
			else
			{
				ErrorNo = COMPILE_ERROR_NOT_EXPECTED_CHARACTER;
				return pPosition;
			}
			if (pExp != NULL)
			{
				pPosition = pExp->SyntaxAnalyze(this, pPosition, ErrorNo);
				if (ErrorNo != COMPILE_NOERROR)
				{
					delete pExp;
					return pPosition;
				}
				*SyntaxTree = pExp;
			}
	}
	//ȥ������Ŀհ׷���
	while ( (*pPosition != 0) && strchr(" \t\r\n", *pPosition) )
		++pPosition;
	return pPosition;	
}

//
bool Interpreter::DefineVariable(const char *VariableName, const char * VariableAddress)
{
	map<string, const char *>::iterator it = m_VaribleList.find(VariableName);
	if ( (it != m_VaribleList.end()) || (VariableAddress == NULL) )
	{
		return false; //�Ѿ�����ͬ���ı������߱�����ַΪ��
	}
	m_VaribleList[VariableName] = VariableAddress;
	return true;
}

//��һ�α�����ʽʱ��ǿ���޸ı��ʽ��ǰ2���ֽ�
//��1���ֽ����ѱ����־0xEE,��2���ֽ���������
//�����������ʱÿ������ǰ��Ԥ��2��tab�ַ�
//��Ҫ��Ϊ����Ѹ�ٶ�λһ�����ʽ���﷨��
const char *Interpreter::Operation(char *Result, int Length, int *ErrorNo, const char *Context)
{
	*ErrorNo = COMPILE_NOERROR;
	
	if ( *(unsigned char *)Context == 0xEE)
	{
		//try { ����ʱ������ COMPILE_RUNTIME_
		//����Expression�����׳��쳣
		try
		{
						//ȡ���ʽ���������
						//modify by weixy 20080606 
			return strncpy(Result, m_SyntaxTree[ (int)*(Context + 1) > 0 ? (int)*(Context + 1): 256+((int)*(Context + 1))  ]->Execute(), Length);

			//return strncpy(Result, m_SyntaxTree[ (unsigned int)*(Context + 1) ]->Execute(), Length);
		}
		catch (...)
		{
			*ErrorNo = COMPILE_RUNTIME_ERROR_UNKNOWN;
			return Result;
		}
	}
	else
	{
		//���ʽ�����������
		//(�������ʹ��һ���ֽڴ洢1~255,����0���ַ���������־δʹ��0)
		if ( m_SyntaxTree.size() >= 256 )
		{
			*ErrorNo = COMPILE_RUNTIME_ERROR_EXPOVERLIMIT;
			return Context;
		}
		
		Expression *pTempExp = NULL;
		const char *pPosition = NULL;
		
		pPosition = CompileExp(Context, &pTempExp, *ErrorNo);
		if (*ErrorNo != COMPILE_NOERROR) 
		{
			return pPosition;
		}
		//�绹��ʣ����ַ�����˵�����ʽ�����ķ�����
		else if (*pPosition != 0)
		{
			*ErrorNo = COMPILE_ERROR_INVALID_EXP;
			delete pTempExp;
			return pPosition;
		}

		//�Ա��ʽ������,��ʽΪ:0xEE �������,��1~255
		//ע:�޸�Context�ĵ�1���ֽ�Ϊ0xEE(��־),��2���ֽ�Ϊ������
		*(unsigned char *) Context = 0xEE;
		*(unsigned char *) (Context + 1) = m_SyntaxTree.size();

		m_SyntaxTree.push_back(pTempExp);
		
		/*if (m_SyntaxTree.size()>=127 )
		{
			 	cout<<"m_SyntaxTree.size()="<<m_SyntaxTree.size()<<endl;
			 	cout<<"Context="<<Context<<endl;
			 	cout<<" *(unsigned char *) (Context + 1)="<<*(unsigned char *) (Context + 1) <<endl;
			 	cout<<"(int)*(Context + 1)="<<(int)*(Context + 1)<<endl;			 	
			}*/
		return Operation(Result, Length, ErrorNo, Context);
	}
}


//like����
Interpreter::LikeExp::~LikeExp() 
{
	delete m_LeftExp;
	delete m_RightExp;
}

const char *Interpreter::LikeExp::Execute() 
{
	if (Compare(m_LeftExp->Execute(), m_RightExp->Execute()) )
		return RET_TRUE;
	else
		return RET_FALSE;
}

const char *Interpreter::LikeExp::SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo)
{
	const char *pPosition = Context + 5; //eat 'like('
	
	//����ʽ
	pPosition = pInterpreter->CompileExp(pPosition, &m_LeftExp, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
	if (*pPosition != ',')
	{
		ErrorNo = COMPILE_ERROR_MISSING_COMMA;
		return pPosition;
	}
	pPosition ++;
	//�ұ��ʽ
	pPosition = pInterpreter->CompileExp(pPosition, &m_RightExp, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
	if (*pPosition != ')' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
		return pPosition;
	}
	
	return pPosition + 1;
}

bool Interpreter::LikeExp::Compare(const char *lv_chCompareString, const char *lv_chCompareMod)
{
	while(1)
	{
		switch(*lv_chCompareMod)
		{	
			case '\0':
				if (*lv_chCompareString == '\0')
				{
					return true;
				}
				else
				{
					return false;
				}
			case '!':
				if (Compare(lv_chCompareString,lv_chCompareMod + 1) == true)
				{
					return false;
				}
				else
				{
					return true;
				}
			case '?' :
				if(*lv_chCompareString == '\0')
				{
					return false;
				}
				return Compare(lv_chCompareString + 1,lv_chCompareMod + 1);
			case '*' :
				if(*(lv_chCompareMod+1) == '\0')
				{
					return true;
				}
				do
				{
					if(Compare(lv_chCompareString,lv_chCompareMod+1)==true)
					{
						return true;
					}
				}while(*(lv_chCompareString++));
				return false;
			case '[' :
				lv_chCompareMod++;
				do
				{
					
					if(*lv_chCompareMod == *lv_chCompareString)
					{
						while(*lv_chCompareMod != '\0' && *lv_chCompareMod != ']')
						{
							lv_chCompareMod++;
						}
						if(*lv_chCompareMod == ']')
						{
							lv_chCompareMod++;
						}
						return Compare(lv_chCompareString+1,lv_chCompareMod);			
					}
					lv_chCompareMod++;
					if((*lv_chCompareMod == ':') && (*(lv_chCompareMod+1) != ']'))
					{
						if((*lv_chCompareString >= *(lv_chCompareMod - 1)) && (*lv_chCompareString <= *(lv_chCompareMod + 1)))
						{
							while(*lv_chCompareMod != '\0' && *lv_chCompareMod != ']')
							{
								lv_chCompareMod++;
							}
							if(*lv_chCompareMod == ']')
							{
								lv_chCompareMod++;
							}
							return Compare(lv_chCompareString+1,lv_chCompareMod);
						}
						lv_chCompareMod++;
						lv_chCompareMod++;

					}
				}while(*lv_chCompareMod != '\0' && *lv_chCompareMod != ']');

				return false;
			default  :
				if(*lv_chCompareString == *lv_chCompareMod)
				{
					return Compare(lv_chCompareString+1,lv_chCompareMod+1);
				}
				else
				{
					return false;
				}
		}
	}
}


//

Interpreter::NotExp::~NotExp()
{
	delete m_BoolExp;
}
const char *Interpreter::NotExp::Execute()
{
	return strcmp(m_BoolExp->Execute(), RET_TRUE) == 0? RET_FALSE:RET_TRUE;
}

const char *Interpreter::NotExp::SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo)
{
	const char *pPosition = Context + 4; //eat 'not('
	
	pPosition = pInterpreter->CompileExp(pPosition, &m_BoolExp, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}		
	if (*pPosition != ')' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
		return pPosition;
	}
	
	return pPosition + 1;
}


//
Interpreter::IfExp::~IfExp()
{
	delete m_BoolExp;
	delete m_True_RetExp;
	delete m_False_RetExp;
}

const char *Interpreter::IfExp::Execute()
{
	if (strcmp(m_BoolExp->Execute(), RET_TRUE) == 0)
		return m_True_RetExp->Execute();
	else
		return m_False_RetExp->Execute();
}

const char *Interpreter::IfExp::SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo)
{
	const char *pPosition = Context + 3; //eat 'if('
	
	//boolexp
	pPosition = pInterpreter->CompileExp(pPosition, &m_BoolExp, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
	if (*pPosition != ',' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_COMMA;
		return pPosition;
	}
	pPosition++;
	
	//true_exp
	pPosition = pInterpreter->CompileExp(pPosition, &m_True_RetExp, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
	if (*pPosition != ',' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_COMMA;
		return pPosition;
	}
	pPosition++;
	
	//false_exp
	pPosition = pInterpreter->CompileExp(pPosition, &m_False_RetExp, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
	if (*pPosition != ')' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
		return pPosition;
	}
	return pPosition + 1;
}

//
Interpreter::CaseExp::~CaseExp() 
{ 
	for(vector<Expression *>::iterator it = m_ExpArray.begin(); it != m_ExpArray.end(); delete *it++) 
	{}
}

const char * Interpreter::CaseExp::Execute()
{
	int UpperBound = m_ExpArray.size() - 1;
	for (int i = 0; i < UpperBound; i += 2)
	{
		if (strcmp(m_ExpArray[i]->Execute(), RET_TRUE) == 0)
			return m_ExpArray[i + 1]->Execute();
	}
	//����Ĭ��ֵ
	return m_ExpArray[UpperBound]->Execute();
}

const char *Interpreter::CaseExp::SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo)
{
	const char *pPosition = Context + 5; //eat 'case('
	
	for (;;)
	{
		//
		Expression *pTempExp = NULL;
		
		pPosition = pInterpreter->CompileExp(pPosition, &pTempExp, ErrorNo);
		if (ErrorNo != COMPILE_NOERROR)
		{
			return pPosition;
		}
		//ȫ���ɹ������ı��ʽ��m_ExpArray����,����ʱ��CaseExp�ͷ�
		m_ExpArray.push_back(pTempExp);
		
		if (*pPosition == ',' )
		{
			pPosition++;
			continue;
		}
		else if (*pPosition == ')' )
		{
			break;
		}
		else
		{
			ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
			return pPosition;
		}
	}
	
	if ( (m_ExpArray.size() < 3) || ( (m_ExpArray.size() - 1) % 2 != 0) )
	{
		ErrorNo = COMPILE_ERROR_CASEEXP_MISSING_SUBEXP;
		return pPosition;
	}
	
	return pPosition + 1;
}

//
Interpreter::AndExp::~AndExp()
{
	delete m_LeftCondition;
	delete m_RightCondition;
}

const char * Interpreter::AndExp::Execute()
{
	// is "false" ?
	if (strcmp(m_RightCondition->Execute(), RET_FALSE) == 0)
	{
		return RET_FALSE;
	}
	// is "false" ?
	if (strcmp(m_LeftCondition->Execute(), RET_FALSE) == 0)
	{
		return RET_FALSE;
	}
	// return "true"
	return RET_TRUE;
}

//
Interpreter::OrExp::~OrExp()
{
	delete m_LeftCondition;
	delete m_RightCondition;
}

const char * Interpreter::OrExp::Execute()
{
	// is "true" ?
	if (strcmp(m_RightCondition->Execute(), RET_TRUE) == 0)
	{
		return RET_TRUE;
	}
	// is "true" ?
	if (strcmp(m_LeftCondition->Execute(), RET_TRUE) == 0)
	{
		return RET_TRUE;
	}
	// return "false"
	return RET_FALSE;
}

//
Interpreter::InExp::~InExp() 
{ 
	for(vector<Expression *>::iterator it = m_ExpArray.begin(); it != m_ExpArray.end(); delete *it++) 
	{}
}

const char * Interpreter::InExp::Execute()
{
	const char *pValue = m_ExpArray[0]->Execute();
	
	for (vector<Expression *>::iterator it = m_ExpArray.begin() + 1; it != m_ExpArray.end(); ++it)
	{
		if (strcmp(pValue, (*it)->Execute()) == 0)
			return RET_TRUE;
	}
	
	return RET_FALSE;
}

const char *Interpreter::InExp::SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo)
{
	const char *pPosition = Context + 3; //eat 'in('
	
	for (;;)
	{
		//
		Expression *pTempExp = NULL;
		
		pPosition = pInterpreter->CompileExp(pPosition, &pTempExp, ErrorNo);
		if (ErrorNo != COMPILE_NOERROR)
		{
			return pPosition;
		}
		//�����ı��ʽ���ȫ������m_ExpArray,��InExp�����ͷ�
		m_ExpArray.push_back(pTempExp);
		
		if (*pPosition == ',' )
		{
			pPosition++;
			continue;
		}
		else if (*pPosition == ')' )
		{
			break;
		}
		else
		{
			ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
			return pPosition;
		}
	}
	
	if ( (m_ExpArray.size() < 2) )
	{
		ErrorNo = COMPILE_ERROR_INEXP_MISSING_SUBEXP;
		return pPosition;
	}
	
	return pPosition + 1;
}


//SubStringExp
Interpreter::SubStringExp::~SubStringExp() 
{ 
	delete m_StringExp;
	delete m_PosExp;
	delete m_LenExp;
}

//���ַ�����1��ʼsubstring(string,pos{,length})
//length��ʡ��,pos��Ϊ����Ϊ��
const char * Interpreter::SubStringExp::Execute()
{
	const char *pString = m_StringExp->Execute();
	long Pos = 0;
	long Length = 0;
	long Strlength = 0;
	
	//��Ҫ����Ƿ������� atol�ɹ�?
	//λ���Լ�����
	Pos = atol(m_PosExp->Execute());
	if (m_LenExp != NULL)
	{
		Length = atol(m_LenExp->Execute());
	}
	else
	{
		Length = sizeof(m_Buffer) - 1;
	}
	//У��
	memset(m_Buffer, 0, sizeof(m_Buffer));
	if ( (pString == NULL) || (*pString == 0) || (Length < 1) ||
		  ((Strlength = strlen(pString)) < Pos) )
		return m_Buffer;
	
	//���ʽ��1��ʼ
	if (Pos > 0)
	{
		Pos--;
	}
	//
	if (Pos >=0 )
	{
		strncpy(m_Buffer, pString + Pos, Length);
	}
	else
	{
		//posΪ��ֵ,�Ӻ���ǰ
		const char *pStartPos = pString + Strlength + Pos;
		if (pStartPos < pString)
			pStartPos = pString;
		strncpy(m_Buffer, pStartPos, Length);	
	}
	return m_Buffer;
}

const char *Interpreter::SubStringExp::SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo)
{
	const char *pPosition = Context + 10; //eat 'substring('
	
	//string value exp
	pPosition = pInterpreter->CompileExp(pPosition, &m_StringExp, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
	if (*pPosition != ',' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_COMMA;
		return pPosition;
	}
	pPosition++;
	
	//pos value exp
	pPosition = pInterpreter->CompileExp(pPosition, &m_PosExp, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
	
	//len value exp ���Ժ���,������Դ�����Ҫȡ���ַ���ĩβ
	if (*pPosition == ',' )
	{
		pPosition++;
		//len value exp
		pPosition = pInterpreter->CompileExp(pPosition, &m_LenExp, ErrorNo);
		if (ErrorNo != COMPILE_NOERROR)
		{
			return pPosition;
		}
	}
	
	if (*pPosition != ')' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
		return pPosition;
	}
	
	return pPosition + 1;
}

//ConnectExp
Interpreter::ConnectExp::~ConnectExp()
{ 
	delete m_FirstStrExp;
	delete m_SecondStrExp;
}


const char * Interpreter::ConnectExp::Execute()
{
	long StringLen = 0;
	
	memset(m_Buffer, 0, sizeof(m_Buffer));
	strncpy(m_Buffer, m_FirstStrExp->Execute(), sizeof(m_Buffer) - 1);
	StringLen = strlen(m_Buffer);
	if (StringLen < (sizeof(m_Buffer) - 1) )
	{
		strncpy(m_Buffer + StringLen, m_SecondStrExp->Execute(), sizeof(m_Buffer) - StringLen - 1);
	}
	
	return m_Buffer;
}

const char *Interpreter::ConnectExp::SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo)
{
	const char *pPosition = Context + 8; //eat 'connect('
	
	//first string
	pPosition = pInterpreter->CompileExp(pPosition, &m_FirstStrExp, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
	if (*pPosition != ',' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_COMMA;
		return pPosition;
	}
	pPosition++;
	
	//second string
	pPosition = pInterpreter->CompileExp(pPosition, &m_SecondStrExp, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}

	if (*pPosition != ')' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
		return pPosition;
	}
	
	return pPosition + 1;
}

//SetExp
Interpreter::SetExp::~SetExp()
{ 
	delete m_VariableExp;
	delete m_StringExp;
}


const char * Interpreter::SetExp::Execute()
{
	return strcpy( (char *)m_VariableExp->Execute(), m_StringExp->Execute() );
}

const char *Interpreter::SetExp::SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo)
{
	const char *pPosition = Context + 4; //eat 'set('
	
	//variable exp $Name
	pPosition = pInterpreter->CompileExp(pPosition, &m_VariableExp, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
	//check the m_VariableExp is VariableExp?
	if (strcmp(m_VariableExp->ExpType(), "VariableExp") != 0)
	{
		ErrorNo = COMPILE_ERROR_SETEXP_FIRSTEXP_NOT_VAR;
		return pPosition;
	}
	
	if (*pPosition != ',' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_COMMA;
		return pPosition;
	}
	pPosition++;
	
	//string exp
	pPosition = pInterpreter->CompileExp(pPosition, &m_StringExp, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}

	if (*pPosition != ')' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
		return pPosition;
	}
	
	return pPosition + 1;
}

//StrCmpExp
Interpreter::StrCmpExp::~StrCmpExp()
{ 
	delete m_StringExp1;
	delete m_StringExp2;
}


const char * Interpreter::StrCmpExp::Execute()
{
	if (m_bIsStrAbove)
	{	//strabove
		return strcmp(m_StringExp1->Execute(), m_StringExp2->Execute()) > 0 ? RET_TRUE:RET_FALSE;
	}
	else
	{	//strbelow
		return strcmp(m_StringExp1->Execute(), m_StringExp2->Execute()) < 0 ? RET_TRUE:RET_FALSE;
	}
}

const char *Interpreter::StrCmpExp::SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo)
{
	const char *pPosition = Context + 9; //eat 'strabove('|'strbelow('
	
	//string1
	pPosition = pInterpreter->CompileExp(pPosition, &m_StringExp1, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
	
	if (*pPosition != ',' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_COMMA;
		return pPosition;
	}
	pPosition++;
	
	//string2
	pPosition = pInterpreter->CompileExp(pPosition, &m_StringExp2, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}

	if (*pPosition != ')' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
		return pPosition;
	}
	
	return pPosition + 1;
}

//NumCMpExp
Interpreter::NumCmpExp::~NumCmpExp()
{ 
	delete m_NumStringExp1;
	delete m_NumStringExp2;
}


const char * Interpreter::NumCmpExp::Execute()
{
	//��Ҫ����Ƿ������� atol�ɹ�? errorno
	if (m_bIsNumAbove)
	{	//numabove
		return atol(m_NumStringExp1->Execute()) > atol(m_NumStringExp2->Execute()) ? RET_TRUE:RET_FALSE;
	}
	else
	{	//numbelow
		return atol(m_NumStringExp1->Execute()) < atol(m_NumStringExp2->Execute()) ? RET_TRUE:RET_FALSE;
	}
}

const char *Interpreter::NumCmpExp::SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo)
{
	const char *pPosition = Context + 9; //eat 'numabove('|'numbelow('
	
	//numstring1
	pPosition = pInterpreter->CompileExp(pPosition, &m_NumStringExp1, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
	
	if (*pPosition != ',' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_COMMA;
		return pPosition;
	}
	pPosition++;
	
	//numstring2
	pPosition = pInterpreter->CompileExp(pPosition, &m_NumStringExp2, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}

	if (*pPosition != ')' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
		return pPosition;
	}
	
	return pPosition + 1;
}

//IsDateTimeExp
Interpreter::IsDateTimeExp::~IsDateTimeExp()
{ 
	delete m_StringExp;
}

//�����¸�ʽ����У��,������Ϸ���'true',����'false'
// ���� 15 yyyymmddhhmisst
// ���� 14 yyyymmddhhmiss
// ���� 13 yymmddhhmisst
// ���� 12 yymmddhhmiss

//У�����:
//1.���ж��Ƿ�����
//2.��ֻ�ж�2000~2050��Ч
//3.�� 01~12
//4.�� (1,3,5,7,8,10,12�� ��01~31) (4,6,9,11�� ��01~30) (2�� �� 01~29)
//5.Сʱ 00~23
//6.�� 00~59
//7.�� 00~59
//8.���� 0~9
const char * Interpreter::IsDateTimeExp::Execute()
{
	//����ʱ��У��
	const char *pDateTimeValue = m_StringExp->Execute();
	int iTemp = strlen(pDateTimeValue);
	char chTemp[10];
	int iTemp2;
	
	if ( (iTemp  < 12 ) || (iTemp > 15) )
		return RET_FALSE;
	
	memset(chTemp, 0, sizeof(chTemp));
	//У����
	if (iTemp < 14)
	{
		strcpy(chTemp, "20");
		strncpy(chTemp + 2, pDateTimeValue, 2);
		pDateTimeValue += 2;
	}
	else
	{
		strncpy(chTemp, pDateTimeValue, 4);
		pDateTimeValue += 4;
	}
	iTemp = atoi(chTemp);
	if ( (iTemp < 2000) || (iTemp > 2050) )
		return RET_FALSE;

	//У����
	strncpy(chTemp, pDateTimeValue, 2);
	chTemp[2] = 0;
	iTemp = atoi(chTemp);
	if ( (iTemp < 1) || (iTemp > 12) )
		return RET_FALSE;
	pDateTimeValue += 2;

	//У����
	strncpy(chTemp, pDateTimeValue, 2);
	chTemp[2] = 0;
	iTemp2 = atoi(chTemp);
	switch(iTemp)
	{
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			if ( (iTemp2 < 1) || (iTemp2 > 31) )
				return RET_FALSE;
			break;
		case 4:
		case 6:
		case 9:
		case 11:
			if ( (iTemp2 < 1) || (iTemp2 > 30) )
				return RET_FALSE;
			break;
		default:
			if ( (iTemp2 < 1) || (iTemp2 > 29) )
				return RET_FALSE;
	}
	pDateTimeValue += 2;
	
	//У��ʱ����
	strncpy(chTemp, pDateTimeValue, 6);
	chTemp[6] = 0;
	//095959 235959
	if ( (strcmp(chTemp, "000000") < 0) || (strcmp(chTemp, "235959") > 0) ||
		( (*chTemp == '2') && (*(chTemp + 1) > '3') ) )
		return RET_FALSE;
	pDateTimeValue += 6;

	//У�����
	if ( (*pDateTimeValue != 0) && 
		( (*pDateTimeValue < '0') || (*pDateTimeValue > '9') ) )
		return RET_FALSE;

	return RET_TRUE;
}

const char *Interpreter::IsDateTimeExp::SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo)
{
	const char *pPosition = Context + 11; //eat 'isdatetime('
	
	//string exp
	pPosition = pInterpreter->CompileExp(pPosition, &m_StringExp, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
	
	if (*pPosition != ')' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
		return pPosition;
	}
	
	return pPosition + 1;
}

//LengthExp
Interpreter::LengthExp::~LengthExp()
{ 
	delete m_StringExp;
}


const char * Interpreter::LengthExp::Execute()
{
	sprintf(m_Length, "%d", strlen(m_StringExp->Execute()) );
	return m_Length;
}

const char *Interpreter::LengthExp::SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo)
{
	const char *pPosition = Context + 7; //eat 'length('
	
	//string exp
	pPosition = pInterpreter->CompileExp(pPosition, &m_StringExp, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
	
	if (*pPosition != ')' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
		return pPosition;
	}
	
	return pPosition + 1;
}

/////////////////

//
Interpreter::DecodeExp::~DecodeExp() 
{ 
	for(vector<Expression *>::iterator it = m_ExpArray.begin(); it != m_ExpArray.end(); delete *it++) 
	{}
}

const char * Interpreter::DecodeExp::Execute()
{
	int UpperBound = m_ExpArray.size() - 1;
	const char *pValue = m_ExpArray[0]->Execute();
	
	for (int i = 1; i < UpperBound; i += 2)
	{
		if (strcmp(pValue, m_ExpArray[i]->Execute()) == 0)
			return m_ExpArray[i + 1]->Execute();
	}
	
	//����Ĭ��ֵ
	return m_ExpArray[UpperBound]->Execute();
}

const char *Interpreter::DecodeExp::SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo)
{
	const char *pPosition = Context + 7; //eat 'decode('
	
	for (;;)
	{
		//
		Expression *pTempExp = NULL;
		
		pPosition = pInterpreter->CompileExp(pPosition, &pTempExp, ErrorNo);
		if (ErrorNo != COMPILE_NOERROR)
		{
			return pPosition;
		}
		//ȫ���ɹ������ı��ʽ��m_ExpArray����,����ʱ��DecodeExp�ͷ�
		m_ExpArray.push_back(pTempExp);
		
		if (*pPosition == ',' )
		{
			pPosition++;
			continue;
		}
		else if (*pPosition == ')' )
		{
			break;
		}
		else
		{
			ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
			return pPosition;
		}
	}
	
	if ( ( m_ExpArray.size() < 4 ) || ( m_ExpArray.size() % 2 != 0 ) )
	{
		ErrorNo = COMPILE_ERROR_DECODEEXP_MISSING_SUBEXP;
		return pPosition;
	}
	
	return pPosition + 1;
}

//////////////commaexp
//comma
Interpreter::CommaExp::~CommaExp() 
{ 
	for (vector<Expression *>::iterator it = m_ExpArray.begin(); it != m_ExpArray.end(); delete *it++) 
	{}
}

const char * Interpreter::CommaExp::Execute()
{
	const char *pValue = NULL;

	for (vector<Expression *>::iterator it = m_ExpArray.begin(); it != m_ExpArray.end(); it++)
	{
		pValue = (*it)->Execute();
	}
	return pValue;

}

const char *Interpreter::CommaExp::SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo)
{
	const char *pPosition = Context + 6; //eat 'comma('
	
	for (;;)
	{
		//
		Expression *pTempExp = NULL;
		
		pPosition = pInterpreter->CompileExp(pPosition, &pTempExp, ErrorNo);
		if (ErrorNo != COMPILE_NOERROR)
		{
			return pPosition;
		}
		//ȫ���ɹ������ı��ʽ��m_ExpArray����,����ʱ��CommaExp�ͷ�
		m_ExpArray.push_back(pTempExp);
		
		if (*pPosition == ',' )
		{
			pPosition++;
			continue;
		}
		else if (*pPosition == ')' )
		{
			break;
		}
		else
		{
			ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
			return pPosition;
		}
	}
	
	if ( m_ExpArray.size() < 1 )
	{
		ErrorNo = COMPILE_ERROR_COMMAEXP_MISSING_SUBEXP;
		return pPosition;
	}
	
	return pPosition + 1;
}

/* added by wulf 2005/10/17 */
//
Interpreter::SumExp::~SumExp() 
{ 
	for(vector<Expression *>::iterator it = m_ExpArray.begin(); it != m_ExpArray.end(); delete *it++) 
	{}
}

const char * Interpreter::SumExp::Execute()
{
	m_lSum = 0;
	m_lCurValue = 0;
	for (vector<Expression *>::iterator it = m_ExpArray.begin(); it != m_ExpArray.end(); ++it)
	{
		m_lCurValue = atol( (*it)->Execute() );
		m_lSum = m_lSum + m_lCurValue;		
	}
	memset( m_szSum, 0, sizeof( m_szSum ) );
	sprintf( m_szSum, "%ld", m_lSum );
	return m_szSum;
}

const char *Interpreter::SumExp::SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo)
{
	const char *pPosition = Context + 4; //eat 'sum('
	
	for (;;)
	{
		//
		Expression *pTempExp = NULL;
		
		pPosition = pInterpreter->CompileExp(pPosition, &pTempExp, ErrorNo);
		if (ErrorNo != COMPILE_NOERROR)
		{
			return pPosition;
		}
		//�����ı��ʽ���ȫ������m_ExpArray,��SumExp�����ͷ�
		m_ExpArray.push_back(pTempExp);
		
		if (*pPosition == ',' )
		{
			pPosition++;
			continue;
		}
		else if (*pPosition == ')' )
		{
			break;
		}
		else
		{
			ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
			return pPosition;
		}
	}
	
	if ( (m_ExpArray.size() < 2) )
	{
		ErrorNo = COMPILE_ERROR_SUMEXP_MISSING_SUBEXP;
		return pPosition;
	}
	
	return pPosition + 1;
}
//end of sum

//list
Interpreter::ListExp::ListExp() 
{
	m_iInitFlag = 0;
	m_iRowNum = 0;
	//add by wulf at 20051216
	m_iParamNum = 0;
	memset( m_szUpdateFlag, 0, sizeof( m_szUpdateFlag ) );
}

Interpreter::ListExp::~ListExp() 
{ 
	for(vector<Expression *>::iterator it = m_ExpArray.begin(); it != m_ExpArray.end(); delete *it++) 
	{}
	m_ExpArray.clear();
	for(vector<CListRow>::iterator it = m_RowArray.begin(); it != m_RowArray.end(); delete [](*it).m_pField,++it) 
	{}
	m_RowArray.clear();
}

void Interpreter::ListExp::Init()
{
	//printf("Init() begin $$$$$$$$$$$$$$$$$$$$$$\n");
	//modify by wulf at 20051216
//	int iNum = m_ExpArray.size() - 1;
	CBindSQL ds( DBConn );
	if( m_iInitFlag != 1 )
	{
		m_iParamNum = m_ExpArray.size() - 2;
		strcpy( m_szSqlId, m_ExpArray[m_iParamNum]->Execute() );
		
		if( m_CurRow.m_pField != NULL )
		{
			delete []m_CurRow.m_pField;
			m_CurRow.m_pField = NULL;	
		}
		m_CurRow.m_pField = new SField[m_iParamNum];
		m_CurRow.m_iFieldNum = m_iParamNum;	
		
		if( strlen( m_szSqlId ) <= 16 )
		{
			memset( m_szSqlContent, 0, sizeof( m_szSqlContent ) );
			ds.Open(" select sql_value from list_sql where sql_id=:a", SELECT_QUERY );
			ds<<m_szSqlId;
			if( !( ds>>m_szSqlContent ) )
			{
				ds.Close();
				sprintf( errmsg, " select sql_value from list_sql where sql_id=%s Error! ", m_szSqlContent );				
  			printf("%s\n",errmsg);
				throw CF_CError(ERR_TYPE_DB,ERR_LEVEL_HIG,SELECT_ERROR_FROM_DB,SELECT_ERROR_FROM_DB,errmsg,__FILE__,__LINE__);
			}
			ds.Close();			
			m_cTabDriveUpdt = 'Y';			
		}
		else
		{
			strcpy( m_szSqlContent, m_szSqlId	);
			m_cTabDriveUpdt = 'N';
		}
	}	
	ds.Open( m_szSqlContent, SELECT_QUERY );  
	int flag = 0;
	m_iRowNum = 0;
	for(;;)
	{
		CListRow TmpRow;
		TmpRow.m_pField = new SField[m_iParamNum];
		TmpRow.m_iFieldNum = m_iParamNum;
		++m_iRowNum;
		//printf( "RowNum = %d\n", m_iRowNum );
		for( int i=0; i<m_iParamNum; ++i )
		{	
			if( !( ds>>TmpRow.m_pField[i].m_szValue ) )
			{				
				flag = 1;
				break;				
			}
			//printf( "Value[%d]=%s\n", i, TmpRow.m_pField[i].m_szValue );							
		}
		if( flag == 1 )
		{
			delete []TmpRow.m_pField;
			TmpRow.m_pField = NULL;
			break;
		}		
		m_RowArray.push_back( TmpRow );	
	} 	
	ds.Close();
	if( m_cTabDriveUpdt == 'Y' )
	{
		ds.Open("update list_sql set update_flag='N' where sql_id =:a", NONSELECT_DML);
		ds<<m_szSqlId;
		ds.Execute();
		ds.Close();		
		DBConn.Commit();
		strcpy( m_szUpdateFlag, "N" );	
	}
	else
	{
		strcpy( m_szUpdateFlag, m_ExpArray[m_iParamNum + 1]->Execute() );
	}
			
	sort( m_RowArray.begin(), m_RowArray.end(), CompareRow );		
	m_iInitFlag = 1;	
}

bool CompareRow( const CListRow &Src, const CListRow &Dest )
{
	int ret = 0;
	for( int i = 0; i< Src.m_iFieldNum; i++ )
	{
		ret = strcmp( Src.m_pField[i].m_szValue, Dest.m_pField[i].m_szValue );
		if( ret != 0 )
			return ( ret < 0 ) ;
	}
	return ( ret < 0 );
}

const char * Interpreter::ListExp::Execute()
{
	//modify by wulf at 20051219
	if( m_iInitFlag != 1 )
	{	
		Init();
	}
	if( 0 != strcmp( m_szUpdateFlag, m_ExpArray[m_iParamNum + 1]->Execute() ) )
	{
		/* release all spaces before init() */
		for(vector<CListRow>::iterator it = m_RowArray.begin(); it != m_RowArray.end(); delete [](*it).m_pField,++it) 
		{}		
		m_RowArray.clear();
		Init();		
	}
	int i=0;
	for( vector<Expression *>::iterator it = m_ExpArray.begin(); it != m_ExpArray.end() - 2; ++it, ++i )
	{
		strcpy( m_CurRow.m_pField[i].m_szValue, (*it)->Execute() );
	}
		
	if( binary_search( m_RowArray.begin(), m_RowArray.end(), m_CurRow , CompareRow ) == true ) 
		return RET_TRUE;	
	else
		return RET_FALSE;
}

const char *Interpreter::ListExp::SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo)
{
	const char *pPosition = Context + 5; //eat 'list('
	
	for (;;)
	{
		//
		Expression *pTempExp = NULL;
		
		pPosition = pInterpreter->CompileExp(pPosition, &pTempExp, ErrorNo);
		if (ErrorNo != COMPILE_NOERROR)
		{
			return pPosition;
		}
		//�����ı��ʽ���ȫ������m_ExpArray,��ListExp�����ͷ�
		m_ExpArray.push_back(pTempExp);
		
		if (*pPosition == ',' )
		{
			pPosition++;
			continue;
		}
		else if (*pPosition == ')' )
		{
			break;
		}
		else
		{
			ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
			return pPosition;
		}
	}
	
	if ( (m_ExpArray.size() < 2) )
	{
		ErrorNo = COMPILE_ERROR_LISTEXP_MISSING_SUBEXP;
		return pPosition;
	}
	
	return pPosition + 1;
}
/* end of 2005/10/17 */

	/* add by wulf 2005/11/28 �ڱ��ʽ�е��ò��*/
int Interpreter::PluginExp::m_iObjCount = 0;
PluginProxy Interpreter::PluginExp::m_PluginProxy;

Interpreter::PluginExp::PluginExp()
{
	m_iInitFlag = 0;
	memset( m_szFuncName, 0, sizeof( m_szFuncName ) );
	memset( m_szSlFileName, 0, sizeof( m_szSlFileName ) );
//	++m_iObjCount;
	m_pPluginObj = NULL;
	//add by wulf at 20060224
	for( int i=0; i<ATTRINUMBER; i++ ) 
		m_pAFieldValue[i] = NULL;
}

Interpreter::PluginExp::~PluginExp()
{
	for(vector<Expression *>::iterator it = m_ExpArray.begin(); it != m_ExpArray.end(); delete *it++) 
	{}
	--m_iObjCount;
	/* close the plugin file */	
	if( m_iObjCount == 0 )
		m_PluginProxy.CloseSLFile();	
}

const char* Interpreter::PluginExp::Execute()
{
	if( m_iInitFlag != 1 )
	{
		Init();		
	}
	
	//commented by wulf at 20060224
//	int i = 0;	
//	for( vector<Expression *>::iterator it = m_ExpArray.begin() + 1; it != m_ExpArray.end(); ++it, ++i )
//	{
//		strcpy( m_AFieldValue[i], (*it)->Execute() );
//	}
	//modify by wulf at 20060224
	return m_pPluginObj->execute( m_PacketParser );
//	if( strcmp( RET_TRUE, m_pPluginObj->execute( m_PacketParser ) ) == 0 )
//		return RET_TRUE;
//	else
//		return RET_FALSE;
}

void Interpreter::PluginExp::Init()
{
	++m_iObjCount;
	/* get the file name for plugin */
	if( m_iObjCount == 1 )
	{
		CBindSQL ds( DBConn );
		//modify by wulf at 20060613
		ds.Open("select B.VAR_VALUE from PIPE A, T_SERVER_ENV B  where A.PIPE_ID=:pipe_id and A.SERVER_ID=B.SERVER_ID and B.VAR_NAME=:varname", SELECT_QUERY );
		ds<<m_szPipeId<<m_szVarname;
		if( !( ds>>m_szSlFileName ) )
		{
			ds.Close();
			sprintf( errmsg, "select B.VAR_VALUE from PIPE A, T_SERVER_ENV B  where A.PIPE_ID=%s and A.SERVER_ID=B.SERVER_ID and B.VAR_NAME='EXP_PLUGIN' Error!", m_szPipeId );				
  		printf("%s\n",errmsg);
			throw CF_CError(ERR_TYPE_DB,ERR_LEVEL_HIG,SELECT_ERROR_FROM_DB,SELECT_ERROR_FROM_DB,errmsg,__FILE__,__LINE__);
		}	
		ds.Close();
//		ds.Open("select VARVALUE from GLOBAL_ENV where VARNAME='EXP_PLUGIN'", SELECT_QUERY );
//		if( !( ds>>m_szSlFileName ) )
//		{
//			ds.Close();
//			sprintf( errmsg, "select VARVALUE from GLOBAL_ENV where VARNAME='EXP_PLUGIN' Error!" );				
//  		printf("%s\n",errmsg);
//			throw CF_CError(ERR_TYPE_DB,ERR_LEVEL_HIG,SELECT_ERROR_FROM_DB,SELECT_ERROR_FROM_DB,errmsg,__FILE__,__LINE__);
//		}	
//		ds.Close();
	
		if ( !m_PluginProxy.Initialize( m_szSlFileName, "CreateEntryPoint" ) )
		{
			sprintf( errmsg, "init slfile %s Error! ", m_szSlFileName );				
  		printf("%s\n",errmsg);
			throw CF_CError(ERR_TYPE_DB,ERR_LEVEL_HIG,OPEN_PLUGIN_ERROR,OPEN_PLUGIN_ERROR,errmsg,__FILE__,__LINE__);
		
		}
	}
	
	strcpy( m_szFuncName, m_ExpArray[0]->Execute() );
	
	m_pPluginObj = m_PluginProxy[m_szFuncName];
	if( m_pPluginObj == NULL )
	{
		sprintf( errmsg, "No found class %s !", m_szFuncName );				
		printf("%s\n",errmsg);
		throw CF_CError(ERR_TYPE_DB,ERR_LEVEL_HIG,OPEN_PLUGIN_ERROR,OPEN_PLUGIN_ERROR,errmsg,__FILE__,__LINE__);	
	}
	
	//add by wulf at 20060224
	int i = 0;	
	for( vector<Expression *>::iterator it = m_ExpArray.begin() + 1; it != m_ExpArray.end(); ++it, ++i )
	{
		m_pAFieldValue[i] = (char*)(*it)->Execute();
		if( i >= ATTRINUMBER )
		{
			sprintf( errmsg, "PluginExp Init Err,because too many params send to plugin!" );				
			printf("%s\n",errmsg);
			throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_HIG,COMPILE_ERROR_INVALID_PARAM,COMPILE_ERROR_INVALID_PARAM,errmsg,__FILE__,__LINE__);	
		}
	}
	
	for( int i=0; i < ( m_ExpArray.size() - 1 ); ++i )
	{
		m_PacketParser.setFieldValue( i, m_pAFieldValue[i], MAX_FIELD_LEN );
	}
	m_iInitFlag = 1;
}

const char *Interpreter::PluginExp::SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo)
{
	const char *pPosition = Context + 7; //eat 'plugin('
	
	for (;;)
	{
		//
		Expression *pTempExp = NULL;
		
		pPosition = pInterpreter->CompileExp(pPosition, &pTempExp, ErrorNo);
		if (ErrorNo != COMPILE_NOERROR)
		{
			return pPosition;
		}
		//�����ı��ʽ���ȫ������m_ExpArray,��PluginExp�����ͷ�
		m_ExpArray.push_back(pTempExp);
		
		if (*pPosition == ',' )
		{
			pPosition++;
			continue;
		}
		else if (*pPosition == ')' )
		{
			break;
		}
		else
		{
			ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
			return pPosition;
		}
	}
	
	if ( ( m_ExpArray.size() < 1 ) )
	{
		ErrorNo = COMPILE_ERROR_PLUGINEXP_MISSING_SUBEXP;
		return pPosition;
	}
	
	return pPosition + 1;
}

/* add by wulf 2005/11/29 */
//for likein
Interpreter::LikeInExp::~LikeInExp() 
{ 
	for(vector<Expression *>::iterator it = m_ExpArray.begin(); it != m_ExpArray.end(); delete *it++) 
	{}
}

const char * Interpreter::LikeInExp::Execute()
{
	const char *pValue = m_ExpArray[0]->Execute();
	
	for (vector<Expression *>::iterator it = m_ExpArray.begin() + 1; it != m_ExpArray.end(); ++it)
	{
		if( memcmp( pValue, (*it)->Execute(), strlen( (*it)->Execute() ) ) == 0 )
			return RET_TRUE;
	}
	
	return RET_FALSE;
}

const char *Interpreter::LikeInExp::SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo)
{
	const char *pPosition = Context + 7; //eat 'likein('
	
	for (;;)
	{
		//
		Expression *pTempExp = NULL;
		
		pPosition = pInterpreter->CompileExp(pPosition, &pTempExp, ErrorNo);
		if (ErrorNo != COMPILE_NOERROR)
		{
			return pPosition;
		}
		//�����ı��ʽ���ȫ������m_ExpArray,��LikeInExp�����ͷ�
		m_ExpArray.push_back(pTempExp);
		
		if (*pPosition == ',' )
		{
			pPosition++;
			continue;
		}
		else if (*pPosition == ')' )
		{
			break;
		}
		else
		{
			ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
			return pPosition;
		}
	}
	
	if ( (m_ExpArray.size() < 2) )
	{
		ErrorNo = COMPILE_ERROR_LIKEINEXP_MISSING_SUBEXP;
		return pPosition;
	}
	
	return pPosition + 1;
}

//for between
Interpreter::BetweenExp::~BetweenExp()
{ 
	delete m_pValue;
	delete m_pLeft;
	delete m_pRight;
}


const char * Interpreter::BetweenExp::Execute()
{
	if( strcmp( m_pValue->Execute(), m_pLeft->Execute()) >= 0 &&
			strcmp( m_pValue->Execute(), m_pRight->Execute()) <= 0 )
		return RET_TRUE;
	return RET_FALSE;
}

const char *Interpreter::BetweenExp::SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo)
{
	const char *pPosition = Context + 8; //eat 'between('
	
	//value
	pPosition = pInterpreter->CompileExp(pPosition, &m_pValue, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
	
	if (*pPosition != ',' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_COMMA;
		return pPosition;
	}
	pPosition++;
	
	//left
	pPosition = pInterpreter->CompileExp(pPosition, &m_pLeft, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
	
	if (*pPosition != ',' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_COMMA;
		return pPosition;
	}
	pPosition++;
	
	pPosition = pInterpreter->CompileExp(pPosition, &m_pRight, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
		
	if (*pPosition != ')' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
		return pPosition;
	}
	
	return pPosition + 1;
}

/*
 * add by wulf at 2006/02/15
 * adddays(Days,OrgDate)	 
 * DaysΪ�����ӵ�������OrgDateΪԭ�������ڣ�8λ��
 * ���ַ����ķ�ʽ������Ӻ�����ڣ�8λ��
 */
Interpreter::AddDaysExp::~AddDaysExp()
{ 
	delete m_ExpDays;
	delete m_ExpOrgDate;	
}

const char * Interpreter::AddDaysExp::Execute()
{
	char szTgtDate[8+1];
	memset( szTgtDate, 0, sizeof(szTgtDate) );
	if( addDays(atoi(m_ExpDays->Execute()), m_ExpOrgDate->Execute(),szTgtDate) == NULL )
	{
		sprintf( errmsg, "AddDaysExp Execute Err when call function addDays" );				
		printf("%s\n",errmsg);
		throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_HIG,COMPILE_ERROR_INVALID_PARAM,COMPILE_ERROR_INVALID_PARAM,errmsg,__FILE__,__LINE__);	
	}
		
	return szTgtDate;
}

const char *Interpreter::AddDaysExp::SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo)
{
	const char *pPosition = Context + 8; //eat 'adddays('
	
	//Days
	pPosition = pInterpreter->CompileExp(pPosition, &m_ExpDays, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
	
	if (*pPosition != ',' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_COMMA;
		return pPosition;
	}
	pPosition++;
	
	//OrgDate
	pPosition = pInterpreter->CompileExp(pPosition, &m_ExpOrgDate, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
		
	if (*pPosition != ')' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
		return pPosition;
	}
	
	return pPosition + 1;
}


/*
 * add by wulf at 2006/02/15
 * minusdays(FirstDay,SecondDay)	 
 * FirstDayΪ��һ�����ڣ�SecondDayΪ�ڶ������ڣ�8λ��   
 * ���ַ����ķ�ʽ������������
 */
Interpreter::MinusDaysExp::~MinusDaysExp()
{ 
	delete m_ExpFirstDay;
	delete m_ExpSecondDay;	
}

const char *Interpreter::MinusDaysExp::Execute()
{
	long iDays;
	char szDays[33];

	iDays = minusDays( m_ExpFirstDay->Execute(), m_ExpSecondDay->Execute() );
	if( iDays == ERR_DATE_FORMAT )
	{
		sprintf( errmsg, "MinusDaysExp Execute Err when call function minusDays" );				
		printf("%s\n",errmsg);
		throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_HIG,COMPILE_ERROR_INVALID_PARAM,COMPILE_ERROR_INVALID_PARAM,errmsg,__FILE__,__LINE__);		
	}
	
	sprintf( szDays, "%d", iDays );
	return szDays;
}

const char *Interpreter::MinusDaysExp::SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo)
{
	const char *pPosition = Context + 10; //eat 'minusdays('
	
	//FirstDay
	pPosition = pInterpreter->CompileExp(pPosition, &m_ExpFirstDay, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
	
	if (*pPosition != ',' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_COMMA;
		return pPosition;
	}
	pPosition++;
	
	//SecondDay
	pPosition = pInterpreter->CompileExp(pPosition, &m_ExpSecondDay, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
		
	if (*pPosition != ')' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
		return pPosition;
	}
	
	return pPosition + 1;
}

/*
 * add by wulf at 2006/02/15
 * addtime(seconds,OrgTime)	 
 * secondsΪ�����ӵ�������OrgTimeΪԭ����ʱ�䣨14λ��
 * ���ַ����ķ�ʽ������Ӻ��ʱ�䣨14λ��
 */
Interpreter::AddTimeExp::~AddTimeExp()
{ 
	delete m_ExpSeconds;
	delete m_ExpOrgTime;	
}

const char *Interpreter::AddTimeExp::Execute()
{
	char szTgtTime[14+1];
	
	long lTgtTime;
	long lOgrTime;
	lOgrTime = timeStr2Time(m_ExpOrgTime->Execute());	
	if( lOgrTime == -1 )
	{
		sprintf( errmsg, "AddTimeExp Execute Err when call function timeStr2Time" );				
		printf("%s\n",errmsg);
		throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_HIG,COMPILE_ERROR_INVALID_PARAM,COMPILE_ERROR_INVALID_PARAM,errmsg,__FILE__,__LINE__);			
	} 
	lTgtTime = lOgrTime + atoi( m_ExpSeconds->Execute() );	
	memset(szTgtTime, 0, sizeof(szTgtTime));
	time2TimeStr(lTgtTime, szTgtTime);
		
	return szTgtTime;
}

const char *Interpreter::AddTimeExp::SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo)
{
	const char *pPosition = Context + 8; //eat 'addtime('
	
	//seconds
	pPosition = pInterpreter->CompileExp(pPosition, &m_ExpSeconds, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
	
	if (*pPosition != ',' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_COMMA;
		return pPosition;
	}
	pPosition++;
	
	//OrgTime
	pPosition = pInterpreter->CompileExp(pPosition, &m_ExpOrgTime, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
		
	if (*pPosition != ')' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
		return pPosition;
	}
	
	return pPosition + 1;
}

/*
 * add by wulf at 2006/02/15
 * minustime(FirstTime,SecondTime)	 
 * FirstTimeΪ��һ��ʱ�䣬SecondTimeΪ�ڶ���ʱ�䣨14λ��   
 * ���ַ����ķ�ʽ������������
 */
Interpreter::MinusTimeExp::~MinusTimeExp()
{ 
	delete m_ExpFirstTime;
	delete m_ExpSecondTime;	
}

const char * Interpreter::MinusTimeExp::Execute()
{
	long lFirstTime;
	long lSecondTime;
	char  szSeconds[33];;	
	memset( szSeconds, 0, sizeof(szSeconds) );
	lFirstTime = timeStr2Time( m_ExpFirstTime->Execute() );
	lSecondTime = timeStr2Time( m_ExpSecondTime->Execute() );
	if( (lFirstTime != -1) && (lSecondTime != -1) )	
		sprintf( szSeconds, "%d", lFirstTime-lSecondTime );
	else
	{
		sprintf( errmsg, "MinusTimeExp Execute Err when call function timeStr2Time" );				
		printf("%s\n",errmsg);
		throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_HIG,COMPILE_ERROR_INVALID_PARAM,COMPILE_ERROR_INVALID_PARAM,errmsg,__FILE__,__LINE__);					
	}
	return szSeconds;
}

const char *Interpreter::MinusTimeExp::SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo)
{
	const char *pPosition = Context + 10; //eat 'minustime('
	
	//FirstTime
	pPosition = pInterpreter->CompileExp(pPosition, &m_ExpFirstTime, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
	
	if (*pPosition != ',' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_COMMA;
		return pPosition;
	}
	pPosition++;
	
	//SecondTime
	pPosition = pInterpreter->CompileExp(pPosition, &m_ExpSecondTime, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
		
	if (*pPosition != ')' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
		return pPosition;
	}
	
	return pPosition + 1;
}



/*
 * add by weixy at 2008/12/01
 * difftime(FirstTime,SecondTime)	 
 * FirstTimeΪ��һ��ʱ�䣬SecondTimeΪ�ڶ���ʱ�䣨14λ��   
 * ���ַ����ķ�ʽ���������������������
 */
Interpreter::DiffTimeExp::~DiffTimeExp()
{ 
	delete m_ExpFirstTime;
	delete m_ExpSecondTime;	
}

const char * Interpreter::DiffTimeExp::Execute()
{
	long lFirstTime;
	long lSecondTime;
	char  szSeconds[33];;	
	memset( szSeconds, 0, sizeof(szSeconds) );
	lFirstTime = timeStr2Time( m_ExpFirstTime->Execute() );
	lSecondTime = timeStr2Time( m_ExpSecondTime->Execute() );
	if( (lFirstTime != -1) && (lSecondTime != -1) )	
		sprintf( szSeconds, "%d", abs((lFirstTime-lSecondTime)/86400) );
	else
	{
		sprintf( errmsg, "DiffTimeExp Execute Err when call function timeStr2Time" );				
		printf("%s\n",errmsg);
		throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_HIG,COMPILE_ERROR_INVALID_PARAM,COMPILE_ERROR_INVALID_PARAM,errmsg,__FILE__,__LINE__);					
	}
	return szSeconds;
}

const char *Interpreter::DiffTimeExp::SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo)
{
	const char *pPosition = Context + 9; //eat 'difftime('
	
	//FirstTime
	pPosition = pInterpreter->CompileExp(pPosition, &m_ExpFirstTime, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
	
	if (*pPosition != ',' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_COMMA;
		return pPosition;
	}
	pPosition++;
	
	//SecondTime
	pPosition = pInterpreter->CompileExp(pPosition, &m_ExpSecondTime, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
	{
		return pPosition;
	}
		
	if (*pPosition != ')' )
	{
		ErrorNo = COMPILE_ERROR_MISSING_RIGHT_BRACKET;
		return pPosition;
	}
	
	return pPosition + 1;
}

//end add by weixy 2008-12-01
