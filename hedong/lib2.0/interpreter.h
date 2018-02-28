/******�ķ�����*********************************************************************************************************
CompileExp ::= ValueExp { BoolOp ValueExp }
BoolOp ::= and|or 
ValueExp ::= (CompileExp)|FunExp|literal|Variable
FunExp ::= set()|like()|case()|in()|substring()|connect()|if()|strabove()|strbelow()|numabove()|
			numbelow()|not|isdatetime
literal ::= '[0-9]|[a-z]|[A-Z]*'|number
Variable ::= $VariableName
(literal�ɴ����ŵĿɼ��ַ��Ͳ������ŵ��������)
************************************************************************************************************************/
/************************************************************************************************************************
********Module :��̬����
********Version:    2.0
********Description:���������ķ�����ľ���
********History:    	
	guol  Version 1.0  2001/09/25 ��̬����
	liusk Version 2.0  2003/09/23 ��Version 1.0��д,�������﷨�����Լ������﷨���Ĺ���

revisional history:
	liusk 2003/10/23 add length, decode ���� ver 2.02
	liusk 2003/11/8  add comma ����  ver 2.03
	wulf  2005/10/17 add list and sum ���� ver 2.04	
	wulf 2007/01/25	��++m_iObjCount;��PluginExp��Ĺ��캯���ƶ���void Interpreter::PluginExp::Init()
	weixy  2008/06/06 �޸�m_SyntaxTree��ַԽ�����⣬��(unsigned int)*�ĳ�(int)*  ��127��չ��255
	weixy  2008/12/01 ���ӱ��ʽdifftime������ͬMinusTimeExp��ֻ�������������
************************************************************************************************************************/


#ifndef _COMPILE_INTERPRETER_
#define _COMPILE_INTERPRETER_

#include "config.h"

//add by wulf 2005/09/15
//listΪ�����б�ֵ���������Ҫ�������ݿ�
#include "COracleDB.h"
//add by wulf 2005/11/28
//plugin���ʽ�е��ò��
#include "PluginLoader.h"
#include "errcode.h"
#include "CF_CError.h"
#include "Common.h"


#define EXPERLENGTH 8192


#define COMPILE_NOERROR				0	/*�����־λΪ�棬��û�д���*/
#define COMPILE_ERROR_MISSING_RIGHT_BRACKET	101	/*�����������Ų�ƥ��*/	
#define COMPILE_ERROR_NOT_EXPECTED_CHARACTER	102	/*���������ֵķ���*/	
#define COMPILE_ERROR_AND_PARAM			103	/*'&'�����������*/
#define COMPILE_ERROR_OR_PARAM			104	/*'|'�����������*/
#define COMPILE_ERROR_MISSING_COMMA		105	/*©��','����*/
#define COMPILE_ERROR_INVALID_PARAM		106	/*��Ч�Ĳ���*/
#define COMPILE_ERROR_VARIABLE_NOT_DEFINE	107	/*δ���������*/
#define COMPILE_ERROR_ZERO_DIVID		108	/*0������*/
#define COMPILE_ERROR_FUNCTION_NOT_DEFINE	109	/*δ����ĺ�����*/
#define COMPILE_ERROR_MISSING_SINGLE_QUOTATION_MARK 110	/*ȱ���ҵ�����*/
#define COMPILE_ERROR_INVALID_NUMBER	111		/*��Ч������,���溬����ĸ*/
#define COMPILE_ERROR_INVALID_EXP	112		/*��Ч���ӱ��ʽ���߱��ʽΪ��*/
#define COMPILE_ERROR_CASEEXP_MISSING_SUBEXP	113	/*case�������ʽȱ���ӱ��ʽ*/
#define COMPILE_ERROR_INEXP_MISSING_SUBEXP	114	/*in�������ʽȱ���ӱ��ʽ*/
#define COMPILE_ERROR_SETEXP_FIRSTEXP_NOT_VAR	115	/*set�����е�һ�����ʽ���Ǳ���*/
#define COMPILE_ERROR_DECODEEXP_MISSING_SUBEXP	116	/*decode������ȱ���ӱ��ʽ*/
#define COMPILE_ERROR_COMMAEXP_MISSING_SUBEXP	117	/*comma������ȱ���ӱ��ʽ*/
/* added by wulf 2005/10/17 */
#define COMPILE_ERROR_SUMEXP_MISSING_SUBEXP	118	/*sum������ȱ���ӱ��ʽ*/
#define COMPILE_ERROR_LISTEXP_MISSING_SUBEXP	119	/*list������ȱ���ӱ��ʽ*/
/* end of 2005/10/17 */

