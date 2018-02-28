/***********************************************************************************************
��	����		CFTPmanager
��	;��		��FTP�������ϵ��ļ����У��ϴ������أ��õ��ļ��б�����Զ���ļ����ļ��С�
��	�ߣ�		��	��
version:    1.0.3.1
update history��
by yangzx 2006.05.16 �޸�List�ӿ�֧�ֻ�ȡԶ��dir�µ������ļ��б������Ŀ¼�µ��ļ�������private method ScanFile()
by yangzx 2006.05.20 ���ӽӿ�SizeFile()ȡ�ļ���С��	Ϊ��֤��ɳ���ļ����ԣ�����ListAllFile�ӿڣ���֧�ֻ�ȡԶ��dir��
										�������ļ��б������Ŀ¼�µ��ļ���
										int ListAllFile( const char *path, const char *listSavePathName, char *errMsg );
										ԭ�е�List�ӿڹ��ܲ��䣬ֻȡ��ǰĿ¼�µ��ļ�����ȡ��Ŀ¼�µ��ļ���
by yangzx 2006.05.21 ʹ��select�������block���⣬��download() ��upload()��ʹ��
by yangzx 2006.06.01 ��Ϊ��hp�Ļ�����ʹ��select���������⣬��ʱע�͵�ʹ��select�����Ĳ��֣��ָ���ǰ��upload��download
										 ����
by yangzx 2006.06.09 12:00 �޸�ListBy��bug��ע�͵�����ʹ�õ�select�����Ĳ���
by yangzx 2006.06.12 ʹ�� poll() �������� select()������select()������hp�Ļ����ϱ��������ʱ���ȶ�
by yangzx 2006.07.06 �޸�ListAllFile()��ScanFile()�������޸����������Ϣ,�޸�DivFileName( listSavePathName,filePath, fileName )�����ڿ���·����filePath��bug��filePath��ʱ��û���ַ�����ֹ�� 0
by yangzx 2006.07.31 �޸�ftpcmd()��read()��write()ǰ��ʹ��IsReadyToRead()��IsReadyToWrite()��װ
by yangzx 2006.08.05 �ں���CFTPmanager()������signal(SIGPIPE,SIG_IGN)
by yangzx 2006.08.10 ʹ�ý��з�������װ��Accept()����ֱ�ӵ���accept()
by yangzx 2006.08.30 ���Connect()��������ftp�û�������������ʱ��û�����m_ftpioָ���bug,��shutdown()����֮��Ҫ����close()��shutdown()����ֻ�ǹر����ӣ������汾��Ϊ1.0.2
by yangzx 2006.09.20 �ڵ���select() �� poll()ʱ���ϵͳ����real time signal ��ʹ���������أ��������errno=EINTR;�޸Ĵ����ڵ���select()��poll()ʱ������EINTR���� 
by yangzx 2006.10.11 debug DivFileName()�����ļ��ڸ�·��ʱ����/test.dat���ú����޷���ȷ�ֽ���ļ���·�����ļ������汾����Ϊ1.0.3.1���³��������л���ݸ132.119.200.60��ʹ��,1.0.5ֻ�ڼƷѲ������·��ӿڻ���ʹ�ù����д�ȫ����ԡ�
by zhanggq 2007.4.19 �޸ĳ�ʱʱ�䣬��60S��Ϊ300S
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

/* begin of ������Ϣ�Ķ���****************************************************************/

//���粿��:
#define GET_SERVER_NAME_ERR			     -1	//��FTP����������ȡFTP������IPʧ��
#define CREATE_SOCK_ERR				       -2	//������FTP������ͨѶ��SOCKETʧ��
#define SOCK_CONNECT_ERR			       -3	//ͨѶ��SOCKET���������������ʧ��
#define RECIEVE_SERVER_MSG_ERR			 -4	//���ܷ�����������Ϣʧ��
#define LOGIN_ERR				             -5	//��¼FTP������ʧ�ܣ��û������������
#define CREATE_LISTENING_SOCK_ERR		 -6	//���շ��������ݵ�SOCKET����ʧ��
#define LISTENING_SOCK_BIND_ERR			 -7	//���ܷ��������ݵ�SOCKET�˿ڰ�ʧ��
#define SOCK_LISTENING_ERR 			     -8	//���ܷ��������ݵ�SOCKET����ʧ��
#define SEND_CMD_ERR				         -9	//�������ݵ�������ʧ��
#define SOCKET_BLOCK_READ		         -10//����read block
#define SOCKET_BLOCK_WRITE           -11//����write block
#define SOCKET_SELECT_ERROR          -12//select()��������
#define SOCKET_POLL_ERROR            -13//poll()error

