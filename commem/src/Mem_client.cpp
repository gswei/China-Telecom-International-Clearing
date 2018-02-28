
#include "CommonMemClient.h"
#include "CF_COracleDB.h"
#include "CF_CReadIni.h"
#include <time.h>
#include <stdlib.h>

CDatabase DBConn;
CLog theLog;

int main( int argc, char *argv[])
{
	string serv_id="SERV1";
	string mem_name="I_TELENO_DEF_PROPERTY";
	int version_id=1;
	char infile[256];
	char logpath[256];
	char envname[50];
	char errmsg[500]; 
	//struct timeval filetime, start, finish;
	//timerclear(&start);
	//timerclear(&finish);

	//核心参数获取
	std::string log_path,log_level;
	if( !tpss::getKenelParam( "log.path", log_path ) ) {
		tpss::writelog( 0, "获取日志路径失败" );
	}
	if( !tpss::getKenelParam( "log.level", log_level ) ) {
		tpss::writelog( 0, "获取日志级别失败" );
	}
	int level_no = es::StringUtil::toInt( log_level );

	strcpy(logpath,log_path.c_str());
	debugFlag =  level_no;
	
	/*strcpy(infile,getenv("ENVPATH"));
	completeDir(infile);
	string DBPATH=infile;
	DBPATH+="zhjs.ini";
	vector<mysize_t> aa;

	CReadIni v_ini;
	v_ini.init(DBPATH.c_str());

	if ( ! v_ini.GetValue("COMMON","LOG_PATH",logpath,'Y') )
	{
		sprintf(errmsg,"get env LOG_PATH from %s err!",DBPATH.c_str());
		printf( "%s\n",errmsg );
		return ( -1 );
	}	
	completeDir( logpath );*/
	
	theLog.setLog(logpath, 1, "COMMEM", "2", 1);
	connectDB( (char *)DBPATH.c_str(), DBConn ); 
	long long timePart1 =0;

	//第一个表
	CommonMemClient *newtable1;
	newtable1 = new CommonMemClient(serv_id,mem_name, version_id ,infile);
  
  string buf64="B701073191";
  theLog<<"开始查找记录["<<buf64.c_str()<<"]"<<endi;
  newtable1->tableRecord.setString(newtable1->recordBuffer, 10, buf64.c_str());
  newtable1->query(newtable1->recordBuffer,aa,1);
  theLog<<"找到"<<aa.size()<<"条记录"<<endi;
  newtable1->printRecordToString(aa[0]);
  theLog<<"循环100万次的时间"<<endi;
  for (int kk=0;kk<1000000;kk++)
  {
  	//gettimeofday(&start, NULL);
  	aa.clear();
  	newtable1->tableRecord.setString(newtable1->recordBuffer, 10, buf64.c_str());
  	newtable1->query(newtable1->recordBuffer,aa,1);
  	//t1+=clock()-tt;
  	//gettimeofday(&finish, NULL);

  	//timePart1+= (finish.tv_sec - start.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - start.tv_usec);
  
  }
  
  //theLog<<"耗时"<<timePart1/1000<<"毫秒"<<endi;
  theLog<<aa.size()<<endi;
  
  timePart1=0;

  theLog<<"2:循环100万次的时间"<<endi;

  for (int kkk=0;kkk<1000000;kkk++)
  {
  	//gettimeofday(&start, NULL);
  	aa.clear();
  	//newtable1->tableRecord.setString(newtable1->recordBuffer, 10, buf64.c_str());
  	newtable1->queryTableRecord(buf64.c_str(),1,aa);
  	//t1+=clock()-tt;
  	//gettimeofday(&finish, NULL);

  	//timePart1+= (finish.tv_sec - start.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - start.tv_usec);
  
  }
  
  //theLog<<"2:耗时"<<timePart1/1000<<"毫秒"<<endi;
  theLog<<aa.size()<<endi;
  

	//第二个表
	CommonMemClient *newtable2;
	string bufs,bufs2,bufs3;
	string mem_name2="I_RATE_CYCLE";
	newtable2 = new CommonMemClient(serv_id,mem_name2, version_id ,infile);

	theLog<<" "<<endi;
	
	bufs="HWCYE";
	bufs2="GZm6A";
	bufs3="CA";
	newtable2->tableRecord.setString(newtable2->recordBuffer, 1, bufs.c_str());
	newtable2->tableRecord.setString(newtable2->recordBuffer, 2, bufs2.c_str());
	newtable2->tableRecord.setString(newtable2->recordBuffer, 4, bufs3.c_str());
	newtable2->query(newtable2->recordBuffer,aa,1);
	theLog<<"查找记录:"<<bufs.c_str()<<endi;
	theLog<<"找到"<<aa.size()<<"条记录"<<endi;
	for ( int i=0;i<aa.size();i++)
	{
		newtable2->printRecordToString(aa[i]);
	}
	
	theLog<<" "<<endi;
	
	//memset(newtable2->recordBuffer,0,newtable2->bufferLen);
	aa.clear();
	bufs="HWCYE,GZm6A,CA";
	newtable2->queryTableRecord(bufs.c_str(),1,aa);
	theLog<<"2:查找记录:"<<bufs.c_str()<<endi;
	theLog<<"2:找到"<<aa.size()<<"条记录"<<endi;
	for ( int i=0;i<aa.size();i++)
	{
		newtable2->printRecordToString(aa[i]);
	}
	
	theLog<<" "<<endi;
	
		
	timePart1=0;
	theLog<<"循环100万次的时间"<<endi;
	for (int kk=0;kk<1000000;kk++)
  {
  	//gettimeofday(&start, NULL);
		//memset(newtable2->recordBuffer,0,newtable2->bufferLen);
  	aa.clear();
		bufs="HWCYE";
		bufs2="GZm6A";
		bufs3="CA";
		newtable2->tableRecord.setString(newtable2->recordBuffer, 1, bufs.c_str());
		newtable2->tableRecord.setString(newtable2->recordBuffer, 2, bufs2.c_str());
		newtable2->tableRecord.setString(newtable2->recordBuffer, 4, bufs3.c_str());
		newtable2->query(newtable2->recordBuffer,aa,1);
  	//t1+=clock()-tt;
  	//gettimeofday(&finish, NULL);

  	//timePart1+= (finish.tv_sec - start.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - start.tv_usec);
  
  }
  //theLog<<"耗时"<<timePart1/1000<<"毫秒"<<endi;
  theLog<<aa.size()<<endi;

	
	bufs="HWCYE,GZm6A,CA";
  timePart1=0;
	
	

  theLog<<"2:循环100万次的时间"<<endi;

  for (int kkk=0;kkk<1000000;kkk++)
  {
  	//gettimeofday(&start, NULL);
  	//memset(newtable2->recordBuffer,0,newtable2->bufferLen);
  	aa.clear();
  	bufs="HWCYE,GZm6A,CA";
  	//newtable2->tableRecord.setString(newtable1->recordBuffer, 10, buf64.c_str());
  	newtable2->queryTableRecord(bufs.c_str(),1,aa);
  	//t1+=clock()-tt;
  	//gettimeofday(&finish, NULL);

  	//timePart1+= (finish.tv_sec - start.tv_sec) * (long) 1000 * (long) 1000 + (finish.tv_usec - start.tv_usec);
  
  }
  
  //theLog<<"2:耗时"<<timePart1/1000<<"毫秒"<<endi;	
  theLog<<aa.size()<<endi;
	theLog<<" "<<endi;

	
	delete newtable1;
	
	delete newtable2;

	return 0;
}

