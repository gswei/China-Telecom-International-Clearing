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
	int GetAppError(); //          获取应用错误
	int GetOsError(); //           获取系统错误
	const char *GetErrMessage();//  获取错误信息描述
	const char *GetErrFileName();// 获取出错所在程序名
	int GetErrFileLine(); //       获取出错所在程序的行
	int PushStack(int nErrCode, const char * szMessage, const char *szErrFileName,
			int nErrFileLine);// 存入当前的信息到下层传来的异常类，传入上层
	const char *GetStack(); //    获取异常类带出来的信息
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
