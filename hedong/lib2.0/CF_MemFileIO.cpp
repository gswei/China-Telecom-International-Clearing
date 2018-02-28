#include "CF_MemFileIO.h"
#include "CF_CError.h"
/*
   2005-8-20 �ļ�ͷβ�����á�
   2005-9-9  ���ӶԳ�;������֧�֡�
   2006-2-13 �޸�Close(int)��������CORE�����⣬û�����ļ��رյ��жϡ�
   2006-2-17 �޸Ķ��ļ�ʱ,mmap����ҳ��ռ�����ļ���ʵ�ʴ�С�����ļ��Ľ���λ�ü�\0��
   2006-2-20 �޸Ķ��ļ�ʱ��BUFF��С�ɷ���FAIL��Ϊ�׳��쳣��
   2006-3-16 ���Ӷ��ļ�ͷβ��¼��ȡ�Ľӿڡ�
   2006-5-7  �޸Ķ��ļ�ʱ����¼����Ϊ�㣬û���ͷ��ڴ�ռ��BUG��
   2006-6-14 �޸����ڼ�¼����(>5000)����core�����⡣
   2006-6-28 ���ӻ�ȡ��һ����¼��pos��
   2006-6-29 �޸�SUN��������β���ļ���СΪ8192�������CORE�����⡣
   2006-7-27  ���ӶԱ䳤ͷβ�Ķ��ӿڡ�
   2006-8-17  �޸�д�ļ���Ȩ�ޣ�Ϊ666����UMASK���롣
   2006-9-8   �޸�MEMIO�����һ����¼ʱ�����س������⡣
   2006-11-13 �޸Ļ�ȡ�ļ�ͷ��Ϣ�����BUG���޸Ķ��ļ�ʱһ����¼�׳��쳣�󣬼�¼��û�����ӵ�BUG��
   2006-11-27 �޸�SUN�»�ȡ�ļ�ͷ�����BUG��8192),���� smmap����ĩβ��
   2007-03-19 �޸Ķ���ͷβû�������ļ������CORE�����⡣
   2007-04-02 �޸Ĵ�����Ϣ�����������ļ�����Ϣ��
   2007-07-12 �޸ĳ���OPENʱ���ʼ��һ�±�����
   2007-07-31 ����ͷβ��������Ϣ��֧�֡�
   2007-11-26 �������ݿ��д�ӿڡ�
   2007-12-27 ���ӶԶ��̵߳�֧�֡�
   Last update:2007-12-27
*/

CF_MemFile::CF_MemFile()
{
	file = 0;
	RecCount = 0;
	tellCount = 0;
	smmap = NULL;
	iPos=0;
	nd_num=0;
	iFieldCount=0;
	iFmtInitFlag=0;
	memset(sch,0,RECLEN);
	iHeadTotalSize=13;
	iEndTotalSize=10;
  strcpy(sz_HeadPrefix,(char *)"SOF");
  strcpy(sz_EndPrefix,(char *)"wEND");
  c_Sizepos='H';
  c_Recordpos='E';
  iSizeOffset=3;
  iRecordOffset=4;
  iSizeByte=10;
  iRecordByte=6;
  iDBFlag=0;
  iHEcopyFlag=0;
  c_SizeFillsymbol='0';
  c_RecordFillsymbol='0';
  c_Fillsymbol=' ';
  fieldList=NULL;
  Ds01.Init(DBConn);
  Ds02.Init(DBConn);
}

CF_MemFile::~CF_MemFile()
{

	if(smmap)
	  munmap(smmap,tellCount);
	if(file)
	  close(file);
	if(fieldList!=NULL)
		delete []fieldList;
	file = 0;
	RecCount = 0;
	tellCount = 0;
	iPos=0;
	nd_num=0;
}

void CF_MemFile::Clear()
{
	if(file)
	  close(file);
	file = 0;
  RecCount = 0;
	tellCount = 0;
	iPos=0;
	nd_num=0;

}

int CF_MemFile::Init(int a0,char *a1,int b0,char *b1,
                  char a2,int a3,int a4,char a5,char b2,int b3,int b4,char b5,char c_Symbol)
{
  iHeadTotalSize=a0;
  iEndTotalSize=b0;
  strcpy(sz_HeadPrefix,a1);
  strcpy(sz_EndPrefix,b1);
  c_Sizepos=a2;
  c_Recordpos=b2;
  iSizeOffset=a3;
  iRecordOffset=b3;
  iSizeByte=a4;
  iRecordByte=b4;
  c_SizeFillsymbol=a5;
  c_RecordFillsymbol=b5;
  c_Fillsymbol=c_Symbol;
  if(c_Symbol=='\0') c_Fillsymbol=' ';
  return SUCCESS;
}


int CF_MemFile::Init(const char *pch_recType,char *pch_TableName,char *pch_OderByBuff)
{
	
  CBindSQL bs(DBConn);
  
  Init(pch_recType);
  
  if(strlen(pch_TableName)==0)
  {
  	iTxtNum=0;
    return SUCCESS;
  }
  else
  {
    strcpy(sz_orderByBuff,pch_OderByBuff);
    strcpy(sz_tableName,pch_TableName);
  	iDBFlag=1;
  }

  bs.Open("SELECT count(*) FROM TXTFILE_FMT WHERE FILETYPE_ID=:1",SELECT_QUERY);
  bs<<pch_recType;
  bs>>iTxtNum;
  bs.Close();
  
  if(iTxtNum!=0)
  {
  	int i=0;
    txtfmtList = new struct TXT_FILE_FMT_INFO[iTxtNum];
    bs.Open("SELECT COLNAME  FROM TXTFILE_FMT WHERE FILETYPE_ID=:1 order by col_index",SELECT_QUERY);
    bs<<pch_recType;
    while(bs>>txtfmtList[i].sz_colName)
    {
    	i++;
    }
    bs.Close();
  }

  return SUCCESS;
}

int CF_MemFile::Init(const char *pch_recType)
{
  CBindSQL bs(DBConn);

  int iRet;
  
  strcpy(sz_fileType,pch_recType);
  
  bs.Open("SELECT count(*) from FILE_HEADEND_DEFINE where FILETYPE_ID=:1",SELECT_QUERY);
  bs<<pch_recType;
  bs>>iRet;
  bs.Close();

  if(iRet!=1)
  {
    sprintf(sz_errMsg,"fileType %s init err!",pch_recType); 	
    throw CF_CErrorFile('E','M',ERR_FILE_INIT,errno,sz_errMsg,(char*)__FILE__, __LINE__);
  }
  sz_HeadPrefix[0]=0;
  sz_EndPrefix[0]=0;
  bs.Open("SELECT HEAD_TOTAL_BYTE,HEAD_PREFIX,END_TOTAL_BYTE,END_PREFIX,\
    SIZE_FLAG,SIZE_OFFSET,SIZE_BYTE,SIZE_FILLSYMBOL,RECORD_FLAG,\
      RECORD_OFFSET,RECORD_BYTE,RECORD_FILLSYMBOL,FILE_FILLSYMBOL FROM FILE_HEADEND_DEFINE\
        WHERE FILETYPE_ID=:1",SELECT_QUERY);
  bs<<pch_recType;
  bs>>iHeadTotalSize>>sz_HeadPrefix>>iEndTotalSize>>sz_EndPrefix>>c_Sizepos>>iSizeOffset>>iSizeByte>>c_SizeFillsymbol>>c_Recordpos>>iRecordOffset>>iRecordByte>>c_RecordFillsymbol>>c_Fillsymbol;
  bs.Close();
  if(c_Fillsymbol=='\0') c_Fillsymbol=' ';
  	
  fieldList=new struct MEMIO_FILE_FIELD_INFO [10];

  int iFieldNum,iCount;
  bs.Open("select count(*) from user_tables where table_name=:1",SELECT_QUERY);
  bs<<"FILE_MEMIO_FIELD_DEFINE";
  bs>>iCount;
  bs.Close();

  if(iCount==0)
  {
  	iFieldCount=0;
  }
  else
  {
    bs.Open("SELECT count(*)  FROM FILE_MEMIO_FIELD_DEFINE WHERE FILETYPE_ID=:1",SELECT_QUERY);
    bs<<pch_recType;
    bs>>iFieldNum;
    bs.Close();
    
    if(iFieldNum==0)
    {
  	  iFieldCount=0;
    }
    else
    {
    	iFieldCount=iFieldNum;
      bs.Open("SELECT  FIELD_POSTION,FIELD_MODE,FIELD_VALUE,FIELD_OFFSET,FIELD_BYTE,FIELD_COMPARE,CP_HE_AVAIL FROM FILE_MEMIO_FIELD_DEFINE WHERE FILETYPE_ID=:1 ORDER BY FIELD_ID",SELECT_QUERY);
      bs<<pch_recType;
      int i=0;
      while(bs>>fieldList[i].cFieldFlag>>fieldList[i].cFieldMode>>fieldList[i].sz_fieldValue>>fieldList[i].iFieldOffset>>fieldList[i].iFieldByte>>fieldList[i].cFieldCompare>>fieldList[i].cHeAvail)
      {
    	  if((fieldList[i].cFieldMode=='R')&&(fieldList[i].cHeAvail=='Y')) iFmtInitFlag=2;
    	  if((fieldList[i].cFieldMode=='R')&&(fieldList[i].cHeAvail=='N')) iFmtInitFlag=iFmtInitFlag>1?2:1;
    	  if(fieldList[i].cFieldMode=='R') fieldList[i].iFieldInx=atoi(fieldList[i].sz_fieldValue);
    	  i++;
      }
      bs.Close();
    }
  }

  return SUCCESS;
}

