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
const int CALCFEE_RULE_LEN_MAX             =   1024;  //批价规则名称/值拼接后的最大长度
const int CALCFEE_PLUGIN_VERSION_NO  		  =   0;	   //插件版本号
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

//20130709 添加公式参数 结构体
struct FormulaParam {
	char szParamName[64] ;            //对象名称
  	char szParamType[2];             //对象类型
  	char szParamValue[1024];         //对象值
};

//20130709 添加公式参数 结构体
struct FormulaExp {
	int index_id;                          //公式序号
  	char szFormula[1024];              //具体公式表达式
  	char szSegmentName[64];          //字段名
  	char szStartTime[15];              //生效时间
  	char szEndTime[15];				//失效时间
  	int output_type;                   //0表示原样输出（目前为两位小数），1  表示向上取整，
                                    //    2表示向下取整，3表示保留固定位数小数
    int decimal_num;              //小数点的位数
};

//20130709 添加公式ID 结构体
struct FormulaStruct {
	char szFormulaID[16];						//公式ID
	//int szFormulaID;
    FormulaExp *szFormulaExp;    //公式表达式
  	FormulaParam *szFormulaParam; // 公式参数
  	int expno;    //一个公式ID 对应的公式总数
  	int paramno; //公式ID 对应的参数数量
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
    //添加摊分相关字段
    char cfgid[4];   //批价规则ID
    char source_id[16];
    char settle_month[16]; //摊分任务对应的帐务月字段
	char intype[2];   //入口类型F 为文件，T为数据库
	char outtype[2]; //出口类型F 为文件，T为数据库
	char invalue[256];  //入口路径
	char outvalue[256] ;  //出口路径
	char intxt_type[10]; //入口格式
	char outtxt_type[10] ; //出口格式	
};

struct SParameter
{
  char szSourceGroup[6];            //数据源分组
  char ServiceID[6];								//服务ID
  char szSourceId[6];
  char szFileName[512];
  char szSourcePath[512];
  char szInrcdType;                 //输入文件记录类型
  char szOutrcdType;                //输出文件记录类型
  	char szInputFiletypeId[6];        //输入文件类型
  char szOutputFiletypeId[6];       //输出文件类型
  char szDebugFlag[50];             //是否输出运行日志
  char szCheckName[20];             //信息点日志对象名
  bool bUseRuleMatch;								//是否使用精确匹配
  bool bUseMultiFind;								//是否使用两边查找
  bool bUseDefRateRule;
  //CRuleExact *rule_exact;						//精确规则类
  CRuleBlur *rule_blur;							//模糊规则类
  CRateGroup *rate_group;						//费率组类
  CChargeType *charge_type;					//计费类型类
  SRuleStruct *rsblur, rstmp,*defblur;				//
  RATESTRUCT *ratestruct;						//
  ADJRATESTRUCT *adjratestruct;  		//
  //char szRuleExactFile[256];				//精确规则文件名
  //char szRuleExactidx[256];					//精确规则共享内存索引文件名
 // char szRuleExactFileBak[256];			//精确规则文件名(备份)
 // int iRuleExpirationDate;					//精确规则多少天不使用则认为失效
  int iBlurCount;										//模糊规则数量
  int iBlurMax;											//模糊规则最大值
  int iDefBlurCount;										//模糊规则数量
  int iDefBlurMax;											//模糊规则最大值
  int iRateCount;										//费率规则数量
  int iAdjRateCount;								//扩展费率规则数量
  int iRuleFieldCount;							//规则字段个数
  int *pRuleFieldIndex;							//批价规则字段来自话单中哪些字段
  int *pRuleFieldIsNull;						//批价规则字段来源的话单字段值可否为空
  int *pRuleFieldMatchMode;					//批价规则字段匹配模式
  int  iTimeIndex;            	    //用于条件判断的日期/时间字段序号
  char szRateConfigId[6];						//批价配置ID
  char szFormulaConfigId[6];                  //批价公式ID 
  SRateConfig rate_config;					//批价配置信息 
  char szRuleFieldItem[RATE_MAX_RULEITEM_NUM][RATE_MAX_RULEITEM_LEN];   	//拼接前的字段名称/值字符串
  char szRuleFieldName[CALCFEE_RULE_LEN_MAX];															//批价规则字段名称拼接后的字符串
  char szRuleFieldCont[CALCFEE_RULE_LEN_MAX];															//批价规则字段值拼接后的字符串
  char szRuleFieldMatchMode[CALCFEE_RULE_LEN_MAX];												//批价规则字段匹配模式拼接后的字符串
  int iOutMode;											//输出模式（0——不分单，1——分单）
  set<int> setColumn;               //存放所有需要批价填写的字段的序号
  char szPluginFilename[256];				//存放插件文件名
  //ProcessRec_Mgr mgr;		//插件类
  C_Compile m_Compile;/*动态编译解释器*/
  //为实现配置是否批价条件表达式添加以下变量  liuw 2007-7-20
  int ConditionNum;
  RateCondition Rate_Condition[1024];
  int iErrorValue;
	char szResult[255];
	const char *theResult;
	
	int iRateFlag;
	
	int MemVersion;//共享内存版本

	STparam szTparam;
	int iFormulacount;     //公式总数量
	int iParamcount;      //公式对应对象数量
	//FormulaStruct *szFormula;  // 公式描述
	//map<string,FormulaStruct> szformulastruct;
	FormulaStruct *szformulastruct;
	FormulaParam szFormulaParam ; //公式参数
	Formula *p_formula;  //公式操作类
	char ratecycle[8];
	//map<string , double> formula_value; //T 类型，需要从表格中查数据的map
  
  //DBConnection conn;//数据库连接
  
  
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

int getCondIdx(char *Sourceid,int total,RateCondition *Rate_Condition);//查找批价条件索引值
int FreeFormula(char *debug_flag, FormulaStruct **forstruct,int &formulano,int &paramno);
int SearchFormula(char *debug_flag,char *formulaID,FormulaStruct *desformula,int &expno,int &paramno,char *formulatablename,char *paramtablename);
int CompareFormula(char *debug_flag, FormulaStruct* cur_formula, FormulaStruct &sformula);
int SearchFormula(char *debug_flag,int fcount,FormulaStruct *formula,FormulaStruct &desformula);
int CompareFormulaTime(char *debug_flag, FormulaStruct* cur_formula, char *truetime);

#endif
