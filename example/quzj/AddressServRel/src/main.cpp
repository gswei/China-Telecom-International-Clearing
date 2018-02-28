/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-8 上午10:11:07
Module:      main.cpp
Author:      瞿兆静
Revision:    $v1.0$
Description: 测试模块用户地址结构 for AddressServRel
*/
#include "address_serv_rel.h"

using namespace tpss;
void LoadRecord( FILE* fp, AddressServRelItem& item )
{
    fscanf( fp, "%lld %lld %lld %d %d %d", &item.serv_id, \
            &item.agreement_id, &item.address_id, &item.bureau_id, \
            &item.exchange_id, &item.stat_region_id );
}

void LoadTestData( const std::string& filename, std::vector<AddressServRelItem>& vector_address_serv_rel_item )
{
    FILE* fp = fopen( filename.c_str(), "rb" );
    AddressServRelItem tmp;

    do {
        LoadRecord( fp, tmp );
        vector_address_serv_rel_item.push_back( tmp );
    } while( !feof( fp ) ) ;

    fclose( fp );
}

int main()
{
    if( !InitActiveApp() ) {
        exit( 1 );
    }

    theLog.Open( "shmtest.log" );
    CShmAddressServRelMgr address_serv_rel;
    AddressServRelItem item;

    address_serv_rel.Init( true );

    /*
    std::vector<AddressServRelItem> vector_address_serv_rel_item;
    std::string filename("test.txt");
    LoadTestData(filename, vector_address_serv_rel_item);
    address_serv_rel.PutData(vector_address_serv_rel_item);
    vector_address_serv_rel_item.clear();
    */
    if( address_serv_rel.LoadDataFromDB() ) {
        if( address_serv_rel.GetItem( 6, item ) ) {
            printf( "stat_region_id = %d\n", item.stat_region_id );
        } else {
            printf( "can't find the data\n" );
        }
    }

    return 0;
}
