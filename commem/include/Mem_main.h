/*
 * =====================================================================================
 *
 *       Filename:  Mem_main.cpp
 *
 *    Description:  将数据库中的数据表放入共享内存区，并保持和数据库的同步，使其它程序更快的完成数据访问
 *
 *        Version:  1.0
 *        Created:  2010年05月09日 08时59分13秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *        Company:
 *
 *    update list: 
 *    2010-05-09 :  初始版本

 * =====================================================================================
 */

#ifndef _COMMON_MAIN_H_
#define _COMMON_MAIN_H_

#include "CommonMemManager.h"
//#include "CF_COracleDB.h"
#include "CF_CReadIni.h"
#include <stdlib.h> 
#include "es/util/StringUtil.h"


//检查 commem -c 进程是否存在
int ProcIsExist(int argc, char *argv[]);
//int commenKeyValue; //   共享内存值，从核心参数获取
//杀掉进程
void  KillProc(int iProc);

#endif
