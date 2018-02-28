
#include "C_CFileLock.h"

//*********************************************************
//********************C_FILELOCK***************************
//*********************************************************
void C_CFileLock::Init(const char* szfile)
{
	sprintf(m_szfile, "%s", szfile);
	m_fd = open(m_szfile, O_RDWR|O_CREAT, 0666);
	if(m_fd == -1)
	{
		//TODO throw here
		char szMsg[512];
		sprintf(szMsg, "open file=%s= err", szfile);
		throw jsexcp::CException(RATE_ERR_IN_OPEN_FILE, szMsg, __FILE__, __LINE__);
	}
}
void C_CFileLock::Lock()
{
	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	fcntl(m_fd, F_SETLKW, &lock);
}
void C_CFileLock::UnLock()
{
	struct flock lock;
	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	fcntl(m_fd, F_SETLKW, &lock);
}

int C_CFileLock::getFileSize()
{
  struct stat stat;
   
  return((fstat(m_fd,&stat) < 0) ? -1 : stat.st_size);
}

//**********************************************************