/***************************************************
function
	int CF_MemFile::AllocMap(int flag)
description:
	���ƹ���ռ�ķ���
input:	
	flag			(R,W)��ʶ��д
output:
return
  SUCCESS     ����ִ�гɹ�
  FAIL				����ִ��ʧ��
throw
*****************************************************/
int CF_MemFile::AllocMap(int flag)
{
	if(!file)
	{
		sprintf(sz_errMsg,"file %s is closed",sz_fileName);
		throw CF_CErrorFile('E','M',ERR_FILE_NOT_OPEN,errno,sz_errMsg,(char*)__FILE__, __LINE__);
	}
	switch(flag)
	{
		case R:
			if((smmap=(char *)mmap(0,tellCount,PROT_READ,MAP_PRIVATE,file,0))==(caddr_t)-1)
			{
				sprintf(sz_errMsg,"mmap file %s error",sz_fileName);
				throw CF_CErrorFile('E', 'M',ERR_MMAP, errno,sz_errMsg,(char*)__FILE__,__LINE__);
      }
      //if(strlen(smmap) > tellCount)
      //   smmap[tellCount]=0;
			 break;
		case W:
			if((smmap=(char *)mmap(0,tellCount, PROT_READ|PROT_WRITE, MAP_FILE | MAP_SHARED, file,0))==(caddr_t)-1)
			//if((smmap=(char *)mmap(0,tellCount, PROT_READ|PROT_WRITE, MAP_PRIVATE, file,0))==(caddr_t)-1)
			{
				sprintf(sz_errMsg,"mmap file %s error",sz_fileName);
				throw CF_CErrorFile('E', 'M',ERR_MMAP, errno,sz_errMsg,(char*)__FILE__, __LINE__);
			}
			break;
	}
	return SUCCESS;
}

/***************************************************
function
	int CF_MemFile::CheckFile(int d, int rec)
description:
	Ч���ļ�
input:	
output:
	iFileOffest				ƫ��ֵ        //ֻ��׷�Ӽ�¼��Ч
	iFileRec          д��ļ�¼��  //ֻ��׷�Ӽ�¼��Ч
return
  SUCCESS     ����ִ�гɹ�
  FAIL				����ִ��ʧ��
throw
*****************************************************/
int CF_MemFile::CheckFile(int& iFileOffest,int& iFileRec)
{
  int checkrec,totalnum,totalrec;
	int sz_HeadPreLen,sz_EndPreLen;
	char sz_buff[255];
	char *pch_tmp = NULL;
	
	checkrec=0;
	totalnum=0;
	totalrec=0;
	int dd = 0;
	
	sz_HeadPreLen=strlen(sz_HeadPrefix);
	sz_EndPreLen=strlen(sz_EndPrefix);
	
	if(iHeadTotalSize>0)
	{
  	if(strncmp(smmap,sz_HeadPrefix,sz_HeadPreLen)!=0)
    {
      return FAIL;
    }
    if(c_Sizepos=='H')
    {
  	  if(iSizeByte>=0)
	    {
	      strncpy(sz_buff,smmap+iSizeOffset,iSizeByte);
	      sz_buff[iSizeByte]=0;
	    }
	    else
	    {
	      strncpy(sz_buff,smmap+iSizeOffset,-iSizeByte);
	    	sz_buff[-iSizeByte]=0;
	    }
	    delChar(sz_buff,c_SizeFillsymbol,iSizeByte);
      totalnum=atoi(sz_buff);
    }
    
    if(c_Recordpos=='H')
    {
  	  if(iRecordByte>=0)
	    {
	    	strncpy(sz_buff,smmap+iRecordOffset,iRecordByte);
	    	sz_buff[iRecordByte]=0;
	    }
	    else
	    {
	    	strncpy(sz_buff,smmap+iRecordOffset,-iRecordByte);
	    	sz_buff[-iRecordByte]=0;
	    }
	    delChar(sz_buff,c_RecordFillsymbol,iRecordByte);
      totalrec=atoi(sz_buff);
    }
    
	  pch_tmp=smmap+iHeadTotalSize+1;
	  strncpy(sz_headBuff,smmap,iHeadTotalSize);
	  sz_headBuff[iHeadTotalSize]='\0';
	}
	else if(iHeadTotalSize==0)
	{
	  pch_tmp=smmap;
    sz_headBuff[0]='\0';
  }
  else if(iHeadTotalSize<0)
  {
  	if(strncmp(smmap,sz_HeadPrefix,sz_HeadPreLen)!=0)
    {
      return FAIL;
    }
    
    int kk=0;
  	for(dd=0;dd<tellCount;dd++)
  	{  		
  		if(smmap[dd]=='\n') kk++;  		
  		if(kk==abs(iHeadTotalSize)) break;
  	}
  	
  	//pch_tmp=strchr(smmap,'\n');
  	//strncpy(sz_headBuff,smmap,strlen(smmap)-strlen(pch_tmp));
  	//sz_headBuff[strlen(smmap)-strlen(pch_tmp)]='\0';
  	//pch_tmp++;
  	strncpy(sz_headBuff,smmap,dd);
  	sz_headBuff[dd]=0;
  	pch_tmp=smmap+dd+1;
  }

	char* c=NULL;
	
	
	if((iEndTotalSize>0)&&(sz_EndPreLen!=0))//��β����ǰ����
	{
		while(1)
	  {
	  	if(strncmp(pch_tmp,sz_EndPrefix,sz_EndPreLen)==0)
	  	{
	  		strncpy(sz_endBuff,pch_tmp,iEndTotalSize);
	      sz_endBuff[iEndTotalSize]='\0';
	  		break;
	  	}
	  	++checkrec;
	  	c=strchr(pch_tmp,'\n');
	  	if(c)
		  	pch_tmp=++c;
		  else
		  	return FAIL;
  	}
  	iFileOffest=pch_tmp-smmap;
    if(c_Sizepos=='E')
    {
  	  if(iSizeByte>=0)
	    {
	      strncpy(sz_buff,pch_tmp+iSizeOffset,iSizeByte);
	      sz_buff[iSizeByte]=0;
	    }
	    else
	    {
	      strncpy(sz_buff,pch_tmp+iSizeOffset,-iSizeByte);
	    	sz_buff[-iSizeByte]=0;
	    }
	    delChar(sz_buff,c_SizeFillsymbol,iSizeByte);
      totalnum=atoi(sz_buff);
    }
    if(c_Recordpos=='E')
    {
  	  if(iRecordByte>=0)
	    {
	    	strncpy(sz_buff,pch_tmp+iRecordOffset,iRecordByte);
	    	sz_buff[iRecordByte]=0;
	    }
	    else
	    {
	    	strncpy(sz_buff,pch_tmp+iRecordOffset,-iRecordByte);
	    	sz_buff[-iRecordByte]=0;
	    }
	    delChar(sz_buff,c_RecordFillsymbol,iRecordByte);
      totalrec=atoi(sz_buff);
    }
  }
  else if((iEndTotalSize>0)&&(sz_EndPreLen==0))//��β��û��ǰ����
  {
    long iTempLong,iLastNo;
    iLastNo=0;
  	if(iHeadTotalSize>=0)
  	  iTempLong=tellCount-iHeadTotalSize-1;
  	else
  		iTempLong=tellCount -dd -1;
  	for(int i=0;i<iTempLong;i++)
  	{
  	  if((pch_tmp[i]=='\n')&&(i!=(iTempLong-1)))
  	  { 
  	  	checkrec++;
  	  	iLastNo=i; 
  	  }
  	}
  	if((iTempLong-iLastNo)>=iEndTotalSize)
    {
      strncpy(sz_endBuff,pch_tmp+iLastNo,iEndTotalSize);
    	sz_endBuff[iEndTotalSize]='\0';
    }
    else
    {
    	return FAIL;
    }
  	//char *pch_buff;
  	//while(1)
  	//{
  	//	++checkrec;
  	//  c=strchr(pch_tmp,'\n');
  	//  if(c)
  	//    pch_tmp=++c;
  	//  else
  	//    break;
  	//  pch_buff=pch_tmp;
  	//}
  	//int iRet;
  	//iRet=pch_tmp-smmap;
  	//checkrec--;
  	//if(iRet==iFileSize)
    //{
    //	strncpy(sz_endBuff,pch_tmp-iEndTotalSize,iEndTotalSize);
    //	sz_endBuff[iEndTotalSize]='\0';
    //	pch_tmp=pch_buff;
    //	checkrec--;
    //}
    //else
    //{
    //	strncpy(sz_endBuff,pch_tmp,iEndTotalSize);
    //	sz_endBuff[iEndTotalSize]='\0';
    //}

    iFileOffest=iFileSize-iEndTotalSize;
    if(smmap[iFileSize-1]=='\n')
    {
    	iFileOffest--;
    }
    pch_tmp=smmap+iFileOffest;
    
    if(c_Sizepos=='E')
    {
  	  if(iSizeByte>=0)
	    {
	      strncpy(sz_buff,pch_tmp+iSizeOffset,iSizeByte);
	      sz_buff[iSizeByte]=0;
	    }
	    else
	    {
	      strncpy(sz_buff,pch_tmp+iSizeOffset,-iSizeByte);
	    	sz_buff[-iSizeByte]=0;
	    }
	    delChar(sz_buff,c_SizeFillsymbol,iSizeByte);
      totalnum=atoi(sz_buff);
    }
    if(c_Recordpos=='E')
    {
  	  if(iRecordByte>=0)
	    {
	    	strncpy(sz_buff,pch_tmp+iRecordOffset,iRecordByte);
	    	sz_buff[iRecordByte]=0;
	    }
	    else
	    {
	    	strncpy(sz_buff,pch_tmp+iRecordOffset,-iRecordByte);
	    	sz_buff[-iRecordByte]=0;
	    }
	    delChar(sz_buff,c_RecordFillsymbol,iRecordByte);
      totalrec=atoi(sz_buff);
    }
  }
  else if(iEndTotalSize==0) //ûβ
  {
  	sz_endBuff[0]='\0';

  	if(tellCount==0) checkrec=0;
  	else
  	{  		
  	  checkrec=1;
  	  long i;
  	  long iTempLong;
  	  if(iHeadTotalSize >= 0)
  	    iTempLong=tellCount-iHeadTotalSize-1;
  	  else
  	  	iTempLong=tellCount - dd -1;
  	  for(i=0;i<iTempLong;i++)
  	  {
  		  if(pch_tmp[i]=='\n') checkrec++;
  	  }
  	  if(pch_tmp[iTempLong-1]=='\n') checkrec--;  	  	
  	}
  }
  else if((iEndTotalSize==-1)&&(sz_EndPreLen==0))
  {
  	char *pch_buff;
  	while(1)
  	{
  		++checkrec;
  	  c=strchr(pch_tmp,'\n');
  	  if(c)
  	    pch_tmp=++c;
  	  else
  	    break;
  	  pch_buff=pch_tmp;
  	}
  	int iRet;
  	iRet=pch_tmp-smmap;
  	checkrec--;
  	if(iRet==iFileSize)
    {
    	checkrec--;
    }
  }
  else if((iEndTotalSize==-1)&&(sz_EndPreLen!=0))//�䳤����β
	{
		while(1)
	  {
	  	if(strncmp(pch_tmp,sz_EndPrefix,sz_EndPreLen)==0)
	  	{
	  		break;
	  	}
	  	++checkrec;
	  	c=strchr(pch_tmp,'\n');
	  	if(c)
		  	pch_tmp=++c;
		  else
		  	return FAIL;
  	}
  	iFileOffest=pch_tmp-smmap;
  }
  
	//printf("totalnum=%d,totalrec=%d\n",totalnum,totalrec);
	//printf("checkrec=%d,totalrec=%d\n",checkrec,totalrec);
	if((iHeadTotalSize!=0&&c_Recordpos=='H')||(iEndTotalSize!=0&&c_Recordpos=='E'))
	{
		if(totalrec!=checkrec)
		  return FAIL;
	}
	iFileRec=checkrec;
	RecCount=checkrec;

	if((iHeadTotalSize!=0&&c_Sizepos=='H')||(iEndTotalSize!=0&&c_Sizepos=='E'))
	{
		if(iFileSize!=totalnum)
		  return FAIL;
	}
	if(iEndTotalSize==0)
	{
		iFileOffest=iFileSize;
  }

	return SUCCESS;
}

