/****************************************************************
filename: dump.cpp
created by: wulei
create date: 2005-01-07
version: 1.0.0             
update lists : 
          2005-01-13 : ���Ӳ������п��ܵ��ж��źţ�
                       ʹ���Ź�ǿ�ļ�¼��Ϣ����־�ĺ������м�¼
          2006-04-21 : ���Ǿ�	��Acc_Interrupt�����е���AllowInterrupt,��ֹ���·������ж��źű��������������� 
          2006-05-17 : ���Ǿ�	HP��,��SIGINT�ź�,ǰһ�ַ�����������,��Ϊʹ��exit(0)�˳�
description:  ��������쳣core dumpʱ����ɲ����źż�¼��Ϣ����
*****************************************************************/
#include "interrupt.h"
#include "CF_InfoPoint.h"

char INTERRUPT_Infopoint_id[21];
char INTERRUPT_CheckName[10];
bool INTERRUPT_InterruptFlag=true;

/*
Function Name		: Interrupt
Description		    : �жϴ�������������û��Զ�����źŽӹܺ�������ĳЩ�źŷ�����ʱ����ô˺���
Input Parameters	:
                    Signo ---   �жϺ�
Returns			    : none
*/ 
void Acc_Interrupt(int Signo)
{
    char start_datetime[15];
    time_t timer;
    struct tm nowtimer;
    time( &timer );
    nowtimer = *localtime ( &timer );
    nowtimer.tm_mon++;
    memset(start_datetime, 0, sizeof(start_datetime));
    sprintf(start_datetime, "%04u%02u%02u%02u%02u%02u", nowtimer.tm_year + 1900,
    nowtimer.tm_mon, nowtimer.tm_mday, nowtimer.tm_hour,
    nowtimer.tm_min, nowtimer.tm_sec); 
    
    char Signame[10];

    if (Signo == SIGABRT)
                sprintf(Signame,"SIGABRT");
	else if (Signo == SIGBUS)
                sprintf(Signame,"SIGBUS");
#ifndef _LINUXOS
	else if (Signo == SIGEMT)
                sprintf(Signame,"SIGEMT"); 
#endif
  	else if (Signo == SIGFPE)
                sprintf(Signame,"SIGFPE");               
	else if (Signo == SIGILL)
                sprintf(Signame,"SIGILL");
	else if (Signo == SIGINT)
                sprintf(Signame,"SIGINT");               
	else if (Signo == SIGIOT)
                sprintf(Signame,"SIGIOT");
	else if (Signo == SIGQUIT)
                sprintf(Signame,"SIGQUIT");               
	else if (Signo == SIGSEGV)
                sprintf(Signame,"SIGSEGV");
	else if (Signo == SIGSYS)
                sprintf(Signame,"SIGSYS");               
	else if (Signo == SIGTERM)
                sprintf(Signame,"SIGTERM");
	else if (Signo == SIGTRAP)
                sprintf(Signame,"SIGTRAP");  
	else if (Signo == SIGXCPU)
                sprintf(Signame,"SIGXCPU");
	else if (Signo == SIGXFSZ)
                sprintf(Signame,"SIGXFSZ");                               
    
    printf("receive the Signal %s , exit!\n",Signame);
        
    char now_datetime[15];
    time( &timer );
    nowtimer = *localtime ( &timer );
    nowtimer.tm_mon++;
    memset(now_datetime, 0, sizeof(now_datetime));
    sprintf(now_datetime, "%04u%02u%02u%02u%02u%02u", nowtimer.tm_year + 1900,
    nowtimer.tm_mon, nowtimer.tm_mday, nowtimer.tm_hour,
    nowtimer.tm_min, nowtimer.tm_sec); 
    char now_date[9];
    memset(now_date, 0, sizeof(now_date));
    sprintf(now_date, "%04u%02u%02u", nowtimer.tm_year + 1900,
    nowtimer.tm_mon, nowtimer.tm_mday); 
                       
    msglog(INTERRUPT_Infopoint_id,INTERRUPT_CheckName,INFO_VALUE_FALSE,Signame,now_date,start_datetime,now_datetime);

	AllowInterrupt();
	if(Signo == SIGINT)
		exit(0);
		
   	kill(getpid(),Signo);
   	//raise(Signo);
    
    //abort();
}


