/****************************************************************
filename: CFileIn.cpp
module: CF - Common Function
created by: zhang guoqiang
create date: 2004-09-09
update list: 
version: 1.0.0
description:
    the fuctions of the class for CFileIn
*****************************************************************/
#include "CFileIn.h"

/***************************************************
description:
  文件的打开操作;
input:
  fileName          要打开的文件路径和文件名；	
output:
  none;
return
  SUCC              函数执行成功
  ERR_IN_FILE_CLOED 文件处于关闭状态
  ERR_IN_FILE_HEAD  文件头错误
  ERR_IN_FILE_BYTE  文件字节数检测错误
  ERR_IN_FILE_END   文件尾错误
  ERR_IN_FILE_NUM   文件记录条数错误
  ERR_IN_FILE_OPEN  打开文件失败
*****************************************************/

int CFileIn::open(char *fileName)
{
  char mid[MAX],ch,msg[MAX],temp[7];
 	openSign=1;
	if((fp=fopen(fileName,"r"))==NULL)//打开文件检测；
	{
		 openSign=0;
		 sprintf(msg, "Error in open the file(%s)\n",fileName);
  	 perror(msg);
  	 throw CF_CError('O','H',ERR_IN_FILE_OPEN,errno,msg,__FILE__,__LINE__);
		 return ERR_IN_FILE_OPEN;
	}
	if(getc(fp)!='S'||getc(fp)!='O'||getc(fp)!='F')//文件头检测
	{
	  fclose(fp);
	  openSign=0;
	  sprintf(msg, "the file(%s) head error\n",fileName);
	  perror(msg);
	  throw CF_CError('O','H',ERR_IN_FILE_HEAD,errno,msg,__FILE__,__LINE__);
	  return ERR_IN_FILE_HEAD;
	}
  fgets(mid,11,fp); 
  fseek(fp,0L,SEEK_END);
	if(ftell(fp)!=atoi(mid))
	{
	  fclose(fp);
	  openSign=0;
	  sprintf(msg, "the bytes num of the file(%s) head error\n",fileName);
  	perror(msg);
	  throw CF_CError('O','H',ERR_IN_FILE_BYTE,errno,msg,__FILE__,__LINE__);
	  return ERR_IN_FILE_BYTE;
	}
	fseek(fp,13L,SEEK_SET);
	if(getc(fp)!='\n')
	{
	 	fclose(fp);
	 	openSign=0;
	 	sprintf(msg, "the file(%s) head error\n",fileName);
  	perror(msg);
  	throw CF_CError('O','H',ERR_IN_FILE_BYTE,errno,msg,__FILE__,__LINE__);
	 	return ERR_IN_FILE_HEAD;
	}	
	
	recNum=0;
	int flag=1;
  for(;flag!=0;recNum++)
	{
		if(fgets(mid,MAX,fp)==NULL) flag=0;
		if(strlen(mid)==10||strlen(mid)==11)
	  {
	  	if(mid[0]=='w'&&mid[1]=='E'&&mid[2]=='N'&&mid[3]=='D')
	  	{
	  		for(int i=0;i<6;i++)
	  		 temp[i]=mid[i+4];
	  		temp[6]='\0';
	  		if(recNum==atoi(temp))
	  		{
	  			fseek(fp,14,SEEK_SET);
	  			return SUCC;
	  		}
        else 
        {
         fclose(fp);
	       openSign=0;
	       sprintf(msg, "file(%s) record num error\n",fileName);
  	     perror(msg);
	       throw CF_CError('O','H',ERR_IN_FILE_NUM,errno,msg,__FILE__,__LINE__);
	       return ERR_IN_FILE_NUM;
        }
      }
    }
  }
  fclose(fp);
	openSign=0;
	sprintf(msg, "%dfile(%s) record num error\n",recNum,fileName);
  perror(msg);
	throw CF_CError('O','H',ERR_IN_FILE_NUM,errno,msg,__FILE__,__LINE__);
	return ERR_IN_FILE_NUM;
}
  
   
/*for(i=1;getc(c)!='\n';i++)
	fseek(fp,-i,SEEK_END);
	fseek(fp,-10-i,SEEK_END);//文件尾检测；
	if(getc(fp)!='\n'||getc(fp)!='w'||getc(fp)!='E'||getc(fp)!='N'||getc(fp)!='D')
	{
	  fclose(fp);
	  openSign=0;
	  sprintf(msg, "the file(%s) end error\n",fileName);
  	perror(msg); 
  	throw CF_CError(ERR_IN_FILE_END, errno, msg);
	  return ERR_IN_FILE_END;
	}
	fseek(fp,-1,SEEK_END);
	if(getc(fp)!='\n')
	{
	  fclose(fp);
	  openSign=0;
	  sprintf(msg, "the file(%s) end error\n",fileName);
  	perror(msg); 
  	throw CF_CError(ERR_IN_FILE_END, errno, msg);
	  return ERR_IN_FILE_END;
	}
	fseek(fp,14,SEEK_SET);//记录条数检测；	  
  recNum = 0; //1.全文检测记录条数，recNum为实际条数；
	while(fgets(mid,MAX,fp)!=NULL)
	  ++recNum;
	recNum=recNum-i+1;
	fseek(fp,-7L,SEEK_END);//2.获取文件中记录条数；
	fgets(mid,7,fp);
	if(atoi(mid)!=recNum)
	{
	  fclose(fp);
	  openSign=0;
	  sprintf(msg, "file(%s) record num error\n",fileName);
  	perror(msg);
	  throw CF_CError(ERR_IN_FILE_NUM, errno, msg);
	  return ERR_IN_FILE_NUM;
	}

	return SUCC;
}*/

