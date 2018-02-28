/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-11 下午8:33:26
Module:      channel_equip_rel.h
Author:      瞿兆静
Revision:    $v1.0$
Description: 网点设备关系结构 for ChannelEquipRel
*/

#ifndef CHANNEL_EQUIP_REL_H_

#define CHANNEL_EQUIP_REL_H_

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
struct ChannelEquipRelItem {
    Int64 	equip_channel_rel_id;
    char 	optical_cable_code[50];
    char	trunk_line_code[50];
    char   	sub_line_box_code[50];
    char 	junction_box_code[50];
    int		channel_id;
    int		party_id;
    bool operator < ( const ChannelEquipRelItem& rhs )const {
        return channel_id < rhs.channel_id;
    }
    std::string& toString( std::string& str )const {
        return str;
    }
};

class CShmChannelEquipRelMgr
{
public:
    CShmChannelEquipRelMgr::CShmChannelEquipRelMgr() : datas_( "ChannelEquipRel", 0 ), index_sub_line_box_code_( "SubLine_ChannelEquipRel", 0 ),
        index_junction_box_code_( "Junction_ChannelEquipRel", 0 ), mutex_( "ChannelEquipRel" ) {
    }
    ~CShmChannelEquipRelMgr();
    bool Init( bool create_shm = false );
    bool LoadDataFromDB();
    std::string GetName()const {
        return std::string( "ChannelEquipRel" );
    }
    bool PutData( const std::vector< ChannelEquipRelItem >& vec_channel_equip_rel );
    bool GetItemByChannelId( int channel_id, ChannelEquipRelItem& item );
    bool GetItemBySubLineBoxCode( const char* sub_line_box_code, ChannelEquipRelItem& item );
    bool GetItemByJunctionBoxCode( const char* junction_box_code, ChannelEquipRelItem& item );
public:
    typedef ns_shm_data::T_SHMSET_NO_MUTEX < ChannelEquipRelItem, MAX_PP - 1 >::iterator ChannelEquipRelIterator;
    struct IndexSubLineBoxCode {
        char sub_line_box_code[50];
        ChannelEquipRelIterator channel_equip_rel_iter;
        bool operator < ( const IndexSubLineBoxCode& rhs )const {
            return strcmp( sub_line_box_code, rhs.sub_line_box_code ) == -1;
        }
        std::string& toString( std::string& str )const {
            return str;
        }
    };
    struct IndexJunctionBoxCode {
        char junction_box_code[50];
        ChannelEquipRelIterator channel_equip_rel_iter;
        bool operator < ( const IndexJunctionBoxCode& rhs )const {
            return strcmp( junction_box_code, rhs.junction_box_code ) == -1;
        }
        std::string& toString( std::string& str )const {
            return str;
        }
    };
private:
    bool Insert( const ChannelEquipRelItem& item );
    ns_shm_data::CManagedMutex mutex_;
    ns_shm_data::T_SHMSET_NO_MUTEX < ChannelEquipRelItem, MAX_PP - 1 >  datas_;
    ns_shm_data::T_SHMSET_NO_MUTEX < IndexSubLineBoxCode, MAX_PP - 2 >  index_sub_line_box_code_;
    ns_shm_data::T_SHMSET_NO_MUTEX < IndexJunctionBoxCode, MAX_PP - 3 > index_junction_box_code_;
};
};

#endif
