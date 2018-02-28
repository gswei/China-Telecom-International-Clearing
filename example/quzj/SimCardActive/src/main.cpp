/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-12 下午3:24:30
Module:      main.cpp
Author:      瞿兆静
Revision:    $v1.0$
Description: SIM卡激活信息结构 for SimCardActive
*/
#include "sim_card_active.h"

using namespace tpss;

void LoadRecord( FILE* fp, SimCardActiveItem& item )
{
    fscanf( fp, "%lld %lld %s %lld %d %hd %lld", &item.active_id, \
            &item.serv_id, &item.acc_nbr, &item.active_date, \
            &item.latn_id, &item.state, &item.state_date );
}

void LoadTestData( const std::string& filename, std::vector<SimCardActiveItem>& vector_sim_card_active )
{
    FILE* fp = fopen( filename.c_str(), "rb" );
    SimCardActiveItem tmp;

    do {
        LoadRecord( fp, tmp );
        vector_sim_card_active.push_back( tmp );
    } while( !feof( fp ) ) ;

    fclose( fp );
}

int main()
{
    CShmSimCardActiveMgr sim_card_active;
    sim_card_active.Init( true );
    SimCardActiveItem item;
    /*
    std::vector<SimCardActiveItem> vector_sim_card_active;


    std::string filename("test.txt");
    LoadTestData(filename, vector_sim_card_active);
    sim_card_active.PutData(vector_sim_card_active);
    vector_sim_card_active.clear();
    */
    sim_card_active.LoadDataFromDB();
    sim_card_active.GetItemByServId( 4, item );
    printf( "state_date = %lld\n", item.state_date );
    sim_card_active.GetItemByAccNbr( "cat", item );
    printf( "state_date = %lld\n", item.state_date );
    return 0;
}
