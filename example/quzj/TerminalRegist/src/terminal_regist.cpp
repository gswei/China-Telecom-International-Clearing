/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-6 ÏÂÎç2:33:46
Module:      terminal_regist.cpp
Author:      öÄÕ×¾²
Revision:    $v1.0$
Description: ÓÃ»§×¢²áÖÕ¶Ë½Ó¿Ú for TerminalRegist
*/
#include "terminal_regist.h"

CShmTerminalRegistMgr::CShmTerminalRegistMgr() : datas_( "TermReg", 0 ), mutex_( "TermReg" )
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Attach( false ) ) {
        printf( "TerminalRegist=>Attach failed£¬%s:%d\n", __FILE__, __LINE__ );
    }

    max_item_count_ = datas_.capacity();
    printf( "the shared memory can afford %d logs\n", max_item_count_ );
    MY_W_LOCK_END
}

CShmTerminalRegistMgr::~CShmTerminalRegistMgr()
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Detach() ) {
        printf( "TerminalRegist=>Detach failed£¬%s:%d\n", __FILE__, __LINE__ );
    }

    MY_W_LOCK_END
}
bool CShmTerminalRegistMgr::PutData( const std::vector<TerminalRegistItem>& vec_term_reg_item )
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    std::vector<TerminalRegistItem>::const_iterator iter;

    for( iter = vec_term_reg_item.begin(); iter != vec_term_reg_item.end(); ++iter ) {
        datas_.insert( *iter );

        if( datas_.size() > max_item_count_ ) {
            printf( "TerminalRegist=>the shared memory can afford %d logs\n", max_item_count_ );
            break;
        }
    }

    MY_W_LOCK_END
    return true;
}

bool CShmTerminalRegistMgr::GetItem( Int64 serv_id, TerminalRegistItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < TerminalRegistItem, MAX_PP - 1 >::const_iterator iter;
    TerminalRegistItem tmp;
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
