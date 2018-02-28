/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-8 上午11:57:32
Module:      acct_item_type.cpp
Author:      瞿兆静
Revision:    $v1.0$
Description: 帐目类型 for AcctItemType
*/

#include "acct_item_type.h"
#include "psutil.h"

using namespace tpss;

bool CShmAcctItemTypeMgr::Init( bool create_shm )
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

bool CShmAcctItemTypeMgr::LoadDataFromDB()
{
    DBConnection conn;
    bool ret = true;

    if( dbConnect( conn ) ) {
        Statement stmt = conn.createStatement();
        std::string sql = "select ACCT_ITEM_TYPE_ID, ACCT_ITEM_CLASS_ID, NAME, "
                          "CHARGE_MARK, TOTAL_MARK, ACCT_ITEM_TYPE_CODE, ACCT_ITEM_CLASS_CODE "
                          "from ACCT_ITEM_TYPE";
        stmt.setSQLString( sql );

        if( stmt.execute() ) {
            AcctItemTypeItem tmp;
            mutex_.AttachOrCreateMutexIfNeed();
            MY_W_LOCK_BEGIN

            while( stmt >> tmp.acct_item_type_id >> tmp.acct_item_class_id
                    >> tmp.name >> tmp.charge_mark >> tmp.total_mark
                    >> tmp.acct_item_type_code >> tmp.acct_item_class_code ) {
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

CShmAcctItemTypeMgr::~CShmAcctItemTypeMgr()
{
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN

    if( !datas_.Detach() ) {
        printf( "AcctItemType=>Detach failed，%s:%d\n", __FILE__, __LINE__ );
    }

    MY_W_LOCK_END
}

bool CShmAcctItemTypeMgr::PutData( const std::vector< AcctItemTypeItem >& vec_serv_equip )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    std::vector<AcctItemTypeItem>::const_iterator iter;
    string str;

    for( iter = vec_serv_equip.begin(); iter != vec_serv_equip.end(); ++iter ) {
        if( !Insert( *iter ) ) {
            ret = false;
        }
    }

    MY_W_LOCK_END
    return ret;
}

bool CShmAcctItemTypeMgr::GetItem( int acct_item_type_id, AcctItemTypeItem& item )
{
    bool ret = true;
    mutex_.AttachOrCreateMutexIfNeed();
    MY_W_LOCK_BEGIN
    ns_shm_data::T_SHMSET_NO_MUTEX < AcctItemTypeItem, MAX_PP - 1 >::const_iterator iter;
    AcctItemTypeItem tmp;
    tmp.acct_item_type_id = acct_item_type_id;
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

bool CShmAcctItemTypeMgr::Insert( const AcctItemTypeItem& acct_item_type_item )
{
    return datas_.insert( acct_item_type_item ).first != datas_.end();
}
