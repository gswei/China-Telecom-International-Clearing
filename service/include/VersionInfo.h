/****************************************************************
filename: VersionInfo.h
module: classify&analyse
created by: Wu Longfeng
create date: 2006/12/28
version: 1.0.0
description:
	所有预处理模块的版本信息
update list:
	20071107 修改输入输出接口，将文件改成文件和数据库通用的接口用于测试
	20071127细化分拣等规则，并去掉SOURCE_INFO表信息的读取
	20080131增加无资料原因字段表示，用于分拣处理插件
	20080202增加时间统计信息，方便查找系统资源消耗
	20080529修改无资料代码，变更程序版本
	20080619增加文件处理优先级功能，并解决HP机器限制和同步通信bug
	20080707修改号码转换资料使用方式，使其可以同步更新
	20080806修改号码转换资料同步更新bug
	20080924修改底层表达式个数限制，重新编译程序
	20081007修改号码转换1严格精确匹配方式，原有方式有bug
*****************************************************************/

#ifndef _VERSIONINFO_H_
#define _VERSIONINFO_H_ 1

const char SYSTEM_NAME[]		=	"YWZH";						//系统名
const char WHO_UPDATE[]			=	"Ouyh";						//更新人
const char MODULE_NAME[]		=	"service_platform";				//模块名
const char VERSION_NO[]			=	"3.0.0";					//版本号
const char UPDATE_TIME[]		=	"2010-07-20";			//更新日期s

#define PLUGIN_ENV_NAME "PREP_PLUGIN"
//const char PLUGIN_ENV_NAME[]=	"PREP_PLUGIN";  //插件文件的变量名
const char PREDEAL_SL_NO[]	=	".0"; 						  //插件文件版本号
const char PLUGIN_NAME[]		=	"ywzhPlugin.sl";		  //插件文件的名字


#endif

