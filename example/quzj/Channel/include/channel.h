/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-12 上午11:01:46
Module:      channel.h
Author:      瞿兆静
Revision:    $v1.0$
Description: 网点资料结构 for Channel
*/
#ifndef CHANNEL_H_

#define CHANNEL_H_

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
struct ChannelItem {
    int 	channel_id;
    char 	channel_name[50];
    char	channel_level_cd[3];
    char   	channel_type_cd[6];
    char	channel_subtype_cd[10];
    int 	status;
    int		parent_chn_id;
    char	channel_nbr[30];
    int		party_id;
    int		lant_id;
    bool operator < ( const ChannelItem& rhs )const {
        return channel_id < rhs.channel_id;
    }
    std::string& toString( std::string& str )const {
        return str;
    }
};

class CShmChannelMgr
{
public:
    CShmChannelMgr(): datas_( "Channel", 0 ), index_( "PartyId_Channel", 0 ), mutex_( "Channel" ) {

    }
    ~CShmChannelMgr();
    bool Init( bool create_shm );
    bool LoadDataFromDB();
    std::string GetName()const {
        return std::string( "Channel" );
    }
    bool PutData( const std::vector< ChannelItem >& vec_channel );
    bool GetItemByChannelId( int channel_id, ChannelItem& item );
    bool GetItemByPartyId( int party_id, ChannelItem& item );
public:
    typedef ns_shm_data::T_SHMSET_NO_MUTEX < ChannelItem, MAX_PP - 1 >::iterator ChannelIterator;
    struct IndexPartyId {
        int party_id;
        ChannelIterator channel_iter;
        bool operator < ( const IndexPartyId& rhs )const {
            return party_id < rhs.party_id;
        }
        std::string& toString( std::string& str )const {
            return str;
        }
    };
private:
    bool Insert( const ChannelItem& item );
    ns_shm_data::CManagedMutex mutex_;
    ns_shm_data::T_SHMSET_NO_MUTEX < ChannelItem, MAX_PP - 1 >  datas_;
    ns_shm_data::T_SHMSET_NO_MUTEX < IndexPartyId, MAX_PP - 2 >  index_;
};
};


#endif