/* add by wulf 2005/11/28 */
#define COMPILE_ERROR_PLUGINEXP_MISSING_SUBEXP	122	/*plugin������ȱ���ӱ��ʽ*/
#define COMPILE_ERROR_LIKEINEXP_MISSING_SUBEXP  123 /*headin������ȱ���ӱ��ʽ*/
/* end of 2005/11/28 */

#define COMPILE_RUNTIME_ERROR_UNKNOWN	120		/*��̬��������ʱ����*/
#define COMPILE_RUNTIME_ERROR_EXPOVERLIMIT	121	/*���ʽ��Ŀ����255*/

#define COMPILE_RUNTIME_ERROR_INIT_PLUGIN	124	/*��ʼ���������*/
#define COMPILE_RUNTIME_ERROR_ACCESS_DB	125   /*�������ݿ����*/

const char* const RET_TRUE = "true";
const char* const RET_FALSE = "false";
const int MAX_FIELD_LEN = 100;	

/* added by wulf 2005/10/17 */
struct SField
{
	char m_szValue[50];
	SField()
	{
		memset( m_szValue, 0, sizeof( m_szValue ) );	
	}			
};

struct CListRow
{
	SField *m_pField;
	int m_iFieldNum;
	CListRow()
	{
		m_pField = NULL;	
		m_iFieldNum = 0;
	}
};

bool CompareRow( const CListRow &Src, const CListRow &Dest );
/* end of 2005/10/17 */

class Interpreter
{
public:
	//add by wulf at 20060613
	void setPipe(const char* szPipeId,const char* szVarname="EXP_PLUGIN"){ strcpy( m_szPipeId, szPipeId);strcpy(m_szVarname,szVarname); }
	//add by yangh 20060824
	Interpreter() { m_SyntaxTree.push_back(NULL); }
	~Interpreter();
	
	//add by weixy 20080606
	int getSyntaxTreeSize(){return m_SyntaxTree.size();};
	//end add by weixy 20080606
	//add by weixy 20080707
	int clear();
	//end add by weixy 20080707
	bool DefineVariable(const char *VariableName, const char * VariableAddress);
	
	const char *Operation(char *Result, int Length, int *Error, const char *Context);
	
