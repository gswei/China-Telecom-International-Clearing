
#ifndef __SUM_MON_H__
#define __SUM_MON_H__

#include<iostream>
#include "Common.h"

#include<sys/types.h>
#include<dirent.h>

#include "CfgParam.h"

#include "RTInfo.h"
//#include "dr_api.h"

using namespace std;

struct SMonthList
{
	char szSourceGrpId[16];   //����Դ��ID
	char szSourceId[16];	  //����ԴID
	//char szFlag[2];			  //�Ƿ���Ч��־  ��ʱû��
	vector<SItemPair> vItemInfo;
	SCom tableItem;
};

class CMsum
{
	private:	

		DBConnection conn;
		SMonthList *pMonthList;
	
		int iSourceCount ;		//��Ч������Դ����
		char szSumMonth[6+1];//��Ҫ���»��ܵ��·�
		char currTime[15];
		char erro_msg[2048];
		char sql[2048];
		
		short petri_status;
		//bool m_enable ;		//����״̬
		//int drStatus;     //ϵͳ״̬ 0��ϵͳ,1��ϵͳ,2������ϵͳ	
		//char m_SerialString[4096];
		//char m_AuditMsg[4096];
		
		SGW_RTInfo	rtinfo;
		char module_id[32];
		long module_process_id;
		string m_triggerFile;

		//map<string,string> fileTypeSourceMap;			//�ļ�����Ӧ����Դ
		IBC_ParamCfgMng param_cfg;

	public:

		CMsum();
		~CMsum();
		
		bool init(int argc,char** argv);
		bool init(char* month);
		bool init(char* source_id, char* source_group_id,char* month);
		
		int  loadSumConfig(char *source_id, char *source_group_id,int pos);
		
		void setDate(char* date);
		
		//bool drInit();
		//bool CheckTriggerFile();
		//int  drVarGetSet(char* m_SerialString);
		//int IsAuditSuccess(const char* dealresult);

		//�ж��ջ��������������ṹSDayList�����е��ļ����ﵽ�������������ݲ�ͬ����������ز�ֵͬ��ע���������ܺ��ػ��ܵķ���ֵ
		int checkMonthCondition(SMonthList  &Smonth,char *sumMonth);

		int sum(int deal_type,SMonthList &Smonth,char *sumMonth,bool del);
		//int redosum(SMonthList &Smonth,char *sumMonth,bool del);

		int run(int deal_type,bool del);  //Ĭ��ɾ����ǰͳ�Ƶ�����

	private:
		//��D_SUMMERY_RESULT��ӿ�
		//��D_SCH_FORMAT�ӿ�
		//��D_CHECK_FILE_DETAIL�ӿ�
		//��D_OUT_FILE_REG�ӿ�
};

#endif



