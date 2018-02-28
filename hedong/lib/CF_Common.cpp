/***************************************************
filename: Common.cpp
description:
   define some common functions used by other module
create date: 2010-04-30
****************************************************/
#include "CF_Common.h"

CF_CCommon::CF_CCommon()
{
}

CF_CCommon::~CF_CCommon()
{
}

char * CF_CCommon:: getVersion()
{
	return("3.0.0");
}

/*************************************************
* 描述    ：连接数据库, 调用时应使用try_catch方式
* 入口参数：pchEnvFile：环境变量文件名
*           db：数据库连接变量
* 出口参数：无
* 返回    ：成功返回0
*           -1:未找到数据库SID
*			-2:未找到UNAME
*			-3:未找到UPASS
*			-4:连接失败
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

	CReadIni Read;
	Read.init(pchEnvFile);

	//读取数据库SID
	if(!(Read.GetValue("COMMON",DBSERVER_ENV_NAME,pchDBname)))
		return -1;

	//读取数据库用户名
	if(!(Read.GetValue("COMMON",DBUSER_ENV_NAME,pchEncryptedUName)))
		return -2;
	encrypt.Decrypt(pchEncryptedUName, pchUserName);
	
	//读取数据库用户密码
	if(!(Read.GetValue("COMMON",DBPASS_ENV_NAME,pchEncryptedUPWD)))
		return -3;
	encrypt.Decrypt(pchEncryptedUPWD, pchUserPWD);
	
	cout<<"pchUserName="<<pchUserName<<" pchUserPWD="<<pchUserPWD<<" pchDBname="<<pchDBname<<endl;
	//连接数据库
	if(db.Connect(pchUserName, pchUserPWD, pchDBname) != 0)
		return -4;
	else
		return 0;
}

/*************************************************
* 描述    ：从数据库中C_SOURCE_ENV中读取环境变量, 如果C_SOURCE_ENV中
*			未定义, 则从C_PROCESS_ENV中读取, 如果C_PROCESS_ENV为定义，最后从
			C_GLOBAL_ENV取，调用时应使用try_catch方式
* 入口参数：pchService：服务ID
*			pchSourceGroup：数据源组ID
*			pchSourceID: 数据源ID
*			pchVarName：环境变量名
* 出口参数：pchVarValue: 环境变量值
* 返回    ：0:从C_SOURCE_ENV中读取环境变量
*           1:从C_PROCESS_ENV中读取环境变量
*						2:从C_GLOBAL_ENV中读取环境变量
*			-1:读取失败,没有值返回
**************************************************/
int getEnvFromDB(const char *pchService, const  char *pchSourceGroup,const  char *pchSourceID, const char *pchVarName, char *pchVarValue)
{
	CBindSQL ds(DBConn);	
	char pchSql[SQL_LEN];

	ds.Open("SELECT VAR_VALUE FROM C_SOURCE_ENV WHERE SERVICE =:v1 AND SOURCE_ID =:v2 AND VARNAME =:v3");
	ds<<pchService<<pchSourceID<<pchVarName;
	if(!(ds >> pchVarValue))
	{
		ds.Close();
		
		ds.Open("SELECT VAR_VALUE FROM C_PROCESS_ENV WHERE SOURCE_GROUP =:v1 AND SERVICE=:v2  AND VARNAME =:v3");
		ds<<pchSourceGroup<<pchService<<pchVarName;
		if(!(ds >> pchVarValue))
		{
			ds.Close();
			ds.Open("SELECT VARVALUE FROM C_GLOBAL_ENV WHERE VARNAME =:v");
			ds<<pchVarName;
			if(!(ds >> pchVarValue))
			{
				ds.Close();
				return -1;
			}
			ds.Close();
			return 2;
		}
		ds.Close();
		return 1;
	}
	ds.Close();
	return 0;
}

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
*参数：char *_dir  [in]    要检查或创建的路径
*描述：检查路径_dir是否存在，如果不存在，则生成该路径 注意：一次只能生成一层目录
*返回：如果路径已经存在或不存在但是创建成功则返回0，如果路径不存在且创建不成功则返回-1
*作者：谭杰    2004.10.15
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
* 描述    ：将目录补充完整(不是以'/'结尾的加上'/')
* 入口参数：pchPath：要补充的目录串
* 出口参数：无
* 返回    ：成功返回0
*           否则返回-1
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
* 描述    ：将含完整路径的文件名分解成路径和文件名
* 入口参数：pchFullName：	要分解的全路径
* 出口参数：pchPath: 		路径
*			pchFileName: 	文件名
* 返回    ：0:	分解出路径和文件名
*           1:	只有路径
*			2:	只有文件名
*			-1:	分解失败
**************************************************/
int dispartFullName(char *pchFullName, char *pchPath, char *pchFileName)
{
	if(pchFullName == NULL)
		return -1;
	
	//只有目录	
	if(pchFullName[strlen(pchFullName) - 1] == '/')
	{
		strcpy(pchPath, pchFullName);
		return 1;
	}
	
	char *pchTmp = strrchr(pchFullName, '/');
	
	//只有文件名	
	if(pchTmp == NULL)
	{
		strcpy(pchFileName, pchFullName);
		return 2;
	}
	else//既有目录, 又有文件名
	{
		strcpy(pchFileName, pchTmp + 1);
		strncpy(pchPath, pchFullName, strlen(pchFullName) - strlen(pchFileName));
		pchPath[strlen(pchFullName) - strlen(pchFileName)] = 0;
		return 0;
	}
}

