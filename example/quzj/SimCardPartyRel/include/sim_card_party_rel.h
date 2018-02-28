/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-11 ÏÂÎç7:37:02
Module:      sim_card_party_rel.h
Author:      öÄÕ×¾²
Revision:    $v1.0$
Description: SIM¿¨ºÏ×÷»ï°é¹ØÏµ½á¹¹ for SimCardPartyRel
*/
#ifndef SIM_CARD_PARTY_REL_H_

#define SIM_CARD_PARTY_REL_H_

#include "shm_Set.h"
#include <vector>
#include <cstring>

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
     
struct SimCardPartyRelItem {
    bool operator < ( const SimCardPartyRelItem& rhs )const {
        return strcmp( card_seq, rhs.card_seq ) == -1;
    }
    std::string& toString( std::string& str )const {
        return str;
    }
    char 	card_seq[30];
    char 	acc_nbr[30];
    int  	card_type;
    char 	card_code[20];
    int  	card_value;
    int  	owner_acc_nbr;
    Int64 	add_date;
    int  	sales_acc_nbr;
    Int64 	sales_time;
    int		latn_id;
    int		party_id;
    Int64	send_date;
};

class CShmSimCardPartyRelMgr
{
public:
    CShmSimCardPartyRelMgr();
    ~CShmSimCardPartyRelMgr();
    std::string GetName()const {
        return std::string( "SimCardPartyRel" );
    }
    bool PutData( const std::vector< SimCardPartyRelItem >& vec_sim_card_party_rel );
    bool GetItemByCardSeq( const char* card_seq, SimCardPartyRelItem& item );
    bool GetItemByAccNbr( const char* acc_nbr, SimCardPartyRelItem& item );
public:
    typedef ns_shm_data::T_SHMSET_NO_MUTEX < SimCardPartyRelItem, MAX_PP - 1 >::iterator SimCardPartyRelIterator;
    struct IndexAccNbr {
        char acc_nbr[30];
        SimCardPartyRelIterator sim_card_party_rel_iter;
        bool operator < ( const IndexAccNbr& rhs )const {
            return strcmp( acc_nbr, rhs.acc_nbr ) == 0;
        }
        std::string& toString( std::string& str )const {
            return str;
        }
    };
private:
    ns_shm_data::CManagedMutex mutex_;
    ns_shm_data::T_SHMSET_NO_MUTEX < SimCardPartyRelItem, MAX_PP - 1 >  datas_;
    ns_shm_data::T_SHMSET_NO_MUTEX < IndexAccNbr, MAX_PP - 2 >  index_;
    int max_item_count_;
};

#endif