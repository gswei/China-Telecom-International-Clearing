/****************************************************************
filename: DIR_Scan.h
module: CF - Common Function
created by: sunhua
create date: 2004-12-21
update list: 
version: 1.1.1
description:
    the header file of the classes for DIR_Scan
*****************************************************************/

#ifndef _DIR_Scan_H_
#define _DIR_Scan_H_ 1

#include "config.h"
#include <dirent.h>
#include <sys/types.h>
#include <strings.h>

#define SUC	0
#define FAIL	-1
#define NODATA 100
#define DBNOFOUND 100

class DIR_Scan {
  public :
  	~DIR_Scan();
  	
  	int openDir(char* dirName);
  	int getFile(char *format ,int flag , char *fileName );
  	int getOneFile(int &flag,char *fileName);
  	void rewind();
		void closeDir();	
  	bool checkFormat(const char *cmpString, const char *format);
  private:
  	DIR *dirfp;
  	char dir[500]; 	
};
#endif