/*************************************************
* 描述    ：检查日期是否符合'YYYYMMDD'或'YYMMDD'格式
* 入口参数：pchString：要进行判断的日期字串
* 出口参数：无
* 返回    ：格式正确返回true
*           否则返回false
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
	
	//YYMMDD格式
	if (iLen == 6)
	{
		//校验年份
		strncpy(tmp, pchString, 2);
		tmp[2] = 0;
		num = atoi(tmp);
		iYear = num + 2000;
		if ((num < 0) || (num > 99))
		{
			return false;
		}
		//校验月份
		strncpy(tmp, pchString + 2, 2);
		tmp[2] = 0;
		num = atoi(tmp);
		iMonth = num;
		if ((num < 1) || (num > 12))
		{
			return false;
		}

		//校验天
		strncpy(tmp, pchString + 4, 2);
		tmp[2] = 0;
		num = atoi(tmp);
		if ((num < 1) || (num > 31))
		{
			return false;
		}
	}
	//YYYYMMDD格式
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
		//校验月份
		strncpy(tmp, pchString + 4, 4);
		tmp[2] = 0;
		num = atoi(tmp);
		iMonth = num;
		if ((num < 1) || (num > 12))
		{
			return false;
		}
		//校验天
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
			//判断闰年
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
* 描述    ：获取'YYYYMMDD'格式的当前日期
* 入口参数：无
* 出口参数：pchDate：当前日期
* 返回    ：成功获取返回0
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
* 描述    ：将'YYYYMMDD24HHMISS'格式时间字串转换为整数时间
* 入口参数：timeStr：要转换的时间字串
* 出口参数：无
* 返回    ：成功返回整数时间
*           否则返回-1
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
* 描述    ：将整数时间转换为'YYYYMMDD24HHMISS'格式时间字串
* 入口参数：ret：要转换的整数时间
* 出口参数：timeStr：结果时间字串
* 返回    ：成功返回结果时间字串指针
*           否则返回NULL
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
* 描述    ：计算原日期加减指定天数后的日期(YYYYMMDD格式)
* 入口参数：nDays：指定天数
            pchOrgDate：原日期
* 出口参数：pchTgtDate：结果日期
* 返回    ：成功返回结果日期字串指针
*           否则返回NULL
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
* 描述    ：计算两个日期相差的天数(YYYYMMDD格式)
* 入口参数：pchFirstDate：第一个日期
            pchSecondDate：第二个日期
* 出口参数：无
* 返回    ：成功返回相差的天数
*           否则返回ERR_DATE_FORMAT(-9999)
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
* 描述    ：计算两个日期相差的天数(YYYYMMDDHHMMSS格式)
* 入口参数：pchFirstTime：第一个时间
            pchSecondTime：第二个时间
* 出口参数：无
* 返回    ：成功返回相差的天数(按照秒数精确计算)
*           否则返回ERR_TIME_FORMAT(-9998)
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
* 描述    ：检查日期是否符合'YYYYMMDDHHMMSS'格式
* 入口参数：pchString：要进行判断的时间字串
* 出口参数：无
* 返回    ：格式正确返回true
*           否则返回false
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
	
		//YYYYMMDDHHMMSS格式
		strncpy(tmp, pchString, 4);
		tmp[4] = '\0';
		num = atoi(tmp);
		iYear = num;
		num = num - 2000;
		if ((num <0) || (num > 99))
		{
			return false;
		}
		//校验月份
		strncpy(tmp, pchString + 4, 4);
		tmp[2] = 0;
		num = atoi(tmp);
		iMonth = num;
		if ((num < 1) || (num > 12))
		{
			return false;
		}
		//校验天
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
			//判断闰年
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
	
	//校验小时
		strncpy(tmp, pchString + 8, 2);
		tmp[2] = 0;
		num = atoi(tmp);
		if ((num < 0) || (num > 23))
		{
			return false;
		}
		//校验分钟
		strncpy(tmp, pchString + 10, 2);
		tmp[2] = 0;
		num = atoi(tmp);
		if ((num < 0) || (num > 59))
		{
			return false;
		}
		//校验秒
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
* 描述    ：文件复制函数（先将文件复制成临时文件，再改名）
* 入口参数：pchSrcFile: 原始文件  
*			pchTgtFile：目标文件
* 出口参数：无
* 返回    ：
*            -1 打开原始文件出错
*            -2 打开目标文件出错
*            -3 文件改名失败
*            0  成功
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
* 描述    ：判断指定pid的进程是否存在
* 入口参数：nPid：指定的进程号
* 出口参数：无
* 返回    ：1:进程存在
*           0:进程不存在
*			>1:检测到多个进程
*			<0:检测失败
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
	pclose(fp);	
	return nProcCount;
}

/*************************************************
* 描述    ：判断与指定关键字匹配的进程是否存在
* 入口参数：pchKeyWord：指定的关键字
* 出口参数：无
* 返回    ：1:进程存在
*           0:进程不存在
*			>1:检测到多个进程
*			<0:检测失败
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
	//return 0;
	return  nProcCount;  //add by hed 20130529
}

/*************************************************
* 描述    ：顺序查找指定字符在某字符串中第n次出现的位置
* 入口参数：str：指定的字符串
*			c：指定的字符
*			nIdx:指定次数
* 出口参数：无
* 返回    ：>=0:字符位置
*			<0:未找到
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
* 描述    ：逆序查找指定字符在某字符串中第n次出现的位置
* 入口参数：str：指定的字符串
*			c：指定的字符
*			nIdx:指定次数
* 出口参数：无
* 返回    ：>=0:字符位置
*			<0:未找到
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
* 描述    ：将文件位置指针移至指定行(从0开始)的开头
* 入口参数：fp：要操作的文件
*			nIndex：行号
*			nLength：如果文件行定长, 为行的长度(不包含换行符);否则, 缺省为-1
* 出口参数：无
* 返回    ：>=0:操作成功,返回操作后文件的当前的位置
*			-1:操作失败
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
* 描述    ：将字符串字母变大写
* 入口参数：sstr：要操作的字符串
* 出口参数：sstr：要操作的字符串
**************************************************/
int toUpper(char *sstr)
{
	char *a=sstr;
	while ( *a !='\0' )
	{
		if ( *a >= 97 && *a <123 )
		{
			*a =*a-32;
		}
		a++;
	}
	return 0;
}
/*************************************************
* 描述    ：将字符串字母变小写
* 入口参数：sstr：要操作的字符串
* 出口参数：sstr：要操作的字符串
**************************************************/
int toLower(char *sstr)
{
	char *a=sstr;
	while ( *a !='\0' )
	{
		if ( *a >= 65 && *a <91 )
		{
			*a =*a+32;
		}
	  a++;
	}
	return 0;
}

