
#include "C_NRWLock.h"


//��ʼ����д��
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

//���ٶ�д��
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

//�õ�����
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

//�õ�д��
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

//�ͷ���
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

