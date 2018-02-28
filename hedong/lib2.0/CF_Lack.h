/*
cf_checkdir : common function for class CF_CErrorLackRec
*/
#ifndef _CF_LACK_
#define _CF_LACK_

#define ERR_DIR_CREATE  -11     //目录生成失败
#define ERR_DIR_CHANGE  -12     //目录转换失败
#define ERR_DIR_NULLITY -13     //无效的绝对路径
#define SUCCESS      	0       //成功

char* DelSpace(char* ss);
char* CurTime(char* stime);
char* DelDir(char* path);
int CheDir(char *path);

#endif
