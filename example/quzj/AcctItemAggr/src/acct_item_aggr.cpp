/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-12 上午10:06:15
Module:      acct_item_aggr.cpp
Author:      瞿兆静
Revision:    $v1.0$
Description: 用户出账信息结构 for AcctItemAggr
*/
#include "acct_item_aggr.h"
#include "psutil.h"

using namespace tpss;

CShmAcctItemAggrMgr::~CShmAcctItemAggrMgr()
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Detach() ) {
        printf( "AcctItemAggr=>Attach failed，%s:%d\n", __FILE__, __LINE__ );
    }

    if( !index_.Detach() ) {
        printf( "AcctItemAggr=>Attach failed %s:%d\n", __FILE__, __LINE__ );
    }

    MY_W_LOCK_END
}

bool CShmAcctItemAggrMgr::Init( bool create_shm )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( create_shm ) {
        if( !datas_.CreateShm() ) {
            printf( "%s文件%d行:datas创建共享内存失败!\n", __FILE__, __LINE__ );
            ret = false;
        }

        if( !index_.CreateShm() ) {
            printf( "%s文件%d行:index创建共享内存失败!\n", __FILE__, __LINE__ );
            ret = false;
        }
    }

    if( !datas_.Attach( false ) ) {
        printf( "%s文件%d行:将数据加载到datas失败!\n", __FILE__, __LINE__ );
        ret = false;
    }

    if( !index_.Attach( false ) ) {
        printf( "%s文件%d行:将数据加载到index_失败!\n", __FILE__, __LINE__ );
        ret = false;
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmAcctItemAggrMgr::LoadDataFromDB()
{
    DBConnection conn;
    bool ret = true;

    if( dbConnect( conn ) ) {
        Statement stmt = conn.createStatement();
        std::string sql = "select ACCT_ITEM_ID, SERV_ID, BILLING_CYCLE_ID, "
                          "ACCT_ITEM_TYPE_ID, CHARGE, ACCT_ID, OFFER_INST_ID, "
                          "ITEM_SOURCE_ID, OFFER_ID, CREATE_DATE from ACCT_ITEM_AGGR";
        stmt.setSQLString( sql );

        if( stmt.execute() ) {
            AcctItemAggrItem tmp;
            mutex_.AttachOrCreateMutexIfNeed();
            MY_W_LOCK_BEGIN

            while( stmt >> tmp.acct_item_id >> tmp.serv_id
                    >> tmp.billing_cycle_id >> tmp.acct_item_type_id
                    >> tmp.charge >> tmp.acct_id >> tmp.offer_inst_id
                    >> tmp.item_source_id >> tmp.offer_id >> tmp.create_date ) {
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

bool CShmAcctItemAggrMgr::PutData( const std::vector< AcctItemAggrItem >& vec_acct_item_aggr )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    std::vector<AcctItemAggrItem>::const_iterator iter;

    for( iter = vec_acct_item_aggr.begin(); iter != vec_acct_item_aggr.end(); ++iter ) {
        if( false == Insert( *iter ) ) {
            printf( "%s文件%d行: 插入数据失败!\n", __FILE__, __LINE__ );
            ret = false;
            break;
        }
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmAcctItemAggrMgr::GetItemByServId( Int64 serv_id, AcctItemAggrItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < AcctItemAggrItem, MAX_PP - 1 >::const_iterator iter;
    AcctItemAggrItem tmp;
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

bool CShmAcctItemAggrMgr::GetItemByOfferInstId( Int64 offer_inst_id, AcctItemAggrItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < IndexOfferInstId, MAX_PP - 2 >::const_iterator iter;
    IndexOfferInstId tmp;
    tmp.offer_inst_id = offer_inst_id;
    iter = index_.find( tmp );

    if( -1 == iter.handle ) {
        ret = false;
    } else {
        item = *( iter->acct_item_aggr_iter );
        ret = true;
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmAcctItemAggrMgr::Insert( const AcctItemAggrItem& item )
{
    pair<AcctItemAggrIterator, bool> p;
    IndexOfferInstId index_offer_inst_id;
    //将数据插入到以ServId为索引的树中
    p = datas_.insert( item );

    //将数据插入到以OfferInstId为索引的树中
    if( p.first == datas_.end() ) {
        return false;
    } else {
        index_offer_inst_id.offer_inst_id = item.offer_inst_id;
        index_offer_inst_id.acct_item_aggr_iter = p.first;

        if( index_.insert( index_offer_inst_id ).first == index_.end() ) {
            return false;
        }
    }

    return true;
}