/*
Function Name		: Deny__Interrupt
Description		    : �趨��¼��Ϣ��������Ҫ����Ϣ��id��Checker_name;�����жϣ�ָ����Щ�ź�ʹ���û�����ĺ���Acc_Interrupt��int Signo�����в�׽������	InterruptFlag = false��
Input Parameters	: ��Ϣ��id��Checker_name
Returns			    : none
*/ 
void DenyInterrupt(char *pInfopoint_id, char *pCheckName)
{
        memcpy(INTERRUPT_Infopoint_id,pInfopoint_id,strlen(pInfopoint_id));
        memcpy(INTERRUPT_CheckName,pCheckName,strlen(pCheckName));
	 INTERRUPT_InterruptFlag = false;
	 
		  signal(SIGABRT,Acc_Interrupt);   //SIGABRT  ����a b o r t����ʱ�������źš������쳣��ֹ.
			//signal(SIGALRM,Acc_Interrupt); //������a l a r m�������õ�ʱ��ʱ�������źš�
			signal(SIGBUS,Acc_Interrupt);    //ָʾһ��ʵ�ֶ����Ӳ������
		#ifndef _LINUXOS
			signal(SIGEMT,Acc_Interrupt);    //ָʾһ��ʵ�ֶ����Ӳ������
		#endif
			signal(SIGFPE,Acc_Interrupt);    //���źű�ʾһ�����������쳣���������0�����������
			signal(SIGILL,Acc_Interrupt);    //���ź�ָʾ������ִ��һ���Ƿ�Ӳ��ָ��
			signal(SIGINT,Acc_Interrupt);    //���û����жϼ���һ�����D E L E T E��C t r l - C��ʱ���ն���������������źŲ�����ǰ̨�������е�ÿһ������
			signal(SIGIOT,Acc_Interrupt);    //��ָʾһ��ʵ�ֶ����Ӳ������
			signal(SIGQUIT,Acc_Interrupt);   //���û����ն��ϰ��˳�����һ�����C t r l - \��ʱ���������źţ�������ǰ̨�������е����н���
			signal(SIGSEGV,Acc_Interrupt);   //ָʾ���̽�����һ����Ч�Ĵ洢���ʡ�
			signal(SIGSYS,Acc_Interrupt);    //ָʾһ����Ч��ϵͳ���á�����ĳ��δ֪ԭ�򣬽���ִ����һ��ϵͳ����ָ�����ָʾϵͳ�������͵Ĳ���ȴ����Ч��
			signal(SIGTERM,Acc_Interrupt);   //������kill(1)����͵�ϵͳĬ����ֹ�ź�
			signal(SIGTRAP,Acc_Interrupt);   //ָʾһ��ʵ�ֶ����Ӳ������
			signal(SIGXCPU,Acc_Interrupt);   //������̳���������CPUʱ�����ƣ���������ź�(setrlimit)
			signal(SIGXFSZ,Acc_Interrupt);   //������̳����������ļ���������(setrlimit)
		
}

/*
Function Name		: Allow__Interrupt
Description		    : ���жϣ�ָ����Щ�ź�ʹ��ϵͳĬ�ϵĶ������нӹܣ�����	InterruptFlag = true��
Input Parameters	: none
Returns			    : none
*/ 
void AllowInterrupt(void)
{
    INTERRUPT_InterruptFlag = true;
    
    signal(SIGABRT,SIG_DFL);
	signal(SIGINT,SIG_DFL);
	signal(SIGQUIT,SIG_DFL);
	signal(SIGTERM,SIG_DFL);
	signal(SIGHUP,SIG_DFL);
	signal(SIGFPE,SIG_DFL);
	signal(SIGSEGV,SIG_DFL);
}

/*
Function Name		: Get__InterruptFlag
Description		    : ȡ�жϱ�־InterruptFlag��ֵ��
Input Parameters	: none
Returns			    : true  -��ʾ�ź�ʹ��ϵͳĬ�ϵĶ������нӹܣ�
		              false -��ʾ�ź�ʹ���û����庯��Acc_Interrupt��int Signo�����в�׽��

*/ 
bool GetInterruptFlag(void)
{
	return INTERRUPT_InterruptFlag;
}


