/***************************************************
filename: Common.cpp
description:
   define some common functions used by other module
create date: 2005-04-01
****************************************************/
#include "Common.h"
#include "Log.h"
#include "encrypt.h"

void delSpace(char *ss, int ss_len)
{
   int i, j, k, len;
   char temp[1024];
   if (ss_len > 0)
      len = ss_len;
   else
      len = strlen(ss);
   memset(temp, 0, 1024);
   for (j = 0;j < len;j++)
      if (ss[j] != ' ')
        break;
   for (k = j;k < len;k++)
      temp[k - j] = ss[k];
   i = strlen(temp) - 1;
   while ( i && temp[i] == ' ' )
      i--;
   temp[i + 1] = 0;
   memset(ss, 0, len);
   i = strlen(temp);
   for (j = 0;j < i;j++)
      ss[j] = temp[j];
}

void getCurTime(char *cur_Time)
{
  char now_datetime[15];
  time_t timer;
  struct tm nowtimer;
  time(&timer);
  nowtimer = *localtime(&timer);
  nowtimer.tm_mon++;
  memset(now_datetime, 0, sizeof(now_datetime));
  sprintf(now_datetime, "%04u%02u%02u%02u%02u%02u", nowtimer.tm_year + 1900,
        nowtimer.tm_mon, nowtimer.tm_mday, nowtimer.tm_hour,
        nowtimer.tm_min, nowtimer.tm_sec);

  strcpy(cur_Time, now_datetime);
}

pthread_mutex_t g_logMutex;

void expTrace(const char *szDebugFlag, const char *szFileName,
  int lineno, const char *msg, ...)
{
  pthread_mutex_lock(&g_logMutex);
  va_list argp;
  char ss0[1024];
  char *ss1, *ss2;
  char *spara;
  int ipara;
  char sline[6];
  char cur_time[15];

  if (strcmp(szDebugFlag, "Y")!=0)
	theLog.SetOutPutFlag(false);
  else
	theLog.SetOutPutFlag(true);
//    return;

  strncpy(ss0, msg, 1024);
  ss0[1023]='\0';
  ss1=ss0;
  sprintf(sline, "%05d", lineno);
//  getCurTime(cur_time);
//  cout<<"["<<cur_time<<"] ";
  //cout<<szFileName<<":"<<sline<<":"<<endl;
  //cout<<szFileName<<":"<<sline<<":";
  va_start( argp, msg );
  while (( ss2 = strchr( ss1, '%') ) != NULL) {
    *ss2 = 0;
    theLog<<ss1;
    switch (ss2[1])
    {
      case 'd':
        ipara=va_arg(argp, int);
        theLog<<ipara;
        ss1=ss2+2;
        break;
      case 's':
        spara=va_arg(argp, char *);
        theLog<<spara;
        ss1=ss2+2;
        break;
      default:
        theLog<<"%";
        ss1=ss2+1;
        break;
    }
  }

  va_end( argp );
  theLog<<ss1;
  theLog<<endi;
  
  pthread_mutex_unlock(&g_logMutex);
  return;
}

int getEnv(const char *fname, const char *comp, char *ret )
{
 int i;
 FILE *tmpfp;
 char ss[2][1000], buf[1000];
   if( ( tmpfp = fopen( fname, "r" ) ) == NULL ) {
   	 //deleted by tanj 20050925
     //cout<<"open file Err!:"<<fname<<endl;    
     return(-1);
   }

   while( fgets( buf, 1000, tmpfp ) != NULL ) {
     if( buf[0] == '#' ) continue;
     ss[1][0]='\0';
     //add by zhanggq
     sscanf( buf, "%s %s", ss[0], ss[1]);
     if( !strcmp( ss[0], comp ) ) {
        strcpy( ret, ss[1] );
        fclose( tmpfp );
        return(0);
     }
   }
   fclose( tmpfp );
   //deleted by tanj 20050925
   //cout<<"Search File "<<fname<<" Nothing like "<<comp;
	return(-2);
}

bool getField(long field, char sep, const char *strin, char *strout)
{
    const char *beginp, *endp;
    beginp = endp = strin;
    int i = 1;
    for (; ;  ) 
    {
    	if ((endp = strchr(endp, sep)) == NULL)
    	{
    		break;
    	}
    	if (i >= field)
    	{
    		break;
    	}
    	endp++;
    	beginp = endp ;
    	i++;
    }
    if (i < field)
    {
    	return false;
    }
    else
    {
    	if (endp == NULL)
    	{
    		strcpy(strout, beginp);
    	}
    	else
    	{
    		strncpy(strout, beginp, endp - beginp);
    		strout[endp - beginp] = 0;
    	}
    	return true;
    }
}

