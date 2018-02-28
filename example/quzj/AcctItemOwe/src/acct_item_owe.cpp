/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-6 下午7:04:24
Module:      acct_item_owe.cpp
Author:      瞿兆静
Revision:    $v1.0$
Description: 用户欠费账目 for AcctItemOwe
*/
#include "acct_item_owe.h"
#include "psutil.h"

using namespace tpss;

CShmAcctItemOweMgr::~CShmAcctItemOweMgr()
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !index_.Detach() ) {
        printf( "AcctItemOwe=>Detach failed，%s:%d\n", __FILE__, __LINE__ );
    }

    if( !datas_.Detach() ) {
        printf( "AcctItemOwe=>Detach failed，%s:%d\n", __FILE__, __LINE__ );
    }

    MY_W_LOCK_END
}

bool CShmAcctItemOweMgr::Init( bool create_shm )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    datas_.CreateShm();
    index_.CreateShm();

    if( !datas_.Attach( false ) ) {
        printf( "AcctItemOwe=>Attach failed，%s:%d\n", __FILE__, __LINE__ );
        ret = false;
    }

    if( !index_.Attach( false ) ) {
        printf( "AcctItemOwe=>Attach failed %s:%d\n", __FILE__, __LINE__ );
        ret = false;
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmAcctItemOweMgr::LoadDataFromDB()
{
    DBConnection conn;
    bool ret = true;

    if( dbConnect( conn ) ) {
        Statement stmt = conn.createStatement();
        std::string sql = "select ACCT_ITEM_ID, SERV_ID, ACCT_ID, "
                          "ITEM_SOURCE_ID, BILLING_CYCLE_ID, ACCT_ITEM_TYPE_ID, AMOUNT, "
                          "BILL_ID, PAYMENT_METHOD, OFFER_ID, FEE_CYCLE_ID, STATE from ACCT_ITEM_OWE";
        stmt.setSQLString( sql );

        if( stmt.execute() ) {
            AcctItemOweItem tmp;
            mutex_.AttachOrCreateMutexIfNeed();
            MY_W_LOCK_BEGIN

            while( stmt >> tmp.acct_item_id >> tmp.serv_id
                    >> tmp.acct_id >> tmp.item_source_id
                    >> tmp.billing_cycle_id >> tmp.acct_item_type_id >> tmp.amount
                    >> tmp.bill_id >> tmp.payment_method >> tmp.offer_id
                    >> tmp.state ) {
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

bool CShmAcctItemOweMgr::PutData( const std::vector< AcctItemOweItem >& vec_term_reg_item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    std::vector<AcctItemOweItem>::const_iterator iter;
    pair<AcctOweIterator, bool> p;
    IndexOfferId index_offer_id;

    for( iter = vec_term_reg_item.begin(); iter != vec_term_reg_item.end(); ++iter ) {
        if( !Insert( *iter ) ) {
            ret = false;
        }
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmAcctItemOweMgr::GetItemByServId( Int64 serv_id, AcctItemOweItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < AcctItemOweItem, MAX_PP - 1 >::const_iterator iter;
    AcctItemOweItem tmp;
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

bool CShmAcctItemOweMgr::GetItemByOfferId( int offer_id, AcctItemOweItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < IndexOfferId, MAX_PP - 2 >::const_iterator iter;
    IndexOfferId tmp;
    tmp.offer_id = offer_id;
    iter = index_.find( tmp );

    if( -1 == iter.handle ) {
        ret = false;
    } else {
        item = *( iter->acct_owe_iterator );
        ret = true;
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmAcctItemOweMgr::Insert( const AcctItemOweItem& item )
{
    pair<AcctOweIterator, bool> p;
    IndexOfferId index_offer_id;
    //将数据插入到以ServId为索引的树中
    p = datas_.insert( item );

    //将数据插入到以OfferInstId为索引的树中
    if( p.first == datas_.end() ) {
        return false;
    } else {
        index_offer_id.offer_id = item.offer_id;
        index_offer_id.acct_owe_iterator = p.first;

        if( index_.insert( index_offer_id ).first == index_.end() ) {
            return false;
        }
    }

    return true;
}