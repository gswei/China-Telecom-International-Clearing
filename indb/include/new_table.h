
#include<iostream>
#include <vector>
#include<dirent.h>

#include "psutil.h"
#include "tp_log_code.h"
#include "CfgParam.h"

#include "CF_Common.h"
#include "CF_CLogger.h"
#include "CF_CFmtChange.h"

#define  STAT_INSERT_CONNT             500	//���¿��ʱһ�β���ļ�¼��
#define	STAT_ITEM_COUNT                100    //�������
#define	STAT_ITEM_NAME_LEN             50    //����������
#define	STAT_ITEM_CONTAINER_LEN        251    //�������ݳ���
#define  STAT_GROUP_COUNT              10    //ͳ�Ʊ�ʶ�����
#define  MAX_STATID_NUM                50    //һ��������������ͳ��ʶ����
#define  SQL_LENGTH					   2000

using namespace tpss;						//��psutil.h��Ӧ
using namespace std;

#define COMMIT_COUNT	500					//ÿ���ύ500��,��ʧ��,ͨ��file_idɾ����ǰ�ύ������

struct STAT_TABLE
{	
	char SzTable_Name[50];//����
	int  NStat_Item_Count;//���ֶθ���
	char SzStat_Item[STAT_ITEM_COUNT][STAT_ITEM_NAME_LEN];		//����ֶ���
	char SzStat_FieldName[STAT_ITEM_COUNT][STAT_ITEM_NAME_LEN];	//�ļ��ֶ���
	int  NItem_Type[STAT_ITEM_COUNT];
	int  NStat_Item_Begin[STAT_ITEM_COUNT];
	int  NStat_Item_End[STAT_ITEM_COUNT];
	char NSql[1024];	
};


class CF_CNewError_Table
{
	public:

		CF_CNewError_Table();
		~CF_CNewError_Table();

		bool init(int config_id,char* OutTypeId);
		
		int setConfig(long file_id,char* rate_cycle);
		int	setBegin(DBConnection& conn);
		int	setRWFlag(bool flag);
		
		int setSQLconf(char* name,int count);

		bool insertData(char* data);	
		int Commit();
		int RollBack();

		void writeSQL(char* data);
		//vector<string> getvSQL();

	private:
		bool mRWFlag;
		int  m_count ;
		long mFileid;
		char mRateCycle[6+1];
		char m_szOutTypeId[10];
		
		char sql_path[256];
		char mFullSqlName[512];

		char erro_msg[2048];
		char sql[1024];
		STAT_TABLE m_SStat_Table;
		CFmt_Change outrcd;
		Statement stmt;

		//vector<string>  vsql;
		string* vsql;
		
};