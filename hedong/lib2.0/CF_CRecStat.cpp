/************************************************
*CF_CRecStat.cpp
*20050920 createde by yangzx
************************************************/
#include"CF_CRecStat.h"

CF_CRecStat::CF_CRecStat()
{
	
	memset(cpCdrBegin,0,sizeof(cpCdrBegin));
	sprintf(earliest_time,"%s","98765432109876");
	sprintf(latest_time,"%s","01234567890123");
	memset(sql,0,sizeof(sql));
	memset(express_stat_ab_bill,0,sizeof(express_stat_ab_bill));
	memset(express_ifdeal_ab_bill,0,sizeof(express_ifdeal_ab_bill));
	memset(PipeId,0,sizeof(PipeId));
	ProcessId=-1;
	stat_err_flag=STAT_NO_ERR;
	err_count=0;
	other_count=0;
	total_err_count=0;
	default_err_percent=101;
	err_percent=0;
}

void CF_CRecStat::Init(char *szPipeId,int szProcessId,const int iStatErrFlag)
{   
    CBindSQL ds(DBConn);
    char temp[256];
    memset(temp,0,sizeof(temp));
    char pchVarName[60];
    strcpy(PipeId,szPipeId);
    ProcessId=szProcessId;
    stat_err_flag=iStatErrFlag;
    char err_rate[101];
    strcpy(err_rate,"200");
    if(stat_err_flag)
    {
    	strcpy(pchVarName,"COMMON_ERROR_RATE");
    	getEnvFromDB(DBConn,szPipeId, szProcessId, pchVarName, err_rate);
    	
    	default_err_percent=atoi(err_rate);
    	if(default_err_percent<=0||default_err_percent>100)
    	{
    		stat_err_flag=STAT_NO_ERR;
    	}
    }
    if(stat_err_flag)
    {
    	
    	strcpy(pchVarName,"STAT_ABNORMAL_BILL");
    	getEnvFromDB(DBConn,szPipeId, szProcessId, pchVarName, express_stat_ab_bill);
    	strcpy(temp,express_stat_ab_bill);
    	sprintf(express_stat_ab_bill,"\t\t%s",temp);
    	//printf("express_stat_ab_bill=%s\n",express_stat_ab_bill);
    } 
    //��ȡ��pipe_env�����õģ����ڸ�pipe�ģ�������¼��ͨ����ʼʱ����ֶ�����ͨ����ʼʱ����ֶ�����txtfile_fmt�������ڱ�pipe������ļ����͵�ͨ����ʼʱ����ֶ���һ�¡�
    strcpy(pchVarName,"COMMON_STAT_CDRBEGIN");
    getEnvFromDB(DBConn,szPipeId, szProcessId, pchVarName, cpCdrBegin);
    //��ȡpipe_env�����õģ����ڸ�pipe�ģ��ж��Ƿ���Ҫ��������¼�ı��ʽ
    strcpy(pchVarName,"IF_DEAL_ABNORMAL_BILL");
    getEnvFromDB(DBConn,szPipeId, szProcessId, pchVarName, express_ifdeal_ab_bill);
    strcpy(temp,express_ifdeal_ab_bill);
    sprintf(express_ifdeal_ab_bill,"\t\t%s",temp);
    //printf("express_ifdeal_ab_bill=%s\n",express_ifdeal_ab_bill);
    
}


int CF_CRecStat::isNeedDeal(CFmt_Change &rec,Interpreter &theCompile)
{
    if(strlen(express_ifdeal_ab_bill)!=0)
    {
    	int ErrorValue = 0;
    	const char * theResult;
    	char Result[255];
    	memset(Result,0,sizeof(Result));
    	theResult=theCompile.Operation(Result, sizeof(Result) - 1, &ErrorValue, express_ifdeal_ab_bill);
    	char errmsg[400];
    	memset(errmsg,0,sizeof(errmsg));
    	if(ErrorValue==0)
    	{
    		//printf("isneeddeal  Result=%s\n",Result);
    		if(strcmp(Result,"true")==0)
    		{//	PIPE_ENV��IF_DEAL_ABNORMAL_BILL���ʽ���ؽ��Ϊ��ʱ,��Ҫ����ģ�鴦��������
    			return 1;  
    		}
    		else
    		{
    			return 0;
    		}
    	}
    	else
    	{
    		sprintf(errmsg,"Compile or run expression =%s= defined in pipe_env where pipe_id=%s ,process_id=%d ,varname=IF_DEAL_ABNORMAL_BILL error!",express_stat_ab_bill,PipeId,ProcessId);
				throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_HIG,ErrorValue,ErrorValue,errmsg,__FILE__,__LINE__);
				return -1;
			}
		}
		return 1;
}

