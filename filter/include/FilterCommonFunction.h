
#ifndef _FILTER_COMMON_FUNCTION_H_
#define _FILTER_COMMON_FUNCTION_H_


#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <iostream.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fstream.h>
#include <string.h>

#include "CF_CLogger.h"
#include "CF_CException.h" 
#include "CF_Common.h"

#include "C_CFTime.h"

typedef long SIZE_TYPE;

const int  FILTER_ERR_CREATE_MEMORY = 7001;  //创建共享内存内存失败
const int  FILTER_ERR_CONNECT_MEMORY = 7002;  //连接共享内存失败内存失败

const int  FILTER_ERR_IN_CREATE_DIR         = 7010;	//创建路径
const int  FILTER_ERR_IN_CREATE_FILE  = 7011;	//创建文件
const int  FILTER_ERR_IN_OPEN_FILE = 7012;	//打开文件错误
const int  FILTER_ERR_IN_WRITE_FILE = 7013;	//写文件出错
const int FILTER_ERR_IN_READ_FILE = 7014;	//读取文件记录出错

const int  FILTER_ERR_IN_CONNECT_DB         = 7020;  //连接数据库出错
const int  FILTER_ERR_NO_RECORD_INDB     = 7021;	//查不到相关记录

const int  FILTER_ERR_UNKNOWN_CATCH         = 7099;  //捕捉到无法识别的错误类

const int  FILTER_ERR_IN_TIME_FIELD_FORMAT  = 7030;  //时间格式不对
const int	FILTER_ERR_OUT_OF_RANGE = 7031;	//超出范围




const int FILTER_ERRMSG_LEN = 500;	//错误消息长度


const int FILTER_SOURCE_INDEX = 1;	//创建共享内存时数据源结构所用的index值
const int FILTER_FILE_INDEX = 2;	//创建共享内存时文件结构所用的index值
const int FILTER_BLOCK_INDEX = 3;	//创建共享内存时数据块结构所用的index值

const int FILTER_VALUESIZE = 17;			//一个key值最大的长度
const int FILTER_MAXINDEXNO = 3000;		//5000一个块中存储的最大索引数
const int FILTER_FILESIZE = 256;			//文件长度

const int FILTER_SOURCE_PROCESS = 4;	//每个数据源允许访问的进程数 (没用)
const int FILTER_PROCESS_INFO_LEN =  20;	//进程信息长度(没用)

const int FILTER_INDEX_FILE_LEN = 10;	//索引文件名最大长度

const int FILTER_BLOCKNUM_PER_FILE = 5;
const int FILTER_MAXLOADBLOCK = 20;	//20一个文件保留在内存中数据块的最大数
const int FILTER_MAXLOADFILE = 24;	//8一个数据源支持最大的文件列表数(共享内存中)
const int FILTER_MAXSOURCE = 50;		//支持的最大数据源数

const int FILTER_MAXSECONDINFILE = 3600;	//文件中最大的索引列表数


const char TMP_DATAHEAD_NAME[]="dataHead.tmp"; 	//数据块头临时文件
const char TMP_DATA_NAME[]="data.tmp";			//数据临时文件
const char TMP_FILEINFO_NAME[]="index.tmp";		//文件头信息临时文件

const char COMMIT_FILE[]="commit.flag";	//标志位文件名
const char BACKUP_DATA_NAME[]="data.backup";		//内存中数据块备份
const char BACKUP_FILEINFO_NAME[]="index.backup";	//内存中文件头信息备份

const char FILTER_WORK_PATH[]="WORK";	//工作子目录




//int chkAllDir(char *path);
//int completeDir(char *pchPath);
//int chkDir(const char *_dir );

int openFile(fstream &file_Stream, char *fileName);
FILE * openfile(char *fileName, const char* mode);
void truncFile(const char* fileName);

//bool findInTemp(const char *file_Name, const long block_No, long &location );

#endif

