/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-8 上午10:10:56
Module:      address_serv_rel.cpp
Author:      瞿兆静
Revision:    $v1.0$
Description: 用户地址结构 for AddressServRel
*/
#include "address_serv_rel.h"

#include "psutil.h"

using namespace tpss;

CShmAddressServRelMgr::~CShmAddressServRelMgr()
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Detach() ) {
        printf( "AddressServRel=>Detach failed，%s:%d\n", __FILE__, __LINE__ );
    }

    MY_W_LOCK_END
}

bool CShmAddressServRelMgr::Init( bool create_shm )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Attach( false ) ) {
        ret = false;
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmAddressServRelMgr::LoadDataFromDB()
{
    DBConnection conn;
    bool ret = true;

    if( dbConnect( conn ) ) {
        Statement stmt = conn.createStatement();
        std::string sql = "select SERV_ID, AGREEMENT_ID, ADDRESS_ID, "
                          "BUREAU_ID, EXCHANGE_ID, STAT_REGION_ID from SERV_LOCATION";
        stmt.setSQLString( sql );

        if( stmt.execute() ) {
            AddressServRelItem tmp;
            mutex_.AttachOrCreateMutexIfNeed();
            MY_W_LOCK_BEGIN

            while( stmt >> tmp.serv_id >> tmp.agreement_id
                    >> tmp.address_id >> tmp.bureau_id >> tmp.exchange_id
                    >> tmp.stat_region_id ) {
                if( false == Insert( tmp ) ) {
                    printf( "%s文件%d行: 插入数据失败!\n", __FILE__, __LINE__ );
                    ret = false;
                    break;
                }
            }

            MY_W_LOCK_END
            conn.close();
        }
    }

    return ret;
}

bool CShmAddressServRelMgr::PutData( const std::vector< AddressServRelItem >& vec_address_serv_rel )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    std::vector<AddressServRelItem>::const_iterator iter;
    string str;

    for( iter = vec_address_serv_rel.begin(); iter != vec_address_serv_rel.end(); ++iter ) {
        if( false == Insert( *iter ) ) {
            ret = false;
            break;
        }
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmAddressServRelMgr::GetItem( Int64 serv_id, AddressServRelItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < AddressServRelItem, MAX_PP - 1 >::const_iterator iter;
    AddressServRelItem tmp;
    tmp.serv_id = serv_id;
    iter = datas_.find( tmp );

    if( -1 == iter.handle ) {
        ret = false;
    } else {
        item = *iter;
        ret = true;
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmAddressServRelMgr::Insert( const AddressServRelItem& item )
{
    return datas_.insert( item ).first != datas_.end();
}
