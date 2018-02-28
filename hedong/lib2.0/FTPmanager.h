/***********************************************************************************************
类	名：		CFTPmanager
用	途：		对FTP服务器上的文件进行：上传，下载，得到文件列表，操作远程文件和文件夹。
作	者：		安	坤
version:    1.0.3.1
update history：
by yangzx 2006.05.16 修改List接口支持获取远程dir下的所有文件列表包括子目录下的文件，增加private method ScanFile()
by yangzx 2006.05.20 增加接口SizeFile()取文件大小，	为保证与旧程序的兼容性，增加ListAllFile接口，以支持获取远程dir下
										的所有文件列表包括子目录下的文件，
										int ListAllFile( const char *path, const char *listSavePathName, char *errMsg );
										原有的List接口功能不变，只取当前目录下的文件，不取子目录下的文件。
by yangzx 2006.05.21 使用select函数解决block问题，在download() 和upload()里使用
by yangzx 2006.06.01 因为在hp的机器上使用select函数有问题，暂时注释掉使用select函数的部分，恢复以前的upload和download
										 函数
by yangzx 2006.06.09 12:00 修改ListBy的bug，注释掉所有使用的select函数的部分
by yangzx 2006.06.12 使用 poll() 函数代替 select()函数，select()函数在hp的机器上被程序调用时不稳定
by yangzx 2006.07.06 修改ListAllFile()和ScanFile()函数，修改输出错误信息,修改DivFileName( listSavePathName,filePath, fileName )函数在拷贝路径到filePath的bug，filePath有时会没有字符串终止符 0
by yangzx 2006.07.31 修改ftpcmd()在read()和write()前，使用IsReadyToRead()和IsReadyToWrite()封装
by yangzx 2006.08.05 在函数CFTPmanager()里增加signal(SIGPIPE,SIG_IGN)
by yangzx 2006.08.10 使用进行非阻塞封装的Accept()代替直接调用accept()
by yangzx 2006.08.30 解决Connect()里，如果发送ftp用户名、密码阻塞时，没有清空m_ftpio指针的bug,在shutdown()函数之后还要调用close()，shutdown()函数只是关闭连接，升级版本号为1.0.2
by yangzx 2006.09.20 在调用select() 或 poll()时如果系统产生real time signal 会使函数报错返回，错误代码errno=EINTR;修改代码在调用select()和poll()时，忽略EINTR错误 
by yangzx 2006.10.11 debug DivFileName()，当文件在根路径时，如/test.dat，该函数无法正确分解出文件的路径和文件名，版本号升为1.0.3.1，新程序已在市话东莞132.119.200.60上使用,1.0.5只在计费部数据下发接口机上使用过，有待全面测试。
by zhanggq 2007.4.19 修改超时时间，由60S改为300S
*************************************************************************************************/
#ifndef _FTPMANAGER_H
#define _FTPMANAGER_H 1

#include <regex.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h> 
#include <string.h>
#include <strings.h>
#include <poll.h>
#include <signal.h>

#include "CEncryptFile.h"
#include "CF_CLzwCompressNoCatch.h"

/* begin of 错误信息的定义****************************************************************/

//网络部分:
#define GET_SERVER_NAME_ERR			     -1	//从FTP服务器名提取FTP服务器IP失败
#define CREATE_SOCK_ERR				       -2	//建立与FTP服务器通讯的SOCKET失败
#define SOCK_CONNECT_ERR			       -3	//通讯的SOCKET与服务器建立连接失败
#define RECIEVE_SERVER_MSG_ERR			 -4	//接受服务器返回信息失败
#define LOGIN_ERR				             -5	//登录FTP服务器失败（用户名或密码错误）
#define CREATE_LISTENING_SOCK_ERR		 -6	//接收服务器数据的SOCKET建立失败
#define LISTENING_SOCK_BIND_ERR			 -7	//接受服务器数据的SOCKET端口绑定失败
#define SOCK_LISTENING_ERR 			     -8	//接受服务器数据的SOCKET监听失败
#define SEND_CMD_ERR				         -9	//发送数据到服务器失败
#define SOCKET_BLOCK_READ		         -10//发生read block
#define SOCKET_BLOCK_WRITE           -11//发生write block
#define SOCKET_SELECT_ERROR          -12//select()函数出错
#define SOCKET_POLL_ERROR            -13//poll()error

