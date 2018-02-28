/****************************************************************
filename: CFileOut.h
module: CF - Common Function
created by: zhang guoqiang
create date: 2004-09-09
update list: 
version: 1.0.0
description:
    the header file of the classes for CFileOut
*****************************************************************/
#ifndef __CFILEOUT_H
#define __CFILEOUT_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "CFmt_Change.h"
#include "CF_CError.h"

#define SUCC 1//ִ�гɹ�;

/*
#define ERR_IN_FILE_OPEN -1//�ļ���ʧ�ܣ�
#define ERR_IN_WRITE_FILE_HEAD -2//д�ļ�ͷʧ�ܣ�
#define ERR_IN_WRITE_REC -3//д��¼ʧ�ܣ�
#define ERR_IN_FILE_CLOSED -4//�ļ����ڹر�״̬��
#define ERR_IN_WRITE_FILE_END -5//д�ļ�βʧ�ܣ�
*/
#define MAX 8000
class CFileOut
{
  private:
    FILE *fp;
    int recNum;
    int openSign;
    char file_name[255];
  public:
    int open(char *fileName);
    int writeRec(CFmt_Change &rec);
    int close(); 
    int close(int flag);
    
};
#endif