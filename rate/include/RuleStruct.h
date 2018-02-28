/**********************************************************************************
*RuleStruct.h
*this file contains the definition of SRuleVar and SRuleValue
*created by tanj 2005.4.11
***********************************************************************************/
#ifndef _RULESTRUCT_H_
#define _RULESTRUCT_H_

#include <errno.h>
#include <string.h>
#include "CF_Common.h"
#include "CF_CException.h" 
#include "rate_error_define.h"


const char RATE_RULE_SEPERATOR = ',';
const int RATE_MAX_RULEITEM_NUM = 30;   //规则字段的最大个数
const int RATE_MAX_RULEITEM_LEN = 100;   //每个规则字段的最大长度
const long RULE_MAX_COUNT = 2;
const int RATE_MAX_RULE_LEN = 250;   //规则的最大长度

//szRateRule中的规则项必须按特定的顺序
//  szInRouteType;szOutRouteType;szCallingDistrict;szCalledDistrict);
//  szCallingTollcode;szCalledTollcode;szCallingMobile;szCalledMobile;
//  szCallingBusiness;szCalledBusiness;szCallingService_A;szCalledService_A;
//  Rule.szCallingService_B;szCalledService_B;szCallingRegion;szCalledRegion;
//  .szGateway;
struct SRuleStruct 
{
  char szRateRule[RATE_MAX_RULE_LEN];
  char szStartTime[15];
  char szEndTime[15];
  char szUpdateTime[15];    //该话单最后一次被用到的时间,不一定是更新的时间
  int  iRuleNo;	
  char szRateGroupId[11];    char szRuleFieldMatchMode[RATE_MAX_RULE_LEN];
//  char szTariffId[6];
//  int iRuleReportA;
//  int iRuleReportB;
//  int iChargeType;
  bool outputToStr(char *szOutStr_, int iLen_);
  bool inputFromStr(char *szInStr_);
  void getRuleItem(char RuleItem[][RATE_MAX_RULEITEM_LEN],int &iCount);    void getRateModeItem(char RateModeItem[][RATE_MAX_RULEITEM_LEN],int &iCount);
};
//将规则项从szRateRule中分开写到RuleItem数组中去，iCount是规则项的个数（输出参数）
inline void SRuleStruct::getRuleItem(char RuleItem[][RATE_MAX_RULEITEM_LEN], int &iCount)
{
	char *szTemp1, *szTemp2;
	//cout<<"rate rule is"<<szRateRule;
	szTemp1 = szTemp2 = szRateRule;
	int i = 0;
	for (; ; i++)
	{
		if ((szTemp2 = (strchr(szTemp1, RATE_RULE_SEPERATOR))) == NULL)
		{
			strcpy(RuleItem[i], szTemp1);
			i++;
			break;
		}
		memcpy(RuleItem[i], szTemp1, szTemp2 - szTemp1);
		memset(RuleItem[i] + (szTemp2 - szTemp1), 0, sizeof(char));
		szTemp2++;
		szTemp1 = szTemp2;
	}
	iCount = i;
}
		   //将规则项的匹配模式从szRateMode 中分开写到RateModeItem数组中去，iCount是规则项的个数（输出参数）
inline void SRuleStruct::getRateModeItem(char RateModeItem[][RATE_MAX_RULEITEM_LEN], int &iCount)
{
	char *szTemp1, *szTemp2;
	//cout<<"rate rule is"<<szRateRule;
	szTemp1 = szTemp2 = szRuleFieldMatchMode;
	int i = 0;
	for (; ; i++)
	{
		if ((szTemp2 = (strchr(szTemp1, RATE_RULE_SEPERATOR))) == NULL)
		{
			strcpy(RateModeItem[i], szTemp1);
			i++;
			break;
		}
		memcpy(RateModeItem[i], szTemp1, szTemp2 - szTemp1);
		memset(RateModeItem[i] + (szTemp2 - szTemp1), 0, sizeof(char));
		szTemp2++;
		szTemp1 = szTemp2;
	}
	iCount = i;
}
		
