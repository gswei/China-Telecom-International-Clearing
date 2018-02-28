#include "settle_item_detail.h"

using namespace tpss;

int main( int argc, char** argv )
{
    theLog.Open( "shmtest.log" );

    //0.
    CShmSettleItemDetailMgr shm;
    shm.Init( true );

    std::cout << "LoadDataFromDB()" << std::endl;
    shm.LoadDataFromDB();


    //1.测试遍历
    std::cout << "QueryAll()" << std::endl;

    if( shm.QueryAll() ) {
        do {
            Int64 settle_detail_id;
            Int64 settle_obj_id;

            shm.GetValue( "settle_detail_id", settle_detail_id );
            shm.GetValue( "settle_obj_id", settle_obj_id );

            std::cout << "settle_detail_id = " << settle_detail_id << "; settle_obj_id ="
                      << settle_obj_id << std::endl;
        } while( shm.Next() );
    }

    if( shm.QueryByIndex( "settle_obj_type|settle_obj_id", "1|2" ) ) {
        do {
            Int64 charge;
            Int64 event_type_id;

            shm.GetValue( "charge", charge );
            shm.GetValue( "event_type_id", event_type_id );

            std::cout << "charge = " << charge << "; event_type_id=" << event_type_id << std::endl;
        } while( shm.Next() );
    }

}