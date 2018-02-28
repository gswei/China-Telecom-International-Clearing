/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-7 OBNg2:51:31
Module:      main.cpp
Author:      vDUW>2
Revision:    $v1.0$
Description: SC;'VU6KPEO" for ServTerminal
*/
#include "serv_terminal.h"

using namespace tpss;

void LoadRecord( FILE* fp, ServTerminalItem& item )
{
    fscanf( fp, "%lld %s %s %hd %s %lld", &item.serv_id, \
            item.terminal_mode, item.brand, &item.intelligence, \
            item.phone_strcode, &item.create_date );
}

void LoadTestData( const std::string& filename, std::vector<ServTerminalItem>& vector_serv_terminal_item )
{
    FILE* fp = fopen( filename.c_str(), "rb" );
    ServTerminalItem tmp;

    do {
        LoadRecord( fp, tmp );
        vector_serv_terminal_item.push_back( tmp );
    } while( !feof( fp ) ) ;

    fclose( fp );
}

int main()
{
    theLog.Open( "shmtest.log" );
    CShmServTerminalMgr serv_terminal;
    ServTerminalItem item;
    /**
    std::vector<ServTerminalItem> vector_serv_terminal_item;
    std::string filename("test.txt");
    LoadTestData(filename, vector_serv_terminal_item);
    serv_terminal.PutData(vector_serv_terminal_item);
    vector_serv_terminal_item.clear();
    */
    serv_terminal.Init( true );

    if( serv_terminal.LoadDataFromDB() ) {
        serv_terminal.GetItem( 2, item );
        printf( "terminal_mode = %s\n", item.terminal_mode );
    }

    return 0;
}

