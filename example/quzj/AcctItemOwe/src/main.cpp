#include "acct_item_owe.h"

using namespace tpss;

int main( int argc, char** argv )
{
    theLog.Open( "shmtest.log" );

    //0.
    CShmAcctItemOweMgr shm;
    shm.Init( true );

    std::cout << "LoadDataFromDB()" << std::endl;
    shm.LoadDataFromDB();


    //1.测试遍历
    std::cout << "QueryAll()" << std::endl;

    if( shm.QueryAll() ) {
        do {
            Int64 item_source_id;
            Int64 billing_cycle_id;

            shm.GetValue( "item_source_id", item_source_id );
            shm.GetValue( "billing_cycle_id", billing_cycle_id );

            std::cout << "charge = " << item_source_id << "; acct_id ="
                      << billing_cycle_id << std::endl;
        } while( shm.Next() );
    }

    if( shm.QueryByIndex( "serv_id", "4" ) ) {
        do {
            Int64 item_source_id;
            Int64 billing_cycle_id;

            shm.GetValue( "item_source_id", item_source_id );
            shm.GetValue( "billing_cycle_id", billing_cycle_id );

            std::cout << "charge = " << item_source_id << "; acct_id ="
                      << billing_cycle_id << std::endl;
        } while( shm.Next() );
    }


    if( shm.QueryByIndex( "offer_id", "4" ) ) {
        do {
            Int64 item_source_id;
            Int64 billing_cycle_id;

            shm.GetValue( "item_source_id", item_source_id );
            shm.GetValue( "billing_cycle_id", billing_cycle_id );

            std::cout << "charge = " << item_source_id << "; acct_id ="
                      << billing_cycle_id << std::endl;
        } while( shm.Next() );
    }

    return true;
}