/*************************************************
* 描述    ：将字符串字母变大写
* 入口参数：buff：要操作的字符串
* 出口参数：buff：要操作的字符串
**************************************************/
void toUpper(string& buff)
{
	string::iterator it = buff.begin();
	while (buff.end() != it) {
		*it = toupper(*it);
		++it;
	}
}
/*************************************************
* 描述    ：将字符串字母变小写
* 入口参数：buff：要操作的字符串
* 出口参数：buff：要操作的字符串
**************************************************/
void toLower(string& buff)
{
	string::iterator it = buff.begin();
	while (buff.end() != it) {
		*it = tolower(*it);
		++it;
	}
}

/*************************************************
* 描述    ：将进程设置为守护进程
* 入口参数：bDaemonFlag:	true-设置为守护进程; 
*							false-不用设置为守护进程	
* 出口参数：无
* 返回    ：无
**************************************************/
void initDaemon(bool bDaemonFlag)
{
	if(bDaemonFlag == false) 
		return;

	int nPid = -1;

	nPid = fork();
	if(nPid < 0)//分裂进程失败，退出
	{
		cout << "Set as daemon process failed! Program exit!" << endl;
		exit(1);
	}
	else if(nPid > 0) 
		exit(0);//结束父进程
	
	//第一子进程在后台继续执行
	//第一子进程成为新的会话组长和进程组长 
	setsid();
	
	nPid = fork();//再分裂
	if (nPid < 0)//分裂进程失败，退出
	{
		cout << "Set as daemon process failed! Program exit!" << endl;
		exit(1);
	}
	else if(nPid > 0) 
		exit(0);//结束第一子进程
	
	//第二子进程继续执行
	//第二子进程不再是会话组长
	
	umask(0);//重设文件创建掩模
}

