/*filename:ReadIni.h
descreption:define the interface to read ini file
create time:2010-04-13*/
/*modify 2010-05-12 :1.�޸���ǰ���ļ������������Ķ�ȡ���ýӿڣ��Ѷ�ȡ�ļ������ó�������һ���ӿڡ�
                     2.�޸Ļ�����һ�������ļ���ȡ��ڣ������ļ��п��ܻ��޸ģ�����Ҫ�ض������ļ���
*/
/*modefy 2010-0526 :1.���Ӷ�ȡ���ͱ����Ľӿ�GetValue(const char *Section, const char *Variable, int &Value, char Reread='N')
                    2.������CF_Common��trim����������ȥ���ո�'\t','\n'�ȡ�
*/
//update 2010-0810  ini�ļ���section�ͱ��������ִ�Сд��ͳһת���ɴ�д����
#ifndef C_CREADINI
#define C_CREADINI

#include <stdio.h>
#include <stdlib.h>
#include <fstream.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "CF_Common.h"

class CReadIni;
extern CReadIni theCfgFile;

//Ҫ���Ӱ�һ������룬Ҫ���Ӷ�"\t"�ȵĴ���getValue����ֱ�ӵ�����getValue

using std::vector;
using std::string;

#ifndef LENTH
#define LENTH 5120
#endif

class CReadIni{
private:
	vector <string> v_record;
	char IniFilename[LENTH];
	
public:
	CReadIni();
	bool init(const char *filaneme);
	bool GetValue(const char *section,const char *Variable,char *Value,char Reread='N');//�����ض������ļ����ܣ�RereadΪY��ʾ���¶�ȡ��Ĭ�ϲ��ض�
    bool GetValue(const char *Section, const char *Variable, int &Value, char Reread='N');//���Ӷ�ȡint�ͱ����Ľ��
	static char* getVersion();
	
};
#endif
