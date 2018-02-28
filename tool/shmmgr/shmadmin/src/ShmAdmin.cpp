#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include "ShmOfferMgr.h"
#include "ShmInfoMgr.h"
#include "CErrorCode.h"
using namespace ns_shm_data;
using namespace ERRCODE;

string MUIInput(char const * prompt,char const * defaultvalue)
{
	char line[10240];
	cout<<prompt<<"(q=exit "<<(strlen(defaultvalue)!=0?"default=":"")<<defaultvalue<<")��"<<endl;
	cin.getline(line,10240);
	if('q'==line[0])exit(0);
	if(0==strlen(line))strcpy(line,defaultvalue);
	theLog<<prompt<<" �û��������:"<<line<<endi;
	return line;
}


bool GetCommandParam(int argc, char **argv,char const * key,string & value)
{
	int i;
	for(i=1;i<argc;++i)
	{
		if(0==strcmp(key,argv[i]))
		{
			if(i+1<argc && argv[i+1][0]!='-')
			{
				value=argv[i+1];
				return true;
			}
		}
	}
	return false;
}

//�źŵ�����
char const * sigstr(long sig)
{
	switch(sig)
	{
	case SIGABRT    :    return "SIGABRT    ���̵���abort���������̷������˳�";
	case SIGALRM    :    return "SIGALRM    ��alarm�������õ�timer��ʱ��setitimer�������õ�interval timer��ʱ";
	case SIGBUS     :    return "SIGBUS     ĳ���ض���Ӳ���쳣��ͨ�����ڴ��������";
	case SIGCHLD    :    return "SIGCHLD    �ӽ���Terminate��Stop";
	case SIGCONT    :    return "SIGCONT    ��stop���лָ�����";
	case SIGEMT     :    return "SIGEMT     ��ʵ����ص�Ӳ���쳣";
	case SIGFPE     :    return "SIGFPE     ��ѧ��ص��쳣���类0��������������ȵ�";
	case SIGHUP     :    return "SIGHUP     �ն˶Ͽ�";
	case SIGILL     :    return "SIGILL     �Ƿ�ָ���쳣";
		//case SIGINFO    :    return "SIGINFO    BSD signal����Status Key������ͨ����CTRL+T�����͸�����Foreground Group�Ľ���     ";
	case SIGINT     :    return "SIGINT     ��Interrupt Key������ͨ����CTRL+C����DELETE";
	case SIGIO      :    return "SIGIO      �첽IO�¼�";
		//case SIGIOT     :    return "SIGIOT     ʵ����ص�Ӳ���쳣��һ���ӦSIGABRT                                              ";
	case SIGKILL    :    return "SIGKILL    ǿ����ֹ";
	case SIGPIPE    :    return "SIGPIPE    ��reader��ֹ֮��дPipe��ʱ����";
		//case SIGPOLL    :    return "SIGPOLL    ��ĳ���¼����͸�Pollable Device��ʱ����                                        ";
	case SIGPROF    :    return "SIGPROF    Setitimerָ����Profiling Interval Timer������";
	case SIGPWR     :    return "SIGPWR     ��ϵͳ��ء���UPS��ء�";
	case SIGQUIT    :    return "SIGQUIT    ����Quit Key��CTRL+\\��";
	case SIGSEGV    :    return "SIGSEGV    �Ƿ��ڴ����";
	case SIGSTOP    :    return "SIGSTOP    ��ֹ����";
	case SIGSYS     :    return "SIGSYS     �Ƿ�ϵͳ����";
	case SIGTERM    :    return "SIGTERM    ������ֹ���̣�kill����ȱʡ����";
	case SIGTRAP    :    return "SIGTRAP    ʵ����ص�Ӳ���쳣��һ���ǵ����쳣";
	case SIGTSTP    :    return "SIGTSTP    Suspend Key��һ����Ctrl+Z";
	case SIGTTIN    :    return "SIGTTIN    ��Background Group�Ľ��̳��Զ�ȡTerminal��ʱ����";
	case SIGTTOU    :    return "SIGTTOU    ��Background Group�Ľ��̳���дTerminal��ʱ����";
	case SIGURG     :    return "SIGURG     ��out-of-band data���յ�ʱ����ܷ���";
	case SIGUSR1    :    return "SIGUSR1    �û��Զ���signal 1";
	case SIGUSR2    :    return "SIGUSR2    �û��Զ���signal 2";
	case SIGVTALRM  :    return "SIGVTALRM  setitimer�������õ�Virtual Interval Timer��ʱ��ʱ��";
	case SIGWINCH   :    return "SIGWINCH   ��Terminal�Ĵ��ڴ�С�ı��ʱ�򣬷��͸�Foreground Group�����н���";
	case SIGXCPU    :    return "SIGXCPU    ��CPUʱ�����Ƴ�ʱ��ʱ��";
	case SIGXFSZ    :    return "SIGXFSZ    ���̳����ļ���С����";
	default:
		static char buf[256];
		sprintf(buf,"δ֪���ź� %ld",sig);
		return buf;
	}
}

