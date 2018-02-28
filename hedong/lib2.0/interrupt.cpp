/****************************************************************
filename: dump.cpp
created by: wulei
create date: 2005-01-07
version: 1.0.0             
update lists : 
          2005-01-13 : 增加捕获所有可能的中断信号；
                       使用张郭强的纪录信息点日志的函数进行纪录
          2006-04-21 : 周亚军	在Acc_Interrupt函数中调用AllowInterrupt,防止重新发出的中断信号被程序自身反复捕获 
          2006-05-17 : 周亚军	HP下,对SIGINT信号,前一种方法不起作用,改为使用exit(0)退出
description:  程序出现异常core dump时，完成捕获信号纪录信息功能
*****************************************************************/
#include "interrupt.h"
#include "CF_InfoPoint.h"

char INTERRUPT_Infopoint_id[21];
char INTERRUPT_CheckName[10];
bool INTERRUPT_InterruptFlag=true;

/*
Function Name		: Interrupt
Description		    : 中断处理这个函数是用户自定义的信号接管函数，当某些信号发生的时候调用此函数
Input Parameters	:
                    Signo ---   中断号
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
Description		    : 设定纪录信息点日至需要的信息点id和Checker_name;并关中断，指定哪些信号使用用户定义的函数Acc_Interrupt（int Signo）进行捕捉，并置	InterruptFlag = false。
Input Parameters	: 信息点id和Checker_name
Returns			    : none
*/ 
void DenyInterrupt(char *pInfopoint_id, char *pCheckName)
{
        memcpy(INTERRUPT_Infopoint_id,pInfopoint_id,strlen(pInfopoint_id));
        memcpy(INTERRUPT_CheckName,pCheckName,strlen(pCheckName));
	 INTERRUPT_InterruptFlag = false;
	 
		  signal(SIGABRT,Acc_Interrupt);   //SIGABRT  调用a b o r t函数时产生此信号。进程异常终止.
			//signal(SIGALRM,Acc_Interrupt); //超过用a l a r m函数设置的时间时产生此信号。
			signal(SIGBUS,Acc_Interrupt);    //指示一个实现定义的硬件故障
		#ifndef _LINUXOS
			signal(SIGEMT,Acc_Interrupt);    //指示一个实现定义的硬件故障
		#endif
			signal(SIGFPE,Acc_Interrupt);    //此信号表示一个算术运算异常，例如除以0，浮点溢出等
			signal(SIGILL,Acc_Interrupt);    //此信号指示进程已执行一条非法硬件指令
			signal(SIGINT,Acc_Interrupt);    //当用户按中断键（一般采用D E L E T E或C t r l - C）时，终端驱动程序产生此信号并送至前台进程组中的每一个进程
			signal(SIGIOT,Acc_Interrupt);    //这指示一个实现定义的硬件故障
			signal(SIGQUIT,Acc_Interrupt);   //当用户在终端上按退出键（一般采用C t r l - \）时，产生此信号，并送至前台进程组中的所有进程
			signal(SIGSEGV,Acc_Interrupt);   //指示进程进行了一次无效的存储访问。
			signal(SIGSYS,Acc_Interrupt);    //指示一个无效的系统调用。由于某种未知原因，进程执行了一条系统调用指令，但其指示系统调用类型的参数却是无效的
			signal(SIGTERM,Acc_Interrupt);   //这是由kill(1)命令发送的系统默认终止信号
			signal(SIGTRAP,Acc_Interrupt);   //指示一个实现定义的硬件故障
			signal(SIGXCPU,Acc_Interrupt);   //如果进程超过了其软CPU时间限制，则产生此信号(setrlimit)
			signal(SIGXFSZ,Acc_Interrupt);   //如果进程超过了其软文件长度限制(setrlimit)
		
}

/*
Function Name		: Allow__Interrupt
Description		    : 开中断，指定哪些信号使用系统默认的动做进行接管，并置	InterruptFlag = true。
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
Description		    : 取中断标志InterruptFlag的值。
Input Parameters	: none
Returns			    : true  -表示信号使用系统默认的动做进行接管；
		              false -表示信号使用用户定义函数Acc_Interrupt（int Signo）进行捕捉。

*/ 
bool GetInterruptFlag(void)
{
	return INTERRUPT_InterruptFlag;
}


