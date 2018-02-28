#include "Globals.h"
#include "shm_Set.h"


bool less_CDemoData( ns_shm_data::CDemoData const & a, ns_shm_data::CDemoData const & b )
{
    return a.n < b.n;
}

int main( int argc, char** argv )
{
    if( !InitActiveApp() ) {
        exit( 1 );
    }

    theLog.Open( "shmtest.log" );

    ns_shm_data::T_SHMSET < ns_shm_data::CDemoData , MAX_PP - 1 > datas( "tmp", 0 );
    ns_shm_data::CDemoData tmp;
    ns_shm_data::T_SHMSET < ns_shm_data::CDemoData , MAX_PP - 1 >::const_iterator it;

    if( !datas.CreateShm() ) {
        return __LINE__;
    }

    if( !datas.Attach( false ) ) {
        return __LINE__;
    }

    string str;
    long maxcount = 10;
    long i;

    for( i = 0; i < maxcount; ++i ) {
        tmp.n = i;
        datas.insert( tmp );
    }

    thelog << datas.Report( str ) << endi;

    for( i = 0; i <= maxcount; ++i ) {
        tmp.n = i;

        if( datas.find( tmp ) == datas.end() ) {
            thelog << i << " not found" << endi;
        } else {
            thelog << tmp.toString( str ) << endi;
        }
    }

    thelog << "遍历：" << endi;

    for( it = datas.begin(); it != datas.end(); ++it ) {
        thelog << it.handle << " : " << ( *it ).toString( str ) << endi;
    }

    while( true ) {
        thelog << endl << "1 删除2添加3清空4 lower_bound 5 upper_bound 6反向遍历" << endi;
        str = ns_shm_data::UIInput( "请输入命令：b=break", -1 );

        if( "b" == str ) {
            break;
        }

        switch( atol( str.c_str() ) ) {
            case 1:
                str = ns_shm_data::UIInput( "请输入要删除的值：b=break", -1 );

                if( "b" == str ) {
                    break;
                }

                tmp.n = atol( str.c_str() );
                datas.erase( datas.find( tmp ) );
                break;

            case 2:
                str = ns_shm_data::UIInput( "请输入要添加的值：b=break", -1 );

                if( "b" == str ) {
                    break;
                }

                tmp.n = atol( str.c_str() );
                datas.insert( tmp );
                break;

            case 3:
                datas.clear();
                break;

            case 4:
                str = ns_shm_data::UIInput( "请输入值：b=break", -1 );

                if( "b" == str ) {
                    break;
                }

                tmp.n = atol( str.c_str() );

                if( datas.end() != ( it = datas.lower_bound( tmp ) ) ) {
                    thelog << it.handle << " : " << ( *it ).toString( str ) << endi;
                } else {
                    thelog << "end()" << endi;
                }

                break;

            case 5:
                str = ns_shm_data::UIInput( "请输入值：b=break", -1 );

                if( "b" == str ) {
                    break;
                }

                tmp.n = atol( str.c_str() );

                if( datas.end() != ( it = datas.upper_bound( tmp, less_CDemoData ) ) ) {
                    thelog << it.handle << " : " << ( *it ).toString( str ) << endi;
                } else {
                    thelog << "end()" << endi;
                }

                break;

            case 6:
                for( it = datas.rbegin(); it != datas.rend(); --it ) {
                    thelog << it.handle << " : " << ( *it ).toString( str ) << endi;
                }

                break;

            default:
                thelog << "无效命令：" << str << "(" << atol( str.c_str() ) << ")" << ende;
                break;
        }

        thelog << datas.Report( str ) << endi;
        thelog << "遍历：" << endi;

        for( it = datas.begin(); it != datas.end(); ++it ) {
            thelog << it.handle << " : " << ( *it ).toString( str ) << endi;
        }
    }

    if( !datas.Detach() ) {
        return __LINE__;
    }

    return 0;
}
