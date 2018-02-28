/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-7 ÏÂÎç3:40:37
Module:      serv_equip.cpp
Author:      öÄÕ×¾²
Revision:    $Id: serv_equip.cpp 1736 2012-10-26 01:51:20Z zhenghb $
Description: xxxx for ServEquip
*/
#include "serv_equip.h"

CShmServEquipMgr::CShmServEquipMgr() : datas_( "ServEquip", 0 ), mutex_( "ServEquip" )
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Attach( false ) ) {
        printf( "ServEquip=>Attach failed£¬%s:%d\n", __FILE__, __LINE__ );
    }

    max_item_count_ = datas_.capacity();
    printf( "the shared memory can afford %d logs\n", max_item_count_ );
    MY_W_LOCK_END
}

CShmServEquipMgr::~CShmServEquipMgr()
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Detach() ) {
        printf( "ServEquip=>Detach failed£¬%s:%d\n", __FILE__, __LINE__ );
    }

    MY_W_LOCK_END
}

bool CShmServEquipMgr::PutData( const std::vector< ServEquipItem >& vec_serv_equip )
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    std::vector<ServEquipItem>::const_iterator iter;

    for( iter = vec_serv_equip.begin(); iter != vec_serv_equip.end(); ++iter ) {
        datas_.insert( *iter );

        if( datas_.size() > max_item_count_ ) {
            printf( "ServEquip=>the shared memory can afford %d logs\n", max_item_count_ );
            break;
        }
    }

    MY_W_LOCK_END
    return true;
}

bool CShmServEquipMgr::GetItem( Int64 serv_id, ServEquipItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < ServEquipItem, MAX_PP - 1 >::const_iterator iter;
    ServEquipItem tmp;
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
