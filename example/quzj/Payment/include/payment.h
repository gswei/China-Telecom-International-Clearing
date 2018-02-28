/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-7 ÏÂÎç4:53:38
Module:      payment.h
Author:      öÄÕ×¾²
Revision:    $v1.0$
Description: ¸¶¿î¼ÇÂ¼ for Payment
*/
#ifndef PAYMENT_H_

#define PAYMENT_H_

#include "shm_Set.h"
#include <vector>


#define MY_W_LOCK_BEGIN 	\
    if(mutex_.WLock())\
    {

#define MY_W_LOCK_END \
    }\
    else\
    {\
        std::cout<<GetName()<<"»¥³âÊ§°Ü£º "<<mutex_.GetErrorMessage()<<endl;\
    }\
    \
    if(!mutex_.WUnLock())\
    {\
        std::cout<<GetName()<<"½âËøÊ§°Ü £º "<<mutex_.GetErrorMessage()<<endl;\
    }\
     

namespace tpss
{
struct PaymentItem {
    Int64 	payment_id;
    Int64 	acct_id;
    int   	payment_method;
    int   	staff_id;
    int   	payed_method;
    char  	operation_type[3];
    char  	amount[3];
    Int64  	payment_date;
    char  	state[3];
    Int64  	serv_id;
    char  	acc_nbr[32];
    int   	pay_cycle_id;

    bool operator < ( const PaymentItem& rhs )const {
        return serv_id < rhs.serv_id;
    }
    string& toString( string& str )const {
        return str;
    }
};

class CShmPaymentMgr
{
public:
    CShmPaymentMgr() : datas_( "Payment", 0 ), mutex_( "Payment" ) {

    }
    ~CShmPaymentMgr();
    std::string GetName()const {
        return std::string( "Payment" );
    }
    bool Init( bool create_shm );
    bool LoadDataFromDB();
    bool PutData( const std::vector< PaymentItem >& vector_payment );
    bool GetItem( Int64 servId, PaymentItem& item );
private:
    bool Insert( const PaymentItem& item );
    ns_shm_data::CManagedMutex mutex_;
    ns_shm_data::T_SHMSET_NO_MUTEX < PaymentItem, MAX_PP - 1 >  datas_;
};
};

#endif
