/******************************************************************
	Copyright (c) 2012, GUANGDONG YOSON TECHNOLOGY CO., LTD
	All rights reserved.

	Created:		2012/9/24
	Filename: 		busi_shm_mgr.cpp
	Description:	业务共享内存管理类实现。
	                提供产品化psshm等命令的业务数据接口；
					提供业务共享内存的管理接口（创建、销毁、加载数据、清除数据等）；

	History:
	<table>
		Revision	Author			Date		Description
		--------	------			----		-----------
		v1.0		raozh		 2012/9/24	    开始编码
	</table>
*******************************************************************/

#include "busi_shm_mgr.h"
#include "es/util/Path.h"

using namespace tpss;

//业务共享内存起始编号(私有进程空间内部使用)
const int PI_BUSISHM_FIRST = 1;

bool CBusiShmMgr::bInstanced = false;

CBusiShmMgr::~CBusiShmMgr()
{
    for( T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.begin(); it != map_table_obj_.end(); it++ ) {
        delete it->second;
    }
}


CBusiShmMgr* CBusiShmMgr::Instance()
{
    CBusiShmMgr* pShm = es::Singleton<CBusiShmMgr>::instance();

    if( pShm != NULL && !bInstanced ) {
        if( !InitActiveApp() ) {
            return NULL;
        }

        if( !pShm->Init() ) {
            return NULL;
        }
    }

    return pShm;
}

bool CBusiShmMgr::Init()
{
    //

    ns_shm_data::IShmActiveObject* p_shm_obj( 0 );
    p_shm_obj = new ns_shm_data::T_SHMSET_NO_MUTEX_EXT<AcctItemAggrItem, PI_BUSISHM_FIRST>( "AcctItemAggr", 0 );

    if( p_shm_obj->Attach( false ) ) {
        map_table_obj_.insert( std::make_pair( std::string( "AcctItemAggr" ), p_shm_obj ) );
        return true;
    } else {
        return false;
    }

    //if (!InitSingleShm<AcctItemAggrItem, PI_BUSISHM_FIRST+1>("AcctItemAggr")) return false;
    /****
    //
    if (!InitSingleShm<IndexOfferInstIdOfAcctItemAggr, PI_BUSISHM_FIRST+2>("",2)) return false;
    //
    if (!InitSingleShm<AcctItemOweItem, PI_BUSISHM_FIRST+3>("")) return false;
    //
    if (!InitSingleShm<IndexOfferIdOfAcctItemOwe, PI_BUSISHM_FIRST+4>("",2)) return false;
    //
    if (!InitSingleShm<AcctItemTypeItem, PI_BUSISHM_FIRST+5>("")) return false;
    //
    if (!InitSingleShm<AddressPartyRelItem, PI_BUSISHM_FIRST+6>("")) return false;
    //
    if (!InitSingleShm<IndexChannelIdOfAddressPartyRel, PI_BUSISHM_FIRST+7>("",2)) return false;
    //
    if (!InitSingleShm<ChannelMemberItem, PI_BUSISHM_FIRST+8>("")) return false;
    //
    if (!InitSingleShm<ChannelEquipRelItem, PI_BUSISHM_FIRST+9>("")) return false;
    //
    if (!InitSingleShm<IndexSubLineBoxCodeOfChannelEquipRel, PI_BUSISHM_FIRST+10>("",2)) return false;
    //
    if (!InitSingleShm<IndexJunctionBoxCodeOfChannelEquipRel, PI_BUSISHM_FIRST+11>("",2)) return false;
    //
    if (!InitSingleShm<ChannelItem, PI_BUSISHM_FIRST+12>("")) return false;
    //
    if (!InitSingleShm<IndexPartyIdOfChannel, PI_BUSISHM_FIRST+13>("",2)) return false;
    //
    if (!InitSingleShm<CalcResultDetailItem, PI_BUSISHM_FIRST+14>("")) return false;
    //
    if (!InitSingleShm<GridItem, PI_BUSISHM_FIRST+15>("")) return false;
    //
    if (!InitSingleShm<ProductOfferItem, PI_BUSISHM_FIRST+16>("")) return false;
    //
    if (!InitSingleShm<PaymentItem, PI_BUSISHM_FIRST+17>("")) return false;
    //
    if (!InitSingleShm<PartnerOfferInstanceItem, PI_BUSISHM_FIRST+18>("")) return false;
    //
    if (!InitSingleShm<PartnerItem, PI_BUSISHM_FIRST+19>("")) return false;
    //
    if (!InitSingleShm<RatableResourceAccumulatorItem, PI_BUSISHM_FIRST+20>("")) return false;
    //
    if (!InitSingleShm<ServTerminalItem, PI_BUSISHM_FIRST+21>("")) return false;
    //
    if (!InitSingleShm<SimCardActiveItem, PI_BUSISHM_FIRST+22>("")) return false;
    //
    if (!InitSingleShm<IndexAccNbrOfSimCardActive, PI_BUSISHM_FIRST+23>("",2)) return false;
    //
    if (!InitSingleShm<StampInfoItem, PI_BUSISHM_FIRST+24>("")) return false;
    //
    if (!InitSingleShm<SimCardPartyRelItem, PI_BUSISHM_FIRST+25>("")) return false;
    //
    if (!InitSingleShm<IndexAccNbrOfSimCardPartyRel, PI_BUSISHM_FIRST+26>("",2)) return false;
    //
    if (!InitSingleShm<SettleItemDetailItem, PI_BUSISHM_FIRST+27>("")) return false;
    //
    if (!InitSingleShm<ServLocationItem, PI_BUSISHM_FIRST+28>("")) return false;
    //
    if (!InitSingleShm<ServGridRelItem, PI_BUSISHM_FIRST+29>("")) return false;
    //
    if (!InitSingleShm<IndexServIdOfServGridRel, PI_BUSISHM_FIRST+30>("",2)) return false;
    //
    if (!InitSingleShm<ServEquipItem, PI_BUSISHM_FIRST+31>("")) return false;
    //
    if (!InitSingleShm<ServChannelRelItem, PI_BUSISHM_FIRST+32>("")) return false;
    //
    if (!InitSingleShm<IndexChannelIdOfServChannelRel, PI_BUSISHM_FIRST+33>("",2)) return false;
    //
    if (!InitSingleShm<TerminalRegistItem, PI_BUSISHM_FIRST+34>("")) return false;
    ****/
}

