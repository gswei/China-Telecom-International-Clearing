#include "tif_fee_bill.h"

using namespace tpss;

int main( int argc, char** argv )
{
    theLog.Open( "shmtest.log" );

    //0.
    CShmTifFeeBillMgr* shm = CShmTifFeeBillMgr::Instance( true );
    TifFeeBillItem item;

    if( shm ) {
        std::cout << "LoadDataFromDB()" << std::endl;
        shm->LoadDataFromDB();
        shm->Report();

        if( shm->GetItemByServId( 1, 1234, item ) ) {
            std::string str;
            std::cout << item.toString( str ) << endl;
        }

        //1.²âÊÔ±éÀú
        std::cout << "QueryAll()" << std::endl;

        if( shm->QueryAll() ) {
            do {
                Int64 serv_id;
                Int64 payment_id;

                shm->GetValue( "serv_id", serv_id );
                shm->GetValue( "payment_id", payment_id );

                std::cout << "charge = " << serv_id << "; acct_id ="
                          << payment_id << std::endl;
            } while( shm->Next() );
        }

        if( shm->QueryByIndex( "serv_id", "4" ) ) {
            do {
                Int64 serv_id;
                Int64 payment_id;

                shm->GetValue( "serv_id", serv_id );
                shm->GetValue( "payment_id", payment_id );

                std::cout << "charge = " << serv_id << "; acct_id ="
                          << payment_id << std::endl;
            } while( shm->Next() );
        }
    }

    return true;
}