
//核对文件和月汇总文件备份按照ADU/SUM  核对文件下面再YYMM/DD建目录,过滤临时文件
//2013-07-26核对文件和月汇总文件核对结果放在同一张表里面，但是处理过程分开

//2013-08-07  将其放到格式化之前，对接收到的核对文件通过文件名，到接收目录下去查找，若有写文件找不到，则将其状态置N
//			  下次继续对核对失败的文件进行查找，若都找到，表示核对成功，通过文件名找到文件所属数据源(通配符)，将文件放到输出路径(格式化的输入路径)
//2013-08-27增加容灾平台,将日汇总和月汇总分开,月汇总一次启停
//2013-08-30 仅当核对文件中全部文件名找到了才发送同步信息,若核对失败则放到失败目录,不写数据库

#include<iostream>
#include <vector>

#include "bill_process.h"
#include  "dr_api.h"

#include "psutil.h"
#include "tp_log_code.h"

#include "CF_CFscan.h"

using namespace tpss;  //和psutil.h对应

using namespace std;

//数据源信息
struct SOURCECFG
{
  char szSourceId[6];		//数据源ID
  //char szFile_Fmt[6];		//数据源对应的格式
  //char szInFileFmt[6];		
  char szSourcePath[256];   //数据源所在路径
  //char szTollCode[16];      //本地网区号
  char filterRule[256] ;  //过滤规则  通过匹配文件名找数据源
  int  file_begin;		  //截取文件名上面的时间
  int  file_length;

  //char serverCatID[5];
};


//核对文件的记录格式
struct Check_Rec_Fmt
{
	string fileName;
	string  rate_flag;
	string month;
};



//月汇总文件的配置文件格式
struct  Check_Sum_Conf
{
	string check_type;
	string sum_table;
	string cdr_count ;
	string cdr_duration;
	vector<string> cdr_fee;
	string rate_cycle;
};

//月汇总文件的记录格式
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

	 int dealFile();		//处理核对文件
	 int dealMonthFile();  //处理月汇总文件

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
	 //map<string,SOURCECFG> m_SourceCfg;   //存放数据源的配置信息
	 //map<string,SOURCECFG>::const_iterator it ;

	 //char m_szSrcGrpID[8];  //数据源组
	 //char m_szService[8];	//serviceID
	 //char m_szSourceID[8];  //数据源ID
	 //char mServCatId[5];
	 //char m_szOutTypeId[8];  //文件输出格式
	
	 char receive_path[512];	 //原文件接收路径
	 char output_path[512];     //原文件输出路径

	 char input_path[512];     ////核对文件输入路径	
	 char month_input_path[512];	//月汇总文件输入路径
	
	 char bak_path1[512];	    //核对文件备份路径
	 char bak_path2[512];	    //月汇总文件备份路径
	 char fail_path[512];
	 char sql[512];
	

	 char m_szFileName[512];  //文件名
	 char fileName[1024];    //全路径文件名	 
	
	 char currTime[15];
	 char file_time[8];			//从文件名上面截取

	 char erro_msg[512];
	 CF_CFscan scan;	//文件扫描接口	

	 map<string,SOURCECFG> m_SourceCfg; 
	 map<string,string>		 mapFileSource ;//文件名,数据源ID

	 vector<Check_Rec_Fmt> fileList;
	 map< string,Check_Sum_Conf > monthSumMap ;		//存放月汇总的配置文件信息
	 vector<Check_Sum_Rec_Fmt> fileList2;

	 bool m_enable ;	//容灾状态
	 int drStatus;     //系统状态 0主系统,1备系统,2非容灾系统
	 char m_SerialString[4096];
	 char m_AuditMsg[1024];
};





