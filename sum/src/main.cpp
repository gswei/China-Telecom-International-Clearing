
#ifndef __SUM_MAIN_CPP__
#define __SUM_MAIN_CPP__

//2013-08-02 功能 进行日汇总 月汇总，摊分汇总
//2013-08-09 实现月汇总，解析命令行参数
//2013-08-29 增加sql写文件,容灾平台
//2013-08-19 C_SUMTABLE_DEFINE新增字段ORGSUMT_SOUR,用来做汇总数据源的查询条件,比月汇总和日汇总多了个条件ORGSUMT_SOUR=当前数据源ID
//2013-09-06 增加容灾平台 实例ID依次+1
//2013-09-14 计算费用和计算占比可同时实现
//2013-09-16 月汇总条件实现变更，直接看日汇总的是否完成了(而不是查询月核对是否完成了)
//2013-09-19 日汇总增加平衡校验表D_BALANCE_DAYCHECK
//2013-10-25 月汇总条件变更,日汇总根据账期将统计到指定的表,表名_账期
//2013-11-13 月汇总,摊分汇总情况都不需要容灾,且重做汇总时不需要汇总条件满足
//2013-11-18 日汇总的原始表也从分了账期,也就是文件入库也是按照账期入不同的表(主要是由于固网的数据量太大)
//2013-12-08 月汇总增加汇总条件不满足时写汇总结果表,便于前台查询原因,删除-e标志,全部汇总情况都先清空表
//2013-12-17 摊分汇总增加将数据按条件汇总到不同的结果表中,日汇总账期字段不需要,月汇总,摊分汇总时间字段可以省去(表名已指明账期)
//2014-01-09 公共接口新增统计格式配置票默认值功能item_type=13时取字段DEFV_OR_FUNC的值
//2014-01-13 在重做日汇总,(重做)月汇总,(重做)摊分汇总先插入记录W 然后更新状态Y
//2014-04-16 增加对ADJ数据源的处理 = 改为like(只涉及日月汇总)
//2014-07-17 增加对可调整账期业务(目前只有长途)的特殊处理,某天某个账期文件可能属于下个账期,修改日汇总
//2014-11-26 增长日汇总时计算张分钱到日汇总同步日志表,便于后续出信息点,增加对长途业务的特殊处理
//2015-01-15 增加对流量的统计,长途业务除外,张分钱按文件名来统计

#include "CDsum.h"
#include "CMsum.h"
#include "CShare.h"

//int dealtype = -1;
CLog theJSLog;

bool demaon =   false ;   //是否是常驻，针对日汇总
bool del_flag = true;	//是否先删除汇总结果表记录，针对重做情况,2013-12-11 改为true
int  deal_type = 0;
char source_group[10],source_id[10],date[9],currTime[15];


//对如下格式进行校验,不判断是否闰年
// 长度 6 yyyymm
// 长度 8 yyyymmdd
bool checkDate()
{
	int iTemp,iTemp2;
	char chTemp[10];
	memset(chTemp,0,sizeof(chTemp));
	
	int len = strlen(date);

	//校验年份2000-2999
	strncpy(chTemp,date,4);
	chTemp[4] = 0;
	iTemp = atoi(chTemp);
	if(iTemp < 2000 || iTemp > 2999)
	{			
			return false;
	}
	
	//校验月份1-12
	//date += 4;
	strncpy(chTemp,date+4,2);
	chTemp[2] = 0;
	iTemp = atoi(chTemp);
	if((iTemp < 1) || (iTemp > 12))
	{
		return false;
	}
		
	if(len > 6)
	{	
		//校验日
		//date += 2;
		strncpy(chTemp, date+6, 2);
		chTemp[2] = 0;
		iTemp2 = atoi(chTemp);
		switch(iTemp)
		{
			case 1:
			case 3:
			case 5:
			case 7:
			case 8:
			case 10:
			case 12:
				 if ( (iTemp2 < 1) || (iTemp2 > 31) ) return false;
				 break;
			case 4:
			case 6:
			case 9:
			case 11:
				if ( (iTemp2 < 1) || (iTemp2 > 30) )  return false;
				break;
			default:
				if ( (iTemp2 < 1) || (iTemp2 > 29) )  return false;
	     }
		
	}
	
	//char tmp[8];
	//memset(tmp,0,sizeof(tmp));
	memset(currTime,0,sizeof(currTime));
	getCurTime(currTime);
	if(deal_type == 2)		//日
	{
		if(len != 8) return false;
		//strncpy(tmp,currTime,8);
		if(strncmp(currTime,date,8) < 0) return false;

	}
	else				   //月
	{
		if(len != 6)  return false;
		if(strncmp(currTime,date,6) < 0) return false;
	}

	return true;
}

