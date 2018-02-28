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
	//20050106 by tanj 增加按字段指定规则字段匹配模式的功能，
	//szMatchMode是用分隔符区分iRuleItemCount个匹配模式字符的字符串，
	//匹配模式字符为1表示模糊匹配；为2表示精确匹配
  CRuleBlur(int iRuleItemCount,const char *szMatchModeStr = NULL);
  //long loadRule(char *_szTableName);
  bool addRule(SRuleStruct &Rule);
  bool searchCdr(SRuleStruct &Cdr);
  int getCount();
  void travel();
  void clearRule();
};

















#endif 
