/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-8 上午10:44:35
Module:      product_offer.h
Author:      瞿兆静
Revision:    $v1.0$
Description: 销售品内存结构 for ProductOffer
*/
#ifndef PRODUCT_OFFER_H_

#define PRODUCT_OFFER_H_

#include "shm_Set.h"
#include <vector>


#define MY_W_LOCK_BEGIN 	\
    if(mutex_.WLock())\
    {

#define MY_W_LOCK_END \
    }\
    else\
    {\
        std::cout<<GetName()<<"互斥失败： "<<mutex_.GetErrorMessage()<<endl;\
    }\
    \
    if(!mutex_.WUnLock())\
    {\
        std::cout<<GetName()<<"解锁失败 ： "<<mutex_.GetErrorMessage()<<endl;\
    }\
     
namespace tpss
{
struct ProductOfferItem {
    int offer_id;
    int band_id;
    int pricing_plan_id;
    int integral_pricing_plan_id;
    char offer_name[50];
    char offer_comments[250];
    char can_be_buy_alone;
    char offer_code[15];
    short int priority;
    char state[3];
    int  offer_type;
    Int64 base_fee;
    Int64 floor_fee;
    char  merge_flag;
    Int64 eff_date;
    Int64 exp_date;
    bool operator < ( const ProductOfferItem& rhs )const {
        if( offer_id < rhs.offer_id ) {
            return true;
        }

        return false;
    }
    std::string& toString( std::string& str )const {
        return str;
    }
};

class CShmProductOfferMgr
{
public:
    CShmProductOfferMgr() : datas_( "ProductOffer", 0 ), mutex_( "ProductOffer" ) {

    }
    ~CShmProductOfferMgr();
    bool Init( bool create_shm = false );
    bool LoadDataFromDB();
    std::string GetName()const {
        return std::string( "ProductOffer" );
    }
    bool PutData( const std::vector< ProductOfferItem >& vector_product_offer );
    bool GetItem( int offer_id, ProductOfferItem& item );
private:
    bool Insert( const ProductOfferItem& item );
    ns_shm_data::CManagedMutex mutex_;
    ns_shm_data::T_SHMSET_NO_MUTEX < ProductOfferItem, MAX_PP - 1 >  datas_;
};
};

#endif
