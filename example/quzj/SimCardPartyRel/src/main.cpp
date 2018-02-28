#include "sim_card_party_rel.h"

using namespace tpss;

int main( int argc, char** argv )
{
    theLog.Open( "shmtest.log" );

    //0.
    CShmSimCardPartyRelMgr shm;
    shm.Init( true );

    std::cout << "LoadDataFromDB()" << std::endl;
    shm.LoadDataFromDB();


    //1.²âÊÔ±éÀú
    std::cout << "QueryAll()" << std::endl;

    if( shm.QueryAll() ) {
        do {
            string card_seq;
            string acc_nbr;

            shm.GetValue( "card_seq", card_seq );
            shm.GetValue( "acc_nbr", acc_nbr );

            std::cout << "card_seq = " << card_seq <<
                      "; acc_nbr =" << acc_nbr << std::endl;
        } while( shm.Next() );
    }


    //2.²âÊÔ°´Ë÷Òý¼ìË÷
    std::cout << "QueryAll()" << std::endl;

    if( shm.QueryByIndex( "card_seq", "1" ) ) {
        do {
            string card_seq;
            string acc_nbr;

            shm.GetValue( "card_seq", card_seq );
            shm.GetValue( "acc_nbr", acc_nbr );

            std::cout << "card_seq = " << card_seq <<
                      "; acc_nbr =" << acc_nbr << std::endl;
        } while( shm.Next() );
    }


    //3.²âÊÔ°´Ë÷Òý¼ìË÷
    std::cout << "QueryAll()" << std::endl;

    if( shm.QueryByIndex( "acc_nbr", "2" ) ) {
        do {
            string card_seq;
            string acc_nbr;

            shm.GetValue( "card_seq", card_seq );
            shm.GetValue( "acc_nbr", acc_nbr );

            std::cout << "card_seq = " << card_seq <<
                      "; acc_nbr =" << acc_nbr << std::endl;
        } while( shm.Next() );
    }

    return 0;
}