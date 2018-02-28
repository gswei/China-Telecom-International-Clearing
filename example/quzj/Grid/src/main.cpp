/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-7 下午12:44:56
Module:      main.cpp
Author:      瞿兆静
Revision:    $v1.0$
Description: 网格结构测试模块 for Grid
*/
#include "grid.h"

using namespace tpss;

void LoadRecord( FILE* fp, GridItem& item )
{
    fscanf( fp, "%lld %s %s %s %s %s", &item.grid_id, \
            item.grid_code, item.grid_name, item.grid_type, item.grid_range, item.status_cd );
}

void LoadTestData( const std::string& filename, std::vector<GridItem>& vector_grid_item )
{
    FILE* fp = fopen( filename.c_str(), "rb" );
    GridItem tmp;

    do {
        LoadRecord( fp, tmp );
        vector_grid_item.push_back( tmp );
    } while( !feof( fp ) ) ;

    fclose( fp );
}

int main()
{
    if( !InitActiveApp() ) {
        exit( 1 );
    }

    theLog.Open( "shmtest.log" );
    CShmGridMgr grid;
    GridItem item;

    /**
    std::vector<GridItem> vector_grid_item;
    std::string filename("test.txt");
    LoadTestData(filename, vector_grid_item);
    grid.PutData(vector_grid_item);
    vector_grid_item.clear();
    */
    grid.Init( true );

    if( grid.LoadDataFromDB() ) {
        grid.GetItem( 3, item );
        printf( "grid_name = %s\n", item.grid_name );
    }

    return 0;
}

