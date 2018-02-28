#ifndef _SETTLERULE_H_
#define _SETTLERULE_H_ 1

#include <stdio.h>
#include <iostream.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <list>
#include <set>
//#include "RuleExact.h"
#include "RateGroup.h"
#include "CF_Config.h"
//#include "CF_COracleDB.h"
#include "CF_CException.h" 
#include "CF_Common.h"
#include "CF_CErrorLackRec.h"
#include "CF_CFscan.h"
#include "CF_CFmtChange.h"
#include "CF_CMemFileIO.h"
#include "DealRecord.h"
#include "rate_error_define.h"
#include "RuleStruct.h"
#include "RuleBlur.h"
#include "ChargeType.h"
#include "CF_CError_Table.h"
#include "CF_CInterpreter.h"
#include "psutil.h"
#include "Formula.h"
#include "tp_log_code.h"

const int RULE_BENCHMARK = 10000;
const int CALCFEE_RULE_LEN_MAX             =   1024;  //���۹�������/ֵƴ�Ӻ����󳤶�
const int CALCFEE_PLUGIN_VERSION_NO  		  =   0;	   //����汾��
const int RECORD_LEN = 101;
const int SERV_CAT_LENGTH = 5;


struct SListSql{
	char m_szSqlId[16+1];
	char m_szUpdateFlag[1+1];
	char m_szUpdateSqlName[32+1];
};

struct SPendingDuration {
  char szDate[9];
  int  iWeekly;
  char szBeginTime[7];
  char szEndTime[7];
};

struct SRateConfig {
  char szConfigId[6];
  char szTablenameRule[31];
  char szTablenameDefRule[31];
  char szTablenameGroupLevel[31];
  char szTablenameRateGroup[31];
  char szTablenameChargeType[31];
  char szTablenameRateLevel[31];
  char szTablenameRate[31];
  char szTablenameAdjrate[31];
  char szRulePipeName[31];
  char szTablenameFormula[31];
  char szTablenameFormulaParam[31];
};

//20130709 ��ӹ�ʽ���� �ṹ��
struct FormulaParam {
	char szParamName[64] ;            //��������
  	char szParamType[2];             //��������
  	char szParamValue[1024];         //����ֵ
};

//20130709 ��ӹ�ʽ���� �ṹ��
struct FormulaExp {
	int index_id;                          //��ʽ���
  	char szFormula[1024];              //���幫ʽ���ʽ
  	char szSegmentName[64];          //�ֶ���
  	char szStartTime[15];              //��Чʱ��
  	char szEndTime[15];				//ʧЧʱ��
  	int output_type;                   //0��ʾԭ�������ĿǰΪ��λС������1  ��ʾ����ȡ����
                                    //    2��ʾ����ȡ����3��ʾ�����̶�λ��С��
    int decimal_num;              //С�����λ��
};

//20130709 ��ӹ�ʽID �ṹ��
struct FormulaStruct {
	char szFormulaID[16];						//��ʽID
	//int szFormulaID;
    FormulaExp *szFormulaExp;    //��ʽ���ʽ
  	FormulaParam *szFormulaParam; // ��ʽ����
  	int expno;    //һ����ʽID ��Ӧ�Ĺ�ʽ����
  	int paramno; //��ʽID ��Ӧ�Ĳ�������
};

struct RATESTRUCT {
  char tariff_id[6];
  char start_time[15];
  char end_time[15];
  char cdrstart_date[9];
  char cdrend_date[9];  
  char cdrstart_time[7];
  char cdrend_time[7];
  int  start_weekly;
  int  end_weekly;
  int  pri_no;
  int  rate_a;
  int  rate_b;
  int  rate_add_a;
  int  rate_add_b;  
  int  meter_count;
  int  charge_unit;
};

struct ADJRATESTRUCT {
  char tariff_id[6];
  char start_time[15];
  char end_time[15];
  int duration_begin;
  int duration_end;  
  double rate_a_factor;
  int rate_a_base;
  double rate_b_factor;
  int rate_b_base;
  double rate_add_a_factor;
  int rate_add_a_base;
  double rate_add_b_factor;
  int rate_add_b_base;  
  int meter_count;
  int charge_unit;
};


