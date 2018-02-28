/****************************************************************
filename: CF_CException.h
module: for log
created by: Tan Zehua
create date: 2010-05-05
update list: 

version: 1.0.0
description:
    the header file of the classes for error
*****************************************************************/

#ifndef CF_CEXCEPTION_H_
#define CF_CEXCEPTION_H_

#include "CF_Config.h"
#include "CF_Cerrcode.h"
#include <vector>
#include <string>

class ExceptionLocation {
public:
			ExceptionLocation(int code, std::string message, std::string file,
					int line);
	int getCode();
	std::string getMessage();
	std::string getFileName();
	int getFileLine();
private:
	int code;
	std::string message;
	std::string filename;
	int line;
};

class CException {
public:
	CException(int nAppError, const char* szErrMessage,
			const char *szErrFileName, int nErrFileLine);
	int GetAppError(); //          ��ȡӦ�ô���
	int GetOsError(); //           ��ȡϵͳ����
	const char *GetErrMessage();//  ��ȡ������Ϣ����
	const char *GetErrFileName();// ��ȡ�������ڳ�����
	int GetErrFileLine(); //       ��ȡ�������ڳ������
	int PushStack(int nErrCode, const char * szMessage, const char *szErrFileName,
			int nErrFileLine);// ���뵱ǰ����Ϣ���²㴫�����쳣�࣬�����ϲ�
	const char *GetStack(); //    ��ȡ�쳣�����������Ϣ
	std::vector<ExceptionLocation> getStackLocations();


private:
	int osErrorCode;
	int appErrorCode;
	std::string errorMessage;
	std::string errorFile;
	int errorLine;
	std::vector<ExceptionLocation> errorLocationStack;
	std::vector<std::string> stacktrace;
	CException();
};

#endif /* CF_CEXCEPTION_H_ */
