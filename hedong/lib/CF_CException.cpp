/****************************************************************
filename: CF_CException.cpp
module: for log
created by: Tan Zehua
create date: 2010-05-05
update list: 

version: 1.0.0
description:
    the cpp of the classes for error
*****************************************************************/

#include "CF_CException.h"

using namespace std;

ExceptionLocation::ExceptionLocation(int code, std::string message,
		std::string file, int line) {
	this->code = code;
	this->message = message;
	this->filename = file;
	this->line = line;
}

int ExceptionLocation::getCode() {
	return this->code;
}

std::string ExceptionLocation::getMessage() {
	return this->message;
}

std::string ExceptionLocation::getFileName() {
	return this->filename;
}

int ExceptionLocation::getFileLine() {
	return this->line;
}

CException::CException(int nAppError, const char* szErrMessage,
		const char *szErrFileName, int nErrFileLine) {
	this->osErrorCode = errno;
	this->appErrorCode = nAppError;
	this->errorMessage = szErrMessage;
	this->errorFile = szErrFileName;
	this->errorLine = nErrFileLine;

	PushStack(nAppError, szErrMessage, szErrFileName, nErrFileLine);

}

int CException::GetAppError() {
	return appErrorCode;
}

const char* CException::GetErrMessage() {
	return errorMessage.c_str();
}

const char* CException::GetErrFileName() {
	return errorFile.c_str();
}

int CException::GetErrFileLine() {
	return errorLine;
}

int CException::PushStack(int nErrCode, const char * szMessage,
		const char *szErrFileName, int nErrFileLine) {
	std::string msg = szMessage;
	std::string file = szErrFileName;
	appErrorCode = nErrCode;

	ExceptionLocation lo(nErrCode, msg, file, nErrFileLine);
	errorLocationStack.push_back(lo);
	return 0;
}

vector<ExceptionLocation> CException::getStackLocations() {
	return errorLocationStack;
}

const char* CException::GetStack() {
	std::string stack;

	// Header
	char eno[10];
	sprintf(eno, "%d", osErrorCode);
	stack.append("\n");
	// Body
	for (int i = (errorLocationStack.size() -1) ; i >=0 ; i--) {
		ExceptionLocation lo = errorLocationStack[i];
		char code[30], line[30], flag[10];
		sprintf(code, "%d", lo.getCode());
		sprintf(line, "%d", lo.getFileLine());
		sprintf(flag, "%d", (errorLocationStack.size() - i));
		std::string tmp;
		tmp.append("                        ").append(flag).append(": ").append(code).append("  ")
		    .append(eno).append("  ").append(lo.getMessage()).append(
				"  (").append(lo.getFileName()).append(":").append(line).append(
				")");
		stack.append(tmp.c_str());
		if (i != 0) {
			stack.append("\n");
		}
	}
	return stack.c_str();
}