bool getField(int field, char sep, const char *strin, int &out)
{
	char temp[10];
	memset(temp, 0, sizeof(temp));
	if (getField(field, sep, strin, temp) )
	{
		out = atol(temp);
		return true;
	}
	else
	{
		return false;
	}
}
bool getField(int field, char sep, const char *strin, long &out)
{
	char temp[10];
	memset(temp, 0, sizeof(temp));
	if (getField(field, sep, strin, temp) )
	{
		out = atol(temp);
		return true;
	}
	else
	{
		return false;
	}
}
/******************************************************************************************
*������char *_dir  [in]    Ҫ���򴴽���·��
*���������·��_dir�Ƿ���ڣ���������ڣ������ɸ�·�� ע�⣺һ��ֻ������һ��Ŀ¼
*���أ����·���Ѿ����ڻ򲻴��ڵ��Ǵ����ɹ��򷵻�0�����·���������Ҵ������ɹ��򷵻�-1
*���ߣ�̷��    2004.10.15
*******************************************************************************************/
int chkDir( char *_dir )
{
 int i;
     //if( chdir( _dir ) ) {
     if ( access( _dir,F_OK ) < 0 ) {
         if( mkdir( _dir,S_IRWXU|S_IRGRP|S_IXGRP ) ) 
         {
             return(-1);
         }
     }
 return(0);
}

int chkAllDir(char *path)
{
	if(completeDir(path) != 0)
		return -1;
	
	char *p = path + 1;
	char temppath[256];
	for (; ;)
	{
		p = strchr(p, '/');
		if (p == NULL)
		{
			break;
		}
		p++;
		memset(temppath, 0, sizeof(temppath));
		strncpy(temppath, path, p - path);
		if (chkDir(temppath))
		{
			return -1;
		}
	}
	return 0;
}

/*************************************************
* ����    ����Ŀ¼��������(������'/'��β�ļ���'/')
* ��ڲ�����pchPath��Ҫ�����Ŀ¼��
* ���ڲ�������
* ����    ���ɹ�����0
*           ���򷵻�-1
**************************************************/
int completeDir(char *pchPath)
{
	if(pchPath == NULL)
		return -1;

	if (pchPath[strlen(pchPath) - 1] != '/')
	{
		strcat(pchPath, "/");
	}

	return 0;
}

/*************************************************
* ����    ����������·�����ļ����ֽ��·�����ļ���
* ��ڲ�����pchFullName��	Ҫ�ֽ��ȫ·��
* ���ڲ�����pchPath: 		·��
*			pchFileName: 	�ļ���
* ����    ��0:	�ֽ��·�����ļ���
*           1:	ֻ��·��
*			2:	ֻ���ļ���
*			-1:	�ֽ�ʧ��
**************************************************/
int dispartFullName(char *pchFullName, char *pchPath, char *pchFileName)
{
	if(pchFullName == NULL)
		return -1;
	
	//ֻ��Ŀ¼	
	if(pchFullName[strlen(pchFullName) - 1] == '/')
	{
		strcpy(pchPath, pchFullName);
		return 1;
	}
	
	char *pchTmp = strrchr(pchFullName, '/');
	
	//ֻ���ļ���	
	if(pchTmp == NULL)
	{
		strcpy(pchFileName, pchFullName);
		return 2;
	}
	else//����Ŀ¼, �����ļ���
	{
		strcpy(pchFileName, pchTmp + 1);
		strncpy(pchPath, pchFullName, strlen(pchFullName) - strlen(pchFileName));
		pchPath[strlen(pchFullName) - strlen(pchFileName)] = 0;
		return 0;
	}
}

