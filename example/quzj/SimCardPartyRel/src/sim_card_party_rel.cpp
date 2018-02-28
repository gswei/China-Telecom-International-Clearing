/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-11 下午7:37:34
Module:      sim_card_party_rel.cpp
Author:      瞿兆静
Revision:    $v1.0$
Description: SIM卡合作伙伴关系结构 for SimCardPartyRel
*/
#include "sim_card_party_rel.h"

CShmSimCardPartyRelMgr::CShmSimCardPartyRelMgr(): datas_( "SimCardPartyRel", 0 ), index_( "AccNbr_SimCardPartyRel", 0 ), mutex_( "SimCardPartyRel" )
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    datas_.CreateShm();
    index_.CreateShm();

    if( !datas_.Attach( false ) ) {
        printf( "SimCardPartyRel=>Attach failed，%s:%d\n", __FILE__, __LINE__ );
    }

    if( !index_.Attach( false ) ) {
        printf( "SimCardPartyRel=>Attach failed %s:%d\n", __FILE__, __LINE__ );
    }

    max_item_count_ = datas_.capacity();
    printf( "the shared memory can afford %d logs\n", max_item_count_ );
    MY_W_LOCK_END
}

CShmSimCardPartyRelMgr::~CShmSimCardPartyRelMgr()
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Detach() ) {
        printf( "SimCardPartyRel=>Attach failed，%s:%d\n", __FILE__, __LINE__ );
    }

    if( !index_.Detach() ) {
        printf( "SimCardPartyRel=>Attach failed %s:%d\n", __FILE__, __LINE__ );
    }

    MY_W_LOCK_END
}

bool CShmSimCardPartyRelMgr::PutData( const std::vector< SimCardPartyRelItem >& vec_sim_card_party_rel )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    std::vector<SimCardPartyRelItem>::const_iterator iter;
    pair<SimCardPartyRelIterator, bool> p;
    IndexAccNbr index_acc_nbr;

    for( iter = vec_sim_card_party_rel.begin(); iter != vec_sim_card_party_rel.end(); ++iter ) {
        //将数据插入到以ServId为索引的树中
        p = datas_.insert( *iter );

        //将数据插入到以OfferId为索引的树中
        if( p.first == datas_.end() ) {
            printf( "SimCardPartyRel=>index_acc_nbr插入失败, 空间已满，已加载\n" );
            ret = false;
            break;
        } else {
            strcpy( index_acc_nbr.acc_nbr, iter->acc_nbr );
            index_acc_nbr.sim_card_party_rel_iter = p.first;
            index_.insert( index_acc_nbr );
        }

        if( datas_.size() >= max_item_count_ ) {
            printf( "SimCardPartyRel=>the shared memory can afford %d logs\n", max_item_count_ );
        }
    }

    MY_W_LOCK_END
}

bool CShmSimCardPartyRelMgr::GetItemByCardSeq( const char* card_seq, SimCardPartyRelItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < SimCardPartyRelItem, MAX_PP - 1 >::const_iterator iter;
    SimCardPartyRelItem tmp;
    strcpy( tmp.card_seq, card_seq );
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

bool CShmSimCardPartyRelMgr::GetItemByAccNbr( const char* acc_nbr, SimCardPartyRelItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < IndexAccNbr, MAX_PP - 2 >::const_iterator iter;
    IndexAccNbr tmp;
    strcpy( tmp.acc_nbr, acc_nbr );
    iter = index_.find( tmp );

    if( -1 == iter.handle ) {
        ret = false;
    } else {
        item = *( iter->sim_card_party_rel_iter );
        ret = true;
    }

    MY_W_LOCK_END
    return ret;
}