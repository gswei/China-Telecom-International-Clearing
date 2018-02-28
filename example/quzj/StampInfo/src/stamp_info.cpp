/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-7 下午4:09:58
Module:      stamp_info.cpp
Author:      瞿兆静
Revision:    $Id: stamp_info.cpp 1736 2012-10-26 01:51:20Z zhenghb $
Description: 代金券领取信息 for StampInfo
*/
#include "stamp_info.h"

CShmStampInfoMgr::CShmStampInfoMgr() : datas_( "StampInfo", 0 ), mutex_( "StampInfo" )
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Attach( false ) ) {
        printf( "StampInfo=>Attach failed，%s:%d\n", __FILE__, __LINE__ );
    }

    max_item_count_ = datas_.capacity();
    printf( "the shared memory can afford %d logs\n", max_item_count_ );
    MY_W_LOCK_END
}

CShmStampInfoMgr::~CShmStampInfoMgr()
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Detach() ) {
        printf( "StampInfo=>Detach failed，%s:%d\n", __FILE__, __LINE__ );
    }

    MY_W_LOCK_END
}

bool CShmStampInfoMgr::PutData( const std::vector< StampInfoItem >& vec_stamp_info )
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    std::vector<StampInfoItem>::const_iterator iter;

    for( iter = vec_stamp_info.begin(); iter != vec_stamp_info.end(); ++iter ) {
        datas_.insert( *iter );

        if( datas_.size() > max_item_count_ ) {
            printf( "StampInfo=>the shared memory can afford %d logs\n", max_item_count_ );
            break;
        }
    }

    MY_W_LOCK_END
    return true;
}

bool CShmStampInfoMgr::GetItem( const char* stamp_code, StampInfoItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < StampInfoItem, MAX_PP - 1 >::const_iterator iter;
    StampInfoItem tmp;
    strcpy( tmp.stamp_code, stamp_code );
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