int CF_MemFile::Get_recCount()
{
	return RecCount;
}

int CF_MemFile::addChar(char *ss,char c_AddChar,int iAddFlag)//>0,����������,<0�ұ���ӵ�����
{
  char sz_temp[1024];
  
  strcpy(sz_temp,ss);
  if(iAddFlag>=0)
  {
    memset(ss,c_AddChar,iAddFlag);
    ss[iAddFlag]=0;
    int iLen;
    iLen=iAddFlag-strlen(sz_temp);
    for(int i=0;i<strlen(sz_temp);i++)
      ss[iLen+i]=sz_temp[i];
  }
  else
  {
  	memset(ss,c_AddChar,-iAddFlag);
    ss[-iAddFlag]=0;
    int iLen;
    for(int i=0;i<strlen(sz_temp);i++)
      ss[i]=sz_temp[i];
  }
	return SUCCESS;
}

int CF_MemFile::delChar(char *ss,char c_DelChar,int iDelFlag)//>0,ȥ����ߵ�����,<0ȥ���ұߵ�����
{
   int i, j, k,iLen;
   char sz_temp[1024];

   iLen=strlen(ss);
   memset(sz_temp,0,1024);
   if(iDelFlag>=0)
   {
     for (i=0;i<iLen;i++)
      if (ss[i]!=c_DelChar)
        break;
     for (j=i;j<iLen;j++)
      sz_temp[j-i]=ss[j];
     strcpy(ss,sz_temp);
   }
   else
   {
     i=strlen(ss)-1;
     while(i&&ss[i]==c_DelChar)
     i--;
     ss[i+1]=0;
   }
   return SUCCESS;
}



/***************************************************
function
	int CF_MemFileO::GetPos()
description:
	��õ�ǰָ���ƫ��ֵ
input:	
output:
return
  ��ǰָ���ƫ��ֵ
throw
*****************************************************/
int CF_MemFile::GetPos()
{
	return iPos;
}

int CF_MemFile::GetFirstRecPos()
{
	if(iHeadTotalSize==0) return 0;
	else
		return (iHeadTotalSize+1);
}

void CF_MemFile::setpos(int i)
{
	iPos=i;
}

int CF_MemFile::GetHead(char *pch_buff,int iLen)
{
	if(strlen(sz_headBuff)>iLen)
	{
		return -1;
	}
	else
	{
		strcpy(pch_buff,sz_headBuff);
		return 0;
	}
}


int CF_MemFile::GetEnd(char *pch_buff,int iLen)
{
	if(strlen(sz_endBuff)>iLen)
	{
		return -1;
	}
	else
	{
		strcpy(pch_buff,sz_endBuff);
		return 0;
	}
}

int CF_MemFile::CheckTotalNum(int iMaxTotalNum)
{
	int iRecNum;
	char sz_buff[255];
  CBindSQL bs(DBConn); 
  
  iRecNum=0;
  
  if(iMaxTotalNum==-1) return 0;
  if(iDBFlag!=1)    return  0; 

  sprintf(sz_buff,"SELECT count(*) from %s",sz_tableName);
  bs.Open(sz_buff,SELECT_QUERY);
  bs>>iRecNum;
  bs.Close();
  
 if(iMaxTotalNum>iRecNum) return 0;
 else   return -1;

}

CF_MemFileO::CF_MemFileO()
{
	memset(sz_fileName, 0, FILELEN);
}

CF_MemFileO::~CF_MemFileO()
{
	//Close();
}

int CF_MemFileO::setHeadEnd(CF_MemFileI &classOrg)
{
	char sz_buff[500];
	
  strcpy(sz_headBuff,classOrg.sz_headBuff);
  strcpy(sz_endBuff,classOrg.sz_endBuff);
	iHEcopyFlag=1;
	
	return 0;
}

