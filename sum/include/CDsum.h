
#ifndef __SUM_DAY_H__
#define __SUM_DAY_H__

#include<iostream>
#include "Common.h"

#include "process.h"
#include "RTInfo.h"

//#include "dr_api.h"
#include "dr_deal.h"

using namespace std;

struct SDayList
{
	char szSourceGrpId[16];   //����Դ��ID
	char szSourceId[16];	  //����ԴID
	//char szFlag[2];			  //�Ƿ���Ч��־  ��ʱû��
	vector<SItemPair> vItemInfo;
	SCom tableItem;
	//char org_source[16];	//2013-08-19 ָ��ԭʼ�������ԴID ͬһ���嵥�������ж������ԴIDʱ��������
	//char rate_cycle[20];
};

struct  Check_Sum_Conf
{
	string check_type;
	string source_id;
	string sum_table;
	string cdr_count ;
	string cdr_duration;
	string cdr_dataflow;
	vector<string> cdr_fee;
	string rate_cycle;
	int busi_type;

	void clear()
	{
		check_type = "";
		source_id = "";
		sum_table = "";
		cdr_count = "";
		cdr_duration = "";
		cdr_dataflow = "";
		rate_cycle = "";
		cdr_fee.clear();
		busi_type = 0;
	}

};


class CDsum:public PS_Process
{
	public:	

		DBConnection conn;
		SDayList *pDayList;

		char szSumDate[8+1];//��Ҫ���ջ��ܵ�����
		char erro_msg[2048];
		char sql[2048];

		char currTime[15];
		int iSourceCount ;		//��Ч������Դ����
		bool flag1;				//�Ƿ�פ
		//bool flag2;				//�Ƿ�ɾ����ǰ�ļ�¼
		
		SGW_RTInfo	rtinfo;
		char module_id[32];
		long module_process_id;

		short petri_status;
		
		DR_Deal	 mdrDeal;
		//bool m_enable ;		//����״̬
		//int drStatus;     //ϵͳ״̬ 0��ϵͳ,1��ϵͳ,2������ϵͳ	
		//char m_SerialString[4096];
		//char m_AuditMsg[4096];
		map< string,Check_Sum_Conf > monthSumMap ;

	public:

		CDsum();
		~CDsum();

		bool init(int argc,char** argv);
		bool init();
		bool init(char *source_id, char *source_group_id);
	
		int  loadSumConfig(char *source_id, char *source_group_id,int pos);
		
		void setDate(char* date);
		void setDaemon(bool flag = true);
		
		//bool drInit();
		//bool CheckTriggerFile();
		//int  drVarGetSet(char* m_SerialString);
		//int IsAuditSuccess(const char* dealresult);

		//int getSql(SCom szSCom,vector< SItemPair >vItemInfo,char *fromDateValue,char* org_source,char* source_id,char *sql);	//�������

		//�ж��ջ��������������ṹSDayList�����е��ļ����ﵽ�������������ݲ�ͬ����������ز�ֵͬ��ע���������ܺ��ػ��ܵķ���ֵ
		int checkDayCondition(SDayList  &Sday,char *sumday);

		int sum(SDayList &Sday,char *sumday);
		int redosum(SDayList &Sday,char *sumday,bool del);

		void run();
		int redorun(char* date,bool del = true);  //Ĭ��ɾ����ǰͳ�Ƶ�����
		
		void prcExit();

	private:
		//��D_SUMMERY_RESULT��ӿ�
		//��D_SCH_FORMAT�ӿ�
		//��D_CHECK_FILE_DETAIL�ӿ�
		//��D_OUT_FILE_REG�ӿ�
};

#endif
