/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-8 ÉÏÎç11:57:42
Module:      acct_item_type.h
Author:      öÄÕ×¾²
Revision:    $v1.0$
Description: ÕÊÄ¿ÀàÐÍ for AcctItemType
*/

#ifndef ACCT_ITEM_TYPE_H_

#define ACCT_ITEM_TYPE_H_

#include "shm_Set.h"
#include <vector>


#define MY_W_LOCK_BEGIN 	\
    if(mutex_.WLock())\
    {

#define MY_W_LOCK_END \
    }\
    else\
    {\
        std::cout<<GetName()<<"»¥³âÊ§°Ü£º "<<mutex_.GetErrorMessage()<<endl;\
    }\
    \
    if(!mutex_.WUnLock())\
    {\
        std::cout<<GetName()<<"½âËøÊ§°Ü £º "<<mutex_.GetErrorMessage()<<endl;\
    }\
     
namespace tpss
{
struct AcctItemTypeItem {
    bool operator < ( const AcctItemTypeItem& rhs )const {
        return acct_item_type_id < rhs.acct_item_type_id;
    }
    string& toString( string& str )const {
        return str;
    }

    int		acct_item_type_id;
    int		acct_item_class_id;
    char	name[50];
    char	charge_mark[3];
    char	total_mark[3];
    char	acct_item_type_code[15];
    char	acct_item_class_code[15];
};

class CShmAcctItemTypeMgr
{
public:
    CShmAcctItemTypeMgr() : datas_( "AcctItemType", 0 ), mutex_( "AcctItemType" ) {

    }
    ~CShmAcctItemTypeMgr();
    std::string GetName()const {
        return std::string( "AcctItemType" );
    }
    bool Init( bool create_shm = false );
    bool LoadDataFromDB();
    bool PutData( const std::vector< AcctItemTypeItem >& vector_acct_item_type );
    bool GetItem( int acct_item_type_id, AcctItemTypeItem& item );
private:
    bool Insert( const AcctItemTypeItem& acct_item_type_item );
    ns_shm_data::CManagedMutex mutex_;
    ns_shm_data::T_SHMSET_NO_MUTEX < AcctItemTypeItem, MAX_PP - 1 >  datas_;
};
};

#endif
