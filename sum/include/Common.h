#ifndef __SUM_COMMON_H__
#define __SUM_COMMON_H__


#include<iostream>
#include<vector>

//#include "psutil.h"
//#include "tp_log_code.h"

#include "CF_Common.h"
#include "CF_CLogger.h"

//using namespace tpss;  //��psutil.h��Ӧ

using namespace std;
//CLog theJSLog;

struct SettCon
{
	int  conSumt;				//��������
	char iDestTableName[32];	//Ŀ������
	char condition[100];		//ͳ������
};

struct SCom
{
	int iOrgSumt;            //ԭʼ���configid
	char szOrgSumtCol[30];   //ָ��ԭʼ��ʱ���ֶ���
	int iDestSumt;           //���ܽ�����configid
	char szDestSumtCol[30];  //���ܽ�����������ֶ�
	char iOrgTableName[30];  //ԭʼ�����
	char iDestTableName[30]; //���ܽ�������

	int count;
	SettCon* mscontion;
	SCom()
	{
		count = 1;
		mscontion = NULL;
	}
};

struct SItem
{
   char szItemName[32];//ͳ�Ʊ���ֶ���
   int iItemType;//�ֶ����ͣ�ͳ��ά��/ͳ���ֶΣ�
   int iSpecType;//�����ʶ����ȡϵͳʱ��ȣ�ͬͳ���ֲᣩ
   int iBeginPos;
   int iEndPos;

   char idefault[50];	//2014-01-09 ����Ĭ��ֵ
};

struct SItemPair
{
   SItem fromItem;  //Դͷ��ṹ
   SItem toItem;   //Ŀ���ṹ
};

//����SCom����ʼ��vItemInfo
int getItemInfo(SCom szSCom,vector<SItemPair> &vItemInfo);

int getSql(SCom szSCom,vector< SItemPair >vItemInfo,char *fromDateValue, char *sql,int type = 0,char* rate_cycle = NULL,int postion = 0);

int insertSql(char *sql);


int getDays(char* time);
#endif