/*************************************************
* ����    ����������Ƿ����'YYYYMMDD'��'YYMMDD'��ʽ
* ��ڲ�����pchString��Ҫ�����жϵ������ִ�
* ���ڲ�������
* ����    ����ʽ��ȷ����true
*           ���򷵻�false
**************************************************/
bool checkDate(const char *pchString)
{
	int num,iLen;
	char tmp[DATETIME_LEN];
	int iYear,iMonth,i;
	
	iLen = strlen(pchString);
	if ((iLen != 6) && (iLen != 8))
	{
		return false;
	}

	for (i=0; i<iLen; i++)
	{
		if ((pchString[i] < '0') || (pchString[i] > '9'))
		{
			return false;
		}
	}
	
	//YYMMDD��ʽ
	if (iLen == 6)
	{
		//У�����
		strncpy(tmp, pchString, 2);
		tmp[2] = 0;
		num = atoi(tmp);
		iYear = num + 2000;
		if ((num < 0) || (num > 99))
		{
			return false;
		}
		//У���·�
		strncpy(tmp, pchString + 2, 2);
		tmp[2] = 0;
		num = atoi(tmp);
		iMonth = num;
		if ((num < 1) || (num > 12))
		{
			return false;
		}

		//У����
		strncpy(tmp, pchString + 4, 2);
		tmp[2] = 0;
		num = atoi(tmp);
		if ((num < 1) || (num > 31))
		{
			return false;
		}
	}
	//YYYYMMDD��ʽ
	else if (iLen == 8)
	{
		strncpy(tmp, pchString, 4);
		tmp[4] = '\0';
		num = atoi(tmp);
		iYear = num;
		num = num - 2000;
		if ((num <0) || (num > 99))
		{
			return false;
		}
		//У���·�
		strncpy(tmp, pchString + 4, 4);
		tmp[2] = 0;
		num = atoi(tmp);
		iMonth = num;
		if ((num < 1) || (num > 12))
		{
			return false;
		}
		//У����
		strncpy(tmp, pchString + 6, 2);
		tmp[2] = 0;
		num = atoi(tmp);
		if ((num < 1) || (num > 31))
		{
			return false;
		}
	}
	
	switch(iMonth)
	{
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			if ((num < 1) || (num > 31))
			{
				return false;
			}
			break;
		case 2:
			//�ж�����
			if (((iYear%4 == 0) && (iYear%100 != 0)) || (iYear%400 == 0))
			{
				if ((num < 1) || (num > 29))
				{
					return false;
				}
			}
			else
			{
				if ((num < 1) || (num > 28))
				{
					return false;
				}
			}
			break;
		default :
			if((num < 1) || (num > 30))
			{
				return false;
			}
	}
	return true;
}

/*************************************************
* ����    ����ȡ'YYYYMMDD'��ʽ�ĵ�ǰ����
* ��ڲ�������
* ���ڲ�����pchDate����ǰ����
* ����    ���ɹ���ȡ����0
**************************************************/
int getCurDate(char *pchDate)
{
	time_t  timer;
	struct tm nowtimer;

    time(&timer);
    nowtimer = *localtime (&timer);
	
	nowtimer.tm_mon++;
    sprintf(pchDate, "%04u%02u%02u", nowtimer.tm_year+1900, nowtimer.tm_mon, nowtimer.tm_mday);
	return 0;
}

/*************************************************
* ����    ����'YYYYMMDD24HHMISS'��ʽʱ���ִ�ת��Ϊ����ʱ��
* ��ڲ�����timeStr��Ҫת����ʱ���ִ�
* ���ڲ�������
* ����    ���ɹ���������ʱ��
*           ���򷵻�-1
**************************************************/
long timeStr2Time(const char* timeStr)
{
	long ret =-1;
	if (!checkTime(timeStr))
		return ret;
	if ((timeStr != NULL) &&(strlen(timeStr) == DATETIME_LEN )) {
		struct tm tm_st;
		int  i ;
		char tmpstr[8];
		memcpy(tmpstr ,timeStr ,4);		tmpstr[4] =0;	i =atoi(tmpstr);	tm_st.tm_year =i -1900;  //pay attention , not forget '-1900'
		memcpy(tmpstr ,timeStr +4,2);	tmpstr[2] =0;	i =atoi(tmpstr);	tm_st.tm_mon =i -1;  //pay attention , not forget '-1'
		memcpy(tmpstr ,timeStr +6,2);	tmpstr[2] =0;	i =atoi(tmpstr);	tm_st.tm_mday =i ;
		memcpy(tmpstr ,timeStr +8,2);	tmpstr[2] =0;	i =atoi(tmpstr);	tm_st.tm_hour =i ;
		memcpy(tmpstr ,timeStr +10,2);	tmpstr[2] =0;	i =atoi(tmpstr);	tm_st.tm_min =i ;
		memcpy(tmpstr ,timeStr +12,2);	tmpstr[2] =0;	i =atoi(tmpstr);	tm_st.tm_sec =i ;
		tm_st.tm_isdst =0;
		return mktime(&tm_st);
	}
	return ret;
}