struct RateCondition
{
	char szSourceId[6];
	char szRateContext[1024];
};

struct ServCatCol
{
	char szSourceId[6];
	int iColIdx;
};

struct SourceMessage{
	char szSourceId[6];
	char szSourcePath[64];	
};

struct RateAudit
{
	char ServCat[10];
	int  iAudit_id;
	char szCondition[1000];
	char start_time[15];
	char end_time[15];
	char Valid_Flag;
	int FileNum;
	char DealType;
	map<int,char*> mStatDim;
};

struct RateDispCheck
{
	char ServCatName[5];
	char CallingNo[21];
	char CalledNo[21];
	char start_time[15];
	//char end_time[15];
	int  duration_begin;
	//int  duration_end;
	int  time_drift;
	int  duration_drift;
	char Field1[100];
	char Field2[100];
	char classify_location;
	char FileType_id[6];
	int m_iRcdCount;
};

struct STparam
{
    //���̯������ֶ�
    char cfgid[4];   //���۹���ID
    char source_id[16];
    char settle_month[16]; //̯�������Ӧ���������ֶ�
	char intype[2];   //�������F Ϊ�ļ���TΪ���ݿ�
	char outtype[2]; //��������F Ϊ�ļ���TΪ���ݿ�
	char invalue[256];  //���·��
	char outvalue[256] ;  //����·��
	char intxt_type[10]; //��ڸ�ʽ
	char outtxt_type[10] ; //���ڸ�ʽ	
};

struct SParameter
{
  char szSourceGroup[6];            //����Դ����
  char ServiceID[6];								//����ID
  char szSourceId[6];
  char szFileName[512];
  char szSourcePath[512];
  char szInrcdType;                 //�����ļ���¼����
  char szOutrcdType;                //����ļ���¼����
  	char szInputFiletypeId[6];        //�����ļ�����
  char szOutputFiletypeId[6];       //����ļ�����
  char szDebugFlag[50];             //�Ƿ����������־
  char szCheckName[20];             //��Ϣ����־������
  bool bUseRuleMatch;								//�Ƿ�ʹ�þ�ȷƥ��
  bool bUseMultiFind;								//�Ƿ�ʹ�����߲���
  bool bUseDefRateRule;
  //CRuleExact *rule_exact;						//��ȷ������
  CRuleBlur *rule_blur;							//ģ��������
  CRateGroup *rate_group;						//��������
  CChargeType *charge_type;					//�Ʒ�������
  SRuleStruct *rsblur, rstmp,*defblur;				//
  RATESTRUCT *ratestruct;						//
  ADJRATESTRUCT *adjratestruct;  		//
  //char szRuleExactFile[256];				//��ȷ�����ļ���
  //char szRuleExactidx[256];					//��ȷ�������ڴ������ļ���
 // char szRuleExactFileBak[256];			//��ȷ�����ļ���(����)
 // int iRuleExpirationDate;					//��ȷ��������첻ʹ������ΪʧЧ
  int iBlurCount;										//ģ����������
  int iBlurMax;											//ģ���������ֵ
  int iDefBlurCount;										//ģ����������
  int iDefBlurMax;											//ģ���������ֵ
  int iRateCount;										//���ʹ�������
  int iAdjRateCount;								//��չ���ʹ�������
  int iRuleFieldCount;							//�����ֶθ���
  int *pRuleFieldIndex;							//���۹����ֶ����Ի�������Щ�ֶ�
  int *pRuleFieldIsNull;						//���۹����ֶ���Դ�Ļ����ֶ�ֵ�ɷ�Ϊ��
  int *pRuleFieldMatchMode;					//���۹����ֶ�ƥ��ģʽ
  int  iTimeIndex;            	    //���������жϵ�����/ʱ���ֶ����
  char szRateConfigId[6];						//��������ID
  char szFormulaConfigId[6];                  //���۹�ʽID 
  SRateConfig rate_config;					//����������Ϣ 
  char szRuleFieldItem[RATE_MAX_RULEITEM_NUM][RATE_MAX_RULEITEM_LEN];   	//ƴ��ǰ���ֶ�����/ֵ�ַ���
  char szRuleFieldName[CALCFEE_RULE_LEN_MAX];															//���۹����ֶ�����ƴ�Ӻ���ַ���
  char szRuleFieldCont[CALCFEE_RULE_LEN_MAX];															//���۹����ֶ�ֵƴ�Ӻ���ַ���
  char szRuleFieldMatchMode[CALCFEE_RULE_LEN_MAX];												//���۹����ֶ�ƥ��ģʽƴ�Ӻ���ַ���
  int iOutMode;											//���ģʽ��0�������ֵ���1�����ֵ���
  set<int> setColumn;               //���������Ҫ������д���ֶε����
  char szPluginFilename[256];				//��Ų���ļ���
  //ProcessRec_Mgr mgr;		//�����
  C_Compile m_Compile;/*��̬���������*/
  //Ϊʵ�������Ƿ������������ʽ������±���  liuw 2007-7-20
  int ConditionNum;
  RateCondition Rate_Condition[1024];
  int iErrorValue;
	char szResult[255];
	const char *theResult;
	
