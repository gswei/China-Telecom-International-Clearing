//2014-05-05 国际语音运营商收支 国际互通短信收支


#include "ismp_lackinfo.h"

CLog theJSLog;

int main(int argc,char** argv)
{
	cout<<"********************************************* "<<endl;
	cout<<"*    China Telecom. Telephone Network         "<<endl;
	cout<<"*    InterNational Account Settle System      "<<endl;
	cout<<"*                                             "<<endl;
	cout<<"*           jssettleBalance                   "<<endl;
	cout<<"*           sys.GJZW.Version 1.0	             "<<endl;
	cout<<"*    created time :      2014-07-30 by  hed   "<<endl;
	cout<<"*    last update time :  2014-07-31 by  hed	 "<<endl;
	cout<<"********************************************* "<<endl;

	Ismp bb;

	if(!bb.init(argc,argv))
	{
		return 1;
	}
	
	bb.dealType();

	return 0;
}



