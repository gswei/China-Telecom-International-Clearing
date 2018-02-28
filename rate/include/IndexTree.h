/*******************************************************************************
*IndexTree.h
*this program is designed for blur(ambigulous) rule search in rate module of 
*zhjs system.
*created by tanj 2005.4.15
*******************************************************************************/
#ifndef _INDEXTREE_H_
#define _INDEXTREE_H_


#include "RuleStruct.h"

#define COMPILE_NOERROR				0	/*错误标志位为真，即没有错误*/
const char* const RETN_TRUE = "true";
const char* const RETN_FALSE = "false";


//enum ENodeType
//{AMB, EXA, CDR};
struct SIndexNode
{
	char szField[RATE_MAX_RULEITEM_LEN];
	char szStartTime[15];
	char szEndTime[15];
	char szCdrTime[15];
	SIndexNode *pNextBrother;
	SIndexNode *pNextLevel;
	SRuleValue *pRuleValue;
	int iMatchMode;
  SIndexNode()
  {
  
  	memset(szField, 0, sizeof(szField));
  	memset(szStartTime, 0, sizeof(szStartTime));
  	memset(szEndTime, 0, sizeof(szEndTime));
  	memset(szCdrTime, 0, sizeof(szCdrTime));
  	//NodeType = AMB;
  	pNextBrother = NULL;
  	pNextLevel = NULL;
  	pRuleValue = NULL;
  	iMatchMode = 1;
  }
  SIndexNode(const SIndexNode &_Node)
  {
  	strcpy(szField, _Node.szField);
  	strcpy(szStartTime, _Node.szStartTime);
  	strcpy(szEndTime, _Node.szEndTime);
  	strcpy(szCdrTime, _Node.szCdrTime);
  	pNextBrother = NULL;
  	pNextLevel = NULL;
  	pRuleValue = NULL;
  	iMatchMode = _Node.iMatchMode;
  }
  SIndexNode(char *field, char *starttime, char *endtime, char *cdrtime, SIndexNode *bro, SIndexNode *nextLevel, SRuleValue *value)
  {
  	strcpy(szField, field);
  	strcpy(szStartTime, starttime);
  	strcpy(szEndTime, endtime);
  	strcpy(szCdrTime, cdrtime);
  	pNextBrother = bro;
  	pNextLevel = nextLevel;
  	pRuleValue = value;
  	iMatchMode = 1;
  }
};



class CIndexTree
{
private:
  SIndexNode *Root;
  SIndexNode *NodeArray;
  //简易表达式，added by liuw at 20080721
  
  	//解释器各种表达式基类
	class Expression
	{
	public:
		Expression() {}
		virtual ~Expression() {}
		virtual const char *Execute(const SIndexNode &CdrNode) = 0;
		virtual const char *SyntaxAnalyze(CIndexTree *pInterpreter, const char *Context, int &ErrorNo) = 0;
		virtual const char *ExpType() { return (const char *)"Expression"; }
	};
	//表达式中出现的变量,语法树中的叶子结点
	class VariableExp: public Expression
	{
	public:
		explicit VariableExp(const char *pVarAddress):m_pVarAddress(pVarAddress) {}
		~VariableExp() {}
		const char *Execute(const SIndexNode &CdrNode) {			
			if (strlen(m_pVarAddress.c_str()) == 0)
			{
				return RETN_TRUE;
			}
			if (strncmp(m_pVarAddress.c_str(), "~", 1) == 0  && 
			    strncmp(m_pVarAddress.substr(1,m_pVarAddress.size()-1).c_str(), CdrNode.szField, m_pVarAddress.size() - 1) != 0 )
			{
				return RETN_TRUE;
			}
			if (strncmp(m_pVarAddress.c_str(), "~", 1) != 0  && 
			    strncmp(m_pVarAddress.c_str(), CdrNode.szField, m_pVarAddress.size()) == 0 )
			{
				return RETN_TRUE;
			}
			return RETN_FALSE; }
		const char *SyntaxAnalyze(CIndexTree *pInterpreter, const char *Context, int &ErrorNo) { return NULL; }
		const char *ExpType() { return (const char *)"VariableExp"; }
	private:
		string m_pVarAddress;
	};
		//not(exp)
	class NotExp: public Expression
	{
	public:
		NotExp():m_BoolExp(0) {}
		~NotExp();
		const char *Execute(const SIndexNode &CdrNode);
		const char *SyntaxAnalyze(CIndexTree *pInterpreter, const char *Context, int &ErrorNo);
		const char *ExpType() { return (const char *)"NotExp"; }
	private:
		Expression *m_BoolExp;
	};
	friend class NotExp;
	
