/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-7 下午4:09:58
Module:      stamp_info.h
Author:      瞿兆静
Revision:    $v1.0$
Description: 代金券领取信息 for StampInfo
*/
#ifndef STAMP_INFO_H_

#define STAMP_INFO_H_

#include "shm_Set.h"
#include <vector>


#define MY_W_LOCK_BEGIN 	\
    if(mutex_.WLock())\
    {

#define MY_W_LOCK_END \
    }\
    else\
    {\
        std::cout<<GetName()<<"互斥失败： "<<mutex_.GetErrorMessage()<<endl;\
    }\
    \
    if(!mutex_.WUnLock())\
    {\
        std::cout<<GetName()<<"解锁失败 ： "<<mutex_.GetErrorMessage()<<endl;\
    }\
     
struct StampInfoItem {
    bool operator < ( const StampInfoItem& rhs )const {
        return strcmp( stamp_code, rhs.stamp_code ) == -1;
    }
    string& toString( string& str )const {
        return str;
    }
    char  	stamp_code[50];
    int   	obj_type;
    Int64 	obj_id;
    int   	stamp_type;
    Int64  	amount;
    int    	latn_id;
    Int64   	create_date;
    short int 	settle_mode;
};

class CShmStampInfoMgr
{
public:
    CShmStampInfoMgr();
    ~CShmStampInfoMgr();
    std::string GetName()const {
        return std::string( "StampInfo" );
    }
    bool PutData( const std::vector< StampInfoItem >& vector_stamp_info );
    bool GetItem( const char* stamp_code, StampInfoItem& item );
private:
    ns_shm_data::CManagedMutex mutex_;
    ns_shm_data::T_SHMSET_NO_MUTEX < StampInfoItem, MAX_PP - 1 >  datas_;
    int max_item_count_;
};

#endif
