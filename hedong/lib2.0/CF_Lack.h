/*
cf_checkdir : common function for class CF_CErrorLackRec
*/
#ifndef _CF_LACK_
#define _CF_LACK_

#define ERR_DIR_CREATE  -11     //Ŀ¼����ʧ��
#define ERR_DIR_CHANGE  -12     //Ŀ¼ת��ʧ��
#define ERR_DIR_NULLITY -13     //��Ч�ľ���·��
#define SUCCESS      	0       //�ɹ�

char* DelSpace(char* ss);
char* CurTime(char* stime);
char* DelDir(char* path);
int CheDir(char *path);

#endif
