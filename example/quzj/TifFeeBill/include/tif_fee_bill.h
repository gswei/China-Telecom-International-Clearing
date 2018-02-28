/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-12 上午10:06:15
Module:      tif_fee_bill.h
Author:      瞿兆静
Revision:    $v1.0$
Description: CRM一次费用 for TifFeeBill
*/

#ifndef TIF_FEE_BILL_H_

#define TIF_FEE_BILL_H_

#include "shm_Set.h"
#include <vector>
#include "intf_shm_resolver.h"
#include "es/util/Singleton.h"

namespace tpss
{
struct TifFeeBillItem {
    Int64 payment_id;			//付款流水号
    int staff_id;				//员工标识
    Int64 acct_id;				//帐户标识
    Int64 serv_id;				//用户标识
    char acc_nbr[32];			//用户号码
    Int64 charge_type;			//缴费类型
    int acct_item_type_id;		//帐目类型
    Int64 charge;				//金额
    Int64 created_date;			//数据创建日期
    int billing_cycle_id;		//帐期标识
    int lant_id;				//本地网标识

    bool operator < ( const TifFeeBillItem& rhs )const {
        return serv_id < rhs.serv_id ||
               ( serv_id == rhs.serv_id && payment_id < rhs.payment_id );
    }
    std::string& toString( std::string& str )const {
        char tmp[256];
        snprintf( tmp, 255, "payment_id=%lld staff_id=%d acct_id=%lld serv_id=%lld "
                  "acc_nbr=%s charge_type=%lld acct_item_type_id=%d charge=%lld "
                  "created_date=%lld billing_cycle_id=%d lant_id=%d", payment_id,
                  staff_id, acct_id, serv_id, acc_nbr, charge_type, acct_item_type_id,
                  charge, created_date, billing_cycle_id, lant_id );
        return str = tmp;
    }
    static bool comp_serv_id( const TifFeeBillItem& a, const TifFeeBillItem& b ) {
        return a.serv_id < b.serv_id;
    }

};


class CShmTifFeeBillMgr : public IShmResolver
{
public:
    CShmTifFeeBillMgr(): datas_( "TifFeeBill", 0 ),
        mutex_( "TifFeeBill" ), is_inited_( false ) {

    }
    ~CShmTifFeeBillMgr();
    std::string GetName()const {
        return std::string( "TifFeeBill" );
    }
    static CShmTifFeeBillMgr* Instance( bool create_shm = false );
    bool PutData( const std::vector< TifFeeBillItem >& vec_tif_fee_bill );
    bool GetItemByServId( Int64 serv_id, Int64 payment_id, TifFeeBillItem& item );

    //2.2实现IShmResolver接口
    bool QueryAll();
    //只支持单索引
    bool QueryByIndex( const std::string& index_name, const std::string& query_value );
    bool Next();
    bool GetValue( const std::string& col_name, Int64 & value );
    bool GetValue( const std::string& col_name, std::string & value );
    bool Reset();
    bool LoadDataFromDB();
    void Report()const;
public:
    typedef ns_shm_data::T_SHMSET_NO_MUTEX<TifFeeBillItem, PI_TIF_FEE_BILL>::iterator TifFeeBillIterator;
private:
    bool Init( bool create_shm );
    bool Insert( const TifFeeBillItem& item );
    typedef ns_shm_data::T_SHMSET_NO_MUTEX<TifFeeBillItem, PI_TIF_FEE_BILL> TTifFeeBillSet;

    static bool is_instanced_;
    bool is_inited_;

    ns_shm_data::CManagedMutex mutex_;
    TTifFeeBillSet datas_;

    TTifFeeBillSet::const_iterator current_iter_;
    TTifFeeBillSet::const_iterator first_iter_;
    TTifFeeBillSet::const_iterator last_iter_;

    std::string current_index_name_;
};


};
#endif