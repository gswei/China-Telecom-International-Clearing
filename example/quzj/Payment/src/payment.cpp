/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-7 下午4:53:59
Module:      payment.cpp
Author:      瞿兆静
Revision:    $v1.0$
Description: 付款记录 for Payment
*/
#include "payment.h"

#include "psutil.h"

using namespace tpss;

CShmPaymentMgr::~CShmPaymentMgr()
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Detach() ) {
        printf( "Payment=>Detach failed，%s:%d\n", __FILE__, __LINE__ );
    }

    MY_W_LOCK_END
}

bool CShmPaymentMgr::Init( bool create_shm )
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

bool CShmPaymentMgr::LoadDataFromDB()
{
    DBConnection conn;
    bool ret = true;

    if( dbConnect( conn ) ) {
        Statement stmt = conn.createStatement();
        std::string sql = "select PAYMENT_ID, ACCT_ID, PAYMENT_METHOD, "
                          "OPERATION_TYPE, AMOUNT, PAYMENT_DATE, STATE, "
                          "SERV_ID, ACC_NBR, PAY_CYCLE_ID from PAYMENT";
        stmt.setSQLString( sql );

        if( stmt.execute() ) {
            PaymentItem tmp;
            mutex_.AttachOrCreateMutexIfNeed();
            MY_W_LOCK_BEGIN

            while( stmt >> tmp.payment_id >> tmp.acct_id
                    >> tmp.payment_method >> tmp.operation_type
                    >> tmp.amount >> tmp.payment_date >> tmp.state
                    >> tmp.serv_id >> tmp.acc_nbr >> tmp.pay_cycle_id ) {
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

bool CShmPaymentMgr::PutData( const std::vector< PaymentItem >& vec_payment )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    std::vector<PaymentItem>::const_iterator iter;

    for( iter = vec_payment.begin(); iter != vec_payment.end(); ++iter ) {
        if( !Insert( *iter ) ) {
            ret = false;
        }
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmPaymentMgr::GetItem( Int64 serv_id, PaymentItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < PaymentItem, MAX_PP - 1 >::const_iterator iter;
    PaymentItem tmp;
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

bool CShmPaymentMgr::Insert( const PaymentItem& item )
{
    return datas_.insert( item ).first != datas_.end();
}
