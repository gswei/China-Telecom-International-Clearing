/*********************************************************************
*	 Copyright (c)	2010. �㶫��Ѹ�Ƽ����޹�˾
*      All Rights Reserved.
*  ����Ƿ񵽴���Ϣ������ʱ��
*	��ǰ�汾:	3.0.0
*	����:	    ���
*	�������:	2010-07-12
*
*********************************************************************/

#include "CTimeCheck.h"



bool CTimeCheck::checkTime()   //����Ƿ񵽴�ʱ��㣬��������
{
  string nowTime,strToday,sztmp,strNxtDay,strDay,strLastday;  
  string strComTime_Day,strComTime_HH,strComTime_MM;
  int HH,MM;
  char szTemp[20];
  char szTemp1[20];
  char szTemp2[20];
  
  memset(szTemp,0,sizeof(szTemp));
  memset(szTemp1,0,sizeof(szTemp1));
  memset(szTemp2,0,sizeof(szTemp2));
  
  strComTime_Day=strCmpTime.substr(0,8);
  strComTime_HH=strCmpTime.substr(8,2);
  strComTime_MM=strCmpTime.substr(10,2);
  
  HH=atoi(strComTime_HH.c_str());
  MM=atoi(strComTime_MM.c_str());
  
  addDays(1,strComTime_Day.c_str(),szTemp1);  
  strNxtDay=szTemp1;
  strNxtDay=strNxtDay.substr(0,8);
  
  addDays(-1,strComTime_Day.c_str(),szTemp2);
  strLastday=szTemp2; 
  strLastday=strLastday.substr(0,8);
  
  getCurTime(szTemp); 
  nowTime=szTemp;
  
  strDay=strTime.substr(1,2);
 
  
  
  if (nowTime.substr(0,8)!=m_strLastRestTime.substr(0,8)) //�жϿ����,�����־
  {
    resetTime(0);
    getCurTime(szTemp);
    m_strLastRestTime=szTemp;  
    return false;
  }

  strPreFrom=strFrom;
  strPreTo=strTo;
  
  if (strDay!="00")//���µ�
  {
    if (bProvided==false&&nowTime.compare(strCmpTime)>0) 
    {
      bProvided=true;
      return true;
    }
  }
  else//���յ�
  {
    if (bProvided==false&&nowTime.compare(strCmpTime)>0)
    {
      bProvided=true;
      if (qcType=='E')//�����ʽ
      {
        qcType='E';
        bProvided=false;        
        
	   if(iDelay==5)       //2014-03-16
	   {
		if(MM<5)
	        {
	      	  strFrom=strComTime_Day+strComTime_HH.substr(0,2)+"0000";
              strTo=strComTime_Day+strComTime_HH.substr(0,2)+"0500";	
		      }
		else if(MM < 55)
			{
			  memset(szTemp,0,sizeof(szTemp));
			  sprintf(szTemp,"%02d00",MM/5*5);
	      	  strFrom=strComTime_Day+strComTime_HH.substr(0,2)+szTemp;

			   memset(szTemp,0,sizeof(szTemp));
			   sprintf(szTemp,"%02d00",(MM/5+1)*5);
               strTo=strComTime_Day+strComTime_HH.substr(0,2)+szTemp;	
			   
		      }
		else
			{
			  memset(szTemp,0,sizeof(szTemp));
			  sprintf(szTemp,"%02d00",MM/5*5);
			  strFrom=strComTime_Day+strComTime_HH.substr(0,2)+szTemp;

			  memset(szTemp,0,sizeof(szTemp));
			  sprintf(szTemp,"%02d0000",HH+1);
              strTo=strComTime_Day+szTemp;
			  
			  if(HH == 23)    //����
			  {
				strTo=strNxtDay+"000000";
			  }

			}
		  strCmpTime = strTo;
	    }
        else if(iDelay==15)
        {
        	if(MM<15)
	        {
	      	  strFrom=strComTime_Day+strComTime_HH.substr(0,2)+"0000";
            strTo=strComTime_Day+strComTime_HH.substr(0,2)+"1500";	
		      }
		      else if(MM>=15 && MM<30)
		      {
		 	      strFrom=strComTime_Day+strComTime_HH+"1500";
            strTo=strComTime_Day+strComTime_HH+"3000";
		      }
		      else if(MM>=30 && MM<45)
		      {
		 	       strFrom=strComTime_Day+strComTime_HH+"3000";
             strTo=strComTime_Day+strComTime_HH+"4500";
		      }
		      else if(MM>=45)
		      {
		 	       strFrom=strComTime_Day+strComTime_HH+"3000";
             strTo=strComTime_Day+strComTime_HH+"4500";
             
             if(HH==23)
		 	       {
		 	 	       strFrom=strComTime_Day+"234500";
               strTo=strNxtDay+"000000";		 	 	
		 	       } 
		 	       else
		 	       {
		 	       	  if (HH<9)
		 	     	    {
		 	     	  	  sprintf(szTemp,"0%d",HH+1);
		 	     	    }
		 	     	    else
		 	     	    {
		 	     	  	  sprintf(szTemp,"%d",HH+1);
		 	     	    }
		 	 	        
		 	 	        sztmp=szTemp;	 	 	
		 	          strFrom=strComTime_Day+strComTime_HH.substr(0,2)+"4500";	 	 	      
		 	 	        strTo=strComTime_Day+sztmp+"0000";
		 	 	
		 	       }	 
		 	      
		       
		      }
		      strCmpTime = strTo;
	      }
	      else if(iDelay==30)
	      {
	        if(MM<30)
		      {
		 	 	    strFrom=strComTime_Day+strComTime_HH+"0000";
            strTo=strComTime_Day+strComTime_HH+"3000";		 	 	
		 	      
		      }
		      else if(MM>=30 )
		      {
		 	       
            if(HH==23)
		 	       {
		 	 	       strFrom=strComTime_Day+"230000";
               strTo=strNxtDay+"000000";		 	 	
		 	       } 
		 	       else
		 	       {
		 	         	if (HH<9)
		 	     	    {
		 	     	  	  sprintf(szTemp,"0%d",HH+1);
		 	     	    }
		 	     	    else
		 	     	    {
		 	     	  	  sprintf(szTemp,"%d",HH+1);
		 	     	    }
		 	 	       
		 	 	        sztmp=	szTemp;	 	 	
		 	          strFrom=strComTime_Day+strComTime_HH.substr(0,2)+"3000";	 	 	      
		 	 	        strTo=strComTime_Day+sztmp+"0000";
		 	 	
		 	       }	
		      }
		       strCmpTime = strTo;
	       }
	       else if(iDelay==60)
	       {
	  	     if(HH==23)
		 	     {
		 	 	      strFrom=strComTime_Day+"230000";
              strTo=strNxtDay+"000000";		 	 	
		 	     } 
		 	     else
		 	     {
		 	     	  if (HH<9)
		 	     	  {
		 	     	  	sprintf(szTemp,"0%d",HH+1);
		 	     	  }
		 	     	  else
		 	     	  {
		 	     	  	sprintf(szTemp,"%d",HH+1);
		 	     	  }
		 	 	      
		 	 	      sztmp=	szTemp;	 	 	
		 	        strFrom=strComTime_Day+strComTime_HH.substr(0,2)+"0000";	 	 	      
		 	 	      strTo=strComTime_Day+sztmp+"0000";
		 	 	
		 	     }	
         strCmpTime = strTo;
        }
      }
      else if(qcType == 'D')//ÿ��ִ��һ�ε�
      {
        bProvided=true;  
        strPreFrom=strLastday+"000000";
        strPreTo=strLastday+"235959";
        
        strFrom=strComTime_Day+"000000";
        strTo=strNxtDay+strTime.substr(3,6);
        
        strCmpTime = strTo;    
      }	
      else
      	;
      return true;
    }
  }
  

  return false;
}


