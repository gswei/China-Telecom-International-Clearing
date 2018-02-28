/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:    2012.9.1 16:41
Module:      calc_result_detail.cpp
Author:      瞿兆静
Revision:    v1.0
Description: 提供从共享内存中读取数据,并且将读取出来的数据进行删除、向共享内存写入数据的接口 为CalResultDetails
*/

#include "calc_result_detail1.h"

using namespace tpss;

bool CShmCalcResultDetailMgr::is_instanced_ = false;

CShmCalcResultDetailMgr::~CShmCalcResultDetailMgr()
{
    if( !datas_.Detach() ) {
        printf( "CShmCalcResultDetailMgr=>Detach failed，%s:%d", __FILE__, __LINE__ );
    }
}

bool CShmCalcResultDetailMgr::Init( bool create_shm )
{
    bool ret = true;

    if( !InitActiveApp() ) {
        return false;
    }

    MY_W_LOCK_BEGIN

    if( create_shm ) {
        if( !datas_.CreateShm() ) {
            ret = false;
        }
    }

    if( !datas_.Attach( false ) ) {
        ret = false;
    }

    MY_W_LOCK_END
    return ret;
}

CShmCalcResultDetailMgr* CShmCalcResultDetailMgr::Instance()
{
    CShmCalcResultDetailMgr* pCalcResultDetail = 0;

    if( !is_instanced_ ) {
        pCalcResultDetail = es::Singleton<CShmCalcResultDetailMgr>::instance();

        if( pCalcResultDetail ) {
            is_instanced_ = true;
        } else {
            printf( "CShmCalcResultDetailMgr初始化失败\n" );
        }
    } else {
        printf( "CShmCalcResultDetailMgr已经初始化过了\n" );
    }

    return pCalcResultDetail;
}

bool CShmCalcResultDetailMgr::GetData( std::vector<CalcResultDetailItem>& vector_detail_item, int default_count )
{
    MY_W_LOCK_BEGIN
    int i = 0;

    while( !datas_.IsEmpty() && i < default_count ) {
        vector_detail_item.push_back( *datas_.Top() );
        datas_.Pop();
        ++ i;
    }

    MY_W_LOCK_END
    return true;
}

bool CShmCalcResultDetailMgr::PutData( const std::vector<CalcResultDetailItem>& vector_detail_item )
{
    bool ret = true;
    MY_W_LOCK_BEGIN
    std::vector<CalcResultDetailItem>::const_iterator iter;
    string str;

    for( iter = vector_detail_item.begin(); iter != vector_detail_item.end(); ++iter ) {
        if( !datas_.Push( *iter ) ) {
            ret = false;
            break;
        }
    }

    MY_W_LOCK_END
    return ret;
}
