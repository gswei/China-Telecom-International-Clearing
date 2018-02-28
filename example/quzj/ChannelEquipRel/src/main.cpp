#include "channel_equip_rel.h"

using namespace tpss;

int main( int argc, char** argv )
{
    theLog.Open( "shmtest.log" );

    //0.
    CShmChannelEquipRelMgr shm;
    shm.Init( true );

    std::cout << "LoadDataFromDB()" << std::endl;
    shm.LoadDataFromDB();


    //1.测试遍历
    std::cout << "QueryAll()" << std::endl;

    if( shm.QueryAll() ) {
        do {
            Int64 equip_channel_rel_id;
            Int64 party_id;

            shm.GetValue( "equip_channel_rel_id", equip_channel_rel_id );
            shm.GetValue( "party_id", party_id );

            std::cout << "equip_channel_rel_id = " << equip_channel_rel_id << "; party_id ="
                      << party_id << std::endl;
        } while( shm.Next() );
    }

    std::cout << "QueryByIndex ChannelId" << std::endl;

    if( shm.QueryByIndex( "channel_id", "4" ) ) {
        do {
            Int64 equip_channel_rel_id;
            Int64 party_id;

            shm.GetValue( "equip_channel_rel_id", equip_channel_rel_id );
            shm.GetValue( "party_id", party_id );

            std::cout << "equip_channel_rel_id = " << equip_channel_rel_id << "; party_id ="
                      << party_id << std::endl;
        } while( shm.Next() );
    }

    std::cout << "QueryByIndex david" << std::endl;

    if( shm.QueryByIndex( "junction_box_code", "david" ) ) {
        do {
            Int64 equip_channel_rel_id;
            Int64 party_id;

            shm.GetValue( "equip_channel_rel_id", equip_channel_rel_id );
            shm.GetValue( "party_id", party_id );

            std::cout << "equip_channel_rel_id = " << equip_channel_rel_id << "; party_id ="
                      << party_id << std::endl;
        } while( shm.Next() );
    }

    std::cout << "QueryByIndex jim" << std::endl;

    if( shm.QueryByIndex( "sub_line_box_code", "cat" ) ) {
        do {
            Int64 equip_channel_rel_id;
            Int64 party_id;

            shm.GetValue( "equip_channel_rel_id", equip_channel_rel_id );
            shm.GetValue( "party_id", party_id );

            std::cout << "equip_channel_rel_id = " << equip_channel_rel_id << "; party_id ="
                      << party_id << std::endl;
        } while( shm.Next() );
    }

    return true;
}