/****************************************************************
filename: writelog.cc
module: WL - write log
created by: chen bingji
create date: 2000-9-18
update list:
  update by wh 2000-12-27, change the env file path
version: 1.1.1
description:
 called by other module to complete the write log operation
******************************************************************/

//include system lib
#include <string.h>
#include <strings.h>

#include <iostream.h>

//include self defined lib
#include "wrlog.h"

//macro define

//#define EnvFileName "/filelog.env"
#define ENV_FILENAME "/phs.env"

//declare internal function
int wrlogproc(int flag,char* buffer);

void int_to_str(long integer,char result[]);

//declare datatype

//declare global variable
// 2000-12-27 by wh
char EnvFilePath[ENVFILEPATHLEN+1]="/usr/net/env/filelog.env";
char LOGFILEPATH[256];
char MSGFILEPATH[256];
char LocalIp[16+1],chHostName[255];

/******************************************************************
desciption:
 convert long int type to string type
input:
 path - the env file path
output:
 none
return:
 none
******************************************************************/


char Topdate[20],Toptime[20];


int TGetTime( int num )
{
 time_t  timer;
 struct tm nowtimer;
 time( &timer ); timer += num;
 nowtimer = *localtime ( &timer ); nowtimer.tm_mon++;

    sprintf( Toptime,"%02u%02u%02u", nowtimer.tm_hour,nowtimer.tm_min ,
                                   nowtimer.tm_sec );
    sprintf( Topdate,"%04u%02u%02u", nowtimer.tm_year+1900,nowtimer.tm_mon,
                                   nowtimer.tm_mday );

 return(0);
}



int wrlog_setEnvPath(char *path)
{
  if (path != NULL)
  {
    strcpy(EnvFilePath, path); 
    int len;
    len = strlen(EnvFilePath);
  //  if (EnvFilePath[len-1] == '/') 
    EnvFilePath[len] = 0;
  }
  FILE *fp;
  char desc[80];
  int len;
  int dirflag=0;
  if((fp=fopen(EnvFilePath,"r"))==NULL)
   	return OPENENVERROR; 

 	while(!feof(fp))
  {
    fscanf(fp,"%s",desc);
    if( strcmp(desc,ERRLOG_PATH)==0)
    {	 
      fscanf(fp,"%s",LOGFILEPATH);
     				 // 2000-12-30 by wh
      len = strlen(LOGFILEPATH);
  	  if (LOGFILEPATH[len-1] == '/') LOGFILEPATH[len-1]='\0';
     	dirflag=1;
    }
  }//end while	
	if(dirflag!=1)   
 	return ENVFILEERROR;
 	fclose(fp);

 	
  dirflag=0;
  if((fp=fopen(EnvFilePath,"r"))==NULL)
   	return OPENENVERROR; 
 	while(!feof(fp))
  {
    fscanf(fp,"%s",desc);
    if(!strcmp(desc,MSGLOG_PATH))
    {	 
      fscanf(fp,"%s",MSGFILEPATH);
      len = strlen(MSGFILEPATH);
  	  if (MSGFILEPATH[len-1] == '/') MSGFILEPATH[len-1]='\0';
     	dirflag=1;
    }
  }
  
	if(dirflag!=1)   
 	return ENVFILEERROR;
 	fclose(fp);
 	struct in_addr addr;
 	/*取得机器名称*/
	if(gethostname(chHostName,sizeof(chHostName)) !=-1)
	{
		/*取得给定机器的信息*/
		struct hostent* phost=gethostbyname(chHostName);
		for(int i=0;phost!= NULL&&phost->h_addr_list[i]!=NULL;i++)
		{
      memcpy(&addr, phost->h_addr_list[i],sizeof(struct in_addr));
    }
    strcpy(LocalIp,inet_ntoa(addr));
	}
  else
    return ENVFILEERROR;
  return 0;
//  strcat(EnvFilePath, ENV_FILENAME);
}
/******************************************************************
desciption:
 convert long int type to string type
input:
 a long int type integer
output:
 a string as the convert result
return:
 none
******************************************************************/