/*************************************************
* ����    ��������ʱ��ת��Ϊ'YYYYMMDD24HHMISS'��ʽʱ���ִ�
* ��ڲ�����ret��Ҫת��������ʱ��
* ���ڲ�����timeStr�����ʱ���ִ�
* ����    ���ɹ����ؽ��ʱ���ִ�ָ��
*           ���򷵻�NULL
**************************************************/
char* time2TimeStr(long ret, char *timeStr)
{
	if ( (ret <= 0) ||(timeStr == NULL))
		return NULL;
		
	time_t  time_st;
	struct tm now;
	time_st =ret ;
	now =*localtime ( &time_st ); now.tm_mon++;
	sprintf( timeStr,"%4u%02u%02u%02u%02u%02u",	now.tm_year+1900, now.tm_mon, now.tm_mday, 
							now.tm_hour, now.tm_min, now.tm_sec);

	return timeStr;
}

/*************************************************
* ����    ������ԭ���ڼӼ�ָ�������������(YYYYMMDD��ʽ)
* ��ڲ�����nDays��ָ������
            pchOrgDate��ԭ����
* ���ڲ�����pchTgtDate���������
* ����    ���ɹ����ؽ�������ִ�ָ��
*           ���򷵻�NULL
**************************************************/
char* addDays(int nDays, const char* pchOrgDate, char* pchTgtDate)
{
	if(strlen(pchOrgDate) != 8)
		return NULL;

	if(!checkDate(pchOrgDate))
		return NULL;

	char pchTime[DATETIME_LEN + 1] = {0};
	strcat(pchTime, pchOrgDate);
	strcat(pchTime, "000000");
	
	long nTgtTime;
	
	nTgtTime = timeStr2Time(pchTime) + nDays*24*60*60;
	
	memset(pchTime, 0, sizeof(pchTime));
	time2TimeStr(nTgtTime, pchTime);

	strncpy(pchTgtDate, pchTime, 8);

	return pchTgtDate;
}

/*************************************************
* ����    ����������������������(YYYYMMDD��ʽ)
* ��ڲ�����pchFirstDate����һ������
            pchSecondDate���ڶ�������
* ���ڲ�������
* ����    ���ɹ�������������
*           ���򷵻�ERR_DATE_FORMAT(-9999)
**************************************************/
int minusDays(const char* pchFirstDate, const char* pchSecondDate)
{
	//Only for format 'YYYYMMDD'
	if(strlen(pchFirstDate) != 8)
		return ERR_DATE_FORMAT;

	if(strlen(pchSecondDate) != 8)
		return ERR_DATE_FORMAT;

	if(!checkDate(pchFirstDate) || !checkDate(pchSecondDate))
		return ERR_DATE_FORMAT;

	char pchTime[DATETIME_LEN + 1] = {0};
	long nFirstDate, nSecondDate;
	
	strcpy(pchTime, pchFirstDate);
	strcat(pchTime, "000000");
	nFirstDate = timeStr2Time(pchTime);

	strcpy(pchTime, pchSecondDate);
	strcat(pchTime, "000000");
	nSecondDate = timeStr2Time(pchTime);

	return (nFirstDate - nSecondDate)/(24*60*60);
}


/*************************************************
* ����    ����������������������(YYYYMMDDHHMMSS��ʽ)
* ��ڲ�����pchFirstTime����һ��ʱ��
            pchSecondTime���ڶ���ʱ��
* ���ڲ�������
* ����    ���ɹ�������������(����������ȷ����)
*           ���򷵻�ERR_TIME_FORMAT(-9998)
**************************************************/
int minusTimes(const char* pchFirstTime, const char* pchSecondTime)
{
	//Only for format 'YYYYMMDDHHMMSS'
	if(strlen(pchFirstTime) != 14)
		return ERR_TIME_FORMAT;

	if(strlen(pchSecondTime) != 14)
		return ERR_TIME_FORMAT;

	if(!checkTime(pchFirstTime) || !checkTime(pchSecondTime))
		return ERR_TIME_FORMAT;

	long nFirstTime, nSecondTime;
	
	nFirstTime = timeStr2Time(pchFirstTime);

	nSecondTime = timeStr2Time(pchSecondTime);

	return (nFirstTime - nSecondTime)/(24*60*60);
}


