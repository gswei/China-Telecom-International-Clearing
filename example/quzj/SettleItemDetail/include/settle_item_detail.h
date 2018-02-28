/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-8 ����2:58:54
Module:      settle_item_detail.h
Author:      ���׾�
Revision:    $v1.0$
Description: �����嵥 for SettleItemDetail
*/
#ifndef SETTLE_ITEM_DETAIL_H_

#define SETTLE_ITEM_DETAIL_H_

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
     
struct SettleItemDetailItem {
    bool operator < ( const SettleItemDetailItem& rhs )const {
        if( settle_obj_id < rhs.settle_obj_id ) {
            return true;
        }

        if( settle_obj_id > rhs.settle_obj_id ) {
            return false;
        }

        if( settle_obj_type < rhs.settle_obj_type ) {
            return true;
        }

        return false;
    }
    string& toString( string& str )const {
        return str;
    }
    short int 	settle_obj_type;
    Int64	settle_obj_id;
    Int64      	charge;
    int	       	billing_cycle_id;
    int        	strategy_id;
    int        	event_type_id;
};

class CShmSettleItemDetailMgr
{
public:
    CShmSettleItemDetailMgr();
    ~CShmSettleItemDetailMgr();
    std::string GetName()const {
        return std::string( "SettleItemDetail" );
    }
    bool PutData( const std::vector< SettleItemDetailItem >& vector_settle_item_detail );
    bool GetItem( Int64 settle_obj_id, short int settle_obj_type, SettleItemDetailItem& item );
private:
    ns_shm_data::CManagedMutex mutex_;
    ns_shm_data::T_SHMSET_NO_MUTEX < SettleItemDetailItem, MAX_PP - 1 >  datas_;
    int max_item_count_;
};

#endif
