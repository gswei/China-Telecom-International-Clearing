
//�˶��ļ����»����ļ����ݰ���ADU/SUM  �˶��ļ�������YYMM/DD��Ŀ¼,������ʱ�ļ�
//2013-07-26�˶��ļ����»����ļ��˶Խ������ͬһ�ű����棬���Ǵ�����̷ֿ�

//2013-08-07  ����ŵ���ʽ��֮ǰ���Խ��յ��ĺ˶��ļ�ͨ���ļ�����������Ŀ¼��ȥ���ң�����д�ļ��Ҳ���������״̬��N
//			  �´μ����Ժ˶�ʧ�ܵ��ļ����в��ң������ҵ�����ʾ�˶Գɹ���ͨ���ļ����ҵ��ļ���������Դ(ͨ���)�����ļ��ŵ����·��(��ʽ��������·��)
//2013-08-27��������ƽ̨,���ջ��ܺ��»��ֿܷ�,�»���һ����ͣ
//2013-08-30 �����˶��ļ���ȫ���ļ����ҵ��˲ŷ���ͬ����Ϣ,���˶�ʧ����ŵ�ʧ��Ŀ¼,��д���ݿ�

#include<iostream>
#include <vector>

#include "bill_process.h"
#include  "dr_api.h"

#include "psutil.h"
#include "tp_log_code.h"

#include "CF_CFscan.h"

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
  char filterRule[256] ;  //���˹���  ͨ��ƥ���ļ���������Դ
  int  file_begin;		  //��ȡ�ļ��������ʱ��
  int  file_length;

  //char serverCatID[5];
};


//�˶��ļ��ļ�¼��ʽ
struct Check_Rec_Fmt
{
	string fileName;
	string  rate_flag;
	string month;
};



//�»����ļ��������ļ���ʽ
struct  Check_Sum_Conf
{
	string check_type;
	string sum_table;
	string cdr_count ;
	string cdr_duration;
	vector<string> cdr_fee;
	string rate_cycle;
};

//�»����ļ��ļ�¼��ʽ
struct  Check_Sum_Rec_Fmt
{
	string file_type;
	long cdr_count ;
	long cdr_duration;
	double cdr_fee;
};

class FileInAduit : public PS_Process
{
    public:
     FileInAduit();
     ~FileInAduit();    
	 
	 bool init(int argc,char** argv);                                               
	 int  LoadSourceCfg();
	 int getSourceFilter(char* source,char* filter,int &index,int &length);
	
	 int writeFile();

	 int dealFile();		//����˶��ļ�
	 int dealMonthFile();  //�����»����ļ�

	 void run(int flag = 1);
	 int checkFile();

	 int check_before_file();
	 int check_file_exist(char* file);
	 bool checkFormat(const char *cmpString, const char *format);

	 bool drInit();
	 bool CheckTriggerFile();
	 int  drVarGetSet(char* m_SerialString);
	 bool IsAuditSuccess(const char* dealresult);

   private:
	
	 DBConnection conn;
	 //map<string,SOURCECFG> m_SourceCfg;   //�������Դ��������Ϣ
	 //map<string,SOURCECFG>::const_iterator it ;

	 //char m_szSrcGrpID[8];  //����Դ��
	 //char m_szService[8];	//serviceID
	 //char m_szSourceID[8];  //����ԴID
	 //char mServCatId[5];
	 //char m_szOutTypeId[8];  //�ļ������ʽ
	
	 char receive_path[512];	 //ԭ�ļ�����·��
	 char output_path[512];     //ԭ�ļ����·��

	 char input_path[512];     ////�˶��ļ�����·��	
	 char month_input_path[512];	//�»����ļ�����·��
	
	 char bak_path1[512];	    //�˶��ļ�����·��
	 char bak_path2[512];	    //�»����ļ�����·��
	 char fail_path[512];
	 char sql[512];
	

	 char m_szFileName[512];  //�ļ���
	 char fileName[1024];    //ȫ·���ļ���	 
	
	 char currTime[15];
	 char file_time[8];			//���ļ��������ȡ

	 char erro_msg[512];
	 CF_CFscan scan;	//�ļ�ɨ��ӿ�	

	 map<string,SOURCECFG> m_SourceCfg; 
	 map<string,string>		 mapFileSource ;//�ļ���,����ԴID

	 vector<Check_Rec_Fmt> fileList;
	 map< string,Check_Sum_Conf > monthSumMap ;		//����»��ܵ������ļ���Ϣ
	 vector<Check_Sum_Rec_Fmt> fileList2;

	 bool m_enable ;	//����״̬
	 int drStatus;     //ϵͳ״̬ 0��ϵͳ,1��ϵͳ,2������ϵͳ
	 char m_SerialString[4096];
	 char m_AuditMsg[1024];
};





