/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-7 ÉÏÎç10:54:56
Module:      partner.h
Author:      öÄÕ×¾²
Revision:    $v1.0$
Description: ºÏ×÷»ï°é for Partner
*/
#ifndef PARTNER_H_

#define PARTNER_H_

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
struct PartnerItem {
    int party_id;
    char pard_code[15];
    char pard_type[10];
    char pard_name[50];
    char pard_desc[250];
    int  lant_id;
    char contact_person[20];
    char contact_phone[40];
    char shopcard_num[30];
    char corporation_name[32];
    char corporation_id_no[30];
    char corporation_phone[30];
    char address_info[250];
    short int state;
    Int64 eff_date;
    Int64 exp_date;

    bool operator < ( const PartnerItem& rhs )const {
        return party_id < rhs.party_id;
    }
    string& toString( string& str )const {
        return str;
    }

};

class CShmPartnerMgr
{
public:
    CShmPartnerMgr() : datas_( "Part", 0 ), mutex_( "Part" ) {
    }
    ~CShmPartnerMgr();
    bool Init( bool create_shm = false );
    bool LoadDataFromDB();
    std::string GetName()const {
        return std::string( "Part" );
    }
    bool PutData( const std::vector< PartnerItem >& vec_partner_item );
    bool GetItem( int party_id, PartnerItem& item );
private:
    bool Insert( const PartnerItem& item );
    ns_shm_data::CManagedMutex mutex_;
    ns_shm_data::T_SHMSET_NO_MUTEX < PartnerItem, MAX_PP - 1 >  datas_;
};
};



#endif
