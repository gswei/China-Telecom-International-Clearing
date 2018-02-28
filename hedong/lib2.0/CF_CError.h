/****************************************************************
filename: CF_CError.h
module: CF - Common Function
created by: wang hui
create date: 2000-010-13
update list: 
			1)2005-04-29	周亚军	将所有errcode移到errcode.h文件;
									将CF_CError构造函数中msg的数据类型
									由char*改为const char*;
version: 1.1.2
description:
    the header file of the classes for error
*****************************************************************/

#ifndef _CF_CERROR_H_
#define _CF_CERROR_H_ 1

//#include <string>
#include "config.h"
#include "errcode.h"

#define ERR_TYPE_OS		'O'
#define ERR_TYPE_DB		'D'
#define ERR_TYPE_ELSE	'E'

#define ERR_LEVEL_LOW	'S'
#define ERR_LEVEL_MID	'M'
#define ERR_LEVEL_HIG	'H'

//using std::string;
class CF_CError
{
  public:
      CF_CError(int errorTypeCode, int errorLevelCode, int appErrorCode, int osErrorCode, 
                const char* msg, char *errHappenFile, int errHappenLine);
                
      //added by tanj 20050907
      CF_CError(int errorTypeCode, int errorLevelCode, int appErrorCode, int osErrorCode, 
                const char* msg, const char *errHappenFile, int errHappenLine);
      int get_appErrorCode() ;
      int get_osErrorCode();
      int get_errType();
      int get_errLevel();
      const char *get_errMessage()  ;
      const char *get_errHappenFileName()  ;
      int get_errHappenLine();
      
  private:
      int appError;
      int osError;
      int errType;
      int errLevel;
      string errMessage;
      string errFileName;  // from which file the error is thrown out
      int errFileLine;	   // from which line in the file the error is thrown out
};


inline CF_CError::CF_CError(int errorTypeCode, int errorLevelCode, 
      		int appErrorCode, int osErrorCode, 
      		const char* msg, char *errHappenFile, int errHappenLine)
{
	appError = appErrorCode;
	osError = osErrorCode;
	errType = errorTypeCode;
	errLevel = errorLevelCode;
	errMessage = msg;
	errFileName = errHappenFile;
	errFileLine = errHappenLine;
}


//added by tanj 20050907
inline CF_CError::CF_CError(int errorTypeCode, int errorLevelCode, 
      		int appErrorCode, int osErrorCode, 
      		const char* msg, const char *errHappenFile, int errHappenLine)
{
	appError = appErrorCode;
	osError = osErrorCode;
	errType = errorTypeCode;
	errLevel = errorLevelCode;
	errMessage = msg;
	errFileName = errHappenFile;
	errFileLine = errHappenLine;
}


inline int CF_CError::get_appErrorCode()
{
	return appError;
}

inline int CF_CError::get_osErrorCode()
{
	return osError;
}

inline int CF_CError::get_errType()
{
   return errType;
}

inline int CF_CError::get_errLevel()
{
   return errLevel;
}
      

inline const char *CF_CError::get_errMessage()
{
	return errMessage.c_str();
}


inline const char *CF_CError::get_errHappenFileName()
{
   return errFileName.c_str();
}

inline int CF_CError::get_errHappenLine()
{
   return errFileLine;
}




class CF_CErrorFile:public CF_CError
{
  public:
      CF_CErrorFile(int errorTypeCode, int errorLevelCode, 
      		int appErrorCode, int osErrorCode, 
      		char* msg, char *errHappenFile, int errHappenLine);

  private:

};

inline CF_CErrorFile::CF_CErrorFile(int errorTypeCode, int errorLevelCode, 
      		int appErrorCode, int osErrorCode, 
      		char* msg, char *errHappenFile, int errHappenLine)
       :CF_CError(errorTypeCode, errorLevelCode, appErrorCode,osErrorCode, 
      		msg, errHappenFile, errHappenLine)
{

}

#endif