extern "C" void sig_default(int sig)
{
	theLog<<sigstr(sig)<<endi;
}

int MakeOfferShmData(int argc,char ** argv)
{
	if(MUIInput("��������Ʒ�����ڴ��ļ��������ص������ڴ棩��������y/n��","n")!="y")return 1;
	int ret=0;

	ns_shm_data::CShmOfferInfoMgr offermanager;

	if(!offermanager.adminCreateOfferManager())return __LINE__;
	return ret;
}

int queryOfferShmServ(int argc,char ** argv)
{
	int ret=0;

	ns_shm_data::CShmOfferInfoMgr offermanager;

	if(!offermanager.AttachToSHM())return __LINE__;
	string id=MUIInput("������SERV_ID��","1");
	string str;
	
	if(!offermanager.DetachFromSHM())return __LINE__;
	return ret;
}

int queryOfferShmMsinfo(int argc,char ** argv)
{
	int ret=0;

	ns_shm_data::CShmOfferInfoMgr offermanager;

	if(0!=offermanager.init())return __LINE__;

	while(true)
	{
		string id=MUIInput("������MSINFO_ID��","1");
		string str;
		MOfferIns	oi;
		bool	bFind;

		if("q"==id)break;

		bFind = offermanager.getInsByMsinfo(atol(id.c_str()), oi);
		if(!bFind){
			theLog<<"not found"<<endi;
		} else {
			theLog << "oi.msinfo_id: " << oi.msinfo_id<< endi;
			theLog << "oi.offer_id: " << oi.offer_id<< endi;
			theLog << "oi.eff_date: " << oi.eff_date<< endi;
			theLog << "oi.cust_id: " << oi.cust_id<< endi;
			theLog << "oi.cust_agreement_id: " << oi.cust_agreement_id<< endi;

			ListDetails details;
			bFind = offermanager.getDetailsByMsinfo(atol(id.c_str()),details);
			if(!bFind){
				theLog<<"item not found"<<endi;
			} else {
				list<detail>::iterator	it;
				for(it=details.begin();it != details.end();it++){
					theLog << "it->serv_id: " << it->serv_id<< endi;
					theLog << "it->eff_date: " << it->eff_date<< endi;
					theLog << "it->exp_date: " << it->exp_date<< endi;
				}
			}
		}
	};
	
	if(!offermanager.uninit()) return __LINE__;
	
	return ret;
}

int showOfferShm(int argc,char ** argv)
{
	int ret=0;

	ns_shm_data::CShmOfferInfoMgr offermanager;

	if(!offermanager.AttachToSHM())	return __LINE__;
	offermanager.Report();
	if(!offermanager.DetachFromSHM())	return __LINE__;
	return ret;
}

int createOfferShm(int argc,char ** argv)
{
	int ret=0;

	ns_shm_data::CShmOfferInfoMgr offermanager;

	if(!offermanager.createShm())return __LINE__;
	return ret;
}


int create_CBillingBaseInfoMgr(int argc,char ** argv)
{
	if(MUIInput("�ؽ��������Ϲ����ڴ潫ʧȥ�������ݣ�������y/n��","n")!="y")return 1;
	ns_shm_data::CBillingBaseInfoMgr a;

	if(!a.CreateShm())return __LINE__;
	return 0;
}

int load_from_db_CBillingBaseInfoMgr(int argc,char ** argv)
{
	ns_shm_data::CBillingBaseInfoMgr a;

	if(!a.Attach(false))return __LINE__;
	//if(!a.Report())return __LINE__;
	if(!a.LoadFromDB())return __LINE__;
	//if(!a.Report())return __LINE__;

	if(!a.Detach())return __LINE__;
	return 0;
}


int show_CBillingBaseInfoMgr(int argc,char ** argv)
{
	ns_shm_data::CBillingBaseInfoMgr a;

	if(!a.Attach(false))return __LINE__;
	if(!a.Report())return __LINE__;
	if(!a.Detach())return __LINE__;
	return 0;
}

