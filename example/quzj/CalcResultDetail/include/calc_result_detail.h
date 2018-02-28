/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:    2012.9.1 16:41
Module:      calc_result_detail.h
Author:      ���׾�
Revision:    v1.0
Description: �ṩ�ӹ����ڴ��ж�ȡ���ݣ����ҽ���ȡ���������ݽ���ɾ���������ڴ�д�����ݵĽӿ� ΪCalResultDetails
*/


#ifndef CALC_RESULT_DETAIL_H_
#define CALC_RESULT_DETAIL_H_

#include "shm_Set.h"
#include <vector>


#define MY_W_LOCK_BEGIN 	\
    if(mutex_.WLock())\
    {

#define MY_W_LOCK_END \
    }\
    else\
    {\
        std::cout<<GetName()<<"����ʧ�ܣ� "<<mutex_.GetErrorMessage()<<endl;\
    }\
    \
    if(!mutex_.WUnLock())\
    {\
        std::cout<<GetName()<<"����ʧ�� �� "<<mutex_.GetErrorMessage()<<endl;\
    }\
     
namespace tpss
{
struct CalcResultDetailItem {
    Int64 	service_control_id;
    Int64 	party_offer_inst_id;
    Int64 	sales_id;
    Int64	staff_id;
    Int64 	serv_id;
    char  	acc_nbr[32];
    int   	product_id;
    short int 	cost_flag;
    short int 	adjust_flag;
    short int 	settle_obj_type;
    Int64     	settle_obj_id;
    int       	acct_item_type_id;
    Int64	charge;
    int		channel_id;
    int		party_id;
    int		billing_cycle_id;
    int		offer_id;
    Int64	created_date;
    short int 	state;
    Int64	state_date;
    int		strategy_id;
    int		lant_id;
    char	element_str[2000];
    short int 	is_settle_item; //�Ƿ�Ϊ�����嵥1��0��
    std::string& toString( std::string& str ) const {
        return str;
    }
    bool operator < ( const CalcResultDetailItem& item ) const {
        return service_control_id < item.service_control_id;
    }

};

class CShmCalcResultDetailMgr
{
public:
    CShmCalcResultDetailMgr(): datas_( "CalcRD", 0 ), mutex_( "CalcRD" ) {

    }
    ~CShmCalcResultDetailMgr();
    bool Init( bool create_shm = false );
    string GetName() const {
        return string( "CalcRD" );
    }
    /**
     * �ӹ����ڴ��л�ȡ���ݣ���������Ϊdefaul_count��
     * @param vector_detail_item:�ӹ����ڴ��л�ȡ���ݣ���ŵ�vector_detail_item���������
     * @param default_count�����ôӹ����ڴ��л�ȡ���ݣ��ŵ�vector_detail_item��������е���������Ĭ������Ϊ1000
     * @return��false��ʾ��ȡʧ�ܣ�true��ʾ��ȡ�ɹ���ʧ��ԭ�򿴴�ӡ�Ľ��
     */
    bool GetData( std::vector<CalcResultDetailItem>& vector_detail_item, int default_count = 10000 );
    /**
     * �����ڴ��з�������
     * @param vector_detail_item:��vector_detail_item�����ݴ洢�������ڴ���
     * @return��false��ʾ��ȡʧ�ܣ�true��ʾ��ȡ�ɹ�,ʧ�ܵ�ԭ�򿴴�ӡ���
     */
    bool PutData( const std::vector<CalcResultDetailItem>& vector_detail_item );
private:
    bool Insert( const CalcResultDetailItem& item );
    //����һ�������ڴ�����ʵ��
    ns_shm_data::T_SHMSET_NO_MUTEX < CalcResultDetailItem , MAX_PP - 1 > datas_;
    //������
    ns_shm_data::CManagedMutex mutex_;
};
};


#endif /* CALC_RESULT_DETAIL_H_ */
