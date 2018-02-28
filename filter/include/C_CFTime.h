
#include  <sys/time.h>

#ifndef _C_CFTIME_H_
#define _C_CFTIME_H_

class MyTimer
{
	public:
		MyTimer();
		int StartRecord();
		int EndRecord();
		const timeval *GetTimeStruct();
		long GetTime();
		~MyTimer();
	private:
		timeval m_start;
		timeval m_end;
		timeval m_time;
	
};


#endif
