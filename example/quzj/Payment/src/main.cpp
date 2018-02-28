/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-7 ÏÂÎç4:54:17
Module:      main.cpp
Author:      öÄÕ×¾²
Revision:    $v1.0$
Description: ²âÊÔÄ£¿é¸¶¿î¼ÇÂ¼ for Payment
*/
#include "payment.h"

using namespace tpss;

void LoadRecord( FILE* fp, PaymentItem& item )
{
    fscanf( fp, "%lld %lld %d %d %d %s %s %lld %s %lld %s %d", &item.payment_id, \
            &item.acct_id, &item.payment_method, &item.staff_id, &item.payed_method, \
            item.operation_type, item.amount, &item.payment_date, item.state, &item.serv_id, \
            item.acc_nbr, &item.pay_cycle_id );
}

void LoadTestData( const std::string& filename, std::vector<PaymentItem>& vector_payment_item )
{
    FILE* fp = fopen( filename.c_str(), "rb" );
    PaymentItem tmp;

    do {
        LoadRecord( fp, tmp );
        vector_payment_item.push_back( tmp );
    } while( !feof( fp ) ) ;

    fclose( fp );
}

int main()
{
    if( !InitActiveApp() ) {
        exit( 1 );
    }

    theLog.Open( "shmtest.log" );
    CShmPaymentMgr payment;
    PaymentItem item;
    /**
    std::vector<PaymentItem> vector_payment_item;
    std::string filename("test.txt");
    LoadTestData(filename, vector_payment_item);
    payment.PutData(vector_payment_item);
    vector_payment_item.clear();
    */
    payment.Init( true );

    if( payment.LoadDataFromDB() ) {
        payment.GetItem( 2, item );
        printf( "acc_nbr = %s\n", item.acc_nbr );
    }

    return 0;
}