	int iRateFlag;
	
	int MemVersion;//�����ڴ�汾

	STparam szTparam;
	int iFormulacount;     //��ʽ������
	int iParamcount;      //��ʽ��Ӧ��������
	//FormulaStruct *szFormula;  // ��ʽ����
	//map<string,FormulaStruct> szformulastruct;
	FormulaStruct *szformulastruct;
	FormulaParam szFormulaParam ; //��ʽ����
	Formula *p_formula;  //��ʽ������
	char ratecycle[8];
	//map<string , double> formula_value; //T ���ͣ���Ҫ�ӱ���в����ݵ�map
  
  //DBConnection conn;//���ݿ�����
  
  
  SParameter()
  {
 	  memset(szSourceGroup, 0, sizeof(szSourceGroup));
 	  memset(ServiceID, 0, sizeof(ServiceID));
 	   	  memset(szInputFiletypeId, 0, 6);
 	  memset(szOutputFiletypeId, 0, 6);
 	  szInrcdType = 0;
 	  szOutrcdType = 0;
 	  sprintf(szDebugFlag, "%s", "Y");
 	  //memset(szSlPath, 0, 251);
    iTimeIndex = -1;
    memset(szCheckName, 0, 20);
    bUseRuleMatch = true;
    bUseMultiFind = false;
    //rule_exact = NULL;
    rule_blur = NULL;
    rsblur = NULL;  //rstmp
    ratestruct = NULL;
    adjratestruct = NULL;
   // memset(szRuleExactFile, 0, 256);
   // memset(szRuleExactFileBak, 0, 256);
    //iRuleExpirationDate = 0;
    iBlurCount = 0;
    iBlurMax = 0;
    iRateCount = 0;
    iAdjRateCount = 0;
    iRuleFieldCount = 0;
    pRuleFieldIndex = NULL;
    pRuleFieldIsNull = NULL;
    pRuleFieldMatchMode = NULL;
    memset(szRateConfigId, 0, 6);
    memset(szFormulaConfigId, 0, 6);
    //rate_config
    //ProcessMonitor
    //szRuleFieldItem
    memset(szRuleFieldName, 0, CALCFEE_RULE_LEN_MAX);
    memset(szRuleFieldCont, 0, CALCFEE_RULE_LEN_MAX);
    memset(szPluginFilename, 0, 256);
    //m_mgr;
  	//memset(szRateContext,0,sizeof(szRateContext));
  	iErrorValue = 0;
		memset(szResult,0,sizeof(szResult));
		setColumn.clear();
		MemVersion = 0;

  }
};

