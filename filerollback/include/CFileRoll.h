/*************************************************************************
Copyright (c) 2011-2012, GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:		 2013-08-23
File:			 CFileRoll.h
Description:	 文件回退模块
1、	初始化，获取配置表中的文件目录和备份目录；
2、	从文件目录中获取文件
3、	读取文件内容，将接收文件名全部读取到内存中
4、	根据文件名查询对应数据源ID，fileid和对应结果表名
5、	根据查询内容，从结果表中将fileid对应的结果删除。


接口说明  输入命令：jsrollfile 
**************************************************************************/

#ifndef CFILEROLL_H
#define CFILEROLL_H


#include "bill_process.h"
#include "psutil.h"
#include "tp_log_code.h"

#include <dirent.h>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>  //stat()函数，查询文件信息
#include <unistd.h>     //读取当前程序运行目录
#include <iostream>
#include <fstream>
#include "dr_api.h"
#include "RTInfo.h"   //petri网状态

using namespace tpss;  //和psutil.h对应
#include "CF_Common.h"
#include "CF_CLogger.h"
#include "CF_CFscan.h"


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

struct SRollFile{
	char filename[50];//文件名
//	string filename;
	char sourceID[10];//文件名对应的数据源ID
	int  fileID;//文件对应的文件ID
	char tablename[20];//数据源对应的日汇总结果表名
};
class CFileRoll :public  PS_Process
{
private:
	vector<SRollFile>  szrollfiles;
    char  m_filename[256];   //全路径文件名
	char  input_path[256];  //文件查找路径
	char  output_path[256]; //文件输出路径
	char  erro_path[256];   //文件失败路径
	char  erro_msg[1024];   //错误信息
	CF_CFscan scan;	//文件扫描接口
    DBConnection conn;//数据库连接
	map<string,SOURCECFG> m_SourceCfg; 
	
	short petri_status ;

	bool m_enable ;	//容灾状态
	int  drStatus;  //系统状态 0主系统,1备系统,2非容灾系统
	char m_SerialString[4096];
	char m_AuditMsg[4096];
    

public:
	CFileRoll();
	~CFileRoll();
	bool init(int argc,char** argv);
	//bool run();
//	bool getFile();//获取目录下的文件名
	bool getFilenames();//从上一个函数获取的文件中读取文件名到内存SRollFile 中；
	bool rollFile(int i);//根据szrollfiles的文件名，从文件接收配置表中查询文件对应的数据源ID，从格式化文件登记表D_SCH_FORMAT中查询fileid，根据数据源ID查询汇总配置表获取对应日汇总结果表名。最后从结果表中删除fileid对应的记录
	void saveLog(char state,int i); 
	bool moveFiles(bool flag);//将已经处理后的文件移动到指定备份目录
	int  LoadSourceCfg();
	int  getSourceFilter(char* source,char* filter,int &index,int &length);
	bool checkFormat(const char *cmpString,const char *format);
	void prcExit();

	//容灾平台
    bool drInit();
	bool CheckTriggerFile();
	int  drVarGetSet(char* m_SerialString);
	bool IsAuditSuccess(const char* dealresult);
	//bool rollFile(char *filename);
};

#endif