int BaseInfoQuery(int argc,char ** argv)
{
	CShmInfoMgr * p=CShmInfoMgr::instance();
	if(0!=p->Init())return __LINE__;

	CBillingBaseInfoMgr::BaseInfo_serv const *serv=NULL;
	CBillingBaseInfoMgr::BaseInfo_cust const *cust=NULL;
	CBillingBaseInfoMgr::BaseInfo_nbr const *nbr=NULL;

	string	str;

	while(true)
	{
		str=MUIInput("������serv_id��","5137676");
		serv = p->GetServInfo(atol(str.c_str()));
		if(serv == NULL)
		{
			theLog<<"not found"<<endi;
		}
		else
		{
			theLog << "serv.agreement_id: " << serv->agreement_id << endi;
			theLog << "serv.product_id: " << serv->product_id << endi;
			theLog << "serv.product_family_id: " << serv->product_family_id << endi;
			theLog << "serv.create_date: " << serv->create_date.c_str() << endi;
			theLog << "serv.rent_date: " << serv->rent_date.c_str() << endi;
			theLog << "serv.completed_date: " << serv->completed_date.c_str() << endi;
			theLog << "serv.state: " << serv->state.c_str() << endi;
			theLog << "serv.cust_id: " << serv->cust_id << endi;
			theLog << "serv.serv_id: " << serv->serv_id << endi;

			cust = p->GetCustInfo(serv->cust_id);
			if(cust == NULL){
				theLog<<"cust not found"<<endi;
			} else {
				theLog << "cust.cust_id: " << cust->cust_id << endi;
				theLog << "cust.user_type_id: " << cust->user_type_id.c_str() << endi;
			}

			nbr = p->GetNbrInfo(serv->serv_id);
			if(nbr == NULL){
				theLog<<"nbr not found"<<endi;
			} else {
				theLog << "nbr.serv_id: " << nbr->serv_id << endi;
				theLog << "nbr.acc_nbr: " << nbr->acc_nbr.c_str() << endi;
			}
		}
	}

	return 0;
}

