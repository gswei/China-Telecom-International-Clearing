#include "channel_member.h"

using namespace tpss;

int main( int argc, char** argv )
{
    theLog.Open( "shmtest.log" );

    //0.
    CShmChannelMemberMgr shm;
    ChannelInfoQueryItem item;
    shm.Init( true );

    std::cout << "LoadDataFromDB()" << std::endl;

    if( shm.LoadDataFromDB() ) {
        if( shm.GetItem( 1233, item ) ) {
            printf( "channel_id = %d channel_nbr = %s party_id = %d channel_member_id = %d\n",
                    item.channel_id, item.channel_nbr, item.party_id, item.channel_member_id );
        }
    }


    return true;
}