/***************************************************
function
	int CF_MemFileO::CheckPos(int i)
description:
	�����д�ַ�����û�г�������ռ䣬���糬����������Ŀռ�
input:	
	i				Ҫд����ַ����ĳ���
output:
return
  SUCCESS     ����ִ�гɹ�
  FAIL				����ִ��ʧ��
throw
	ERR_UNKOWN		δ֪����
*****************************************************/
int CF_MemFileO::CheckPos(int i)
{
	if(iPos+i>tellCount)
	{
		if(munmap(smmap,tellCount))
	  {
	  	sprintf(sz_errMsg,"%s delete memory space error",sz_fileName);
			throw CF_CErrorFile('O','M',errno,ERR_DEL_SPACE,sz_errMsg,(char*)__FILE__, __LINE__);
		}
		tellCount = tellCount + sysconf(_SC_PAGESIZE)*2;
		if(ftruncate(file,tellCount))
		{
	  	sprintf(sz_errMsg,"%s memory length ftruncate error",sz_fileName);
			throw CF_CErrorFile('O', 'M',errno,ERR_FILELEN_CHANGE,sz_errMsg,(char*)__FILE__, __LINE__);
		}
		int t = AllocMap(W);
		return t;
	}

	return SUCCESS;
}
/***************************************************
function
	int CF_MemFileO::process_header()
description:
	д��ͷ��¼
input:	
output:
return
  SUCCESS     ����ִ�гɹ�
  FAIL				����ִ��ʧ��
throw
*****************************************************/
int CF_MemFileO::process_header()
{  
	char sz_buff[1024];

	//if(iHeadTotalSize!=0)
	if(iHeadTotalSize>0)
	{
		CheckPos(iHeadTotalSize+30);
		if(iHEcopyFlag==0)
		  memset(sz_buff,c_Fillsymbol,iHeadTotalSize);
		else
		{
			strcpy(sz_buff,sz_headBuff);
			sz_buff[iHeadTotalSize]='\0';
		}
		for(int i=0;i<strlen(sz_HeadPrefix);i++)
		{
			sz_buff[i]=sz_HeadPrefix[i];
		}
		sz_buff[iHeadTotalSize]=0;
    strcat(sz_buff,"\n");
		sprintf(smmap,"%s",sz_buff);
	  iPos=strlen(sz_buff);
	}
	else
	  iPos=0;
	return SUCCESS;
}
/***************************************************
function
	int CF_MemFileO::process_column(char* str)
description:
	д�뻰������
input:	
	str				�����ַ���
output:
return
  SUCCESS     ����ִ�гɹ�
  FAIL				����ִ��ʧ��
throw
*****************************************************/
int CF_MemFileO::process_column(char* str)
{
	int i=strlen(str);
	if(str[i - 1] == '\n')
	{
		CheckPos(i+30);
		sprintf(smmap + iPos, "%s", str);
		iPos += i;
	}
	else
	{
		++i;
		CheckPos(i+30);
		sprintf(smmap + iPos, "%s\n", str);
		iPos = iPos + i;
	}
	RecCount++;
	return SUCCESS;
}
/***************************************************
function
	int CF_MemFileO::process_end()
description:
	д�뻰��β��¼
input:	
output:
return
  SUCCESS     ����ִ�гɹ�
  FAIL				����ִ��ʧ��
throw
*****************************************************/
int CF_MemFileO::process_end()
{
	char sz_buff[1024];
	char sz_tmpA[1024];
  char sz_tmpB[1024];
  char sz_tmpC[1024];
  char sz_tmpD[1024];
	int iTmp;
	
	//if(iEndTotalSize!=0)
	if(iEndTotalSize>0)
	{
	  CheckPos(iEndTotalSize+30);
	  iTmp=iPos;
	  iPos=iPos+iEndTotalSize+1;
    
    if(iHEcopyFlag==0)
    {
    	memset(sz_buff,c_Fillsymbol,iEndTotalSize);
      sz_buff[iEndTotalSize]=0;
    }
    else
    {
    	strcpy(sz_buff,sz_endBuff);
    	sz_buff[iEndTotalSize]=0;
    }
    for(int i=0;i<strlen(sz_EndPrefix);i++)
      sz_buff[i]=sz_EndPrefix[i];
    
    if(c_Sizepos=='E')
	  {
	  	sprintf(sz_tmpA,"%d",iPos);
      addChar(sz_tmpA,c_SizeFillsymbol,iSizeByte);
      for(int i=0;i<strlen(sz_tmpA);i++)
        sz_buff[i+iSizeOffset]=sz_tmpA[i];
	  }
	  if(c_Recordpos=='E')
	  {
	  	sprintf(sz_tmpA,"%d",RecCount);
      addChar(sz_tmpA,c_RecordFillsymbol,iRecordByte);
      for(int i=0;i<strlen(sz_tmpA);i++)
        sz_buff[i+iRecordOffset]=sz_tmpA[i];
    }

    if(iFieldCount!=0)
    {
    	for(int i=0;i<iFieldCount;i++)
    	{
			  if((iHEcopyFlag==1)&&(fieldList[i].cHeAvail=='N')) continue;
    		if(fieldList[i].cFieldFlag=='E')
    		{
    		  for(int j=0;j<strlen(fieldList[i].sz_fieldBuff);j++)
    	  	{
    		  	sz_buff[j+fieldList[i].iFieldOffset]=fieldList[i].sz_fieldBuff[j];
    	  	}
    	  }
    	}
    }
    strcat(sz_buff,"\n");
    sprintf(smmap+iTmp,"%s",sz_buff);
  }
	//if(iHeadTotalSize!=0)
	if(iHeadTotalSize>0)
	{
    if(c_Sizepos=='H')
   	{
   		sprintf(sz_tmpA,"%d",iPos);
      addChar(sz_tmpA,c_SizeFillsymbol,iSizeByte);
      for(int i=0;i<strlen(sz_tmpA);i++)
        smmap[iSizeOffset+i]=sz_tmpA[i];
	  }
	  if(c_Recordpos=='H')
	  {
	  	sprintf(sz_tmpA,"%d",RecCount);
      addChar(sz_tmpA,c_RecordFillsymbol,iRecordByte);
      for(int i=0;i<strlen(sz_tmpA);i++)
        smmap[iRecordOffset+i]=sz_tmpA[i];
	  }
    if(iFieldCount!=0)
    {
    	for(int i=0;i<iFieldCount;i++)
    	{
    		if(fieldList[i].cFieldFlag=='H')
    		{
    		 if((iHEcopyFlag==1)&&(fieldList[i].cHeAvail=='N')) continue;
         for(int j=0;j<strlen(fieldList[i].sz_fieldBuff);j++)  
          smmap[fieldList[i].iFieldOffset+j]=fieldList[i].sz_fieldBuff[j];
        }
    	}
    }
	}
	return SUCCESS;
}


