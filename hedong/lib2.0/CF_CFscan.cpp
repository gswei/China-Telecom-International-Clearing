/****************************************************************
filename: CF_CFscan.ec
module: CF - Common Function
created by: wang hui
create date: 2001-04-04
update list: 
version: 1.1.1
description:
    the header file of the classes for CF_CFscan
    2006-01-23 ����closeʱ�Ա�־flag=0���㴦����ֹ������ʱ�����ιر�
    2006-4-17  ���ӻ�ȡ�ļ�����ʱ���ȡ����ͨ�䣬ÿ���ÿո�ֿ�  ��ADD BY ZGQ��
    2006-5-18  ����sz_VarBuf����ʱ����ղ�����
    2008-5-21  �Զ�ͨ�����ʱ����ո���ѭ���Ĵ���
*****************************************************************/



#include "CF_CFscan.h"

// include system lib


//#include <it.h>

// include self defined lib
#include "CF_CFscan.h"

// macro define

// declare internal function

// declare datatype

// declare global variable 
CF_CFscan::CF_CFscan()
{
	flag=0;
	sz_VarBuf[0]='\0';
	iVarNum=0;
}

CF_CFscan::~CF_CFscan()
{
	if (flag == 1)
	{
		closedir(dirfp);	
	}
}	

/***************************************************
description:
   scan and get the first file in the dir
input:
  dirName - the directory path
  fileString - the string in the fileName to scan
output:
  fileName - the filename begin scaned
return
  SUC/FAIL;
*****************************************************/
int CF_CFscan::openDir(char* dirName)
{
 int   i,ll;
  flag = 0;  //flag == 0 ��ʾû�д�Ŀ¼
  if( (dirfp=opendir( dirName )) == NULL) {
  	char msg[300];
  	sprintf(msg, "access directory (%s) fail !", dirName);
  	perror(msg);
  	return FAIL;
   }
  else {
  	flag = 1;
  	strcpy(dir, dirName);
  	return SUC;
   }
   	
}   // openDir()

/***************************************************
description:
   scan and get the next file in the dir
input:
  format - ƥ����ַ�����֧��*,?,[]��ͨ���
  		
output:
  fileName - the fileName to be scaned 
return
  -1 -- can't stat the file
  0 -- success
  100 -- not get filename
*****************************************************/
int CF_CFscan::getFile(const char *format, char *fileName)
{
 int   i,ll;
 struct dirent  *sdir;
 struct stat    ustat;

  char tmpName[250];
  memset(tmpName,0,250);
  int len;
  if(strcmp(format,sz_VarBuf)!=0)
  {
  	strcpy(sz_VarBuf,format);
    iVarNum=SplitBuf(format,' ');
    if(iVarNum<0) return -1;
   }

  if (format == NULL)
     len = 0;
  else
     len = strlen(format);
  
  while((sdir=readdir(dirfp))!= NULL)
  {
    if(!sdir->d_ino) continue;
//***************modified by tanj 2004.11.25 ����·�������Դ���'/'�����
    //sprintf(tmpName,"%s/%s",dir,sdir->d_name);
    strcpy(tmpName, dir);
    if (tmpName[strlen(tmpName) - 1] != '/')
    {
     	strcat(tmpName, "/");
    }
    strcat(tmpName, sdir->d_name);
//***************end of modified ********************************
    if( stat(tmpName, &ustat) )
    { 
     	//closedir(dirfp);
     	return( -1 ); 
    }
    if( ustat.st_mode & 0040000 ) continue;

    if(len>0)
    {
     	/* char *ch = strstr(sdir->d_name, format);
     	if (&&ch == NULL) 
     		continue; */
      for(int j=0;j<iVarNum;j++)
      {
     	  int ret = checkFormat(sdir->d_name, sz_Var[j]);
     	  if (ret == true)
     	  {
     	  	strcpy(fileName, tmpName);
           return 0;
     	  }
      }
      continue;
    }
    else
    {
    	strcpy(fileName, tmpName);
      return 0;
    }
  }

  return 100;
}   // getFile()


/***************************************************
description:
   close dir
input:
  none 
output:
  none
return
  none
*****************************************************/

void CF_CFscan::closeDir()
{
  closedir(dirfp);
  flag=0;
}	

/***************************************************
description:
   remove the dirtp to the beginning
input:
  pos - the position to the begin 
output:
  none
return
  none
*****************************************************/

void CF_CFscan::rewind()
{
  rewinddir(dirfp);
}	

