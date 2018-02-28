/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-6 下午2:33:26
Module:      terminal_regist.h
Author:      瞿兆静
Revision:    $v1.0$
Description: 用户注册终端 for TerminalRegist
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
        std::cout<<GetName()<<"互斥失败： "<<mutex_.GetErrorMessage()<<endl;\
    }\
    \
    if(!mutex_.WUnLock())\
    {\
        std::cout<<GetName()<<"解锁失败 ： "<<mutex_.GetErrorMessage()<<endl;\
    }\
     
struct TerminalRegistItem {
public:
    bool operator < ( const TerminalRegistItem& rhs )const {
        return serv_id < rhs.serv_id;
    }
    string& toString( string& str )const {
        return str;
    }
    Int64        regist_id;             //注册标识
    Int64        serv_id;               //用户标识
    char         acc_nbr[32];           //用户号码
    char         terminal_mode[60];     //终端型号
    char         brand[60];             //终端品牌
    short int    intelligence;          //智能机标志
    char         phone_strcode[30];     //手机串号
    Int64        regist_date;           //注册时间
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
