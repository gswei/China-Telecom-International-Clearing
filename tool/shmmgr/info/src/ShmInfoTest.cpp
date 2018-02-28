/****************************************************************************
 * Copyright (c) 2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
 * All rights reserved.
 *
 * Created:      2012/2/9 11:08:02
 * Filename:     hbbaseinfo.cpp
 * Project:      2.8
 * Author:       hychang
 * Description:  加载基础资料的命令集
 * Revision:     $Id: ShmInfoTest.cpp 1898 2012-10-31 07:13:00Z huangyb $
 *
 ****************************************************************************/
 
#include "ShmBillingInfo.h"
#include "ShmInfoMgr.h" 

using namespace ns_shm_data;

class baseinfo:public Application
{
public:
	/**
	 * 构造函数
	 */
	baseinfo();

	/**
	 * 析构函数
	 */
	virtual ~baseinfo();

	virtual string getUsage() const;	

	/**
	 * 参数校验
	 */
	virtual string getOptionString() const;
	/**
	 * 主程序逻辑
	 */
	virtual int run(int argc, char** argv);
	
	/**
     * 获取是否以一个短横线表示长选项, 不存在复合短选项
     * 默认为false, 标准的长选项以一对短横线开始, 一个短横线后面跟的是短选项或复合短选项
     */
	virtual bool hasOptionLongOnly() const;
		
};

/**
 * 构造函数
 */
baseinfo::baseinfo()
{
    
}

/**
 * 析构函数
 */
baseinfo::~baseinfo()
{
    //delete _shmLog;
}

string baseinfo::getUsage() const {
    return 
    "psinfomgr -create 创建用户资料共享内存\n"
    "psinfomgr -load   加载用户资料到共享内存\n";
}

/**
 * 获取程序允许的参数字符串
 */
string baseinfo::getOptionString() const {
    return "";
}

/**
 * 获取是否以一个短横线表示长选项, 不存在复合短选项
 * 默认为false, 标准的长选项以一对短横线开始, 一个短横线后面跟的是短选项或复合短选项
*/
bool baseinfo::hasOptionLongOnly() const
{
    return true;
} 

void onsignal(int no) 
{
	//theLog<<HbError("HBER_PROC_RECIVE_QUIT","信号: "+es::str(no),"").what()<<endi;
	exit(0);
}

/**
 * 主处理逻辑
 */
int baseinfo::run(int argc, char** argv) 
{ 
	if(!InitActiveApp())exit(1);
	
    string pattern = "create,0,0|load,0,0|h,0,0";

	if(!CheckOption::check(argc,argv,pattern)) {
		cout<<getUsage()<<endl;
		return 1;
	}
	ArgList args;
	if(hasOption("h",args))
    {
    	cout<<getUsage()<<endl;
		return 1;
	}

	theLog << "CheckOption check succ!" << endi;

    signal(SIGTERM,onsignal);
    signal(SIGUSR1,onsignal);
    signal(SIGUSR2,onsignal);
	
	createConnection();

	ns_shm_data::CBillingBaseInfoMgr infomgr;
    if(hasOption("load",args))
    {       		
		theLog << "Attach " << endi;
		if(!infomgr.Attach(false))
		{
			if(!infomgr.CreateShm())
			{
				theLog << "创建共享内存失败" << ende;
				return 1;
			}
			if(!infomgr.Attach(false))return 1;
		}
		
		theLog << "LoadFromDB " << endi;
		if(!infomgr.LoadFromDB())
		{
			theLog << "加载共享内存失败" << ende;
			return 1;
		}
		theLog << "Detach " << endi;
		if(!infomgr.Detach())
		{
			theLog << "断开共享内存失败" << ende;
			return 1;
		}	 
		return 0;
    }
    else if(hasOption("create",args))
    {
        theLog << "CreateShm " << endi;
		if(!infomgr.CreateShm())
		{
			theLog << "创建共享内存失败" << ende;
			return 1;
		}
		return 0;
    }		

    return 0;
}


/**
 * 程序入口声明
 */
APPLICATION_DECLARE(baseinfo)



