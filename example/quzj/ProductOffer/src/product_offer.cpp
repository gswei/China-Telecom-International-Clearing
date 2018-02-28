/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-8 上午10:44:59
Module:      product_offer.cpp
Author:      瞿兆静
Revision:    $v1.0$
Description: 销售品内存结构 for ProductOffer
*/
#include "product_offer.h"

#include "psutil.h"

using namespace tpss;

CShmProductOfferMgr::~CShmProductOfferMgr()
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Detach() ) {
        printf( "ProductOffer=>Detach failed，%s:%d\n", __FILE__, __LINE__ );
    }

    MY_W_LOCK_END
}

bool CShmProductOfferMgr::Init( bool create_shm )
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

bool CShmProductOfferMgr::LoadDataFromDB()
{
    DBConnection conn;
    bool ret = true;

    if( dbConnect( conn ) ) {
        Statement stmt = conn.createStatement();
        std::string sql = "select OFFER_ID, BAND_ID, PRICING_PLAN_ID, INTEGRAL_PRICING_PLAN_ID, "
                          "OFFER_NAME, OFFER_COMMENTS, CAN_BE_BUY_ALONE, OFFER_CODE, "
                          "PRIORITY, STATE, OFFER_TYPE, BASE_FEE, FLOOR_FEE, MERGE_FLAG, "
                          "EFF_DATE, EXP_DATE from PRODUCT_OFFER";
        stmt.setSQLString( sql );

        if( stmt.execute() ) {
            ProductOfferItem tmp;
            mutex_.AttachOrCreateMutexIfNeed();
            MY_W_LOCK_BEGIN

            while( stmt >> tmp.offer_id >> tmp.band_id >> tmp.pricing_plan_id
                    >> tmp.integral_pricing_plan_id >> tmp.offer_name >> tmp.offer_comments
                    >> tmp.can_be_buy_alone >> tmp.offer_code
                    >> tmp.priority >> tmp.state >> tmp.offer_type
                    >> tmp.base_fee >> tmp.floor_fee >> tmp.merge_flag
                    >> tmp.eff_date >> tmp.exp_date ) {
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

bool CShmProductOfferMgr::PutData( const std::vector< ProductOfferItem >& vec_product_offer )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    std::vector<ProductOfferItem>::const_iterator iter;

    for( iter = vec_product_offer.begin(); iter != vec_product_offer.end(); ++iter ) {
        if( !Insert( *iter ) ) {
            ret = false;
            break;
        }
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmProductOfferMgr::GetItem( int offer_id, ProductOfferItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < ProductOfferItem, MAX_PP - 1 >::const_iterator iter;
    ProductOfferItem tmp;
    tmp.offer_id = offer_id;
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

bool CShmProductOfferMgr::Insert( const ProductOfferItem& item )
{
    return datas_.insert( item ).first != datas_.end();
}
