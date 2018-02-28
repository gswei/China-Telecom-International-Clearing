/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-8 上午10:10:43
Module:      address_serv_rel.h
Author:      瞿兆静
Revision:    $v1.0$
Description: 用户地址结构 for AddressServRel
*/
#ifndef ADDRESS_SERV_REL_H_

#define ADDRESS_SERV_REL_H_

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
struct AddressServRelItem {
    Int64 serv_id;
    Int64 agreement_id;
    Int64 address_id;
    int   bureau_id;
    int   exchange_id;
    int   stat_region_id;

    bool operator < ( const AddressServRelItem& rhs )const {
        return serv_id < rhs.serv_id;
    }
    std::string& toString( string& str )const {
        return str;
    }
};

class CShmAddressServRelMgr
{
public:
    CShmAddressServRelMgr() : datas_( "AddressServRel", 0 ), mutex_( "AddressServRel" ) {

    }
    ~CShmAddressServRelMgr();
    bool Init( bool create_shm );
    bool LoadDataFromDB();
    std::string GetName()const {
        return std::string( "AddressServRel" );
    }

    bool PutData( const std::vector< AddressServRelItem >& vector_address_serv_rel );
    bool GetItem( Int64 servId, AddressServRelItem& item );
private:
    bool Insert( const AddressServRelItem& item );
    ns_shm_data::CManagedMutex mutex_;
    ns_shm_data::T_SHMSET_NO_MUTEX < AddressServRelItem, MAX_PP - 1 >  datas_;
};
};

#endif
