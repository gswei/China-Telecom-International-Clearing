/*
 * =====================================================================================
 *
 *       Filename:  C_NRWLock.h
 *
 *    Description:  ��д��
 *
 *        Version:  3.0
 *        Created:  2010��05��17�� 08ʱ40��48��
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
	//��ʼ����д��
	bool InitLock(pthread_rwlock_t *rwptr,const pthread_rwlockattr_t *attr);
	//���ٶ�д��
	bool DestroyLock(pthread_rwlock_t *rwptr);	
	//�õ�����
	bool GetReadLock(pthread_rwlock_t *rwptr);
	//�õ�д��
	bool GetWriteLock(pthread_rwlock_t *rwptr);
	//�ͷ���
	bool UnLock(pthread_rwlock_t *rwptr);
};

#endif
