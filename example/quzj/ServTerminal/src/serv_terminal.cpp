/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-7 下午2:51:46
Module:      serv_terminal.cpp
Author:      瞿兆静
Revision:    $v1.0$
Description: 用户终端信息 for ServTerminal
*/
#include "serv_terminal.h"

CShmServTerminalMgr::CShmServTerminalMgr() : datas_( "ServTerm", 0 ), mutex_( "ServTerm" )
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Attach( false ) ) {
        printf( "ServTerminal=>Attach failed，%s:%d\n", __FILE__, __LINE__ );
    }

    max_item_count_ = datas_.capacity();
    printf( "the shared memory can afford %d logs\n", max_item_count_ );
    MY_W_LOCK_END
}

CShmServTerminalMgr::~CShmServTerminalMgr()
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Detach() ) {
        printf( "ServTerminal=>Detach failed，%s:%d\n", __FILE__, __LINE__ );
    }

    MY_W_LOCK_END
}

bool CShmServTerminalMgr::PutData( const std::vector< ServTerminalItem >& vec_serv_terminal )
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    std::vector<ServTerminalItem>::const_iterator iter;

    for( iter = vec_serv_terminal.begin(); iter != vec_serv_terminal.end(); ++iter ) {
        datas_.insert( *iter );

        if( datas_.size() > max_item_count_ ) {
            printf( "AcctItemOwe=>the shared memory can afford %d logs\n", max_item_count_ );
            break;
        }
    }

    MY_W_LOCK_END
    return true;
}

bool CShmServTerminalMgr::GetItem( Int64 serv_id, ServTerminalItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < ServTerminalItem, MAX_PP - 1 >::const_iterator iter;
    ServTerminalItem tmp;
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


