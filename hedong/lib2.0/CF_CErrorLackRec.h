#ifndef _CERRORLACKREC_
#define _CERRORLACKREC_

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

#include "CF_Lack.h"
#include "CFmt_Change.h"
#include "CF_CError.h"
#include "CF_CFscan.h"
#include "COracleDB.h"
#include "CF_Error_Table.h"
#include "Common.h"


#define PATHLEN          256 //文件路径长度
#define MSGLEN	 	 	 300 //话单记录的长度
#define FILELEN	         256 //文件名的长度
#define LACKLEN		 	 500 //出错信息长度
#define COLLEN		 	 500 //每条数据源记录长度
#define CURLEN			 2048//存储游标字符串的长度
#define CURCOUNT		 20  //插入游标的最大插入记录数
#define TABLELEN		 50	 //表名的长度
//#define MAX_COLUMN	     30	 //错单字段数	
#define MAX_VARLEN 		 100


//#define ERR_FILE_OPEN	 -9  //文件创建或打开时为空
#define ERR_DIR_OPEN	 -10 //目录打开时为空
//#define ERR_FILE_WRITE   -2  //写文件错误
//#define ERR_FILE_CLOSE   -3  //关闭文件出错
//#define ERR_RENAME_FILE	 -4  //
//#define ERR_REMOVE_FILE  -8  //
#define ERR_NOT_COMMIT	 -5  //没有提交
//#define ERR_GET_RECORD   -6  //读出数据源记录（针对CFmt_Change）错误
#define ERR_UNKOWN	     -7  //未知错误
#define DO_CLOSE_FIRST	 1   //成功(自动保存后提交)
#define SUCCESS		     0   //成功
#define FAIL		     -1  //失败
#define SEARCH_EOF	     100 //查找不到符合条件的内容
#define SUC_DO_COMMIT    20  //成功,但必须提交数据了

enum abnormity_type{
		abenum_lackinfo,    /*无资料话单*/
		abenum_error,       /*错单*/
		abenum_nonStand,    /*cp结算普通不规则文件*///rewrite by wulei 2005-02-22	
		abenum_FixDup      /*cp结算包月重单文件*///rewrite by wulei  2005-02-22
	};
	
struct ErrorLackRec_Link
{
	char filename[FILELEN];
	int  error_type;
	char source_id[6];
	CFmt_Change change;
	ErrorLackRec_Link* next;
};
struct SErrLackParam
{
  char pipe_id[6];
  char Stat_Table_Config[100];
  int Is_Save_Table;
  int Save_Table_Config;
  int Is_Stat_Table;
};


class CF_CErrorLackRec
{
private:
	FILE* fp;                       //保存无资料话单的文件指针
	int Count;			            //保存当前文件数据记录数
	char path_name[PATHLEN];        //配置文件里无资料话单的根目录+异常类型目录+处理过程代码目录
	char file_name[FILELEN];	    //保存上次的文件名
	char temp_file[FILELEN];	    //保存临时文件名
	int proc_id;			        //处理过程代码
	abnormity_type type;		    //标明操作的异常话单类型
	ErrorLackRec_Link* header;		//话单数据记录首记录
	ErrorLackRec_Link* current;		//话单数据记录尾记录
	int LinkCount;					//话单数据记录的数量
	char table[TABLELEN];			//记录话单保持到的表
	char szInCtlTabname[TABLELEN];
	char szSourceId[200][6];
	int iSourceCount;
    SErrLackParam sELParam;
    CF_CError_Table OSave_Table;
    CF_DUP_CStat OStat_Table;
    int Is_Commit;
private:
	int process_header();
	int process_column(int id,char* filename,CFmt_Change* change);
	int process_end();
	int Open();
	
public:
	CF_CErrorLackRec();
	~CF_CErrorLackRec();
	int init_Table(char *Pipe_id,int process_id);
	int init(char* rootpath,abnormity_type type_id,int process_id);
	int saveErrLackRec(int id,char* filename,CFmt_Change* change,char* source_id);
	int Close();
	int Commit();
	int RollBack();
	int Proc_TMPFile(char* filename);
	
	int initToDB(abnormity_type type_id, int process_id, char* tablename);
	int saveErrLackRecToDB(char* filename, int error_type, char* source_id, CFmt_Change* change);
	int CommitToDB();
	int RollBackToDB();
};

#endif
