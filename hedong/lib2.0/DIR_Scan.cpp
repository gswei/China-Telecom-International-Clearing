/****************************************************************
filename: DIR_Scan.ec
module: CF - Common Function
created by: wang hui
create date: 2001-04-04
update list: 
version: 1.1.1
description:
    the header file of the classes for DIR_Scan
*****************************************************************/



#include "DIR_Scan.h"

// include system lib
#include <stdio.h>
#include <iostream.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

//#include <it.h>

// include self defined lib
#include "DIR_Scan.h"

// macro define

// declare internal function

// declare datatype

// declare global variable 
DIR_Scan::~DIR_Scan()
{
  closedir(dirfp);	
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
int DIR_Scan::openDir(char* dirName)
{
 int   i,ll;
 
  if( (dirfp=opendir( dirName )) == NULL) {
  	char msg[300];
  	sprintf(msg, "access directory (%s) fail !", dirName);
  	perror(msg);
  	return FAIL;
   }
  else {
  	strcpy(dir, dirName);
  	return SUC;
   }
   	
}   // openDir()

/***************************************************
description:
   scan and get the next file in the dir
input:
  format - 匹配的字符串，支持*,?,[]等通配符
  flag  -  1：搜索文件;0：搜索目录		
output:
  fileName - the fileName to be scaned 
return
  -1 -- can't stat the file
  0 -- success
  100 -- not get filename
*****************************************************/
int DIR_Scan::getFile(char *format, int flag, char *fileName)
{
 int   i,ll;
 struct dirent  *sdir;
 struct stat    ustat;

  char tmpName[500];
  memset(tmpName,0,500);
  int len;
  
  if (format == NULL)
     len = 0;
  else
     len = strlen(format);
  
  while( (sdir=readdir(dirfp)) != NULL ) {
     if( !sdir->d_ino ) continue;
     if ( !strcmp(sdir->d_name,".")  || !strcmp(sdir->d_name,"..") ) continue;
     
     sprintf(tmpName,"%s/%s",dir,sdir->d_name);
     if( stat(tmpName, &ustat) )  { 
     	//closedir(dirfp);
     	return( -1 ); 
     }
     //if( flag && (ustat.st_mode & 0040000) ) continue;
     if( flag && S_ISDIR(ustat.st_mode ) ) continue;
     //else if ( !flag && !(ustat.st_mode & 0040000) ) continue;
     else if ( !flag && S_ISREG(ustat.st_mode ) ) continue;
     
     if (len > 0) {
     	/* char *ch = strstr(sdir->d_name, format);
     	if (&&ch == NULL) 
     		continue; */
     		
     	int ret = checkFormat(sdir->d_name, format);
     	if (ret == false)
     	   continue; 
     }
     	     
     //strcpy(fileName, tmpName);
     strcpy(fileName, sdir->d_name);
     return 0;
  }
  
  return 100;
}   // getFile()

/***************************************************
description:
   get one file and its type
input:
  none 
output:
  flag:  file type;1:directory;0:file
  fileName:  filename
return
  -1 -- can't stat the file
  0 -- success
  100 -- not get filename
*****************************************************/

int DIR_Scan::getOneFile(int &flag,char *fileName)
{
 struct dirent  *sdir;
 struct stat    ustat;

  char tmpName[500];
  memset(tmpName,0,500);

  while( (sdir=readdir(dirfp)) != NULL ) 
  {
		if( !sdir->d_ino ) continue;
		if ( !strcmp(sdir->d_name,".")  || !strcmp(sdir->d_name,"..") ) continue;
	
		sprintf(tmpName,"%s/%s",dir,sdir->d_name);
		if( stat(tmpName, &ustat) )  { 
		//closedir(dirfp);
		return( -1 ); 
		}
     
		if ( S_ISDIR(ustat.st_mode ) ) 
		{
     	flag = 1;
     	strcpy(fileName, sdir->d_name);
     	return 0;
    }
		else if ( S_ISREG(ustat.st_mode ) ) 
		{
			flag = 0;
     	strcpy(fileName, sdir->d_name);
     	return 0;
    }
		else continue;
	}
	return 100;
}

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

void DIR_Scan::closeDir()
{
  closedir(dirfp);	
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

void DIR_Scan::rewind()
{
  rewinddir(dirfp);
}	

/**************************************************
*	Function Name:	checkFormat
*	Description:	比较两个字符串是否匹配（相等）
*	Input Param:
*		cmpString -------> 被比较的字符串
*		format	   -------> 匹配的字符串，支持*,?,[]等通配符
*	Returns:
*		如果两个字符串匹配，返回SUC
*		如果两个字符串不匹配，返回FAIL
*	complete:	2001/12/13
******************************************************/
bool DIR_Scan::checkFormat(const char *cmpString,const char *format)
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

