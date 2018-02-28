/*
 * =====================================================================================
 *
 *       Filename:  S_CommonTable.h
 *
 *    Description:  管理与同步共享内存
 *
 *        Version:  1.0
 *        Created:  2010年04月03日 08时40分48秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zhengxx,
 *        Company:  Poson
 *
 * =====================================================================================
 */

#ifndef __S_COMMON_TABLE_H__
#define __S_COMMON_TABLE_H__


#define MEM_MAX_COUNT 100
#define MEM_MAX_INDEX	10
#define BUFFER 101

	struct S_CommonTableInfo{
		char MemName[BUFFER];
		int	version;
		char chTableName[BUFFER];
		int	iMemSize;
		int iRealRows;
		int iMemRows;
		key_t iTableKey;
		int ishmid;
		int iIdxKey[MEM_MAX_INDEX];
		int iIdxshmid[MEM_MAX_INDEX];
		//key_t isemkey;
		//int isemid;
	};
	
	struct S_MemManager{
		int isUsed;
		int iTotalTable;
		S_CommonTableInfo m_TableInfo[MEM_MAX_COUNT+1];
		
		pthread_rwlock_t memlock[1];

	};


#endif
