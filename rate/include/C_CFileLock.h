#ifndef _C_CFILELOCK_
#define _C_CFILELOCK_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "CF_CException.h"
#include "rate_error_define.h"

const int FILE_NAME_LENGTH = 512;

class C_CFileLock
{
	public:
		C_CFileLock()
		{
			memset(m_szfile, 0, sizeof(m_szfile));
			m_fd = -1;
		};
		~C_CFileLock()
		{
			if(m_fd != -1)
			{
				close(m_fd);
			}
		};
		void Init(const char* szPath);
		void Lock();
		void UnLock();
		int getFileSize();
	private:
		char m_szfile[FILE_NAME_LENGTH];
		int m_fd;
};

#endif