void int_to_str(long integer,char result[])
  {
/*     if(integer<0)
	{ strcpy(result," "); return;}
     if(integer==0)	
	{ strcpy(result,"0"); return;}
  
     char temp[10];
     int p=0;
     int top;
     
     while(integer>=1)
        {
          temp[p]=integer%10+48;
          integer=integer/10;
          p++;
        }
     top=p;
     while(p>=1)
      {
      	result[top-p]=temp[p-1];
        p--;
      }
  
     result[top]='\0';
     */
sprintf(result,"%d",integer);
result[strlen(result)]=0;

  }

void int_to_str(int integer,char result[])
  {
/*    
     if(integer==0)	
	{ strcpy(result,"0"); return;}
  
     char temp[6];
     int p=0;
     int top;
     
     while(integer>=1)
        {
          temp[p]=integer%10+48;
          integer=integer/10;
          p++;
        }
     top=p;
     while(p>=1)
      {
      	result[top-p]=temp[p-1];
        p--;
      }
  
     result[top]='\0';
  */
  sprintf(result,"%d",integer);
result[strlen(result)]=0;
}

void DeleteChr(char *ss,char c)
{
	/*get rid of the tail space*/
	int i;
	i = strlen(ss)-1;
	while (i && ss[i] == c) i--;
	ss[i+1] = 0;
	/*get rid of the head space*/
	i = 0;
	int len = strlen(ss);
	while((i<len)&&ss[i]==c)i++;
	if(i!=0)
	{
	for(int j = i;j<len;j++)
	{
	   ss[j-i] = ss[j];
	}
	}
	ss[len-i] = 0;
}
/*****************************************************************
description:
 complete the write log operation,three forms for three kinds of log
input:
 contents of the log needed to write
output:
 none
return:
 operation result or error code
*****************************************************************/

int wrlog(
          char* tollcode,
          int opl_modul_id,
          char* opl_filename,
          char* starttime,
          char* endtime,
          long opl_total_count,
          long opl_mainflow_count,
          long opl_lackinfo_count,
          long opl_error_count
         )
   {
   	char buffer[350];
        char id[6];
        char result1[10];
        char result2[10];
        char result3[10];
        char result4[10];
        int returnvalue;
        
        int_to_str(opl_total_count,result1);
        int_to_str(opl_mainflow_count,result2);
        int_to_str(opl_lackinfo_count,result3);
        int_to_str(opl_error_count,result4);
        int_to_str(opl_modul_id,id);
        
        strcpy(buffer,tollcode);
        strcat(buffer,",");
        strcat(buffer,id);
 	strcat(buffer,",");
 	strcat(buffer,opl_filename);
 	strcat(buffer,",");
 	strcat(buffer,starttime);
 	strcat(buffer,",");
 	strcat(buffer,endtime);
 	strcat(buffer,",");
 	strcat(buffer,result1);
	strcat(buffer,",");
 	strcat(buffer,result2);
 	strcat(buffer,",");
 	strcat(buffer,result3);
	strcat(buffer,",");
 	strcat(buffer,result4);
 
        returnvalue=wrlogproc(1,buffer);
        return returnvalue;

     }//end of function prototype 1


int wrlog(
          char* tollcode,
          char* gat_filename,
          char* gat_receivetime,
          char* gat_endpoint,
          char gat_state,
          char gat_direction
         )
    {
    	char buffer[350];
 	char state[2];
 	char direction[2];
 	int returnvalue;
 	
 	state[0]=gat_state;
 	state[1]='\0';
 	direction[0]=gat_direction;
 	direction[1]='\0';
 
        strcpy(buffer,tollcode);
        strcat(buffer,",");
 	strcat(buffer,gat_filename);
 	strcat(buffer,",");
 	strcat(buffer,gat_receivetime);
 	strcat(buffer,",");
 	strcat(buffer,gat_endpoint);
 	strcat(buffer,",");
 	strcat(buffer,state);
 	strcat(buffer,",");
 	strcat(buffer,direction);
 
 	returnvalue=wrlogproc(2,buffer);
 	return returnvalue;
    }//end of function prototype2


//32

