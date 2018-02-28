/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-8 下午4:02:34
Module:      ratable_resource_accumulator.cpp
Author:      瞿兆静
Revision:    $v1.0$
Description: 计费累计值 for RatableResourceAccumulator
*/
#include "ratable_resource_accumulator.h"

#include "psutil.h"

using namespace tpss;

CShmRatableResourceAccumulatorMgr::~CShmRatableResourceAccumulatorMgr()
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Detach() ) {
        printf( "RatableResourceAccumulator=>Detach failed，%s:%d\n", __FILE__, __LINE__ );
    }

    MY_W_LOCK_END
}

bool CShmRatableResourceAccumulatorMgr::Init( bool create_shm )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( create_shm ) {
        if( !datas_.CreateShm() ) {
            ret = false;
        }
    }

    if( !datas_.Attach( false ) ) {
        ret = false;
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmRatableResourceAccumulatorMgr::LoadDataFromDB()
{
    DBConnection conn;
    bool ret = true;

    if( dbConnect( conn ) ) {
        Statement stmt = conn.createStatement();
        std::string sql = "select RATABLE_RESOURCE_ID, OWNER_TYPE, OWNER_ID, "
                          "RATABLE_CYCLE_ID, BALANCE from RATABLE_RESOURCE_ACCUMULATOR";
        stmt.setSQLString( sql );

        if( stmt.execute() ) {
            RatableResourceAccumulatorItem tmp;
            mutex_.AttachOrCreateMutexIfNeed();
            MY_W_LOCK_BEGIN

            while( stmt >> tmp.ratable_resource_id >> tmp.owner_type
                    >> tmp.owner_id >> tmp.ratable_cycle_id >> tmp.balance ) {
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

bool CShmRatableResourceAccumulatorMgr::PutData( const std::vector< RatableResourceAccumulatorItem >& vec_ratable_resource_accumulator )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    std::vector<RatableResourceAccumulatorItem>::const_iterator iter;

    for( iter = vec_ratable_resource_accumulator.begin(); iter != vec_ratable_resource_accumulator.end(); ++iter ) {
        if( !Insert( *iter ) ) {
            ret = false;
            break;
        }
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmRatableResourceAccumulatorMgr::GetItem( int ratable_resource_id, Int64 owner_id, RatableResourceAccumulatorItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < RatableResourceAccumulatorItem, MAX_PP - 1 >::const_iterator iter;
    RatableResourceAccumulatorItem tmp;
    tmp.ratable_resource_id = ratable_resource_id;
    tmp.owner_id = owner_id;
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

bool CShmRatableResourceAccumulatorMgr::Insert( const RatableResourceAccumulatorItem& item )
{
    return datas_.insert( item ).first != datas_.end();
}