//�ļ����֣�
#define TARGET_FILE_CREATE_OPEN_ERR	 -20	//Ŀ���ļ�����/��ʧ��
#define DOWNLOAD_SOURCE_NOT_EXIST		 -21	//FTP��������Ҫ���ص�Դ�ļ�������
#define DELETE_SOURCE_ERR			       -22	//�������ɾ��Դ�ļ�ʧ��
#define STORE_SOURCE_ERR			       -23	//������󱸷�Դ�ļ�ʧ��
#define OPEN_SOURCE_FILE_ERR			   -24	//����Դ�ļ���ʧ��
#define UPLOAD_TARGET_PATH_ERR			 -25	//FTP���������ϴ��ļ��ı���·��������
#define SOURCE_PATH_NOT_EXIST			   -26	//Դ�ļ�·��������
#define SOURCE_FILE_NOT_EXIST			   -27	//Դ�ļ�������
#define TARGET_PATH_NOT_EXIST			   -28	//Ŀ���ļ��в�����
#define BOTH_NULL_ERR				         -29	//Ŀ���ļ���Ŀ���ļ���������������ͬʱΪ��
#define FOLDER_PATH_NOT_EXIST			   -30	//�ļ���·��������
#define FOLDER_NOT_EXIST			       -31	//�ļ��в�����
#define FOLDER_EXIST				         -34	//�ļ����Ѿ�����
#define RENAME_FILE_ERR				       -35	//�ļ�������ʧ��
#define FILE_SIZE_ERR				         -36	//�ļ���С��һ��
#define MOVE_FILE_ERR                -37  //�ƶ��ļ�����
#define DELETE_FILE_ERR              -38  //ɾ���ļ�����
#define LOCK_FILE_ERR                -39  //���ļ�����
#define UNLOCK_FILE_ERR              -40  //���ļ�����

//ҵ���߼�����:
#define UNRECOGNIZED_SENDING_MODE		-50	//����ʶ����ģʽ
#define UNRECOGNIZED_COVER_MODE			-51	//����ģʽ�������󣨲���ʶ��

/**************************************************************** end of ������Ϣ�Ķ���*/

/* begin of ����ʹ�ò����Ķ��� *********************************************************/
#define MODE_BIN			2	//�ļ�����ģʽ��2���ƣ�
#define MODE_ASC			10	//�ļ�����ģʽ���ı���
#define TARGET_COVER	1	//�ļ��ϴ�ʱĿ���ļ�����ʱ�Ĵ���ʽ�����ǣ�
#define TARGET_STORE	0	//�ļ��ϴ�ʱĿ���ļ�����ʱ�Ĵ���ʽ�����ݣ�
#define SOURCE_DELE		"*"	//�ļ�������ɺ�Դ�ļ��Ĵ���ʽ��ɾ����
#define SOURCE_IGNORE	"**"	//�ļ�������ɺ�Դ�ļ��Ĵ���ʽ(������)
#define COMPRESS_Y		"Y"	//�ļ�������ɺ�Ҫ��ѹ
#define COMPRESS_N		"N"	//�ļ�������ɺ󲻽�ѹ
#define PASS_N				"*"	//�ļ�������ɺ󲻽���

/*********************************************************** end of ����ʹ�ò����Ķ��� */

/*begin of const varible define *********************************************************/
int const BLOCK_TIME_OUT_SEC=300;    //select �����е�socket �ȴ�ʱ�䣬�������� struct timeval.tv_sec
int const BLOCK_TIME_OUT_uSEC=0;		 //select �����е�socket �ȴ�ʱ�䣬�������� struct timeval.tv_sec
int const BLOCK_TIME_OUT_MSEC=300000;//poll() �����е�socket �ȴ�ʱ�� 
/*********************************************************** end of const varible define */

//��������Ϣ
struct SERVER_INFO
{
	char cIp[20];		      //Զ��FTP������������IP
	char cUser[100];		  //FTP�������ĵ�¼�û���
	char cPassword[100];  //FTP�������ĵ�¼����
};

//������Ϣ
struct SEND_INFO
{
	char localFile[1024];	//�����ļ�
	char remoteFile[1024];//Զ���ļ�	
	int	 coverFlag;		    //�ļ����串�Ǳ�־
	int	 sendMode;		    //�ļ�����ģʽ
	char cStorePath[200]; //������ɺ�Դ�ļ�����ʽ
};

//�ļ���Ϣ

int _getsockname(int,struct sockaddr *,int *);

int _accept(int,struct sockaddr *,int *);

