#include "FileLoad.h"

//CDatabase DBConn;
//CLog theLog;

//2013-07-05 增加sql更新落地当系统处于维护态
//2013-07-08 增加容灾平台处理,主系统处理完一个文件后发送同步变量给备份系统
//2013-07-31 D_SCH_LOAD加file_id
//2013-08-16 新增可以配置每次扫描数据源目录下面指定个数文件后调到下个数据源,增加获取pertri网状态
//2013-08-24 增加容灾平台,将数据库的更新操作写文件(调用父类接口)
//2013-08-30 去除容灾平台
//2013-09-02 增加容灾平台,sql写文件,备份不按照时间来备份,备份路径不存在自行创建
//2013-11-04 增加向调度发送数据库状态切换状态的功能,备系统根据主系统的状态,R时先切主系统,W先切备系统
//2013-12-01 仲裁发生在申请话单块后,状态置-1
//2013-12-06 由于vector不能释放内存,目前采用指针形式
//2013-12-09 sql只写insert,不update

//CReadIni theCfgFile;
CLog theJSLog;

int main(int argc,char** argv)
{
	cout<<"********************************************* "<<endl;
	cout<<"*    China Telecom. Telephone Network         "<<endl;
	cout<<"*    InterNational Account Settle System      "<<endl;
	cout<<"*                                             "<<endl;
	cout<<"*           jsload							 "<<endl;
	cout<<"*           sys.GJZW.Version 1.0	             "<<endl;
	cout<<"*     created time :      2013-07-01 by  hed  "<<endl;
	cout<<"*     last update time :  2013-12-09 by  hed	 "<<endl;
	cout<<"**********************************************"<<endl;



	FileLoad load ;
	if(!load.init(argc,argv)) return -1;

	load.run();
	
    return 0;

}

