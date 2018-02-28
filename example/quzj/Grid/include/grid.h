/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-7 ÏÂÎç12:45:04
Module:      grid.h
Author:      öÄÕ×¾²
Revision:    $v1.0$
Description: Íø¸ñ½á¹¹ for Grid
*/
#ifndef GRID_H_

#define GRID_H_

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
struct GridItem {
    Int64 grid_id;
    char  grid_code[30];
    char  grid_name[250];
    char  grid_type[3];
    char  grid_range[2000];
    char  status_cd[3];
    bool operator < ( const GridItem& rhs )const {
        return grid_id < rhs.grid_id;
    }
    string& toString( string& str )const {
        return str;
    }
};

class CShmGridMgr
{
public:
    CShmGridMgr() : datas_( "Grid", 0 ), mutex_( "Grid" ) {
    }
    ~CShmGridMgr();
    bool Init( bool create_shm = false );
    bool LoadDataFromDB();
    std::string GetName()const {
        return std::string( "Grid" );
    }
    bool PutData( const std::vector< GridItem >& vec_grid_item );
    bool GetItem( int party_id, GridItem& item );
private:
    bool Insert( const GridItem& item );
    ns_shm_data::CManagedMutex mutex_;
    ns_shm_data::T_SHMSET_NO_MUTEX < GridItem, MAX_PP - 1 >  datas_;
};
};

#endif
