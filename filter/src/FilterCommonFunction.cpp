
#include "FilterCommonFunction.h"

//返回值:	1 表示此文件之前不存在,在函数中新建并打开
//				0 表示此文件之前存在,只是打开
//异常CException:		无法打开或创建文件
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
		sprintf(szMsg, "无法分析文件=%s=",fileName);
		throw CException(1, szMsg, __FILE__, __LINE__);
	}
	strncpy(szDir, fileName, pos-fileName+1);
	strcpy(szFileName, pos+1);
	
	int returnValue = 0;
	if(access(fileName, F_OK) == -1)
	{
		//文件不存在;
		returnValue = 1;
		chkAllDir(szDir);
		FILE *tp = fopen(fileName, "ab+");
		fclose(tp);
	}
	
	FILE *p = fopen(fileName, mode);
	if(p == NULL)
	{
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "无法打开文件=%s=",fileName);
		throw CException(1, szMsg, __FILE__, __LINE__);
	}
	return p;
}

//返回值:	1 表示此文件之前不存在,在函数中新建并打开
//				0 表示此文件之前存在,只是打开
//异常CException:		无法打开或创建文件
int openFile(fstream &file_Stream, char *fileName)
{                  
	/*以下打开文件*/
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
		sprintf(szMsg, "无法分析文件=%s=",fileName);
		throw CException(1, szMsg, __FILE__, __LINE__);
	}
	strncpy(szDir, fileName, pos-fileName+1);
	strcpy(szFileName, pos+1);
	
	int returnValue = 0;
	if(access(fileName, F_OK) == -1)
	{
		//文件不存在;
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
			sprintf(szMsg, "无法打开文件=%s=",fileName);
			throw CException(1, szMsg, __FILE__, __LINE__);
		}
		tempStream.close();
		file_Stream.open(fileName, ios::in|ios::out|ios::binary);
		if (!file_Stream)
		{
			char szMsg[FILTER_ERRMSG_LEN];
			sprintf(szMsg, "无法打开文件=%s=",fileName);
			throw CException(1, szMsg, __FILE__, __LINE__);
		}
	}
	streambuf * ptrbuff = file_Stream.rdbuf(); //~ 将streambuf与文件句柄关联
	ptrbuff->pubsetbuf(NULL, 81920); //~ 设置缓冲区，即设置in的缓冲区
	return returnValue;
}


//清空文件内容
//输入: 文件名全路径
//返回: 无
//异常: 无法打开文件
void truncFile(const char * fileName)
{
	fstream outStream;
	outStream.open(fileName, ios::out|ios::trunc);
	if (!outStream)
	{
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "无法打开文件=%s=",fileName);
		throw CException(1, szMsg, __FILE__, __LINE__);
	}
	outStream.close();
	return;
}


