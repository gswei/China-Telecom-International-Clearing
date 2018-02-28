/************************************************************************************************
*��	����		CCJmanager��C��J����
*��	;�� 		FTP�Զ��������䣨�ɼ����ļ�����
*��	�ߣ�		��	��
*ʹ�÷�����
			1.�趨�ϴ������ص�ENV��
				SetEnvPathName( char *cDownloadEnvPathName, char *cUploadEnvPathName );
			2.�趨��־�ļ���
				DealWithLog( char *logPathName );
			3�ϴ���(��ʼ����ENV�ļ��Զ��������ϴ��ļ��������û�������������)
				BeginUpload( char *errMsg );
			4���أ�(��ʼ����ENV�ļ��Զ������������ļ��������û�������������)
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

//���룺wjjs: 3f841c6e   wjjs123: 3f841c6e565b92

//����ƥ�䷽����ϸ��
struct MODE_INFO{
	char		cStyle[100];		//�ļ���ʽ��������ʽ��
	char		cMode[5];		//�ļ�����ģʽ��10��2����
	char		cCover[5];		//�ļ�����ģʽ
	char		cSource[200];		//������ɺ�Դ�ļ��Ĵ���ʽ
};

//һ�����ȼ�������
struct GRADE_INFO{
	int		iGrade;			//���ȼ�
	char		cLocal[200];		//����·��
	char		cRemote[200];		//Զ��FTP��������·��
	char		cChild[5];		//�ϴ�/����·�����Ƿ�������ļ���
	char		cTree[5];		//�Ƿ�����Ŀ¼��
	char		cCompress[5];		//�Ƿ��ѹ��
	char		cPass[100];		//���ܵ��ܳף���*��������
	MODE_INFO	modeInfo[100];		//����ƥ�䷽����ϸ��	
	int		iModeCount;		//MODE_INFO����Ԫ�صĸ���
};

class CCJmanager{
public:
	char		m_cDownloadEnvPathName[1024];		//FTP���ȼ����õ�ENV�ļ�ȫ·��+�ļ���
	char		m_cUploadEnvPathName[1024];		//FTP���ȼ����õ�ENV�ļ�ȫ·��+�ļ���
	char		m_cFtpEnvPathName[1024];		//FTP���ȼ����õ�ENV�ļ�ȫ·��+�ļ���


	CCJmanager();
	~CCJmanager();
	
	//��־�ĳ�ʼ������
	int DealWithLog( char *logPathName );
	
	//����ENV��ʼ�Զ�����
	int BeginDownload( char *errMsg );
	
	//����ENV��ʼ�Զ��ϴ�
	int BeginUpload( char *errMsg );
	
	//����FTP���ȼ����õ�ENV�ļ�ȫ·��+�ļ���
	void SetEnvPathName( char *cDownloadEnvPathName, char *cUploadEnvPathName );
	
	//���ĳһԶ��·���µ�ĳһָ��ʱ����ڵ��ļ��б�
	int GetUnixListBetween( const char *path, const char *listFile, const char *beginTime, 
				const char *endTime, SERVER_INFO serverInfo, char *errMsg );
	
	//���ĳһԶ��·���µ�ĳһָ��ʱ����ڵ��ļ��б�
	int GetWindowsListBetween( const char *path, const char *listFile, const char *beginTime, 
				const char *endTime, SERVER_INFO serverInfo, char *errMsg );
	
	//����ĳָ���ļ��б��е��ļ�
	int DownloadFromList( const char *listName, const char *listPath, SERVER_INFO serverInfo, SEND_INFO sendInfo, char *errMsg );
	
	//���ĳһԶ��·���µķ���ĳһָ��������������ʽ�����ļ��б�
	int GetUnixListBy( const char *path, const char *listFile, const char *condition, 
				SERVER_INFO serverInfo, char *errMsg );

	//���ĳһԶ��·���µķ���ĳһָ��������������ʽ�����ļ��б�
	int GetWindowsListBy( const char *path, const char *listFile, const char *condition, 
				SERVER_INFO serverInfo, char *errMsg );

private:	

	//��ĳһָ���ļ��б��з���ĳ������������ʽ�����ļ�������б�
	int GetWindowsListByCondition( const char *oldListFile, const char *newListFile,
						 const char *condition, char *errMsg );

	//��ĳһָ���ļ��б���ָ��ʱ����ڵ��ļ�������б�
	int GetWindowsListByTime( const char *oldListFile, const char *newListFile, const char *beginTime, 
				const char *endTime, char *errMsg );

	//��ĳһָ���ļ��б��з���ĳ������������ʽ�����ļ�������б�
	int GetUnixListByCondition( const char *oldListFile, const char *newListFile,
						 const char *condition, char *errMsg );

	//��ĳһָ���ļ��б���ָ��ʱ����ڵ��ļ�������б�
	int GetUnixListByTime( const char *oldListFile, const char *newListFile, const char *beginTime, 
				const char *endTime, char *errMsg );

	//�����·ݵ�Ӣ�ĵõ��·ݵ������ַ���
	int GetMonth( const char *oldMonth, char *newMonth );

	//�õ���������Ϣ
	int GetServerInfo( SERVER_INFO *struGradeInfo, char *cErrMsg, int iMode );
	
	//�õ�ĳһ���ȼ�����Ϣ
	int GetGradeInfo( int iGrade, GRADE_INFO *struGradeInfo, char *cErrMsg, int iMode );
	
	//���GRADE_INFO�����ݵ�������
	int CheckGradeInfo( GRADE_INFO serverInfo, char *cErrMsg );
	
	//���SERVER_INFO�����ݵ�������
	int CheckServerInfo( SERVER_INFO struGradeInfo, char *cErrMsg );

	//�������ȼ�Ϊgrade���ļ�
	int DownloadByGrade( int grade, char *errMsg );
	
	//�ϴ����ȼ�Ϊgrade���ļ�
	int UploadByGrade( int grade, char *errMsg );
	
	//�ж��ļ��Ƿ����
	int IsFileExist(char *p_cPathName);
	
	//����ĳ�ļ����µ������ļ�
	int DownloadFrom( char *downloadPath, SERVER_INFO serverInfo, GRADE_INFO struDataInfo, char *errMsg );
	
	//�ж��ļ����Ƿ����������ʽ����
	int IsFileMatch( char *cOldFileName, char *cCondition );
	
	//�ϴ�ĳ�ļ����µ������ļ�
	int UploadFrom( char *uploadPath, SERVER_INFO serverInfo, GRADE_INFO struDataInfo, char *errMsg );
	
	//�ж����ļ������ļ���
	int IsItDir(char *cPathName);
	
	//��ENV�ļ��зֽ�STYLE,MODE,COVER,SOURCE�ĸ���
	int GetDetail( const char *cLine, GRADE_INFO *gradeInfo, int iFlag );
	
	//ȥ���ַ������ߵĿո�,Tab,�س�
	void Trim(char *cStr);
	
	//�õ��ļ���С
	int GetFileSize( const char *filePathName );
	
};

#endif