void CTimeCheck::getFromTo(char *from,char *to)
{
	string nowTime,strLastday,strToday,strLastMonth,strCurMonth,strtmp;
	char sztmp[20];
	getCurTime(sztmp);
	nowTime=sztmp;
  
  getCurDate(sztmp);
  strToday=sztmp;
  
  GetLastMonth(strLastMonth);
  
  strCurMonth=strToday.substr(0,6);
  
  memset(sztmp,0,sizeof(sztmp));
  addDays(-1,strToday.c_str(),sztmp);
  strLastday=sztmp;
  
	if (strTime[0]=='E')//�����ʽ//��ȡ��ʼ����ֹʱ��
  {
  	strcpy(from,strPreFrom.c_str());
    strcpy(to,strPreTo.c_str());
  }
  else if (strTime[0]=='D')
  {
  	//SQL��������ȡ���ݵ�ʱ���
    strPreFrom=strToday+"000000";
    strPreTo=strToday+"235959";
    
  	strcpy(to,strPreTo.c_str());
    strcpy(from,strPreFrom.c_str());
  }
  else if (strTime[0]=='M')
  {
  	strcpy(from,strLastMonth.c_str());
    strcpy(to,strCurMonth.c_str());
  }
  expTrace("Y", __FILE__, __LINE__, "next run time:%s",strCmpTime); 
    
}

