/*
 * =====================================================================================
 *
 *       Filename:  C_NRWLock.h
 *
 *    Description:  读写锁
 *
 *        Version:  3.0
 *        Created:  2010年05月17日 08时40分48秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Sunhua,
 *        Company:  eshore
 *
 * =====================================================================================
 */

#ifndef __C_NRWLOCK_H__
#define __C_NRWLOCK_H__

#include <pthread.h>

class C_NRWLock
{
public:
	//初始化读写锁
	bool InitLock(pthread_rwlock_t *rwptr,const pthread_rwlockattr_t *attr);
	//销毁读写锁
	bool DestroyLock(pthread_rwlock_t *rwptr);	
	//得到读锁
	bool GetReadLock(pthread_rwlock_t *rwptr);
	//得到写锁
	bool GetWriteLock(pthread_rwlock_t *rwptr);
	//释放锁
	bool UnLock(pthread_rwlock_t *rwptr);
};

#endif
