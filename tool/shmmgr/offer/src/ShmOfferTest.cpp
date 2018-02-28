/****************************************************************************
 * Copyright (c) 2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
 * All rights reserved.
 *
 * Created:      2012/2/9 11:08:02
 * Filename:     hbbaseinfo.cpp
 * Project:      2.8
 * Author:       hychang
 * Description:  加载基础资料的命令集1
 * Revision:     $Id: ShmOfferTest.cpp 1898 2012-10-31 07:13:00Z huangyb $
 *
 ****************************************************************************/
 
#include "ShmOfferMgr.h" 

using namespace ns_shm_data;

class offerinfo:public Application
{
public:
	/**
	 * 构造函数
	 */
	offerinfo();

	/**
	 * 析构函数
	 */
	virtual ~offerinfo();

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
offerinfo::offerinfo()
{
    
}

/**
 * 析构函数
 */
offerinfo::~offerinfo()
{
    //delete _theLog;
}

string offerinfo::getUsage() const {
    return 
    "psoffermgr -create 创建销售品资料共享内存\n"
    "psoffermgr -load   加载销售品资料到共享内存\n";
}

/**
 * 获取程序允许的参数字符串
 */
string offerinfo::getOptionString() const {
    return "";
}

/**
 * 获取是否以一个短横线表示长选项, 不存在复合短选项
 * 默认为false, 标准的长选项以一对短横线开始, 一个短横线后面跟的是短选项或复合短选项
*/
bool offerinfo::hasOptionLongOnly() const
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
int offerinfo::run(int argc, char** argv) 
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
	
    ns_shm_data::COfferManager offermanager;
    if(hasOption("create",args))
    {       		
		if(!offermanager.createShm())
		{
		    theLog<<"销售品基础资料共享内存创建失败"<<ende;
		}
		theLog<<"销售品基础资料共享内存创建成功"<<endi;
		return 0;
    }
    else if(hasOption("load",args))
    {
        if(!offermanager.adminCreateOfferManager())
		{
			theLog<<"预处理销售品新接口出错"<<ende;
			return 1;
		}
		return 0;
    }	

    return 0;
}


/**
 * 程序入口声明
 */
APPLICATION_DECLARE(offerinfo)