//对输入参数进行校验
int checkArg(int argc,char** argv)
{
	//int ret = -1;
	int flow_id = -1;

	//cout<<"参数个数："<<argc<<endl;

	if(argc < 3)		//参数太少
	{
		cout<<"参数个数太少！"<<argc<<endl;
		return -1 ;
	}
	
	memset(source_group,0,sizeof(source_group));
	memset(source_id,0,sizeof(source_id));
	memset(date,0,sizeof(date));

	for(int i = 1;i<argc;i++)
	{
	  if(strcmp(argv[i],"-d") == 0)			//日汇总
	  {		
			if(deal_type) return -1;							//表示已经代表其他类型了 如-rd

			demaon = true;
			deal_type = 1;
	  }
	  else if(strcmp(argv[i],"-rd") == 0)	//重新日汇总
	  {
			if(deal_type) return -1;
		
			deal_type = 2;
	  }
	  else if(strcmp(argv[i],"-m") == 0)
	  {
			if(deal_type)	return -1;

			deal_type = 3;
	  }
	  else if(strcmp(argv[i],"-rm") == 0)
	  {
			if(deal_type)	return -1;
			
			deal_type = 4;
	  }
	  else if(strcmp(argv[i],"-f") == 0)
	  {
			if(deal_type)	return -1;

			deal_type = 5;
	  }
	  else if(strcmp(argv[i],"-rf") == 0)
	  {
			if(deal_type)	return -1;
			
			deal_type = 6;
	  }
	  else if(strcmp(argv[i],"-t") == 0)
	  {
			if(argc < (i+2))		return -1;
			
			strcpy(date,argv[i+1]);
			//日期检查，根据deal_type判断
	  }
	  else if(strcmp(argv[i],"-g") == 0)
	  {
			if(argc < (i+2))  return -1;
			strcpy(source_group,argv[i+1]);
	  }
	  else if(strcmp(argv[i],"-s") == 0)
	  {
			if(argc < (i+2))   return -1;
			strcpy(source_id,argv[i+1]);
	  }
	  else if(strncmp(argv[i],"-f",2) == 0)
	  {
			flow_id = atoi(argv[i]+2);

	  }
	  //else if(strcmp(argv[i],"-e") == 0)		//表示
	  //{
	  //		del_flag = true;	
	  //}

	}
	
	if(deal_type == 0)		return -1;
	
	if(deal_type == 1)
	{
		if(flow_id == -1)
		{
			return -1;
		}
	}
	else						//其余情况都要指定时间
	{
		if(strcmp(date,"") == 0)  return -1;
	}
	
	//if((deal_type%2) && del_flag)
	//{	
	//	theJSLog<<"-e标识只能出现在重汇总中"<<endi;
	//	return -1;
	//}

	//if(deal_type%2)   
	//{
		del_flag = true;		//汇总情况应该先删除结果表的数据
	//}

	if(deal_type > 1)
	{
		if(!checkDate()) 
		{
			theJSLog<<"时间:"<<date<<" 不合法."<<endw;
			return -1;
		}
	}

	return 0;
}

//根据输入参数判断处理类型
int dealSum(int dealtype,int argc,char** argv)
{
	if(deal_type  == 1)									
	{		
			CDsum day;
			if(!(day.init(argc,argv))) return -1;
			if(!(day.init(source_id,source_group))) return -1;			
			
			while(1)
			{
				theJSLog.reSetLog();
				day.run();
				sleep(10);
			}
	}
	else if(deal_type == 2)
	{		
			CDsum day;
			day.setDaemon(false);			//非常驻
			if(!(day.init(argc,argv))) return -1;
			if(!(day.init(source_id,source_group))) return -1;
			day.redorun(date,del_flag);
	}
	else if(deal_type == 3)	
	{		
			CMsum  month;
			if(!(month.init(argc,argv))) return -1;
			if(!(month.init(source_id,source_group,date)))  return -1;
			month.run(1,del_flag);
	}
	else if(deal_type == 4)
	{		
			CMsum  month;
			if(!(month.init(argc,argv))) return -1;
			if(!(month.init(source_id,source_group,date))) return -1;
			month.run(2,del_flag);
	}
	else if(deal_type == 5)
	{		
			CShare share;
			if(!(share.init(argc,argv))) return -1;
			if(!(share.init(source_id,source_group,date)))  return -1;
			share.run(1,del_flag);
	}
	else if(deal_type == 6)
	{		
			CShare share;
			if(!(share.init(argc,argv))) return -1;
			if(!(share.init(source_id,source_group,date)))  return -1;
			share.run(2,del_flag);
	}

	return 0;
}

int main(int argc,char** argv)
{
	cout<<"**********************************************"<<endl;
	cout<<"*    China Telecom. Telephone Network         "<<endl;
	cout<<"*    InterNational Account Settle System      "<<endl;
	cout<<"*                                             "<<endl;
	cout<<"*           jsSummary						 "<<endl;
	cout<<"*           sys.GJZW.Version 1.0	             "<<endl;
	cout<<"*     created time :      2013-08-02 by  hed	 "<<endl;
	cout<<"*     lase update time :  2015-01-15 by  hed	 "<<endl;
	cout<<"********************************************* "<<endl;
	
	if(checkArg(argc,argv))
	{
		cout<<endl;
		cout<<"参数格式不对："<<endl;
		cout<<"日汇总：			jsSummary -f[flow_id]	-d	[-g	groupID| -s sourceID]"<<endl;
		cout<<"重新日汇总		jsSummary -rd -t YYYYMMDD	[-g groupID| -s sourceID] "<<endl;
		
		cout<<"月汇总：			jsSummary -m -t YYYYMM	[-g groupID| -s sourceID]"<<endl;
		cout<<"重新月汇总：		jsSummary -rm  -t YYYYMM	[-g groupID| -s sourceID]"<<endl;
		cout<<"摊分汇总：		jsSummary -f -t YYYYMM	[-g groupID| -s sourceID]"<<endl;
		cout<<"重新摊分汇总：	jsSummary -rf   -t YYYYMM	[-g groupID| -s sourceID]"<<endl;
		
		cout<<endl;

		return -1;
	}

	dealSum(deal_type,argc,argv);

    return 0;

}



#endif