/*************************************************
* ����    ����������Ƿ����'YYYYMMDDHHMMSS'��ʽ
* ��ڲ�����pchString��Ҫ�����жϵ�ʱ���ִ�
* ���ڲ�������
* ����    ����ʽ��ȷ����true
*           ���򷵻�false
**************************************************/
bool checkTime(const char *pchString)
{
	int num,iLen;
	char tmp[DATETIME_LEN];
	int iYear,iMonth,i;
	
	iLen = strlen(pchString);
	if (iLen != 14)
	{
		return false;
	}

	for (i=0; i<iLen; i++)
	{
		if ((pchString[i] < '0') || (pchString[i] > '9'))
		{
			return false;
		}
	}
	
		//YYYYMMDDHHMMSS��ʽ
		strncpy(tmp, pchString, 4);
		tmp[4] = '\0';
		num = atoi(tmp);
		iYear = num;
		num = num - 2000;
		if ((num <0) || (num > 99))
		{
			return false;
		}
		//У���·�
		strncpy(tmp, pchString + 4, 4);
		tmp[2] = 0;
		num = atoi(tmp);
		iMonth = num;
		if ((num < 1) || (num > 12))
		{
			return false;
		}
		//У����
		strncpy(tmp, pchString + 6, 2);
		tmp[2] = 0;
		num = atoi(tmp);
		if ((num < 1) || (num > 31))
		{
			return false;
		}
			
	switch(iMonth)
	{
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			if ((num < 1) || (num > 31))
			{
				return false;
			}
			break;
		case 2:
			//�ж�����
			if (((iYear%4 == 0) && (iYear%100 != 0)) || (iYear%400 == 0))
			{
				if ((num < 1) || (num > 29))
				{
					return false;
				}
			}
			else
			{
				if ((num < 1) || (num > 28))
				{
					return false;
				}
			}
			break;
		default :
			if((num < 1) || (num > 30))
			{
				return false;
			}
	}
	
	//У��Сʱ
		strncpy(tmp, pchString + 8, 2);
		tmp[2] = 0;
		num = atoi(tmp);
		if ((num < 0) || (num > 23))
		{
			return false;
		}
		//У�����
		strncpy(tmp, pchString + 10, 2);
		tmp[2] = 0;
		num = atoi(tmp);
		if ((num < 0) || (num > 59))
		{
			return false;
		}
		//У����
		strncpy(tmp, pchString + 12, 2);
		tmp[2] = 0;
		num = atoi(tmp);
		if ((num < 0) || (num > 59))
		{
			return false;
		}
	
	return true;
}




/*************************************************
* ����    ���ļ����ƺ������Ƚ��ļ����Ƴ���ʱ�ļ����ٸ�����
* ��ڲ�����pchSrcFile: ԭʼ�ļ�  
*			pchTgtFile��Ŀ���ļ�
* ���ڲ�������
* ����    ��
*            -1 ��ԭʼ�ļ�����
*            -2 ��Ŀ���ļ�����
*            -3 �ļ�����ʧ��
*            0  �ɹ�
**************************************************/
int copyFile( char *pchSrcFile,char *pchTgtFile )
{
  char ch;
  long i,j,k,l,fsize,lsize,nsize;
  FILE *fp1,*fp2;
  char Bufs[101000],sz_tempname[800];
  sprintf(sz_tempname,"%s.TMP",pchTgtFile);
  if((fp1 = fopen(pchSrcFile,"r"))==NULL)
  {
    return(-1);
  }
  if((fp2 = fopen(sz_tempname,"w"))==NULL)
  {
     return(-2);
  }
  fseek(fp1,0L,SEEK_END);
  fsize=ftell(fp1);
  fseek(fp1,0L,SEEK_SET);

  lsize=fsize;
  while( 1 )
  {
    if( lsize > 100000 ) i=100000;
    else i = lsize;
    nsize =fread(Bufs,1,i,fp1);
    lsize =lsize-nsize;
    k=nsize;
    l=0;
    while(1)
    {
      j=fwrite(Bufs+l,1,k,fp2 );
      k =k -j;
      l=l+j;
      if(k<=0) break;
    }
    if(lsize<=0) break;
  }
  fclose(fp1);
  fclose(fp2);
  if(rename(sz_tempname,pchTgtFile))
  {
    return (-3);
  }
  return 0;
}

/*************************************************
* ����    ���趨��־Ŀ¼
* ��ڲ�����pchPath����־��Ŀ¼
            pchPipeID��PIPEID(��Ŀ¼)
* ���ڲ�������
* ����    ���ɹ�����0
*           ���򷵻�-1
**************************************************/
int setLogPath(const char* pchPath, const char* pchPipeID)
{
	char pchLogPath[PATH_NAME_LEN];

	if(pchPath == NULL)
		return -1;

	strcpy(pchLogPath, pchPath);
	if(completeDir(pchLogPath) != 0)
		return -1;

	strcat(pchLogPath, pchPipeID);

	if(chkAllDir(pchLogPath) != 0)
		return -1;

	theLog.SetLogPath(pchLogPath);
	return 0;
}