class CFTPmanager
{
  private:	
	  char sz_serverIp[20];		     //FTP������IP
	  char sz_loginName[100];	     //FTP��������¼��
	  char sz_loginPassword[100];	 //FTP��������¼����
	  int  iServerPort;            //FTP�������˿ں�
	  FILE *m_ftpio;               //��FTPserverͨѶ���ļ�ָ��
	  int	isockftp;		             //��FTPserverͨѶ���׽���
	  int temp_num;                //��ʱ�ļ������
	  int iFilesizeFlag;          //��ȡ�ļ���С�Ŀ���ftp�����ʶ��0,��δ�ж���ʲô������ã�1,STAT,SIZE �������ã�2,STAT����;3,SIZE,����
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
	  int Download(const char *,const char *,int,char *,int ,const char *,const char *,const char *);	//��FTP�������������ļ�(��������/ѹ)

	  int Upload(const char *,const char *,int,char *,int ,const char *);
	  int Upload(SEND_INFO,char *);

	  int List(const char *,const char *,char *);
	  int ListAllFile(char *,const char *,char *);//�õ�FTP��������ָ��Ŀ¼�µ��ļ��б�,������Ŀ¼�µ��ļ��б��ļ�����ȫ·����Ϣ
	  int ListUnixBy(const char *,const char *,const char *,char *);//�õ�Unixϵͳ(Զ�̻�)ĳ·���·���ĳ������ʽ���ļ����б�
    int ListWindowsBy(const char *,const char *,const char *,char *);//�õ�Unixϵͳ(Զ�̻�)ĳ·���·���ĳ������ʽ���ļ����б�
		int ListUnixBetween(const char *,const char *,const char *,const char *,char *);//���ĳһԶ��·���µ�ĳһָ��ʱ����ڵ��ļ��б�
	  int ListWindowsBetween(const char *,const char *,const char *,const char *,char *);//���ĳһԶ��·���µ�ĳһָ��ʱ����ڵ��ļ��б�
		int ListBy(const char *,const char *,const char *,char *);//��ȡԶ�̻�����ĳ·���·���ĳ������ʽ���ļ����б�

	  int ChDir(const char *,char *);//�л���ĳһ·��
	  int MkDir(const char *,char * );//������·��
	  int RmDir(const char *,char *);//ɾ��Զ���ļ�
	  int DirFileExist(const char *);//���Զ���ļ��Ƿ����
	  int RmFile(const char *,char *);//ɾ��Զ���ļ�
	  int MoveFile(const char *,const char *, char *);//�ƶ�Զ���ļ�
	  
	  int DivFileName(const char *, char *, char *);//���ļ�ȫ·���ֽ���·�����ļ���
	  int SizeFile(char * ,int & ,char *);//��ȡָ��Զ���ļ��Ĵ�С
	  int IsFileExist(const char *);
	  int IsUnixZhengZe(const char *buf,const char *condition );//�жϸü�¼���ļ����Ƿ����Ҫ��(������ʽ)
	  int IsWindowsZhengZe(const char *,const char * );//�жϸü�¼���ļ����Ƿ����Ҫ��(������ʽ)
	  int IsMatchZhengZe(char *,char *);//�ж�ĳ�ַ����Ƿ����ĳ������ʽ
	  int IsUnixBetween(const char *,const char *,const char *,char *);//�жϸü�¼���ļ�ʱ���Ƿ���ĳָ��ʱ�䷶Χ��
	  int IsWindowsBetween(const char *,const char *,const char *,char *);//�жϸü�¼���ļ�ʱ���Ƿ���ĳָ��ʱ�䷶Χ��
	  int GetDate(char *);//ȡ��XXXX-XX-XX��ʽ�ĵ�ǰ����
	  int GetMonthByChr(const char *,char *); //����ĸ�·�ȡ�������·�
	  int Reconnect(char *);//�Զ������ӷ�����
	  int GetFileSize(const char * );//�õ��ļ���С
    void CheckPath(char *);//���path�����Ƿ��'/',�������ɾ�� 
    int ChFile(const char *,char *);
	private:
		int ScanFile(char * ,char* ,char*);//��ȡԶ��Ŀ¼�µ������ļ���Ϣ��������Ϣ��ŵ�v_file��
		void Position();//��ÿ�β�����ʼǰ�Զ���λ������Ϣ
		int my_lock(int);
		int my_unlock(int);
	  int ftpcmd(char *,char *, ...);
	  int IsCmdOk(const char *,const char *);
	  bool IsReadyToRead(int);
	  bool IsReadyToWrite(int);
	  int Accept(int,struct sockaddr *,int *);
};

#endif