#include "bill_process.h"
#include <iostream>
#include <string>

class Psbill: public PS_BillProcess
{
public:
    Psbill(): PS_BillProcess() {
        std::cerr << "Psbill()" << std::endl;
        srand( time( NULL ) );
    }
    int onBeforeTask() {
        return 1;
    }
    int onTaskBegin( void *task_addr ) {}
    bool onChildInit() {
        std::cerr << "Psbill::onChildInit()" << std::endl;
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
        std::cout<<"child_ret:"<<child_ret<<std::endl;
        return child_ret;
    }
    virtual ~Psbill() {}
};


int main( int argc, char** argv )
{
    std::cout << "start main" << std::endl;

    try {
        Psbill psbill;
        psbill.init( argc, argv );
        psbill.run();
        return 0;
    } catch( const std::exception& ex ) {
        std::cerr << ex.what() << std::endl;
    } catch( ... ) {
        std::cerr << "exception caught" << std::endl;
    }

    return 1;
}
