
#ifndef __SUM_MAIN_CPP__
#define __SUM_MAIN_CPP__

//2013-08-02 ���� �����ջ��� �»��ܣ�̯�ֻ���
//2013-08-09 ʵ���»��ܣ����������в���
//2013-08-29 ����sqlд�ļ�,����ƽ̨
//2013-08-19 C_SUMTABLE_DEFINE�����ֶ�ORGSUMT_SOUR,��������������Դ�Ĳ�ѯ����,���»��ܺ��ջ��ܶ��˸�����ORGSUMT_SOUR=��ǰ����ԴID
//2013-09-06 ��������ƽ̨ ʵ��ID����+1
//2013-09-14 ������úͼ���ռ�ȿ�ͬʱʵ��
//2013-09-16 �»�������ʵ�ֱ����ֱ�ӿ��ջ��ܵ��Ƿ������(�����ǲ�ѯ�º˶��Ƿ������)
//2013-09-19 �ջ�������ƽ��У���D_BALANCE_DAYCHECK
//2013-10-25 �»����������,�ջ��ܸ������ڽ�ͳ�Ƶ�ָ���ı�,����_����
//2013-11-13 �»���,̯�ֻ������������Ҫ����,����������ʱ����Ҫ������������
//2013-11-18 �ջ��ܵ�ԭʼ��Ҳ�ӷ�������,Ҳ�����ļ����Ҳ�ǰ��������벻ͬ�ı�(��Ҫ�����ڹ�����������̫��)
//2013-12-08 �»������ӻ�������������ʱд���ܽ����,����ǰ̨��ѯԭ��,ɾ��-e��־,ȫ���������������ձ�
//2013-12-17 ̯�ֻ������ӽ����ݰ��������ܵ���ͬ�Ľ������,�ջ��������ֶβ���Ҫ,�»���,̯�ֻ���ʱ���ֶο���ʡȥ(������ָ������)
//2014-01-09 �����ӿ�����ͳ�Ƹ�ʽ����ƱĬ��ֵ����item_type=13ʱȡ�ֶ�DEFV_OR_FUNC��ֵ
//2014-01-13 �������ջ���,(����)�»���,(����)̯�ֻ����Ȳ����¼W Ȼ�����״̬Y
//2014-04-16 ���Ӷ�ADJ����Դ�Ĵ��� = ��Ϊlike(ֻ�漰���»���)
//2014-07-17 ���ӶԿɵ�������ҵ��(Ŀǰֻ�г�;)�����⴦��,ĳ��ĳ�������ļ����������¸�����,�޸��ջ���
//2014-11-26 �����ջ���ʱ�����ŷ�Ǯ���ջ���ͬ����־��,���ں�������Ϣ��,���ӶԳ�;ҵ������⴦��
//2015-01-15 ���Ӷ�������ͳ��,��;ҵ�����,�ŷ�Ǯ���ļ�����ͳ��

#include "CDsum.h"
#include "CMsum.h"
#include "CShare.h"

//int dealtype = -1;
CLog theJSLog;

bool demaon =   false ;   //�Ƿ��ǳ�פ������ջ���
bool del_flag = true;	//�Ƿ���ɾ�����ܽ�����¼������������,2013-12-11 ��Ϊtrue
int  deal_type = 0;
char source_group[10],source_id[10],date[9],currTime[15];


//�����¸�ʽ����У��,���ж��Ƿ�����
// ���� 6 yyyymm
// ���� 8 yyyymmdd
bool checkDate()
{
	int iTemp,iTemp2;
	char chTemp[10];
	memset(chTemp,0,sizeof(chTemp));
	
	int len = strlen(date);

	//У�����2000-2999
	strncpy(chTemp,date,4);
	chTemp[4] = 0;
	iTemp = atoi(chTemp);
	if(iTemp < 2000 || iTemp > 2999)
	{			
			return false;
	}
	
	//У���·�1-12
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
		//У����
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
	if(deal_type == 2)		//��
	{
		if(len != 8) return false;
		//strncpy(tmp,currTime,8);
		if(strncmp(currTime,date,8) < 0) return false;

	}
	else				   //��
	{
		if(len != 6)  return false;
		if(strncmp(currTime,date,6) < 0) return false;
	}

	return true;
}

//�������������У��
int checkArg(int argc,char** argv)
{
	//int ret = -1;
	int flow_id = -1;

	//cout<<"����������"<<argc<<endl;

	if(argc < 3)		//����̫��
	{
		cout<<"��������̫�٣�"<<argc<<endl;
		return -1 ;
	}
	
	memset(source_group,0,sizeof(source_group));
	memset(source_id,0,sizeof(source_id));
	memset(date,0,sizeof(date));

	for(int i = 1;i<argc;i++)
	{
	  if(strcmp(argv[i],"-d") == 0)			//�ջ���
	  {		
			if(deal_type) return -1;							//��ʾ�Ѿ��������������� ��-rd

			demaon = true;
			deal_type = 1;
	  }
	  else if(strcmp(argv[i],"-rd") == 0)	//�����ջ���
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
			//���ڼ�飬����deal_type�ж�
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
	  //else if(strcmp(argv[i],"-e") == 0)		//��ʾ
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
	else						//���������Ҫָ��ʱ��
	{
		if(strcmp(date,"") == 0)  return -1;
	}
	
	//if((deal_type%2) && del_flag)
	//{	
	//	theJSLog<<"-e��ʶֻ�ܳ������ػ�����"<<endi;
	//	return -1;
	//}

	//if(deal_type%2)   
	//{
		del_flag = true;		//�������Ӧ����ɾ������������
	//}

	if(deal_type > 1)
	{
		if(!checkDate()) 
		{
			theJSLog<<"ʱ��:"<<date<<" ���Ϸ�."<<endw;
			return -1;
		}
	}

	return 0;
}

//������������жϴ�������
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
			day.setDaemon(false);			//�ǳ�פ
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
		cout<<"������ʽ���ԣ�"<<endl;
		cout<<"�ջ��ܣ�			jsSummary -f[flow_id]	-d	[-g	groupID| -s sourceID]"<<endl;
		cout<<"�����ջ���		jsSummary -rd -t YYYYMMDD	[-g groupID| -s sourceID] "<<endl;
		
		cout<<"�»��ܣ�			jsSummary -m -t YYYYMM	[-g groupID| -s sourceID]"<<endl;
		cout<<"�����»��ܣ�		jsSummary -rm  -t YYYYMM	[-g groupID| -s sourceID]"<<endl;
		cout<<"̯�ֻ��ܣ�		jsSummary -f -t YYYYMM	[-g groupID| -s sourceID]"<<endl;
		cout<<"����̯�ֻ��ܣ�	jsSummary -rf   -t YYYYMM	[-g groupID| -s sourceID]"<<endl;
		
		cout<<endl;

		return -1;
	}

	dealSum(deal_type,argc,argv);

    return 0;

}



#endif