//字符串处理函数strlen()，防止不同操作系统编译出错
int safeStrLen(const char* cpSrc)
{
	if(cpSrc == NULL)
		return 0;
	else
		return strlen(cpSrc);
}

/********************************************************
* 描述	  ：截去s尾部的空白字符（包括空格、回车、换行、跳格）
* 入口参数：s：要截去空白字符的字串
* 出口参数：无
* 返回    ：截完后的字串长度
********************************************************/
int trimRight(char *s)
{
	int i,l;

	l=safeStrLen(s);
	for(i=l-1;i>=0;i--)  {
		if( s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n' )       
			s[i]=0;
		else
			break;
	}
	return i+1;
}

/********************************************************
* 描述：    截去s左端所有的空白字符（包括空格、回车、换行、跳格） 
* 入口参数：s：要截去空白字符的字串
* 出口参数：无
* 返回    ：截完后的字串长度
********************************************************/
int trimLeft(char *s)
{
	int l,i,k;

	l=safeStrLen(s);
	for(i=0;i<l;i++)  {
		if(s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n')
			continue;
		else
			break;
        }
        if(i!=0) {
		for(k=0;k<l-i;k++) {
			s[k]=s[k+i];
		}
		s[k]=0;
        }
        return l-i;
}

/********************************************************
* 描述    ：截去s左、右端所有的空白字符（包括空格、回车、换行、跳格）
* 入口参数：s：要截去空白字符的字串
* 出口参数：无
* 返回    ：截完后的字串长度
********************************************************/
int trim(char *s)
{
	trimRight(s);
	return trimLeft(s);
}

/********************************************************
* 描述	  ：将源字串截断或填补或到定长, 注意字串的结束符'\0'
*			是不计在字串长度内的
* 入口参数：szSrc：源字串
*			nLength：操作后的字串长度
*			ch：用于填补的字符, 缺省为空格
* 出口参数：无
* 返回    ：成功填补后的字串
*         ：失败返回NULL
********************************************************/
char* fixStrLength(char *szSrc, unsigned int nLength, char ch)
{
	if(szSrc == NULL)
		return NULL;

	int nOrgLen = strlen(szSrc);

	if(nOrgLen > nLength)
	{
		szSrc[nLength] = 0;
		return szSrc;
	}

	int i;
	for(i = nOrgLen; i < nLength; i++)  
	{
		szSrc[i] = ch;
	}

	szSrc[nLength] = 0;
	
	return szSrc;
}

/********************************************************
* 描述	  ：将源字串按分隔符拆分成多个字串,单个字串长度不能超过STR_BUF_LENGTH
* 入口参数：szSrc：源字串
*			szSeparator：分隔符
*			bSkipSpace：是否跳过空字串, 缺省是跳过(true)
* 出口参数：vecResultStr：结果字串集
* 返回    ：结果字串的个数
********************************************************/
int splitString(char *szSrc, const char *szSeparator, vector<string> &vecResultStr, bool bSkipSpace)
{
	//分解序号列表
	if(szSrc == NULL)
		return 0;
	
	int i = 0;
	string strTmp;
	char szTmp[STR_BUF_LENGTH];
	char *szToken = NULL;
	
	szToken = strtok(szSrc, szSeparator);
	while(szToken != NULL)
	{
		strcpy(szTmp, szToken);
		trim(szTmp);
		strTmp = szTmp;
		
		if(bSkipSpace)//跳过空字串
		{
			if(strTmp.length() != 0)
			{
				vecResultStr.push_back(strTmp);
				i++;
			}
		}
		else
		{
			vecResultStr.push_back(strTmp);
			i++;
		}

		szToken = strtok(NULL, szSeparator);
	}

	return i;
}

/********************************************************
* 描述	  ：将源字串中的某子串替换为指定字串
* 入口参数：strBig：源字串
*			strsrc：被替换的子串
*			strdst：指定的替换字串
* 出口参数：strBig：替换结果
* 返回    ：无
********************************************************/
void string_replace(string & strBig, const string & strsrc, const string &strdst) 
{
	string::size_type pos=0;
	string::size_type srclen=strsrc.size();
	string::size_type dstlen=strdst.size();
	while( (pos=strBig.find(strsrc, pos)) != string::npos)
	{
		strBig.replace(pos, srclen, strdst);
		pos += dstlen;
	}
}