	// bool_exp1 and bool_exp2
	class AndExp: public Expression
	{
	public:
		AndExp():m_LeftCondition(0),m_RightCondition(0) {};
		~AndExp();
		const char * Execute(const SIndexNode &CdrNode);
		const char *SyntaxAnalyze(CIndexTree *pInterpreter, const char *Context, int &ErrorNo) { return NULL; }
		const char *ExpType() { return (const char *)"AndExp"; }
		
		void AddLeftExp(Expression *LeftExp) { m_LeftCondition = LeftExp; }
		void AddRightExp(Expression *RightExp) { m_RightCondition = RightExp; }
	private:
		Expression *m_LeftCondition;
		Expression *m_RightCondition;
	};
	
	// bool_exp1 or bool_exp2
	class OrExp: public Expression
	{
	public:
		OrExp():m_LeftCondition(0),m_RightCondition(0) {};
		~OrExp();
		const char * Execute(const SIndexNode &CdrNode);
		const char *SyntaxAnalyze(CIndexTree *pInterpreter, const char *Context, int &ErrorNo) { return NULL; }
		const char *ExpType() { return (const char *)"OrExp"; }
		
		void AddLeftExp(Expression *LeftExp) { m_LeftCondition = LeftExp; }
		void AddRightExp(Expression *RightExp) { m_RightCondition = RightExp; }
	private:
		Expression *m_LeftCondition;
		Expression *m_RightCondition;
	};
		//索引0未使用
	vector<Expression *> m_SyntaxTree;
	
	//end-20080721
	
  int INDEXTREE_MAX_LEVEL;          //索引树的高度
  bool insertNode(SIndexNode* &_pNode, const SRuleValue &Value_, int _iLevel); //the _iLevel begins with 0
  bool searchRule(SIndexNode* pNode, SRuleValue &Value, int iLevel);
  bool searchCdr(SIndexNode *pNode, SRuleValue &Value, int iLevel);
  void clearNode(SIndexNode *pNode, int iLevel);
  void travel(SIndexNode* pNode, int iLevel);
  bool BlurCmpBlur(const SIndexNode &First, const SIndexNode &Second);
  bool BlurCmpCdr(const SIndexNode &BlurNode, const SIndexNode &CdrNode);
  bool BlurCmpExact(const SIndexNode &BlurNode, const SIndexNode &ExactNode);  

  //表达式解析 added by liuw at 20080721
	const char *ValueExp(const char *Context, Expression ** SyntaxTree, int &ErrorNo);
	const char *CompileExp(const char *Context, Expression ** SyntaxTree, int &ErrorNo);
	const char *Operation(char *Result, int Length, int *ErrorNo, const char *Context, const SIndexNode &CdrNode);
	
public:
  CIndexTree(int iRuleItemCount);
  ~CIndexTree();
  bool insertRule(SIndexNode *_NodeArray, SRuleValue RuleValue);
  bool searchRule(SIndexNode *_NodeArray, SRuleValue &Value);
  bool searchCdr(SIndexNode *_NodeArray, SRuleValue &Value);
  void travel();
  bool clearTree();
  bool getIndexNode(int index, SIndexNode &Node);

};



#endif 
