/*******************************************************************
*******************************************************************/
#ifndef _RATEGROUP_H_
#define _RATEGROUP_H_

#include "CF_Common.h"
#include "CF_Config.h"
//#include "CF_COracleDB.h"
#include "CF_CException.h" 
#include "psutil.h"

struct SRateGroup
{
  char szRateGroupId[6];
  char szStartTime[15];
  char szEndTime[15];
  char szTariffId[6];
  int  iRuleReportA;
  int  iRuleReportB;
  int  iRepnoCdrduration;
  int  iRepnoRateduration;
  int  iChargeType;
  int  iPriNo;
  int  iCalcMode;
  double dUnitPrice;
  int  iCarryMode;
  int  iCarryUnit;
  int  iRepnoCdrcount;
  char szFormulaID[6];
};

class CRateGroup
{
private:
  int m_iRecordCount;
  SRateGroup *m_RateGroup;
  SRateGroup *cur_rategroup;
  SRateGroup *first_rategroup;
  
  bool m_ArriveTop;
  bool m_ArriveBottom;
  char szCurGroupId[6];
  char szCurCdrTime[15];
  char szDebugFlag[2];
  int compareRateGroup(char *szDebugFlag, SRateGroup* cur_rategroup, SRateGroup &srategroup);
  int compareRateGroupTime(char *szDebugFlag, SRateGroup* cur_rategroup, char *szTrueTime);  
  int compareTime(char *debug_flag, char *begintime, char *endtime, char *truetime);
  
public:
  CRateGroup();
  ~CRateGroup();
  bool clear();
  bool init(char *szRateGroupTableName, char *szRateLevelTableName, char *szGroupLevelTableName, char *szDebug);
  bool getFirst(SRateGroup &RG);
  bool getNext(SRateGroup &RG);
  int  getCount();
  //DBConnection conn;//数据库连接
};


#endif 
