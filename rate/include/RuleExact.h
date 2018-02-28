/********************************************************  
*RuleExact.h                                               
*this class is designed to store search the exact rate rule
*which uses the AVL tree( or STL::MAP) as the data structure in memory, a
*periodly save the whole tree to the file.                 
*created by tanj 2005.4.11                                 
**********************************************************/
#ifndef _RULEXACT_H_
#define _RULEXACT_H_

#include <map>
#include <fstream.h>
#include <stdlib.h>
#include <string.h>
#include "CF_Common.h"
#include "RuleStruct.h"
#include "CF_CMemManager.h"


class CRuleExact
{
private:
  int iIncreasedNo;

  char szFileName[FILE_NAME_LEN];
  char szIdxFile[FILE_NAME_LEN];
  std::map<SRuleVar , SRuleValue> RuleMap;

  long lCurRule;
	ifstream InStream;
	
private:
  void RuleWriteToFile(ofstream &ostream, SRuleStruct &Rule, char *szFName);

public:
	MemManager memManager;
	int MemIndex;
	  int iTotalNo;
  std::map<SRuleVar, SRuleValue> RuleMapTemp;	    
  
public:
  CRuleExact(char *_szFileName,char *_idxFileName);
  bool searchRule(SRuleStruct &_RuleStruct);
  bool insertRule(SRuleStruct &_RuleStruct);
  bool collectRule(SRuleStruct &_RuleStruct);
  bool loadTmpMap();
  bool loadRuleMap();
  void saveRule(char *szBakupFile);
  void closeRuleFile();
  bool readFromFile(SRuleStruct &_RuleStruct);
  bool rewindFile();
  int getIncreasedNo();
  void clearIncreasedNo();
  int getTotalNo();
  void clearRule();
};










#endif 

