/*******************************************************************************
*IndexTree.cpp
*this program is designed for blur(ambigulous) rule search in rate module of 
*zhjs system.
*created by tanj 2005.4.15
*******************************************************************************/

#include "IndexTree.h"
CIndexTree::CIndexTree(int iRuleItemCount)
{
	Root = NULL;
	INDEXTREE_MAX_LEVEL = iRuleItemCount + 1; //还要加上生效时间
	NodeArray = new SIndexNode[INDEXTREE_MAX_LEVEL];
	 m_SyntaxTree.push_back(NULL);//add by liuw 200800721 for 表达式语法树
}
CIndexTree::~CIndexTree()
{
	clearNode(Root, 0);
	if (NodeArray != NULL)
	{
		delete[] NodeArray;
	}
}

	
bool CIndexTree::BlurCmpBlur(const SIndexNode &First, const SIndexNode &Second)
{
	if (strlen(First.szStartTime) == 0)
	{
		if (strcmp(First.szField, Second.szField) == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		if (strcmp(First.szStartTime, Second.szStartTime) == 0 &&
		    strcmp(First.szEndTime, Second.szEndTime) == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}
bool CIndexTree::BlurCmpExact(const SIndexNode &BlurNode, const SIndexNode &ExactNode)
{
		  int iErrorValue;
	char szResult[255];
	if (strlen(BlurNode.szStartTime) == 0)
	{      
		if (BlurNode.iMatchMode == 1)
		{
		if (BlurNode.iMatchMode == 1)
		{
			if (strlen(BlurNode.szField) == 0)
			{
				return true;
			}
			if (strncmp(BlurNode.szField, "~", 1) == 0  && 
			    strncmp(&(BlurNode.szField[1]), ExactNode.szField, strlen(BlurNode.szField) - 1) != 0 )
			{
				return true;
			}
			if (strncmp(BlurNode.szField, "~", 1) != 0  && 
			    strncmp(BlurNode.szField, ExactNode.szField, strlen(BlurNode.szField)) == 0 )
			{
				return true;
			}
			return false;
		}
		else if(BlurNode.iMatchMode == 2)
		{
			if (strcmp(BlurNode.szField, ExactNode.szField) == 0)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else if (BlurNode.iMatchMode == 3)
		{
			if (strlen(BlurNode.szField) == 0)
			{
				return true;
			}
			else
				{
					if ( !memcmp( Operation(szResult, sizeof(szResult), &iErrorValue, BlurNode.szField, ExactNode),"false",5 ) ) 
						return false;
					else return true;
				}
		}
	}
}
	else
	{
		if (strcmp(BlurNode.szStartTime, ExactNode.szStartTime) == 0 &&
		    strcmp(BlurNode.szEndTime, ExactNode.szEndTime) == 0
		   )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

bool CIndexTree::BlurCmpCdr(const SIndexNode &BlurNode, const SIndexNode &CdrNode)
{
	  int iErrorValue;
	char szResult[255];
	if (strlen(BlurNode.szStartTime) == 0)
	{         
		if (BlurNode.iMatchMode == 1)
		{
			if (strlen(BlurNode.szField) == 0)
			{
				return true;
			}
			if (strncmp(BlurNode.szField, "~", 1) == 0  && 
			    strncmp(&(BlurNode.szField[1]), CdrNode.szField, strlen(BlurNode.szField) - 1) != 0 )
			{
				return true;
			}
			if (strncmp(BlurNode.szField, "~", 1) != 0  && 
			    strncmp(BlurNode.szField, CdrNode.szField, strlen(BlurNode.szField)) == 0 )
			{
				return true;
			}
			return false;
		}
		else if(BlurNode.iMatchMode == 2)
		{
			if (strcmp(BlurNode.szField, CdrNode.szField) == 0)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else if (BlurNode.iMatchMode == 3)
		{
			if (strlen(BlurNode.szField) == 0)
			{
				return true;
			}
			else
				{
					if ( !memcmp( Operation(szResult, sizeof(szResult), &iErrorValue, BlurNode.szField, CdrNode),"false",5 ) )
						{
							if ( iErrorValue )
							{
								printf( "excute compiler =%s= err",BlurNode.szField);
								exit(-1);
							}							 
							return false;
						}
					else return true;
				}
		}
	}
	else
	{	
    if (strlen(BlurNode.szEndTime) == 0)
    {
    	if (strcmp(BlurNode.szStartTime, CdrNode.szStartTime) <= 0)
    	{
    		return true;
    	}
    	else
    	{
    		return false;
    	}
    }
    else
    {
    	if (strcmp(BlurNode.szStartTime, CdrNode.szStartTime) <= 0  &&
    	    strcmp(BlurNode.szEndTime, CdrNode.szStartTime) >= 0)
    	{
    		return true;
    	}
    	else
    	{
    		return false;
    	}
    }
  }
}


//the parameter _NodeArray is a array, which means INDEXTREE_MAX_LEVEL nodes
bool CIndexTree::insertRule(SIndexNode *NodeArray_, SRuleValue Value)
{
	for (int i = 0; i < INDEXTREE_MAX_LEVEL; i++)
	{
		CIndexTree::NodeArray[i] = NodeArray_[i];
	}
	return insertNode(Root, Value, 0);
}

bool CIndexTree::insertNode(SIndexNode* &pInNode_, const SRuleValue &Value_, int iLevel_)
{
	if (NULL == pInNode_)
	{
		pInNode_ = new SIndexNode(NodeArray[iLevel_]);
		if ( pInNode_ == NULL)
		{
   		return false;
    }
    if (iLevel_ == INDEXTREE_MAX_LEVEL -1)
    {
    	//this is the last level
    	pInNode_->pRuleValue = new SRuleValue(Value_);
    	if (pInNode_->pRuleValue == NULL)
    	{
    		return false;
    	}
    	return true;
    }
    return insertNode(pInNode_->pNextLevel,Value_, iLevel_ + 1);
	}
	else
	{
		SIndexNode *pTemp1,*pTemp2;
		pTemp1 = pInNode_;
		pTemp2 = pInNode_;
		for ( ; pTemp2 != NULL; pTemp1 = pTemp2, pTemp2 = pTemp2->pNextBrother)
		{
			if (BlurCmpBlur(*pTemp2, NodeArray[iLevel_]))
			{
		    if (iLevel_ == INDEXTREE_MAX_LEVEL -1)
        {
        	//this is the last level
        	pTemp2->pRuleValue = new SRuleValue(Value_);
        	if (pTemp2->pRuleValue == NULL)
        	{
        		return false;
        	}
        	return true;
        }
        else
        {
        	return insertNode(pTemp2->pNextLevel, Value_, iLevel_ + 1);
        }
			}
		}
		pTemp1->pNextBrother = new SIndexNode(NodeArray[iLevel_]);
		if (pTemp1->pNextBrother == NULL)
		{
			return false;
		}
    if (iLevel_ == INDEXTREE_MAX_LEVEL - 1)
    {
    	//this is the last level
    	pTemp1->pNextBrother->pRuleValue = new SRuleValue(Value_);
    	if (pTemp1->pNextBrother->pRuleValue == NULL)
    	{
    		return false;
    	}
    	return true;
    }
    else
    {
    	return insertNode(pTemp1->pNextBrother->pNextLevel, Value_, iLevel_ + 1);
    }
	}
}

void CIndexTree::travel()
{
        cout<<"INDEXTREE_MAX_LEVEL:"<<INDEXTREE_MAX_LEVEL<<endl;
	travel(Root, 0);
}
void CIndexTree::travel(SIndexNode* pNode, int iLevel)
{
	cout<<"iLevel:"<<iLevel<<endl;
        SIndexNode *pTemp = pNode;
	for ( ; pTemp != NULL; pTemp = pTemp->pNextBrother)
	{
	  NodeArray[iLevel] = *pTemp;
		if (iLevel == INDEXTREE_MAX_LEVEL - 1)
		{
			for (int i = 0; i < INDEXTREE_MAX_LEVEL; i++)
			{
				if (strlen(NodeArray[i].szStartTime) == 0)
				{
					cout<<NodeArray[i].szField<<"|";
				}
				else
				{
					cout<<NodeArray[i].szStartTime<<"|"<<NodeArray[i].szEndTime<<"|";
				}
			}
			cout<<pTemp->pRuleValue->iRuleNo<<"|"<<pTemp->pRuleValue->szRateGroupId<<endl;
		  return;
		}
		travel(pTemp->pNextLevel, iLevel + 1);
	}
}
void CIndexTree::clearNode(SIndexNode *pNode, int iLevel)
{
        //travel(pNode,iLevel);
        //cout<<"in clearNode,iLevel:"<<iLevel<<endl;
	SIndexNode *pTemp1, *pTemp2;
	pTemp1 = pNode;
        //if(pTemp1->pNextBrother == NULL)
           //cout<<"pTemp1->pNextBrother is NULL"<<endl;
	pTemp2 = pTemp1->pNextBrother;
	for (; pTemp1 != NULL; )
	{
		if (iLevel == INDEXTREE_MAX_LEVEL - 1)
		{
			if (pTemp1->pRuleValue != NULL)
			{
                               // cout<<"delete pRuleValue:"<<pTemp1->pRuleValue->iRuleNo<<endl;
				delete pTemp1->pRuleValue;
				pTemp1->pRuleValue = NULL;
			}
			delete pTemp1;
			pTemp1 = NULL;
		}
		else
		{
			clearNode(pTemp1->pNextLevel, iLevel + 1);
			//added by tanj 20060422
                        //cout<<"clear ilevel+1"<<endl;
			delete pTemp1;
                        pTemp1 = NULL;
		}
		pTemp1 = pTemp2;
		if (pTemp2 != NULL)
		{
			pTemp2 = pTemp2->pNextBrother;
		}
	}
}


bool CIndexTree::searchRule(SIndexNode *NodeArray_, SRuleValue &Value)
{
	for (int i = 0; i < INDEXTREE_MAX_LEVEL; i++)
	{
		CIndexTree::NodeArray[i] = NodeArray_[i];
	}
	return searchRule(Root, Value, 0);
}	

bool CIndexTree::searchRule(SIndexNode *pNode, SRuleValue &Value, int iLevel)
{
	if (pNode == NULL)
	{
		return false;
	}
	SIndexNode *pTemp;
	for (pTemp = pNode; pTemp != NULL; pTemp = pTemp->pNextBrother)
	{
		if (BlurCmpExact(*pTemp,NodeArray[iLevel]) && iLevel < INDEXTREE_MAX_LEVEL - 1)
		{
			if (searchRule(pTemp->pNextLevel, Value, iLevel + 1))
			{
				return true;
			}
			else
			{
				continue;
			}
		}
		if (BlurCmpExact(*pTemp, NodeArray[iLevel]) && iLevel == INDEXTREE_MAX_LEVEL - 1)
		{
			Value = *(pTemp->pRuleValue);
			return true;
		}
	}
	return false;
}
			
	
bool CIndexTree::searchCdr(SIndexNode *NodeArray_, SRuleValue &Value)
{
	for (int i = 0; i < INDEXTREE_MAX_LEVEL; i++)
	{
		CIndexTree::NodeArray[i] = NodeArray_[i];
	}
	return searchCdr(Root, Value, 0);
}	

bool CIndexTree::searchCdr(SIndexNode *pNode, SRuleValue &Value, int iLevel)
{
	if (pNode == NULL)
	{
		return false;
	}
	SIndexNode *pTemp;
	for (pTemp = pNode; pTemp != NULL; pTemp = pTemp->pNextBrother)
	{
		if (BlurCmpCdr(*pTemp,NodeArray[iLevel]) && iLevel < INDEXTREE_MAX_LEVEL - 1)
		{
			if (searchCdr(pTemp->pNextLevel, Value, iLevel + 1))
			{
				return true;
			}
			else
			{
				continue;
			}
		}
		if (BlurCmpCdr(*pTemp, NodeArray[iLevel]) && iLevel == INDEXTREE_MAX_LEVEL - 1)
		{
			Value = *(pTemp->pRuleValue);
			strcpy(NodeArray[iLevel].szStartTime, pTemp->szStartTime); //this is for returning 
			strcpy(NodeArray[iLevel].szEndTime, pTemp->szEndTime);     //the time in rule
			return true;
		}
	}
	return false;
}
bool CIndexTree::getIndexNode(int index, SIndexNode &Node)
{
	if (index < 0 || index >= INDEXTREE_MAX_LEVEL)
	{
	  return false;
	}
	Node = NodeArray[index];
}

bool CIndexTree::clearTree()
{
	//added by tanj 20060326 check Root's value first 
	if (0 != Root)
	{
		clearNode(Root,0);
		Root = 0;
	}

	return true;
}	



const char *CIndexTree::ValueExp(const char *Context, Expression ** SyntaxTree, int &ErrorNo)
{
	const char *pPosition = Context;
	char Token[255] = "";
	char *pToken;
	map<string, const char *>::iterator it;
	
	//先吃掉前面的空白
	while ( (*pPosition != 0) && strchr(" \t\r\n", *pPosition) )
		++pPosition;
	
	switch(*pPosition)
	{

		//括号,定义表达式的优先级
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
			//函数表达式
			if (strncmp(pPosition, "not(", 4) == 0)
			{
				pExp = new NotExp();
				pPosition = pExp->SyntaxAnalyze(this, pPosition, ErrorNo);
			}
			//出现未预期的字符串
			else
			{
				pToken = Token;
				while ( (*pPosition != 0) && (strchr(" \t,()\r\n", *pPosition) == NULL))
					*pToken++ = *pPosition++;
				*pToken = 0;
				pExp = new VariableExp(Token);				
			}
			if (pExp != NULL)
			{
				//pPosition = pExp->SyntaxAnalyze(this, pPosition, ErrorNo);
				if (ErrorNo != COMPILE_NOERROR)
				{
					delete pExp;
					return pPosition;
				}
				*SyntaxTree = pExp;
			}
	}
	//去掉后面的空白符号
	while ( (*pPosition != 0) && strchr(" \t\r\n", *pPosition) )
		++pPosition;
	return pPosition;	
}

const char *CIndexTree::CompileExp(const char *Context, Expression ** SyntaxTree, int &ErrorNo)
{
	Expression *LeftExp = NULL;
	const char *pPosition = NULL;
	
	pPosition = ValueExp(Context, &LeftExp, ErrorNo);
	if (ErrorNo != COMPILE_NOERROR)
		goto error_exit;

	for (;;)
	{
		//and 表达式创建 token = " & ",&前后必须有空白字符
		if ( (strncmp(pPosition, "&", 1) == 0) &&
			( (*(pPosition - 1) == ' ') || (*(pPosition - 1) == '\t') ) &&
			( (*(pPosition + 1) == ' ') || (*(pPosition + 1) == '\t') ) 
				)
		{
			AndExp *TempExp = new AndExp();
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
			//左递归
			LeftExp = TempExp;
		}
		//or 表达式创建 token = " | ",|前后必须有空白字符
		else if ( (strncmp(pPosition, "|", 1) == 0) &&
				( (*(pPosition - 1) == ' ') || (*(pPosition - 1) == '\t') ) &&
				( (*(pPosition + 1) == ' ') || (*(pPosition + 1) == '\t') ) 
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
		//语法分析正常(注:需要在分析完成后检查是否有剩余的上下文)
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


CIndexTree::NotExp::~NotExp()
{
	delete m_BoolExp;
}
const char *CIndexTree::NotExp::Execute(const SIndexNode &CdrNode)
{
	return strcmp(m_BoolExp->Execute(CdrNode), RETN_TRUE) == 0? RETN_FALSE:RETN_TRUE;
}

const char *CIndexTree::NotExp::SyntaxAnalyze(CIndexTree *pInterpreter, const char *Context, int &ErrorNo)
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
CIndexTree::AndExp::~AndExp()
{
	delete m_LeftCondition;
	delete m_RightCondition;
}

const char * CIndexTree::AndExp::Execute(const SIndexNode &CdrNode)
{
	// is "false" ?
	if (strcmp(m_RightCondition->Execute(CdrNode), RETN_FALSE) == 0)
	{
		return RETN_FALSE;
	}
	// is "false" ?
	if (strcmp(m_LeftCondition->Execute(CdrNode), RETN_FALSE) == 0)
	{
		return RETN_FALSE;
	}
	// return "true"
	return RETN_TRUE;
}

//
CIndexTree::OrExp::~OrExp()
{
	delete m_LeftCondition;
	delete m_RightCondition;
}

const char * CIndexTree::OrExp::Execute(const SIndexNode &CdrNode)
{
	// is "true" ?
	if (strcmp(m_RightCondition->Execute(CdrNode), RETN_TRUE) == 0)
	{
		return RETN_TRUE;
	}
	// is "true" ?
	if (strcmp(m_LeftCondition->Execute(CdrNode), RETN_TRUE) == 0)
	{
		return RETN_TRUE;
	}
	// return "false"
	return RETN_FALSE;
}


//第一次编译表达式时会强制修改表达式的前2个字节
//第1个字节是已编译标志0xEE,第2个字节是索引号
//建议在配规则时每条规则前面预留2个tab字符
//主要是为了能迅速定位一个表达式的语法树
const char *CIndexTree::Operation(char *Result, int Length, int *ErrorNo, const char *Context, const SIndexNode &CdrNode)
{
	*ErrorNo = COMPILE_NOERROR;
	
	if ( *(unsigned char *)Context == 0xEE)
	{
		//try { 运行时错误编号 COMPILE_RUNTIME_
		//需由Expression对象抛出异常
		try
		{
						//取表达式的索引编号
						//modify by weixy 20080606 
			return strncpy(Result, m_SyntaxTree[ (int)*(Context + 1) > 0 ? (int)*(Context + 1): 256+((int)*(Context + 1))  ]->Execute(CdrNode), Length);

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
		//表达式超过最大限制
		//(索引编号使用一个字节存储1~255,由于0是字符串结束标志未使用0)
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
		//如还有剩余的字符串则说明表达式不符文法规则
		else if (*pPosition != 0)
		{
			*ErrorNo = COMPILE_ERROR_INVALID_EXP;
			delete pTempExp;
			return pPosition;
		}

		//对表达式加索引,格式为:0xEE 索引编号,从1~255
		//注:修改Context的第1个字节为0xEE(标志),第2个字节为索引号
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
		return Operation(Result, Length, ErrorNo, Context,CdrNode);
	}
}
