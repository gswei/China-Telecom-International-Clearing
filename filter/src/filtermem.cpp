

/*

删除共享内存 		filterMem -1 memfile ../env
清理sourceid的数据	filterMem -1 memfile sourceid ../env
加锁sourceid			filterMem 1 memfile sourceid ../env
解锁sourceid			filterMem 0 memfile sourceid ../env

*/

#include "C_CMemFilter.h"

CDatabase DBConn;
CLog theLog;

void DeleteAllMem(const char *szMemFile)
{
	C_CMemFilter filter;
	filter.Destroy(szMemFile);
}

void DealSource(int iDealType, const char *szMemFile, const char* sourceid)
{
	C_CMemFilter filter;
	if(iDealType == 0)
	{
		theLog << "解锁 " << sourceid <<endi;
		filter.ClearSemLock(szMemFile, sourceid);
	}
	else if(iDealType == 1)
	{
		theLog << "加锁 " << sourceid <<endi;
		errno = 0;
		filter.SemLock(szMemFile, sourceid);
	}
}

void displayInfo()
{
	cout << "****************************************************************"<<endl;
	cout << "****	       删除所有共享内存                                 "<<endl;
	cout << "****              usage : filtermem -1 [MEMFILE]                         " <<endl;
	cout << "****	       在共享内存中为SOURCEID加锁            "<<endl;
	cout << "****              usage : filtermem 1 [MEMFILE] [SOURCEID]	       " <<endl;
	cout << "****	       在共享内存中为SOURCEID解锁            "<<endl;
	cout << "****              usage : filtermem 0 [MEMFILE] [SOURCEID]        " <<endl;
	cout << "****************************************************************"<<endl;
}

int main(int argc, char* argv[])
{
	C_CMemFilter filter;
	try
	{
		if(argc !=4 && argc != 3)
		{
			displayInfo();
			return -1;
		}

		if(argc == 3)
		{
			if(strcmp(argv[1], "-1") != 0)
			{
				displayInfo();
				return -1;
			}
			DeleteAllMem(argv[2]);
			
		}
		else if(argc == 4)
		{
			DealSource(atoi(argv[1]), argv[2], argv[3]);
		}
		
	}
	catch(CException e)
	{
		errLog(LEVEL_ERROR, "nofile" , errno, "help", __FILE__, __LINE__, e);
	}
	return 0;
}