int CF_CRecStat::StatRec(CFmt_Change &rec,Interpreter &theCompile)
{
    //��ȡ���������ͨ����ʼʱ��
    if(strlen(cpCdrBegin)!=0)
    {//�ж��Ƿ���Ҫͳ�����������ͨ����ʼʱ��
    	char temp[15];
    	memset(temp,0,15);
    	rec.Get_Field(cpCdrBegin,temp);
    	if(strcmp(earliest_time,temp)>0)
    	{
    		strcpy(earliest_time,temp);
    		
    	}
    	if(strcmp(latest_time,temp)<0)
    	{
    		strcpy(latest_time,temp);
    		
    	}   
    }
    //�ж��Ƿ�Ϊ��
    if(stat_err_flag)
    {
    	if(strlen(express_stat_ab_bill)!=0)
    	{
    		int ErrorValue = 0;
    		const char * theResult;
    		char Result[255];
    		memset(Result,0,sizeof(Result));
    		theResult=theCompile.Operation(Result, sizeof(Result) - 1, &ErrorValue, express_stat_ab_bill);
    		char errmsg[400];
    		memset(errmsg,0,sizeof(errmsg));
    		if(ErrorValue==0)
    		{
    			//printf("Result=%s\n",Result);
    			if(strcmp(Result,"true")==0)
    			{//	PIPE_ENV��STAT_ABNORMAL_BILL���ʽ���ؽ��Ϊ��ʱ��������
    				++other_count;
    				return 0;
    			}
    			else
    			{
    				++err_count;
    				return 1;
    			}
    		}
    		else
    		{
    			sprintf(errmsg,"Compile or run expression =%s= defined in pipe_env where pipe_id=%s ,process_id=%d ,varname=STAT_ABNORMAL_BILL error!",express_stat_ab_bill,PipeId,ProcessId);
					throw CF_CError(ERR_TYPE_ELSE,ERR_LEVEL_HIG,ErrorValue,ErrorValue,errmsg,__FILE__,__LINE__);
					return -1;
				}
			}
    }
    ++other_count;//�������Ҫͳ���쳣�����ͰѶ���Ļ������뵽�������ļ�����
    return 0;
}


int CF_CRecStat::StatRec(CFmt_Change &rec)
{
    //��ȡ���������ͨ����ʼʱ��
    if(strlen(cpCdrBegin)!=0)
    {//�ж��Ƿ���Ҫͳ�����������ͨ����ʼʱ��
    	char temp[15];
    	memset(temp,0,15);
    	rec.Get_Field(cpCdrBegin,temp);
    	if(strcmp(earliest_time,temp)>0)
    	{
    		strcpy(earliest_time,temp);
    		
    	}
    	if(strcmp(latest_time,temp)<0)
    	{
    		strcpy(latest_time,temp);
    		
    	}   
    }
    return 0;    
}


int CF_CRecStat::getErrNo(void)
{
	return err_count;
}
	

int CF_CRecStat::getErrorPercent(void)
{
	
	return err_percent;
}

bool CF_CRecStat::isErrFile(void)
{
	int rate=0;
	if(err_count+other_count!=0)
	{
		rate=100*err_count/(err_count+other_count);
	}
	err_count=0;
	other_count=0;
	err_percent=rate;
	if(rate>=default_err_percent)
	{
		return true;
	}
	else
	{
		return false;
	}
}


void CF_CRecStat::getEarliestTime(char *szEarliestTime)
{
	if(strlen(cpCdrBegin)!=0)
		strcpy(szEarliestTime,earliest_time);
	//printf("szEarliestTime=%s\n",szEarliestTime);
	strcpy(earliest_time,"98765432109876");
	
}


void CF_CRecStat::getLatestTime(char *szLatestTime)
{
	if(strlen(cpCdrBegin)!=0)
		strcpy(szLatestTime,latest_time);
	//printf("szLatestTime=%s\n",szLatestTime);
	strcpy(latest_time,"01234567890123");
}




	