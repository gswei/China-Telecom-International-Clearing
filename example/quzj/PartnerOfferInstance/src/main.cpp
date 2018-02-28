#include "partner_offer_instance.h"

using namespace tpss;

int main( int argc, char** argv )
{
    theLog.Open( "shmtest.log" );

    CShmPartnerOfferInstanceMgr* shm = new CShmPartnerOfferInstanceMgr;
    shm->Init( true );

    std::cout << "LoadDataFromDB()" << std::endl;
    shm->LoadDataFromDB();


    //1.测试遍历
    std::cout << "QueryAll()" << std::endl;

    if( shm->QueryAll() ) {
        do {
            Int64 eff_date;
            Int64 obj_id;

            shm->GetValue( "eff_date", eff_date );
            shm->GetValue( "obj_id", obj_id );

            std::cout << "eff_date = " << eff_date  << "; obj_id ="
                      << obj_id << std::endl;
        } while( shm->Next() );
    }

    std::cout << "QueryByIndex" << endl;

    if( shm->QueryByIndex( "obj_type|obj_id", "2|2" ) ) {
        do {
            Int64 obj_type;
            Int64 obj_id;

            shm->GetValue( "obj_type", obj_type );
            shm->GetValue( "obj_id", obj_id );

            std::cout << "obj_type = " << obj_type << "; obj_id ="
                      << obj_id << std::endl;
        } while( shm->Next() );
    }

    return true;
}