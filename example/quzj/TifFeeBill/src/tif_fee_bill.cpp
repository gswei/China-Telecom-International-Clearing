/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-12 上午10:06:15
Module:      tif_fee_bill.cpp
Author:      瞿兆静
Revision:    $v1.0$
Description: CRM一次费用 for TifFeeBill
*/

#include "tif_fee_bill.h"
#include "psutil.h"

using namespace tpss;

bool CShmTifFeeBillMgr::is_instanced_ = false;

CShmTifFeeBillMgr::~CShmTifFeeBillMgr()
{
    if( is_inited_ ) {
        if( !datas_.Detach() ) {
            printf( "TifFeeBill=>Attach failed，%s:%d\n", __FILE__, __LINE__ );
        }
    }
}

CShmTifFeeBillMgr* CShmTifFeeBillMgr::Instance( bool create_shm )
{
    CShmTifFeeBillMgr* pTifFeeBill = es::Singleton<CShmTifFeeBillMgr>::instance();

    if( !is_instanced_ ) {
        if( pTifFeeBill ) {
            if( !pTifFeeBill->Init( create_shm ) ) {
                pTifFeeBill = 0;
                is_instanced_ = false;
            } else {
                is_instanced_ = true;
            }
        } else {
            printf( "CShmTifFeeBillMgr初始化失败\n" );
        }
    } else {
        printf( "CShmTifFeeBillMgr已经实例化过了\n" );
    }

    return pTifFeeBill;
}

