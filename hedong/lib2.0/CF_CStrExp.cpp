//CF_CStrExp.cpp
//by tanj 20060207
//利用自定义的表达式从输入字符串中取相应的字符组成指定的输出字符串

#include "CF_CStrExp.h"

bool CF_CStrExp::init(const char *szExp)
{
	strcpy(m_szExpression,szExp);
	delSpace(m_szExpression, 0);
	if (m_szExpression[0] == NULL)
	{
		m_iCharType[0] = CHARTYPE_END;
		return true;
	}
	if (m_szExpression[0] != '#' && m_szExpression[0] != '$')
	{
		//表达式必须以#或$开头
		m_iCharType[0] = CHARTYPE_END;
		return false;
	}
	int iExpIte = 0;    //表达式中的位置
	int iCharIte = 0;   //结果字符串中的位置
	while(true)
	{
		if (m_szExpression[iExpIte] == '#')
		{
			iExpIte++;
			if (m_szExpression[iExpIte] == NULL)
			{
				//表示转义字符的\后面不能为空
				m_iCharType[0] = CHARTYPE_END;
				return false;
			}			
			m_iCharType[iCharIte] = CHARTYPE_CONSTANT;

			m_szConstant[iCharIte] = m_szExpression[iExpIte];
			iCharIte++;
			if (m_szExpression[iExpIte + 1] == NULL)
			{
				m_iCharType[iCharIte] = CHARTYPE_END;
				return true;
			}
			iExpIte++;			
			if (m_szExpression[iExpIte] != '#' && m_szExpression[iExpIte] != '$')
			{
				// #的后面只能紧跟一个字符
				m_iCharType[0] = CHARTYPE_END;
				return false;
			}

		}
		else if (m_szExpression[iExpIte] == '$')
		{
			iExpIte++;
			if (m_szExpression[iExpIte] == NULL)
			{
				//$后面不能为空
				m_iCharType[0] = CHARTYPE_END;
				return false;
			}		
			m_iCharType[iCharIte] = CHARTYPE_VARIABLE;
			char szTemp[10];
			int iTemp = 0;
			while (true)
			{
				if (m_szExpression[iExpIte] == NULL)
				{
					break;
				}
				if (m_szExpression[iExpIte] == '$' || m_szExpression[iExpIte] == '#')
				{
					break;
				}
				if (m_szExpression[iExpIte] > '9' && m_szExpression[iExpIte] < '0')
				{
					//$后面必须跟数字
					m_iCharType[0] = CHARTYPE_END;
					return false;
				}
				szTemp[iTemp] = m_szExpression[iExpIte];
				iTemp++;
			  iExpIte++;
			}
			szTemp[iTemp] = NULL;
			if (iTemp == 0)
			{
				//$后面不能紧跟$或\
				m_iCharType[0] = CHARTYPE_END;
				return false;
			}
			m_iVariable[iCharIte] = atol(szTemp);
			iCharIte++;
			if (m_szExpression[iExpIte] == NULL)
			{
				m_iCharType[iCharIte] = CHARTYPE_END;
				return true;
			}
		}
	}
}

bool CF_CStrExp::execute(const char *szOrgStr, char *szRetStr)
{
	int iOrgStrLen = strlen(szOrgStr);
	int i;
	for (i = 0;m_iCharType[i] != CHARTYPE_END ;i++)
	{
		if (m_iCharType[i] == CHARTYPE_CONSTANT)
		{
			szRetStr[i] = m_szConstant[i];
		}
		else if (m_iCharType[i] == CHARTYPE_VARIABLE)
		{
			if (m_iVariable[i] > iOrgStrLen)
			{
				return false;
			} 
			szRetStr[i] = szOrgStr[m_iVariable[i] - 1];
		}
	}
	szRetStr[i] = NULL;
	return true;
}
			
			
				
				
		
		
		
		