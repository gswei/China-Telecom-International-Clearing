/*filename:ReadIni.h
descreption:define the interface to read ini file
create time:2010-04-13*/
/*modify 2010-05-12 :1.修改以前把文件名当做参数的读取配置接口，把读取文件单独拿出来当做一个接口。
                     2.修改或新增一个更新文件读取借口，配置文件有可能会修改，这是要重读配置文件。
*/
/*modefy 2010-0526 :1.增加读取整型变量的接口GetValue(const char *Section, const char *Variable, int &Value, char Reread='N')
                    2.调用了CF_Common的trim函数，可以去除空格，'\t','\n'等。
*/
//update 2010-0810  ini文件的section和变量不区分大小写，统一转换成大写处理
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

//要增加爱一个宏编译，要增加对"\t"等的处理，getValue数字直接调用邋getValue

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
	bool GetValue(const char *section,const char *Variable,char *Value,char Reread='N');//增加重读配置文件功能，Reread为Y表示重新读取。默认不重读
    bool GetValue(const char *Section, const char *Variable, int &Value, char Reread='N');//增加读取int型变量的借口
	static char* getVersion();
	
};
#endif
