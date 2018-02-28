//CF_CStrExp.cpp
//by tanj 20060207
//�����Զ���ı��ʽ�������ַ�����ȡ��Ӧ���ַ����ָ��������ַ���

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
		//���ʽ������#��$��ͷ
		m_iCharType[0] = CHARTYPE_END;
		return false;
	}
	int iExpIte = 0;    //���ʽ�е�λ��
	int iCharIte = 0;   //����ַ����е�λ��
	while(true)
	{
		if (m_szExpression[iExpIte] == '#')
		{
			iExpIte++;
			if (m_szExpression[iExpIte] == NULL)
			{
				//��ʾת���ַ���\���治��Ϊ��
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
				// #�ĺ���ֻ�ܽ���һ���ַ�
				m_iCharType[0] = CHARTYPE_END;
				return false;
			}

		}
		else if (m_szExpression[iExpIte] == '$')
		{
			iExpIte++;
			if (m_szExpression[iExpIte] == NULL)
			{
				//$���治��Ϊ��
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
					//$������������
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
				//$���治�ܽ���$��\
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
			
			
				
				
		
		
		
		