//2013-07-18	����־�����÷ŵ����Ĳ������棬������־д������
//2013-07-29	�����ļ���غ���⹦��,�޸�CF_CError_Table�ӿ�setFileName()����Statementʹ���ܻع�ǰ�������
//2013-08-01    ������ʱĿ¼WR_FILE_TMP_DIR  ���ʧ�ܺ������ļ��Ĵ��Ŀ¼
//2013-08-13	���Ӵ��ļ����Ͻ�ȡʱ������ȱ�(��ȡ����Դ����������Ϣ��C_FILE_REVEIVE_ENV)���������Ļ��ܺ˶�ģ��
//2013-08-17    ���ӻ�ȡpetri״̬,�����ݿ�ֻ��ʱ��ֻд�ļ�

#include<iostream>
#include <vector>

#include "bill_process.h"
#include "CF_CPkgBlock.h"
#include "CF_CFscan.h"
#include "CF_CFmtChange.h"
#include "CF_CError_Table.h"

#include "psutil.h"
#include "tp_log_code.h"
#include "RTInfo.h"
#include "dr_api.h"

using namespace tpss;  //��psutil.h��Ӧ

using namespace std;

const char ERR_CODE[20]="ERR_CODE";
const char ERR_COLINDEX[20]="ERR_COLINDEX";
const char LINE_NUM[20]="LINE_NUM";
const char FILE_ID[20]="FileId";
const char SOURCE_ID[20]="SourceID";

//����Դ��Ϣ
struct SOURCECFG
{
  char szSourceId[6];		//����ԴID
  //char szFile_Fmt[6];		//����Դ��Ӧ�ĸ�ʽ
  char szInFileFmt[6];		
  char szSourcePath[256];   //����Դ����·��
  char szTollCode[16];      //����������
  char serverCatID[5];

  char filterRule[256] ;	//����Դ�Ĺ��˹���
  int  file_begin;		  //��ȡ�ļ��������ʱ��
  int  file_length;

};


class Write_File : public PS_BillProcess
{
    public:
     Write_File();
     ~Write_File();    
	 
	 bool init(int argc,char** argv);
	 int  writeFile(char* fileName,PkgBlock) ;                    //��ȡ�ڴ�д���ļ�                                                  

	 int getSourceFilter(char* source,char* filter,int &index,int &length);
	 int LoadSourceCfg();
	
	 //int dealFile();  //�����ļ�����������ϵͳ�ϲ�����
	 int deleteFile(char* path,char* filter);
	 int commitErrMsg();	//�ύ�����¼��Ϣ

	 int updateDB(char* sql);
	 int scanSQLFile();

	 int  indb(char* file,char* name);  //2013-07-29	�ļ����

	 bool drInit();
	 bool CheckTriggerFile();
	 int  drVarGetSet(char* m_SerialString);
	 bool IsAuditSuccess(const char* dealresult);

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
	 
	 map<string,int> mapConfig;   //������ı��־
	 map< string,CF_CError_Table> mapTabConf	;

	 char m_szSrcGrpID[8];  //����Դ��
	 char m_szService[8];	//serviceID
	 char m_szSourceID[8];  //����ԴID

	 char mServCatId[5];
	 char m_szOutTypeId[8];  //�ļ������ʽ
	 
	 char out_path[245];	   //������·��
	 char erro_path[256];  //�����ļ�·��
	 char bak_path[256];
	 char other_path[256];     //�ļ����ʧ�ܵ��ļ�·��

	 char m_szFileName[256];  //ԭʼ�ļ���
	 char outFileName[256];		  //����ļ���
	 CF_CFscan scan;	//�ļ�ɨ��ӿ�	 
	 
	 CFmt_Change outrcd;

	 char currTime[15];

	 char sql[1024];

	 int status ;//������״̬
	 int file_status ; //�ļ�״̬ 

	 char erro_sql[1024]; //����sql�ļ��Ǽ���
	 char erro_msg[1024];
		
	  int record_num;  //�ļ���¼��

	  char file_time[8];			//���ļ��������ȡ
	  short petri_status;	   //��ȡpetri��״̬

	  bool m_enable ;	//����״̬
	  int drStatus;     //ϵͳ״̬ 0��ϵͳ,1��ϵͳ,2������ϵͳ
	  //string m_triggerFile;	�����Ѿ�ʵ��
	  char m_SerialString[4096];
	  char m_AuditMsg[1024];
};