int InitRuleStruct(char *debug_flag, SRuleStruct **rulestruct, char *tablename,
  char *fieldname, int &rulecount, int &maxcount, char *pipe_id);
int InitDefRuleStruct(char *debug_flag, SRuleStruct **rulestruct, char *tablename,
  char *fieldname, int &rulecount, int &maxcount, char *pipe_id);
int FreeRuleStruct(char *debug_flag, SRuleStruct **rulestruct, int rulecount);
int SearchRuleBlur(char *debug_flag, SRuleStruct *rulestruct, int rulecount, SRuleStruct &srule);
int CompareRuleBlur(char *debug_flag, SRuleStruct *blurstruct, SRuleStruct *truestruct);
int CompareRuleBlur(char *debug_flag, char *ruletext, char *truetext,char *rulemode);
int CheckRuleExact(char *debug_flag, SRuleStruct *rulestruct, 
  int rulecount, SRuleStruct &srule);

int InitRateStruct(char *debug_flag, RATESTRUCT **ratestruct, int &ratecount, char *tablename);
int FreeRateStruct(char *debug_flag, RATESTRUCT **ratestruct);
int SearchRate(char *debug_flag, RATESTRUCT* ratestruct, int ratecount, RATESTRUCT &srate);
int SearchRate(char *debug_flag, RATESTRUCT &srate, char *tablename);
int SearchRate(char *debug_flag, RATESTRUCT* ratestruct, int ratecount, RATESTRUCT srate, RATESTRUCT **rslast, int &matchcount);
int CompareRate(char *debug_flag, RATESTRUCT* cur_rate, RATESTRUCT &srate);
int CompareRateTime(char *debug_flag, RATESTRUCT* cur_rate, char *truetime);
int CompareRateTime(char *debug_flag, RATESTRUCT* cur_rate, char *truetime, char *donetime);

int InitFormulaStruct(char *debug_flag, FormulaStruct **formulastruct, int &formulacount, int &paramcount,char *tablename,char *paramtablename);
//int InitFormulaStruct(char *debug_flag, map<string,FormulaStruct> &formulastruct, int &formulacount, int &paramcount,char *tablename,char *paramtablename);

int InitAdjRateStruct(char *debug_flag, ADJRATESTRUCT **adjratestruct, int &adjratecount, char *tablename);
int FreeAdjRateStruct(char *debug_flag, ADJRATESTRUCT **adjratestruct);
int SearchAdjRate(char *debug_flag, ADJRATESTRUCT* adjratestruct, int adjratecount, ADJRATESTRUCT &srate);
int SearchAdjRate(char *debug_flag, ADJRATESTRUCT &srate, char *tablename);
int CompareAdjRate(char *debug_flag, ADJRATESTRUCT* cur_rate, ADJRATESTRUCT &srate);  
int CompareAdjRateTime(char *debug_flag, ADJRATESTRUCT* cur_rate, char *truetime, int duration);
int CompareDuration(char *debug_flag, int beginduration, int endduration, int trueduration);

int CompareTime(char *debug_flag, char *begintime, char *endtime, char *truetime);

void PrintRuleCondition(char *debug_flag, SRuleStruct rulestruct, char *szDoneTime);
void PrintRuleResult(char *debug_flag, SRuleStruct rulestruct);
int timeStrAddSecond(char *timeStr, int addSecond);
int timeGetWeek(char *timeStr);

int getCondIdx(char *Sourceid,int total,RateCondition *Rate_Condition);//����������������ֵ
int FreeFormula(char *debug_flag, FormulaStruct **forstruct,int &formulano,int &paramno);
int SearchFormula(char *debug_flag,char *formulaID,FormulaStruct *desformula,int &expno,int &paramno,char *formulatablename,char *paramtablename);
int CompareFormula(char *debug_flag, FormulaStruct* cur_formula, FormulaStruct &sformula);
int SearchFormula(char *debug_flag,int fcount,FormulaStruct *formula,FormulaStruct &desformula);
int CompareFormulaTime(char *debug_flag, FormulaStruct* cur_formula, char *truetime);

#endif
