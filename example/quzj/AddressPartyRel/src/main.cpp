#include "address_party_rel.h"

using namespace tpss;

int main( int argc, char** argv )
{
    theLog.Open( "shmtest.log" );

    //0.
    CShmAddressPartyRelMgr shm;
    shm.Init( true );

    std::cout << "LoadDataFromDB()" << std::endl;
    shm.LoadDataFromDB();



    std::cout << "QueryAll()" << std::endl;

    if( shm.QueryAll() ) {
        do {
            Int64 addr_party_rel_id;
            Int64 address_id;
            Int64 party_id;

            shm.GetValue( "addr_party_rel_id", addr_party_rel_id );
            shm.GetValue( "address_id", address_id );
            shm.GetValue( "party_id", party_id );

            std::cout << "addr_party_rel_id = " << addr_party_rel_id
                      << "; address_id =" << address_id
                      << "party_id" << party_id << endl;
        } while( shm.Next() );
    }

    std::cout << "QueryByIndex address_id" << std::endl;

    if( shm.QueryByIndex( "address_id", "2" ) ) {
        do {
            Int64 addr_party_rel_id;
            Int64 address_id;
            Int64 party_id;

            shm.GetValue( "addr_party_rel_id", addr_party_rel_id );
            shm.GetValue( "address_id", address_id );
            shm.GetValue( "party_id", party_id );

            std::cout << "addr_party_rel_id = " << addr_party_rel_id
                      << "; address_id =" << address_id
                      << "party_id" << party_id << endl;
        } while( shm.Next() );
    }

    std::cout << "QueryByIndex channel_id" << std::endl;

    if( shm.QueryByIndex( "channel_id", "4" ) ) {
        do {
            Int64 addr_party_rel_id;
            Int64 address_id;
            Int64 party_id;

            shm.GetValue( "addr_party_rel_id", addr_party_rel_id );
            shm.GetValue( "address_id", address_id );
            shm.GetValue( "party_id", party_id );

            std::cout << "addr_party_rel_id = " << addr_party_rel_id
                      << "; address_id =" << address_id
                      << "; party_id=" << party_id << endl;
        } while( shm.Next() );
    }

    return 0;
}