/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-12 下午4:19:42
Module:      main.cpp
Author:      瞿兆静
Revision:    $v1.0$
Description: 用户设备资源结构 for ServEquip
*/
#include "serv_equip.h"

using namespace tpss;

void LoadRecord( FILE* fp, ServEquipItem& item )
{
    fscanf( fp, "%lld %s %s %s", &item.serv_id, item.trunk_line_code,
            item.sub_line_box_code, item.junction_box_code );
}

void LoadTestData( const std::string& filename, std::vector<ServEquipItem>& vector_serv_equip )
{
    FILE* fp = fopen( filename.c_str(), "rb" );
    ServEquipItem tmp;

    do {
        LoadRecord( fp, tmp );
        vector_serv_equip.push_back( tmp );
    } while( !feof( fp ) ) ;

    fclose( fp );
}

int main()
{
    theLog.Open( "shmtest.log" );

    CShmServEquipMgr shm;

    if( !shm.Init( true ) ) {
        printf( "InitActiveApp失败\n" );
        return false;
    }

    std::cout << "LoadDataFromDB()" << std::endl;
    shm.LoadDataFromDB();

    //1.测试遍历
    std::cout << "QueryAll()" << std::endl;

    if( shm.QueryAll() ) {
        do {
            Int64 serv_id;
            std::string  trunk_line_code;

            shm.GetValue( "serv_id", serv_id );
            shm.GetValue( "trunk_line_code", trunk_line_code );

            std::cout << "serv_id = " << serv_id << "; trunk_line_code=" << trunk_line_code << std::endl;
        } while( shm.Next() );
    }


    //2.测试按索引检索
    std::cout << "QueryAll()" << std::endl;

    if( shm.QueryByIndex( "serv_id", "1004" ) ) {
        do {
            Int64 serv_id;
            std::string  trunk_line_code;

            shm.GetValue( "serv_id", serv_id );
            shm.GetValue( "trunk_line_code", trunk_line_code );

            std::cout << "serv_id = " << serv_id << "; trunk_line_code=" << trunk_line_code << std::endl;
        } while( shm.Next() );
    }
}

