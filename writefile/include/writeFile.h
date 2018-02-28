//2013-07-15	//��һ���ļ�д���������ʱĳ��������ļ�¼��Ϣ��������˸��ļ��Ѿ�����Ļ����飬����ɾ��������¼������Ϣ��sql�ļ���
		        //�������¸��ļ�ʱ��д����ǼǱ�����Դ�ļ�copy������Ŀ���ļ�
//2013-07-18	����־�����÷ŵ����Ĳ������棬������־д������
//2013-07-29	�����ļ���غ���⹦��,�޸�CF_CError_Table�ӿ�setFileName()����Statementʹ���ܻع�ǰ�������
//2013-08-01    ������ʱĿ¼WR_FILE_TMP_DIR  ���ʧ�ܺ������ļ��Ĵ��Ŀ¼
//2013-08-13	���Ӵ��ļ����Ͻ�ȡʱ������ȱ�(��ȡ����Դ����������Ϣ��C_FILE_REVEIVE_ENV)���������Ļ��ܺ˶�ģ��
//2013-08-17    ���ӻ�ȡpetri״̬,�����ݿ�ֻ��ʱ��ֻд�ļ�
//2013-09-01    ȥ������ƽ̨,����⶯��,���,������дһ��ģ��,�쳣����sql��д���ȱ��sqlд��һ���ļ�����,ÿ�����ļ�����ʱ����һ��sql
//2013-09-16    �޸Ļ��������ʱȥ�Ӹ�ʽ����ԭʼ�ļ��ķ�ʽ��ֱ�Ӹ��ݵ�ǰʱ��ȥ����7�����ڵ�ʱ��Ŀ¼(��ǰ��ͨ����ʽ�����ȱ��ʱ���ֶν�ȡĿ¼)
//2013-10-31    ���ļ�ȫ���������������ֻ�����һ��������ɾ����ʱ�ļ���������(��ʱ�ļ�ɾ��ʧ��)����ʱ����û����ʱ�ļ�����,���ж���ʱ�ļ��Ƿ�����
//2013-11-04    �������ݿ��л�ʱ���ܱ�֤�������ݿ�״̬��ȫһ��,���Զ�ͳһдsql�ļ�
//2013-12-01	������jsload�ٲ�ʧ��ʱ���ж�,������״̬-1

#include<iostream>
//#include <vector>

#include "bill_process.h"
#include "CF_CPkgBlock.h"
//#include "CF_CFscan.h"
#include "CF_CFmtChange.h"
//#include "CF_CError_Table.h"

#include "psutil.h"
#include "tp_log_code.h"
//#include "RTInfo.h"

using namespace tpss;  //��psutil.h��Ӧ
using namespace std;

const int JS_MAX_SQL_LEN			=	1024;
const int JS_MAX_FILENAME_LEN		= 	256;
const int JS_MAX_FILEPATH_LEN		=	256;
const int JS_MAX_FILEFULLPATH_LEN	=	512;
const int JS_MAX_ERRMSG_LEN			=	1024;
const int JS_MAX_RECORD_LEN			=	1024;

const char ERR_CODE[20]="ErrCode";
const char ERR_COLINDEX[20]="ErrColIndex";
const char LINE_NUM[20]="LineID";
const char FILE_ID[20]="FileID";
const char SOURCE_ID[20]="SourceID";

//����Դ��Ϣ
struct SOURCECFG
{
	char szSourceId[6];						//����ԴID
	//char szInFileFmt[6];		
	char szSourcePath[JS_MAX_FILEPATH_LEN];   //����Դ����·��
	char serverCatID[6];
	char filterRule[50] ;						//����Դ�Ĺ��˹���
	int  file_begin;							//��ȡ�ļ��������ʱ��
	int  file_length;