/***************************************************
description:
  从文件中读取一条记录
input:
  &rec               类CFmt_Change CF的引用  		
output:
  none
return
  SUCC               成功读取记录
  ERR_IN_READ_REC    读取记录失败
  ERR_IN_FILE_CLOSED 文件处于关闭状态，不能读取
  ERR_IN_AT_END      读取文件读到文件尾了
*****************************************************/ 
int CFileIn::readRec(CFmt_Change &rec)
{
	char msg[MAX];
	char mid[MAX];
  if (openSign==0)
  {
    openSign=0;
    strcpy(mid,"");
     sprintf(msg, "the file is colsed\n");
 	  perror(msg);
 	  throw CF_CError('O','H',ERR_IN_FILE_CLOSED,errno,msg,__FILE__,__LINE__);
 	  return ERR_IN_FILE_CLOSED;
  }
  if(recNum==0)
  {
 	  strcpy(mid,"");
 	  return ERR_IN_AT_END;
  }
  if((fgets(mid,MAX,fp))==NULL)
  {
 	  fclose(fp);
    openSign=0;
    strcpy(mid,"");
 	  sprintf(msg, "the file read error\n");
 	  perror(msg);
 	  throw CF_CError('O','H',ERR_IN_READ_REC,errno, msg,__FILE__,__LINE__);
 	  return ERR_IN_READ_REC;
  }
  if(strcmp(mid,"\n")==0)
  {
  	fclose(fp);
    openSign=0;
    strcpy(mid,"");
 	  sprintf(msg, "the file read error\n");
 	  perror(msg);
 	  throw CF_CError('O','H',ERR_IN_READ_REC,errno,msg,__FILE__,__LINE__);
 	  return ERR_IN_READ_REC;
 	}
  rec.Set_record(mid);
  recNum--;
 return SUCC;
}

/***************************************************
description:
  close file
input:
  none 
output:
  none
return
  SUCC               成功关闭文件；
  ERR_IN_FILE_CLOSED 文件已经处于关闭状态；
*****************************************************/
int CFileIn::close()
{
	char msg[MAX];
  if (openSign==0)
  {
    sprintf(msg, "the file is closed\n");
 	  perror(msg);
 	  throw CF_CError('O','H',ERR_IN_FILE_CLOSED, errno,msg,__FILE__,__LINE__);
 	  return ERR_IN_FILE_CLOSED;
  }
  fclose(fp); 
  return SUCC;
}