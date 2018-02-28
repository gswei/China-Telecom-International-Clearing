/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-12 上午11:01:55
Module:      channel.cpp
Author:      瞿兆静
Revision:    $v1.0$
Description: 网点资料结构 for Channel
*/
#include "channel.h"

#include "psutil.h"

using namespace tpss;

CShmChannelMgr::~CShmChannelMgr()
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Detach() ) {
        printf( "Channel=>Attach failed，%s:%d\n", __FILE__, __LINE__ );
    }

    if( !index_.Detach() ) {
        printf( "Channel=>Attach failed %s:%d\n", __FILE__, __LINE__ );
    }

    MY_W_LOCK_END
}

bool CShmChannelMgr::Init( bool create_shm )
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
        ret = false;
    }

    if( !index_.Attach( false ) ) {
        ret = false;
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmChannelMgr::LoadDataFromDB()
{
    DBConnection conn;
    bool ret = true;

    if( dbConnect( conn ) ) {
        Statement stmt = conn.createStatement();
        std::string sql = "select CHANNEL_ID, CHANNEL_NAME, CHANNEL_LEVEL_CD, "
                          "CHANNEL_TYPE_CD, CHANNEL_SUBTYPE_CD, STATUS, PARENT_CHN_ID, "
                          "CHANNEL_NBR, PARTY_ID, LANT_ID from CHANNEL";
        stmt.setSQLString( sql );

        if( stmt.execute() ) {
            ChannelItem tmp;
            mutex_.AttachOrCreateMutexIfNeed();
            MY_W_LOCK_BEGIN

            while( stmt >> tmp.channel_id >> tmp.channel_name
                    >> tmp.channel_level_cd >> tmp.channel_type_cd
                    >> tmp.channel_subtype_cd >> tmp.status >> tmp.parent_chn_id
                    >> tmp.channel_nbr >> tmp.party_id >> tmp.lant_id ) {
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
bool CShmChannelMgr::PutData( const std::vector< ChannelItem >& vec_channel )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    std::vector<ChannelItem>::const_iterator iter;
    pair<ChannelIterator, bool> p;
    IndexPartyId index_party_id;

    for( iter = vec_channel.begin(); iter != vec_channel.end(); ++iter ) {
        if( !Insert( *iter ) ) {
            ret = false;
            break;
        }
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmChannelMgr::GetItemByChannelId( int channel_id, ChannelItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < ChannelItem, MAX_PP - 1 >::const_iterator iter;
    ChannelItem tmp;
    tmp.channel_id = channel_id;
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


bool CShmChannelMgr::GetItemByPartyId( int party_id, ChannelItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < IndexPartyId, MAX_PP - 2 >::const_iterator iter;
    IndexPartyId tmp;
    tmp.party_id = party_id;
    iter = index_.find( tmp );

    if( -1 == iter.handle ) {
        ret = false;
    } else {
        item = *( iter->channel_iter );
        ret = true;
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmChannelMgr::Insert( const ChannelItem& item )
{
    pair<ChannelIterator, bool> p;
    IndexPartyId index_party_id;
    //将数据插入到以ServId为索引的树中
    p = datas_.insert( item );

    //将数据插入到以PartyId为索引的树中
    if( p.first == datas_.end() ) {
        return false;
    } else {
        index_party_id.party_id = item.party_id;
        index_party_id.channel_iter = p.first;

        if( index_.insert( index_party_id ).first == index_.end() ) {
            return false;
        }
    }

    return true;
}