	SOURCECFG()
	{
		file_begin = -1;
		file_length = -1;
		memset(szSourceId,0,sizeof(szSourceId));
		//memset(szInFileFmt,0,sizeof(szInFileFmt));
		memset(szSourcePath,0,sizeof(szSourcePath));
		memset(filterRule,0,sizeof(filterRule));
		memset(serverCatID,0,sizeof(serverCatID));
	}

};

//����ˮ��Ϊ��λ
struct SParameter
{
	char szSrcGrpID[6];							//����Դ��
	char szService[6];							//serviceID
	char  iWorkflowId[10];						//������ģ��ID  
	int  iflowID;								//��ˮ��ID
	int  iModuleId;								//ģ���ʶ
	char  iInputId[10];							//����ӿ�ID
	char  iOutputId[10];						//����ӿ�ID

	char szOutputFiletypeId[6];					//����ļ�����
	char szSchCtlTabname[32+1];					//���ȱ��

	char szOutPath[JS_MAX_FILEPATH_LEN];		//���·��(���·����	
	char szSrcBakPath[JS_MAX_FILEPATH_LEN];		//ԭʼ��������·��
	char szErroPath[JS_MAX_FILEPATH_LEN];		//�����ļ�·��
		 
	int szSleepTime;							//ÿ������ʱ��

	//char szDebugFlag[50];						//�Ƿ����������־

	SParameter()
	{
		iflowID = -1;
		iModuleId = 0;		
		szSleepTime = 5;

		memset(iWorkflowId,0,sizeof(iWorkflowId));
		memset(szSrcGrpID,0,sizeof(szSrcGrpID));
		memset(szService,0,sizeof(szService));
		memset(szSchCtlTabname,0,sizeof(szSchCtlTabname));
		memset(szOutputFiletypeId,0,sizeof(szOutputFiletypeId));	
		memset(iInputId,0,sizeof(iInputId));
		memset(iOutputId,0,sizeof(iOutputId));
		memset(szOutPath,0,sizeof(szOutPath));
		memset(szErroPath,0,sizeof(szErroPath));
		memset(szSrcBakPath,0,sizeof(szSrcBakPath));	
	}
};


class Write_File : public PS_BillProcess
{
    public:

     Write_File();
     ~Write_File();    
	 
	 bool init(int argc,char** argv);                                                 
	 int getSourceFilter(char* source,char* filter,int &index,int &length);
	 int LoadSourceCfg();			 
	 int getBackTimeDir(char* dir,char* orgFileName);  //2013-09-16 ��ȡ��ʽ������ʱ��Ŀ¼

   protected:

    //�ӽ�����Ҫʵ�ֵĺ��� �̳�PS_BillProcess
	int onBeforeTask();
	int onTaskBegin(void *task_addr);
	bool onChildInit();
	int onTask(void *task_addr, int offset, int ticket_num);
	//int onTask(int event_sn, int event_type, long param1, long param2, long src_id);
	//bool onChildTask(int event_sn, int event_type, long param1, long param2, long src_id);
	void onChildExit();
	int onTaskOver(int child_ret);
	
   private:	

	 DBConnection conn;
	 map<string,SOURCECFG> m_SourceCfg;   //�������Դ��������Ϣ
	 map<string,SOURCECFG>::const_iterator it ; 
	 SParameter mConfParam;
	 CFmt_Change outrcd;
	 //CF_CFscan scan;			//�ļ�ɨ��ӿ�	  
	
	 char m_szSourceID[8];		//����ԴID
	 char mServCatId[6];
	 char m_szFileName[JS_MAX_FILENAME_LEN];    //�ļ���
	 char sql[JS_MAX_SQL_LEN];
	 char erro_msg[JS_MAX_ERRMSG_LEN];
	 char currTime[15];
	
	 int status ;				//������״̬
	 int file_status ;			//�ļ�״̬ 
	 int record_num;			//�ļ���¼��	 
	 long file_id;
	 char file_time[8+1];		//���ļ��������ȡ

};

