/******************************************************************
	Copyright (c) 2012, GUANGDONG YOSON TECHNOLOGY CO., LTD
	All rights reserved.

	Created:		2012/9/19
	Filename: 		busi_shm_mgr.h
	Description:	业务共享内存管理类。
	                提供产品化psshm等命令的业务数据接口；
					提供业务共享内存的管理接口（创建、销毁、加载数据、清除数据等）；
					特别说明：此管理类实现的import/export接口仅实现数据的导入/导出，未实现索引同步更新；

	History:
	<table>
		Revision	Author			Date		Description
		--------	------			----		-----------
		v1.0		raozh		 2012/9/19	    完成类接口的定义
	</table>
*******************************************************************/

#ifndef BUSI_SHM_MGR_H_
#define BUSI_SHM_MGR_H_

#include "busi_shm_struct.h"
#include "es/util/Singleton.h"
#include <string>
#include <vector>
#include <map>

namespace tpss
{

typedef ns_shm_data::ShmStatInfo ShmStatInfo;
typedef ns_shm_data::ShmTableInfo ShmTableInfo;

//错误码定义
const int SHMTABLE_NOT_EXISTS = -100001;         //查询的内存表不存在
const int CONDITION_IS_INVALID = -100002;		 //查询条件不合法
const int TABLE_NOT_EXISTS = -100003;            //数据库表不存在

//每页show的记录行数
const int SHOW_PAGE_SIZE = 2;

//业务共享内存管理类
class CBusiShmMgr
{
public:
    CBusiShmMgr() {};
    ~CBusiShmMgr();

    //0.通用接口
    //初始化函数

    static CBusiShmMgr* Instance();
    //获取错误信息
    void GetErrorMsg( int& err_code, std::string& err_msg ) {
        err_code = err_code_;
        err_msg = err_msg_;
    };

public:
    //1.产品化命令集接口

    //查询所有业务共享内存统计信息
    //对应产品化命令: psshm -stat
    bool QueryStatInfo( std::vector<ShmStatInfo>& vec_stat_info );

    //查询所有业务共享内存表信息
    //对应产品化命令：规范不要求，可自行扩展
    bool QueryTableInfo( std::vector<ShmTableInfo>& vec_table_info );

    //查看表记录
    //对应产品化命令: psshm -show table_name
    //返回值：1 后续有记录 0 结束 -1 出错
    int ShowTable( const std::string& table_name,
                   std::vector<std::vector<std::pair<std::string, std::string> > >& vec_record );
    //对应产品化命令: psshm -show table_name [indexcolumn=value]
    //返回值：1 后续有记录 0 结束 -1 出错
    //indexcolumn要支持所有列，非索引使用遍历实现
    int ShowTable( const std::string& table_name, const std::string& condition,
                   std::vector<std::vector<std::pair<std::string, std::string> > >& vec_record );
    //分页复位.复位到第一页
    bool ShowReset();


    //统计记录数
    //对应产品化命令: psshm -count table_name
    //返回值：>=0 记录数  -1 出错
    int QueryRecordCount( const std::string& table_name );
    //对应产品化命令: psshm -count table_name [indexcolumn=value]
    //返回值：>=0 记录数  -1 出错
    int QueryRecordCount( const std::string& table_name, const std::string& condition );

    //查询锁状态
    //对应产品化命令: psshm -lockstat [table_name]
    //返回值：0 正常  1加锁  -1出错
    int QueryLockStatus( const std::string& table_name );
    //重置锁状态
    //对应产品化命令: psshm -lockreset [table_name] [,table_name]
    //返回值：0 正常  1加锁  -1出错
    int LockReset( const std::string& table_name );

    //导入
    //对应产品化命令: psimport –m disct –t disct_data
    //返回值：>=0 成功导入的记录数 -1 出错
    int ImportFromDB( const std::string& shm_name, const std::string& table_name );
    //对应产品化命令：psimport –m disct –f file_name
    //返回值：>=0 成功导入的记录数 -1 出错
    int ImportFromFile( const std::string& shm_name, const std::string& file_name );

    //导出
    //对应产品化命令: psexport –m disct –t disct_data
    //返回值：>=0 成功导出的记录数 -1 出错
    int ExportToDB( const std::string& shm_name, const std::string& table_name );
    //对应产品化命令：psexport –m disct –f file_name
    //返回值：>=0 成功导出的记录数 -1 出错
    int ExportToFile( const std::string& shm_name, const std::string& file_name );

    //高额
    //对应产品化命令：psshm -show –highquota
    //返回值：>0:后续有记录  0:结束  -1:出错
    int ShowHighQuota( std::vector<std::vector<std::pair<std::string, std::string> > >& vec_record ) {};
    //高额分页复位.复位到第一页
    bool ShowReset_HighQuota() {};

    //LRU
    //对应产品化命令：psshm -show –lru
    //返回值：>0:后续有记录  0:结束  -1:出错
    int ShowLRU( std::vector<std::pair<std::string, std::string> > & vec_record ) {};
public:
    //2.管理接口（创建、销毁、加载数据、清除数据等）

public:
    //typedef各业务共享内存
    //T_SHMSET_NO_MUTEX

private:
    typedef std::map<std::string, ns_shm_data::IShmActiveObject*> T_SHM_OBJ_MAP;

    bool Init();

    static bool bInstanced;                 //是否已实例化过
    int err_code_;                          //当前错误码
    std::string err_msg_;                   //当前错误信息
    T_SHM_OBJ_MAP map_table_obj_;           //业务共享内存表对象集合(map)
    T_SHM_OBJ_MAP map_index_obj_;           //业务共享内存索引对象集合(map)
    std::string table_name_;                //当前show的table_name
    std::string conditon_;                  //当前show的condition
    int page_index_;                        //当前show的page_index
};

}

#endif  /*BUSI_SHM_MGR_H_*/