inline bool SRuleStruct::outputToStr(char *szOutStr_, int iLen_)
{
	if ( (strlen(szRateRule) + strlen(szStartTime) + strlen(szEndTime) +
	      strlen(szRateGroupId) + strlen(szUpdateTime) + sizeof(int) + 5) >= iLen_)
	{
		return false;
	}
	sprintf(szOutStr_, "%s%c%d%c%s%c%s%c%s%c%s",szUpdateTime,RATE_RULE_SEPERATOR,
	        iRuleNo, RATE_RULE_SEPERATOR,szRateGroupId,RATE_RULE_SEPERATOR,
	        szStartTime,RATE_RULE_SEPERATOR,szEndTime,RATE_RULE_SEPERATOR,szRateRule);
}

//原字符串(szInStr)中分隔符会被改写成'\0'
inline bool SRuleStruct::inputFromStr(char *szInStr_)
{
  char *pTemp1,*pTemp2;
  pTemp1 = szInStr_;
  pTemp2 = strchr(pTemp1, RATE_RULE_SEPERATOR);
  if (pTemp2 == NULL)
  {
  	return false;
  }
  *pTemp2 = 0;
  strcpy(szUpdateTime, pTemp1);
  pTemp1 = pTemp2 + 1;
  
  pTemp2 = strchr(pTemp1, RATE_RULE_SEPERATOR);
  if (pTemp2 == NULL)
  {
  	return false;
  }
  *pTemp2 = 0;
  iRuleNo = atol(pTemp1);
  pTemp1 = pTemp2 + 1;

  pTemp2 = strchr(pTemp1, RATE_RULE_SEPERATOR);
  if (pTemp2 == NULL)
  {
  	return false;
  }
  *pTemp2 = 0;
  strcpy(szRateGroupId, pTemp1);
  pTemp1 = pTemp2 + 1; 
  	
  pTemp2 = strchr(pTemp1, RATE_RULE_SEPERATOR);
  if (pTemp2 == NULL)
  {
  	return false;
  }
  *pTemp2 = 0;
  strcpy(szStartTime, pTemp1);
  pTemp1 = pTemp2 + 1;    	
   	
  pTemp2 = strchr(pTemp1, RATE_RULE_SEPERATOR);
  if (pTemp2 == NULL)
  {
  	return false;
  }
  *pTemp2 = 0;
  strcpy(szEndTime, pTemp1);
  pTemp1 = pTemp2 + 1;  

  strcpy(szRateRule, pTemp1);
  return true;
}



//当VarType = CDR 时，szStartTime是话单时间，此时szEndTime应该是空
struct SRuleVar
{
  char szRateRule[RATE_MAX_RULE_LEN];
	char szStartTime[15];	
	char szEndTime[15];

