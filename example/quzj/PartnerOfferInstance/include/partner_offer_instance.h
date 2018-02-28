/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-8 ����11:25:36
Module:      partner_offer_instance.h
Author:      ���׾�
Revision:    $v1.0$
Description: �������������Ʒ��ϵ�ڴ�ṹ for PartnerOfferInstance
*/
#ifndef PARTNER_OFFER_INSTANCE_H_

#define PARTNER_OFFER_INSTANCE_H_

#include "shm_Set.h"
#include <vector>


#define MY_W_LOCK_BEGIN 	\
    if(mutex_.WLock())\
    {

#define MY_W_LOCK_END \
    }\
    else\
    {\
        std::cout<<GetName()<<"����ʧ�ܣ� "<<mutex_.GetErrorMessage()<<endl;\
    }\
    \
    if(!mutex_.WUnLock())\
    {\
        std::cout<<GetName()<<"����ʧ�� �� "<<mutex_.GetErrorMessage()<<endl;\
    }\
     
namespace tpss
{
struct PartnerOfferInstanceItem {
    Int64 	party_offer_inst_id;
    int   	obj_type;
    int   	obj_id;
    int   	offer_id;
    Int64 	eff_date;
    Int64 	exp_date;

    bool operator < ( const PartnerOfferInstanceItem& rhs )const {
        if( obj_id < rhs.obj_id ) {
            return true;
        }

        if( obj_id > rhs.obj_id ) {
            return false;
        }

        if( obj_type < rhs.obj_type ) {
            return true;
        }

        return false;
    }

    std::string& toString( std::string& str )const {
        return str;
    }
};

class CShmPartnerOfferInstanceMgr
{
public:
    CShmPartnerOfferInstanceMgr::CShmPartnerOfferInstanceMgr() : datas_( "PartnerOfferInstance", 0 ), mutex_( "PartnerOfferInstance" ) {

    }
    ~CShmPartnerOfferInstanceMgr();
    std::string GetName()const {
        return std::string( "PartnerOfferInstance" );
    }
    bool Init( bool create_shm = false );
    bool LoadDataFromDB();
    bool PutData( const std::vector< PartnerOfferInstanceItem >& vector_partner_offer_instance );
    bool GetItem( int obj_id, int obj_type, PartnerOfferInstanceItem& item );
private:
    bool Insert( const PartnerOfferInstanceItem& item );
    ns_shm_data::CManagedMutex mutex_;
    ns_shm_data::T_SHMSET_NO_MUTEX < PartnerOfferInstanceItem, MAX_PP - 1 >  datas_;
};
};

#endif
