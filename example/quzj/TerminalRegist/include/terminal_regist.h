/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-6 ����2:33:26
Module:      terminal_regist.h
Author:      ���׾�
Revision:    $v1.0$
Description: �û�ע���ն� for TerminalRegist
*/

#ifndef TERMINAL_REGIST_H_
#define TERMINAL_REGIST_H_

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
     
struct TerminalRegistItem {
public:
    bool operator < ( const TerminalRegistItem& rhs )const {
        return serv_id < rhs.serv_id;
    }
    string& toString( string& str )const {
        return str;
    }
    Int64        regist_id;             //ע���ʶ
    Int64        serv_id;               //�û���ʶ
    char         acc_nbr[32];           //�û�����
    char         terminal_mode[60];     //�ն��ͺ�
    char         brand[60];             //�ն�Ʒ��
    short int    intelligence;          //���ܻ���־
    char         phone_strcode[30];     //�ֻ�����
    Int64        regist_date;           //ע��ʱ��
};

class CShmTerminalRegistMgr
{
public:
    CShmTerminalRegistMgr();
    ~CShmTerminalRegistMgr();
    std::string GetName()const {
        return std::string( "TermReg" );
    }
    bool PutData( const std::vector< TerminalRegistItem >& vecTermRegItem );
    bool GetItem( Int64 servId, TerminalRegistItem& item );
private:
    ns_shm_data::CManagedMutex mutex_;
    ns_shm_data::T_SHMSET_NO_MUTEX < TerminalRegistItem, MAX_PP - 1 >  datas_;
    int max_item_count_;
};

#endif /* TERMINAL_REGIST_H */