/*************************************************
* ����    ���������ݿ�, ����ʱӦʹ��try_catch��ʽ
* ��ڲ�����pchEnvFile�����������ļ���
*           db�����ݿ����ӱ���
* ���ڲ�������
* ����    ���ɹ�����0
*           -1:δ�ҵ����ݿ�SID
*			-2:δ�ҵ�UNAME
*			-3:δ�ҵ�UPASS
*			-4:����ʧ��
**************************************************/
int connectDB(char *pchEnvFile, CDatabase &db)
{
	CEncryptAsc encrypt;
	char *pchValue;
	char pchDBname[50];
	char pchUserName[50];
	char pchUserPWD[50];

	char pchEncryptedUName[USER_NAME_LEN];
	char pchEncryptedUPWD[USER_PASS_LEN];

	//��ȡ���ݿ�SID
	if(getEnv(pchEnvFile, DBSERVER_ENV_NAME, pchDBname) != 0)
		return -1;

	//��ȡ���ݿ��û���
	if(getEnv(pchEnvFile, DBUSER_ENV_NAME, pchEncryptedUName) != 0)
		return -2;
	encrypt.Decrypt(pchEncryptedUName, pchUserName);
	
	//��ȡ���ݿ��û�����
	if(getEnv(pchEnvFile, DBPASS_ENV_NAME, pchEncryptedUPWD) != 0)
		return -3;
	encrypt.Decrypt(pchEncryptedUPWD, pchUserPWD);

	//�������ݿ�
	if(db.Connect(pchUserName, pchUserPWD, pchDBname) != 0)
		return -4;
	else
		return 0;
}

/*************************************************
* ����    �������ݿ���pipe_env�ж�ȡ��������, ���pipe_env��
*			δ����, ���global_env�ж�ȡ, ����ʱӦʹ��try_catch��ʽ
* ��ڲ�����db�����ݿ����ӱ���
*			pchPipeID��ָ����PIPE_ID
*			pchVarName������������
* ���ڲ�����pchVarValue: ��������ֵ
* ����    ��0:��pipe_env�ж�ȡ��������
*           1:��global_env�ж�ȡ��������
*			-1:��ȡʧ��,û��ֵ����
**************************************************/
/*int getEnvFromDB(CDatabase &db, const char *pchPipeID, const char *pchVarName, char *pchVarValue)
{
	CBindSQL ds(db);
	char pchSql[SQL_LEN];
	sprintf(pchSql, "SELECT VAR_VALUE FROM PIPE_ENV WHERE PIPE_ID = '%s' AND VARNAME = '%s'",
			pchPipeID, pchVarName);

	ds.Open(pchSql, SELECT_QUERY);
	
	if(!(ds >> pchVarValue))
	{
		//deleted by zgq 20051101
		//cout << pchSql << " returned no rows!" << endl;
		ds.Close();
		
		sprintf(pchSql, "SELECT VARVALUE FROM GLOBAL_ENV WHERE VARNAME = '%s'"
		,pchVarName);
		
		ds.Open(pchSql, SELECT_QUERY);
		if(!(ds >> pchVarValue))
		{
			ds.Close();
		  //deleted by zgq 20051101
			//cout << pchSql << " returned no rows!" << endl;
			return -1;
		}

		return 1;
	}
	ds.Close();
	return 0;
}*/

