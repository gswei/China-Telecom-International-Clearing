/*******************************************************************
*RuleBlur.cpp
*this class is designed for ambigulous search 
*created by tanj
*******************************************************************/
#include "RuleBlur.h"

CRuleBlur::CRuleBlur(int iRuleItemCount,const char * szMatchModeStr):Tree(iRuleItemCount)
{
	//add by tanj 20060310
	m_iBlurRuleCount = 0;
	//end of addition
	m_iRuleItemCount = iRuleItemCount;
	m_iMatchModeArray = new int[m_iRuleItemCount];
	int iMatchMode;
	for (int i = 0; i < m_iRuleItemCount; i++)
	{
		if (szMatchModeStr != NULL)
		{
			if (!getField(i + 1, RATE_RULE_SEPERATOR,szMatchModeStr,iMatchMode))
			{
				jsexcp::CException e(RATE_ERR_IN_MATCH_MODE_STRING, "MATCH MODE ERR", __FILE__, __LINE__);
				throw e;	
			}
			m_iMatchModeArray[i] = iMatchMode;
		}
		else
		{
			m_iMatchModeArray[i] = 1;
		}
	}
}

bool CRuleBlur::addRule(SRuleStruct &Rule)
{
	SRuleValue RuleValue;
	SIndexNode IndexNode[RATE_MAX_RULEITEM_NUM];
	RuleValue.iRuleNo = Rule.iRuleNo;
	strcpy(RuleValue.szRateGroupId, Rule.szRateGroupId);
	char RuleItem[RATE_MAX_RULEITEM_NUM][RATE_MAX_RULEITEM_LEN];
	int iItemCount;	
	Rule.getRuleItem(RuleItem, iItemCount);
	if (iItemCount != m_iRuleItemCount)
	{
	  char errmsg[500];
	  sprintf(errmsg, "Rule Item Count error!FILED COUNT:%d;COLUMN COUNT:%d",iItemCount,m_iRuleItemCount);
		jsexcp::CException e(RATE_ERR_RULE_ITEM_COUNT, errmsg, __FILE__, __LINE__);
		throw e;		  
  }		
	int i = 0;
	for (; i < iItemCount; i++)
	{
		strcpy(IndexNode[i].szField, RuleItem[i]);
		IndexNode[i].iMatchMode = m_iMatchModeArray[i];
	}
	strcpy(IndexNode[i].szStartTime, Rule.szStartTime);
	strcpy(IndexNode[i].szEndTime, Rule.szEndTime);
	if (Tree.insertRule(IndexNode, RuleValue))
	{
		m_iBlurRuleCount++;
	}
	return true;
}



int CRuleBlur::getCount()
{
	return m_iBlurRuleCount;
}

bool CRuleBlur::searchCdr(SRuleStruct &Cdr)
{
	SIndexNode IndexNode[RATE_MAX_RULEITEM_NUM];	
	char RuleItem[RATE_MAX_RULEITEM_NUM][RATE_MAX_RULEITEM_LEN];
	int iItemCount;	
	Cdr.getRuleItem(RuleItem, iItemCount);
	int i = 0;
	for (; i < iItemCount; i++)
	{
		strcpy(IndexNode[i].szField, RuleItem[i]);
	}
	strcpy(IndexNode[i].szStartTime, Cdr.szStartTime);
	strcpy(IndexNode[i].szEndTime, Cdr.szEndTime);	

  SRuleValue Value;
 
  if (Tree.searchCdr(IndexNode, Value))
  {
    Cdr.iRuleNo = Value.iRuleNo;
    strcpy(Cdr.szRateGroupId, Value.szRateGroupId);
    SIndexNode Node;
    Tree.getIndexNode(i, Node);
    strcpy(Cdr.szStartTime, Node.szStartTime);
    strcpy(Cdr.szEndTime, Node.szEndTime);
    return true;
  }
  else
  {
  	return false;
  }
}    
void CRuleBlur::travel()
{
	Tree.travel();
}
    
void CRuleBlur::clearRule()
{
	Tree.clearTree();
	m_iBlurRuleCount = 0;
}        
  