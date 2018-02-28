/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-11 ����8:34:02
Module:      channel_equip_rel.cpp
Author:      ���׾�
Revision:    $v1.0$
Description: �����豸��ϵ�ṹ for ChannelEquipRel
*/

#include "channel_equip_rel.h"

#include "psutil.h"

using namespace tpss;

CShmChannelEquipRelMgr::~CShmChannelEquipRelMgr()
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Detach() ) {
        printf( "ChannelEquipRel=>Attach failed��%s:%d\n", __FILE__, __LINE__ );
    }

    if( !index_sub_line_box_code_.Detach() ) {
        printf( "ChannelEquipRel=>Attach failed %s:%d\n", __FILE__, __LINE__ );
    }

    if( !index_junction_box_code_.Detach() ) {
        printf( "ChannelEquipRel=>Attach failed %s:%d\n", __FILE__, __LINE__ );
    }

    MY_W_LOCK_END
}

bool CShmChannelEquipRelMgr::Init( bool create_shm )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( create_shm ) {
        if( !datas_.CreateShm() ) {
            ret = false;
        }

        if( !index_sub_line_box_code_.CreateShm() ) {
            ret = false;
        }

        if( !index_junction_box_code_.CreateShm() ) {
            ret = false;
        }
    }

    if( !datas_.Attach( false ) ) {
        ret = false;
    }

    if( !index_sub_line_box_code_.Attach( false ) ) {
        ret = false;
    }

    if( !index_junction_box_code_.Attach( false ) ) {
        ret = false;
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmChannelEquipRelMgr::LoadDataFromDB()
{
    DBConnection conn;
    bool ret = true;

    if( dbConnect( conn ) ) {
        Statement stmt = conn.createStatement();
        std::string sql = "select EQUIP_CHANNEL_REL_ID, OPTICAL_CABLE_CODE, TRUNK_LINE_CODE, "
                          "SUB_LINE_BOX_CODE, JUNCTION_BOX_CODE, CHANNEL_ID, PARTY_ID "
                          "from CHANNEL_EQUIP_REL";
        stmt.setSQLString( sql );

        if( stmt.execute() ) {
            ChannelEquipRelItem tmp;
            mutex_.AttachOrCreateMutexIfNeed();
            MY_W_LOCK_BEGIN

            while( stmt >> tmp.equip_channel_rel_id >> tmp.optical_cable_code
                    >> tmp.trunk_line_code >> tmp.sub_line_box_code
                    >> tmp.junction_box_code >> tmp.channel_id >> tmp.party_id ) {
                if( false == Insert( tmp ) ) {
                    printf( "%s�ļ�%d��: ��������ʧ��!\n", __FILE__, __LINE__ );
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

bool CShmChannelEquipRelMgr::PutData( const std::vector< ChannelEquipRelItem >& vec_channel_equip_rel )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    std::vector<ChannelEquipRelItem>::const_iterator iter;
    pair<ChannelEquipRelIterator, bool> p;
    IndexSubLineBoxCode index_sub_line_box_code;
    IndexJunctionBoxCode index_junction_box_code;

    for( iter = vec_channel_equip_rel.begin(); iter != vec_channel_equip_rel.end(); ++iter ) {
        if( !Insert( *iter ) ) {
            ret = false;
            break;
        }
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmChannelEquipRelMgr::GetItemByChannelId( int channel_id, ChannelEquipRelItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < ChannelEquipRelItem, MAX_PP - 1 >::const_iterator iter;
    ChannelEquipRelItem tmp;
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

bool CShmChannelEquipRelMgr::GetItemBySubLineBoxCode( const char* sub_line_box_code, ChannelEquipRelItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < IndexSubLineBoxCode, MAX_PP - 2 >::const_iterator iter;
    IndexSubLineBoxCode tmp;
    strcpy( tmp.sub_line_box_code, sub_line_box_code );
    iter = index_sub_line_box_code_.find( tmp );

    if( -1 == iter.handle ) {
        ret = false;
    } else {
        item = *( iter->channel_equip_rel_iter );
        ret = true;
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmChannelEquipRelMgr::GetItemByJunctionBoxCode( const char* junction_box_code, ChannelEquipRelItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < IndexJunctionBoxCode, MAX_PP - 3 >::const_iterator iter;
    IndexJunctionBoxCode tmp;
    strcpy( tmp.junction_box_code, junction_box_code );
    iter = index_junction_box_code_.find( tmp );

    if( -1 == iter.handle ) {
        ret = false;
    } else {
        item = *( iter->channel_equip_rel_iter );
        ret = true;
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmChannelEquipRelMgr::Insert( const ChannelEquipRelItem& item )
{
    pair<ChannelEquipRelIterator, bool> p;
    IndexSubLineBoxCode index_sub_line_box_code;
    IndexJunctionBoxCode index_junction_box_code;
    //�����ݲ��뵽��Channel_IdΪ����������
    p = datas_.insert( item );

    if( p.first == datas_.end() ) {
        return false;
    }

    //�����ݲ��뵽��index_sub_line_box_code������
    strcpy( index_sub_line_box_code.sub_line_box_code, item.sub_line_box_code );
    index_sub_line_box_code.channel_equip_rel_iter = p.first;

    if( index_sub_line_box_code_.insert( index_sub_line_box_code ).first ==
            index_sub_line_box_code_.end() ) {
        return false;
    }

    strcpy( index_junction_box_code.junction_box_code, item.junction_box_code );
    index_junction_box_code.channel_equip_rel_iter = p.first;

    if( index_junction_box_code_.insert( index_junction_box_code ).first ==
            index_junction_box_code_.end() ) {
        return false;
    }

    return true;
}
