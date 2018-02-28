/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-6 OBNg2:32:11
Module:      main.cpp
Author:      vDUW>2
Revision:    $v1.0$
Description: 2bJT for TerminalRegist
*/
#include "terminal_regist.h"

void LoadRecord( FILE* fp, TerminalRegistItem& item )
{
    fscanf( fp, "%lld %lld %s %s %s %hd %s %lld", &item.regist_id, \
            &item.serv_id, item.acc_nbr, item.terminal_mode, item.brand, \
            &item.intelligence, item.phone_strcode, &item.regist_date );
}
void LoadTestData( const std::string& filename, std::vector<TerminalRegistItem>& vector_terminal_regist_item )
{
    FILE* fp = fopen( filename.c_str(), "rb" );
    TerminalRegistItem tmp;

    do {
        LoadRecord( fp, tmp );
        vector_terminal_regist_item.push_back( tmp );
    } while( !feof( fp ) ) ;

    fclose( fp );
}

int main()
{
    if( !InitActiveApp() ) {
        exit( 1 );
    }

    theLog.Open( "shmtest.log" );
    CShmTerminalRegistMgr terminal_regist;
    TerminalRegistItem item;
    std::vector<TerminalRegistItem> vector_terminal_regist_item;

    std::string filename( "test.txt" );
    LoadTestData( filename, vector_terminal_regist_item );
    terminal_regist.PutData( vector_terminal_regist_item );
    vector_terminal_regist_item.clear();

    terminal_regist.GetItem( 4, item );
    printf( "acc_nbr = %s\n", item.acc_nbr );


    return 0;
}


