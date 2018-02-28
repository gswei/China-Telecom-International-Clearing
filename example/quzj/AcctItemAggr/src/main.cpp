#include "acct_item_aggr.h"

using namespace tpss;

int main( int argc, char** argv )
{
    theLog.Open( "shm_test.log" );

    //0.
    CShmAcctItemAggrMgr* shm = CShmAcctItemAggrMgr::Instance();
    //shm->Init();

    std::cout << "LoadDataFromDB()" << std::endl;
    //shm->LoadDataFromDB();


    //1.测试遍历
    std::cout << "QueryAll()" << std::endl;

    if( shm->QueryAll() ) {
        do {
            Int64 charge;
            Int64 acct_id;

            shm->GetValue( "charge", charge );
            shm->GetValue( "acct_id", acct_id );

            std::cout << "charge = " << charge << "; acct_id =" << acct_id << std::endl;
        } while( shm->Next() );
    }

    //report
    std::cout << "Report()" << std::endl;
    shm->Report();

    /*
    if (shm->QueryByIndex("serv_id", "1"))
    {
    	do
    	{
    		Int64 charge;
    		Int64 acct_id;

    		shm->GetValue("charge", charge);
    		shm->GetValue("acct_id", acct_id);

    		std::cout << "charge = " << charge << "; acct_id ="
                         << acct_id << std::endl;
    	}
    	while (shm->Next());
    }


    if (shm->QueryByIndex("offer_inst_id", "1"))
    {
    	do
    	{
    		Int64 charge;
    		Int64 acct_id;

    		shm->GetValue("charge", charge);
    		shm->GetValue("acct_id", acct_id);

    		std::cout << "charge = " << charge << "; acct_id ="
                         << acct_id << std::endl;
    	}
    	while (shm->Next());
    }
           */

    return true;
}
