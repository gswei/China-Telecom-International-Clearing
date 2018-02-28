/************************************************************************************************
*类	名：		CCJmanager（C采J集）
*用	途： 		FTP自动连续传输（采集）文件的类
*作	者：		安	坤
*使用方法：
			1.设定上传和下载的ENV：
				SetEnvPathName( char *cDownloadEnvPathName, char *cUploadEnvPathName );
			2.设定日志文件：
				DealWithLog( char *logPathName );
			3上传：(开始根据ENV文件自动连续的上传文件，无须用户再做更多设置)
				BeginUpload( char *errMsg );
			4下载：(开始根据ENV文件自动连续的下载文件，无须用户再做更多设置)
				BeginDownload( char *errMsg );
*************************************************************************************************/
#ifndef _CJMANAGER_H
#define _CJMANAGER_H

#include "FTPmanager.h"

#include <sys/stat.h> 
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdarg.h>
#include <netdb.h>
#include <strings.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <list>
#include <vector>
#include <set>
#include <queue>
#include <algorithm>
#include <numeric>
#include <fstream.h>
#include <iostream.h>
#include <iomanip.h>
#include <stdexcept>
#include <regex.h>
#include <map>
#include <unistd.h>
#include <utility>
#include <dlfcn.h>
#include <Log.h>
#include <CEncryptFile.h>
#include <CF_CLzwCompressNoCatch.h>

#include <encrypt.h>
#include <Common.h>

//密码：wjjs: 3f841c6e   wjjs123: 3f841c6e565b92

//各种匹配方案的细节
struct MODE_INFO{
	char		cStyle[100];		//文件格式（正则表达式）
	char		cMode[5];		//文件传输模式：10或2进制
	char		cCover[5];		//文件覆盖模式
	char		cSource[200];		//传输完成后源文件的处理方式
};

//一个优先级的描述
struct GRADE_INFO{
	int		iGrade;			//优先级
	char		cLocal[200];		//本地路径
	char		cRemote[200];		//远程FTP服务器上路径
	char		cChild[5];		//上传/下载路径中是否包括子文件夹
	char		cTree[5];		//是否建立子目录树
	char		cCompress[5];		//是否解压缩
	char		cPass[100];		//解密的密匙，“*”不解密
	MODE_INFO	modeInfo[100];		//各种匹配方案的细节	
	int		iModeCount;		//MODE_INFO数组元素的个数
};

class CCJmanager{
public:
	char		m_cDownloadEnvPathName[1024];		//FTP优先级设置的ENV文件全路径+文件名
	char		m_cUploadEnvPathName[1024];		//FTP优先级设置的ENV文件全路径+文件名
	char		m_cFtpEnvPathName[1024];		//FTP优先级设置的ENV文件全路径+文件名


	CCJmanager();
	~CCJmanager();
	
	//日志的初始化设置
	int DealWithLog( char *logPathName );
	
	//根据ENV开始自动下载
	int BeginDownload( char *errMsg );
	
	//根据ENV开始自动上传
	int BeginUpload( char *errMsg );
	
	//设置FTP优先级设置的ENV文件全路径+文件名
	void SetEnvPathName( char *cDownloadEnvPathName, char *cUploadEnvPathName );
	
	//获得某一远程路径下的某一指定时间段内的文件列表
	int GetUnixListBetween( const char *path, const char *listFile, const char *beginTime, 
				const char *endTime, SERVER_INFO serverInfo, char *errMsg );
	
	//获得某一远程路径下的某一指定时间段内的文件列表
	int GetWindowsListBetween( const char *path, const char *listFile, const char *beginTime, 
				const char *endTime, SERVER_INFO serverInfo, char *errMsg );
	
	//下载某指定文件列表中的文件
	int DownloadFromList( const char *listName, const char *listPath, SERVER_INFO serverInfo, SEND_INFO sendInfo, char *errMsg );
	
	//获得某一远程路径下的符合某一指定条件（正则表达式）的文件列表
	int GetUnixListBy( const char *path, const char *listFile, const char *condition, 
				SERVER_INFO serverInfo, char *errMsg );

	//获得某一远程路径下的符合某一指定条件（正则表达式）的文件列表
	int GetWindowsListBy( const char *path, const char *listFile, const char *condition, 
				SERVER_INFO serverInfo, char *errMsg );

private:	

	//将某一指定文件列表中符合某条件（正则表达式）的文件组成新列表
	int GetWindowsListByCondition( const char *oldListFile, const char *newListFile,
						 const char *condition, char *errMsg );

	//将某一指定文件列表中指定时间段内的文件组成新列表
	int GetWindowsListByTime( const char *oldListFile, const char *newListFile, const char *beginTime, 
				const char *endTime, char *errMsg );

	//将某一指定文件列表中符合某条件（正则表达式）的文件组成新列表
	int GetUnixListByCondition( const char *oldListFile, const char *newListFile,
						 const char *condition, char *errMsg );

	//将某一指定文件列表中指定时间段内的文件组成新列表
	int GetUnixListByTime( const char *oldListFile, const char *newListFile, const char *beginTime, 
				const char *endTime, char *errMsg );

	//根据月份的英文得到月份的数字字符串
	int GetMonth( const char *oldMonth, char *newMonth );

	//得到服务器信息
	int GetServerInfo( SERVER_INFO *struGradeInfo, char *cErrMsg, int iMode );
	
	//得到某一优先级的信息
	int GetGradeInfo( int iGrade, GRADE_INFO *struGradeInfo, char *cErrMsg, int iMode );
	
	//检查GRADE_INFO的数据的完整性
	int CheckGradeInfo( GRADE_INFO serverInfo, char *cErrMsg );
	
	//检查SERVER_INFO的数据的完整性
	int CheckServerInfo( SERVER_INFO struGradeInfo, char *cErrMsg );

	//下载优先级为grade的文件
	int DownloadByGrade( int grade, char *errMsg );
	
	//上传优先级为grade的文件
	int UploadByGrade( int grade, char *errMsg );
	
	//判断文件是否存在
	int IsFileExist(char *p_cPathName);
	
	//下载某文件夹下的所有文件
	int DownloadFrom( char *downloadPath, SERVER_INFO serverInfo, GRADE_INFO struDataInfo, char *errMsg );
	
	//判断文件名是否符合正则表达式条件
	int IsFileMatch( char *cOldFileName, char *cCondition );
	
	//上传某文件夹下的所有文件
	int UploadFrom( char *uploadPath, SERVER_INFO serverInfo, GRADE_INFO struDataInfo, char *errMsg );
	
	//判断是文件还是文件夹
	int IsItDir(char *cPathName);
	
	//从ENV文件中分解STYLE,MODE,COVER,SOURCE的各项
	int GetDetail( const char *cLine, GRADE_INFO *gradeInfo, int iFlag );
	
	//去掉字符串两边的空格,Tab,回车
	void Trim(char *cStr);
	
	//得到文件大小
	int GetFileSize( const char *filePathName );
	
};

#endif
