#ifndef _ERRCODE_H_
#define _ERRCODE_H_ 
/********************************************************************
 *	Usage		: 定义基础模块的错误代码
 *	Author		: 周亚军
 *  Create date	: 2005-04-29
 *	Version		: 1.0.0
 *	Updatelist	:
 *			1)2005-05-08	周亚军	将无资料接口中的ERR_FILE_OPEN(706)改为
 *									ERR_OPEN_FILE, 避免和ERR_FILE_OPEN(604)冲突;
 *									将OCCI数据库接口中的错误码改为负值;
 *	version: 1.1.2
 ********************************************************************/

//动态解释器涉及的错误代码（H）
#define COMPILE_ERROR_MISSING_RIGHT_BRACKET			101	//函数左右括号不匹配	
#define COMPILE_ERROR_NOT_EXPECTED_CHARACTER		102	//不期望出现的符号	
#define COMPILE_ERROR_AND_PARAM						103	//'&'运算参数错误
#define COMPILE_ERROR_OR_PARAM						104	//'|'运算参数错误
#define COMPILE_ERROR_MISSING_COMMA					105	//漏掉','符号
#define COMPILE_ERROR_INVALID_PARAM					106	//无效的参数
#define COMPILE_ERROR_VARIABLE_NOT_DEFINE			107	//未定义变量名
#define COMPILE_ERROR_ZERO_DIVID					108	//0做除数
#define COMPILE_ERROR_FUNCTION_NOT_DEFINE			109	//未定义的函数名
#define COMPILE_ERROR_MISSING_SINGLE_QUOTATION_MARK 110	//缺少右单引号
#define COMPILE_ERROR_INVALID_NUMBER				111	//无效的数字,里面含有字母
#define COMPILE_ERROR_INVALID_EXP					112	//无效的子表达式或者表达式为空
#define COMPILE_ERROR_CASEEXP_MISSING_SUBEXP		113	//case函数表达式缺少子表达式
#define COMPILE_ERROR_INEXP_MISSING_SUBEXP			114	//in函数表达式缺少子表达式
#define COMPILE_ERROR_SETEXP_FIRSTEXP_NOT_VAR		115	//set函数中第一个表达式不是变量
#define COMPILE_ERROR_DECODEEXP_MISSING_SUBEXP		116	//decode函数中缺少子表达式
#define COMPILE_ERROR_COMMAEXP_MISSING_SUBEXP		117	//comma函数中缺少子表达式
#define COMPILE_RUNTIME_ERROR_UNKNOWN				120	//动态编译运行时错误
#define COMPILE_RUNTIME_ERROR_EXPOVERLIMIT			121	//表达式数目超过255

//以下是可配置插件类所涉及的错误代码
#define	NOT_ENOUGH_MEMORY_TO_APPLY		201	//申请内存空间时出错
#define SELECT_ERROR_FROM_DB			211 //查询数据库出错
#define INSERT_ERROR_TO_DB				212 //插入数据库出错
#define UPDATE_ERROR_TO_DB				213 //更新数据库出错
#define COMPILE_EXCUTE_ERROR			221	//解释器执行条件语句出错
#define OPEN_PLUGIN_ERROR				231	//打开动态插件库出错
#define CLOSE_PLUGIN_ERROR				232	//关闭动态插件库出错
#define CAN_NOT_FIND_PLUGIN_CLASS		233	//动态插件库找不到该插件
#define PLUGIN_STRING_DEFINE_ERR		234	//插件定义串格式错误
#define INIT_CFMT_CHANGE_ERROR			241	//初始化输入输出话单记录实例出错
#define ERR_FILETYPE_MATCH          242 //话单格式转换类                          	

//CFileIn.cpp(M)                    	
#define ERR_IN_FILE_CLOSED				301	//文件处于关闭状态
#define ERR_IN_FILE_HEAD				302	//文件头错误
#define ERR_IN_FILE_BYTE				303	//文件字节数检测错误
#define ERR_IN_FILE_END					304	//文件尾错误
#define ERR_IN_FILE_NUM					305	//文件记录条数错误
#define ERR_IN_FILE_OPEN				306	//打开文件失败
#define ERR_IN_READ_REC					307	//读取记录失败
                                    	
//CFileOut.cpp(M)                   	
#define ERR_OUT_FILE_OPEN				311	//文件打开错误
#define ERR_OUT_WRITE_FILE_HEAD			312	//写文件头错误
#define ERR_OUT_WRITE_REC				313	//文件写入失败
#define ERR_OUT_FILE_CLOSED				314	//文件处于关闭状态
#define ERR_OUT_WRITE_FILE_END			315	//文件尾写失败

