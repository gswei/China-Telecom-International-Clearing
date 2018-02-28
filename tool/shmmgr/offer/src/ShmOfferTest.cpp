/****************************************************************************
 * Copyright (c) 2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
 * All rights reserved.
 *
 * Created:      2012/2/9 11:08:02
 * Filename:     hbbaseinfo.cpp
 * Project:      2.8
 * Author:       hychang
 * Description:  ���ػ������ϵ����1
 * Revision:     $Id: ShmOfferTest.cpp 1898 2012-10-31 07:13:00Z huangyb $
 *
 ****************************************************************************/
 
#include "ShmOfferMgr.h" 

using namespace ns_shm_data;

class offerinfo:public Application
{
public:
	/**
	 * ���캯��
	 */
	offerinfo();

	/**
	 * ��������
	 */
	virtual ~offerinfo();

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
offerinfo::offerinfo()
{
    
}

/**
 * ��������
 */
offerinfo::~offerinfo()
{
    //delete _theLog;
}

string offerinfo::getUsage() const {
    return 
    "psoffermgr -create ��������Ʒ���Ϲ����ڴ�\n"
    "psoffermgr -load   ��������Ʒ���ϵ������ڴ�\n";
}

/**
 * ��ȡ��������Ĳ����ַ���
 */
string offerinfo::getOptionString() const {
    return "";
}

/**
 * ��ȡ�Ƿ���һ���̺��߱�ʾ��ѡ��, �����ڸ��϶�ѡ��
 * Ĭ��Ϊfalse, ��׼�ĳ�ѡ����һ�Զ̺��߿�ʼ, һ���̺��ߺ�������Ƕ�ѡ��򸴺϶�ѡ��
*/
bool offerinfo::hasOptionLongOnly() const
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
		    theLog<<"����Ʒ�������Ϲ����ڴ洴��ʧ��"<<ende;
		}
		theLog<<"����Ʒ�������Ϲ����ڴ洴���ɹ�"<<endi;
		return 0;
    }
    else if(hasOption("load",args))
    {
        if(!offermanager.adminCreateOfferManager())
		{
			theLog<<"Ԥ��������Ʒ�½ӿڳ���"<<ende;
			return 1;
		}
		return 0;
    }	

    return 0;
}


/**
 * �����������
 */
APPLICATION_DECLARE(offerinfo)




