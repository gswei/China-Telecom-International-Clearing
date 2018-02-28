/******文法规则*********************************************************************************************************
CompileExp ::= ValueExp { BoolOp ValueExp }
BoolOp ::= and|or 
ValueExp ::= (CompileExp)|FunExp|literal|Variable
FunExp ::= set()|like()|case()|in()|substring()|connect()|if()|strabove()|strbelow()|numabove()|
			numbelow()|not|isdatetime
literal ::= '[0-9]|[a-z]|[A-Z]*'|number
Variable ::= $VariableName
(literal由带引号的可见字符和不带引号的数字组成)
************************************************************************************************************************/
/************************************************************************************************************************
********Module :动态编译
********Version:    2.0
********Description:解释如上文法定义的句子
********History:    	
	guol  Version 1.0  2001/09/25 动态编译
	liusk Version 2.0  2003/09/23 由Version 1.0改写,增加了语法分析以及建立语法树的功能

revisional history:
	liusk 2003/10/23 add length, decode 函数 ver 2.02
	liusk 2003/11/8  add comma 函数  ver 2.03
	wulf  2005/10/17 add list and sum 函数 ver 2.04	
	wulf 2007/01/25	把++m_iObjCount;从PluginExp类的构造函数移动到void Interpreter::PluginExp::Init()
	weixy  2008/06/06 修改m_SyntaxTree地址越界问题，把(unsigned int)*改成(int)*  将127扩展到255
	weixy  2008/12/01 增加表达式difftime，功能同MinusTimeExp，只将秒数变成天数
************************************************************************************************************************/


#ifndef _COMPILE_INTERPRETER_
#define _COMPILE_INTERPRETER_

#include "config.h"

//add by wulf 2005/09/15
//list为处理列表值的情况，需要访问数据库
#include "COracleDB.h"
//add by wulf 2005/11/28
//plugin表达式中调用插件
#include "PluginLoader.h"
#include "errcode.h"
#include "CF_CError.h"
#include "Common.h"


#define EXPERLENGTH 8192


#define COMPILE_NOERROR				0	/*错误标志位为真，即没有错误*/
#define COMPILE_ERROR_MISSING_RIGHT_BRACKET	101	/*函数左右括号不匹配*/	
#define COMPILE_ERROR_NOT_EXPECTED_CHARACTER	102	/*不期望出现的符号*/	
#define COMPILE_ERROR_AND_PARAM			103	/*'&'运算参数错误*/
#define COMPILE_ERROR_OR_PARAM			104	/*'|'运算参数错误*/
#define COMPILE_ERROR_MISSING_COMMA		105	/*漏掉','符号*/
#define COMPILE_ERROR_INVALID_PARAM		106	/*无效的参数*/
#define COMPILE_ERROR_VARIABLE_NOT_DEFINE	107	/*未定义变量名*/
#define COMPILE_ERROR_ZERO_DIVID		108	/*0做除数*/
#define COMPILE_ERROR_FUNCTION_NOT_DEFINE	109	/*未定义的函数名*/
#define COMPILE_ERROR_MISSING_SINGLE_QUOTATION_MARK 110	/*缺少右单引号*/
#define COMPILE_ERROR_INVALID_NUMBER	111		/*无效的数字,里面含有字母*/
#define COMPILE_ERROR_INVALID_EXP	112		/*无效的子表达式或者表达式为空*/
#define COMPILE_ERROR_CASEEXP_MISSING_SUBEXP	113	/*case函数表达式缺少子表达式*/
#define COMPILE_ERROR_INEXP_MISSING_SUBEXP	114	/*in函数表达式缺少子表达式*/
#define COMPILE_ERROR_SETEXP_FIRSTEXP_NOT_VAR	115	/*set函数中第一个表达式不是变量*/
#define COMPILE_ERROR_DECODEEXP_MISSING_SUBEXP	116	/*decode函数中缺少子表达式*/
#define COMPILE_ERROR_COMMAEXP_MISSING_SUBEXP	117	/*comma函数中缺少子表达式*/
/* added by wulf 2005/10/17 */
#define COMPILE_ERROR_SUMEXP_MISSING_SUBEXP	118	/*sum函数中缺少子表达式*/
#define COMPILE_ERROR_LISTEXP_MISSING_SUBEXP	119	/*list函数中缺少子表达式*/
/* end of 2005/10/17 */

