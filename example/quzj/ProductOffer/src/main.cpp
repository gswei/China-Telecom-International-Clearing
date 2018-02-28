/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-8 上午10:44:45
Module:      main.cpp
Author:      瞿兆静
Revision:    $v1.0$
Description: 销售品内存结构 for ProductOffer
*/
#include "product_offer.h"

using namespace tpss;

void LoadRecord( FILE* fp, ProductOfferItem& item )
{
    fscanf( fp, "%d %d %d %d %s %s %c %s %hd %s %d %lld %lld %c %lld %lld",
            &item.offer_id, &item.band_id, &item.pricing_plan_id, \
            &item.integral_pricing_plan_id, &item.offer_name, &item.offer_comments, \
            &item.can_be_buy_alone, &item.offer_code, &item.priority, &item.state, \
            &item.offer_type, &item.base_fee, &item.floor_fee, &item.merge_flag, \
            &item.eff_date, &item.exp_date );
}

void LoadTestData( const std::string& filename, std::vector<ProductOfferItem>& vector_product_offer_item )
{
    FILE* fp = fopen( filename.c_str(), "rb" );
    ProductOfferItem tmp;

    do {
        LoadRecord( fp, tmp );
        vector_product_offer_item.push_back( tmp );
    } while( !feof( fp ) ) ;

    fclose( fp );
}

int main()
{
    if( !InitActiveApp() ) {
        exit( 1 );
    }

    theLog.Open( "shmtest.log" );
    CShmProductOfferMgr product_offer;
    ProductOfferItem item;

    /**
    std::vector<ProductOfferItem> vector_product_offer_item;
    std::string filename("test.txt");
    LoadTestData(filename, vector_product_offer_item);
    product_offer.PutData(vector_product_offer_item);
    vector_product_offer_item.clear();
    */
    product_offer.Init( false );

    if( product_offer.LoadDataFromDB() ) {
        if( product_offer.GetItem( 2, item ) ) {
            printf( "offer_code = %s\n", item.offer_code );
        } else {
            printf( "can't find the data\n" );
        }
    }

    return 0;
}