//文件记录格式类：
#define ERR_TYPE						401	//记录类型错误
#define ERR_REQ_MEM						402	//申请内存出错
#define ERR_FILETYPEID					403	//记录类型定义不等
#define ERR_COUNT_LESS					404	//记录字段太少
#define ERR_LENGTH_LONG					405	//记录太长
#define ERR_INPUTCOUNT_OVER_UPLIMIT		406	//输入字段数超出定义
#define ERR_INPUTLENGTH_OVER_UPLIMIT	407	//输入字段长度超出定义
#define ERR_INPUTLENGTH_LESS			408	//输入记录长度小于定义

//OCCI数据库接口：（H）
#define	INVALID_PARAMETER				-502	//无效输入参数
#define INVALID_PRE_CONDITION			-503	//前置条件不满足（无效调用）
#define DATA_TYPE_NOT_SUPPORTED			-504	//暂不支持的数据类型
#define DATA_TYPE_INVALID_CONVERSION	-505	//无法转换的数据类型
#define INVALID_INPUT_BIND_VAR_COUNT	-506	//输入绑定变量数目有误
#define INVALID_OUTPUT_BIND_VAR_COUNT	-507	//输出返回值或列数目有误

//wangds话单读写
#define ERR_MMAP                  		601 //申请内存空间出错
#define ERR_DEL_SPACE             		602 //释放内存空间出错
#define ERR_FILELEN_CHANGE        		603 //改变文件大小出错
#define ERR_FILE_OPEN             		604 //打开文件出错
#define ERR_FOR_BILL_BE           		605 //文件头尾记录出错
#define ERR_CLOSE_FILE            		606 //关闭文件出错       
#define ERR_FOR_ACCESS_FILE       		607 //文件不存在
#define ERR_FILE                  		609 //错误文件，字节数小于23
#define ERR_OPEN_FOR_READ_FAIL			610 //只读方式打开文件失败
#define ERR_OPEN_FOR_WRITE_FAIL			611 //读写方式打开文件失败
#define ERR_FILE_NOT_OPEN 	  			612 //文件处于关闭状态
#define ERR_RECORD_TOO_LONG         613//文件记录太长

//无资料接口
#define ERR_FILE_WRITE					701 //文件在写入的时候发生错误，系统忙或者I/O出错
#define ERR_GET_RECORD					702 //读取话单字段出错，改出错产生由CFmt_Change返回
#define ERR_DIR_CREATE					703 //生成子目录出错，在生成目录时因为系统原因导致子目录生成错误
#define ERR_DIR_CHANGE					704 //进入子目录错误，没有权限访问指定的目录，用户对文件的权限不够
#define ERR_DIR_NULLITY					705 //程序在运行过程中要求使用绝对路径。
#define ERR_OPEN_FILE					706 //文件打开错误，用户没有权限打开改文件
#define ERR_FILE_CLOSE					707 //文件关闭时错误，系统忙或者文件被移动
#define ERR_RENAME_FILE					708 //对文件改名出错，系统忙，或者临时文件被移动
#define ERR_REMOVE_FILE					709 //删除临时文件时出错，系统忙，或者临时文件被移动
#define ERR_DIR_OPEN					710 //指定的目录不存在，或者用户没有权限访问该目录

//流水配置信息及调度表信息接口
#define SCHINFO_ERR_CONFIG_PIPE				801 //PIPE表中找不到相应配置
#define SCHINFO_ERR_CONFIG_INTERFACE		802 //INTERFACE表中找不到相应配置
#define SCHINFO_ERR_CONFIG_MODEL_INTERFACE	803 //MODEL_INTERFACE表中找不到相应配置
#define SCHINFO_ERR_CONFIG_FILETYPE_DEFINE	804 //FILETYPE_DEFINE表中找不到相应配置
#define SCHINFO_ERR_CONFIG_SOURCE			804 //SOURCE表中找不到相应配置
#define SCHINFO_ERR_UPDATE_TABLE			805 //更新入口调度表失败
#define SCHINFO_ERR_INSERT_TABLE			806 //向出口调度表插入记录失败

//进程监控信息共享内存接口
#define PROCINFO_ERR_CREATE_FAIL			901 //创建监控信息共享内存失败
#define PROCINFO_ERR_DESTROY_FAIL			902 //删除监控信息共享内存失败
#define PROCINFO_ERR_ATTACH_FAIL			903 //连接监控信息共享内存失败
#define PROCINFO_ERR_DETACH_FAIL			904 //断开和监控信息共享内存连接失败
#define PROCINFO_ERR_READ_FAIL				904 //读取监控信息共享内存失败
#define PROCINFO_ERR_WRITE_FAIL				905 //写入监控信息共享内存失败
#define PROCINFO_ERR_MEMIDX					906 //错误的进程索引号
#define PROCINFO_ERR_PROCESS_ALREADY_RUN	907 //已有进程在运行

//处理日志接口错误类型
#define DEALLOG_ERR_IN_OPEN_FILE      1001
#define DEALLOG_ERR_IN_WRITE_FILE     1002
#define DEALLOG_ERR_IN_GET_ENV        1003

#endif//_ERRCODE_H_