/***************************************************
function
	int CF_MemFileO::Open(char* filename)
description:
	��һ��д����ļ�
input:	
	filename		�ļ���
output:
return
  SUCCESS     ����ִ�гɹ�
  FAIL				����ִ��ʧ��
throw
	ERR_FILE_OPEN	�ļ��򿪴���
	ERR_UNKOWN		δ֪����
*****************************************************/
int CF_MemFileO::Open(char* filename,int mode)
{

	if(iDBFlag==1)
	{
		DbOpen(filename);
		return SUCCESS;
	}
	
	Clear();
	strcpy(sz_fileName,filename);
	if(mode!='A' && mode!='W')
		return FAIL;
	struct stat buf;
	if(mode=='A')
	{
		if(stat(filename,&buf))
		{
			tellCount = sysconf(_SC_PAGESIZE)*2;
			//file = open(filename, O_CREAT | O_RDWR | O_TRUNC, S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
		  file = open(filename, O_CREAT | O_RDWR | O_TRUNC, S_IREAD|S_IWRITE|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);

      mode='W';
		}
		else
		{
		  iFileSize=buf.st_size;
			tellCount = ((buf.st_size/sysconf(_SC_PAGESIZE))+1)*sysconf(_SC_PAGESIZE);
			file = open(filename, O_RDWR, S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
		}
	}
	else
	{
		tellCount = sysconf(_SC_PAGESIZE)*2;
		//file = open(filename, O_CREAT | O_RDWR | O_TRUNC,S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
		file = open(filename, O_CREAT | O_RDWR | O_TRUNC,S_IREAD|S_IWRITE|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	}
	if(file == -1)
	{
		tellCount = 0;
		file = 0;
		sprintf(sz_errMsg,"file %s open error",sz_fileName);
		throw CF_CErrorFile('O', 'M', ERR_FILE_OPEN,errno,sz_errMsg,(char*)__FILE__, __LINE__);
	}
	if(ftruncate(file,tellCount))
	{
		sprintf(sz_errMsg,"%s ftruncate error",sz_fileName);
		throw CF_CErrorFile('O', 'M',ERR_FILELEN_CHANGE, errno,sz_errMsg,(char*)__FILE__, __LINE__);
	}
	if(AllocMap(W))
		return FAIL;
	if(mode=='A')
		if(CheckFile(iPos,RecCount)==FAIL)
		{
			munmap(smmap,tellCount);
			close(file);
			file = 0;
			iPos = 0;
			RecCount = 0;
			smmap = NULL;
			sprintf(sz_errMsg,"file(%s) head or end error",sz_fileName);
			throw CF_CErrorFile('E','M',ERR_FOR_BILL_BE,0,sz_errMsg,(char*)__FILE__, __LINE__);
		}
	//printf("iPos=%d\n",iPos); 
	sprintf(sz_fileName,"%s",filename);
	if(mode=='W')
		return process_header();
	else
		return SUCCESS;
}


int CF_MemFileO::Open(char* filename,int iOffset,int iRec)
{
	struct stat buf;
	
  Clear();
 	strcpy(sz_fileName,filename);
  if(stat(filename,&buf))
	{
		tellCount = sysconf(_SC_PAGESIZE)*2;
		//file = open(filename, O_CREAT | O_RDWR | O_TRUNC, S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
		file = open(filename, O_CREAT | O_RDWR | O_TRUNC, S_IREAD|S_IWRITE|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);

	}
	else
	{
		iFileSize=buf.st_size;
		tellCount = ((buf.st_size/sysconf(_SC_PAGESIZE))+1)*sysconf(_SC_PAGESIZE);
		//file=open(filename, O_RDWR, S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
		file=open(filename, O_RDWR, S_IREAD|S_IWRITE|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	}
	if(file==-1)
	{
		tellCount = 0;
		file = 0;
		sprintf(sz_errMsg,"file %s open error",sz_fileName);
		throw CF_CErrorFile('O', 'M', ERR_FILE_OPEN, errno,sz_errMsg,(char*)__FILE__, __LINE__);
	}
	if(ftruncate(file,tellCount))
	{
    sprintf(sz_errMsg,"%s ftruncate error",sz_fileName);
		throw CF_CErrorFile('O', 'M',ERR_FILELEN_CHANGE, errno,sz_errMsg,(char*)__FILE__, __LINE__);
	}
	if(AllocMap(W))
		return FAIL;
	iPos=iOffset;
  RecCount=iRec;
  return SUCCESS;
}




/***************************************************
function
	int CF_MemFileO::Close()
description:
	�����ļ�
input:	
output:
return
  SUCCESS     ����ִ�гɹ�
  FAIL				����ִ��ʧ��
throw
	ERR_UNKOWN		δ֪����
*****************************************************/
int CF_MemFileO::Close()
{
  if(iDBFlag==1)
	{
		DbClose();
		return SUCCESS;
	}
	
	if(file)
	{
		process_end();
		munmap(smmap,tellCount);
		if(ftruncate(file,iPos))
		{
      sprintf(sz_errMsg,"%s ftruncate error",sz_fileName);
			throw CF_CErrorFile('O','M',errno,ERR_FILELEN_CHANGE,sz_errMsg,(char*)__FILE__,__LINE__);
		}
		if(close(file))
		{
			file = 0;
			sprintf(sz_errMsg,"file %s close error",sz_fileName);
			throw CF_CErrorFile('O','M',errno,ERR_CLOSE_FILE,sz_errMsg,(char*)__FILE__,__LINE__);
		}
		file = 0;
		iPos = 0;
		RecCount = 0;
		smmap = NULL;
		memset(sz_fileName,0,FILELEN);
	}
	return SUCCESS;
}

int CF_MemFileO::Close(int)
{
  if(iDBFlag==1)
	{
		DbClose();
		return SUCCESS;
	}
	
	if(RecCount)
		Close();
	else if(file==0)
	{
		return SUCCESS;
	}
	else
	{
		munmap(smmap,tellCount);
    if(close(file))
		{
			file = 0;
			sprintf(sz_errMsg,"file %s close error",sz_fileName);
			throw CF_CErrorFile('O','M',errno,ERR_CLOSE_FILE,sz_errMsg,(char*)__FILE__,__LINE__);
		}
		remove(sz_fileName);
		file = 0;
		iPos = 0;
		RecCount = 0;
		smmap = NULL;
		memset(sz_fileName,0,FILELEN);
	}
	return SUCCESS;
}


int CF_MemFileO::Close(CBindSQL &DsOne)
{
	if(iDBFlag==1)
	  DbClose(DsOne);
	else
	  Close();

  return SUCCESS;
}
/***************************************************
function
	int CF_MemFileO::writeRec(CFmt_Change& change)
description:
	д���¼
input:	
	change			������ʽ
output:
return
  SUCCESS     ����ִ�гɹ�
  FAIL				����ִ��ʧ��
throw
*****************************************************/

int CF_MemFileO::writeRec(CFmt_Change& change,char *pch_cmpareBuff)
{
  if(iDBFlag==1)
	{
    DbWriteRec(change,pch_cmpareBuff);
		return SUCCESS;
	}
	
	if(!file)
		return FAIL;
	change.Get_record(sch,RECLEN);
  if(iFieldCount!=0)
	{
		char sz_buff[255];
		for(int i=0;i<iFieldCount;i++)
		{
      if((iHEcopyFlag==1)&&(fieldList[i].cHeAvail=='N')) continue;
      if(fieldList[i].cFieldMode=='R')
      {
			  change.Get_Field(fieldList[i].iFieldInx,sz_buff);
			  if(fieldList[i].cFieldCompare=='B')
			  {
			  	if(fieldList[i].sz_fieldBuff[0]=='\0')
			  	{
			    	strcpy(fieldList[i].sz_fieldBuff,sz_buff);
			  	}
			  	else if (sz_buff[0]=='\0')
			  	{
			  		
			  	}
			    else if(strcmp(fieldList[i].sz_fieldBuff,sz_buff)<0)
			    {
			    	strcpy(fieldList[i].sz_fieldBuff,sz_buff);
			    }
			  }
			  else
			  {
			  	if(fieldList[i].sz_fieldBuff[0]=='\0')
			  	{
			    	strcpy(fieldList[i].sz_fieldBuff,sz_buff);
			  	}
			  	else if (sz_buff[0]=='\0')
			  	{
			  		
			  	}
			  	else if(strcmp(fieldList[i].sz_fieldBuff,sz_buff)>0)
			    {
			    	strcpy(fieldList[i].sz_fieldBuff,sz_buff);
			    }
			  }
			}
		}
	}
	process_column(sch);
	return SUCCESS;
}

int CF_MemFileO::writeRec(CFmt_Change& change,CBindSQL &DsOne,char *pch_cmpareBuff)
{
	int iReturn;

  if(iDBFlag==1)
  {
    iReturn=DbWriteRec(change,DsOne,pch_cmpareBuff);
  }
  else
  {
    iReturn=writeRec(change,pch_cmpareBuff);
  }

  return iReturn;

}


int CF_MemFileO::writeRec(char *pch_record)
{
	if(!file)
		return FAIL;

  if(iFieldCount!=0)
	{
		if((iFmtInitFlag==2)||(iFmtInitFlag==1)&&(iHEcopyFlag==0))
		{
		  char cRecordType;
      CBindSQL bs(DBConn);
      bs.Open("SELECT RECORD_TYPE From FILETYPE_DEFINE Where FILETYPE_ID=:1",SELECT_QUERY);
      bs<<sz_fileType;
      bs>>cRecordType;
      bs.Close();

      classRecordType.Init(sz_fileType,cRecordType);
      iFmtInitFlag=3;
    }

    char sz_buff[255];
    classRecordType.Set_record(pch_record);
		for(int i=0;i<iFieldCount;i++)
		{
			if((iHEcopyFlag==1)&&(fieldList[i].cHeAvail=='N')) continue;
			if(fieldList[i].cFieldMode=='R') 
			{
				classRecordType.Get_Field(fieldList[i].iFieldInx,sz_buff);
			  if(fieldList[i].cFieldCompare=='B')
			  {
				  if(fieldList[i].sz_fieldBuff[0]=='\0')
				  {
			  	  strcpy(fieldList[i].sz_fieldBuff,sz_buff);
				  }
				  else if (sz_buff[0]=='\0')
				  {

				  }
			    else if(strcmp(fieldList[i].sz_fieldBuff,sz_buff)<0)
			    {
			  	  strcpy(fieldList[i].sz_fieldBuff,sz_buff);
			    }
			  }
			  else
			  {
				  if(fieldList[i].sz_fieldBuff[0]=='\0')
				  {
			    	strcpy(fieldList[i].sz_fieldBuff,sz_buff);
				  }
				  else if (sz_buff[0]=='\0')
				  {
					
				  }
				  else if(strcmp(fieldList[i].sz_fieldBuff,sz_buff)>0)
			    {
			  	  strcpy(fieldList[i].sz_fieldBuff,sz_buff);
			    }
			  }
			}
		}
	}
	strcpy(sch,pch_record);

	process_column(sch);

	return SUCCESS;
}

int CF_MemFileO::operator<<(CFmt_Change& change)
{
	return writeRec(change,"\0");
}

CF_MemFileI::CF_MemFileI()
{
	iRecordNo=0;
}

CF_MemFileI::~CF_MemFileI()
{
	//Close();
}
/***************************************************
function
	int CF_MemFileI::Open(char* filename)
description:
	�򿪶������ļ�
input:	
	filename		�ļ���
output:
return
  SUCCESS     ����ִ�гɹ�
  FAIL				����ִ��ʧ��
throw
	ERR_FILE_OPEN	�ļ��򿪴���
	FAIL			ʧ��
*****************************************************/
int CF_MemFileI::Open(char* filename,int iMode)
{
  
  if(iDBFlag==1)
	{
		if(iMode==1) iDelFlag=1;
		else  iDelFlag=0;
		int iReturn;
		iReturn=DbOpen(filename);
		return iReturn;
	}
	
	struct stat buf;

	Clear();
	strcpy(sz_fileName,filename);
	if(stat(filename,&buf))
	{
		file=0;
		sprintf(sz_errMsg,"file %s not exist",sz_fileName);
		throw CF_CErrorFile('O', 'M',errno,ERR_FOR_ACCESS_FILE,sz_errMsg,(char*)__FILE__, __LINE__ );
	}
	if(buf.st_size<(iHeadTotalSize+iEndTotalSize))
	{
		file=0;
		sprintf(sz_errMsg,"file %s head or end error",sz_fileName);
		throw CF_CErrorFile('E', 'S',ERR_FOR_BILL_BE, errno,sz_errMsg,(char*)__FILE__, __LINE__);
	}
	tellCount=buf.st_size;
	iFileSize=tellCount;
	file=open(filename,O_RDONLY, S_IREAD);
	if(file==-1)
	{
		tellCount=0;
		file=0;
		sprintf(sz_errMsg,"file %s open error",sz_fileName);
		throw CF_CErrorFile('O','M',errno,ERR_FILE_OPEN,sz_errMsg,(char*)__FILE__, __LINE__);
	}
	if(tellCount==0)
	{
		iReadNum=0;
		RecCount=0;
		return SUCCESS;
	}
	if(AllocMap(R))
		return FAIL;
	int d,rec;
	if(CheckFile(d,rec)==FAIL)
	{
		munmap(smmap,tellCount);
		close(file);
		file=0;
		tellCount=0;
		sprintf(sz_errMsg,"file %s head or end error",sz_fileName);
		throw CF_CErrorFile('E','S',ERR_FOR_BILL_BE,errno,sz_errMsg,(char*)__FILE__, __LINE__);
	}
	iReadNum=0;
	return SUCCESS;
}

int CF_MemFileI::Open(char* filename,CBindSQL &DsOne,int iMode)
{
  if(iDBFlag==1)
  { 
	  if(iMode==1) iDelFlag=1;
	  else  iDelFlag=0;
	  int iReturn;
	  iReturn=DbOpen(filename,DsOne);
	  return iReturn;
	}
	else
	{
		int iReturn;
	  iReturn=Open(filename,iMode);
	  return iReturn;
	}
}
	
/***************************************************
function
	int CF_MemFileI::Close()
description:
	�ر��Ѵ򿪵��ļ�
input:	
output:
return
  SUCCESS     ����ִ�гɹ�
  FAIL				����ִ��ʧ��
throw
*****************************************************/
int CF_MemFileI::Close()
{
	if(iDBFlag==1)
	{
	  DbClose();
    return SUCCESS;
	}

	if(file)
	{
		//if(RecCount)
	  munmap(smmap,tellCount);
		close(file);
		file = 0;
		iPos = 0;
		RecCount = 0;
		smmap = NULL;
	}
	return SUCCESS;
}

int CF_MemFileI::Close(CBindSQL &DsOne)
{
  if(iDBFlag==1)
	  DbClose(DsOne);
	else
		Close();
  return SUCCESS;
}
/***************************************************
function
	int CF_MemFileI::readRec(CFmt_Change& change)
description:
	����¼
input:	
	change			������ʽ
output:
return
  SUCCESS         ����ִ�гɹ�
  FAIL				    ����ִ��ʧ��
  ERR_READ_END		���ļ�β��
throw
*****************************************************/
int CF_MemFileI::readRec(CFmt_Change& change)
{
	
	if(iDBFlag==1)
	{
		int iReturn;
		iReturn=DbReadRec(change);
		return iReturn;
	}
	
	if(!file)
		return FAIL;
	if(iReadNum==RecCount)
		return READ_AT_END;
	char* pch_tmp=NULL;
	if((!iPos)&&(iHeadTotalSize!=0))
	{
		pch_tmp=strchr(smmap,'\n');
		if(!pch_tmp)
		{
      sprintf(sz_errMsg,"Record Buff Error");
			throw CF_CErrorFile('E', 'M',FAIL, errno,sz_errMsg,(char*)__FILE__,__LINE__);
			return FAIL;
		}
		iPos=pch_tmp-smmap+1;
	}

	if((iReadNum==RecCount-1)&&(iEndTotalSize==0))
	{
		pch_tmp=NULL;
	  for(int i=iPos;i<iFileSize;i++)
	  {
	  	if(smmap[i]=='\n')
	  	{
	  		pch_tmp=smmap+i;
	  		break;
	  	}
	  }
	}
	else 
  {
    pch_tmp = strchr(smmap+iPos,'\n');
  }

	if(pch_tmp==NULL)
	{
		int iTmpSize;
		iTmpSize=iFileSize-iPos;
		strncpy(sch,smmap+iPos,iTmpSize);
		sch[iTmpSize]='\0';
		if(sch[strlen(sch)-1]=='\n') sch[strlen(sch)-1]=0;
		iPos+=strlen(sch);
	}
	else
	{
		int j = pch_tmp-smmap-iPos+1;
		if(j>=5000)
		{
		  sprintf(sz_errMsg,"file(%s) record too long",sz_fileName);
			throw CF_CErrorFile('E', 'M',ERR_RECORD_TOO_LONG, errno,sz_errMsg,(char*)__FILE__,__LINE__);
		}
		strncpy(sch,smmap+iPos,j);
		sch[j]=0;
		iPos+=j;
	}
	iReadNum++;
	change.Set_record(sch);
	return SUCCESS;
}

int CF_MemFileI::readRec(char *buff,int buff_long)
{
	
	if(!file)
	  return FAIL;
	char* pch_tmp = NULL;
	if(iReadNum==RecCount)
		return READ_AT_END;
	if((!iPos)&&(iHeadTotalSize!=0))//�ļ�ͷ��Ϊ�գ�
	{
		if(iHeadTotalSize<0)//���С��0��������ָ��������
		{	int k;		
			int a = 0;
			for( k=0; k < strlen(smmap); k++)
			{				
				if(smmap[k]=='\n') a++;
				if(a == abs(iHeadTotalSize)) break;
			}
			pch_tmp = smmap + k;			
		}
		else if(iHeadTotalSize>0)
			pch_tmp = strchr(smmap,'\n');//�������0��������1��
		if(!pch_tmp)
		{
      sprintf(sz_errMsg,"Record Buff Error");
			throw CF_CErrorFile('E', 'M',FAIL, errno,sz_errMsg,(char*)__FILE__,__LINE__);
			return FAIL;
		}
		iPos=pch_tmp-smmap+1;		
	}
	//if(smmap[iPos]=='w' && smmap[iPos+1]=='E' && smmap[iPos+2]=='N' && smmap[iPos+3]=='D')
  //if(strncmp(tmp+1,sz_suffix,strlen(sz_suffix))==0)
	//if(strncmp(smmap+iPos,sz_suffix,strlen(sz_suffix))==0)
	//pch_tmp=strchr(smmap+iPos,'\n');
	
	if((iReadNum==RecCount-1)&&(iEndTotalSize==0))
	{
		pch_tmp=NULL;
	  for(int i=iPos;i<iFileSize;i++)
	  {
	  	if(smmap[i]=='\n')
	  	{
	  		pch_tmp=smmap+i;
	  		break;
	  	}
	  }
	}
	else 
  {
    pch_tmp=strchr(smmap+iPos,'\n');
  }

	if(pch_tmp==NULL)
	{
		int iTmpSize;
		iTmpSize=iFileSize-iPos;
		strncpy(sch,smmap+iPos,iTmpSize);
		sch[iTmpSize]='\0';
		if(sch[strlen(sch)-1]=='\n') sch[strlen(sch)-1]=0;
		iPos+=strlen(sch);
	}
	else
	{
		int j=pch_tmp-smmap-iPos+1;
		if(j>=5000)
		{
		  sprintf(sz_errMsg,"file(%s) record too long",sz_fileName);
			throw CF_CErrorFile('E', 'M',ERR_RECORD_TOO_LONG, errno,sz_errMsg,(char*)__FILE__,__LINE__);
		}
		strncpy(sch,smmap+iPos,j);
		sch[j]=0;
		iPos+=j;
	}
	iReadNum++;

	if(strlen(sch)>buff_long)
	{
		sprintf(sz_errMsg,"%s:the buff is not enough",sz_fileName);
		throw CF_CErrorFile('O', 'M', ERR_IN_READ_REC, errno,sz_errMsg,(char*)__FILE__, __LINE__);
    return FAIL;
	}
	else strcpy(buff,sch);
	
	return SUCCESS;
}

int CF_MemFileI::readRec(CFmt_Change& change,CBindSQL &DsOne)
{
	int iReturn;

	if(iDBFlag==1)
	{
	  iReturn=DbReadRec(change,DsOne);
	}
	else
	{
	  iReturn=readRec(change);
	}
	return iReturn;
}

int CF_MemFile::get_num()
{
	return nd_num;
}

int CF_MemFileI::operator>>(CFmt_Change& change)
{
	return readRec(change);
}

int CF_MemFileLO::Open(char *filename)
{
	char sz_buff[255];
	
	Clear();
  strcpy(sz_fileName,filename);
	tellCount = sysconf(_SC_PAGESIZE)*2;
	//file = open(filename, O_CREAT | O_RDWR | O_TRUNC,S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
	file = open(filename, O_CREAT | O_RDWR | O_TRUNC,S_IREAD|S_IWRITE|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	
	if(file == -1)
	{
		tellCount = 0;
		file = 0;
	  sprintf(sz_errMsg,"file %s open error",sz_fileName);
		throw CF_CErrorFile('O', 'M', ERR_FILE_OPEN, errno,sz_errMsg,(char*)__FILE__, __LINE__);
	}
	if(ftruncate(file,tellCount))
	{
    sprintf(sz_errMsg,"%s ftruncate error",sz_fileName);
		throw CF_CErrorFile('O','M',ERR_FILELEN_CHANGE, errno,sz_errMsg,(char*)__FILE__, __LINE__);
	}
	if(AllocMap(W))
		return FAIL;

  CheckPos(93);
	memset(sz_buff,' ',255);
	sz_buff[93]=0;
  sprintf(smmap,"%s",sz_buff);
	iPos=93;
	return SUCCESS;
}

int CF_MemFileLO::Close()
{
	if(file)
	{
		munmap(smmap,tellCount);
		if(ftruncate(file,iPos))
		{
      sprintf(sz_errMsg,"%s ftruncate error",sz_fileName);
			throw CF_CErrorFile('O','M',errno,ERR_FILELEN_CHANGE,sz_errMsg,(char*)__FILE__,__LINE__);
		}
		if(close(file))
		{
			file = 0;
			sprintf(sz_errMsg,"file %s close error",sz_fileName);
			throw CF_CErrorFile('O','M',errno,ERR_CLOSE_FILE,sz_errMsg,(char*)__FILE__,__LINE__);
		}
		file = 0;
		iPos = 0;
		RecCount=0;
		smmap=NULL;
	}
	return SUCCESS;
}


int CF_MemFileLO::setFileInfo(struct CDRFileInfo &e)
{
	char sz_headBuf[255],sz_buff[255];

	memset(sz_headBuf,' ',255);
	strncpy(sz_headBuf,e.sz_sourceId,5);
	strncpy(sz_headBuf+5,e.sz_begBillPeriod,strlen(e.sz_begBillPeriod));
	strncpy(sz_headBuf+13,e.sz_endBillPeriod,strlen(e.sz_endBillPeriod));
	strncpy(sz_headBuf+21,e.sz_beginDate,strlen(e.sz_beginDate));
	strncpy(sz_headBuf+35,e.sz_endDate,strlen(e.sz_endDate));
	sprintf(sz_buff,"%d",e.iFileId);
	sz_buff[8]=0;
	strncpy(sz_headBuf+49,sz_buff,strlen(sz_buff));
  sprintf(sz_buff,"%d",iPos);
  sz_buff[10]=0;
  strncpy(sz_headBuf+57,sz_buff,strlen(sz_buff));
  sprintf(sz_buff,"%d",e.iHotbill);
  sz_buff[1]=0;
  strncpy(sz_headBuf+67,sz_buff,strlen(sz_buff));
  sprintf(sz_buff,"%d",e.iTotalCDR);
  sz_buff[8]=0;
  strncpy(sz_headBuf+68,sz_buff,strlen(sz_buff));
  sprintf(sz_buff,"%d",e.iAvailCDR);
  sz_buff[8]=0;
  strncpy(sz_headBuf+76,sz_buff,strlen(sz_buff));
  sprintf(sz_buff,"%d",e.iInvalidCDR);
  sz_buff[8]=0;
  strncpy(sz_headBuf+84,sz_buff,strlen(sz_buff));
  sz_headBuf[92]='\n';
  sz_headBuf[93]=0;

  for(int i=0;i<strlen(sz_headBuf);i++)
    smmap[i]=sz_headBuf[i];

  return SUCCESS;
}

int CF_MemFileI::DbOpen(char *pch_FileName)
{
	char sz_sqlBuff[1000];
	
  strcpy(sz_fileName,pch_FileName);
  sz_fieldListBuff[0]='\0';
  
  strcpy(sz_fieldListBuff,(char *)"RECORDNO,");
  for(int i=0;i<iTxtNum;i++)
  {
	  sprintf(sz_fieldListBuff,"%s%s,",sz_fieldListBuff,txtfmtList[i].sz_colName);
  }

  sz_fieldListBuff[strlen(sz_fieldListBuff)-1]='\0';
  
  sprintf(sz_sqlBuff,"SELECT count(*)  FROM %s WHERE FILENAME='%s'",sz_tableName,pch_FileName);
	Ds01.Open(sz_sqlBuff,SELECT_QUERY);
	Ds01>>RecCount;
	Ds01.Close();

	sprintf(sz_sqlBuff,"SELECT %s  FROM %s WHERE FILENAME='%s' %s",sz_fieldListBuff,sz_tableName,pch_FileName,sz_orderByBuff);
	Ds01.Open(sz_sqlBuff,SELECT_QUERY);

  sprintf(sz_fieldListBuff,"DELETE FROM %s WHERE FILENAME='%s'",sz_tableName,pch_FileName);

	return SUCCESS;
}
int CF_MemFileI::DbOpen(char *pch_FileName,CBindSQL &DsOne)
{
	char sz_sqlBuff[1000];
	
  strcpy(sz_fileName,pch_FileName);
  sz_fieldListBuff[0]='\0';
  
  strcpy(sz_fieldListBuff,(char *)"RECORDNO,");
  for(int i=0;i<iTxtNum;i++)
  {
	  sprintf(sz_fieldListBuff,"%s%s,",sz_fieldListBuff,txtfmtList[i].sz_colName);
  }

  sz_fieldListBuff[strlen(sz_fieldListBuff)-1]='\0';
  
  sprintf(sz_sqlBuff,"SELECT count(*)  FROM %s WHERE FILENAME='%s'",sz_tableName,pch_FileName);
	DsOne.Open(sz_sqlBuff,SELECT_QUERY);
	DsOne>>RecCount;
	DsOne.Close();

	sprintf(sz_sqlBuff,"SELECT %s  FROM %s WHERE FILENAME='%s' %s",sz_fieldListBuff,sz_tableName,pch_FileName,sz_orderByBuff);
	DsOne.Open(sz_sqlBuff,SELECT_QUERY);

  sprintf(sz_fieldListBuff,"DELETE FROM %s WHERE FILENAME='%s'",sz_tableName,pch_FileName);

	return SUCCESS;
}

int CF_MemFileI::DbReadRec(CFmt_Change& change)
{
	char sz_buff[200];
  int i=1;
  Ds01>>sz_buff;
  iRecordNo=atoi(sz_buff);
  while(Ds01>>sz_buff)
  {
    change.Set_Field(i,sz_buff);
    i++;
    if(i>iTxtNum)
    {
    	return SUCCESS;
    }
  }
  Ds01.Close();
  return READ_AT_END;
}

int CF_MemFileI::DbReadRec(CFmt_Change& change,CBindSQL &DsOne)
{
	char sz_buff[200];
  int i=1;
  DsOne>>sz_buff;
  iRecordNo=atoi(sz_buff);
  while(DsOne>>sz_buff)
  {
    change.Set_Field(i,sz_buff);
    i++;
    if(i>iTxtNum)
    {
    	return SUCCESS;
    }
  }
  DsOne.Close();
  
  return READ_AT_END;
}

int CF_MemFileI::DbClose(CBindSQL &DsOne)
{
  if(iDelFlag==1)
  {
    DsOne.Open(sz_fieldListBuff,NONSELECT_DML);
    DsOne.Execute();
    DsOne.Close();
  }

	return SUCCESS;
}

int CF_MemFileI::DbClose()
{
  if(iDelFlag==1)
  {
    Ds02.Open(sz_fieldListBuff,NONSELECT_DML);
    Ds02.Execute();
    Ds02.Close();
    DBConn.Commit();
  }

	return SUCCESS;
}

char* CF_MemFileI::getRecordNo()
{
	char sz_buff[255];
	sprintf(sz_buff,"%s %d",sz_tableName,iRecordNo);
	return sz_buff;
}

int CF_MemFileO::DbOpen(char *pch_FileName)
{

  strcpy(sz_fileName,pch_FileName);

  iCheckFlag=0;
  RecCount=0;
  
	return SUCCESS;
}

int CF_MemFileO::DbWriteRec(CFmt_Change& change,char *pch_cmpareBuff)
{
	char sz_IntableName[100],sz_buff[100];
	int iInRecordNo;
	sz_IntableName[0]='\0';
	sz_buff[0]='\0';
  sscanf(pch_cmpareBuff,"%s %s",sz_IntableName,sz_buff);
  iInRecordNo=atoi(sz_buff);
  
	if((iInRecordNo!=0)&&(strcmp(sz_IntableName,sz_tableName)==0))//��¼��Ų�Ϊ0,������ͬ,��ʾupdate������
	{
		if(iCheckFlag==0)
    {
      char sz_sqlBuff[RECLEN];
    
      sz_fieldListBuff[0]='\0';
          
      for(int i=0;i<iTxtNum;i++)
      {
	      sprintf(sz_fieldListBuff,"%s%s=:%d,",sz_fieldListBuff,txtfmtList[i].sz_colName,i);
      }
      sz_fieldListBuff[strlen(sz_fieldListBuff)-1]='\0';
      sprintf(sz_sqlBuff,"UPDATE  %s SET %s  WHERE FILENAME ='%s' AND RECORDNO=:200",sz_tableName,sz_fieldListBuff,sz_fileName);
	    Ds01.Open(sz_sqlBuff,NONSELECT_DML);
	    iCheckFlag=1;
	  }
	  
	  for(int i=1;i<=iTxtNum;i++)
	  {
	  	char sz_buff[255];
	  	change.Get_Field(i,sz_buff);
	  	Ds01<<sz_buff;
	  }
	  Ds01<<iInRecordNo;
    RecCount++;
    if((RecCount%EXECUTE_NUM)==0)
      Ds01.Execute();

	  return SUCCESS;
	}
  else//��¼���Ϊ0����ʾinsert������
  {
    if(iCheckFlag==0)
    {
      char sz_valuesBuff[1000];
      char sz_sqlBuff[RECLEN];
    
      sz_valuesBuff[0]='\0';
      sz_fieldListBuff[0]='\0';
      
      strcpy(sz_fieldListBuff,"FILENAME,RECORDNO,");
      strcpy(sz_valuesBuff,":A,:B,");
      
      for(int i=0;i<iTxtNum;i++)
      {
	      sprintf(sz_fieldListBuff,"%s%s,",sz_fieldListBuff,txtfmtList[i].sz_colName);
	      sprintf(sz_valuesBuff,"%s:%d,",sz_valuesBuff,i);
      }
      sz_fieldListBuff[strlen(sz_fieldListBuff)-1]='\0';
      sz_valuesBuff[strlen(sz_valuesBuff)-1]='\0';
    
      sprintf(sz_sqlBuff,"INSERT INTO %s(%s) VALUES(%s)",sz_tableName,sz_fieldListBuff,sz_valuesBuff);
	    Ds01.Open(sz_sqlBuff,NONSELECT_DML);
	    iCheckFlag=1;
	  }
    
    Ds01<<sz_fileName<<RecCount+1;
	  for(int i=1;i<=iTxtNum;i++)
	  {
	  	char sz_buff[255];
	  	//change.Get_Field(i,sz_buff);
	  	Ds01<<change.Get_Field(i);
	  }
    RecCount++;
    if((RecCount%EXECUTE_NUM)==0)
    {
    	//printf("EXECUTE_NUM<%d>\n",RecCount);
      Ds01.Execute();
	  }
	  return SUCCESS;
	}
}

int CF_MemFileO::DbDeleteRec(char *pch_cmpareBuff)
{
	
  char sz_IntableName[100],sz_buff[255];
  int iInRecordNo;
	sz_IntableName[0]='\0';
	sz_buff[0]='\0';
  sscanf(pch_cmpareBuff,"%s %s",sz_IntableName,sz_buff);
  iInRecordNo=atoi(sz_buff);
	
	if(strcmp(sz_IntableName,sz_tableName)==0)
	{
	  sprintf(sz_buff,"DELETE FROM %s WHERE  FILENAME='%s' AND RECORDNO=:1",sz_IntableName,sz_fileName);
    Ds02.Open(sz_buff,NONSELECT_DML);
    Ds02<<iInRecordNo;
    //printf("delete<%d>\n",iRecordNumber);
    Ds02.Execute();
    Ds02.Close();
  }
  
  return SUCCESS;
}

int CF_MemFileO::DbClose()
{
	if((RecCount%EXECUTE_NUM)!=0)
	{
    Ds01.Execute();
	}
	Ds01.Close();
	DBConn.Commit();  
	return SUCCESS;
}

int CF_MemFileO::DbWriteRec(CFmt_Change& change,CBindSQL &DsOne,char *pch_cmpareBuff)
{
	char sz_IntableName[100],sz_buff[100];
	int iInRecordNo;
	sz_IntableName[0]='\0';
	sz_buff[0]='\0';
  sscanf(pch_cmpareBuff,"%s %s",sz_IntableName,sz_buff);
  iInRecordNo=atoi(sz_buff);
  
	if((iInRecordNo!=0)&&(strcmp(sz_IntableName,sz_tableName)==0))//��¼��Ų�Ϊ0,������ͬ,��ʾupdate������
	{
		if(iCheckFlag==0)
    {
      char sz_sqlBuff[RECLEN];
    
      sz_fieldListBuff[0]='\0';
          
      for(int i=0;i<iTxtNum;i++)
      {
	      sprintf(sz_fieldListBuff,"%s%s=:%d,",sz_fieldListBuff,txtfmtList[i].sz_colName,i);
      }
      sz_fieldListBuff[strlen(sz_fieldListBuff)-1]='\0';
      sprintf(sz_sqlBuff,"UPDATE  %s SET %s  WHERE FILENAME ='%s' AND RECORDNO=:200",sz_tableName,sz_fieldListBuff,sz_fileName);
	    DsOne.Open(sz_sqlBuff,NONSELECT_DML);
	    iCheckFlag=1;
	  }
	  
	  for(int i=1;i<=iTxtNum;i++)
	  {
	  	char sz_buff[255];
	  	change.Get_Field(i,sz_buff);
	  	DsOne<<sz_buff;
	  }
	  DsOne<<iInRecordNo;
    RecCount++;
    if((RecCount%EXECUTE_NUM)==0)
      DsOne.Execute();

	  return SUCCESS;
	}
  else//��¼���Ϊ0����ʾinsert������
  {
    if(iCheckFlag==0)
    {
      char sz_valuesBuff[1000];
      char sz_sqlBuff[RECLEN];
    
      sz_valuesBuff[0]='\0';
      sz_fieldListBuff[0]='\0';
      
      strcpy(sz_fieldListBuff,"FILENAME,RECORDNO,");
      strcpy(sz_valuesBuff,":A,:B,");
      
      for(int i=0;i<iTxtNum;i++)
      {
	      sprintf(sz_fieldListBuff,"%s%s,",sz_fieldListBuff,txtfmtList[i].sz_colName);
	      sprintf(sz_valuesBuff,"%s:%d,",sz_valuesBuff,i);
      }
      sz_fieldListBuff[strlen(sz_fieldListBuff)-1]='\0';
      sz_valuesBuff[strlen(sz_valuesBuff)-1]='\0';
    
      sprintf(sz_sqlBuff,"INSERT INTO %s(%s) VALUES(%s)",sz_tableName,sz_fieldListBuff,sz_valuesBuff);
	    DsOne.Open(sz_sqlBuff,NONSELECT_DML);
	    iCheckFlag=1;
	  }
    
    DsOne<<sz_fileName<<RecCount+1;
	  for(int i=1;i<=iTxtNum;i++)
	  {
	  	char sz_buff[255];
	  	//change.Get_Field(i,sz_buff);
	  	DsOne<<change.Get_Field(i);
	  }
    RecCount++;
    if((RecCount%EXECUTE_NUM)==0)
    {
    	//printf("EXECUTE_NUM<%d>\n",RecCount);
      DsOne.Execute();
	  }
	  return SUCCESS;
	}
}

int CF_MemFileO::DbDeleteRec(char *pch_cmpareBuff,CBindSQL &DsTwo)
{
	
  char sz_IntableName[100],sz_buff[255];
  int iInRecordNo;
	sz_IntableName[0]='\0';
	sz_buff[0]='\0';
  sscanf(pch_cmpareBuff,"%s %s",sz_IntableName,sz_buff);
  iInRecordNo=atoi(sz_buff);
	
	if(strcmp(sz_IntableName,sz_tableName)==0)
	{
	  sprintf(sz_buff,"DELETE FROM %s WHERE  FILENAME='%s' AND RECORDNO=:1",sz_IntableName,sz_fileName);
    DsTwo.Open(sz_buff,NONSELECT_DML);
    DsTwo<<iInRecordNo;
    DsTwo.Execute();
    DsTwo.Close();
  }
  
  return SUCCESS;
}

int CF_MemFileO::DbClose(CBindSQL &DsOne)
{
	if((RecCount%EXECUTE_NUM)!=0)
	{
    DsOne.Execute();
	}
	DsOne.Close();
	
	return SUCCESS;
}
