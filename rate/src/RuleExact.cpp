/********************************************************  
*RuleExact.cpp                                               
*this class is designed to store search the exact rate rule
*which uses the AVL tree( or STL::MAP) as the data structure in memory, a
*periodly save the whole tree to the file.                 
*created by tanj 2005.4.11                                 
**********************************************************/


#include "RuleExact.h"

//需要替换到共享内存里申请空间
//CF_CMemManager MemManager(200*1024*1024);

CRuleExact::CRuleExact(char *_szFileName,char *_idxFileName)
{
	strcpy(CRuleExact::szFileName, _szFileName);
	strcpy(CRuleExact::szIdxFile, _idxFileName);
	CRuleExact::lCurRule = 0;
	CRuleExact::iIncreasedNo = 0;
	CRuleExact::iTotalNo = 0;

//判断文件是否存在，不存在，则建立该文件；
	char szPath[256];
	strcpy(szPath,szIdxFile);
	char *p = strrchr(szPath,'/');
	*p = 0;
	if (access(szPath,F_OK) != 0)
	{
		char errmsg[500];
		sprintf(errmsg, "Path(%s) not exist!", szPath);
    jsexcp::CException e(RATE_ERR_IN_OPEN_FILE, errmsg, __FILE__, __LINE__);
		throw e;		
	}
		
	MemIndex = memManager.Init(CRuleExact::szIdxFile);
	
}

bool CRuleExact::insertRule(SRuleStruct &_RuleStruct)
{
	SRuleVar *Var;
	SRuleValue *Value;
	
	if(memManager.pShm->iRuleCount>=memManager.pShm->iMaxRule)
	{
		return false;
	}
	
	Var = memManager.ShmVarPos++;
	Value = memManager.ShmValuePos++;
  if (strlen(_RuleStruct.szUpdateTime) == 0)
  {
  	getCurTime(Value->szUpdateTime);
  }
  else
  {
  	strcpy(Value->szUpdateTime, _RuleStruct.szUpdateTime);
  }

	strcpy(Var->szRateRule, _RuleStruct.szRateRule);
  strcpy(Var->szStartTime, _RuleStruct.szStartTime);
  strcpy(Var->szEndTime, _RuleStruct.szEndTime);
  Var->setRule();
  Value->iRuleNo = _RuleStruct.iRuleNo;
  strcpy(Value->szRateGroupId, _RuleStruct.szRateGroupId);
    
 	
	if (!RuleMap.insert(std::make_pair(*Var, *Value)).second)
	{
		return false;
	}
	else
	{
	  iIncreasedNo++;
	  memManager.pShm->iRuleCount++;
	  iTotalNo++;
		return true;
	}
}

bool CRuleExact::searchRule(SRuleStruct &_RuleStruct)
{
	SRuleVar Var;
	SRuleValue Value;
	char szCurTime[15];
	getCurTime(szCurTime);
	char szTemp[1024];
	strcpy(Var.szRateRule , szTemp);
	strcpy(Var.szRateRule, _RuleStruct.szRateRule);
  strcpy(Var.szStartTime, _RuleStruct.szStartTime); 
  Var.setCdr();  //very important, to identify the Var as a cdr, not a rule
	std::map<SRuleVar, SRuleValue>::iterator RuleIterator;
  if ((RuleIterator = RuleMap.find(Var)) == RuleMap.end())
  {
  	if((RuleIterator = RuleMapTemp.find(Var)) == RuleMapTemp.end())
  	return false;
  	else
  		{
  			strcpy((RuleIterator->second).szUpdateTime, szCurTime);
  			Value = RuleIterator->second;
  			_RuleStruct.iRuleNo = Value.iRuleNo;
  			strcpy(_RuleStruct.szRateGroupId, Value.szRateGroupId);
  			strcpy(_RuleStruct.szStartTime, (RuleIterator->first).szStartTime);
  			strcpy(_RuleStruct.szEndTime, (RuleIterator->first).szEndTime);
  			return true;
  		}
  }
  else
  {
  	strcpy((RuleIterator->second).szUpdateTime, szCurTime);
  	Value = RuleIterator->second;
  	_RuleStruct.iRuleNo = Value.iRuleNo;
  	strcpy(_RuleStruct.szRateGroupId, Value.szRateGroupId);
  	strcpy(_RuleStruct.szStartTime, (RuleIterator->first).szStartTime);
  	strcpy(_RuleStruct.szEndTime, (RuleIterator->first).szEndTime);
  	return true;
  }
}

