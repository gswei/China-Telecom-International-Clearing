/*******************************************************************
*RuleBlur.h
*this class is designed for ambigulous search 
*created by tanj
*******************************************************************/
#ifndef _RULEBLUR_H_
#define _RULEBLUR_H_

#include "CF_Common.h"
#include "CF_Config.h"
//#include "CF_COracleDB.h"
#include "RuleStruct.h"
#include "IndexTree.h"
class CRuleBlur
{
private:
  CIndexTree Tree;
  int m_iBlurRuleCount;
  int m_iRuleItemCount;
  int *m_iMatchModeArray;
public:
	//20050106 by tanj ���Ӱ��ֶ�ָ�������ֶ�ƥ��ģʽ�Ĺ��ܣ�
	//szMatchMode���÷ָ�������iRuleItemCount��ƥ��ģʽ�ַ����ַ�����
	//ƥ��ģʽ�ַ�Ϊ1��ʾģ��ƥ�䣻Ϊ2��ʾ��ȷƥ��
  CRuleBlur(int iRuleItemCount,const char *szMatchModeStr = NULL);
  //long loadRule(char *_szTableName);
  bool addRule(SRuleStruct &Rule);
  bool searchCdr(SRuleStruct &Cdr);
  int getCount();
  void travel();
  void clearRule();
};

















#endif 
