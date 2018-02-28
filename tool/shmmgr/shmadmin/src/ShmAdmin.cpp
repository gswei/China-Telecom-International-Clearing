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
	cout<<prompt<<"(q=exit "<<(strlen(defaultvalue)!=0?"default=":"")<<defaultvalue<<")："<<endl;
	cin.getline(line,10240);
	if('q'==line[0])exit(0);
	if(0==strlen(line))strcpy(line,defaultvalue);
	theLog<<prompt<<" 用户输入的是:"<<line<<endi;
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

//信号的描述
char const * sigstr(long sig)
{
	switch(sig)
	{
	case SIGABRT    :    return "SIGABRT    进程调用abort函数，进程非正常退出";
	case SIGALRM    :    return "SIGALRM    用alarm函数设置的timer超时或setitimer函数设置的interval timer超时";
	case SIGBUS     :    return "SIGBUS     某种特定的硬件异常，通常由内存访问引起";
	case SIGCHLD    :    return "SIGCHLD    子进程Terminate或Stop";
	case SIGCONT    :    return "SIGCONT    从stop的中恢复运行";
	case SIGEMT     :    return "SIGEMT     和实现相关的硬件异常";
	case SIGFPE     :    return "SIGFPE     数学相关的异常，如被0除，浮点溢出，等等";
	case SIGHUP     :    return "SIGHUP     终端断开";
	case SIGILL     :    return "SIGILL     非法指令异常";
		//case SIGINFO    :    return "SIGINFO    BSD signal。由Status Key产生，通常是CTRL+T。发送给所有Foreground Group的进程     ";
	case SIGINT     :    return "SIGINT     由Interrupt Key产生，通常是CTRL+C或者DELETE";
	case SIGIO      :    return "SIGIO      异步IO事件";
		//case SIGIOT     :    return "SIGIOT     实现相关的硬件异常，一般对应SIGABRT                                              ";
	case SIGKILL    :    return "SIGKILL    强制中止";
	case SIGPIPE    :    return "SIGPIPE    在reader中止之后写Pipe的时候发送";
		//case SIGPOLL    :    return "SIGPOLL    当某个事件发送给Pollable Device的时候发送                                        ";
	case SIGPROF    :    return "SIGPROF    Setitimer指定的Profiling Interval Timer所产生";
	case SIGPWR     :    return "SIGPWR     和系统相关。和UPS相关。";
	case SIGQUIT    :    return "SIGQUIT    输入Quit Key（CTRL+\\）";
	case SIGSEGV    :    return "SIGSEGV    非法内存访问";
	case SIGSTOP    :    return "SIGSTOP    中止进程";
	case SIGSYS     :    return "SIGSYS     非法系统调用";
	case SIGTERM    :    return "SIGTERM    请求中止进程，kill命令缺省发送";
	case SIGTRAP    :    return "SIGTRAP    实现相关的硬件异常。一般是调试异常";
	case SIGTSTP    :    return "SIGTSTP    Suspend Key，一般是Ctrl+Z";
	case SIGTTIN    :    return "SIGTTIN    当Background Group的进程尝试读取Terminal的时候发送";
	case SIGTTOU    :    return "SIGTTOU    当Background Group的进程尝试写Terminal的时候发送";
	case SIGURG     :    return "SIGURG     当out-of-band data接收的时候可能发送";
	case SIGUSR1    :    return "SIGUSR1    用户自定义signal 1";
	case SIGUSR2    :    return "SIGUSR2    用户自定义signal 2";
	case SIGVTALRM  :    return "SIGVTALRM  setitimer函数设置的Virtual Interval Timer超时的时候";
	case SIGWINCH   :    return "SIGWINCH   当Terminal的窗口大小改变的时候，发送给Foreground Group的所有进程";
	case SIGXCPU    :    return "SIGXCPU    当CPU时间限制超时的时候";
	case SIGXFSZ    :    return "SIGXFSZ    进程超过文件大小限制";
	default:
		static char buf[256];
		sprintf(buf,"未知的信号 %ld",sig);
		return buf;
	}
}

