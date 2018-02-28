#include "channel.h"

using namespace tpss;

int main( int argc, char** argv )
{
    theLog.Open( "shmtest.log" );

    //0.
    CShmChannelMgr shm;
    shm.Init( true );

    std::cout << "LoadDataFromDB()" << std::endl;
    shm.LoadDataFromDB();


    //1.测试遍历
    std::cout << "QueryAll()" << std::endl;

    if( shm.QueryAll() ) {
        do {
            Int64 status;
            Int64 lant_id;

            shm.GetValue( "status", status );
            shm.GetValue( "lant_id", lant_id );

            std::cout << "status = " << status << "; lant_id ="
                      << lant_id << std::endl;
        } while( shm.Next() );
    }

    if( shm.QueryByIndex( "channel_id", "3" ) ) {
        do {
            Int64 status;
            Int64 lant_id;

            shm.GetValue( "status", status );
            shm.GetValue( "lant_id", lant_id );

            std::cout << "status = " << status << "; lant_id ="
                      << lant_id << std::endl;
        } while( shm.Next() );
    }


    if( shm.QueryByIndex( "party_id", "1" ) ) {
        do {
            Int64 status;
            Int64 lant_id;

            shm.GetValue( "status", status );
            shm.GetValue( "lant_id", lant_id );

            std::cout << "status = " << status << "; lant_id ="
                      << lant_id << std::endl;
        } while( shm.Next() );
    }

    return true;
}