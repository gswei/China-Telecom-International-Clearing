
#include "bill_process.h"
#include <iostream>
#include <time.h>
#include <stdlib.h>

class Predeal: public PS_BillProcess
{
public:
    Predeal(): PS_BillProcess() {
        std::cerr << "Predeal()" << std::endl;
        srand( time( NULL ) );
    }
    int onBeforeTask() {
        return 1;
    }
    int onTaskBegin( void *task_addr ) {
        return 0;
    }
    bool onChildInit() {
        std::cerr << "Predeal::onChildInit()" << std::endl;
        return true;
    }
    int onTask( void *task_addr, int offset, int ticket_num ) {
        int rows = 0;
        memcpy( &rows, task_addr, sizeof( int ) );
        int rd = rand() % 10 + 5;
        std::cerr << "rows=" << rows << " offset=" << offset << " ticket_num=" << ticket_num << " seconds=[" << rd << "]" << std::endl;
        sleep( rd );
        return ticket_num;
    }
    void onChildExit() {
        std::cerr << "子进程退出成功" << std::endl;
    }
    int onTaskOver( int child_ret ) {
        return child_ret;
    }
    virtual ~Predeal() {}
};


int main( int argc, char** argv )
{
    try {
        Predeal predeal;
        predeal.init( argc, argv );
        predeal.run();
        return 0;
    } catch( const std::exception& ex ) {
        std::cerr << ex.what() << std::endl;
    } catch( ... ) {
        std::cerr << "exception caught" << std::endl;
    }

    return 1;
}
