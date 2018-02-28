
//2013-07-23 ,���Ƿ�������ļ������·�����óɿ����õģ�c_source_audit_env
//2013-08-27  ��������ƽ̨,ÿ�մ���
//2013-08-30 �����֣�,ͨ����ȡ���ݿ�״̬�ж��Ƿ�(����sql����Ӱ����һ��ѭ��ִ��)
#include<iostream>
#include <vector>

//#include "bill_process.h"
#include "process.h"

#include "psutil.h"
#include "tp_log_code.h"

#include "RTInfo.h"
#include "dr_api.h"

using namespace tpss;  //��psutil.h��Ӧ
using namespace std;

const int JS_MAX_SQL_LEN			=	1024;
const int JS_MAX_FILENAME_LEN		= 	256;
const int JS_MAX_FILEPATH_LEN		=	256;
const int JS_MAX_FILEFULLPATH_LEN	=	512;
const int JS_MAX_ERRMSG_LEN			=	1024;
const int JS_MAX_RECORD_LEN			=	1024;

//����ˮ��Ϊ��λ
struct SParameter
{
	//char szSrcGrpID[6];							//����Դ��
	char szService[6];							//serviceID
	//char  iWorkflowId[10];						//������ģ��ID  
	int  iflowID;								//��ˮ��ID
	int  iModuleId;								//ģ���ʶ
	//char  iInputId[10];							//����ӿ�ID
	//char  iOutputId[10];						//����ӿ�ID
	//char szSchCtlTabname[32+1];					//���ȱ��

	//char szInPath[JS_MAX_FILEPATH_LEN];			//����·��(���·����	
	//char szOutPath[JS_MAX_FILEPATH_LEN];		//�����ļ�·��
			 
	int szSleepTime;							//ÿ������ʱ��

	//char szDebugFlag[50];						//�Ƿ����������־

	SParameter()
	{
		iflowID = -1;
		iModuleId = 0;		
		szSleepTime = 5;
		//memset(iWorkflowId,0,sizeof(iWorkflowId));
		//memset(szSrcGrpID,0,sizeof(szSrcGrpID));
		memset(szService,0,sizeof(szService));
		//memset(szSchCtlTabname,0,sizeof(szSchCtlTabname));
		//memset(iInputId,0,sizeof(iInputId));
		//memset(iOutputId,0,sizeof(iOutputId));
		//memset(szInPath,0,sizeof(szInPath));
		//memset(szOutPath,0,sizeof(szOutPath));	
	}
};

struct DRparameter
{
	 bool m_enable ;						//����״̬
	 int  drStatus;							//ϵͳ״̬ 0��ϵͳ,1��ϵͳ,2������ϵͳ
	 char m_SerialString[4096];				//ͬ����
	 char m_AuditMsg[4096];					//�ٲô�

	 DRparameter()
	 {
		m_enable = false;
		drStatus = 2;
		memset(m_SerialString,0,sizeof(m_SerialString));
		memset(m_AuditMsg,0,sizeof(m_AuditMsg));
	 }
};

//������������Ϣ
struct AduitEnv
{
	vector<string> arrayFile;
	char out_path[512];
	char null_out_flag;
};

class FileOutAduit : public PS_Process
{
    public:
     FileOutAduit();
     ~FileOutAduit();    
	 
	 bool init(int argc,char** argv);                                               
	 void run();
	 int updateDB();
	 void prcExit();

	 bool drInit();
	 bool CheckTriggerFile();
	 int  drVarGetSet(char* m_SerialString);
	 int IsAuditSuccess(const char* dealresult);

	 bool checkAuditBefore(char* date);

	 void clearMap();
	
   private:
	
	 DBConnection conn;
	 
	 SParameter mConfParam;
	 DRparameter mdrParam;

	 char currTime[15];
	 char m_szFileName[JS_MAX_FILENAME_LEN];  //�ļ��� 
	 char sql[JS_MAX_SQL_LEN];
	 char erro_msg[JS_MAX_ERRMSG_LEN];
	 
	 short petri_status;					//��ȡpetri��״̬ 

	 vector<string> fileList;
	 map< string,vector<string> > sourceMap ;
	 map< string,AduitEnv > fileNameMap ;

	 vector<string>  vsql;
};