int wrlog(
          char* tollcode,
          int erl_modul_id,
          char* erl_filename,
          char erl_cat,
          char erl_level,
          int erl_code1,
          int erl_code2,
          char* erl_time,
          char* erl_desc,
          char* errHappenFile,
          int errHappenLine
         )
     {   
     	char buffer[1200];
 	long inte[4];
 	char cat[2];
 	char level[2];
 	int returnvalue;
 	char result1[10];
 	char result2[10];
 	
 	cat[0]=erl_cat;
 	cat[1]='\0';
 	level[0]=erl_level;
 	level[1]='\0';

        char id[6];
        int_to_str(erl_modul_id,id);
        
        
 	strcpy(buffer,tollcode);
 	strcat(buffer,",");
 	strcat(buffer,id);
 	strcat(buffer,",");
	strcat(buffer,erl_filename);
	strcat(buffer,",");
 	strcat(buffer,cat);
 	strcat(buffer,",");
 	strcat(buffer,level);
 	strcat(buffer,",");
	if(erl_code1<0)
           {
           	erl_code1=-erl_code1;
	        sprintf(result1,"%05d",erl_code1);
                //int_to_str(erl_code1,result1);
                //strcat(buffer,"-");
 		strcat(buffer,result1);
	   }
        else
           { 
           //	int_to_str(erl_code1,result1);
           sprintf(result1,"%05d",erl_code1);
  		strcat(buffer,result1);
           }

        strcat(buffer,",");

        if(erl_code2<0)
           {
           	erl_code2=-erl_code2;
 		int_to_str(erl_code2,result2);
 		strcat(buffer,"-");
 		strcat(buffer,result2);
	   }
        else
           { 
           	int_to_str(erl_code2,result2);
                strcat(buffer,result2);
	   }

        strcat(buffer,",");
        if(erl_time==NULL)
        	{
        	char tt[20];	
       		TGetTime(0); sprintf( tt,"%s%s",Topdate,Toptime );
	        strcat(buffer,tt);
        	}
	        else strcat(buffer,erl_time);
	        
        strcat(buffer,",");
        strcat(buffer,erl_desc);
        int line=errHappenLine;
        if(line>0)
        	{strcat(buffer," ");
	        strcat(buffer,errHappenFile);
		char _line[10];
		int_to_str(line,_line);
	        strcat(buffer," ");
	        strcat(buffer,_line);
        	}
        
        returnvalue=wrlogproc(3,buffer);
        return returnvalue;
      }//end of function prototype3


//34

int wrlog(char* tollcode,int erl_modul_id,char* erl_filename,char* erl_time,CF_CError & CError)
{
  char buffer[1200];
  char tmp[1024];
  long inte[4];
 	char cat[2];
 	char level[2];
 	int returnvalue;
 	char result1[10];
 	char result2[10];
 	
 	cat[0]=CError.get_errType();
 	cat[1]='\0';
 	level[0]=CError.get_errLevel();
 	level[1]='\0';
 
  char id[6];
  int_to_str(erl_modul_id,id);
        
 	strcpy(buffer,tollcode);
 	strcat(buffer,",");
 	strcat(buffer,id);
 	strcat(buffer,",");
	strcat(buffer,erl_filename);
	strcat(buffer,",");
 	strcat(buffer,cat);
 	strcat(buffer,",");
 	strcat(buffer,level);
 	strcat(buffer,",");


	int erl_code1;
	erl_code1=CError.get_appErrorCode();
	if(erl_code1<0)
  {
    erl_code1=-erl_code1;
    sprintf(result1,"%05d",erl_code1);
    //int_to_str(erl_code1,result1);
    //strcat(buffer,"-");
 		strcat(buffer,result1);
	}
  else
  { 
    //	int_to_str(erl_code1,result1);
    sprintf(result1,"%05d",erl_code1);
  	strcat(buffer,result1);
  }

  strcat(buffer,",");

	int erl_code2;
	erl_code2=CError.get_osErrorCode();
  if(erl_code2<0)
  {
    erl_code2=-erl_code2;
 		int_to_str(erl_code2,result2);
 		strcat(buffer,"-");
 		strcat(buffer,result2);
  }
        else
           { 
           	int_to_str(erl_code2,result2);
                strcat(buffer,result2);
	   }

        strcat(buffer,",");
        if(erl_time==NULL)
        	{
        	char tt[20];	
       		TGetTime(0); sprintf( tt,"%s%s",Topdate,Toptime );
	        strcat(buffer,tt);
        	}
	        else strcat(buffer,erl_time);
	strcat(buffer,",");
	if(strlen(CError.get_errMessage())>=1024)
	{
		strncpy(tmp,CError.get_errMessage(),1000);
		tmp[1000]=0;
	}
	else strcpy(tmp,CError.get_errMessage());
	DeleteChr(tmp,'\r');
	DeleteChr(tmp,'\n');
        strcat(buffer,tmp);
        int line=CError.get_errHappenLine();
        if(line>0)
        	{strcat(buffer," ");
	        strcat(buffer,CError.get_errHappenFileName());
		char _line[10];
		int_to_str(line,_line);
		strcat(buffer," ");
	        strcat(buffer,_line);
        	}
        
        returnvalue=wrlogproc(3,buffer);
        return returnvalue;

}
/******************************************************************************/
//add by zhoulh 20050704
//增加接口
//35