bool CShmTifFeeBillMgr::PutData( const std::vector< TifFeeBillItem >& vec_tif_fee_bill )
{
    bool ret = true;
    MY_W_LOCK_BEGIN
    std::vector<TifFeeBillItem>::const_iterator iter;

    for( iter = vec_tif_fee_bill.begin(); iter != vec_tif_fee_bill.end(); ++iter ) {
        if( false == Insert( *iter ) ) {
            printf( "%s文件%d行: 插入数据失败!\n", __FILE__, __LINE__ );
            ret = false;
            break;
        }
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmTifFeeBillMgr::GetItemByServId( Int64 serv_id, Int64 payment_id, TifFeeBillItem& item )
{
    bool ret = true;
    MY_W_LOCK_BEGIN
    TTifFeeBillSet::const_iterator iter;
    TifFeeBillItem tmp;
    tmp.serv_id = serv_id;
    tmp.payment_id = payment_id;
    iter = datas_.find( tmp );

    if( -1 == iter.handle ) {
        ret = false;
    } else {
        item = *iter;
        ret = true;
    }

    MY_W_LOCK_END
    return ret;
}


bool CShmTifFeeBillMgr::QueryAll()
{
    current_index_name_ = "serv_id";
    first_iter_ = datas_.begin();
    last_iter_ = datas_.end();
    current_iter_ = first_iter_;

    return current_iter_ != datas_.end();
}

//只支持单索引
bool CShmTifFeeBillMgr::QueryByIndex( const std::string& index_name, const std::string& query_value )
{
    current_index_name_ = "";

    if( index_name == "serv_id" ) {
        current_index_name_ = "serv_id";
        TifFeeBillItem item;
        item.serv_id = StringUtil::toInt64( query_value );

        first_iter_ = datas_.lower_bound( item, TifFeeBillItem::comp_serv_id );
        last_iter_ = datas_.upper_bound( item, TifFeeBillItem::comp_serv_id );
        current_iter_ = first_iter_;

        return first_iter_ != last_iter_;

    }

    return false;
}

bool CShmTifFeeBillMgr::Next()
{
    if( "serv_id" == current_index_name_ ) {
        if( current_iter_ != last_iter_ ) {
            ++current_iter_;
        }

        return current_iter_ != last_iter_;
    }

    return false;
}

bool CShmTifFeeBillMgr::GetValue( const std::string& col_name, Int64 & value )
{
    TTifFeeBillSet::const_iterator iter = datas_.end();
    value = 0;

    if( "serv_id" == current_index_name_ && current_iter_ != last_iter_ ) {
        iter = current_iter_;
    } else {
        return false;
    }

    if( iter != datas_.end() ) {
        if( "payment_id" == col_name ) {
            value = ( *iter ).payment_id;
        } else if( "staff_id" == col_name ) {
            value = ( *iter ).staff_id;
        } else if( "acct_id" == col_name ) {
            value = ( *iter ).acct_id;
        } else if( "serv_id" == col_name ) {
            value = ( *iter ).serv_id;
        } else if( "charge_type" == col_name ) {
            value = ( *iter ).charge_type;
        } else if( "acct_item_type_id" == col_name ) {
            value = ( *iter ).acct_item_type_id;
        } else if( "charge" == col_name ) {
            value = ( *iter ).charge;
        } else if( "created_date" == col_name ) {
            value = ( *iter ).created_date;
        } else if( "billing_cycle_id" == col_name ) {
            value = ( *iter ).billing_cycle_id;
        } else if( "lant_id" == col_name ) {
            value = ( *iter ).billing_cycle_id;
        } else {
            return false;
        }
    } else {
        return false;
    }

    return true;
}

bool CShmTifFeeBillMgr::GetValue( const std::string& col_name, std::string & value )
{
    TTifFeeBillSet::const_iterator iter = datas_.end();
    value = "";

    if( "serv_id" == current_index_name_ && current_iter_ != last_iter_ ) {
        iter = current_iter_;
    } else {
        return false;
    }

    if( iter != datas_.end() ) {
        if( "payment_id" == col_name ) {
            value = ( *iter ).payment_id;
        } else if( "staff_id" == col_name ) {
            value = ( *iter ).staff_id;
        } else if( "acct_id" == col_name ) {
            value = ( *iter ).acct_id;
        } else if( "serv_id" == col_name ) {
            value = ( *iter ).serv_id;
        } else if( "acc_nbr" == col_name ) {
            value = ( *iter ).acc_nbr;
        } else if( "charge_type" == col_name ) {
            value = ( *iter ).charge_type;
        } else if( "acct_item_type_id" == col_name ) {
            value = ( *iter ).acct_item_type_id;
        } else if( "charge" == col_name ) {
            value = ( *iter ).charge;
        } else if( "created_date" == col_name ) {
            value = ( *iter ).created_date;
        } else if( "billing_cycle_id" == col_name ) {
            value = ( *iter ).billing_cycle_id;
        } else if( "lant_id" == col_name ) {
            value = ( *iter ).billing_cycle_id;
        } else {
            return false;
        }
    }

    return true;
}

bool CShmTifFeeBillMgr::Reset()
{
    if( "serv_id" == current_index_name_ ) {
        current_iter_ = first_iter_;
    }

    return true;
}

bool CShmTifFeeBillMgr::LoadDataFromDB()
{
    DBConnection conn;
    bool ret = true;

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

    if( dbConnect( conn ) ) {
        Statement stmt = conn.createStatement();
        std::string sql = "select PAYMENT_ID, STAFF_ID, ACCT_ID, SERV_ID, "
                          "ACC_NBR, CHARGE_TYPE, ACCT_ITEM_TYPE_ID, CHARGE, "
                          "CREATED_DATE, BILLING_CYCLE_ID, LANT_ID from TIF_FEE_BILL";
        stmt.setSQLString( sql );

        if( stmt.execute() ) {
            TifFeeBillItem tmp;
            MY_W_LOCK_BEGIN

            while( stmt >> tmp.payment_id >> tmp.staff_id
                    >> tmp.acct_id >> tmp.serv_id >> tmp.acc_nbr >> tmp.charge_type
                    >> tmp.acct_item_type_id >> tmp.charge >> tmp.created_date
                    >> tmp.billing_cycle_id >> tmp.lant_id ) {
                if( false == Insert( tmp ) ) {
                    printf( "%s文件%d行: 插入数据失败!\n", __FILE__, __LINE__ );
                    ret = false;
                    break;
                }
            }

            MY_W_LOCK_END
            conn.close();
        }
    }

    return ret;
}

void CShmTifFeeBillMgr::Report()const
{
    datas_.Report();
}

bool CShmTifFeeBillMgr::Init( bool create_shm )
{
    bool ret = true;

    if( !InitActiveApp() ) {
        return false;
    }

    MY_W_LOCK_BEGIN

    if( create_shm ) {
        if( !datas_.CreateShm() ) {
            printf( "%s文件%d行:datas创建共享内存失败!\n", __FILE__, __LINE__ );
            ret = false;
        }
    }

    if( !datas_.Attach( false ) ) {
        printf( "%s文件%d行:将数据加载到datas失败!\n", __FILE__, __LINE__ );
        ret = false;
    }

    MY_W_LOCK_END
    is_inited_ = ret;
    return ret;
}

bool CShmTifFeeBillMgr::CShmTifFeeBillMgr::Insert( const TifFeeBillItem& item )
{
    //将数据插入到以ServId为索引的树中
    return datas_.insert( item ).first != datas_.end();
    return true;
}
