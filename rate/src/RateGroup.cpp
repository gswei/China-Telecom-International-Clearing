/*******************************************************************
*******************************************************************/
#include "RateGroup.h"
using namespace std;
using namespace tpss;

CRateGroup::CRateGroup()
{
  m_iRecordCount = 0;
  m_RateGroup = NULL;
  m_ArriveTop = false;
  m_ArriveBottom = false;
}

CRateGroup::~CRateGroup()
{
  clear();
}

bool CRateGroup::clear()
{
  if (m_RateGroup)
  {
    delete[] m_RateGroup;
    m_RateGroup = NULL;
  }
  m_iRecordCount = 0;
  m_ArriveTop = false;
  m_ArriveBottom = false;  
  return true;
}

bool CRateGroup::init(char *szRateGroupTableName, char *szRateLevelTableName, char *szGroupLevelTableName, char *szDebug)
{
  int i, iCount;
  char szSqlTmp[SQL_LEN+1];
  char szLogStr[LOG_MSG_LEN+1];

  sprintf(szDebugFlag, "%s", szDebug);
  DBConnection conn;//数据库连接
  try{			
  	
	if (dbConnect(conn))
	 {
			Statement stmt = conn.createStatement();
			sprintf(szSqlTmp, "select count(*) from %s", szRateGroupTableName);
      stmt.setSQLString(szSqlTmp);	
			stmt.execute();
			stmt >> iCount;
    
      m_RateGroup=new SRateGroup[iCount];
      if (!m_RateGroup)
      {
    	  sprintf(szLogStr, "new m_RateGroup fail.");
    		jsexcp::CException e(-1, szLogStr, __FILE__, __LINE__);
    		throw e;		  
      }

       // 20130709 添加公式ID 获取  m_RateGroup[i].szFormulaID
      sprintf(szSqlTmp, "select a.RATEGROUP_ID, a.START_TIME, a.END_TIME, a.TARIFF_ID, a.FORMULA_ID, a.RULEREPNOA, a.RULEREPNOB, a.REPNO_CDRDURATION, a.REPNO_RATEDURATION, a.CHARGE_TYPE, a.PRI_NO, b.calc_mode, b.unit_price, b.carry_mode, b.carry_unit, c.repno_cdrcount from %s a, %s b, %s c where a.tariff_id=b.tariff_id  and a.rategroup_id = c.rategroup_id ORDER BY a.RATEGROUP_ID, a.PRI_NO", szRateGroupTableName, szRateLevelTableName, szGroupLevelTableName);
      stmt.setSQLString(szSqlTmp);	
	  stmt.execute();
      //***********************************************************
      // Repeat reading all record
      //***********************************************************
      for (i=0; i<iCount; i++) 
      {
        stmt>>m_RateGroup[i].szRateGroupId>>m_RateGroup[i].szStartTime>>
          m_RateGroup[i].szEndTime>>m_RateGroup[i].szTariffId>> m_RateGroup[i].szFormulaID>>
          m_RateGroup[i].iRuleReportA>>m_RateGroup[i].iRuleReportB>>
          m_RateGroup[i].iRepnoCdrduration>>m_RateGroup[i].iRepnoRateduration>>
          m_RateGroup[i].iChargeType>>m_RateGroup[i].iPriNo>>
          m_RateGroup[i].iCalcMode>>m_RateGroup[i].dUnitPrice>>
          m_RateGroup[i].iCarryMode>>m_RateGroup[i].iCarryUnit>>
          m_RateGroup[i].iRepnoCdrcount;
        //cout << "m_RateGroup[i].szFormulaID = " << m_RateGroup[i].szFormulaID << endl;
      } //end for
     
      //sprintf(szSqlTmp,"");
      //stmt.setSQLString();
	
	 }else{
  	 	cout<<"connect error."<<endl;
  	 	return false;
	 }
	    conn.close();
	 } catch( SQLException e ) {
  		cout<<e.what()<<endl;
  		theJSLog << "CRateGroup初始化 出错" << endi;
  		throw jsexcp::CException(0, "CRateGroup初始化 出错", __FILE__, __LINE__);
  		conn.close();
  		return false;
     } 

  m_iRecordCount=iCount;
	return true;
}