int BaseInfoUpdate(int argc,char ** argv)
{
	CShmInfoMgr * p=CShmInfoMgr::instance();
	if(0!=p->Init())return __LINE__;

	CBillingBaseInfoMgr::BaseInfo_serv const *serv=NULL;
	CBillingBaseInfoMgr::BaseInfo_cust const *cust=NULL;
	CBillingBaseInfoMgr::BaseInfo_nbr const *nbr=NULL;

	CBillingBaseInfoMgr::BaseInfo_serv servu;

	string	str;

	str=MUIInput("������serv_id��","5137676");
	serv = p->GetServInfo(atol(str.c_str()));
	if(serv == NULL)
	{
		theLog<<"not found"<<endi;
		return -1;
	}
	else
	{
		theLog << "serv.agreement_id: " << serv->agreement_id << endi;
		theLog << "serv.product_id: " << serv->product_id << endi;
		theLog << "serv.product_family_id: " << serv->product_family_id << endi;
		theLog << "serv.create_date: " << serv->create_date.c_str() << endi;
		theLog << "serv.rent_date: " << serv->rent_date.c_str() << endi;
		theLog << "serv.completed_date: " << serv->completed_date.c_str() << endi;
		theLog << "serv.state: " << serv->state.c_str() << endi;
		theLog << "serv.cust_id: " << serv->cust_id << endi;
		theLog << "serv.serv_id: " << serv->serv_id << endi;

		servu.agreement_id = serv->agreement_id;
		servu.product_id = serv->product_id;
		servu.product_family_id = serv->product_family_id;
		servu.create_date = serv->create_date;
		servu.rent_date = serv->rent_date;
		servu.completed_date = serv->completed_date;
		servu.state = serv->state;
		servu.cust_id = serv->cust_id;
		servu.serv_id = serv->serv_id;

		/*
		cust = p->GetCustInfo(serv->cust_id);
		if(cust == NULL){
			theLog<<"cust not found"<<endi;
		} else {
			theLog << "cust.cust_id: " << cust->cust_id << endi;
			theLog << "cust.user_type_id: " << cust->user_type_id.c_str() << endi;
		}

		nbr = p->GetNbrInfo(serv->serv_id);
		if(nbr == NULL){
			theLog<<"nbr not found"<<endi;
		} else {
			theLog << "nbr.serv_id: " << nbr->serv_id << endi;
			theLog << "nbr.acc_nbr: " << nbr->acc_nbr.c_str() << endi;
		}
		*/
	}

	string	yy, str2;
	yy=UIInput("�Ƿ�Ҫ�޸�agreement_id��","N");
	if(yy == "Y"){
		str2=UIInput("�������µ�agreement_id��",-1);
		if(0==str2.size())
		{
			writelog(ERRCODE::LOG_CODE_PE_SHM_SET_ERR,"agreement_idΪ��");
			return -1;
		}
		servu.agreement_id = atol(str2.c_str());
		cout << "servu.agreement_id: " << servu.agreement_id << endl;
	}
	yy=UIInput("�Ƿ�Ҫ�޸�product_id��","N");
	if(yy == "Y"){
		str2=UIInput("�������µ�product_id��",-1);
		if(0==str2.size())
		{
			writelog(ERRCODE::LOG_CODE_PE_SHM_SET_ERR,"product_idΪ��");
			return -1;
		}
		servu.product_id = atol(str2.c_str());
		cout << "servu.product_id: " << servu.product_id << endl;
	}
	yy=UIInput("�Ƿ�Ҫ�޸�product_family_id��","N");
	if(yy == "Y"){
		str2=UIInput("�������µ�product_family_id��",-1);
		servu.product_family_id = atol(str2.c_str());
		cout << "servu.product_family_id: " << servu.product_family_id << endl;
	}
	yy=UIInput("�Ƿ�Ҫ�޸�create_date��","N");
	if(yy == "Y"){
		str2=UIInput("�������µ�create_date(YYYYMMDDHH24MISS)��",-1);
		if(str2.size() && (14!=str2.size()))
		{
			writelog(ERRCODE::LOG_CODE_PE_SHM_SET_ERR,"create_date����ĳ���,����Ϊ14λ");
			return -1;
		}
		servu.create_date = str2;
		cout << "servu.create_date: " << servu.create_date.c_str() << endl;
	}
	yy=UIInput("�Ƿ�Ҫ�޸�rent_date��","N");
	if(yy == "Y"){
		str2=UIInput("�������µ�rent_date��",-1);
		if(0==str2.size())
		{
			writelog(ERRCODE::LOG_CODE_PE_SHM_SET_ERR,"rent_dateΪ��");
			return -1;
		}
		if(14!=str2.size())
		{
			writelog(ERRCODE::LOG_CODE_PE_SHM_SET_ERR,"rent_date����ĳ���,����Ϊ14λ");
			return -1;
		}
		servu.rent_date = str2;
		cout << "servu.rent_date: " << servu.rent_date.c_str() << endl;
	}
	yy=UIInput("�Ƿ�Ҫ�޸�completed_date��","N");
	if(yy == "Y"){
		str2=UIInput("�������µ�completed_date��",-1);
		if(0==str2.size())
		{
			writelog(ERRCODE::LOG_CODE_PE_SHM_SET_ERR,"completed_dateΪ��");
			return -1;
		}
		if(14!=str2.size())
		{
			writelog(ERRCODE::LOG_CODE_PE_SHM_SET_ERR,"completed_date����ĳ���,����Ϊ14λ");
			return -1;
		}
		servu.completed_date = str2;
		cout << "servu.completed_date: " << servu.completed_date.c_str() << endl;
	}
	yy=UIInput("�Ƿ�Ҫ�޸�state��","N");
	if(yy == "Y"){
		str2=UIInput("�������µ�state��",-1);
		if(0==str2.size())
		{
			writelog(ERRCODE::LOG_CODE_PE_SHM_SET_ERR,"stateΪ��");
			return -1;
		}
		if(3!=str2.size())
		{
			writelog(ERRCODE::LOG_CODE_PE_SHM_SET_ERR,"state����ĳ���,����Ϊ3λ");
			return -1;
		}
		servu.state = str2;
		cout << "servu.state: " << servu.state.c_str() << endl;
	}
	yy=UIInput("�Ƿ�Ҫ�޸�cust_id��","N");
	if(yy == "Y"){
		str2=UIInput("�������µ�cust_id��",-1);
		if(0==str2.size())
		{
			writelog(ERRCODE::LOG_CODE_PE_SHM_SET_ERR,"cust_idΪ��");
			return -1;
		}
		servu.cust_id = atol(str2.c_str());
		cout << "servu.cust_id: " << servu.cust_id << endl;
	}

	if(!p->UpdateServInfo(servu)){
		theLog<<"������Ϣʧ��"<<ende;	
	} else {
		theLog<<"������Ϣ�ɹ���"<<endi;
	}
	
	/*
	string str2=UIInput("�������µ�״̬��",-1);
	if(0==str2.size())
	{
		theLog<<"�û�ȡ��"<<endi;
	}
	else if(3!=str2.size())
	{
		theLog<<"����ĳ���,����Ϊ3λ"<<ende;
	}
	else
	{
		servu.state = str2;
		if(!p->UpdateServInfo(servu)){
			theLog<<"����״̬ʧ��"<<ende;	
		} else {
			theLog<<"���º��״̬��"<<str2<<endi;
		}
	}
	*/
	
	if(0!=p->Detach())return __LINE__;
	return 0;
}

