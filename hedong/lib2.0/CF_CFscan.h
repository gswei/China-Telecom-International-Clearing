/****************************************************************
filename: CF_CFscan.h
module: CF - Common Function
created by: wang hui
create date: 2001-04-04
update list: 
version: 1.1.1
description:
    the header file of the classes for CF_CFscan
*****************************************************************/

#ifndef _CF_CFscan_H_
#define _CF_CFscan_H_ 1

#include <stdio.h>
#include <iostream.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define SUC	0
#define FAIL	-1
#define NODATA 100
#define DBNOFOUND 100

class CF_CFscan
{
  public :
  	CF_CFscan();
  	~CF_CFscan();
  	int openDir(char* dirName);
  	int getFile(const char *format, char *fileName);
  	//add by lwu 2005-06-23
  	int getFile(char *fileName);
  	int getDir(char *format, char *fileName);
  	void rewind();
	  void closeDir();	
	private:
  	bool checkFormat(const char *cmpString, const char *format);
  	int  SplitBuf(const char *,char);

  private:
  	DIR *dirfp;
  	char dir[300];
  	char sz_Var[20][100];
  	char  sz_VarBuf[400];
  	int  iVarNum;
  	int flag;
  	
  	
  	
};
#endif
