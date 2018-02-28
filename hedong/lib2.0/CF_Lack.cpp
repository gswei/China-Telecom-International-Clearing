#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
//#include <direct.h>
#include "CF_Lack.h"

char* CurTime(char* stime)
{
    time_t	time1;
    struct tm	*time2;
    time(&time1);
    time2 = localtime(&time1);
    sprintf(stime, "%4d%02d%02d%02d%02d%02d", time2->tm_year + 1900,
            time2->tm_mon + 1, time2->tm_mday, time2->tm_hour, time2->tm_min, time2->tm_sec);
    return stime;
}

char* DelSpace(char* ss)
{
    if(!ss)
      return ss;
    int i;
    i = strlen(ss) - 1;
    while(i && ss[i] == ' ')
      i--;
    ss[i + 1] = '\0';
    i = 0;
    int len = strlen(ss);
    while((i<len) && ss[i] == ' ')
      i++;
    if(i != 0)
      ss += i;
    return ss;
}

char* DelDir(char* path)
{
    int len = strlen(path);
    if (path[len - 1] == '/')
        path[len - 1] = 0;
    return path;
}

int CheDir(char* path)
{
	if(!path)
		return ERR_DIR_NULLITY;
	path = DelSpace(path);
	DelDir(path);
	if(path[0] != '/')
		return ERR_DIR_NULLITY;
		
	char* pos=NULL;
	char* ch=path+1;

	while((pos=strchr(ch,'/')) != NULL)
	{
		int i = pos - path;
		char p[150];
		strncpy(p, path, i);
		p[i] = 0;
		if ( access( p,F_OK ) < 0 )
			if(mkdir(p, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH))
				return ERR_DIR_CREATE;
		ch = pos + 1;
	}
	if( access( path,F_OK ) < 0 )
		if(mkdir(path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH))
			return ERR_DIR_CREATE;
	return SUCCESS;
}