	enum
	{
		RULE,CDR
	}VarType;
	bool operator==(const SRuleVar &right) const;
	bool operator<(const SRuleVar &right) const;
	bool operator>(const SRuleVar &right) const;
	int timeCmp(const SRuleVar &right) const;
	SRuleVar()
	{
		memset(szRateRule , 0,sizeof(szRateRule));
		memset(szStartTime, 0, sizeof(szStartTime));
		memset(szEndTime, 0, sizeof(szEndTime));
		VarType = RULE;
	}
	bool isRule()
	{
		if (VarType == RULE)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	bool isCdr()
	{
		if (VarType == CDR)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	void setRule()
	{
		VarType = RULE;
	}
	void setCdr()
	{
		VarType = CDR;
	}
};
inline int SRuleVar::timeCmp(const SRuleVar &right) const
{
  if (VarType == RULE && right.VarType == CDR)
  {
  	 if (strlen(szEndTime) != 0)
  	 {   	 	
  	   if (strcmp(right.szStartTime, szStartTime) >= 0 && 
  	       strcmp(right.szStartTime, szEndTime) <= 0)
  	   {
  	   	 return 0;
  	   }
  	   else if (strcmp(right.szStartTime, szStartTime) < 0)
  	   {
  	   	 return 1;
  	   }
  	   else
  	   {
  	   	 return -1;
  	   }
  	 }
  	 else
  	 {
  	 	 if (strcmp(right.szStartTime, szStartTime) >= 0)
  	 	 {
  	 	 	 return 0;
  	 	 }
  	 	 else
  	 	 {
  	 	 	 return 1;
  	 	 }
  	 }
  }
  else if (VarType == RULE && right.VarType == RULE)
  {
  	 if (strlen(szEndTime) != 0 && strlen(right.szEndTime) != 0)
  	 {
  	   if (strcmp(szStartTime, right.szStartTime) == 0 &&
  	       strcmp(szEndTime, right.szEndTime) == 0 )
  	   {
  	   	 return 0;
  	   }
  	   else if (strcmp(szStartTime, right.szEndTime) > 0)
  	   {
  	   	 return 1;
  	   }
  	   else if (strcmp(szEndTime, right.szStartTime) < 0)
  	   {
  	   	 return -1;
  	   }
  	   else
  	   {
      	 char errmsg[500];
      	 sprintf(errmsg, "times overlap (%s, %s), (%s, %s)", szStartTime, szEndTime,
      	                  right.szStartTime, right.szEndTime);
				 CException e(RATE_TIMES_OVERLAP, errmsg, __FILE__, __LINE__);
				 throw e;      	                  
      }	   	 
  	 }
  	 else if (strlen(szEndTime) == 0 && strlen(right.szEndTime) != 0)
  	 {
  	 	 if (strcmp(szStartTime, right.szEndTime) > 0)
  	 	 {
  	 	 	 return 1;
  	 	 }
  	 	 else
  	 	 {
      	 char errmsg[500];
      	 sprintf(errmsg, "times overlap (%s, %s), (%s, %s)", szStartTime, szEndTime,
      	                  right.szStartTime, right.szEndTime);
				 CException e(RATE_TIMES_OVERLAP, errmsg, __FILE__, __LINE__);
				 throw e;      	                  
      }
    }
    else if (strlen(szEndTime) != 0 && strlen(right.szEndTime) == 0)
    {
    	 if (strcmp(szEndTime, right.szStartTime) < 0)
    	 {
    	 	 return -1;
    	 }
    	 else
    	 {
      	 char errmsg[500];
      	 sprintf(errmsg, "times overlap (%s, %s), (%s, %s)", szStartTime, szEndTime,
      	                  right.szStartTime, right.szEndTime);
				 CException e(RATE_TIMES_OVERLAP, errmsg, __FILE__, __LINE__);
				 throw e;      	                  
      }
    }
    else
    {
    	 if (strcmp(szStartTime, right.szStartTime) == 0)
    	 {
    	 	 return 0;
    	 }
    	 else
    	 {
      	 char errmsg[500];
      	 sprintf(errmsg, "times overlap (%s, %s), (%s, %s)", szStartTime, szEndTime,
      	                  right.szStartTime, right.szEndTime);
				 CException e(RATE_TIMES_OVERLAP, errmsg, __FILE__, __LINE__);
				 throw e;      	                  
      }
    }  	   	 	 
  }
  else if (VarType == CDR && right.VarType == RULE)
  {
  	 if (strlen(right.szEndTime) != 0)
  	 {   	 	
  	   if (strcmp(szStartTime, right.szStartTime) >= 0 && 
  	       strcmp(szStartTime, right.szEndTime) <= 0)
  	   {
  	   	 return 0;
  	   }
  	   else if (strcmp(szStartTime, right.szStartTime) < 0)
  	   {
  	   	 return -1;
  	   }
  	   else
  	   {
  	   	 return 1;
  	   }
  	 }
  	 else
  	 {
  	 	 if (strcmp(szStartTime, right.szStartTime) >= 0)
  	 	 {
  	 	 	 return 0;
  	 	 }
  	 	 else
  	 	 {
  	 	 	 return -1;
  	 	 }
  	 }
  }
}
inline bool SRuleVar::operator==(const SRuleVar &right) const
{
	if ( strcmp(szRateRule, right.szRateRule))
   {
      if (timeCmp(right) == 0)
      {
      	return true;
      }
      else
      {
      	return false;
      }
   }
   else
   {
   	 return false;
   }

}
inline bool SRuleVar::operator<(const SRuleVar &right) const 
{
	int flag = strcmp(szRateRule, right.szRateRule);
  if (flag < 0)
  {
  	return true;
  }
  else if (flag > 0)
  {
  	return false;
  }
  
  if (timeCmp(right) < 0)
  {
  	return true;
  }
  else
  {
  	return false;
  }
}
			
inline bool SRuleVar::operator>(const SRuleVar &right) const
{
	if (*this == right)
	{
		return false;
	}
	else
	{
		return !(*this < right);
	}
}
		
struct SRuleValue
{
	int iRuleNo;
	char szRateGroupId[11];
	char szUpdateTime[15];
};






#endif 
