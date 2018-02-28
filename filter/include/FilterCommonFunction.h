
#ifndef _FILTER_COMMON_FUNCTION_H_
#define _FILTER_COMMON_FUNCTION_H_


#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <iostream.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fstream.h>
#include <string.h>

#include "CF_CLogger.h"
#include "CF_CException.h" 
#include "CF_Common.h"

#include "C_CFTime.h"

typedef long SIZE_TYPE;

const int  FILTER_ERR_CREATE_MEMORY = 7001;  //���������ڴ��ڴ�ʧ��
const int  FILTER_ERR_CONNECT_MEMORY = 7002;  //���ӹ����ڴ�ʧ���ڴ�ʧ��

const int  FILTER_ERR_IN_CREATE_DIR         = 7010;	//����·��
const int  FILTER_ERR_IN_CREATE_FILE  = 7011;	//�����ļ�
const int  FILTER_ERR_IN_OPEN_FILE = 7012;	//���ļ�����
const int  FILTER_ERR_IN_WRITE_FILE = 7013;	//д�ļ�����
const int FILTER_ERR_IN_READ_FILE = 7014;	//��ȡ�ļ���¼����

const int  FILTER_ERR_IN_CONNECT_DB         = 7020;  //�������ݿ����
const int  FILTER_ERR_NO_RECORD_INDB     = 7021;	//�鲻����ؼ�¼

const int  FILTER_ERR_UNKNOWN_CATCH         = 7099;  //��׽���޷�ʶ��Ĵ�����

const int  FILTER_ERR_IN_TIME_FIELD_FORMAT  = 7030;  //ʱ���ʽ����
const int	FILTER_ERR_OUT_OF_RANGE = 7031;	//������Χ




const int FILTER_ERRMSG_LEN = 500;	//������Ϣ����


const int FILTER_SOURCE_INDEX = 1;	//���������ڴ�ʱ����Դ�ṹ���õ�indexֵ
const int FILTER_FILE_INDEX = 2;	//���������ڴ�ʱ�ļ��ṹ���õ�indexֵ
const int FILTER_BLOCK_INDEX = 3;	//���������ڴ�ʱ���ݿ�ṹ���õ�indexֵ

const int FILTER_VALUESIZE = 17;			//һ��keyֵ���ĳ���
const int FILTER_MAXINDEXNO = 3000;		//5000һ�����д洢�����������
const int FILTER_FILESIZE = 256;			//�ļ�����

const int FILTER_SOURCE_PROCESS = 4;	//ÿ������Դ������ʵĽ����� (û��)
const int FILTER_PROCESS_INFO_LEN =  20;	//������Ϣ����(û��)

const int FILTER_INDEX_FILE_LEN = 10;	//�����ļ�����󳤶�

const int FILTER_BLOCKNUM_PER_FILE = 5;
const int FILTER_MAXLOADBLOCK = 20;	//20һ���ļ��������ڴ������ݿ�������
const int FILTER_MAXLOADFILE = 24;	//8һ������Դ֧�������ļ��б���(�����ڴ���)
const int FILTER_MAXSOURCE = 50;		//֧�ֵ��������Դ��

const int FILTER_MAXSECONDINFILE = 3600;	//�ļ������������б���


const char TMP_DATAHEAD_NAME[]="dataHead.tmp"; 	//���ݿ�ͷ��ʱ�ļ�
const char TMP_DATA_NAME[]="data.tmp";			//������ʱ�ļ�
const char TMP_FILEINFO_NAME[]="index.tmp";		//�ļ�ͷ��Ϣ��ʱ�ļ�

const char COMMIT_FILE[]="commit.flag";	//��־λ�ļ���
const char BACKUP_DATA_NAME[]="data.backup";		//�ڴ������ݿ鱸��
const char BACKUP_FILEINFO_NAME[]="index.backup";	//�ڴ����ļ�ͷ��Ϣ����

const char FILTER_WORK_PATH[]="WORK";	//������Ŀ¼




//int chkAllDir(char *path);
//int completeDir(char *pchPath);
//int chkDir(const char *_dir );

int openFile(fstream &file_Stream, char *fileName);
FILE * openfile(char *fileName, const char* mode);
void truncFile(const char* fileName);

//bool findInTemp(const char *file_Name, const long block_No, long &location );

#endif

