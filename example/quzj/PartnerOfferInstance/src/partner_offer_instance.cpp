/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-8 上午11:26:50
Module:      partner_offer_instance.cpp
Author:      瞿兆静
Revision:    $Id: partner_offer_instance.cpp 1736 2012-10-26 01:51:20Z zhenghb $
Description: 合作伙伴与销售品关系内存结构 for PartnerOfferInstance
*/
#include "partner_offer_instance.h"

#include "psutil.h"

using namespace tpss;


CShmPartnerOfferInstanceMgr::~CShmPartnerOfferInstanceMgr()
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Detach() ) {
        printf( "PartnerOfferInstance=>Detach failed，%s:%d\n", __FILE__, __LINE__ );
    }

    MY_W_LOCK_END
}

bool CShmPartnerOfferInstanceMgr::Init( bool create_shm )
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

bool CShmPartnerOfferInstanceMgr::LoadDataFromDB()
{
    DBConnection conn;
    bool ret = true;

    if( dbConnect( conn ) ) {
        Statement stmt = conn.createStatement();
        std::string sql = "select PARTY_OFFER_INST_ID, OBJ_TYPE, OBJ_ID, OFFER_ID, EFF_DATE, "
                          "EXP_DATE from PARTNER_OFFER_INSTANCE";
        stmt.setSQLString( sql );

        if( stmt.execute() ) {
            PartnerOfferInstanceItem tmp;
            mutex_.AttachOrCreateMutexIfNeed();
            MY_W_LOCK_BEGIN

            while( stmt >> tmp.party_offer_inst_id >> tmp.obj_type
                    >> tmp.obj_id >> tmp.eff_date >> tmp.exp_date ) {
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

bool CShmPartnerOfferInstanceMgr::PutData( const std::vector< PartnerOfferInstanceItem >& vec_serv_equip )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    std::vector<PartnerOfferInstanceItem>::const_iterator iter;

    for( iter = vec_serv_equip.begin(); iter != vec_serv_equip.end(); ++iter ) {
        if( !Insert( *iter ) ) {
            ret = false;
        }
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmPartnerOfferInstanceMgr::GetItem( int obj_id, int obj_type, PartnerOfferInstanceItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < PartnerOfferInstanceItem, MAX_PP - 1 >::const_iterator iter;
    PartnerOfferInstanceItem tmp;
    tmp.obj_type = obj_type;
    tmp.obj_id = obj_id;
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

bool CShmPartnerOfferInstanceMgr::Insert( const PartnerOfferInstanceItem& item )
{
    return datas_.insert( item ).first != datas_.end();
}
