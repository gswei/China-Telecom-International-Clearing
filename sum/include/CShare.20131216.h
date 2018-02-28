
#ifndef __SUM_SHARE_H__
#define __SUM_SHARE_H__

#include<iostream>
#include "Common.h"

#include<sys/types.h>
#include<dirent.h>

#include "CfgParam.h"
#include "RTInfo.h"
#include "dr_api.h"

using namespace std;

//�������
struct SShareFee
{
	char settlesum_col[20];
	char settle_formula[20];
	char settle_formula_param[20];
};

//����ռ��
struct SSharePercent
{
	char base_col[20];
	char province_col[20];
	int percent_pos;
	char outtable_name[20];
};


struct SShareList 
{
	char szSourceGrpId[6];   //����Դ��ID
	char szSourceId[6];	  //����ԴID
	char szFlag[2];			  //�Ƿ���Ч��־  ��ʱû��

	//char szSettleId[6];           //̯������ID
	//int szSettleType;           //�������ͣ�1�����ܷ��ã�2����ʹ��ռ��

	vector<SItemPair> vItemInfo;
	SCom tableItem;

	bool shareFeeFlag;
	bool sharePercentFlag;
	SShareFee stShareFee;
	SSharePercent stSharePercent;
};

class CShare
{
	public:	

		DBConnection conn;
		SShareList  *pShareList;

		char szSumMonth[6+1];//��Ҫ���»��ܵ��·�
		char erro_msg[2048];
		char sql[2048];
		char currTime[15];
		int iSourceCount ;		//��Ч������Դ����

		IBC_ParamCfgMng param_cfg;

		short petri_status;
		//bool m_enable ;		//����״̬
		//int drStatus;     //ϵͳ״̬ 0��ϵͳ,1��ϵͳ,2������ϵͳ	
		//char m_SerialString[4096];
		//char m_AuditMsg[4096];
		
		SGW_RTInfo	rtinfo;
		char module_id[32];
		long module_process_id;
		string m_triggerFile;

	public:

		CShare();
		~CShare();

		bool init(int argc,char** argv);

		bool init(char* month);
		bool init(char* source_id, char* source_group_id,char* month);
		
		int  loadSumConfig(char *source_id, char *source_group_id,int pos);
		int	 loadShareConfig(char*source_id,SShareList &Share);

		void setDate(char* date);
		
		//bool drInit();
		//bool CheckTriggerFile();
		//int  drVarGetSet(char* m_SerialString);
		//int IsAuditSuccess(const char* dealresult);

		//�ж��ջ��������������ṹSDayList�����е��ļ����ﵽ�������������ݲ�ͬ����������ز�ֵͬ��ע���������ܺ��ػ��ܵķ���ֵ
		int checkSettCondition(SShareList   &Smonth,char *sumMonth);

		int sum(int deal_type,SShareList  &Smonth,char *sumMonth,bool del);
		int shareCal(SShareList  &Smonth,Statement &stmt);

		int run(int deal_type,bool del);  //Ĭ��ɾ����ǰͳ�Ƶ�����

	private:
		//��D_SUMMERY_RESULT��ӿ�
		//��D_SCH_FORMAT�ӿ�
		//��D_CHECK_FILE_DETAIL�ӿ�
		//��D_OUT_FILE_REG�ӿ�
};

#endif





