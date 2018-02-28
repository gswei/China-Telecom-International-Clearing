//2013-12-25 程序用处理国际固网 语音和短信的摊分平衡
//2014-01-07 处理国际固网语音时 增加受付的表D_GWSF_SHARE_RESULT
//2014-04-24 调账输入表带账期,里面的账期字段表示各个月的
//2014-05-05 调整的基础取值有变 去取去话转去 dir=2,3
//2014-05-06 摊分到省的结果保存2位小数

#include "Balance.h"

CLog theJSLog;

int main(int argc,char** argv)
{
	cout<<"********************************************* "<<endl;
	cout<<"*    China Telecom. Telephone Network         "<<endl;
	cout<<"*    InterNational Account Settle System      "<<endl;
	cout<<"*                                             "<<endl;
	cout<<"*           jssettleBalance                   "<<endl;
	cout<<"*           sys.GJZW.Version 1.0	             "<<endl;
	cout<<"*    created time :      2013-12-23 by  hed   "<<endl;
	cout<<"*    last update time :  2014-05-05 by  hed	 "<<endl;
	cout<<"********************************************* "<<endl;

	Balance bb;

	if(!bb.init(argc,argv))
	{
		return 1;
	}
	
	bb.run();

	return 0;
}
