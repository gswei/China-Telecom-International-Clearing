
#include "FilterCommonFunction.h"

//����ֵ:	1 ��ʾ���ļ�֮ǰ������,�ں������½�����
//				0 ��ʾ���ļ�֮ǰ����,ֻ�Ǵ�
//�쳣CException:		�޷��򿪻򴴽��ļ�
FILE * openfile(char *fileName, const char * mode)
{
	char szDir[FILTER_FILESIZE];
	memset(szDir, 0, sizeof(szDir));
	char szFileName[FILTER_FILESIZE];
	memset(szFileName, 0, sizeof(szFileName));
	char *pos = NULL;
	
	if((pos=strrchr(fileName, '/')) == NULL)
	{
		//TODO: throw here;
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "�޷������ļ�=%s=",fileName);
		throw CException(1, szMsg, __FILE__, __LINE__);
	}
	strncpy(szDir, fileName, pos-fileName+1);
	strcpy(szFileName, pos+1);
	
	int returnValue = 0;
	if(access(fileName, F_OK) == -1)
	{
		//�ļ�������;
		returnValue = 1;
		chkAllDir(szDir);
		FILE *tp = fopen(fileName, "ab+");
		fclose(tp);
	}
	
	FILE *p = fopen(fileName, mode);
	if(p == NULL)
	{
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "�޷����ļ�=%s=",fileName);
		throw CException(1, szMsg, __FILE__, __LINE__);
	}
	return p;
}

//����ֵ:	1 ��ʾ���ļ�֮ǰ������,�ں������½�����
//				0 ��ʾ���ļ�֮ǰ����,ֻ�Ǵ�
//�쳣CException:		�޷��򿪻򴴽��ļ�
int openFile(fstream &file_Stream, char *fileName)
{                  
	/*���´��ļ�*/
	if (file_Stream)
	{
		file_Stream.close();              
	}
	
	char szDir[FILTER_FILESIZE];
	memset(szDir, 0, sizeof(szDir));
	char szFileName[FILTER_FILESIZE];
	memset(szFileName, 0, sizeof(szFileName));
	char *pos = NULL;
	
	if((pos=strrchr(fileName, '/')) == NULL)
	{
		//TODO: throw here;
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "�޷������ļ�=%s=",fileName);
		throw CException(1, szMsg, __FILE__, __LINE__);
	}
	strncpy(szDir, fileName, pos-fileName+1);
	strcpy(szFileName, pos+1);
	
	int returnValue = 0;
	if(access(fileName, F_OK) == -1)
	{
		//�ļ�������;
		returnValue = 1;
		chkAllDir(szDir);
	}
	
	file_Stream.open(fileName, ios::in|ios::out|ios::binary);
	
	if (!file_Stream)
	{
		fstream tempStream;
		tempStream.open(fileName, ios::out|ios::binary);
		if (!tempStream)
		{
			char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "�޷����ļ�=%s=",fileName);
			throw CException(1, szMsg, __FILE__, __LINE__);
		}
		tempStream.close();
		file_Stream.open(fileName, ios::in|ios::out|ios::binary);
		if (!file_Stream)
		{
			char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "�޷����ļ�=%s=",fileName);
			throw CException(1, szMsg, __FILE__, __LINE__);
		}
	}
	streambuf * ptrbuff = file_Stream.rdbuf(); //~ ��streambuf���ļ��������
	ptrbuff->pubsetbuf(NULL, 81920); //~ ���û�������������in�Ļ�����
	return returnValue;
}


//����ļ�����
//����: �ļ���ȫ·��
//����: ��
//�쳣: �޷����ļ�
void truncFile(const char * fileName)
{
	fstream outStream;
	outStream.open(fileName, ios::out|ios::trunc);
	if (!outStream)
	{
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "�޷����ļ�=%s=",fileName);
		throw CException(1, szMsg, __FILE__, __LINE__);
	}
	outStream.close();
	return;
}


