/***********************************************************
Project: LBAS 
	Copyright (c) 2000-2003. All Rights Reserved by Poson
Subsystem:
Filename: config.h
Created by: lihuah
Created date: 2001/06/02
Version: 1.2.1
=============================================================
Description:
	通用头文件,.h文件
Last update: 2003/07/17
Update list:
	1.	修改时间:2003/07/16
		修改原因:使用程序同时兼容LINUX平台
	2.	修改时间:
		修改原因:
************************************************************/
#ifndef __CF_CCONFIG_H
#define __CF_CCONFIG_H

// choose target platform: _WINDOWS | _SUNOS | _HPOS | _UNIXWARE
// choose target compiler: _MS_VC | _SUN_CC | _HP_ACC | _UW_CC

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <list>
#include <vector>
#include <set>
#include <queue>
#include <algorithm>
#include <numeric>
#include <fstream.h>
#include <iostream.h>
#include <iomanip.h>
#include <stdexcept>
#include <regex.h>
//add by monkey10
#include <map>
//#include <assert.h>
#include <unistd.h>
#include <utility>
#include <dlfcn.h>

#include <strings.h>
#include <errno.h>


#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/sem.h> 
#include <sys/shm.h> 
#include <sys/ipc.h> 


//union semun {
//        int val;                                        /* value for SETVAL */
//        struct semid_ds *buf;           /* buffer for IPC_STAT & IPC_SET */
//        unsigned short *array;          /* array for GETALL & SETALL */
//        struct seminfo *__buf;          /* buffer for IPC_INFO */   //test!!
//        void *__pad;
//};


#ifdef _MS_VC
#include <strstrea.h>
#else
#include <iostream.h>
#endif

#ifdef _HP_ACC
#define _NOSTD
#endif

#ifdef _LINUX_ACC
#define _NOSTD
#endif

#ifndef _NOSTD
	using std::copy;
	using std::equal_range;
	using std::back_insert_iterator;
	using std::ostream_iterator;
	using std::pair;
	using std::min_element;
	using std::string;
	using std::list;
	using std::vector;
	using std::set;
	using std::find;
	using std::find_if;
	using std::binary_search;
	using std::lower_bound;
	using std::upper_bound;
	using std::accumulate;
	using std::range_error;
	using std::sort;
	using std::map;
	using std::less;
	using std::out_of_range;

	inline ostream& operator<<(ostream& os, const string& tmp)
	{
		return os << tmp.c_str();
	}
	template <class A, class B> A min(const A& a, const B& b) { return (a > b ? b:a); };
	template <class A, class B> A max(const A& a, const B& b) { return (a < b ? b:a); };
#endif

#ifndef _WINDOWS
#define ACCOUNT_LOG_QUEUE_VAR ("ACCOUNT_LOG_QUEUE_KEY")
#define ACCOUNT_LOG_QUEUE_TYPE 1
#define USING_LOG_QUEUE
#endif

using namespace std;
using std::string;
using std::vector;

#endif
