#include "CF_Common.h"
#include "CF_CFscan.h"
#include "CF_CLogger.h"
#include "CF_CFmtChange.h"
#include "CF_CMemFileIO.h"

#include "SysCtrl.h"

using namespace std;
using namespace _SYS_;

CLog theJSLog;

//src是否包含于des
bool CommpareAB(char* src,char* des)
{
	char sz1[2048],sz2[2048];
	memset(sz1,0,sizeof(sz1));
	memset(sz2,0,sizeof(sz2));
	strcpy(sz1,src);
	strcpy(sz2,des);
	
	vector<string> va;
	vector<string> vb;
	splitString(sz2,",",vb,true,true);		//跳过空格,跳过重复的字符串
	splitString(sz1,",",va,true,true);		//跳过空格,跳过重复的字符串

	set<string> sa,sb;						//最后存放的号码
	int pos = 0,ia,ib;
	string aa,bb;
	char tmp[10];

	for(int i = 0;i<vb.size();i++)
	{
		pos = vb[i].find('-',1);
		if(0 == string::npos)
		{
			sb.insert(vb[i]);
		}
		else		//按号段
		{
			aa = vb[i].substr(0,pos);		//08-011
			bb = vb[i].substr(pos+1);
			
			ia = atoi(aa.c_str());
			ib = atoi(bb.c_str());
				
			//sprintf(tmp_base,"%.*lf",percent.percent_pos,cur_base);
			int len = strlen(aa.c_str());
			for(int ii = ia;ii<=ib;ii++)
			{	
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%.*d",len,ii);
				sb.insert(tmp);
			}
		}

	}
	
	for(int i = 0;i<va.size();i++)
	{
		pos = va[i].find('-',1);
		if(0 == string::npos)
		{
			sa.insert(va[i]);
		}
		else		//按号段
		{
			aa = va[i].substr(0,pos);
			bb = va[i].substr(pos+1);
			
			ia = atoi(aa.c_str());
			ib = atoi(bb.c_str());

			//sprintf(tmp_base,"%.*lf",percent.percent_pos,cur_base);
			int len = strlen(aa.c_str());
			for(int ii = ia;ii<=ib;ii++)
			{	
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%.*d",len,ii);
				sa.insert(tmp);
			}
		}

	}
	
	for(set<string>::iterator iter = sa.begin();iter != sa.end();++iter)
	{
		if(sb.count(*iter)  == 0)
		{
			return false;
		}

	}

	return true;
}

bool create(key_t shmkey, size_t size, bool rebuild)
{
	int m_iShmID=shmget(shmkey,0,0);
	
	cout<<"m_iShmID="<<m_iShmID<<endl;

	if(m_iShmID!=-1 && rebuild)
	{
		cout<<"先删除共享 内存"<<endl;
		if( shmctl(m_iShmID,IPC_RMID,NULL)==-1 )
		{
			return false;
		}
	}

	//bool m_bNewCreated=(m_iShmID==-1 || rebuild);

	//m_iShmID = shmget(shmkey, size, IPC_CREAT|0600);
	//cout<<"m_iShmID="<<m_iShmID<<endl;

	//if(m_iShmID==-1) return false;
	
	//char* m_pShmAddr=(char *)shmat(m_iShmID,0,0);
	//cout<<"m_pShmAddr="<<m_pShmAddr<<endl;

	//if(m_pShmAddr==(char *)-1) return false;

	return true;
}


//检查CPU和内存使用率
bool chkSysUsed()
{
	//all_ok=true;
	strCpuInfo cpu_info;
	strMemInfo mem_info;
	//int now_state;
	short now_state;
	int max_cpu_used, max_mem_used;

	//now_state=GetSysState();

	//-------CPU----------------------------

	if( getSysResource(cpu_info) != 1 )
	{
		//{Print("获取CPU占用率失败！"<<endl, "")}
		return false;
	}
	//{Print("CPU占用率"<<100 - cpu_info.iCpuIdle<<"%\n", "CPU used "<<100 - cpu_info.iCpuIdle<<"%\n")}
	
	cout<<"###################CPU占用率["<<100 - cpu_info.iCpuIdle<<"]######################"<<endl;

	//----------内存-------------------------
	if( getMemoryResource(mem_info) != 1 )
	{
		//{Print("获取内存占用率失败！"<<endl, "")}
		return false;
	}
	int mem_used=100 * (mem_info.iTotalPhyMem - mem_info.iFreePhyMem) / mem_info.iTotalPhyMem;
	//{Print("内存使用率"<<mem_used<<"%\n", "Memory used "<<mem_used<<"%\n")}
	cout<<"####################内存使用率["<<mem_used<<"]###########################"<<endl;

	return true;
}

int  main(int argc,char* argv)
{

	while(1)
	{
		chkSysUsed();
		sleep(30);
	}


	//bool ret = create(87690037,52210240,true);

	//cout<<"ret = "<<ret<<endl;

/*
	string* vsql;
	vsql = new string[4];

	vsql[0] = "ABC";
	vsql[1] = "BCD";


	delete[] vsql;
	vsql = NULL;

	if(!vsql)
	{
		cout<<"尼玛,释放了"<<endl;
	}
*/
	char* pp = "06-07,61145,61401-61403,61411-61413,61421-61423,61431-61432,61434-61435,61466,61468,61478-61479,61481-61482";
	char* qq = "06-08,61145,61401-61403,61411-61482";
/*	
	bool ret = CommpareAB(pp,qq);

	if(ret)
	{
		cout<<"YES,号段["<<pp<<"]在号段["<<qq<<"]中"<<endl;
	}
	else
	{
		cout<<"NO,号段["<<pp<<"]不在号段["<<qq<<"]中"<<endl;
	}
*/

	
	return 0;
}


