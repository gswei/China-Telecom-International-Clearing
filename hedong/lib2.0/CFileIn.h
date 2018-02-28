/****************************************************************
filename: CFileIn.h
module: CF - Common Function
created by: zhang guoqiang
create date: 2004-09-09
update list: 
version: 1.0.0
description:
    the header file of the classes for CFileIn
*****************************************************************/
#ifndef __CFILELN_H
#define __CFILELN_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "CFmt_Change.h"
#include "CF_CError.h"

#define SUCC 1//����ִ�гɹ�
/*
#define ERR_IN_FILE_CLOSED -1//�ļ����ڹر�״̬
#define ERR_IN_FILE_HEAD -2//�ļ�ͷ����
#define ERR_IN_FILE_BYTE -3//�ļ��ֽ���������
#define ERR_IN_FILE_END -4//�ļ�β����
#define ERR_IN_FILE_NUM -5//�ļ���¼��������
#define ERR_IN_FILE_OPEN -6//���ļ�����
#define ERR_IN_READ_REC -7//��ȡ��¼����
*/
#define ERR_IN_AT_END -8

#define MAX 8000

class CFileIn
{
  private:
    FILE *fp;
    int openSign;
    int recNum;
  public: 
    int open(char *fileName);
    int readRec(CFmt_Change &rec);
    int close();
};
#endif