/*************************************************
* ����    �������ݿ���pipe_env�ж�ȡ��������, ���pipe_env��
*			δ����, ���global_env�ж�ȡ, ����ʱӦʹ��try_catch��ʽ
* ��ڲ�����db�����ݿ����ӱ���
*			pchPipeID��ָ����PIPE_ID
*			pchProcessID: ����ID
*			pchVarName������������
* ���ڲ�����pchVarValue: ��������ֵ
* ����    ��0:��pipe_env�ж�ȡ��������
*           1:��global_env�ж�ȡ��������
*			-1:��ȡʧ��,û��ֵ����
**************************************************/
int getEnvFromDB(CDatabase &db, const char *pchPipeID, const int pchProcessID, const char *pchVarName, char *pchVarValue)
{
	CBindSQL ds(db);
	char pchSql[SQL_LEN];
	sprintf(pchSql, "SELECT VAR_VALUE FROM PIPE_ENV WHERE PIPE_ID = '%s' AND PROCESS_ID = %d AND VARNAME = '%s'",
			pchPipeID, pchProcessID, pchVarName);

	ds.Open(pchSql, SELECT_QUERY);
	
	if(!(ds >> pchVarValue))
	{
		//deleted by zgq 20051101
		//cout << pchSql << " returned no rows!" << endl;
		ds.Close();
		
		sprintf(pchSql, "SELECT VAR_VALUE FROM PIPE_ENV WHERE PIPE_ID = '%s' AND PROCESS_ID=-1 AND VARNAME = '%s'",
				pchPipeID,pchVarName);
		
		ds.Open(pchSql, SELECT_QUERY);
		if(!(ds >> pchVarValue))
		{
			ds.Close();
		  	//deleted by zgq 20051101
			//cout << pchSql << " returned no rows!" << endl;
			sprintf(pchSql, "SELECT VARVALUE FROM GLOBAL_ENV WHERE VARNAME = '%s'",
				pchVarName);
		
			ds.Open(pchSql, SELECT_QUERY);
			if(!(ds >> pchVarValue))
			{
				ds.Close();
				return -1;
			}
			ds.Close();
			return 1;
		}
		ds.Close();
		return 0;
	}
	ds.Close();
	return 0;
}

/*************************************************
* ����    ���ж�ָ��pid�Ľ����Ƿ����
* ��ڲ�����nPid��ָ���Ľ��̺�
* ���ڲ�������
* ����    ��1:���̴���
*           0:���̲�����
*			>1:��⵽�������
*			<0:���ʧ��
**************************************************/
int isProcExist(int nPid)
{
	if(nPid <= 0)
		return 0;
	
	char pchBuf[LOG_MSG_LEN];
	FILE *fp;
	int nProcCount = 0;
	
	sprintf(pchBuf , "ps -ef | awk '{ if($2 == %d ) print $0}'", nPid);
	if((fp = popen(pchBuf , "r")) == NULL)
	{
		cout << "Can't create a pipe: " << pchBuf << endl;
		return -1;
	}
	while(fgets(pchBuf, sizeof(pchBuf), fp) != NULL)
	{
		cout << pchBuf << endl;
		nProcCount++;
	}
	int jj=0;
	jj=pclose(fp);	
	return nProcCount;
}

/*************************************************
* ����    ���ж���ָ���ؼ���ƥ��Ľ����Ƿ����
* ��ڲ�����pchKeyWord��ָ���Ĺؼ���
* ���ڲ�������
* ����    ��1:���̴���
*           0:���̲�����
*			>1:��⵽�������
*			<0:���ʧ��
**************************************************/
int isProcExist(const char* pchKeyWord)
{
	if(pchKeyWord == NULL)
		return -1;
	
	char pchBuf[LOG_MSG_LEN];
	FILE *fp;
	int nProcCount = 0;
	
	sprintf(pchBuf, "ps -ef | grep \"%s\" | grep -v grep", pchKeyWord);
	
	if((fp = popen(pchBuf , "r")) == NULL)
	{
		cout << "Can't create a pipe: " << pchBuf << endl;
		return -1;
	}
	
	while(fgets(pchBuf, sizeof(pchBuf), fp) != NULL)
	{
		cout << pchBuf << endl;
		nProcCount++;
	}
	
	pclose(fp);	
	return 0;
}

/*************************************************
* ����    ��˳�����ָ���ַ���ĳ�ַ����е�n�γ��ֵ�λ��
* ��ڲ�����str��ָ�����ַ���
*			c��ָ�����ַ�
*			nIdx:ָ������
* ���ڲ�������
* ����    ��>=0:�ַ�λ��
*			<0:δ�ҵ�
**************************************************/
int strncspn(const char* str, int c, int nIdx)
{
	if(str == NULL)
		return -1;

	int i = 0;
	for(int nPos = 0; nPos < strlen(str); nPos++)
	{
		if(str[nPos] == c)
		{
			i++;
			if(i == nIdx)
				return nPos;
		}
	}

	return -1;
}

/*************************************************
* ����    ���������ָ���ַ���ĳ�ַ����е�n�γ��ֵ�λ��
* ��ڲ�����str��ָ�����ַ���
*			c��ָ�����ַ�
*			nIdx:ָ������
* ���ڲ�������
* ����    ��>=0:�ַ�λ��
*			<0:δ�ҵ�
**************************************************/
int strrncspn(const char* str, int c, int nIdx)
{
	if(str == NULL)
		return -1;
	
	int i = 0;
	for(int nPos = strlen(str) - 1; nPos >= 0; nPos--)
	{
		if(str[nPos] == c)
		{
			i++;
			if(i == nIdx)
				return nPos;
		}
	}

	return -1;
}