bool CRateGroup::getFirst(SRateGroup &RG)
{
  int pstart=0;
  int pend = m_iRecordCount-1;
  int pmiddle = (pend+pstart)/2;
  int plast = 0;
  int ifind = 0;

  int islarger = 0;
  int inrange=-2;
  int isequal = 0;
  char oper_time[15];

  cur_rategroup = m_RateGroup;
  sprintf(szCurGroupId, "%s", RG.szRateGroupId);
  sprintf(szCurCdrTime, "%s", RG.szStartTime);

  sprintf(oper_time, "%s", RG.szStartTime);
  while (pstart <= pend) {
    if ((isequal==0)&&(pstart == pend))
      isequal = 1;
      else if (isequal==1)
        break;
    pmiddle = (pend+pstart)/2;
    if (plast<pmiddle) {
      cur_rategroup=cur_rategroup+(pmiddle-plast);
    }
      else if (plast>pmiddle) {
        cur_rategroup=cur_rategroup-(plast-pmiddle);
      }
    
    islarger = compareRateGroup(szDebugFlag, cur_rategroup, RG);
    if (islarger>0) {
      //forward
      pstart = pmiddle+1;
      plast  = pmiddle;
    }
      else if (islarger<0) {
        //backward
        pend = pmiddle-1;
        plast  = pmiddle;
      }
        else if (islarger==0) {
        	//移动到最后一个
/****************20080226修改此段，增加时间的匹配****************/
        	while (m_iRecordCount-(pmiddle+1)>0)
        	{
        		cur_rategroup++;
        		pmiddle++;
        		if (compareRateGroup(szDebugFlag, cur_rategroup, RG) != 0)
        		{
        			cur_rategroup--;
        			pmiddle--;
        			break;
        		}
        		
        	}
        	//前移查找时间符合的该最高优先级的rategroup定义
        	int higher_pri = cur_rategroup->iPriNo;
        	while((compareRateGroup(szDebugFlag, cur_rategroup, RG) == 0)&&(cur_rategroup->iPriNo == higher_pri))
        		{
        			if (compareRateGroupTime(szDebugFlag, cur_rategroup, oper_time)==0)
        				{
        					ifind = 1;
        					break;
        				}
        			cur_rategroup--;
        		}
        	if((compareRateGroup(szDebugFlag, cur_rategroup, RG) != 0)||(cur_rategroup->iPriNo != higher_pri))
        		{
        			ifind = -1; //最高优先级没有匹配项，挂起
        		}
        		
               
/**********************20080226edit end***************************/
//          inrange = compareRateGroupTime(szDebugFlag, cur_rategroup, oper_time);
//          if (inrange==0) 
//          {
//            ifind = 1;
//            plast = pmiddle;
//            break;
//          }
//            else
//            {
//              //search around
//              int move_count =0;
//              if (pmiddle>0) {
//                move_count++;
//                pmiddle--;
//                cur_rategroup--;
//              }
//                else {
//                  //break;
//                  move_count=0;
//                }
//
//              islarger = compareRateGroup(szDebugFlag, cur_rategroup, RG);
//
//              while (islarger == 0) {
//                if (compareRateGroupTime(szDebugFlag, cur_rategroup, oper_time)==0) 
//                {
//                  ifind = 1;
//                  break;
//                }
//
//                if (pmiddle>0) {
//                  move_count ++;
//                  cur_rategroup--;
//                  pmiddle--;
//                }
//                  else
//                    break;
//
//                islarger = compareRateGroup(szDebugFlag, cur_rategroup, RG);
//              }
//              if (ifind == 1)
//                break;
//
//              for (int k=0;k<move_count+1;k++) {
//                pmiddle++;
//                cur_rategroup++;
//              }
//              move_count = 0 ;
//              islarger = compareRateGroup(szDebugFlag, cur_rategroup, RG);
//              while (islarger == 0) {
//                if (compareRateGroupTime(szDebugFlag, cur_rategroup, oper_time)==0) {
//                  ifind = 1;
//                  break;
//                }
//
//                if (m_iRecordCount-pmiddle>0) {
//                  pmiddle ++;
//                  cur_rategroup++;
//                  move_count ++;
//                }
//                  else
//                    break;
//
//                islarger = compareRateGroup(szDebugFlag, cur_rategroup, RG);
//              }
//              break;
//            }
        } //end else
        if (ifind == 1 || ifind == -1)
           break;
    }  //end while

  if (ifind==1) 
  {
    //write result
    //cout << "ifind = 1" <<endl;
    sprintf(RG.szStartTime, "%s", cur_rategroup->szStartTime);
    sprintf(RG.szEndTime, "%s", cur_rategroup->szEndTime);
    sprintf(RG.szTariffId, "%s", cur_rategroup->szTariffId);
    sprintf(RG.szFormulaID,"%s",cur_rategroup->szFormulaID);
    RG.iRuleReportA=cur_rategroup->iRuleReportA;
    RG.iRuleReportB=cur_rategroup->iRuleReportB;
    RG.iRepnoCdrduration=cur_rategroup->iRepnoCdrduration;
    RG.iRepnoRateduration=cur_rategroup->iRepnoRateduration;
    RG.iChargeType=cur_rategroup->iChargeType;
    RG.iPriNo=cur_rategroup->iPriNo;
    RG.iCalcMode=cur_rategroup->iCalcMode;
    RG.dUnitPrice=cur_rategroup->dUnitPrice;
    RG.iCarryMode=cur_rategroup->iCarryMode;
    RG.iCarryUnit=cur_rategroup->iCarryUnit;
    RG.iRepnoCdrcount=cur_rategroup->iRepnoCdrcount;
    first_rategroup = cur_rategroup;
    m_ArriveTop = false;
    m_ArriveBottom = false;
    return true;
  }
  else 
  {
    return false;
  }
}

