/************************************************************
*CF_CRecStat.h
*20050920 by tanj
*本接口为预处理阶段所有模块所调用，
*其主要功能两个
*1.统计一个文件中话单的最早最晚通话开始时间
*2.统计该文件的异常单（错单、重单、超频超短话单）比例
*注：目前功能二只有去重和超频超短话单合并模块需要使用
* 20050921 updated by yangzx
************************************************************/

#ifndef _CF_CRECSTAT_H_
#define _CF_CRECSTAT_H_

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>


#include "config.h"
#include "COracleDB.h"
#include "CFmt_Change.h"
#include "interpreter.h"
#include "CF_CError.h"
#include "Common.h"

const int  STAT_NO_ERR   =  0;
const int  STAT_ERR      =  1;
class CF_CRecStat
{
public:
  CF_CRecStat();
  
  //初始化函数，默认参数iStatErrFlag = STAT_NO_ERR 为不统计异常单比例
  //如果把输入参数iStatErrFlag 设置成STAT_ERR 则为统计异常单比例，需从数据库中的pipe_env表里取异常单比例阀值COMMON_ERROR_RATE
  //从果不能从数据库中取到异常单比例阀值,则把参数iStatErrFlag 置为 STAT_NO_ERR，不统计异常单比例
  void Init(char *szPipeId,int szProcessId,const int iStatErrFlag = STAT_NO_ERR);
  
 
  //判断读入的一条话单记录是否需要使用本接口的模块处理返回值为1时代表需要处理，返回值为0代表不需要处理，返回值为-1代表判定是否需要处理的表达式设置错误
  int isNeedDeal(CFmt_Change &rec,Interpreter &theCompile);
  
  
  
  //本函数对话单中的最早，最晚通话开始时间做统计
  //如果在Init()函数中调用的统计参数iStatErrFlag的值是STAT_ERR,还需要读话单的各个异常单字段，看本话单是否是异常单,返回值为0，代表正常单，返回值为1,代表异常单,-1代表判断异常单的表达式设置错误
  int StatRec(CFmt_Change &rec,Interpreter &theCompile);
  
  //本函数只对话单中的最早，最晚通话开始时间作统计
  int StatRec(CFmt_Change &rec);
  
  //在一个文件处理完毕之后，获取本模块处理的异常单数
  int getErrNo(void);
  
 //执行完addErrNo函数之后，返回该文件错误话单占话单总量的百分比
  int getErrorPercent(void);
 
 //isErrFile函数返回当前文件是否是错误文件,返回true，是错误文件；返回false不是错误文件
  bool isErrFile(void);
  
  //整个文件统计完毕后，返回该文件中最早的通话起始时间
  void getEarliestTime(char *szEarliestTime);

  //整个文件统计完毕后，返回该文件中最晚的通话起始时间
  void getLatestTime(char *szLatestTime);

 
  
private:
	char earliest_time[15];
	char latest_time[15];
  char cpCdrBegin[21];
  char sql[1000];
  char express_stat_ab_bill[256];
  char express_ifdeal_ab_bill[256];
  char PipeId[9];
  int ProcessId; 	
  int stat_err_flag;
	int err_percent;
	int default_err_percent;
	int err_count;
  int other_count;
  int total_err_count;
};

#endif 


