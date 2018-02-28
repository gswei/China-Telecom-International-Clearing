//2013-07-20 ��פ���̣�ɨ������ļ���Ϣ�����������Ϣд�쳣˵���ļ������Ѵ����Դ�ļ����ظ�����ϵͳ���ٵǼ��쳣˵���ļ�
//2013-07-22 ��������Ϣд����д������ļ��Ǽ�Ŀ¼
//2013-08-17    ���ӻ�ȡpetri״̬,�����ݿ�ֻ��ʱ��ֻд�ļ�
//2013-08-27  ��������ƽ̨
//2013-08-30 ɾ������ƽ̨,sql��д�ļ�ͨ����ȡ���ݿ�״̬(����sql����Ӱ����һ��ѭ��ִ��)

#include<iostream>
#include <vector>
#include <errno.h>

#include "bill_process.h"
#include "CF_CFmtChange.h"

#include "psutil.h"
#include "tp_log_code.h"
#include "RTInfo.h"

//#include "dr_api.h"

using namespace tpss;  //��psutil.h��Ӧ

using namespace std;

//����Դ��Ϣ
struct SOURCECFG
{
  char szSourceId[6];		//����ԴID
  //char szFile_Fmt[6];		//����Դ��Ӧ�ĸ�ʽ
  //char szInFileFmt[6];		
  char szSourcePath[256];   //����Դ����·��
  //char szTollCode[16];      //����������
  //char filterRule[256] ;	//����Դ�Ĺ��˹���

  char serverCatID[5];
};


//�쳣˵���ļ���Ϣ
struct ERRINFO
{
	string filename;
	char source_id[6];
	//char dealtime[15];
	char err_msg[2];
	char err_code[10];
	int  err_col;
	int	 err_line;
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
	 //int scanSQLFile();
	 //int writeSQL(char* sql);

	 void run();
	
   private:
	
	 DBConnection conn;
	 map<string,SOURCECFG> m_SourceCfg;   //�������Դ��������Ϣ
	 map<string,SOURCECFG>::const_iterator it ;

	 //char m_szSrcGrpID[8];  //����Դ��
	 //char m_szService[8];	//serviceID
	 char m_szSourceID[8];  //����ԴID
	 //char mServCatId[5];
	 //char m_szOutTypeId[8];  //�ļ������ʽ
	
	 char m_szFileName[256];  //ԭʼ�ļ���
	 char input_path[256];     //�������·��
	 char out_path[512];	   //�������·��
 
	 char outFileName[1024];  //���ȫ·���ļ��� 
	 char currTime[15];
	 short petri_status;	   //��ȡpetri��״̬ 
	
	 char sql[512];
	 char erro_msg[512];

	 map< string,vector<ERRINFO> > erroinfoMap ;
	 vector<string>  vsql;

};