bool CRateGroup::getNext(SRateGroup &RG)
{
	sprintf(RG.szRateGroupId, "%s", szCurGroupId);
	sprintf(RG.szStartTime, "%s", szCurCdrTime);
	while (m_ArriveTop == false)
	{
		cur_rategroup--;
		if (compareRateGroup(szDebugFlag, cur_rategroup, RG) == 0)
		{
			if (compareRateGroupTime(szDebugFlag, cur_rategroup, szCurCdrTime) == 0)
		  {
        sprintf(RG.szStartTime, "%s", cur_rategroup->szStartTime);
        sprintf(RG.szEndTime, "%s", cur_rategroup->szEndTime);
        sprintf(RG.szTariffId, "%s", cur_rategroup->szTariffId);
        RG.iRuleReportA=cur_rategroup->iRuleReportA;
        RG.iRuleReportB=cur_rategroup->iRuleReportB;
        RG.iRepnoCdrduration=cur_rategroup->iRepnoCdrduration;
        RG.iRepnoRateduration=cur_rategroup->iRepnoRateduration;
        RG.iChargeType=cur_rategroup->iChargeType;			  
        RG.iPriNo=cur_rategroup->iPriNo;
        RG.iCalcMode=cur_rategroup->iCalcMode;
        RG.dUnitPrice=cur_rategroup->dUnitPrice;
        RG.iCarryMode=cur_rategroup->iCarryMode;
        RG.iCarryUnit=cur_rategroup->iCarryUnit;
        RG.iRepnoCdrcount=cur_rategroup->iRepnoCdrcount;
			  return true;
		  }
		  else
		  {
			  continue;
		  }
		}
		else
		{
			m_ArriveTop = true;
			cur_rategroup = first_rategroup;
		}
	}

//	while (m_ArriveBottom == false)
//	{
//		cur_rategroup++;
//		if (compareRateGroup(szDebugFlag, cur_rategroup, RG) == 0)
//		{
//			if (compareRateGroupTime(szDebugFlag, cur_rategroup, szCurCdrTime) == 0)
//		  {
//        sprintf(RG.szStartTime, "%s", cur_rategroup->szStartTime);
//        sprintf(RG.szEndTime, "%s", cur_rategroup->szEndTime);
//        sprintf(RG.szTariffId, "%s", cur_rategroup->szTariffId);
//        RG.iRuleReportA=cur_rategroup->iRuleReportA;
//        RG.iRuleReportB=cur_rategroup->iRuleReportB;
//        RG.iChargeType=cur_rategroup->iChargeType;			  
//        RG.iPriNo=cur_rategroup->iPriNo;
//			  return true;
//		  }
//		  else
//		  {
//			  continue;
//		  }
//		}
//		else
//		{
//			m_ArriveBottom = true;
//			//cur_rategroup = first_rategroup;
//		}
//	}
	
	return false;
}

int CRateGroup::getCount()
{
	return m_iRecordCount;
}

int CRateGroup::compareRateGroup(char *szDebugFlag, SRateGroup* cur_rategroup, SRateGroup &srategroup)
{
  int iRet;
  iRet = strcmp(srategroup.szRateGroupId, cur_rategroup->szRateGroupId);
  //cout << "srategroup.szRateGroupId " << srategroup.szRateGroupId<<endl;
  //cout << "cur_rategroup->szRateGroupId " << cur_rategroup->szRateGroupId<<endl;
  return iRet;
}

int CRateGroup::compareRateGroupTime(char *szDebugFlag, SRateGroup* cur_rategroup, char *szTrueTime)
{
  int iRet;

  if ((compareTime(szDebugFlag, cur_rategroup->szStartTime, cur_rategroup->szEndTime, szTrueTime ) == 0))
  {
    iRet = 0; //时间完全匹配
  }
  else
  {
  	iRet = -1; //只要有任何一个时间不匹配，即返回-1
  }

  return iRet;
}

int CRateGroup::compareTime(char *debug_flag, char *begintime, char *endtime, char *truetime)
{
  int inrange;
/*
  expTrace(debug_flag, __FILE__, __LINE__, "Start CompareTime()");
  expTrace(debug_flag, __FILE__, __LINE__, "begintime=%s;", begintime);
  expTrace(debug_flag, __FILE__, __LINE__, "endtime=%s;", endtime);
  expTrace(debug_flag, __FILE__, __LINE__, "truetime=%s;", truetime);
*/

  if ((endtime == NULL) || (strcmp(endtime,"") == 0))
  {
    if (strcmp(truetime, begintime)>=0) {
      inrange = 0;
    }
      else {
        inrange = 1;
      }
  }
    else {
      if ((strcmp(truetime, begintime)>=0) &&
        (strcmp(truetime, endtime)<=0)) {
        inrange = 0;
      }
        else if (strcmp(truetime, begintime)<0) {
          inrange = 1;
        }
          else {
            inrange = -1;
          }
    }

  //expTrace(debug_flag, __FILE__, __LINE__, "End CompareTime(), Ret=%d;", inrange);
  return inrange;
}