/**************************************************
*	Function Name:	checkFormat
*	Description:	�Ƚ������ַ����Ƿ�ƥ�䣨��ȣ�
*	Input Param:
*		cmpString -------> ���Ƚϵ��ַ���
*		format	   -------> ƥ����ַ�����֧��*,?,[]��ͨ���
*	Returns:
*		��������ַ���ƥ�䣬����SUC
*		��������ַ�����ƥ�䣬����FAIL
*	complete:	2001/12/13
******************************************************/
bool CF_CFscan::checkFormat(const char *cmpString,const char *format)
{
	while(1)
	{
		switch(*format)
	  	{
	  		case '\0':
					if (*cmpString == '\0')
					{
						return true;
					}
					else
					{
						return false;
					}
			case '!':
					if (checkFormat(cmpString,format + 1) == true)
					{
						return false;
					}
					else
					{
						return true;
					}
			case '?' :
					if(*cmpString == '\0')
					{
						return false;
					}
					return checkFormat(cmpString + 1,format + 1);
			case '*' :
					if(*(format+1) == '\0')
					{
						return true;
					}
					do
					{
						if(checkFormat(cmpString,format+1)==true)
						{
							return true;
						}
					}while(*(cmpString++));
					return false;
			case '[' :
					format++;
					do
					{
						
						if(*format == *cmpString)
						{
							while(*format != '\0' && *format != ']')
							{
								format++;
							}
							if(*format == ']')
							{
								format++;
							}
							return checkFormat(cmpString+1,format);			
						}
						format++;
						if((*format == ':') && (*(format+1) != ']'))
						{
							if((*cmpString >= *(format - 1)) && (*cmpString <= *(format + 1)))
							{
								while(*format != '\0' && *format != ']')
								{
									format++;
								}
								if(*format == ']')
								{
									format++;
								}
								return checkFormat(cmpString+1,format);
							}
							format++;
							format++;

						}
					}while(*format != '\0' && *format != ']');

					return false;
			default  :
					if(*cmpString == *format)
					{
						return checkFormat(cmpString+1,format+1);
					}
					else
					{
						return false;
					}
		}//switch
	}//while(1)
}

//add by lwu 2005-06-23
/***************************************************
description:
   scan and get the next file in the dir
input:
  dirName - the directory path
output:
  none
return
  -1 -- can't stat the file
  0 -- success
  100 -- not get filename
*****************************************************/
int CF_CFscan::getFile(char *fileName)
{
 int   i,ll;
 char  tmp1[250];
 char  *tmp2;
 struct dirent  *sdir;
 struct stat    ustat;

 memset(tmp1,0,250);
  while( (sdir=readdir(dirfp)) != NULL ) { //while dir is not empty
       if( !sdir->d_ino ) continue;          //the file has not d_ino value
       sprintf(tmp1,"%s%s",dir,sdir->d_name);
   
     if( stat(tmp1, &ustat) )  { 
     	//closedir(dirfp);
     	return( -1 ); 
     }
     if(!( ustat.st_mode & 0040000 )) continue;
     if ( !strcmp(sdir->d_name,".") || !strcmp(sdir->d_name,"..") ) continue;
     //tmp2 = strrchr(tmp1,'/');
     //if(tmp2!=NULL) strcpy(fileName,tmp2+1);  // get the filename contains fileString
     //else 
     strcpy(fileName,tmp1);
     return 0; 
  }
  
  return 100;
}   // getFile()


//add by zgq
/*****************************************************
output:
  fileName - the fileName to be scaned 
return
  -1 -- can't stat the file
  0 -- success
  100 -- not get Dir
*****************************************************/
int CF_CFscan::getDir(char *format, char *fileName)
{
 int   i,ll;
 struct dirent  *sdir;
 struct stat    ustat;

  char tmpName[250];
  memset(tmpName,0,250);
  int len;
  
  if (format == NULL)
     len = 0;
  else
     len = strlen(format);
  
  while( (sdir=readdir(dirfp)) != NULL )
  {
     if( !sdir->d_ino ) continue;
//***************modified by tanj 2004.11.25 ����·�������Դ���'/'�����
     //sprintf(tmpName,"%s/%s",dir,sdir->d_name);
     strcpy(tmpName, dir);
     if (tmpName[strlen(tmpName) - 1] != '/')
     {
     	 strcat(tmpName, "/");
     }
     strcat(tmpName, sdir->d_name);
//***************end of modified ********************************
     if( stat(tmpName, &ustat) ) 
     { 
     	//closedir(dirfp);
     	return( -1 ); 
     }
     if(!( ustat.st_mode & 0040000 )) continue;
     
     if (len > 0)
     {
     	/* char *ch = strstr(sdir->d_name, format);
     	if (&&ch == NULL) 
     		continue; */
     		
     	int ret = checkFormat(sdir->d_name, format);
     	if (ret == false)
     	   continue; 
     }
     	     
     strcpy(fileName, tmpName);
     return 0;
  }
  return 100;
}


int CF_CFscan::SplitBuf(const char *pch_Buf,char cWord)
{
  char *pch_tmp;
  char sz_buff[400];
  int i;
  
  i=0;
  strcpy(sz_buff,pch_Buf);
  while(1)
  {
    int iLen;
    if((pch_tmp=strchr(sz_buff,cWord))==NULL) break;
    iLen=strlen(sz_buff)-strlen(pch_tmp);
    if(iLen==0)
    {
      strcpy(sz_buff,pch_tmp+1);
    	continue;
    }
    if(i==20) return -1;
    strncpy(sz_Var[i],sz_buff,iLen);
    sz_Var[i][iLen]='\0';
    strcpy(sz_buff,pch_tmp+1);
    i++;
  }
  if(strlen(sz_buff)!=0)
  {
  	if(i==20) return -1;
    strcpy(sz_Var[i],sz_buff);
    i++;
  }
  return i;
}
