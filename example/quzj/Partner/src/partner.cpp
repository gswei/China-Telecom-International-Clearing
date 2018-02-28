/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-7 上午10:55:30
Module:      partner.cpp
Author:      瞿兆静
Revision:    $v1.0$
Description: 合作伙伴 for Partner
*/
#include "partner.h"

#include "psutil.h"

using namespace tpss;

CShmPartnerMgr::~CShmPartnerMgr()
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Detach() ) {
        printf( "Partner=>Detach failed，%s:%d\n", __FILE__, __LINE__ );
    }

    MY_W_LOCK_END
}

bool CShmPartnerMgr::Init( bool create_shm )
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

bool CShmPartnerMgr::LoadDataFromDB()
{
    DBConnection conn;
    bool ret = true;

    if( dbConnect( conn ) ) {
        Statement stmt = conn.createStatement();
        std::string sql = "select PARTY_ID, PARD_CODE, PARD_TYPE, PARD_NAME, "
                          "PARD_DESC, LANT_ID, CONTACT_PERSON, CONTACT_PHONE, "
                          "SHOPCARD_NUM, CORPORATION_ID_NO, CORPORATION_PHONE, "
                          "ADDRESS_INFO, STATE, EFF_DATE, EXP_DATE from PARTNER";
        stmt.setSQLString( sql );

        if( stmt.execute() ) {
            PartnerItem tmp;
            mutex_.AttachOrCreateMutexIfNeed();
            MY_W_LOCK_BEGIN

            while( stmt >> tmp.party_id >> tmp.pard_code
                    >> tmp.pard_type >> tmp.pard_name
                    >> tmp.pard_desc >> tmp.lant_id >> tmp.contact_person
                    >> tmp.contact_phone >> tmp.shopcard_num >> tmp.corporation_id_no
                    >> tmp.corporation_phone >> tmp.address_info >> tmp.state
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

bool CShmPartnerMgr::PutData( const std::vector< PartnerItem >& vec_partner_item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    std::vector<PartnerItem>::const_iterator iter;
    string str;

    for( iter = vec_partner_item.begin(); iter != vec_partner_item.end(); ++iter ) {
        if( Insert( *iter ) ) {
            ret = false;
        }
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmPartnerMgr::GetItem( int party_id, PartnerItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < PartnerItem, MAX_PP - 1 >::const_iterator iter;
    PartnerItem tmp;
    tmp.party_id = party_id;
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

bool CShmPartnerMgr::Insert( const PartnerItem& item )
{
    return datas_.insert( item ).first != datas_.end();
}