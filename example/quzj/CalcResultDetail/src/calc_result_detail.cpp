/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:    2012.9.1 16:41
Module:      calc_result_detail.cpp
Author:      瞿兆静
Revision:    v1.0
Description: 提供从共享内存中读取数据,并且将读取出来的数据进行删除、向共享内存写入数据的接口 为CalResultDetails
*/

#include "calc_result_detail.h"

using namespace tpss;

CShmCalcResultDetailMgr::~CShmCalcResultDetailMgr()
{
    MY_W_LOCK_BEGIN

    if( !datas_.Detach() ) {
        printf( "CShmCalcResultDetailMgr=>Detach failed，%s:%d", __FILE__, __LINE__ );
    }

    MY_W_LOCK_END
}

bool CShmCalcResultDetailMgr::Init( bool create_shm )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
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

bool CShmCalcResultDetailMgr::GetData( std::vector<CalcResultDetailItem>& vector_detail_item, int default_count )
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < CalcResultDetailItem, MAX_PP - 1 >::const_iterator iter;
    ns_shm_data::T_SHMSET_NO_MUTEX < CalcResultDetailItem, MAX_PP - 1 >::const_iterator tmp;

    for( iter = datas_.begin(); iter != datas_.end(); ) {
        vector_detail_item.push_back( *iter );
        tmp = iter;
        ++iter;
        datas_.erase( tmp );
    }

    MY_W_LOCK_END
    return true;
}

bool CShmCalcResultDetailMgr::PutData( const std::vector<CalcResultDetailItem>& vector_detail_item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    std::vector<CalcResultDetailItem>::const_iterator iter;
    string str;

    for( iter = vector_detail_item.begin(); iter != vector_detail_item.end(); ++iter ) {
        if( !Insert( *iter ) ) {
            ret = false;
            break;
        }
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmCalcResultDetailMgr::Insert( const CalcResultDetailItem& item )
{
    return datas_.insert( item ).first != datas_.end();
}