void CRuleExact::saveRule(char *szBakupFile)
{
	char szTempName[FILE_NAME_LEN];
	sprintf(szTempName, "%s.tmp", CRuleExact::szFileName);
	int m_fd = open(szTempName, O_RDWR|O_CREAT, 0666);
	if(m_fd == -1)
	{
		//TODO throw here
		char errmsg[500];
		sprintf(errmsg, "open file=%s= err", szTempName);
		throw jsexcp::CException(RATE_ERR_IN_OPEN_FILE, errmsg, __FILE__, __LINE__);
	}		
	ofstream OutStream;
	OutStream.open(szTempName);
	if (!OutStream)
	{
		char errmsg[500];
		sprintf(errmsg, "Cannot open file %s", szTempName);
		jsexcp::CException e(RATE_ERR_IN_OPEN_FILE, errmsg, __FILE__, __LINE__);
		throw e;
  }
	SRuleVar Var;
	SRuleValue Value;
	SRuleStruct RuleStruct;
	std::map<SRuleVar, SRuleValue>::iterator RuleIterator;	
	char szOutputTemp[500];
	for (RuleIterator = RuleMap.begin(); RuleIterator != RuleMap.end(); RuleIterator++)
	{
		Var = RuleIterator->first;
		Value = RuleIterator->second;
    strcpy(RuleStruct.szUpdateTime,         Value.szUpdateTime);
    strcpy(RuleStruct.szStartTime,          Var.szStartTime);
    strcpy(RuleStruct.szEndTime,            Var.szEndTime);
    strcpy(RuleStruct.szRateRule , Var.szRateRule);
    RuleStruct.iRuleNo = Value.iRuleNo;
    strcpy(RuleStruct.szRateGroupId, Value.szRateGroupId);
    RuleWriteToFile(OutStream, RuleStruct,szTempName);
	}
	OutStream.close();
	if (access(szFileName, F_OK) == 0)
	{
	  if (rename(szFileName, szBakupFile))
	  {
	  	char errmsg[500];
	  	sprintf(errmsg, "Cannot rename file %s to %s", szFileName, szBakupFile);
	  	jsexcp::CException e(RATE_ERR_IN_RENAME_FILE, errmsg, __FILE__, __LINE__);
			throw e;
	  }
	}
	if (rename(szTempName, szFileName))
	{
		char errmsg[500];
		sprintf(errmsg, "Cannot rename file %s to %s", szTempName, szFileName);
	  jsexcp::CException e(RATE_ERR_IN_RENAME_FILE, errmsg, __FILE__, __LINE__);
		throw e;		
	}		
	clearIncreasedNo();
}

void CRuleExact::RuleWriteToFile(ofstream &ostream, SRuleStruct &Rule, char *szFName)
{
	char szTemp[500];
	Rule.outputToStr(szTemp, sizeof(szTemp));
	ostream<<szTemp<<endl;
	if ( !(ostream.good()) )
	{
	  char errmsg[256];
	  sprintf(errmsg, "Cannot write  file %s",szFName);
    jsexcp::CException e(RATE_ERR_IN_WRITE_FILE, errmsg, __FILE__, __LINE__);
		throw e;
	}
}

//get the rules from file one by one 
bool CRuleExact::readFromFile(SRuleStruct &_RuleStruct)
{
	char szTemp[500];
	InStream.getline(szTemp, sizeof(szTemp));
	if ( !(InStream.good()) )
	{
	  return false ;
	}
	else
	{
		if (_RuleStruct.inputFromStr(szTemp))
		{
			lCurRule++;
			return true;
		}
		else
		{
			return false;
		}
	}
}

bool CRuleExact::rewindFile()
{
	//判断目录是否存在
	char szPath[256];
	strcpy(szPath,szFileName);
	char *p = strrchr(szPath,'/');
	*p = 0;
	if (access(szPath,F_OK) != 0)
	{
		char errmsg[500];
		sprintf(errmsg, "Path(%s) not exist!", szPath);
    jsexcp::CException e(RATE_ERR_IN_OPEN_FILE, errmsg, __FILE__, __LINE__);
		throw e;		
	}

		
	CRuleExact::lCurRule = 0;
	if (InStream)
	{
		InStream.close();
	}
	InStream.open(szFileName);
	InStream.seekg(0, ios::beg);
	if (!InStream)
	{
		char errmsg[500];
		sprintf(errmsg, "Cannot open file %s", szFileName);
		//throw CF_CError(ERR_TYPE_OS, ERR_LEVEL_HIG, errno, RATE_ERR_IN_OPEN_FILE, errmsg, __FILE__, __LINE__);
		return false;
	}

}
	
int CRuleExact::getIncreasedNo()
{
	return CRuleExact::iIncreasedNo;
}
void CRuleExact::clearIncreasedNo()
{
	CRuleExact::iIncreasedNo = 0;
}
int CRuleExact::getTotalNo()
{
	return CRuleExact::iTotalNo;
}


