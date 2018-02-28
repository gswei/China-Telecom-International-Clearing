#include "bill_process.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>

class CollectA: public PS_BillProcess
{
public:
    CollectA(): PS_BillProcess(), _buffer( NULL ), _ticketLength( 0 ), _ticketNum( 0 ), _taskFinish( true ) {
        std::cerr << "CollectA()" << std::endl;
    }
    bool init( int argc, char *argv[] ) {
        if( !PS_BillProcess::init( argc, argv ) ) {
            return false;
        }

        _readStream.open( "/abm/data/agent/agent_ts/data/appData/zs.pt_tl_serv_data.20015784.201207.txt" );

        if( !_readStream ) {
            std::cerr << "文件不存在" << std::endl;
            return false;
        }

        _ticketLength = getTicketLength();
        _buffer = new char[_ticketLength];
        memset( _buffer, 0, _ticketLength );
        _taskFinish = false;
        return true;
    }
    int onBeforeTask() {
        if( _taskFinish ) {
            return 0;
        }

        return 1;

    }

    int onTaskBegin( void *task_addr ) {

        return 0;
    }
    bool onChildInit() {
        std::cerr << "CollectA::onChildInit()" << std::endl;
    }
    int onTask( void *task_addr, int offset, int ticket_num ) {
        int rows = 0;
        memset( _buffer, '\0', _ticketLength );

        while( rows < 10 && _readStream.getline( _buffer, _ticketLength, '\n' ) ) {
            if( _buffer[0] == '#' ) {
                continue;
            }

            char* ticketAddress = ( char* )task_addr + ( rows + 1 ) * _ticketLength;
            memcpy( ticketAddress, _buffer, _ticketLength );
            rows++;
        }

        memcpy( task_addr, &( rows ), sizeof( rows ) );
        std::cout << "ticketNum=" << _ticketNum << " rows=" << rows << std::endl;

        if( rows == 0 ) {
            _taskFinish = true;
        }

        return rows;
    }
    void onChildExit() {
        std::cerr << "子进程退出" << std::endl;
    }
    int onTaskOver( int child_ret ) {
        return child_ret;
    }
    virtual ~CollectA() {

    }
private:
    std::ifstream _readStream;
    char*         _buffer;
    int           _ticketNum;
    int           _ticketLength;
    bool          _taskFinish;
};


int main( int argc, char** argv )
{
    try {
        CollectA collect;
        collect.init( argc, argv );
        collect.run();
        return 0;
    } catch( const std::exception& ex ) {
        std::cerr << ex.what() << std::endl;
    } catch( ... ) {
        std::cerr << "exception caught" << std::endl;
    }

    return 1;
}
