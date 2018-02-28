/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:     2012-9-7 ÏÂÎç2:51:58
Module:      serv_terminal.h
Author:      öÄÕ×¾²
Revision:    $v1.0$
Description: ÓÃ»§ÖÕ¶ËÐÅÏ¢ for ServTerminal
*/
#ifndef SERV_TERMINAL_H_

#define SERV_TERMINAL_H_

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
     

struct ServTerminalItem {
    bool operator < ( const ServTerminalItem& rhs )const {
        return serv_id < rhs.serv_id;
    }
    string& toString( string& str )const {
        return str;
    }
    int64 serv_id;
    char  terminal_mode[60];
    char  brand[60];
    short int intelligence;
    char phone_strcode[30];
    int64 create_date;
};

class CShmServTerminalMgr
{
public:
    CShmServTerminalMgr();
    ~CShmServTerminalMgr();

    std::string GetName()const {
        return std::string( "ServTerm" );
    }
    bool PutData( const std::vector< ServTerminalItem >& vector_serv_term );
    bool GetItem( Int64 servId, ServTerminalItem& item );
private:
    ns_shm_data::CManagedMutex mutex_;
    ns_shm_data::T_SHMSET_NO_MUTEX < ServTerminalItem, MAX_PP - 1 >  datas_;
    int max_item_count_;
};

#endif
