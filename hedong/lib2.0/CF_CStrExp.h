//CF_CStrExp.h
//by tanj 20060207
//利用自定义的表达式从输入字符串中取相应的字符组成指定的输出字符串

#ifndef _CF_CSTREXP_
#define _CF_CSTREXP_

#include <stdlib.h>
#include "Common.h"

//字符类型
enum
{
	CHARTYPE_END,           //结束标志
	CHARTYPE_CONSTANT,      //该字符时常量
	CHARTYPE_VARIABLE       //该字符时变量，来自输入字符串
};
	

class CF_CStrExp
{
public:
	bool init(const char *szExp);
	bool execute(const char *szOrgStr, char *szRetStr);
	
private:
	char m_szExpression[256];
	char m_szConstant[256];
	int  m_iVariable[256];
	int  m_iCharType[256];
};

#endif 

