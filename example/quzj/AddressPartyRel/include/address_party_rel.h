/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-7 ÉÏÎç8:58:56
Module:      address_party_rel.h
Author:      öÄÕ×¾²
Revision:    $v1.0$
Description: µØÖ·ºÏ×÷»ï°é for AddressPartyRel
*/
#ifndef ADDRESS_PARTY_REL_H_

#define ADDRESS_PARTY_REL_H_

#include "shm_Set.h"
#include <vector>

#define MY_W_LOCK_BEGIN 	\
    if(mutex_.WLock())\
    {

#define MY_W_LOCK_END \
    }\
    else\
    {\
        std::cout<<GetName()<<"»¥³âÊ§°Ü£º "<<mutex_.GetErrorMessage()<<endl;\
    }\
    \
    if(!mutex_.WUnLock())\
    {\
        std::cout<<GetName()<<"½âËøÊ§°Ü £º "<<mutex_.GetErrorMessage()<<endl;\
    }\
     
namespace tpss
{
struct AddressPartyRelItem {
    Int64 addr_party_rel_id;
    Int64 address_id;
    int	  party_id;
    int   channel_id;
    Int64 eff_date;
    Int64 exp_date;

    bool operator < ( const AddressPartyRelItem& rhs )const {
        return address_id < rhs.address_id;
    }
    std::string& toString( std::string& str )const {
        return str;
    }
};

class CShmAddressPartyRelMgr
{
public:
    CShmAddressPartyRelMgr();
    ~CShmAddressPartyRelMgr();
    std::string GetName()const {
        return std::string( "AddressPartyRel" );
    }
    bool Init( bool create_shm = false ) ;
    bool LoadDataFromDB();
    bool PutData( const std::vector< AddressPartyRelItem >& vec_address_party_rel );
    bool GetItemByAddressId( Int64 address_id, AddressPartyRelItem& item );
    bool GetItemByChannelId( int channel_id, AddressPartyRelItem& item );
public:
    typedef ns_shm_data::T_SHMSET_NO_MUTEX < AddressPartyRelItem, MAX_PP - 1 >::iterator AddressPartyRelIterator;
    struct IndexChannelId {
        int channel_id;
        AddressPartyRelIterator address_party_rel_iter;
        bool operator < ( const IndexChannelId& rhs )const {
            return channel_id < rhs.channel_id;
        }
        std::string& toString( std::string& str )const {
            return str;
        }
    };
private:
    bool Insert( const AddressPartyRelItem& item );
    ns_shm_data::CManagedMutex mutex_;
    ns_shm_data::T_SHMSET_NO_MUTEX < AddressPartyRelItem, MAX_PP - 1 >  datas_;
    ns_shm_data::T_SHMSET_NO_MUTEX < IndexChannelId, MAX_PP - 2 >  index_;
};
};



#endif