/*************************************************
* ����    �����ļ�λ��ָ������ָ����(��0��ʼ)�Ŀ�ͷ
* ��ڲ�����fp��Ҫ�������ļ�
*			nIndex���к�
*			nLength������ļ��ж���, Ϊ�еĳ���(���������з�);����, ȱʡΪ-1
* ���ڲ�������
* ����    ��>=0:�����ɹ�,���ز������ļ��ĵ�ǰ��λ��
*			-1:����ʧ��
**************************************************/
long fgotoLine(FILE *fp, long nIndex, int nLength)
{
	long nOffset = -1;

	if(fp == NULL || nIndex < 0)
		return -1;

	if(nLength > 0)
	{
		nOffset = nIndex * (nLength + 1);
		if(fseek(fp, nOffset, SEEK_CUR) != 0)
			return -1;
		else
			return nOffset;
	}
	else
	{
		int nRow = 0;
		char pchBuf[MAX_LINE_LENGTH];

		rewind(fp);
		do
		{
			if(nRow < nIndex)
			{
				nRow++;
				continue;
			}
			else if(nRow > nIndex)//Overleaped
				return -1;
			
			if(fseek(fp, 0, SEEK_CUR) != 0)
				return -1;
			else
				return ftell(fp);
		}while(fgets(pchBuf, MAX_LINE_LENGTH, fp) != NULL);

		return -1;
	}
}

/*************************************************
* ����    ������������Ϊ�ػ�����
* ��ڲ�����cEnvPath:		env�ļ���·��
* ���ڲ�������
* ����    ����
**************************************************/
void initDaemon(char *cEnvPath)
{
	//��env�ļ��м���־
	char	cRet[5];
	char cEnvFile[256];
	strcpy(cEnvFile,cEnvPath);
	completeDir(cEnvFile);
	strcat(cEnvFile,ENV_FILE_NAME);
	if(getEnv(cEnvFile, "DAEMON_MODE", cRet) != 0) 
		return;
	if(cRet[0] == 'N') 
		return;

	int iPid = -1;

	iPid = fork();
	if(iPid < 0)//���ѽ���ʧ�ܣ��˳�
	{
		theLog << "Set as daemon process failed! Program exit!" << ende;
		exit(1);
	}
	else if(iPid > 0) 
		exit(0);//����������
	
	//��һ�ӽ����ں�̨����ִ��
	//��һ�ӽ��̳�Ϊ�µĻỰ�鳤�ͽ����鳤 
	setsid();
	
	iPid = fork();//�ٷ���
	if (iPid < 0)//���ѽ���ʧ�ܣ��˳�
	{
		theLog << "Set as daemon process failed! Program exit!" << ende;
		exit(1);
	}
	else if(iPid > 0) 
		exit(0);//������һ�ӽ���
	
	//�ڶ��ӽ��̼���ִ��
	//�ڶ��ӽ��̲����ǻỰ�鳤
	
	umask(0);//�����ļ�������ģ
	theLog << "Set as daemon process success!" << endi;
	theLog.ResetPid();
}

int Update_ProSTS(int JobID,char Deal_STS,char* szProcess_log)
{

  if(JobID == 0) return 0;
  
  CBindSQL ds(DBConn);
  char sqltmp[4000];
  int iProNoID=0;

  iProNoID = getpid();
//  if(iProNoID == -1)
//  	return -1;
  if(szProcess_log == NULL)
    sprintf(sqltmp,"Update T_JOB Set DEAL_STS = '%c',LASTDEAL_TIME = TO_CHAR(SYSDATE,'YYYYMMDDHHMMSS'),PROCESS_NO = '%d' Where JOBID = %d",Deal_STS,iProNoID,JobID);
  else sprintf(sqltmp,"Update T_JOB Set DEAL_STS = '%c',LASTDEAL_TIME = TO_CHAR(SYSDATE,'YYYYMMDDHHMMSS'),PROCESS_NO = '%d',PROCESS_LOG='%s' Where JOBID = %d",Deal_STS,iProNoID,szProcess_log,JobID);
  ds.Open(sqltmp);
  ds.Execute();
  if(ds.IsError())
  {
    ds.Close();
   	return (-1);
  }	
  ds.Close();

  return 0;
}
