/****************************************************************
filename: CF_PREP_Error.h
module: classify&analyse
created by: Wulf
create date: 2005-12-23
update list: 
version: 1.1.1
description:
    the header file of the classes for error
*****************************************************************/

#ifndef _CF_PREP_CERROR_H_
#define _CF_PREP_CERROR_H_ 1

#include <string>


const int PREDEAL_ERR_NOT_ENOUGH_MEMORY  = 5001;  //动态分配内存失败
const int PREDEAL_ERR_IN_SELECT_DB       = 5002;  //查询数据出错
const int PREDEAL_ERR_IN_CONNECT_DB      = 5003;  //连接数据库出错
const int ERR_SHM_ERROR       					 = 5004;  //共享内存访问出错
const int PREDEAL_ERR_SLFILE_NOT_EXIT    = 5005;  //插件文件不存在
const int PREDEAL_ERR_PK_DUPLICATE       = 5006;  //数据表主键重复
const int PREDEAL_ERR_UNKNOWN_CATCH      = 5007;  //捕捉到无法识别的错误类
const int PREDEAL_ERR_NEED_RESTART       = 5008;  //严重错误，进程需重新启动
const int  ERR_LACK_PARAM      			=		     5009;  //插件参数个数不正确
const int ERR_SEM_ERROR                  = 5010;  //信号量访问出错
const int ERR_NEW_STRUCT                 = 5011;  //插件申请结构体空间出错
const int PREDEAL_ERR_SERVER_DIF         = 5012;  //zhjs.env中的SERVER与PIPE中的不一样
const int ERR_GET_FILE                   = 5013;  //读取队列中的文件出错

#endif
