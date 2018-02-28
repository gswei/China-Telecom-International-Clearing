
#include "C_CFTime.h"

MyTimer::MyTimer()
{
	m_start.tv_sec = 0;
	m_start.tv_usec = 0;
	m_end.tv_sec = 0;
	m_end.tv_usec = 0;
	m_time.tv_sec = 0;
	m_time.tv_usec = 0;
}

int MyTimer::StartRecord()
{
	struct timezone tz;
	return gettimeofday(&m_start, &tz);
}

int MyTimer::EndRecord()
{
	struct timezone tz;
	return gettimeofday(&m_end, &tz);
}

const timeval * MyTimer::GetTimeStruct()
{
	m_time.tv_sec = m_end.tv_sec - m_start.tv_sec;
	m_time.tv_usec = m_end.tv_usec - m_start.tv_usec;
	return &m_time;
}

long MyTimer::GetTime()
{
	m_time.tv_sec = m_end.tv_sec - m_start.tv_sec;
	m_time.tv_usec = m_end.tv_usec - m_start.tv_usec;
	return (m_time.tv_sec * 1000 * 1000 + m_time.tv_usec);
}

MyTimer::~MyTimer()
{
}

