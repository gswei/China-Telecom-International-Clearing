/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-7 ÉÏÎç10:55:58
Module:      main.cpp
Author:      öÄÕ×¾²
Revision:    $v1.0$
Description: ºÏ×÷»ï°é²âÊÔÄ£¿é for Partner
*/
#include "partner.h"

using namespace tpss;

void LoadRecord( FILE* fp, PartnerItem& item )
{
    fscanf( fp, "%d %s %s %s %s %d %s %s %s %s %s %s %s %hd %lld %lld", &item.party_id, \
            item.pard_code, item.pard_type, item.pard_name, item.pard_desc, \
            &item.lant_id, item.contact_person, item.contact_phone, item.shopcard_num, \
            item.corporation_name, item.corporation_id_no, item.corporation_phone, \
            item.address_info, &item.state, &item.eff_date, &item.exp_date );
}

void LoadTestData( const std::string& filename, std::vector<PartnerItem>& vector_partner_item )
{
    FILE* fp = fopen( filename.c_str(), "rb" );
    PartnerItem tmp;

    do {
        LoadRecord( fp, tmp );
        vector_partner_item.push_back( tmp );
    } while( !feof( fp ) ) ;

    fclose( fp );
}

int main()
{
    if( !InitActiveApp() ) {
        exit( 1 );
    }

    theLog.Open( "shmtest.log" );
    CShmPartnerMgr partner;
    PartnerItem item;
    partner.Init( true );

    /**
    std::vector<PartnerItem> vector_partner_item;
    std::string filename("test.txt");
    LoadTestData(filename, vector_partner_item);
    partner.PutData(vector_partner_item);
    vector_partner_item.clear();
    */
    if( partner.LoadDataFromDB() ) {
        partner.GetItem( 999, item );
        printf( "party_id = %s\n", item.contact_person );
    }

    return 0;
}