//查询所有业务共享内存统计信息
//对应产品化命令: psshm -stat
bool CBusiShmMgr::QueryStatInfo( std::vector<ShmStatInfo>& vec_stat_info )
{
    //
    bool ret = true;

    //表对象
    for( T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.begin(); it != map_table_obj_.end(); it++ ) {
        ret = ret && it->second->QueryStatInfo( vec_stat_info );
    }

    //索引对象
    for( T_SHM_OBJ_MAP::const_iterator it = map_index_obj_.begin(); it != map_index_obj_.end(); it++ ) {
        ret = ret && it->second->QueryStatInfo( vec_stat_info );
    }

    return ret;
}

//查询所有业务共享内存表信息
//对应产品化命令：规范不要求，可自行扩展
bool CBusiShmMgr::QueryTableInfo( std::vector<ShmTableInfo>& vec_table_info )
{
    //
    bool ret = true;

    for( T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.begin(); it != map_table_obj_.end(); it++ ) {
        ret = ret && it->second->QueryTableInfo( vec_table_info );
    }

    return ret;
}

//查看表记录
//对应产品化命令: psshm -show table_name
//返回值：1 后续有记录 0 结束 -1 出错
int CBusiShmMgr::ShowTable( const std::string& table_name,
                            std::vector<std::vector<std::pair<std::string, std::string> > >& vec_record )
{
    int ret = -1;

    T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.find( table_name );

    if( it != map_table_obj_.end() ) {
        if( table_name != table_name_ || conditon_ != "" ) {
            page_index_ = 1;
            table_name_ = table_name;
            conditon_ = "";
        }

        ret = it->second->ShowData( page_index_, SHOW_PAGE_SIZE, vec_record );
        ++page_index_;
    } else {
        err_code_ = SHMTABLE_NOT_EXISTS;
        err_msg_ = "表名" + table_name + "不存在";
    }

    return ret;
}

//对应产品化命令: psshm -show table_name [indexcolumn=value][,indexcolumn=value]
//返回值：1 后续有记录 0 结束 -1 出错
//indexcolumn要支持所有列，非索引使用遍历实现
int CBusiShmMgr::ShowTable( const std::string& table_name, const std::string& condition,
                            std::vector<std::vector<std::pair<std::string, std::string> > >& vec_record )
{
    int ret = -1;

    T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.find( table_name );

    if( it != map_table_obj_.end() ) {
        if( table_name != table_name_ || condition != conditon_ ) {
            page_index_ = 1;
            table_name_ = table_name;
            conditon_ = condition;
        }

        ret = it->second->ShowData( condition, page_index_, SHOW_PAGE_SIZE, vec_record, err_msg_ );
        ++page_index_;

        if( ret = -1 ) {
            err_code_ = CONDITION_IS_INVALID;
        }
    } else {
        err_code_ = SHMTABLE_NOT_EXISTS;
        err_msg_ = "表名" + table_name + "不存在";
    }

    return ret;
}

//分页复位.复位到第一页
bool CBusiShmMgr::ShowReset()
{
    page_index_ = 1;
}


//统计记录数
//对应产品化命令: psshm -count table_name
//返回值：>=0 记录数  -1 出错
int CBusiShmMgr::QueryRecordCount( const std::string& table_name )
{
    int ret = -1;

    T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.find( table_name );

    if( it != map_table_obj_.end() ) {
        ret = it->second->QueryRecordCount();
    } else {
        err_code_ = SHMTABLE_NOT_EXISTS;
        err_msg_ = "表名" + table_name + "不存在";
    }

    return ret;
}

