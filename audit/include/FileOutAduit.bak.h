
//2013-07-23 ,���Ƿ�������ļ������·�����óɿ����õģ�c_source_audit_env

#include<iostream>
#include <vector>

#include "bill_process.h"

#include "psutil.h"
#include "tp_log_code.h"

using namespace tpss;  //��psutil.h��Ӧ

using namespace std;

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

	 //int getSourceFilter(char* source,char* filter);
	 //int LoadSourceCfg();
	
	 //int deleteFile(char* path,char* filter);
	 //int commitErrMsg();	//�ύ�����¼��Ϣ
	
	 int writeFile();

	 int updateDB(char* sql);
	 int scanSQLFile();

	 void run();
	
   private:
	
	 DBConnection conn;
	 //map<string,SOURCECFG> m_SourceCfg;   //�������Դ��������Ϣ
	 //map<string,SOURCECFG>::const_iterator it ;

	 //char m_szSrcGrpID[8];  //����Դ��
	 //char m_szService[8];	//serviceID
	 //char m_szSourceID[8];  //����ԴID
	 //char mServCatId[5];
	 //char m_szOutTypeId[8];  //�ļ������ʽ

	 //char input_path[256];     //�������·��
	 //char out_path[1024];	   //������·��
	  //char erro_path[256];  //�����ļ�·��
	 char sql[1024];
	 char m_szFileName[256];  //�ļ���
	 //char outFileName[256];		  //����ļ���
	 //CF_CFscan scan;	//�ļ�ɨ��ӿ�	 
	
	 char currTime[15];

	 //int record_num;  //�ļ���¼��
	 char sqlFile[1024];
	
	 //int status ;//������״̬
	 //int file_status ; //�ļ�״̬ 

	 //char erro_sql[1024]; //����sql�ļ��Ǽ���
	 char erro_msg[1024];
	 
	 vector<string> fileList;
	 map< string,vector<string> > sourceMap ;
	 map< string,AduitEnv > fileNameMap ;
};





