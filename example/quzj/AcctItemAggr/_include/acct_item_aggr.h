/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-12 上午10:06:04
Module:      acct_item_aggr.h
Author:      瞿兆静
Revision:    $v1.0$
Description: 用户出账信息结构for AcctItemAggr
*/
#ifndef ACCT_ITEM_AGGR_H_

#define ACCT_ITEM_AGGR_H_

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
struct AcctItemAggrItem {
    Int64 	acct_item_id;
    Int64 	serv_id;
    int	  	billing_cycle_id;
    int   	acct_item_type_id;
    Int64 	charge;
    Int64 	acct_id;
    Int64	offer_inst_id;
    int		item_source_id;
    int		offer_id;
    Int64	create_date;

    bool operator < ( const AcctItemAggrItem& rhs )const {
        return serv_id < rhs.serv_id;
    }
    std::string& toString( std::string& str )const {
        return str;
    }
};

class CShmAcctItemAggrMgr
{
public:
    CShmAcctItemAggrMgr(): datas_( "AcctItemAggr", 0 ),
        index_( "OfferInstId_AcctItemAggr", 0 ),
        mutex_( "AcctItemAggr" ) {

    }
    ~CShmAcctItemAggrMgr();
    std::string GetName()const {
        return std::string( "AcctItemAggr" );
    }
    bool Init( bool create_shm = false );
    bool LoadDataFromDB();
    bool PutData( const std::vector< AcctItemAggrItem >& vec_acct_item_aggr );
    bool GetItemByServId( Int64 serv_id, AcctItemAggrItem& item );
    bool GetItemByOfferInstId( Int64 offer_inst_id, AcctItemAggrItem& item );
public:
    typedef ns_shm_data::T_SHMSET_NO_MUTEX < AcctItemAggrItem, MAX_PP - 1 >::iterator AcctItemAggrIterator;
    struct IndexOfferInstId {
        int offer_inst_id;
        AcctItemAggrIterator acct_item_aggr_iter;
        bool operator < ( const IndexOfferInstId& rhs )const {
            return offer_inst_id < rhs.offer_inst_id;
        }
        std::string& toString( std::string& str )const {
            return str;
        }
    };
private:
    bool Insert( const AcctItemAggrItem& item );
    ns_shm_data::CManagedMutex mutex_;
    ns_shm_data::T_SHMSET_NO_MUTEX < AcctItemAggrItem, MAX_PP - 1 >  datas_;
    ns_shm_data::T_SHMSET_NO_MUTEX < IndexOfferInstId, MAX_PP - 2 >  index_;
};
};

#endif