void CRuleExact::clearRule()
{
	RuleMap.clear();
	CRuleExact::lCurRule = 0;
	CRuleExact::iIncreasedNo = 0;
	CRuleExact::iTotalNo = 0;
}
void CRuleExact::closeRuleFile()
{
	//if (InStream)
	//{
		InStream.close();
	//}
}

bool CRuleExact::collectRule(SRuleStruct &_RuleStruct)
{
	SRuleVar Var;
	SRuleValue Value;
  if (strlen(_RuleStruct.szUpdateTime) == 0)
  {
  	getCurTime(Value.szUpdateTime);
  }
  else
  {
  	strcpy(Value.szUpdateTime, _RuleStruct.szUpdateTime);
  }

	strcpy(Var.szRateRule , _RuleStruct.szRateRule);	
  strcpy(Var.szStartTime, _RuleStruct.szStartTime);
  strcpy(Var.szEndTime, _RuleStruct.szEndTime);
  Var.setRule();
  Value.iRuleNo = _RuleStruct.iRuleNo;
  strcpy(Value.szRateGroupId, _RuleStruct.szRateGroupId);
  
  if (strlen(_RuleStruct.szStartTime)==0&&strlen(_RuleStruct.szEndTime)==0)
	{
		DEBUG_LOG<<"RuleMapTemp raterule is "<<_RuleStruct.szRateRule<<";"<<endd;
		DEBUG_LOG<<"RuleMapTemp raterule starttime "<<_RuleStruct.szStartTime<<";"<<endd;
		DEBUG_LOG<<"RuleMapTemp raterule endtime "<<_RuleStruct.szEndTime<<";"<<endd;
	}
	
	if(!RuleMapTemp.insert(std::make_pair(Var, Value)).second)
		{
			return false;
		}
	else
	{
	  iIncreasedNo++;
	  //iTotalNo++;
		return true;
	}
}


bool CRuleExact::loadTmpMap()
{
	std::map<SRuleVar, SRuleValue>::iterator iter;
	
	for (iter = RuleMapTemp.begin(); iter != RuleMapTemp.end(); iter++ ) 
		{
			if(memManager.pShm->iRuleCount>=memManager.pShm->iMaxRule)
	    {
		    break;
	    }
			std::map<SRuleVar, SRuleValue>::iterator RuleIterator;			
  		if ((RuleIterator = RuleMap.find(iter->first)) == RuleMap.end())
  		{
  			  //DEBUG_LOG<<"current raterule is "<<MemManager.ShmVarPos->szRateRule<<";"<<endd;
			    //DEBUG_LOG<<"source raterule is "<<iter->first.szRateRule<<";"<<endd;  			  		
					strcpy(memManager.ShmVarPos->szRateRule , iter->first.szRateRule);	
  				strcpy(memManager.ShmVarPos->szStartTime, iter->first.szStartTime);
  				strcpy(memManager.ShmVarPos->szEndTime, iter->first.szEndTime);
  				memManager.ShmVarPos->setRule();
  				strcpy(memManager.ShmValuePos->szUpdateTime, iter->second.szUpdateTime);
 					memManager.ShmValuePos->iRuleNo = iter->second.iRuleNo;
  				strcpy(memManager.ShmValuePos->szRateGroupId, iter->second.szRateGroupId);				
			
  				memManager.ShmValuePos++;
  				memManager.ShmVarPos++;
					memManager.pShm->iRuleCount++;
				}
    }
  RuleMapTemp.clear();
  loadRuleMap();
}

bool CRuleExact::loadRuleMap()
{
	if(!RuleMap.empty())
	RuleMap.clear();
	DEBUG_LOG<<"MemManager count ..."<<memManager.pShm->iRuleCount<<endd;
	DEBUG_LOG<<"MemManager Version ..."<<memManager.pShm->MemVersion<<endd;
	DEBUG_LOG<<"memory size"<<memManager.pShm->iCurSize<<endd;
	DEBUG_LOG<<"begin address"<<memManager.RealVarShm<<endd;
	DEBUG_LOG<<"current address"<<memManager.ShmVarPos<<endd;
	DEBUG_LOG<<"unit size"<<sizeof(SRuleVar)<<endd;
	DEBUG_LOG<<"unit num"<<memManager.ShmVarPos-memManager.RealVarShm<<endd;
	DEBUG_LOG<<"max num"<<memManager.pShm->iMaxRule<<endd;
	
	
	
	for(int i=0;i < memManager.pShm->iRuleCount ;i++) 
	{
		RuleMap.insert(std::make_pair(memManager.RealVarShm[i], memManager.RealValueShm[i]));
		
	}
	//iTotalNo = RuleMapTemp.size()+ MemManager.pShm->iRuleCount;
}