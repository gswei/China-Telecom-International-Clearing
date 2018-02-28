#include "bill_process.h"
#include <iostream>


class Psrealcalc: public PS_BillProcess
{
public:
    Psrealcalc(): PS_BillProcess() {
        std::cerr << "Psrealcalc()" << std::endl;
        srand( time( NULL ) );
    }
    int onBeforeTask() {
        return 1;
    }
    int onTaskBegin( void *task_addr ) {}
    bool onChildInit() {
        std::cerr << "Psrealcalc::onChildInit()" << std::endl;
    }
    int onTask( void *task_addr, int offset, int ticket_num ) {
        int rows = 0;
        memcpy( &rows, task_addr, sizeof( int ) );
        int rd = rand() % 10 + 5;
        std::cerr << "rows=" << rows << " offset=" << offset << " ticket_num=" << ticket_num << " seconds=" << rd << std::endl;
        sleep( rd );
        return ticket_num;
    }
    void onChildExit() {
        std::cerr << "子进程退出" << std::endl;
    }
    int onTaskOver( int child_ret ) {
        return child_ret;
    }
    virtual ~Psrealcalc() {}
};


int main( int argc, char** argv )
{
    try {
        Psrealcalc psrealcalc;
        psrealcalc.init( argc, argv );
        psrealcalc.run();
        return 0;
    } catch( const std::exception& ex ) {
        std::cerr << ex.what() << std::endl;
    } catch( ... ) {
        std::cerr << "exception caught" << std::endl;
    }

    return 1;
}