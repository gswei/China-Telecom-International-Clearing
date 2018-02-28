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
  �ļ��Ĵ򿪲���;
input:
  fileName          Ҫ�򿪵��ļ�·�����ļ�����	
output:
  none;
return
  SUCC              ����ִ�гɹ�
  ERR_IN_FILE_CLOED �ļ����ڹر�״̬
  ERR_IN_FILE_HEAD  �ļ�ͷ����
  ERR_IN_FILE_BYTE  �ļ��ֽ���������
  ERR_IN_FILE_END   �ļ�β����
  ERR_IN_FILE_NUM   �ļ���¼��������
  ERR_IN_FILE_OPEN  ���ļ�ʧ��
*****************************************************/

int CFileIn::open(char *fileName)
{
  char mid[MAX],ch,msg[MAX],temp[7];
 	openSign=1;
	if((fp=fopen(fileName,"r"))==NULL)//���ļ���⣻
	{
		 openSign=0;
		 sprintf(msg, "Error in open the file(%s)\n",fileName);
  	 perror(msg);
  	 throw CF_CError('O','H',ERR_IN_FILE_OPEN,errno,msg,__FILE__,__LINE__);
		 return ERR_IN_FILE_OPEN;
	}
	if(getc(fp)!='S'||getc(fp)!='O'||getc(fp)!='F')//�ļ�ͷ���
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
	fseek(fp,-10-i,SEEK_END);//�ļ�β��⣻
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
	fseek(fp,14,SEEK_SET);//��¼������⣻	  
  recNum = 0; //1.ȫ�ļ���¼������recNumΪʵ��������
	while(fgets(mid,MAX,fp)!=NULL)
	  ++recNum;
	recNum=recNum-i+1;
	fseek(fp,-7L,SEEK_END);//2.��ȡ�ļ��м�¼������
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
  ���ļ��ж�ȡһ����¼
input:
  &rec               ��CFmt_Change CF������  		
output:
  none
return
  SUCC               �ɹ���ȡ��¼
  ERR_IN_READ_REC    ��ȡ��¼ʧ��
  ERR_IN_FILE_CLOSED �ļ����ڹر�״̬�����ܶ�ȡ
  ERR_IN_AT_END      ��ȡ�ļ������ļ�β��
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
  SUCC               �ɹ��ر��ļ���
  ERR_IN_FILE_CLOSED �ļ��Ѿ����ڹر�״̬��
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