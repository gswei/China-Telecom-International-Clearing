#include "serv_channel_rel.h"

using namespace tpss;

int main( int argc, char** argv )
{
    theLog.Open( "shmtest.log" );

    //0.
    CShmServChannelRelMgr shm;
    shm.Init( true );

    std::cout << "LoadDataFromDB()" << std::endl;
    shm.LoadDataFromDB();


    //1.测试遍历
    std::cout << "QueryAll()" << std::endl;

    if( shm.QueryAll() ) {
        do {
            Int64 party_id;
            Int64 create_date;

            shm.GetValue( "party_id", party_id );
            shm.GetValue( "create_date", create_date );

            std::cout << "party_id = " << party_id << "; create_date ="
                      << create_date << std::endl;
        } while( shm.Next() );
    }

    if( shm.QueryByIndex( "serv_id", "1" ) ) {
        do {
            Int64 party_id;
            Int64 create_date;

            shm.GetValue( "party_id", party_id );
            shm.GetValue( "create_date", create_date );

            std::cout << "party_id = " << party_id << "; create_date ="
                      << create_date << std::endl;
        } while( shm.Next() );
    }


    if( shm.QueryByIndex( "channel_id", "2" ) ) {
        do {
            Int64 party_id;
            Int64 create_date;

            shm.GetValue( "party_id", party_id );
            shm.GetValue( "create_date", create_date );

            std::cout << "party_id = " << party_id << "; create_date ="
                      << create_date << std::endl;
        } while( shm.Next() );
    }

    return true;
}