extern "C" void sig_default(int sig)
{
	theLog<<sigstr(sig)<<endi;
}

int MakeOfferShmData(int argc,char ** argv)
{
	if(MUIInput("构造销售品共享内存文件（不加载到共享内存），继续吗？y/n：","n")!="y")return 1;
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
	string id=MUIInput("请输入SERV_ID：","1");
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
		string id=MUIInput("请输入MSINFO_ID：","1");
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
	if(MUIInput("重建基本资料共享内存将失去所有数据，继续吗？y/n：","n")!="y")return 1;
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
		str=MUIInput("请输入serv_id：","5137676");
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

	str=MUIInput("请输入serv_id：","5137676");
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
	yy=UIInput("是否要修改agreement_id：","N");
	if(yy == "Y"){
		str2=UIInput("请输入新的agreement_id：",-1);
		if(0==str2.size())
		{
			writelog(ERRCODE::LOG_CODE_PE_SHM_SET_ERR,"agreement_id为空");
			return -1;
		}
		servu.agreement_id = atol(str2.c_str());
		cout << "servu.agreement_id: " << servu.agreement_id << endl;
	}
	yy=UIInput("是否要修改product_id：","N");
	if(yy == "Y"){
		str2=UIInput("请输入新的product_id：",-1);
		if(0==str2.size())
		{
			writelog(ERRCODE::LOG_CODE_PE_SHM_SET_ERR,"product_id为空");
			return -1;
		}
		servu.product_id = atol(str2.c_str());
		cout << "servu.product_id: " << servu.product_id << endl;
	}
	yy=UIInput("是否要修改product_family_id：","N");
	if(yy == "Y"){
		str2=UIInput("请输入新的product_family_id：",-1);
		servu.product_family_id = atol(str2.c_str());
		cout << "servu.product_family_id: " << servu.product_family_id << endl;
	}
	yy=UIInput("是否要修改create_date：","N");
	if(yy == "Y"){
		str2=UIInput("请输入新的create_date(YYYYMMDDHH24MISS)：",-1);
		if(str2.size() && (14!=str2.size()))
		{
			writelog(ERRCODE::LOG_CODE_PE_SHM_SET_ERR,"create_date错误的长度,必须为14位");
			return -1;
		}
		servu.create_date = str2;
		cout << "servu.create_date: " << servu.create_date.c_str() << endl;
	}
	yy=UIInput("是否要修改rent_date：","N");
	if(yy == "Y"){
		str2=UIInput("请输入新的rent_date：",-1);
		if(0==str2.size())
		{
			writelog(ERRCODE::LOG_CODE_PE_SHM_SET_ERR,"rent_date为空");
			return -1;
		}
		if(14!=str2.size())
		{
			writelog(ERRCODE::LOG_CODE_PE_SHM_SET_ERR,"rent_date错误的长度,必须为14位");
			return -1;
		}
		servu.rent_date = str2;
		cout << "servu.rent_date: " << servu.rent_date.c_str() << endl;
	}
	yy=UIInput("是否要修改completed_date：","N");
	if(yy == "Y"){
		str2=UIInput("请输入新的completed_date：",-1);
		if(0==str2.size())
		{
			writelog(ERRCODE::LOG_CODE_PE_SHM_SET_ERR,"completed_date为空");
			return -1;
		}
		if(14!=str2.size())
		{
			writelog(ERRCODE::LOG_CODE_PE_SHM_SET_ERR,"completed_date错误的长度,必须为14位");
			return -1;
		}
		servu.completed_date = str2;
		cout << "servu.completed_date: " << servu.completed_date.c_str() << endl;
	}
	yy=UIInput("是否要修改state：","N");
	if(yy == "Y"){
		str2=UIInput("请输入新的state：",-1);
		if(0==str2.size())
		{
			writelog(ERRCODE::LOG_CODE_PE_SHM_SET_ERR,"state为空");
			return -1;
		}
		if(3!=str2.size())
		{
			writelog(ERRCODE::LOG_CODE_PE_SHM_SET_ERR,"state错误的长度,必须为3位");
			return -1;
		}
		servu.state = str2;
		cout << "servu.state: " << servu.state.c_str() << endl;
	}
	yy=UIInput("是否要修改cust_id：","N");
	if(yy == "Y"){
		str2=UIInput("请输入新的cust_id：",-1);
		if(0==str2.size())
		{
			writelog(ERRCODE::LOG_CODE_PE_SHM_SET_ERR,"cust_id为空");
			return -1;
		}
		servu.cust_id = atol(str2.c_str());
		cout << "servu.cust_id: " << servu.cust_id << endl;
	}

	if(!p->UpdateServInfo(servu)){
		theLog<<"更新信息失败"<<ende;	
	} else {
		theLog<<"更新信息成功："<<endi;
	}
	
	/*
	string str2=UIInput("请输入新的状态：",-1);
	if(0==str2.size())
	{
		theLog<<"用户取消"<<endi;
	}
	else if(3!=str2.size())
	{
		theLog<<"错误的长度,必须为3位"<<ende;
	}
	else
	{
		servu.state = str2;
		if(!p->UpdateServInfo(servu)){
			theLog<<"更新状态失败"<<ende;	
		} else {
			theLog<<"更新后的状态："<<str2<<endi;
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
        
		//显示全局设置
		theLog<<"全局设置（可通过命令设置）" <<endi;

		if(GetCommandParam(argc,argv,"-cmd",cmd))
		{
			loop=false;
		}
		else
		{
			cout<<"\n"<<"----------------------------------------"<<"\n"<<"命令表：（q=exit）"<<"\n"
				<<"........................................"<<"\n"
				<<"1 show_COfferMgr 显示新基本资料共享内存"<<'\n'
				<<"2 create_COfferMgr 显示新基本资料共享内存"<<'\n'
				<<"3 load_COfferMgr 显示新基本资料共享内存"<<'\n'
				<<"4 find_COfferMgr(serv_id) 显示新基本资料共享内存"<<'\n'
				<<"5 find_COfferMgr(msinfo_id) 显示新基本资料共享内存"<<'\n'
				<<"10 show_CBillingBaseInfoMgr 显示新基本资料共享内存"<<'\n'
				<<"11 create_CBillingBaseInfoMgr 重建新基本资料共享内存（不加载数据）"<<'\n'
				<<"12 load_from_db_CBillingBaseInfoMgr 加载新基本资料共享内存 "<<'\n'
				<<"13 find_CBillingBaseInfoMgr 查询基本资料共享内存 "<<'\n'
				<<"14 update_CBillingBaseInfoMgr 更新基本资料共享内存 "<<'\n'
				<<"........................................"<<'\n'
				<<"----------------------------------------"<<endl;
			cmd=MUIInput("请选择命令：","0");
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
			cout<<"无效的命令："<<cmd<<"("<<nCmd<<")"<<endl;
			break;
		}
		theLog<<"命令 "<<nCmd<<" 返回 "<<ret<<endi;
		MUIInput("Press any key to continue ...","y");
	}

	return 0;
}

class ShmAdmin:public Application
{
public:
	/**
	 * 构造函数
	 */
	ShmAdmin(){};

	/**
	 * 析构函数
	 */
	virtual ~ShmAdmin(){};

	/**
	 * 获取库名
	 */
	virtual string getName() const{return "TestMain";};

	/**
	 * 获取程序使用方法
	 */
	virtual string getUsage() const{return "";};
	/**
	 * 参数校验
	 */
	virtual string getOptionString() const{return "";};
	/**
	 * 主程序逻辑
	 */
	virtual int run(int argc, char** argv)
	{
		if(!InitActiveApp())exit(1);
	    main_fun(argc,argv);
	    return 0;
	}	
};

/**
 * 程序入口声明
 */
APPLICATION_DECLARE(ShmAdmin)


