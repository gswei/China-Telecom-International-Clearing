/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-7 上午8:59:10
Module:      address_party_rel.cpp
Author:      瞿兆静
Revision:    $v1.0$
Description: 地址合作伙伴 for AddressPartyRel
*/
#include "address_party_rel.h"

#include "psutil.h"

using namespace tpss;

CShmAddressPartyRelMgr::CShmAddressPartyRelMgr(): datas_( "AddressPartyRel", 0 ), index_( "ChannelId_AddressPartyRel", 0 ), mutex_( "AddressPartyRel" )
{

}

CShmAddressPartyRelMgr::~CShmAddressPartyRelMgr()
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Detach() ) {
        printf( "AddressPartyRel=>Attach failed，%s:%d\n", __FILE__, __LINE__ );
    }

    if( !index_.Detach() ) {
        printf( "AddressPartyRel=>Attach failed %s:%d\n", __FILE__, __LINE__ );
    }

    MY_W_LOCK_END
}

bool CShmAddressPartyRelMgr::Init( bool create_shm )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( create_shm ) {
        if( !datas_.CreateShm() ) {
            ret = false;
        }

        if( !index_.CreateShm() ) {
            ret = false;
        }
    }

    if( !datas_.Attach( false ) ) {
        printf( "AddressPartyRel=>Attach failed，%s:%d\n", __FILE__, __LINE__ );
    }

    if( !index_.Attach( false ) ) {
        printf( "AddressPartyRel=>Attach failed %s:%d\n", __FILE__, __LINE__ );
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmAddressPartyRelMgr::LoadDataFromDB()
{
    DBConnection conn;
    bool ret = true;

    if( dbConnect( conn ) ) {
        Statement stmt = conn.createStatement();
        std::string sql = "select ADDR_PARTY_REL_ID, ADDRESS_ID, PARTY_ID, "
                          "CHANNEL_ID, EFF_DATE, EXP_DATE from ADDRESS_PARTY_REL";
        stmt.setSQLString( sql );

        if( stmt.execute() ) {
            AddressPartyRelItem tmp;
            mutex_.AttachOrCreateMutexIfNeed();
            MY_W_LOCK_BEGIN

            while( stmt >> tmp.addr_party_rel_id >> tmp.address_id
                    >> tmp.party_id >> tmp.channel_id
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

bool CShmAddressPartyRelMgr::PutData( const std::vector< AddressPartyRelItem >& vec_address_party_rel )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    std::vector<AddressPartyRelItem>::const_iterator iter;
    pair<AddressPartyRelIterator, bool> p;
    IndexChannelId index_channel_id;

    for( iter = vec_address_party_rel.begin(); iter != vec_address_party_rel.end(); ++iter ) {
        Insert( *iter );
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmAddressPartyRelMgr::GetItemByAddressId( Int64 address_id, AddressPartyRelItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < AddressPartyRelItem, MAX_PP - 1 >::const_iterator iter;
    AddressPartyRelItem tmp;
    tmp.address_id = address_id;
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

bool CShmAddressPartyRelMgr::GetItemByChannelId( int channel_id, AddressPartyRelItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < IndexChannelId, MAX_PP - 2 >::const_iterator iter;
    IndexChannelId tmp;
    tmp.channel_id = channel_id;
    iter = index_.find( tmp );

    if( -1 == iter.handle ) {
        ret = false;
    } else {
        item = *( iter->address_party_rel_iter );
        ret = true;
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmAddressPartyRelMgr::Insert( const AddressPartyRelItem& item )
{
    bool ret = true;
    pair<AddressPartyRelIterator, bool> p;
    IndexChannelId index_channel_id;
    //将数据插入到以AddressId为索引的树中
    p = datas_.insert( item );

    //将数据插入到以ChannelId为索引的树中
    if( p.first == datas_.end() ) {
        return false;
    } else {
        index_channel_id.channel_id = item.channel_id;
        index_channel_id.address_party_rel_iter = p.first;

        if( index_.insert( index_channel_id ).first == index_.end() ) {
            return false;
        }
    }

    return true;
}
