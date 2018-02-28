
#ifndef __SUM_DAY_H__
#define __SUM_DAY_H__

#include<iostream>
#include "Common.h"
#include "bill_process.h"

using namespace std;

struct SDayList
{
	//map<int,string>allFiles;  //�ļ�ID���ļ����ļ���
	char szSourceGrpId[16];   //����Դ��ID
	char szSourceId[16];	  //����ԴID
	//char szFlag[2];			  //�Ƿ���Ч��־  ��ʱû��
	vector<SItemPair> vItemInfo;
	SCom tableItem;
	char org_source[16];	//2013-08-19 ָ��ԭʼ�������ԴID ͬһ���嵥�������ж������ԴIDʱ��������
};

class CDsum:public PS_Process
{
	public:	

		DBConnection conn;
		SDayList *pDayList;

		char szSumDate[8];//��Ҫ���ջ��ܵ�����
		char erro_msg[1024];
		char sql[1024];
		char sqlFile[1024];		//ά��̬sqlд�ļ�

		char currTime[15];
		int iSourceCount ;		//��Ч������Դ����
		bool flag1;				//�Ƿ�פ
		//bool flag2;				//�Ƿ�ɾ����ǰ�ļ�¼

	public:

		CDsum();
		~CDsum();

		bool init(int argc,char** argv);
		bool init();
		bool init(char *source_id, char *source_group_id);
	
		int  loadSumConfig(char *source_id, char *source_group_id,int pos);
		
		void setDate(char* date);
		void setDaemon(bool flag = true);

		//int getSql(SCom szSCom,vector< SItemPair >vItemInfo,char *fromDateValue,char* org_source,char* source_id,char *sql);	//�������

		//�ж��ջ��������������ṹSDayList�����е��ļ����ﵽ�������������ݲ�ͬ����������ز�ֵͬ��ע���������ܺ��ػ��ܵķ���ֵ
		int checkDayCondition(SDayList  &Sday,char *sumday);

		int sum(SDayList &Sday,char *sumday);
		int redosum(SDayList &Sday,char *sumday,bool del);

		void run();
		int redorun(char* date,bool del = true);  //Ĭ��ɾ����ǰͳ�Ƶ�����

	private:
		//��D_SUMMERY_RESULT��ӿ�
		//��D_SCH_FORMAT�ӿ�
		//��D_CHECK_FILE_DETAIL�ӿ�
		//��D_OUT_FILE_REG�ӿ�
};

#endif