int wrlog(
          char* tollcode,
          char* erl_modul_id,
          char* erl_filename,
          char erl_cat,
          char erl_level,
          int erl_code1,
          int erl_code2,
          char* erl_time,
          char* erl_desc,
          char* errHappenFile,
          int errHappenLine
         )
     {   
     	char buffer[1200];
 	long inte[4];
 	char cat[2];
 	char level[2];
 	int returnvalue;
 	char result1[10];
 	char result2[10];
 	
 	cat[0]=erl_cat;
 	cat[1]='\0';
 	level[0]=erl_level;
 	level[1]='\0';

        char id[6];
//        int_to_str(erl_modul_id,id);
        
        
 	strcpy(buffer,tollcode);
 	strcat(buffer,",");
 	strcat(buffer,erl_modul_id);
 	strcat(buffer,",");
	strcat(buffer,erl_filename);
	strcat(buffer,",");
 	strcat(buffer,cat);
 	strcat(buffer,",");
 	strcat(buffer,level);
 	strcat(buffer,",");
	if(erl_code1<0)
           {
           	erl_code1=-erl_code1;
	        sprintf(result1,"%05d",erl_code1);
                //int_to_str(erl_code1,result1);
                //strcat(buffer,"-");
 		strcat(buffer,result1);
	   }
        else
           { 
           //	int_to_str(erl_code1,result1);
           sprintf(result1,"%05d",erl_code1);
  		strcat(buffer,result1);
           }

        strcat(buffer,",");

        if(erl_code2<0)
           {
           	erl_code2=-erl_code2;
 		int_to_str(erl_code2,result2);
 		strcat(buffer,"-");
 		strcat(buffer,result2);
	   }
        else
           { 
           	int_to_str(erl_code2,result2);
                strcat(buffer,result2);
	   }

        strcat(buffer,",");
        if(erl_time==NULL)
        	{
        	char tt[20];	
       		TGetTime(0); sprintf( tt,"%s%s",Topdate,Toptime );
	        strcat(buffer,tt);
        	}
	        else strcat(buffer,erl_time);
	        
        strcat(buffer,",");
        strcat(buffer,erl_desc);
        int line=errHappenLine;
        if(line>0)
        	{strcat(buffer," ");
	        strcat(buffer,errHappenFile);
		char _line[10];
		int_to_str(line,_line);
	        strcat(buffer," ");
	        strcat(buffer,_line);
        	}
        
        returnvalue=wrlogproc(3,buffer);
        return returnvalue;
      }//end of function prototype3


//36