	void DumpVar(void) {}
	//˽����
private:
	//add by wulf at 20060613
	static char m_szPipeId[100];
	static char m_szVarname[100];//Ĭ�ϵı��ʽ������ΪEXP_PLUGIN,���ǿ����Զ���
	//���������ֱ��ʽ����
	class Expression
	{
	public:
		Expression() {}
		virtual ~Expression() {}
		virtual const char *Execute() = 0;
		virtual const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo) = 0;
		virtual const char *ExpType() { return (const char *)"Expression"; }
	};
	
	//�ս�����ʽ,Ҷ�ӽڵ㲻��Ҫ�ڽ����﷨����
	//�﷨���е�Ҷ�ӽ��,һЩ�����ַ�������'11','12','011'��
	class LiteralExp: public Expression
	{
	public:
		explicit LiteralExp(const char *Literal):m_Literal(Literal) {}
		~LiteralExp() {}
		const char *Execute() { return m_Literal.c_str(); }
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo) { return NULL; }
		const char *ExpType() { return (const char *)"LiteralExp"; }
	private:
		string m_Literal;
	};
	
	//���ʽ�г��ֵı���,�﷨���е�Ҷ�ӽ��
	class VariableExp: public Expression
	{
	public:
		explicit VariableExp(const char *pVarAddress):m_pVarAddress(pVarAddress) {}
		~VariableExp() {}
		const char *Execute() { return m_pVarAddress; }
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo) { return NULL; }
		const char *ExpType() { return (const char *)"VariableExp"; }
	private:
		const char *m_pVarAddress;
	};
	
	//�������ʽ
	
	// like(exp1,exp2)
	class LikeExp: public Expression
	{
	public:
		LikeExp() :m_LeftExp(0),m_RightExp(0) {}
		~LikeExp();
		const char *Execute();
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo);
		const char *ExpType() { return (const char *)"LikeExp"; }
	private:
		bool Compare(const char *CompareString, const char *CompareMod);
		Expression *m_LeftExp;
		Expression *m_RightExp;
	};
	friend class LikeExp;
	
	//not(exp)
	class NotExp: public Expression
	{
	public:
		NotExp():m_BoolExp(0) {}
		~NotExp();
		const char *Execute();
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo);
		const char *ExpType() { return (const char *)"NotExp"; }
	private:
		Expression *m_BoolExp;
	};
	friend class NotExp;
	
	//if(exp, true_return, false_return)
	class IfExp: public Expression
	{
	public:
		IfExp():m_BoolExp(0),m_True_RetExp(0),m_False_RetExp(0) {}
		~IfExp();
		const char *Execute();
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo);
		const char *ExpType() { return (const char *)"IfExp"; }
	private:
		Expression *m_BoolExp;
		Expression *m_True_RetExp;
		Expression *m_False_RetExp;
	};
	friend class IfExp;
	
	//case(condition1,result1,condition2,result2,....,default_result)
	class CaseExp: public Expression
	{
	public:
		CaseExp() {}
		~CaseExp();
		const char * Execute();
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo);
		const char *ExpType() { return (const char *)"CaseExp"; }
		
	private:
		vector<Expression *> m_ExpArray;
	};
	friend class CaseExp;
	
	// bool_exp1 and bool_exp2
	class AndExp: public Expression
	{
	public:
		AndExp():m_LeftCondition(0),m_RightCondition(0) {};
		~AndExp();
		const char * Execute();
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo) { return NULL; }
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
		const char * Execute();
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo) { return NULL; }
		const char *ExpType() { return (const char *)"OrExp"; }
		
		void AddLeftExp(Expression *LeftExp) { m_LeftCondition = LeftExp; }
		void AddRightExp(Expression *RightExp) { m_RightCondition = RightExp; }
	private:
		Expression *m_LeftCondition;
		Expression *m_RightCondition;
	};
	
	// in(value,test_value1,test_value2,....)
	class InExp: public Expression
	{
	public:
		InExp() {};
		~InExp();
		const char * Execute();
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo);
		const char *ExpType() { return (const char *)"InExp"; }
	private:
		vector<Expression *> m_ExpArray;
	};
	friend class InExp;
	
	/*
	* added by wulf 2005/10/17
	* list(value1,value2,value3,...,"sql_sentance")
	* ƥ���б�ֵ���ʽ���ж�(value1,value2,value3,...)�Ƿ�����е�ĳ����¼ƥ��
	* ������ƥ����������Ӧ��ֵ
	*/
	class ListExp: public Expression
	{
	public:
		explicit ListExp();
		~ListExp();
		const char *Execute();
		void Init();		
		const char *SyntaxAnalyze( Interpreter *pInterpreter, const char *Context, int &ErrorNo );
		const char *ExpType() 
		{ 
			return (const char *)"ListExp";
		}
	private:		
		vector<Expression *> m_ExpArray;
		vector<CListRow> m_RowArray;
		char m_szSqlContent[512];
		//add by wulf at 20051219
		char m_cTabDriveUpdt;
		char m_szSqlId[512];
		char m_szUpdateFlag[512];
		int m_iParamNum;
		int m_iInitFlag;
		int m_iRowNum;
		CListRow m_CurRow;
		
	};
	friend class ListExp;
	/*
	 * sum(value1,value2,value3,...)
	 * value1, value2, value3,...ת��Ϊ������Ȼ�����
	 * ���ַ����ķ�ʽ������Ӻ�Ľ��
	 */
	class SumExp: public Expression
	{
	public:
		SumExp() {};
		~SumExp();
		const char * Execute();
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo);
		const char *ExpType() { return (const char *)"SumExp"; }
	private:
		vector<Expression *> m_ExpArray;
		long m_lSum;
		char m_szSum[50];
		long m_lCurValue;
	};
	friend class SumExp;
	/* end of 2005/10/17 */
	
	/*
	 * add by wulf at 2006/02/15
	 * adddays(Days,OrgDate)	 
	 * DaysΪ�����ӵ�������OrgDateΪԭ�������ڣ�8λ��
	 * ���ַ����ķ�ʽ������Ӻ�����ڣ�8λ��
	 */
	class AddDaysExp: public Expression
	{
	public:
		AddDaysExp():m_ExpDays(0),m_ExpOrgDate(0) {};
		~AddDaysExp();
		const char * Execute();
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo);
		const char *ExpType() { return (const char *)"AddDaysExp"; }
	private:
		Expression *m_ExpDays;
		Expression *m_ExpOrgDate;				
	};
	friend class AddDaysExp;
	
	/*
	 * add by wulf at 2006/02/15
	 * minusdays(FirstDay,SecondDay)	 
	 * FirstDayΪ��һ�����ڣ�SecondDayΪ�ڶ������ڣ�8λ��   
	 * ���ַ����ķ�ʽ������������
	 */
	class MinusDaysExp: public Expression
	{
	public:
		MinusDaysExp():m_ExpFirstDay(0),m_ExpSecondDay(0) {};
		~MinusDaysExp();
		const char * Execute();
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo);
		const char *ExpType() { return (const char *)"MinusDaysExp"; }
	private:
		Expression *m_ExpFirstDay;
		Expression *m_ExpSecondDay;				
	};
	friend class MinusDaysExp;
	
		/*
	 * add by wulf at 2006/02/15
	 * addtime(seconds,OrgTime)	 
	 * secondsΪ�����ӵ�������OrgTimeΪԭ����ʱ�䣨14λ��
	 * ���ַ����ķ�ʽ������Ӻ��ʱ�䣨14λ��
	 */
	class AddTimeExp: public Expression
	{
	public:
		AddTimeExp():m_ExpSeconds(0),m_ExpOrgTime(0) {};
		~AddTimeExp();
		const char * Execute();
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo);
		const char *ExpType() { return (const char *)"AddTimeExp"; }
	private:
		Expression *m_ExpSeconds;
		Expression *m_ExpOrgTime;				
	};
	friend class AddTimeExp;
	
		/*
	 * add by wulf at 2006/02/15
	 * minustime(FirstTime,SecondTime)	 
	 * FirstTimeΪ��һ��ʱ�䣬SecondTimeΪ�ڶ���ʱ�䣨14λ��   
	 * ���ַ����ķ�ʽ������������
	 */
	class MinusTimeExp: public Expression
	{
	public:
		MinusTimeExp():m_ExpFirstTime(0),m_ExpSecondTime(0) {};
		~MinusTimeExp();
		const char * Execute();
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo);
		const char *ExpType() { return (const char *)"MinusTimeExp"; }
	private:
		Expression *m_ExpFirstTime;
		Expression *m_ExpSecondTime;				
	};
	friend class MinusTimeExp;
	/* end of 2006/02/15 */
	
			/*
	 * add by weixy at 2008/12/01
	 * minustime(FirstTime,SecondTime)	 
	 * FirstTimeΪ��һ��ʱ�䣬SecondTimeΪ�ڶ���ʱ�䣨14λ��   
	 * ���ַ����ķ�ʽ�����������������������
	 */
	class DiffTimeExp: public Expression
	{
	public:
		DiffTimeExp():m_ExpFirstTime(0),m_ExpSecondTime(0) {};
		~DiffTimeExp();
		const char * Execute();
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo);
		const char *ExpType() { return (const char *)"DiffTimeExp"; }
	private:
		Expression *m_ExpFirstTime;
		Expression *m_ExpSecondTime;				
	};
	friend class DiffTimeExp;
	/* end of 2008/12/01 */
	
		
	//substring(string,{-} pos {,length}) posΪ��ʱ�����Ҵ�1��ʼ,posΪ��ʱ���������-1��ʼ
	//�������length��ʾȡ���ַ���ĩβ
	class SubStringExp: public Expression
	{
	public:
		SubStringExp():m_StringExp(0),m_PosExp(0),m_LenExp(0) {}
		~SubStringExp();
		const char * Execute();
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo);
		const char *ExpType() { return (const char *)"SubStringExp"; }
	private:
		Expression *m_StringExp;
		Expression *m_PosExp;
		Expression *m_LenExp;
		//��Ž��
		char m_Buffer[255];
	};
	friend class SubStringExp;
	
	//connect(string1, string2)
	class ConnectExp: public Expression
	{
	public:
		ConnectExp():m_FirstStrExp(0),m_SecondStrExp(0) {}
		~ConnectExp();
		const char * Execute();
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo);
		const char *ExpType() { return (const char *)"ConnectExp"; }
	private:
		Expression *m_FirstStrExp;
		Expression *m_SecondStrExp;
		//��Ž��
		char m_Buffer[255];
	};
	friend class ConnectExp;
	
	//set($VarExp, string)
	class SetExp: public Expression
	{
	public:
		SetExp():m_VariableExp(0),m_StringExp(0) {}
		~SetExp();
		const char * Execute();
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo);
		const char *ExpType() { return (const char *)"SetExp"; }
	private:
		Expression *m_VariableExp;
		Expression *m_StringExp;
	};
	friend class SetExp;
	
	//strabove(string1,string2)|strbelow(string1,string2)
	class StrCmpExp: public Expression
	{
	public:
		explicit StrCmpExp(bool bIsStrAbove):m_StringExp1(0),m_StringExp2(0),m_bIsStrAbove(bIsStrAbove) {}
		~StrCmpExp();
		const char * Execute();
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo);
		const char *ExpType() { return m_bIsStrAbove? (const char *)"StrAbove":(const char *)"StrBelow"; }
	private:
		Expression *m_StringExp1;
		Expression *m_StringExp2;
		bool m_bIsStrAbove;
	};
	friend class StrCmpExp;

	//numabove(numstring1,numstring2)|numbelow(numstring1,numstring2)
	class NumCmpExp: public Expression
	{
	public:
		explicit NumCmpExp(bool bNumStrAbove):m_NumStringExp1(0),m_NumStringExp2(0),m_bIsNumAbove(bNumStrAbove) {}
		~NumCmpExp();
		const char * Execute();
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo);
		const char *ExpType() { return m_bIsNumAbove? (const char *)"NumAbove":(const char *)"NumBelow"; }
	private:
		Expression *m_NumStringExp1;
		Expression *m_NumStringExp2;
		bool m_bIsNumAbove;
	};
	friend class NumCmpExp;
	
	//isdatetime(string)
	class IsDateTimeExp: public Expression
	{
	public:
		explicit IsDateTimeExp(): m_StringExp(0) {}
		~IsDateTimeExp();
		const char * Execute();
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo);
		const char *ExpType() { return (const char *)"IsDateTimeExp"; }
	private:
		Expression *m_StringExp;
	};
	friend class IsDateTimeExp;
	
	//add by liusk 2003/10/23
	//�����ַ�������
	//length(string)
	class LengthExp: public Expression
	{
	public:
		explicit LengthExp(): m_StringExp(0) {}
		~LengthExp();
		const char * Execute() ;
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo);
		const char *ExpType() { return (const char *)"LengthExp"; }
	private:
		char m_Length[30];
		Expression *m_StringExp;
	};
	friend class LengthExp;

	//add by liusk 2003/10/23
	//���value������(serarch_n)�б��е��κ�һ������򷵻���Ӧ��result,���򷵻�Ĭ��ֵdefault_result
	//��case�����Ĳ�ͬΪ,caseΪ�������������򷵻�,decodeΪ�Ƚϲ�����һϵ�е�ֵ�Ƚ���������򷵻ض�Ӧ�Ľ��
	//decode(value_exp,search1,result1,search2,result2,..,default_result)
	class DecodeExp: public Expression
	{
	public:
		explicit DecodeExp() {}
		~DecodeExp();
		const char * Execute() ;
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo);
		const char *ExpType() { return (const char *)"DecodeExp"; }
	private:
		vector<Expression *> m_ExpArray;
	};
	friend class DecodeExp;
	
	//add by liusk 2003/11/8
	//comma(exp1,exp2,exp3,....,expn)
	//���ű��ʽ,ִ�����еı��ʽ���,���������һ�����ʽ��ֵ
	//����һЩ���ʽ������
	class CommaExp: public Expression
	{
	public:
		explicit CommaExp() {}
		~CommaExp();
		const char * Execute() ;
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo);
		const char *ExpType() { return (const char *)"CommaExp"; }
	private:
		vector<Expression *> m_ExpArray;
	};
	friend class CommaExp;
	
 /* add by wulf 2005/11/28
	* plugin(�������,value1,value2,...)
	* �����ڱ��ʽ��ʹ�ò��
	*/
	class PluginExp: public Expression
	{
	public:
		explicit PluginExp();		
		~PluginExp();
		const char *Execute();
		void Init();		
		const char *SyntaxAnalyze( Interpreter *pInterpreter, const char *Context, int &ErrorNo );
		const char *ExpType() 
		{ 
			return (const char *)"PluginExp";
		}
	private:		
		vector<Expression *> m_ExpArray;
		PacketParser m_PacketParser;
		static int m_iObjCount;
		static PluginProxy m_PluginProxy;
		char m_szFuncName[100];
		BasePlugin* m_pPluginObj;
		char m_szSlFileName[1000];
		//modify by wulf at 20060224
//		char m_AFieldValue[ATTRINUMBER][MAX_FIELD_LEN + 1];	
		char* m_pAFieldValue[ATTRINUMBER];	
		int m_iInitFlag;		
	};
	friend class PluginExp;
	
 /* add by wulf 2002/11/29
	* likein(value,test_value1,test_value2,....)
	* �����жϺ���ͷ���ַ���ͷ��
	*/
	class LikeInExp: public Expression
	{
	public:
		LikeInExp() {};
		~LikeInExp();
		const char * Execute();
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo);
		const char *ExpType() { return (const char *)"LikeInExp"; }
	private:
		vector<Expression *> m_ExpArray;
	};
	friend class LikeInExp; 
	
/* add by wulf 2005/11/29 
 * between(value,left,right)
 * �ж�value�Ƿ���[left, right]�ķ�Χ֮��
 */
 class BetweenExp: public Expression
	{
	public:
		explicit BetweenExp():m_pValue(0),m_pLeft(0),m_pRight(0){}
		~BetweenExp();
		const char * Execute();
		const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo);
		const char *ExpType() { return (const char *)"BetweenExp"; }
	private:
		Expression *m_pValue;
		Expression *m_pLeft;
		Expression *m_pRight;
	};
	friend class BetweenExp;
	
private:
	//�����б�
	map<string, const char *> m_VaribleList;
	
	//����0δʹ��
	vector<Expression *> m_SyntaxTree;
	
	//�﷨��������
	const char * CompileExp(const char *Context, Expression **SyntaxTree, int &ErrorNo);
	const char *ValueExp(const char *Context, Expression **SyntaxTree, int &ErrorNo);

};

typedef Interpreter C_Compile;

#endif
