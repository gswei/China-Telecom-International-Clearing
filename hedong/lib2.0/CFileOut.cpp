/****************************************************************
filename: CFileOut.cpp
module: CF - Common Function
created by: zhang guoqiang
create date: 2004-09-09
update list: 
version: 1.0.0
description:
    the fuctions of the class for CFileOut
*****************************************************************/
#include "CFileOut.h"

/***************************************************
description:
  文件的打开操作;
input:
  fileName 要打开的文件路径和文件名；  		
output:
  none;
return
   SUCC                   文件打开成功
   ERR_OUT_FILE_OPEN       文件打开错误
   ERR_OUT_WRITE_FILE_HEAD 写文件头错误
    
*****************************************************/

int CFileOut::open(char *fileName)//根据文件头尾判断打开是否成功;
{
	char msg[MAX];
  int i;
  memset(file_name,0,255);
  strcpy(file_name,fileName);
  recNum=0;
  openSign=1;
  if((fp=fopen(fileName,"w+"))==NULL)
  {
    openSign=0;
    sprintf(msg, "Error in open the file(%s)\n",fileName);
  	perror(msg);
  	throw CF_CError('O','H',ERR_OUT_FILE_OPEN, errno, msg,__FILE__,__LINE__);
  	return ERR_OUT_FILE_OPEN;
  }
	char *z1="SOF0000000000";
	if(fputs(z1,fp)==EOF)
	{
	  fclose(fp);
	  openSign=0;
	  sprintf(msg, "Error in  writing file head\n");
  	perror(msg);
  	throw CF_CError('O','H',ERR_OUT_WRITE_FILE_HEAD, errno, msg,__FILE__,__LINE__);
    return ERR_OUT_WRITE_FILE_HEAD;
  }
  if(putc('\n',fp)==EOF)
  {
    fclose(fp);
    openSign=0;
    sprintf(msg, "Error in  writing file head\n");
  	perror(msg);
    throw CF_CError('O','H',ERR_OUT_WRITE_FILE_HEAD, errno, msg,__FILE__,__LINE__);
    return ERR_OUT_WRITE_FILE_HEAD;
  }
	return SUCC;
}
/***************************************************
description:
  文件的写入操作;
input:
  rec                 类CFmt_Change CF的引用 		
output:
  none;
return
  SUCC                文件写入成功；
  ERR_OUT_WRITE_REC   文件写入失败
  ERR_OUT_FILE_CLOSED  文件处于关闭状态
*****************************************************/
		
int CFileOut::writeRec(CFmt_Change &rec)
{ 
	char msg[MAX];
	char ch[MAX];
  if(openSign==0)
  {
  sprintf(msg, "the file is closed\n");
  perror(msg);
  throw CF_CError('O','H',ERR_OUT_FILE_CLOSED, errno, msg,__FILE__,__LINE__);
  return ERR_OUT_FILE_CLOSED;
  }
  rec.Get_record(ch,MAX);
  if(fputs(ch,fp)==EOF){//写记录；
  	openSign=0;
  	fclose(fp);
  	sprintf(msg, "ERROR IN WRITE REC\n");
    perror(msg);
    throw CF_CError('O','H',ERR_OUT_WRITE_REC, errno, msg,__FILE__,__LINE__);
    return ERR_OUT_WRITE_REC;
  }
  putc('\n',fp);
  recNum++;
  return SUCC; 
}
 /***************************************************
description:
  文件的关闭操作;
input:
  fileName 要打开的文件路径和文件名； 		
output:
  none;
return
  SUCC                  文件成功关闭
  ERR_OUT_FILE_CLOSED    文件处于关闭状态
  ERR_OUT_WRITE_FILE_END 文件尾写失败
*****************************************************/
int CFileOut::close()
{
	char msg[MAX];
  int i;
  long temp;
  if(openSign==0)
  {
    sprintf(msg, "the file is closed\n");
    perror(msg);
   	throw CF_CError('O','H',ERR_OUT_FILE_CLOSED, errno, msg,__FILE__,__LINE__);
    return ERR_OUT_FILE_CLOSED;
  }
  char mid[11]="wEND000000";//写文件尾记录；
  fseek(fp,0L,SEEK_END);
  if(fputs(mid,fp)==EOF) {
  	fclose(fp);
  	sprintf(msg, "write the file end error\n");
    perror(msg);
    throw CF_CError('O','H',ERR_OUT_WRITE_FILE_END, errno, msg,__FILE__,__LINE__);
    return ERR_OUT_WRITE_FILE_END;
  }
  sprintf(mid,"%06d",recNum);//写记录条数；
  fseek(fp,-6L,SEEK_END);
   if(fputs(mid,fp)==EOF) {
  	fclose(fp);
  	sprintf(msg, "write the file end error\n");
    perror(msg);
    throw CF_CError('O','H',ERR_OUT_WRITE_FILE_END, errno, msg,__FILE__,__LINE__);
    return ERR_OUT_WRITE_FILE_END;
  }
  putc('\n',fp);

  temp=ftell(fp);//写字节数；
  sprintf(mid,"%010d",temp);
  fseek(fp,3,SEEK_SET);
  fputs(mid,fp);
  fclose(fp);
  openSign=0;
  return SUCC;
}

int CFileOut::close(int flag)
{
	char msg[MAX];
  int i;
  long temp;
  if(openSign==0)
  {
    sprintf(msg, "the file is closed\n");
    perror(msg);
   	throw CF_CError('O','H',ERR_OUT_FILE_CLOSED, errno, msg,__FILE__,__LINE__);
    return ERR_OUT_FILE_CLOSED;
  }
  char mid[11]="wEND000000";//写文件尾记录；
  fseek(fp,0L,SEEK_END);
  if(fputs(mid,fp)==EOF) {
  	fclose(fp);
  	sprintf(msg, "write the file end error\n");
    perror(msg);
    throw CF_CError('O','H',ERR_OUT_WRITE_FILE_END, errno, msg,__FILE__,__LINE__);
    return ERR_OUT_WRITE_FILE_END;
  }
  sprintf(mid,"%06d",recNum);//写记录条数；
  fseek(fp,-6L,SEEK_END);
   if(fputs(mid,fp)==EOF) {
  	fclose(fp);
  	sprintf(msg, "write the file end error\n");
    perror(msg);
    throw CF_CError('O','H',ERR_OUT_WRITE_FILE_END, errno, msg,__FILE__,__LINE__);
    return ERR_OUT_WRITE_FILE_END;
  }
  putc('\n',fp);

  temp=ftell(fp);//写字节数；
  sprintf(mid,"%010d",temp);
  fseek(fp,3,SEEK_SET);
  fputs(mid,fp);
  fclose(fp);
  if(recNum==0) unlink(file_name);
  openSign=0;
  return SUCC;
}