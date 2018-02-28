/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-12 ÏÂÎç3:24:15
Module:      sim_card_active.h
Author:      öÄÕ×¾²
Revision:    $v1.0$
Description: SIM¿¨¼¤»îÐÅÏ¢½á¹¹ for SimCardActive
*/
#ifndef SIM_CARD_ACTIVE_H_

#define SIM_CARD_ACTIVE_H_

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
     
class SimCardActiveItem
{
public:
    bool operator < ( const SimCardActiveItem& rhs )const {
        return serv_id < rhs.serv_id;
    }
    std::string& toString( std::string& str )const {
        return str;
    }
    Int64 	active_id;
    Int64 	serv_id;
    char   	acc_nbr[32];
    Int64 	active_date;
    int		latn_id;
    short int state;
    Int64	state_date;
};

class CShmSimCardActiveMgr
{
public:
    CShmSimCardActiveMgr();
    ~CShmSimCardActiveMgr();
    std::string GetName()const {
        return std::string( "SimCardActive" );
    }
    bool PutData( const std::vector< SimCardActiveItem >& vec_sim_card_active );
    bool GetItemByServId( Int64 serv_id, SimCardActiveItem& item );
    bool GetItemByAccNbr( const char* acc_nbr, SimCardActiveItem& item );
public:
    typedef ns_shm_data::T_SHMSET_NO_MUTEX < SimCardActiveItem, MAX_PP - 1 >::iterator SimCardActiveIterator;
    struct IndexAccNbr {
        char acc_nbr[32];
        SimCardActiveIterator sim_card_active_iter;
        bool operator < ( const IndexAccNbr& rhs )const {
            return strcmp( acc_nbr, rhs.acc_nbr ) == -1;
        }
        std::string& toString( std::string& str )const {
            return str;
        }
    };
private:
    ns_shm_data::CManagedMutex mutex_;
    ns_shm_data::T_SHMSET_NO_MUTEX < SimCardActiveItem, MAX_PP - 1 >  datas_;
    ns_shm_data::T_SHMSET_NO_MUTEX < IndexAccNbr, MAX_PP - 2 >  index_;
    int max_item_count_;
};

#endif