//对应产品化命令: psshm -count table_name [indexcolumn=value]
//返回值：>=0 记录数  -1 出错
int CBusiShmMgr::QueryRecordCount( const std::string& table_name, const std::string& condition )
{
    int ret = -1;

    T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.find( table_name );

    if( it != map_table_obj_.end() ) {
        ret = it->second->QueryRecordCount( condition, err_msg_ );

        if( ret == -1 ) {
            err_code_ = CONDITION_IS_INVALID;
        }
    } else {
        err_code_ = SHMTABLE_NOT_EXISTS;
        err_msg_ = "表名" + table_name + "不存在";
    }

    return ret;
}

//查询锁状态
//对应产品化命令: psshm -lockstat [table_name]
//返回值：0 正常  1加锁
int CBusiShmMgr::QueryLockStatus( const std::string& table_name )
{
    int ret = -1;

    T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.find( table_name );

    if( it != map_table_obj_.end() ) {
        ret = it->second->QueryLockStatus();
    } else {
        err_code_ = SHMTABLE_NOT_EXISTS;
        err_msg_ = "表名" + table_name + "不存在";
    }

    return ret;
}

//重置锁状态
//对应产品化命令: psshm -lockreset [table_name] [,table_name]
//返回值：0 正常  1加锁
int CBusiShmMgr::LockReset( const std::string& table_name )
{
    StringTokenizer token( table_name, "," );

    //先核查table_name是否都合法
    for( int i = 0; i < token.size(); i++ ) {
        T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.find( token[i] );

        if( it == map_table_obj_.end() ) {
            err_code_ = SHMTABLE_NOT_EXISTS;
            err_msg_ = "表名" + token[i] + "不存在";
            return -1;
        }
    }

    //Reset
    for( int i = 0; i < token.size(); i++ ) {
        T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.find( token[i] );

        if( it != map_table_obj_.end() ) {
            it->second->LockReset();
        }
    }

    return 0;
}

//导入
//对应产品化命令: psimport –m disct –t disct_data
//返回值：>=0 成功导入的记录数 -1 出错
int CBusiShmMgr::ImportFromDB( const std::string& shm_name, const std::string& table_name )
{
    int ret = -1;

    T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.find( shm_name );

    if( it != map_table_obj_.end() ) {
        ret = it->second->ImportFromDB( table_name, err_code_, err_msg_ );
    } else {
        err_code_ = SHMTABLE_NOT_EXISTS;
        err_msg_ = "共享内存" + shm_name + "不存在";
    }

    return ret;
}

//对应产品化命令：psimport –m disct –f file_name
//返回值：>=0 成功导入的记录数 -1 出错
int CBusiShmMgr::ImportFromFile( const std::string& shm_name, const std::string& file_name )
{
    int ret = -1;

    std::string file_full_name = file_name;

    if( !es::Path::isAbsolute( file_name ) ) {
        std::string default_path = getenv( "SETTLEDIR" );
        default_path = default_path + "/tmp/shm_bak";
        file_full_name = default_path + "/" + file_name + ".txt";
    }

    T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.find( shm_name );

    if( it != map_table_obj_.end() ) {
        ret = it->second->ImportFromFile( file_full_name, err_code_, err_msg_ );
    } else {
        err_code_ = SHMTABLE_NOT_EXISTS;
        err_msg_ = "共享内存" + shm_name + "不存在";
    }

    return ret;
}

//导出
//对应产品化命令: psexport –m disct –t disct_data
//返回值：>=0 成功导出的记录数 -1 出错
int CBusiShmMgr::ExportToDB( const std::string& shm_name, const std::string& table_name )
{
    int ret = -1;

    T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.find( shm_name );

    if( it != map_table_obj_.end() ) {
        ret = it->second->ExportToDB( table_name, err_code_, err_msg_ );
    } else {
        err_code_ = SHMTABLE_NOT_EXISTS;
        err_msg_ = "共享内存" + shm_name + "不存在";
    }

    return ret;
}

//对应产品化命令：psexport –m disct –f file_name
//返回值：>=0 成功导出的记录数 -1 出错
int CBusiShmMgr::ExportToFile( const std::string& shm_name, const std::string& file_name )
{
    int ret = -1;

    std::string file_full_name = file_name;

    if( !es::Path::isAbsolute( file_name ) ) {
        std::string default_path = getenv( "SETTLEDIR" );
        default_path = default_path + "/tmp/shm_bak";
        file_full_name = default_path + "/" + file_name + ".txt";
    }

    std::string file_path = es::Path::extractPath( es::Path( file_full_name ) );

    //判断目录是否存在，不存在则创建
    if( access( file_path.c_str(), F_OK ) < 0 ) {
        mkdir( file_path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO );
    }

    T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.find( shm_name );

    if( it != map_table_obj_.end() ) {
        ret = it->second->ExportToFile( file_full_name, err_code_, err_msg_ );
    } else {
        err_code_ = SHMTABLE_NOT_EXISTS;
        err_msg_ = "共享内存" + shm_name + "不存在";
    }

    return ret;
}

