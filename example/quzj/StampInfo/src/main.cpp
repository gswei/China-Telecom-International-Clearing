/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-7 OBNg4:09:46
Module:      main.cpp
Author:      vDUW>2
Revision:    $Id: main.cpp 1736 2012-10-26 01:51:20Z zhenghb $
Description: xxxx for StampInfo
*/
#include "stamp_info.h"

using namespace tpss;

void LoadRecord( FILE* fp, StampInfoItem& item )
{
    fscanf( fp, "%s %d %lld %d %lld %d %lld %hd", item.stamp_code, &item.obj_type, &item.obj_id, \
            &item.stamp_type, &item.amount, &item.latn_id, &item.create_date, \
            &item.setller_mode );
}

void LoadTestData( const std::string& filename, std::vector<StampInfoItem>& vector_stamp_info_item )
{
    FILE* fp = fopen( filename.c_str(), "rb" );
    StampInfoItem tmp;

    do {
        LoadRecord( fp, tmp );
        vector_stamp_info_item.push_back( tmp );
    } while( !feof( fp ) ) ;

    fclose( fp );
}

int main()
{
    if( !InitActiveApp() ) {
        exit( 1 );
    }

    theLog.Open( "shmtest.log" );
    CShmStampInfoMgr* stamp_info =  CShmStampInfoMgr::Instance();
    StampInfoItem item;
    /**
    std::vector<StampInfoItem> vector_stamp_info_item;

    std::string filename("test.txt");
    LoadTestData(filename, vector_stamp_info_item);
    stamp_info.PutData(vector_stamp_info_item);
    vector_stamp_info_item.clear();
    */
    stamp_info->LoadDataFromDB();
    stamp_info->GetItem( "ST879784832", item );
    printf( "obj_type = %d\n", item.obj_type );

    return 0;
}
