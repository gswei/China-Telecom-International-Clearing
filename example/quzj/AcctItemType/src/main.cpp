/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-8 上午11:57:20
Module:      main.cpp
Author:      瞿兆静
Revision:    $v1.0$
Description: 帐目类型 for AcctItemType
*/

#include "acct_item_type.h"

using namespace tpss;

void LoadRecord( FILE* fp, AcctItemTypeItem& item )
{
    fscanf( fp, "%d %d %s %s %s %s", &item.acct_item_type_id, \
            &item.acct_item_class_id, item.name, item.charge_mark, \
            item.total_mark, item.acct_item_type_code );
}

void LoadTestData( const std::string& filename, std::vector<AcctItemTypeItem>& vector_acct_item_type_item )
{
    FILE* fp = fopen( filename.c_str(), "rb" );
    AcctItemTypeItem tmp;

    do {
        LoadRecord( fp, tmp );
        vector_acct_item_type_item.push_back( tmp );
    } while( !feof( fp ) ) ;

    fclose( fp );
}

int main()
{
    theLog.Open( "shmtest.log" );
    CShmAcctItemTypeMgr acct_item_type;
    AcctItemTypeItem item;
    acct_item_type.Init( true );
    /*
    std::vector<AcctItemTypeItem> vector_acct_item_type_item;
    std::string filename("test.txt");
    LoadTestData(filename, vector_acct_item_type_item);
    acct_item_type.PutData(vector_acct_item_type_item);
    vector_acct_item_type_item.clear();
    */
    acct_item_type.LoadDataFromDB();

    if( acct_item_type.GetItem( 2, item ) ) {
        printf( "name = %s\n", item.name );
    } else {
        printf( "can't find the data\n" );
    }

    return 0;
}
