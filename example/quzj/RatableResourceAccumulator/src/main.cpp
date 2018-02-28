#include "ratable_resource_accumulator.h"

using namespace tpss;

int main( int argc, char** argv )
{
    theLog.Open( "shmtest.log" );

    //0.
    CShmRatableResourceAccumulatorMgr shm;
    shm.Init( true );

    std::cout << "LoadDataFromDB()" << std::endl;
    shm.LoadDataFromDB();


    //1.测试遍历
    std::cout << "QueryAll()" << std::endl;

    if( shm.QueryAll() ) {
        do {
            Int64 balance;
            Int64 ratable_cycle_id;

            shm.GetValue( "balance", balance );
            shm.GetValue( "ratable_cycle_id", ratable_cycle_id );

            std::cout << "balance = " << balance << "; ratable_cycle_id ="
                      << ratable_cycle_id << std::endl;
        } while( shm.Next() );
    }

    if( shm.QueryByIndex( "ratable_resource_id|owner_id", "302|11" ) ) {
        std::cout << "QueryAll()" << std::endl;

        do {
            Int64 balance;
            Int64 ratable_cycle_id;

            shm.GetValue( "balance", balance );
            shm.GetValue( "ratable_cycle_id", ratable_cycle_id );

            std::cout << "balance = " << balance << "; ratable_cycle_id ="
                      << ratable_cycle_id << std::endl;
        } while( shm.Next() );
    }

    return true;
}