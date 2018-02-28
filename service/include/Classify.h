/****************************************************************
filename: classify.h
module:
created by: ouyh
version: 3.0.0
update list:
****************************************************************/
#ifndef _CLASSIFY_H_
#define _CLASSIFY_H_ 1

#include "CF_Common.h"
#include "CF_Config.h"
//#include "CF_COracleDB.h"
#include "CF_CFmtChange.h"
#include "CF_CMemFileIO.h"
#include "CF_CErrorLackRec.h"
#include "CF_CStat.h"
#include "CF_CRecordChange.h"
#include "CF_PREP_Error.h"
#include "psutil.h"

string& replace_all(string& str,const string& old_value,const string& new_value);

class CClassifyRule
{
	//private:
	public:
		/* ��ʼ����־ */
		int iInitFlag;
		/*�ּ��־:BΪ����;OΪ�����̻���*/
		char m_PickFlag[1+1]; 
		/*�ּ�Ŀ�꣺���ļ�(F)/���(T)*/
		char m_PickTarget[1+1];
		/*�ּ�Ľ���ļ��ļ����ĺ�׺*/
		char m_FileNamePostfix[FILE_NAME_LEN+1];
		/*�ּ�Ľ���ļ��ļ�����ǰ׺*/
		char m_FileNamePrefix[FILE_NAME_LEN+1];
		/*���m_PickTargetΪF,��Ϊ�ּ�·������"/"��ͷΪ����·��������Ϊ�����source_path�����·��
		*���m_PickTargetΪT,��Ϊ����������*/
		char m_PickPath[PATH_NAME_LEN+1];
		/*�ּ�Ļ������ͣ�IΪ��ģ������뻰����ʽ;����Ϊָ�������ʽ*/
		char m_OutputType[5+1];
		CFmt_Change m_OtherFileType;//���ڶ�Ӧ�ּ���ļ�ʱ��3���ļ���ʽ
		CFmt_Change m_InFileFmt;
		CRecord_Change m_RecordChange;//����ת�����������ʽ
		/*�ּ����ͱ�ʶ*/
		char m_PickType[32];
		/*ͳ����������Ϊ"N"��ʾ����Ҫͳ�����;����Ϊͳ�����������,����֧�ֶ��*/
		char m_StatConfig[TABLENAME_LEN+1];
		/*�ּ����ı���*/
		char m_PickTableName[TABLENAME_LEN+1];


		C_Compile m_Compile;

		char m_ServiceId[FILE_NAME_LEN+1];
		char m_SourceId[FILE_NAME_LEN+1];
		char m_SourcePath[PATH_NAME_LEN+1];
		char m_FileName[FILE_NAME_LEN+1];
		char m_Time[14+1];
		char m_PickFileName[FILE_NAME_LEN+1];

		CF_CErrorLackRec* m_Pick2Table;
		int m_Pick2TabConfigId;
		int m_PickRecordCount;
		
		char m_InFileType[5+1];
		char m_InRcdType;
		char m_OutFileType[5+1];
		char m_OutRcdType;
		
		char tmp_DealMonth[6+1];
		char m_LogFlag[2];//�Ƿ�д��־��D_FORMAT_STAT_LOG
		vector<string> filename;

		DBConnection conn;//���ݿ�����

		struct sPickInfo
		{
			//unsigned int iPathId;
			int iPickNum;
			char szRealName[FILE_NAME_LEN+PATH_NAME_LEN+1];
			char szTmpName[FILE_NAME_LEN+PATH_NAME_LEN+1];
			char szOutPath[FILE_NAME_LEN+PATH_NAME_LEN+1];
			FILE *fp;
			int iEndFlag;
			sPickInfo()
			{
				memset(szRealName, 0, sizeof(szRealName));
				memset(szTmpName, 0, sizeof(szTmpName));
				memset(szOutPath, 0, sizeof(szOutPath));
				iPickNum = 0;
				iEndFlag = 0;
				fp = NULL;
			};
		};
		//typedef map<unsigned int, sPickInfo*> MAP_PINFO;	
		typedef vector<sPickInfo> VEC_PINFO;
		VEC_PINFO m_OutInfo;
		
		void checkPath(char* szPath, char* szTime);
	public:
		CClassifyRule();
		~CClassifyRule();
		int setRule(int RuleId);
		bool initFile(char* ServiceId, char* SourceId, char* SourcePath, char* FileName, char* FileType, char* StartTime);
		void newFile(char* ServiceId, char* SourceId, char* SourcePath, char* FileName, char* StartTime);
		void endFile();
		int dealRecord(CFmt_Change& inrcd);
		bool commit();
		bool rollback();
		void resetDealNum();
		int getDealNum();
		int getInitFlag();
		void getFilename();
		
};

typedef map<int, CClassifyRule> CLASSIFY_RULE;
class CClassify
{
	private:    //Ϊ�˻�ȡ���ݣ��޸�����	
		char m_ServiceId[FILE_NAME_LEN+1];
		char m_SourceId[FILE_NAME_LEN+1];
		char m_SourcePath[PATH_NAME_LEN+1];
		char m_FileName[FILE_NAME_LEN+1];
		char m_FileType[5+1];
		char m_RcdType;
		char m_Time[14+1];
		bool m_NewFile;
		CLASSIFY_RULE rule;

		int m_Initflag;
	public:
		CClassify();
		~CClassify();
		bool initFile(char* ServiceId, char* SourceId, char* SourcePath, char* FileName, char* FileType, char* StartTime);
		void endFile();
		int dealRecord(char* iRuleId, CFmt_Change& inrcd);
		bool commit();
		bool rollback();
		void getAllRule(int& tmprule,vector<string>& rule_filename);
};

unsigned int crc32(char *string ,int len);

#endif