int wrlog(char* tollcode,char* erl_modul_id,char* erl_filename,char* erl_time,CF_CError & CError)
{
  char buffer[1200];
  char tmp[1024];
  long inte[4];
 	char cat[2];
 	char level[2];
 	int returnvalue;
 	char result1[10];
 	char result2[10];
 	
 	cat[0]=CError.get_errType();
 	cat[1]='\0';
 	level[0]=CError.get_errLevel();
 	level[1]='\0';
 
  char id[6];
//  int_to_str(erl_modul_id,id);
        
 	strcpy(buffer,tollcode);
 	strcat(buffer,",");
 	strcat(buffer,erl_modul_id);
 	strcat(buffer,",");
	strcat(buffer,erl_filename);
	strcat(buffer,",");
 	strcat(buffer,cat);
 	strcat(buffer,",");
 	strcat(buffer,level);
 	strcat(buffer,",");


	int erl_code1;
	erl_code1=CError.get_appErrorCode();
	if(erl_code1<0)
  {
    erl_code1=-erl_code1;
    sprintf(result1,"%05d",erl_code1);
    //int_to_str(erl_code1,result1);
    //strcat(buffer,"-");
 		strcat(buffer,result1);
	}
  else
  { 
    //	int_to_str(erl_code1,result1);
    sprintf(result1,"%05d",erl_code1);
  	strcat(buffer,result1);
  }

  strcat(buffer,",");

	int erl_code2;
	erl_code2=CError.get_osErrorCode();
  if(erl_code2<0)
  {
    erl_code2=-erl_code2;
 		int_to_str(erl_code2,result2);
 		strcat(buffer,"-");
 		strcat(buffer,result2);
  }
        else
           { 
           	int_to_str(erl_code2,result2);
                strcat(buffer,result2);
	   }

        strcat(buffer,",");
        if(erl_time==NULL)
        	{
        	char tt[20];	
       		TGetTime(0); sprintf( tt,"%s%s",Topdate,Toptime );
	        strcat(buffer,tt);
        	}
	        else strcat(buffer,erl_time);
	strcat(buffer,",");
		if(strlen(CError.get_errMessage())>=1024)
	{
		strncpy(tmp,CError.get_errMessage(),1000);
		tmp[1000]=0;
	}
	else strcpy(tmp,CError.get_errMessage());
	DeleteChr(tmp,'\r');
	DeleteChr(tmp,'\n');
        strcat(buffer,tmp);
        int line=CError.get_errHappenLine();
        if(line>0)
        	{strcat(buffer," ");
	        strcat(buffer,CError.get_errHappenFileName());
		char _line[10];
		int_to_str(line,_line);
		strcat(buffer," ");
	        strcat(buffer,_line);
        	}
        
        returnvalue=wrlogproc(3,buffer);
        return returnvalue;

}
//end add

/*****************************************************************************/
int msglog(char *infoId,char *checkName,char *infoValue,char *objName,char *batch,char *begTime,char *endTime)
{
	char buff[255],obpath[255];
	char infoLog[255],msg[400];
  int i;
	FILE *fp;
  TGetTime(0);
  sprintf(obpath,"%s/%s",MSGFILEPATH,Topdate);
  mkdir(obpath,0777);
  chmod(obpath,0777);
  strcpy(infoLog,obpath);
  strcat(infoLog,"/");
  if(strlen(infoId)>20) strncat(infoLog,infoId,20);
  else
    strcat(infoLog,infoId);
  strcat(infoLog,".");
  if(strlen(checkName)>20) strncat(infoLog,checkName,20);
  else 
    strcat(infoLog,checkName);
  strcat(infoLog,".infolog");
  //sprintf(infoLog,"%s/%s.%s.infolog",obpath,infoId,checkName);
  if(strlen(infoId)>20)
  {
  	strncpy(msg,infoId,20);
  	msg[20]=0;
  }
  else strcpy(msg,infoId);
  strcat(msg,";");
  if(strlen(infoValue)>20) strncat(msg,infoValue,20);
  else
    strcat(msg,infoValue);
  strcat(msg,";");
  if(strlen(objName)>256) strncat(msg,objName,256);
  else
    strcat(msg,objName);
  strcat(msg,";");
  if(strlen(batch)>8) strncat(msg,batch,8);
  else
    strcat(msg,batch);
  strcat(msg,";");
  strcat(msg,LocalIp);
  strcat(msg,";");
  strcat(msg,chHostName);
  strcat(msg,";");
  if(strlen(begTime)>14) strncat(msg,begTime,14);
  else
    strcat(msg,begTime);
  strcat(msg,";");
  if(strlen(endTime)>14) strncat(msg,endTime,14);
  else
    strcat(msg,endTime);
	if((fp=fopen(infoLog,"a+"))==NULL)
	{
    return WRITE_INFLOG_ERROR;
	}
	if(fputs(msg,fp)==EOF)
	{
    fclose(fp);
    return WRITE_INFLOG_ERROR;
	}
	putc('\n',fp);
	fclose(fp);
	return 0;
}
