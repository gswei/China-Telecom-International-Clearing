/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-8 下午2:59:14
Module:      settle_item_detail.cpp
Author:      瞿兆静
Revision:    $v1.0$
Description: 结算清单 for SettleItemDetail
*/
#include "settle_item_detail.h"

CShmSettleItemDetailMgr::CShmSettleItemDetailMgr() : datas_( "SettleItemDetail", 0 ), mutex_( "SettleItemDetail" )
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Attach( false ) ) {
        printf( "SettleItemDetail=>Attach failed，%s:%d\n", __FILE__, __LINE__ );
    }

    max_item_count_ = datas_.capacity();
    printf( "the shared memory can afford %d logs\n", max_item_count_ );
    MY_W_LOCK_END
}

CShmSettleItemDetailMgr::~CShmSettleItemDetailMgr()
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Detach() ) {
        printf( "SettleItemDetail=>Detach failed，%s:%d\n", __FILE__, __LINE__ );
    }

    MY_W_LOCK_END
}
bool CShmSettleItemDetailMgr::PutData( const std::vector< SettleItemDetailItem >& vec_serv_equip )
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    std::vector<SettleItemDetailItem>::const_iterator iter;

    for( iter = vec_serv_equip.begin(); iter != vec_serv_equip.end(); ++iter ) {
        datas_.insert( *iter );

        if( datas_.size() > max_item_count_ ) {
            printf( "SettleItemDetail=>the shared memory can afford %d logs\n", max_item_count_ );
            break;
        }
    }

    MY_W_LOCK_END
    return true;
}

bool CShmSettleItemDetailMgr::GetItem( Int64 settle_obj_id, short int settle_obj_type, SettleItemDetailItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < SettleItemDetailItem, MAX_PP - 1 >::const_iterator iter;
    SettleItemDetailItem tmp;
    tmp.settle_obj_id = settle_obj_id;
    tmp.settle_obj_type = settle_obj_type;
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
