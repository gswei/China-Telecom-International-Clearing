#include "CF_FileScan.h"
/***********************************
2007-12-11   增加构造函数并对filelist进行为NULL的判断.
************************************/
CF_FileScan::CF_FileScan()
{
  filelist=NULL;
}

int CF_FileScan::scan_file(char *fdir,int num,char *tmp)
{
	int i;
	
	for(i=0;i<11;i++)
	  fmt[i][0]=0;
	filterNum=0;
	strcpy(scanPath,fdir);
	if(scanPath[strlen(scanPath)-1]=='/') scanPath[strlen(scanPath)-1]=0;
	if (tmp==NULL) 
	{
		strcpy(fmt[1],"*");
		filterNum=1;
	}
	else 
	{
		sscanf(tmp,"%s %s %s %s %s %s %s %s %s %s",fmt[1],fmt[2],fmt[3],
		  fmt[4],fmt[5],fmt[6],fmt[7],fmt[8],fmt[9],fmt[10]);
		for(i=1;i<11;i++)
		  if(fmt[i][0]!=0) filterNum++;
	}
	floor=0;
	add_count=100;
	if(filelist==NULL)
	{  
		filelist=new FILE_INFO[MAX_NUM+1];
		max_count=MAX_NUM;
	}
	file_count=0;
	flag=0;
	if(num<1) return 0;
	if(pro_file(scanPath,num)==-1)
	  return -1;
	return 0;
}

int CF_FileScan::pro_file(char *fdir,int num)
{
  char temp[255],tmpName[255],buff[255];
  int i;
	struct dirent  *sdir;
  struct  stat ustat;
  DIR *fp;
  floor++;
  if((fp=opendir(fdir))==NULL)
  {
  	printf("open dir(%s) error\n",fdir);
  	floor--;
  	return (-1);
  }
  while((sdir=readdir(fp))!= NULL)
  {
    if(!sdir->d_ino) continue;
    sprintf(tmpName,"%s/%s",fdir,sdir->d_name);
    if(stat(tmpName,&ustat))
    {
     	continue;
    }
    if(ustat.st_mode & 0040000)
    {
      if(sdir->d_name[0]=='.') continue;
      sprintf(buff,"%s/%s",fdir,sdir->d_name);
      if(floor<num) pro_file(buff,num);
      continue;
    }
    bool ret;
    ret=false;
    for(i=1;i<=filterNum;i++)
    {
    	if(checkFormat(sdir->d_name,fmt[i]))
    	{
    		ret=true;
    		break;
    	}
    }
    if(ret==false) continue;
    sprintf(filelist[file_count].filename,"%s/%s",fdir,sdir->d_name);
    file_count++;
    if(file_count==max_count)
    {
      FILE_INFO *tmp=new FILE_INFO[max_count+add_count];
      for(i=0;i<(file_count);i++)
      {
        strcpy(tmp[i].filename,filelist[i].filename);
      }
      delete []filelist;
      filelist = tmp;
      tmp = NULL;
      max_count=add_count+ max_count;
    }
  }
  closedir(fp);
  floor--;
  return 0;
}

int CF_FileScan::get_file(char *tmp)
{
	if(flag==file_count) return 100;
	strcpy(tmp,filelist[flag].filename);
	flag++;
	return 0;
}

bool CF_FileScan::checkFormat(const char *cmpString,const char *format)
{
	while(1)
	{
		switch(*format)
	  {
	  	case '\0':
				if(*cmpString == '\0')
				{
					return true;
				}
				else
				{
					return false;
				}
			case '!':
				if(checkFormat(cmpString,format + 1)==true)
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
			default:
				if(*cmpString == *format)
				{
					return checkFormat(cmpString+1,format+1);
				}
				else
				{
					return false;
				}
		}
	}
}

CF_FileScan::~CF_FileScan()
{
	if (filelist!=NULL)
	{
		delete []filelist;
		filelist=NULL;
	}
}