/* add by wulf 2005/11/28 */
#define COMPILE_ERROR_PLUGINEXP_MISSING_SUBEXP	122	/*plugin函数中缺少子表达式*/
#define COMPILE_ERROR_LIKEINEXP_MISSING_SUBEXP  123 /*headin函数中缺少子表达式*/
/* end of 2005/11/28 */

#define COMPILE_RUNTIME_ERROR_UNKNOWN	120		/*动态编译运行时错误*/
#define COMPILE_RUNTIME_ERROR_EXPOVERLIMIT	121	/*表达式数目超过255*/

#define COMPILE_RUNTIME_ERROR_INIT_PLUGIN	124	/*初始化插件错误*/
#define COMPILE_RUNTIME_ERROR_ACCESS_DB	125   /*访问数据库出错*/

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
	//私有类
private:
	//add by wulf at 20060613
	static char m_szPipeId[100];
	static char m_szVarname[100];//默认的表达式变量名为EXP_PLUGIN,但是可以自定义
	//解释器各种表达式基类
	class Expression
	{
	public:
		Expression() {}
		virtual ~Expression() {}
		virtual const char *Execute() = 0;
		virtual const char *SyntaxAnalyze(Interpreter *pInterpreter, const char *Context, int &ErrorNo) = 0;
		virtual const char *ExpType() { return (const char *)"Expression"; }
	};
	
	//终结符表达式,叶子节点不需要在进行语法分析
	//语法树中的叶子结点,一些常量字符串，如'11','12','011'等
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
	
	//表达式中出现的变量,语法树中的叶子结点
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
	
	//函数表达式
	
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
	* 匹配列表值表达式，判断(value1,value2,value3,...)是否与表中的某条记录匹配
	* 并根据匹配结果返回相应的值
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
	 * value1, value2, value3,...转化为整数，然后相加
	 * 用字符串的方式返回相加后的结果
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
	 * Days为待增加的天数，OrgDate为原来的日期（8位）
	 * 用字符串的方式返回相加后的日期（8位）
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
	 * FirstDay为第一个日期，SecondDay为第二个日期（8位）   
	 * 用字符串的方式返回相差的天数
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
	 * seconds为待增加的秒数，OrgTime为原来的时间（14位）
	 * 用字符串的方式返回相加后的时间（14位）
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
	 * FirstTime为第一个时间，SecondTime为第二个时间（14位）   
	 * 用字符串的方式返回相差的秒数
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
	 * FirstTime为第一个时间，SecondTime为第二个时间（14位）   
	 * 用字符串的方式返回相差的秒数并折算成天数
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
	
		
	//substring(string,{-} pos {,length}) pos为正时从左到右从1开始,pos为负时从右往左从-1开始
	//如果忽略length表示取到字符串末尾
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
		//存放结果
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
		//存放结果
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
	//计算字符串长度
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
	//如果value和搜索(serarch_n)列表中的任何一个相等则返回相应的result,否则返回默认值default_result
	//与case函数的不同为,case为条件测试满足则返回,decode为比较测试与一系列的值比较如果满足则返回对应的结果
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
	//逗号表达式,执行所有的表达式语句,但返回最后一个表达式的值
	//可做一些表达式的容器
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
	* plugin(插件名字,value1,value2,...)
	* 用于在表达式中使用插件
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
	* 用于判断号码头或字符串头部
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
 * 判断value是否在[left, right]的范围之内
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
	//变量列表
	map<string, const char *> m_VaribleList;
	
	//索引0未使用
	vector<Expression *> m_SyntaxTree;
	
	//语法分析过程
	const char * CompileExp(const char *Context, Expression **SyntaxTree, int &ErrorNo);
	const char *ValueExp(const char *Context, Expression **SyntaxTree, int &ErrorNo);

};

typedef Interpreter C_Compile;

#endif
