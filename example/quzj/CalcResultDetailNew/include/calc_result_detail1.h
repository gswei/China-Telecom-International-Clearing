/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:    2012.9.1 16:41
Module:      calc_result_detail.h
Author:      瞿兆静
Revision:    v1.0
Description: 提供从共享内存中读取数据，并且将读取出来的数据进行删除、向共享内存写入数据的接口 为CalResultDetails
*/


#ifndef CALC_RESULT_DETAIL_H_
#define CALC_RESULT_DETAIL_H_

#include "shm_queue.h"
#include <vector>
#include "intf_shm_resolver.h"
#include "es/util/Singleton.h"

namespace tpss
{
struct CalcResultDetailItem {
    Int64 	service_control_id;
    Int64 	party_offer_inst_id;
    Int64 	sales_id;
    Int64	staff_id;
    Int64 	serv_id;
    char  	acc_nbr[33];
    int   	product_id;
    short int 	cost_flag;
    short int 	adjust_flag;
    short int 	settle_obj_type;
    Int64     	settle_obj_id;
    int       	acct_item_type_id;
    Int64	charge;
    int		channel_id;
    int		party_id;
    int		billing_cycle_id;
    int		offer_id;
    Int64	created_date;
    short int 	state;
    Int64	state_date;
    int		strategy_id;
    int		lant_id;
    char	element_str[2001];
    short int 	is_settle_item; //是否为结算清单1是0否
    std::string& toString( std::string& str ) const {
        char t_dbg[500];
        snprintf( t_dbg, 499, "%lld %lld %lld %lld %lld %s %d %hd %hd %hd %lld %d %lld %d %d %d %d"
                  "%lld %hd %lld %d %d %s %hd", service_control_id, party_offer_inst_id,
                  sales_id, staff_id, serv_id, acc_nbr, product_id, cost_flag, adjust_flag,
                  settle_obj_type, settle_obj_id, acct_item_type_id, charge, channel_id,
                  party_id, billing_cycle_id, offer_id, created_date, state, state_date,
                  strategy_id, lant_id, element_str );
        return str = t_dbg;
    }
    bool operator < ( const CalcResultDetailItem& item ) const {
        return service_control_id < item.service_control_id;
    }

};

class CShmCalcResultDetailMgr
{
public:
    CShmCalcResultDetailMgr(): datas_( "CalcRD", 0 ), mutex_( "CalcRD" ) {

    }
    ~CShmCalcResultDetailMgr();
    static CShmCalcResultDetailMgr* Instance();
    bool Init( bool create_shm = false );
    string GetName() const {
        return string( "CalcRD" );
    }
    /**
     * 从共享内存中获取数据，数据条数为defaul_count条
     * @param vector_detail_item:从共享内存中获取数据，存放到vector_detail_item这个容器中
     * @param default_count：设置从共享内存中获取数据，放到vector_detail_item这个容器中的数据量，默认设置为1000
     * @return：false表示获取失败，true表示获取成功，失败原因看打印的结果
     */
    bool GetData( std::vector<CalcResultDetailItem>& vector_detail_item, int default_count = 10000 );
    /**
     * 向共享内存中放入数据
     * @param vector_detail_item:将vector_detail_item的数据存储到共享内存中
     * @return：false表示获取失败，true表示获取成功,失败的原因看打印结果
     */
    bool PutData( const std::vector<CalcResultDetailItem>& vector_detail_item );
private:
    typedef ShmQueue<CalcResultDetailItem , PI_CALC_RD> TCalcResultDetailQueue;

    static bool is_instanced_;
    //基于一个共享内存数组实现
    TCalcResultDetailQueue datas_;
    //互斥锁
    ns_shm_data::CManagedMutex mutex_;
};
};


#endif /* CALC_RESULT_DETAIL_H_ */
