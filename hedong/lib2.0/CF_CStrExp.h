//CF_CStrExp.h
//by tanj 20060207
//�����Զ���ı��ʽ�������ַ�����ȡ��Ӧ���ַ����ָ��������ַ���

#ifndef _CF_CSTREXP_
#define _CF_CSTREXP_

#include <stdlib.h>
#include "Common.h"

//�ַ�����
enum
{
	CHARTYPE_END,           //������־
	CHARTYPE_CONSTANT,      //���ַ�ʱ����
	CHARTYPE_VARIABLE       //���ַ�ʱ���������������ַ���
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

