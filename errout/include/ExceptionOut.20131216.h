//2013-07-20 ��פ���̣�ɨ������ļ���Ϣ�����������Ϣд�쳣˵���ļ������Ѵ����Դ�ļ����ظ�����ϵͳ���ٵǼ��쳣˵���ļ�
//2013-07-22 ��������Ϣд����д������ļ��Ǽ�Ŀ¼
//2013-08-17    ���ӻ�ȡpetri״̬,�����ݿ�ֻ��ʱ��ֻд�ļ�
//2013-08-27  ��������ƽ̨
//2013-08-30 ɾ������ƽ̨,sql��д�ļ�ͨ����ȡ���ݿ�״̬(����sql����Ӱ����һ��ѭ��ִ��)
//2013-09-03 ��������ƽ̨,ɨ���ʱһ�����ļ����ٲ�

#include<iostream>
#include <vector>
#include <errno.h>

//#include "bill_process.h"
#include "process.h"
#include "CF_CFmtChange.h"

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

//����Դ��Ϣ
struct SOURCECFG
{
	char szSourceId[6];						  //����ԴID		
	char szSourcePath[JS_MAX_FILEPATH_LEN];   //����Դ����·��
	
	SOURCECFG()
	{
		memset(szSourceId,0,sizeof(szSourceId));
		memset(szSourcePath,0,sizeof(szSourcePath));
	}
};

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
	char szInPath[JS_MAX_FILEPATH_LEN];			//����·��(���·����	
	char szOutPath[JS_MAX_FILEPATH_LEN];		//�����ļ�·��			 
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
		//memset(szOutputFiletypeId,0,sizeof(szOutputFiletypeId));	
		//memset(iInputId,0,sizeof(iInputId));
		//memset(iOutputId,0,sizeof(iOutputId));
		memset(szInPath,0,sizeof(szInPath));
		memset(szOutPath,0,sizeof(szOutPath));	
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

//�쳣˵���ļ���Ϣ
struct ERRINFO
{
	string filename;
	char source_id[6];
	char err_msg[2];
	char err_code[10];
	int  err_col;
	int	 err_line;
	long err_seq;
};

class ExceptionOut : public PS_Process
{
    public:
     ExceptionOut();
     ~ExceptionOut();    
	 
	 bool init(int argc,char** argv);                                               

	 int getSourceFilter(char* source,char* filter);
	 int LoadSourceCfg();

	 int updateDB();

	 bool drInit();
	 bool CheckTriggerFile();
	 int  drVarGetSet(char* m_SerialString);
	 int IsAuditSuccess(const char* dealresult);

	 void run();
	 void prcExit();

   private:
	
	 DBConnection conn;
	 map<string,SOURCECFG> m_SourceCfg;   //�������Դ��������Ϣ
	 map<string,SOURCECFG>::const_iterator it ;
	
	 SParameter mConfParam;
	 DRparameter mdrParam;

	 //char m_szSrcGrpID[8];  //����Դ��
	 //char m_szService[8];	//serviceID
	 char m_szSourceID[8];  //����ԴID
	 //char mServCatId[5];
	 //char m_szOutTypeId[8];  //�ļ������ʽ
	
	 char m_szFileName[JS_MAX_FILENAME_LEN];		//ԭʼ�ļ���
	 //char input_path[JS_MAX_FILEPATH_LEN];			//�������·��
	 //char out_path[JS_MAX_FILEFULLPATH_LEN];	    //�������·��
 
	 char outFileName[JS_MAX_FILEFULLPATH_LEN];		//���ȫ·���ļ��� 
	 char currTime[15];
	 short petri_status;	   //��ȡpetri��״̬ 
	
	 char sql[JS_MAX_SQL_LEN];
	 char erro_msg[JS_MAX_ERRMSG_LEN];

	 map< string,vector<ERRINFO> > erroinfoMap ;
	 vector<string>  vsql;

	 //bool m_enable ;	//����״̬
	 //int drStatus;     //ϵͳ״̬ 0��ϵͳ,1��ϵͳ,2������ϵͳ
	 //char m_SerialString[4096];
	 //char m_AuditMsg[4096];

};





