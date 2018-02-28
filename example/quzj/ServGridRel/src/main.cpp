#include "serv_grid_rel.h"

using namespace tpss;

int main( int argc, char** argv )
{
    theLog.Open( "shmtest.log" );

    //0.
    CShmServGridRelMgr shm;
    shm.Init( true );

    std::cout << "LoadDataFromDB()" << std::endl;
    shm.LoadDataFromDB();


    //1.²âÊÔ±éÀú
    std::cout << "QueryAll()" << std::endl;

    if( shm.QueryAll() ) {
        do {
            Int64 serv_id;
            Int64 grid_id;

            shm.GetValue( "grid_id", grid_id );
            shm.GetValue( "serv_id", serv_id );

            std::cout << "grid_id = " << grid_id << "; serv_id =" << serv_id << std::endl;
        } while( shm.Next() );
    }


    //2.²âÊÔ°´Ë÷Òý¼ìË÷
    std::cout << "QueryAll()" << std::endl;

    if( shm.QueryByIndex( "serv_id", "12" ) ) {
        do {
            Int64 serv_id;
            Int64 grid_id;

            shm.GetValue( "serv_id", serv_id );
            shm.GetValue( "grid_id", grid_id );

            std::cout << "serv_id = " << serv_id << "; grid_id=" << grid_id << std::endl;
        } while( shm.Next() );
    }


    //3.²âÊÔ°´Ë÷Òý¼ìË÷
    std::cout << "QueryAll()" << std::endl;

    if( shm.QueryByIndex( "grid_id", "1" ) ) {
        do {
            Int64 grid_id;
            Int64  serv_id;

            shm.GetValue( "grid_id", grid_id );
            shm.GetValue( "serv_id", serv_id );

            std::cout << "grid_id = " << grid_id << "; serv_id=" << serv_id << std::endl;
        } while( shm.Next() );
    }
}