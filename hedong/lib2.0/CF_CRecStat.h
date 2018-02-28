/************************************************************
*CF_CRecStat.h
*20050920 by tanj
*���ӿ�ΪԤ����׶�����ģ�������ã�
*����Ҫ��������
*1.ͳ��һ���ļ��л�������������ͨ����ʼʱ��
*2.ͳ�Ƹ��ļ����쳣���������ص�����Ƶ���̻���������
*ע��Ŀǰ���ܶ�ֻ��ȥ�غͳ�Ƶ���̻����ϲ�ģ����Ҫʹ��
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
  
  //��ʼ��������Ĭ�ϲ���iStatErrFlag = STAT_NO_ERR Ϊ��ͳ���쳣������
  //������������iStatErrFlag ���ó�STAT_ERR ��Ϊͳ���쳣��������������ݿ��е�pipe_env����ȡ�쳣��������ֵCOMMON_ERROR_RATE
  //�ӹ����ܴ����ݿ���ȡ���쳣��������ֵ,��Ѳ���iStatErrFlag ��Ϊ STAT_NO_ERR����ͳ���쳣������
  void Init(char *szPipeId,int szProcessId,const int iStatErrFlag = STAT_NO_ERR);
  
 
  //�ж϶����һ��������¼�Ƿ���Ҫʹ�ñ��ӿڵ�ģ�鴦����ֵΪ1ʱ������Ҫ��������ֵΪ0������Ҫ��������ֵΪ-1�����ж��Ƿ���Ҫ����ı��ʽ���ô���
  int isNeedDeal(CFmt_Change &rec,Interpreter &theCompile);
  
  
  
  //�������Ի����е����磬����ͨ����ʼʱ����ͳ��
  //�����Init()�����е��õ�ͳ�Ʋ���iStatErrFlag��ֵ��STAT_ERR,����Ҫ�������ĸ����쳣���ֶΣ����������Ƿ����쳣��,����ֵΪ0������������������ֵΪ1,�����쳣��,-1�����ж��쳣���ı��ʽ���ô���
  int StatRec(CFmt_Change &rec,Interpreter &theCompile);
  
  //������ֻ�Ի����е����磬����ͨ����ʼʱ����ͳ��
  int StatRec(CFmt_Change &rec);
  
  //��һ���ļ��������֮�󣬻�ȡ��ģ�鴦����쳣����
  int getErrNo(void);
  
 //ִ����addErrNo����֮�󣬷��ظ��ļ����󻰵�ռ���������İٷֱ�
  int getErrorPercent(void);
 
 //isErrFile�������ص�ǰ�ļ��Ƿ��Ǵ����ļ�,����true���Ǵ����ļ�������false���Ǵ����ļ�
  bool isErrFile(void);
  
  //�����ļ�ͳ����Ϻ󣬷��ظ��ļ��������ͨ����ʼʱ��
  void getEarliestTime(char *szEarliestTime);

  //�����ļ�ͳ����Ϻ󣬷��ظ��ļ��������ͨ����ʼʱ��
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