int main_fun(int argc,char ** argv)
{
	cout << "main_fun !" << endl;

	bool loop=true;
	while(loop)
	{
		cout << "while !" << endl;
		int ret=0;
		string cmd;
        
		//��ʾȫ������
		theLog<<"ȫ�����ã���ͨ���������ã�" <<endi;

		if(GetCommandParam(argc,argv,"-cmd",cmd))
		{
			loop=false;
		}
		else
		{
			cout<<"\n"<<"----------------------------------------"<<"\n"<<"�������q=exit��"<<"\n"
				<<"........................................"<<"\n"
				<<"1 show_COfferMgr ��ʾ�»������Ϲ����ڴ�"<<'\n'
				<<"2 create_COfferMgr ��ʾ�»������Ϲ����ڴ�"<<'\n'
				<<"3 load_COfferMgr ��ʾ�»������Ϲ����ڴ�"<<'\n'
				<<"4 find_COfferMgr(serv_id) ��ʾ�»������Ϲ����ڴ�"<<'\n'
				<<"5 find_COfferMgr(msinfo_id) ��ʾ�»������Ϲ����ڴ�"<<'\n'
				<<"10 show_CBillingBaseInfoMgr ��ʾ�»������Ϲ����ڴ�"<<'\n'
				<<"11 create_CBillingBaseInfoMgr �ؽ��»������Ϲ����ڴ棨���������ݣ�"<<'\n'
				<<"12 load_from_db_CBillingBaseInfoMgr �����»������Ϲ����ڴ� "<<'\n'
				<<"13 find_CBillingBaseInfoMgr ��ѯ�������Ϲ����ڴ� "<<'\n'
				<<"14 update_CBillingBaseInfoMgr ���»������Ϲ����ڴ� "<<'\n'
				<<"........................................"<<'\n'
				<<"----------------------------------------"<<endl;
			cmd=MUIInput("��ѡ�����","0");
			if(cmd=="q")break;
		}
		long nCmd=atol(cmd.c_str());
		switch(nCmd)
		{
		case 1:
			ret=showOfferShm(argc,argv);
			break;
		case 2:
			ret=createOfferShm(argc,argv);
			break;
		case 3:
			ret=MakeOfferShmData(argc,argv);
			break;
		case 4:
			ret=queryOfferShmServ(argc,argv);
			break;
		case 5:
			ret=queryOfferShmMsinfo(argc,argv);
			break;
		case 10:
			ret=show_CBillingBaseInfoMgr(argc,argv);
			break;
		case 11:
			ret=create_CBillingBaseInfoMgr(argc,argv);
			break;
		case 12:
			ret=load_from_db_CBillingBaseInfoMgr(argc,argv);
			break;
		case 13:
			ret=BaseInfoQuery(argc,argv);
			break;
		case 14:
			ret=BaseInfoUpdate(argc,argv);
			break;	
		default:
			cout<<"��Ч�����"<<cmd<<"("<<nCmd<<")"<<endl;
			break;
		}
		theLog<<"���� "<<nCmd<<" ���� "<<ret<<endi;
		MUIInput("Press any key to continue ...","y");
	}

	return 0;
}

class ShmAdmin:public Application
{
public:
	/**
	 * ���캯��
	 */
	ShmAdmin(){};

	/**
	 * ��������
	 */
	virtual ~ShmAdmin(){};

	/**
	 * ��ȡ����
	 */
	virtual string getName() const{return "TestMain";};

	/**
	 * ��ȡ����ʹ�÷���
	 */
	virtual string getUsage() const{return "";};
	/**
	 * ����У��
	 */
	virtual string getOptionString() const{return "";};
	/**
	 * �������߼�
	 */
	virtual int run(int argc, char** argv)
	{
		if(!InitActiveApp())exit(1);
	    main_fun(argc,argv);
	    return 0;
	}	
};

/**
 * �����������
 */
APPLICATION_DECLARE(ShmAdmin)


