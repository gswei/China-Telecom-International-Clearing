/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-7 下午12:45:04
Module:      grid.h
Author:      瞿兆静
Revision:    $v1.0$
Description: 网格结构 for Grid
*/

#include "grid.h"

#include "psutil.h"

using namespace tpss;

CShmGridMgr::~CShmGridMgr()
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Detach() ) {
        printf( "Grid=>Detach failed，%s:%d\n", __FILE__, __LINE__ );
    }

    MY_W_LOCK_END
}

bool CShmGridMgr::Init( bool create_shm )
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

bool CShmGridMgr::LoadDataFromDB()
{
    DBConnection conn;
    bool ret = true;

    if( dbConnect( conn ) ) {
        Statement stmt = conn.createStatement();
        std::string sql = "select GRID_ID, GRID_CODE, GRID_NAME, GRID_TYPE, "
                          "GRID_RANGE, STATUS_CD from GRID";
        stmt.setSQLString( sql );

        if( stmt.execute() ) {
            GridItem tmp;
            mutex_.AttachOrCreateMutexIfNeed();
            MY_W_LOCK_BEGIN

            while( stmt >> tmp.grid_id >> tmp.grid_code
                    >> tmp.grid_name >> tmp.grid_type
                    >> tmp.grid_range >> tmp.status_cd ) {
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
bool CShmGridMgr::PutData( const std::vector< GridItem >& vec_grid_item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    std::vector<GridItem>::const_iterator iter;

    for( iter = vec_grid_item.begin(); iter != vec_grid_item.end(); ++iter ) {
        if( !Insert( *iter ) ) {
            ret = false;
        }
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmGridMgr::GetItem( int grid_id, GridItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < GridItem, MAX_PP - 1 >::const_iterator iter;
    GridItem tmp;
    tmp.grid_id = grid_id;
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

bool CShmGridMgr::Insert( const GridItem& item )
{
    return datas_.insert( item ).first != datas_.end();
}