void CTimeCheck::resetTime(int iCheck)		//����ʱ��,���ݵ�ǰʱ�䣬������Ϣ�㾫ȷ�Ŀ�ʼʱ��strFrom�ͽ���ʱ�strTo?
{
  string nowTime,strLastday,strToday,strLastMonth,strCurMonth,strNxtDay;
  string strNowTime_Day,strNowTime_HH,strNowTime_MM,strtmp;
  int HH,MM;
  char sztmp[20];
  char szTemp[20];
  
  getCurTime(sztmp);
  nowTime=sztmp;
  
  getCurDate(sztmp);
  strToday=sztmp;
  
  GetLastMonth(strLastMonth);
  
  strCurMonth=strToday.substr(0,6);
  
  memset(sztmp,0,sizeof(sztmp));
  addDays(-1,strToday.c_str(),sztmp);
  strLastday=sztmp;
  
  addDays(1,strToday.c_str(),sztmp);
  strNxtDay=sztmp;
  
  strDay=strTime.substr(1,2);
    strNowTime_Day=nowTime.substr(0,8);
    strNowTime_HH=nowTime.substr(8,2);
    strNowTime_MM=nowTime.substr(10,2);
    HH=atoi(strNowTime_HH.c_str());
    MM=atoi(strNowTime_MM.c_str());

  if (strTime[0]=='E'&&iCheck==1)//�����ʽ//��ȡ��ʼ����ֹʱ��
  {
    qcType='E';

	if(iDelay==5)              //2014-03-16 
	{
		if(MM<5)
	        {
	      	  strFrom=strNowTime_Day+strNowTime_HH.substr(0,2)+"0000";
              strTo=strNowTime_Day+strNowTime_HH.substr(0,2)+"0500";	
		      }
		else if(MM < 55)
			{
			  memset(szTemp,0,sizeof(szTemp));
			  sprintf(szTemp,"%02d00",MM/5*5);
	      	  strFrom=strNowTime_Day+strNowTime_HH.substr(0,2)+szTemp;

			   memset(szTemp,0,sizeof(szTemp));
			   sprintf(szTemp,"%02d00",(MM/5+1)*5);
               strTo=strNowTime_Day+strNowTime_HH.substr(0,2)+szTemp;	
			   
		      }
		else
			{
			  memset(szTemp,0,sizeof(szTemp));
			  sprintf(szTemp,"%02d00",MM/5*5);
			  strFrom=strNowTime_Day+strNowTime_HH.substr(0,2)+szTemp;

			  memset(szTemp,0,sizeof(szTemp));
			  sprintf(szTemp,"%02d0000",HH+1);
              strTo=strNowTime_Day+szTemp;	

			  if(HH == 23)    //����
			  {
				strTo=strNxtDay+"000000";
			  }

			}
		strCmpTime = strTo;
	}
    else if(iDelay==15)
        {
        	if(MM<15)
	        {
	      	  strFrom=strNowTime_Day+strNowTime_HH.substr(0,2)+"0000";
            strTo=strNowTime_Day+strNowTime_HH.substr(0,2)+"1500";	
		      }
		      else if(MM>=15 && MM<30)
		      {
		 	      strFrom=strNowTime_Day+strNowTime_HH+"1500";
            strTo=strNowTime_Day+strNowTime_HH+"3000";
		      }
		      else if(MM>=30 && MM<45)
		      {
		 	       strFrom=strNowTime_Day+strNowTime_HH+"3000";
             strTo=strNowTime_Day+strNowTime_HH+"4500";
		      }
		      else if(MM>=45)
		      {
		 	       strFrom=strNowTime_Day+strNowTime_HH+"3000";
             strTo=strNowTime_Day+strNowTime_HH+"4500";
             
             if(HH==23)
		 	       {
		 	 	       strFrom=strNowTime_Day+"234500";
               strTo=strNxtDay+"000000";		 	 	
		 	       } 
		 	       else
		 	       {
		 	       	  if (HH<9)
		 	     	    {
		 	     	  	  sprintf(szTemp,"0%d",HH+1);
		 	     	    }
		 	     	    else
		 	     	    {
		 	     	  	  sprintf(szTemp,"%d",HH+1);
		 	     	    }
		 	 	        
		 	 	        strtmp=szTemp;	 	 	
		 	          strFrom=strNowTime_Day+strNowTime_HH.substr(0,2)+"4500";	 	 	      
		 	 	        strTo=strNowTime_Day+strtmp+"0000";
		 	 	
		 	       }
		      }
		      strCmpTime = strTo;
	      }
	      else if(iDelay==30)
	      {
	        if(MM<30)
		      {
		 	 	    strFrom=strNowTime_Day+strNowTime_HH+"0000";
            strTo=strNowTime_Day+strNowTime_HH+"3000";		 	 	
		 	      
		      }
		      else if(MM>=30 )
		      {
		 	       
            if(HH==23)
		 	       {
		 	 	       strFrom=strNowTime_Day+"230000";
               strTo=strNxtDay+"000000";		 	 	
		 	       } 
		 	       else
		 	       {
		 	         	if (HH<9)
		 	     	    {
		 	     	  	  sprintf(szTemp,"0%d",HH+1);
		 	     	    }
		 	     	    else
		 	     	    {
		 	     	  	  sprintf(szTemp,"%d",HH+1);
		 	     	    }
		 	 	       
		 	 	        strtmp=	szTemp;	 	 	
		 	          strFrom=strNowTime_Day+strNowTime_HH.substr(0,2)+"3000";	 	 	      
		 	 	        strTo=strNowTime_Day+strtmp+"0000";
		 	 	
		 	       }	
		      }
		       strCmpTime = strTo;
	       }
	       else if(iDelay==60)
	       {
	  	     if(HH==23)
		 	     {
		 	 	      strFrom=strNowTime_Day+"230000";
              strTo=strNxtDay+"000000";		 	 	
		 	     } 
		 	     else
		 	     {
		 	     	  if (HH<9)
		 	     	  {
		 	     	  	sprintf(szTemp,"0%d",HH+1);
		 	     	  }
		 	     	  else
		 	     	  {
		 	     	  	sprintf(szTemp,"%d",HH+1);
		 	     	  }
		 	 	      
		 	 	      strtmp=szTemp;	 	 	
		 	        strFrom=strNowTime_Day+strNowTime_HH.substr(0,2)+"0000";	 	 	      
		 	 	      strTo=strNowTime_Day+strtmp+"0000";
		 	 	
		 	     }	
         strCmpTime = strTo;
        }
  }

  else if (strTime[0]=='D')
  {
    qcType='D';
    strDay=strTime.substr(1,2);
    strHHMMSS=strTime.substr(3);
    strFrom=strLastday+"000000";
    strTo=strToday+strHHMMSS;
    strCmpTime = strTo;
  } 
  else if (strTime[0]=='M')
  {
    qcType='M';
    strDay=strTime.substr(1,2);
    strHHMMSS=strTime.substr(3);
    strFrom=strLastMonth+strTime.substr(1);
    strTo=strCurMonth+strTime.substr(1);
    strCmpTime=strTo;
  }  
  
  bProvided=false;
  
  getCurTime(sztmp); 
  nowTime=sztmp;
  
  strPreFrom=strFrom;
  strPreTo=strTo;

  if (iCheck==1) //����ʱ�԰��շַ���ȫ��Ϊfalse�����·ַ���Ҫ��顣
  {
    if (strTime[0]=='D'&&nowTime.compare(strCmpTime)>0) 
      bProvided=true;
  }
  
  if (strDay!="00"&&nowTime.compare(strCmpTime)>0) 
    bProvided=true;
  
  if (strTime[0]=='E') bProvided=false;  
  
  expTrace("Y", __FILE__, __LINE__, "TimeCheck Mode:%s",strTime); 
  expTrace("Y", __FILE__, __LINE__, "init TimeCheck success,next runtime:%s",strCmpTime); 
}



void CTimeCheck::init(char *inStr,int iDy)  //��ʼ�����ж����͵�
{
  char sztmp[20];
  strTime=inStr;
  bProvided=false;
  qcType=inStr[0]; //Ĭ��
  iDelay=	iDy;
  getCurTime(sztmp);
  m_strLastRestTime=sztmp;
  resetTime(1);
  
 
}

void CTimeCheck::GetLastMonth(string& mon)
{
  time_t		time1;
  struct tm	*time2;
  char curtime[100];
  time(&time1);
  int year,month;
  time2 = localtime(&time1);
  year=time2->tm_year+1900;
  month=time2->tm_mon;
  if (month==0)
  {
    year--;
    month=12;
  }
  sprintf(curtime, "%4d%02d", year,month);
  mon=curtime;
}

