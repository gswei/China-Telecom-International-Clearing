
#ifndef _CF_CFILTERPLUGIN_H_
#define _CF_CFILTERPLUGIN_H_

#include "CF_CPlugin.h"
#include "CF_CHash.h"
#include "CF_CInterpreter.h"
//#include "CF_COracleDB.h"
#include "C_CMemFilter.h"
#include "CF_CPluginPacket.h"
#include "CF_Config.h"

#include "psutil.h"
using namespace std;
using namespace tpss;

const int FILTER_MAX_KEYNUM = 30;

struct SPickKey
{
	char szFileType[20];	//�ļ�����
	char szColName[50];	//�ֶ���
	char szTimeFlag;	//�Ƿ�ʱ���־Y:ʱ�� N:��ʱ��
	char szKeyType;	//�ؼ������� I:��ʾ�ֶ�ֵͨ�����ʽ�ϳ� F:ֱ��ȡ�ֶ�
	char szPickKeyFormat[256];	//���ʽ�ִ�
	int iLen;	//ÿ���ֶγ���
	SPickKey()
	{
		memset(szFileType, 0, sizeof(szFileType));
		memset(szColName, 0, sizeof(szColName));
		memset(szPickKeyFormat, 0, sizeof(szPickKeyFormat));
		iLen = 0;
	};
	SPickKey& operator = (const SPickKey& source)
	{
		if(this==&source)
			return *this;
		
		strcpy(this->szFileType, source.szFileType);
		strcpy(this->szColName, source.szColName);
		strcpy(this->szPickKeyFormat, source.szPickKeyFormat);
		this->szTimeFlag = source.szTimeFlag;
		this->szKeyType = source.szKeyType;
		this->iLen = source.iLen;
		return *this;
	};
};

struct SSourceEnv
{
	char szPickConfigId[20];	//ȥ�عؼ�����
	long lPickLen;	//�ؼ���ѹ����ĳ���(������ʱ���ֶ�)
	SPickKey key[FILTER_MAX_KEYNUM];
	int ikeyCount;	//����ʱ���ֶ�,��key��Ѱַʱ��0��ʼ����
	SSourceEnv()
	{
		memset(szPickConfigId, 0, sizeof(szPickConfigId));
		lPickLen = 0;
		ikeyCount = 0;
	}
	SSourceEnv& operator = (const SSourceEnv& source)
	{
		if(this==&source)
			return *this;
		
		strcpy(this->szPickConfigId, source.szPickConfigId);
		this->lPickLen = source.lPickLen;
		this->ikeyCount = source.ikeyCount;
		for(int i=0; i<this->ikeyCount; i++)
		{
			this->key[i] = source.key[i];
		}
		return *this;
	};
};



class C_CFilterPlugin:public BasePlugin
{
	public:
		static C_CFilterPlugin* getInstance()
		{
			if(icout >=1)
			{
				char szLogStr[500];
				sprintf(szLogStr, "�ظ�����ȥ�ز��");
				throw CException(1, szLogStr, __FILE__, __LINE__);
			}
			icout++;
			pfilterplugin = new C_CFilterPlugin();
			return pfilterplugin;
		}
		
		void init(char *szSourceGroupID, char *szServiceID, int index);

		void execute(PacketParser& ps, ResParser& retValue);
		
		void dealNegativeExecute(CFmt_Change & org_rcd);

		void message(MessageParser&  pMessage);

		std::string getPluginName()
		{
			return "filter";
		}

		std::string getPluginVersion(){
			return "3.0.0";
		}
		
		void printMe(){
			//��ӡ�汾��
			printf( "\t�������:ȥ��,�汾�ţ�123 \n");
			return;
		};
		~C_CFilterPlugin();

	private:
		static C_CFilterPlugin* pfilterplugin;
		static int icout ;
		C_CFilterPlugin();
		void InitPickKey(const char *szSourceGroupID, const char *szServiceID);
		C_CMemFilter filter;
		CF_CHash hash;
		Interpreter interpreter;
		map<string, SSourceEnv> mapSourceKey;
		SSourceEnv source;
		char m_szComFile[FILTER_FILESIZE];
		char m_szFileName[FILTER_FILESIZE];
		char m_szSourceGroup[20];
		char m_szSourceId[20];
		CFmt_Change inrcd;
};


void selectFromProcessEnv(const char *szGroupId, const char *szServiceId, const char *szEnvName, char *szEnvValue);
void selectFromSourceEnv(const char *szSourceId, const char *szServiceId, const char *szEnvName, char *szEnvValue);

int TransHToD(char *instr,char *outstr,int outstrlen);

#endif
