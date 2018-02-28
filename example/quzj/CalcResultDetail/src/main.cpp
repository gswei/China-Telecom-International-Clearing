/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-4 14:10:52
Module:      main.cpp
Author:      瞿兆静
Revision:    $v1.0$
Description: 测试模块 for CalcResultDetail
*/

#include "calc_result_detail.h"

using namespace tpss;

void LoadRecord( FILE* fp, CalcResultDetailItem& item )
{
    fscanf( fp, "%lld %lld %lld %lld %lld %s %d %hd %hd %hd %lld %d %lld"
            "%d %d %d %d %lld %d %lld %d %d %s %hd", &item.service_control_id, \
            &item.party_offer_inst_id, &item.sales_id, &item.staff_id, &item.serv_id, \
            item.acc_nbr, &item.product_id, &item.cost_flag, &item.adjust_flag,  \
            &item.settle_obj_type, &item.settle_obj_id, &item.acct_item_type_id, \
            &item.charge, &item.channel_id, &item.party_id, &item.billing_cycle_id, \
            &item.offer_id, &item.created_date, &item.state, &item.state_date,    \
            &item.strategy_id, &item.lant_id, item.element_str, &item.is_settle_item );
}
void LoadTestData( const std::string& filename, std::vector<CalcResultDetailItem>& vector_result_item )
{
    FILE* fp = fopen( filename.c_str(), "rb" );
    CalcResultDetailItem tmp;

    do {
        LoadRecord( fp, tmp );
        vector_result_item.push_back( tmp );
    } while( !feof( fp ) ) ;

    fclose( fp );
}

int main()
{
    CShmCalcResultDetailMgr* pCal_result_detail;
    std::vector<CalcResultDetailItem> vector_detail_item;

    //在这里只需要Instance就行了，返回一个共享内存管理指针
    if( pCal_result_detail = CShmCalcResultDetailMgr::Instance() ) {
        std::string filename( "test.txt" );
        LoadTestData( filename, vector_detail_item );
        printf( "存入数据" );
        pCal_result_detail->PutData( vector_detail_item );
        vector_detail_item.clear();
        printf( "获取数据\n" );
        pCal_result_detail->GetData( vector_detail_item, 20 );
        printf( "显示数据的serv_id字段\n" );

        for( int i = 0; i < vector_detail_item.size(); ++i ) {
            printf( "service_control_id = %d\n", vector_detail_item[i].service_control_id );
        }
    } else {
        printf( "实例化pCal_result_detail = CShmCalcResultDetailMgr::Instance()" );
    }

    if( pCal_result_detail = CShmCalcResultDetailMgr::Instance() ) {

    } else {
        printf( "实例化CShmCalcResultDetailMgr失败\n" );
    }

    return 0;
}
/*

#include "shm_queue.h"

using namespace tpss;

int main()
{
	if(!InitActiveApp())exit(1);
	ShmQueue<int, MAX_PP - 1> que("ShmQueue", 0);
	if (!que.CreateShm()) {
		printf ("创建共享内存失败\n");
		return 0;
	}
	if (!que.Attach(false)) {
		printf ("加载共享内存失败\n");
		return 0;
	}
	que.Push(1);
	que.Push(2);
	que.Push(3);

	que.Pop();
	que.Pop();
	que.Pop();

	que.Detach();
	return 0;
};
*/