//文件部分：
#define TARGET_FILE_CREATE_OPEN_ERR	 -20	//目标文件创建/打开失败
#define DOWNLOAD_SOURCE_NOT_EXIST		 -21	//FTP服务器上要下载的源文件不存在
#define DELETE_SOURCE_ERR			       -22	//传输完后删除源文件失败
#define STORE_SOURCE_ERR			       -23	//传输完后备份源文件失败
#define OPEN_SOURCE_FILE_ERR			   -24	//本地源文件打开失败
#define UPLOAD_TARGET_PATH_ERR			 -25	//FTP服务器上上传文件的保存路径不存在
#define SOURCE_PATH_NOT_EXIST			   -26	//源文件路径不存在
#define SOURCE_FILE_NOT_EXIST			   -27	//源文件不存在
#define TARGET_PATH_NOT_EXIST			   -28	//目标文件夹不存在
#define BOTH_NULL_ERR				         -29	//目标文件和目标文件夹两个参数不能同时为空
#define FOLDER_PATH_NOT_EXIST			   -30	//文件夹路径不存在
#define FOLDER_NOT_EXIST			       -31	//文件夹不存在
#define FOLDER_EXIST				         -34	//文件夹已经存在
#define RENAME_FILE_ERR				       -35	//文件重命名失败
#define FILE_SIZE_ERR				         -36	//文件大小不一致
#define MOVE_FILE_ERR                -37  //移动文件出错
#define DELETE_FILE_ERR              -38  //删除文件出错
#define LOCK_FILE_ERR                -39  //锁文件出错
#define UNLOCK_FILE_ERR              -40  //解文件出错

//业务逻辑部分:
#define UNRECOGNIZED_SENDING_MODE		-50	//不能识别传输模式
#define UNRECOGNIZED_COVER_MODE			-51	//覆盖模式参数错误（不能识别）

/**************************************************************** end of 错误信息的定义*/

/* begin of 函数使用参数的定义 *********************************************************/
#define MODE_BIN			2	//文件传输模式（2进制）
#define MODE_ASC			10	//文件传输模式（文本）
#define TARGET_COVER	1	//文件上传时目标文件存在时的处理方式（覆盖）
#define TARGET_STORE	0	//文件上传时目标文件存在时的处理方式（背份）
#define SOURCE_DELE		"*"	//文件传输完成后源文件的处理方式（删除）
#define SOURCE_IGNORE	"**"	//文件传输完成后源文件的处理方式(不处理)
#define COMPRESS_Y		"Y"	//文件传输完成后要解压
#define COMPRESS_N		"N"	//文件传输完成后不解压
#define PASS_N				"*"	//文件传输完成后不解密

/*********************************************************** end of 函数使用参数的定义 */

/*begin of const varible define *********************************************************/
int const BLOCK_TIME_OUT_SEC=300;    //select 函数中的socket 等待时间，用于设置 struct timeval.tv_sec
int const BLOCK_TIME_OUT_uSEC=0;		 //select 函数中的socket 等待时间，用于设置 struct timeval.tv_sec
int const BLOCK_TIME_OUT_MSEC=300000;//poll() 函数中的socket 等待时间 
/*********************************************************** end of const varible define */

//服务器信息
struct SERVER_INFO
{
	char cIp[20];		      //远程FTP服务器主机的IP
	char cUser[100];		  //FTP服务器的登录用户名
	char cPassword[100];  //FTP服务器的登录密码
};

//传输信息
struct SEND_INFO
{
	char localFile[1024];	//本地文件
	char remoteFile[1024];//远程文件	
	int	 coverFlag;		    //文件传输覆盖标志
	int	 sendMode;		    //文件传输模式
	char cStorePath[200]; //传输完成后源文件处理方式
};

//文件信息

int _getsockname(int,struct sockaddr *,int *);

int _accept(int,struct sockaddr *,int *);

