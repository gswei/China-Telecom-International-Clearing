/*******************************************************************
*******************************************************************/
#ifndef _CHARGETYPE_H_
#define _CHARGETYPE_H_

#include "CF_Common.h"
#include "CF_Config.h"
//#include "CF_COracleDB.h"
#include "CF_CException.h"
#include "psutil.h"

struct SChargeType
{
  int iChargeType;
  int iColindexFeeA;
  int iColindexFeeB;
  int iColindexFeeAddA;
  int iColindexFeeAddB;
  int iColindexMeterCount;
  int iColindexCdrDuration;
  int iColindexRateDuration;
  int iColindexRuleNo;
  int iColindexRategroupId;
  int iColindexTariffId;
  int iColindexRepnoA;
  int iColindexRepnoB;
  int iColindexCdrCount;
  int iChargeStyle;
  int iColindexRateA;
  int iColindexRateB;
  int iColindexRateAddA;
  int iColindexRateAddB;  
  int iColindexRepnoCdrduration;
  int iColindexRepnoRateduration;
  int iColindexRepnoCdrcount;
  int iColindexRateFlag;
};

class CChargeType
{
private:
  int m_iRecordCount;
  SChargeType *m_ChargeType;
  SChargeType *cur_ChargeType;
  char szDebugFlag[2];
  int iCurChargeType;
  int compareChargeType(char *szDebugFlag, SChargeType* cur_ChargeType, SChargeType &sChargeType);
  //DBConnection conn;//数据库连接
public:
  CChargeType();
  ~CChargeType();
  bool clear();
  bool init(char *szTableName, char *szDebug);
  bool getRecord(SChargeType &CT);
  int  getCount();
};


#endif 
