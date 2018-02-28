#ifndef CF_FilE_SCAN_H
#define CF_FilE_SCAN_H 1

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_NUM 50000

class CF_FileScan
{
    private:
	  typedef struct FILE_INFO
          {
	    char filename[300];
          } FILE_INFO;
    int file_count,max_count,add_count;
    int flag,floor,filterNum;
    char fmt[10][20];
    char scanPath[255];
    FILE_INFO  *filelist;
	public:
	  CF_FileScan();
	  ~CF_FileScan();
	  int scan_file(char *,int,char *temp=NULL);
	  int pro_file(char *,int);
	  int get_file(char *);
    bool checkFormat(const char *,const char *);
};

#endif

