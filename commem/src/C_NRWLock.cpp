
#include "C_NRWLock.h"


//初始化读写锁
bool C_NRWLock::InitLock(pthread_rwlock_t *rwptr,const pthread_rwlockattr_t *attr)
{
	if(0 == pthread_rwlock_init(rwptr,attr))
	{
		return true;
	}
	else
	{
		return false;
	}
}

//销毁读写锁
bool C_NRWLock::DestroyLock(pthread_rwlock_t *rwptr)
{
	if(0 == pthread_rwlock_destroy(rwptr))
	{
		return true;
	}
	else
	{
		return false;
	}
}

//得到读锁
bool C_NRWLock::GetReadLock(pthread_rwlock_t *rwptr)
{
	if(0 == pthread_rwlock_rdlock(rwptr))
	{
		return true;
	}
	else
	{
		return false;
	}
}

//得到写锁
bool C_NRWLock::GetWriteLock(pthread_rwlock_t *rwptr)
{
	if(0 == pthread_rwlock_wrlock(rwptr))
	{
		return true;
	}
	else
	{
		return false;
	}
}

//释放锁
bool C_NRWLock::UnLock(pthread_rwlock_t *rwptr)
{
	if(0 == pthread_rwlock_unlock(rwptr))
	{
		return true;
	}
	else
	{
		return false;
	}
}

