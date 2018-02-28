/****************************************************************************
 * Copyright (c) 2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
 * All rights reserved.
 *
 * Created:      2012/2/9 11:08:02
 * Filename:     hbbaseinfo.cpp
 * Project:      2.8
 * Author:       hychang
 * Description:  ���ػ������ϵ����
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
	 * ���캯��
	 */
	baseinfo();

	/**
	 * ��������
	 */
	virtual ~baseinfo();

	virtual string getUsage() const;	

	/**
	 * ����У��
	 */
	virtual string getOptionString() const;
	/**
	 * �������߼�
	 */
	virtual int run(int argc, char** argv);
	
	/**
     * ��ȡ�Ƿ���һ���̺��߱�ʾ��ѡ��, �����ڸ��϶�ѡ��
     * Ĭ��Ϊfalse, ��׼�ĳ�ѡ����һ�Զ̺��߿�ʼ, һ���̺��ߺ�������Ƕ�ѡ��򸴺϶�ѡ��
     */
	virtual bool hasOptionLongOnly() const;
		
};

/**
 * ���캯��
 */
baseinfo::baseinfo()
{
    
}

/**
 * ��������
 */
baseinfo::~baseinfo()
{
    //delete _shmLog;
}

string baseinfo::getUsage() const {
    return 
    "psinfomgr -create �����û����Ϲ����ڴ�\n"
    "psinfomgr -load   �����û����ϵ������ڴ�\n";
}

/**
 * ��ȡ��������Ĳ����ַ���
 */
string baseinfo::getOptionString() const {
    return "";
}

/**
 * ��ȡ�Ƿ���һ���̺��߱�ʾ��ѡ��, �����ڸ��϶�ѡ��
 * Ĭ��Ϊfalse, ��׼�ĳ�ѡ����һ�Զ̺��߿�ʼ, һ���̺��ߺ�������Ƕ�ѡ��򸴺϶�ѡ��
*/
bool baseinfo::hasOptionLongOnly() const
{
    return true;
} 

void onsignal(int no) 
{
	//theLog<<HbError("HBER_PROC_RECIVE_QUIT","�ź�: "+es::str(no),"").what()<<endi;
	exit(0);
}

/**
 * �������߼�
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
				theLog << "���������ڴ�ʧ��" << ende;
				return 1;
			}
			if(!infomgr.Attach(false))return 1;
		}
		
		theLog << "LoadFromDB " << endi;
		if(!infomgr.LoadFromDB())
		{
			theLog << "���ع����ڴ�ʧ��" << ende;
			return 1;
		}
		theLog << "Detach " << endi;
		if(!infomgr.Detach())
		{
			theLog << "�Ͽ������ڴ�ʧ��" << ende;
			return 1;
		}	 
		return 0;
    }
    else if(hasOption("create",args))
    {
        theLog << "CreateShm " << endi;
		if(!infomgr.CreateShm())
		{
			theLog << "���������ڴ�ʧ��" << ende;
			return 1;
		}
		return 0;
    }		

    return 0;
}


/**
 * �����������
 */
APPLICATION_DECLARE(baseinfo)