class CFTPmanager
{
  private:	
	  char sz_serverIp[20];		     //FTP服务器IP
	  char sz_loginName[100];	     //FTP服务器登录名
	  char sz_loginPassword[100];	 //FTP服务器登录密码
	  int  iServerPort;            //FTP服务器端口号
	  FILE *m_ftpio;               //与FTPserver通讯的文件指针
	  int	isockftp;		             //与FTPserver通讯的套接字
	  int temp_num;                //临时文件的序号
	  int iFilesizeFlag;          //获取文件大小的可用ftp命令标识，0,尚未判断有什么命令可用，1,STAT,SIZE 都不可用，2,STAT可用;3,SIZE,可用
	  char c_PassiveFlag;
	  char sz_errorMessage[300];

	  FILE *filelist;	

  public:

	  CFTPmanager();
	  ~CFTPmanager();

	  int ShowVer();

	  int Connect(const char *,const char *,const char *,char *,char,int port=21);
    int Connect(const char *,const char *,const char *,char *,int port=21);
    //int Connect(const char *,const char *,const char *,char *,int,char);
	  int Connect(SERVER_INFO,char *);
	  int Disconnect();
    
    int Download(const char *,const char *,int,char *,int ,const char *);
    int Download(const char *,const char *,int,char *,int ,const char *,int&); 
	  int Download(SEND_INFO,char *);
	  int Download(const char *,const char *,int,char *,int ,const char *,const char *,const char *);	//从FTP服务器上下载文件(包括解密/压)

	  int Upload(const char *,const char *,int,char *,int ,const char *);
	  int Upload(SEND_INFO,char *);

	  int List(const char *,const char *,char *);
	  int ListAllFile(char *,const char *,char *);//得到FTP服务器上指定目录下的文件列表,包括子目录下的文件列表，文件名带全路径信息
	  int ListUnixBy(const char *,const char *,const char *,char *);//得到Unix系统(远程机)某路经下符合某正则表达式的文件的列表
    int ListWindowsBy(const char *,const char *,const char *,char *);//得到Unix系统(远程机)某路经下符合某正则表达式的文件的列表
		int ListUnixBetween(const char *,const char *,const char *,const char *,char *);//获得某一远程路径下的某一指定时间段内的文件列表
	  int ListWindowsBetween(const char *,const char *,const char *,const char *,char *);//获得某一远程路径下的某一指定时间段内的文件列表
		int ListBy(const char *,const char *,const char *,char *);//获取远程机器上某路经下符合某正则表达式的文件的列表

	  int ChDir(const char *,char *);//切换到某一路径
	  int MkDir(const char *,char * );//创建新路径
	  int RmDir(const char *,char *);//删除远程文件
	  int DirFileExist(const char *);//检测远程文件是否存在
	  int RmFile(const char *,char *);//删除远程文件
	  int MoveFile(const char *,const char *, char *);//移动远程文件
	  
	  int DivFileName(const char *, char *, char *);//将文件全路径分解问路径和文件名
	  int SizeFile(char * ,int & ,char *);//获取指定远程文件的大小
	  int IsFileExist(const char *);
	  int IsUnixZhengZe(const char *buf,const char *condition );//判断该记录中文件名是否符合要求(正则表达式)
	  int IsWindowsZhengZe(const char *,const char * );//判断该记录中文件名是否符合要求(正则表达式)
	  int IsMatchZhengZe(char *,char *);//判断某字符串是否符合某正则表达式
	  int IsUnixBetween(const char *,const char *,const char *,char *);//判断该记录中文件时间是否在某指定时间范围内
	  int IsWindowsBetween(const char *,const char *,const char *,char *);//判断该记录中文件时间是否在某指定时间范围内
	  int GetDate(char *);//取得XXXX-XX-XX形式的当前日期
	  int GetMonthByChr(const char *,char *); //由字母月份取得数字月份
	  int Reconnect(char *);//自动重连接服务器
	  int GetFileSize(const char * );//得到文件大小
    void CheckPath(char *);//检查path后面是否带'/',如果带则删除 
    int ChFile(const char *,char *);
	private:
		int ScanFile(char * ,char* ,char*);//获取远程目录下的所有文件信息，并把信息存放到v_file中
		void Position();//在每次操作开始前自动定位返回信息
		int my_lock(int);
		int my_unlock(int);
	  int ftpcmd(char *,char *, ...);
	  int IsCmdOk(const char *,const char *);
	  bool IsReadyToRead(int);
	  bool IsReadyToWrite(int);
	  int Accept(int,struct sockaddr *,int *);
};

#endif