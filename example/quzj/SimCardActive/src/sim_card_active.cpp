/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-12 ����3:24:42
Module:      sim_card_active.cpp
Author:      ���׾�
Revision:    $v1.0$
Description:  SIM��������Ϣ�ṹ for SimCardActive
*/
#include "sim_card_active.h"

CShmSimCardActiveMgr::CShmSimCardActiveMgr(): datas_( "SimCardActive", 0 ), index_( "AccNbr_SimCardActive", 0 ), mutex_( "SimCardActive" )
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    datas_.CreateShm();
    index_.CreateShm();

    if( !datas_.Attach( false ) ) {
        printf( "SimCardActive=>Attach failed��%s:%d\n", __FILE__, __LINE__ );
    }

    if( !index_.Attach( false ) ) {
        printf( "SimCardActive=>Attach failed %s:%d\n", __FILE__, __LINE__ );
    }

    max_item_count_ = datas_.capacity();
    printf( "the shared memory can afford %d logs\n", max_item_count_ );
    MY_W_LOCK_END
}

CShmSimCardActiveMgr::~CShmSimCardActiveMgr()
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Detach() ) {
        printf( "SimCardActive=>Attach failed��%s:%d\n", __FILE__, __LINE__ );
    }

    if( !index_.Detach() ) {
        printf( "SimCardActive=>Attach failed %s:%d\n", __FILE__, __LINE__ );
    }

    MY_W_LOCK_END
}

bool CShmSimCardActiveMgr::PutData( const std::vector< SimCardActiveItem >& vec_sim_card_active )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    std::vector<SimCardActiveItem>::const_iterator iter;
    pair<SimCardActiveIterator, bool> p;
    IndexAccNbr index_acc_nbr;

    for( iter = vec_sim_card_active.begin(); iter != vec_sim_card_active.end(); ++iter ) {
        //�����ݲ��뵽��ServIdΪ����������
        p = datas_.insert( *iter );

        //�����ݲ��뵽��OfferIdΪ����������
        if( p.first == datas_.end() ) {
            printf( "SimCardActive=>index_acc_nbr����ʧ��, �ռ��������Ѽ���\n" );
            ret = false;
            break;
        } else {
            strcpy( index_acc_nbr.acc_nbr, iter->acc_nbr );
            index_acc_nbr.sim_card_active_iter = p.first;
            index_.insert( index_acc_nbr );
        }

        if( datas_.size() >= max_item_count_ ) {
            printf( "SimCardActive=>the shared memory can afford %d logs\n", max_item_count_ );
        }
    }

    MY_W_LOCK_END
}

bool CShmSimCardActiveMgr::GetItemByServId( Int64 serv_id, SimCardActiveItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < SimCardActiveItem, MAX_PP - 1 >::const_iterator iter;
    SimCardActiveItem tmp;
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

bool CShmSimCardActiveMgr::GetItemByAccNbr( const char* acc_nbr, SimCardActiveItem& item )
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
        item = *( iter->sim_card_active_iter );
        ret = true;
    }

    MY_W_LOCK_END
    return ret;
}
