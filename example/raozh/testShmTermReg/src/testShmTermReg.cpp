#include "terminal_regist.h"

using namespace tpss;

int main( int argc, char** argv )
{
    //0.
    CShmTerminalRegistMgr* pshm = CShmTerminalRegistMgr::Instance();

    std::cout << "LoadDataFromDB()" << std::endl;
    pshm->LoadDataFromDB();


    //1.²âÊÔ±éÀú
    std::cout << "QueryAll()" << std::endl;

    if( pshm->QueryAll() ) {
        do {
            Int64 serv_id;
            std::string  acc_nbr;

            pshm->GetValue( "serv_id", serv_id );
            pshm->GetValue( "acc_nbr", acc_nbr );

            std::cout << "serv_id = " << serv_id << "; acc_nbr=" << acc_nbr << std::endl;
        } while( pshm->Next() );
    }


    //2.²âÊÔ°´Ë÷Òı¼ìË÷
    std::cout << "QueryByIndex()" << std::endl;

    if( pshm->QueryByIndex( "serv_id", "1004" ) ) {
        do {
            Int64 serv_id;
            std::string  acc_nbr;

            pshm->GetValue( "serv_id", serv_id );
            pshm->GetValue( "acc_nbr", acc_nbr );

            std::cout << "serv_id = " << serv_id << "; acc_nbr=" << acc_nbr << std::endl;
        } while( pshm->Next() );
    }
}
