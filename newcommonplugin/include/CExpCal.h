 /****************************************************************

 filename: CExpCal.h

 module: SC;

 created by:	daiw

 create date:	2014-06-13

 version: 3.0.0

 description: 

	M(SC2iQ/

 update:



 *****************************************************************/



#ifndef _CEXPCAL_H_

#define _CEXPCAL_H_  1

 

#include <sys/time.h>

#include "es/util/StringUtil.h"

#include "CF_CPlugin.h"

#include "CF_CException.h"

#include "CF_Common.h"

#include "ComFunction.h"

#include "CF_Lack_Abn_Code.h"

#include "CF_PREP_Error.h"

#include "CommonPluginFactory.h"

class SFieldValue
{
public:
	int iFlag;
	string sField;
};


template <class Type>       
class STACK{                 //定义栈类
private:
	Type base[MSG_LEN];
	int Size;
public:
	STACK(){Size=0;};
	void push(Type a)     //入栈
	{
		base[Size]=a;
		Size++;
	}
	Type pop()            //出栈
	{
		return base[--Size];
	}
	int size()
	{return Size;}
};



class Calculate_Cla
{
public:
	Calculate_Cla()
	{}

	~Calculate_Cla()
	{}
	
	bool IsData(char);
	
	bool IsSym(char);
	
	int IsPar(char);
	
	bool Check(char *);
	
	int setPri(char);                 //判断符号的优先极别
	
	double ToData(char*);               //把字符串转化为数值
	
	double Call(double data,double sum,char ch);    //具体按符号计算
	
	int GetMatch(char* buffer,int pos); //利用栈找到匹配的括号
	
	void Opr( STACK<char>&, STACK<double>&, int& ); //利用栈计算
	
	double Calculate(char*, double& );   //字符串的读入及调配

};



//参数解析类
class CParaParser
{

public:
	CParaParser();

	~CParaParser();

	bool parseFormular(char* exp);

	bool isVariable(int& pos,string& svalue);

	bool isFieldNumber(int& pos,string& svalue);

	bool isSymbol(int& pos,string& svalue);

	void Space(int& pos);

	vector<SFieldValue> getParseVec();

	string getParseExpress(PacketParser& pps);

	string transform(int n,int imethod,double dres);

	enum ExpType
	{
		NUMB = 0,//数字
		VARI = 1,//变量
		SYMB = 2,//运算符
	};

private:

	int _index;
	
	string _expr,_target;

	vector<SFieldValue> _vStack;



};


/**
  表达式运算插件ExpCal(express)
  -----------------------------------------------------------------------
  表达式作为输入参数
  表达式可以由$符号的参数组成
  运算插件只支持四则运算,不支持位运算,支持小数,负数的运算,需要用括号将负数括起。
  输入参数的表达式不能以$符号开始(是否修改待定)
  -----------------------------------------------------------------------
**/
class CExpCal: public BasePlugin

{

public:	

	CExpCal();

	void init(char *szSourceGroupID, char *szServiceID, int index);

	void execute(PacketParser& pps,ResParser& retValue);

	void message(MessageParser&  pMessage);

	std::string getPluginName();

	std::string getPluginVersion();

	void printMe();

	~CExpCal();


private:

	int dealMethod;

	CParaParser _parser;

	bool bHavParse;//是否已经解析标识

	vector<SFieldValue> _vParseField;

	string _needParseOperat,_result;

	char _cbuf[FIELD_LEN];
	char _cpos[FIELD_LEN];
	char _cmethod[FIELD_LEN];
	char _cfrontfield[FIELD_LEN];

	Calculate_Cla _cl;//四则运算表达式解析计算类

};


#endif



