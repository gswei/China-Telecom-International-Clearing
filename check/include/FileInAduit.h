
//�˶��ļ����»����ļ����ݰ���ADU/SUM  �˶��ļ�������YYMM/DD��Ŀ¼,������ʱ�ļ�
//2013-07-26�˶��ļ����»����ļ��˶Խ������ͬһ�ű����棬���Ǵ�����̷ֿ�

//2013-08-07  ����ŵ���ʽ��֮ǰ���Խ��յ��ĺ˶��ļ�ͨ���ļ�����������Ŀ¼��ȥ���ң�����д�ļ��Ҳ���������״̬��N
//			  �´μ����Ժ˶�ʧ�ܵ��ļ����в��ң������ҵ�����ʾ�˶Գɹ���ͨ���ļ����ҵ��ļ���������Դ(ͨ���)�����ļ��ŵ����·��(��ʽ��������·��)
//2013-08-27��������ƽ̨,���ջ��ܺ��»��ֿܷ�,�»���һ����ͣ
//2013-08-30 �����˶��ļ���ȫ���ļ����ҵ��˲ŷ���ͬ����Ϣ,���˶�ʧ����ŵ�ʧ��Ŀ¼,��д���ݿ�
//2013-09-02 �»�����������ƽ̨
//2013-09-19 �»������ӽ�ÿ���ļ����͵ļ�¼��� D_MONTH_CHECK_RESULT
//2013-10-12 �˶��ļ��б���������ԴID�ֶ�,���ں�����ջ�������
//2013-10-20 �޸��ٲ�����,��Ϊֻ�ٲü�¼�ĸ���,��ֹ�ļ���̫��
//2013-10-24 D_CHECK_FILE_DETAIL�����ӷ��ʱ�־�ֶ�(CYCLE_FLAG), C_RATE_CYCLE��д���ʱ�־,������д�¸����ڵ�
//2013-11-12 �º˶���������,���½��������ʱ���,�Ž��к˶�
//2014-01-11 ��������·��ͨ��file.check.receive_path����ֵ��һλ�ַ���'/'�ж������·�����Ǿ���·��
//2014-08-04 �º˶�����ȡ����Դ ����֧�ֶ���ֶ�,�»������������ֶ�busi_type����ҵ������(�»����ļ���ʽ��ͬ),
//2014-10-15 ����ʱ���Ӹ�������,�º˶�Ӧ���ڷ���֮ǰ,ȥ������;�޸���ϵͳ�޷������պ˶Ժ��º˶ԵĴ���(��Ҫ��ǰ�ж�),

#include<iostream>
#include <vector>

#include "process.h"
//#include "bill_process.h"

//#include  "dr_api.h"
#include "dr_deal.h"
#include "RTInfo.h"

//#include "psutil.h"
//#include "tp_log_code.h"
#include "CF_Common.h"
#include "CF_CLogger.h"
#include "CF_CFscan.h"

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

/*
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
*/

//����Դ��Ϣ
struct SOURCECFG
{
	char szSourceId[6];		//����ԴID		
	char szSourcePath[256];   //����Դ����·��
	char filterRule[50] ;    //���˹���  ͨ��ƥ���ļ���������Դ
	int  file_begin;		   //��ȡ�ļ��������ʱ��
	int  file_length;

	SOURCECFG()
	{
		memset(szSourceId,0,sizeof(szSourceId));
		memset(szSourcePath,0,sizeof(szSourcePath));
		memset(filterRule,0,sizeof(filterRule));
		file_begin = -1;
		file_length =-1;
	}
};


//�˶��ļ��ļ�¼��ʽ
struct Check_Rec_Fmt
{
	string fileName;
	string rate_flag;
	string month;
};

//�»����ļ��������ļ���ʽ
struct  Check_Sum_Conf
{
	string check_type;
	string source_id;
	string sum_table;
	string cdr_count ;
	string cdr_duration;
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
		rate_cycle = "";
		cdr_fee.clear();
		busi_type = 0;
	}

};

//�»����ļ��ļ�¼��ʽ
struct  Check_Sum_Rec_Fmt
{
	string file_type;
	long cdr_count ;
	long cdr_duration;
	long cdr_fee;
};

class FileInAduit : public PS_Process
{
    public:
     FileInAduit();
     ~FileInAduit();    
	 
	 bool init(int argc,char** argv);                                               
	 int  LoadSourceCfg();
	 int getSourceFilter(char* source,char* filter,int &index,int &length);

	 //int dealFile();		//����˶��ļ�
	 //int dealMonthFile();  //�����»����ļ�
	
	 void execute();
	 void run(int flag = 1);
	 void run2();

	 int checkFile();
	 int checkMonthFile();

	 //int check_before_file();
	 int check_file_exist(char* file);
	 bool checkFormat(const char *cmpString, const char *format);
	
	 int updateDB();
	 void prcExit();

	 //bool drInit();
	 //bool CheckTriggerFile();
	 //int  drVarGetSet(char* m_SerialString);
	 //int IsAuditSuccess(const char* dealresult);

   private:
	
	 DBConnection conn;
	 
	 SParameter mConfParam;
	 //DRparameter mdrParam;
	 DR_Deal	 mdrDeal;

	 //char m_szSrcGrpID[8];  //����Դ��
	 //char m_szService[8];	//serviceID
	 //char m_szSourceID[8];  //����ԴID
	 //char mServCatId[5];
	 //char m_szOutTypeId[8];  //�ļ������ʽ
	
	 char receive_path[256];	 //ԭ�ļ�����·��
	 char output_path[256];     //ԭ�ļ����·��

	 char input_path[512];     //�˶��ļ�����·��	
	 char month_input_path[512];	//�»����ļ�����·��
	
	 char bak_path1[512];	    //�˶��ļ�����·��
	 char bak_path2[512];	    //�»����ļ�����·��
	 char fail_path[512];
	 char sql[1024];
	
	 char m_szSourceID[6];		//2013-10-12
	 char m_szFileName[256];  //�ļ���
	 char fileName[512];    //ȫ·���ļ���	 
	
	 char currTime[15];
	 char file_time[8+1];	 //���ļ��������ȡ

	 char erro_msg[1024];

	 short petri_status;	   //��ȡpetri��״̬
	 short petri_status_tmp;

	 vector<string>  m_vsql;

	 CF_CFscan scan;	//�ļ�ɨ��ӿ�	

	 map<string,SOURCECFG> m_SourceCfg; 
	 map<string,string>		 mapFileSource ;//�ļ���,����ԴID

	 vector<Check_Rec_Fmt> fileList;
	 map< string,Check_Sum_Conf > monthSumMap ;		//����»��ܵ������ļ���Ϣ
	 vector<Check_Sum_Rec_Fmt> fileList2;
};





