
#include "C_CFileLock.h"

//*********************************************************
//********************C_FILELOCK***************************
//*********************************************************
/*˵��:��ʼ���ļ�����
  *����:ȫ·���ļ���
  *����:��
  *�쳣:CException e */
void C_CFileLock::Init(const char* szfile)
{
	if(szfile == NULL)
	{
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "input argument=szfile= is null");
		throw CException(FILTER_ERR_IN_OPEN_FILE, szMsg, __FILE__, __LINE__);
	}
	
	sprintf(m_szfile, "%s", szfile);
	m_fd = open(m_szfile, O_RDWR|O_CREAT, 0666);
	if(m_fd == -1)
	{
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "open file=%s= err", szfile);
		throw CException(FILTER_ERR_IN_OPEN_FILE, szMsg, __FILE__, __LINE__);
	}
}
/*˵��:����
  *����:��
  *����:��
  *�쳣:CException e */
void C_CFileLock::Lock()
{
	if(m_fd == -1)
	{
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "haven't init yet!");
		throw CException(FILTER_ERR_IN_OPEN_FILE, szMsg, __FILE__, __LINE__);
	}
	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	fcntl(m_fd, F_SETLKW, &lock);
}

/*˵��:����
  *����:��
  *����:��
  *�쳣:CException e */
void C_CFileLock::UnLock()
{
	if(m_fd == -1)
	{
		char szMsg[FILTER_ERRMSG_LEN];
		sprintf(szMsg, "haven't init yet!");
		throw CException(FILTER_ERR_IN_OPEN_FILE, szMsg, __FILE__, __LINE__);
	}
	struct flock lock;
	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	fcntl(m_fd, F_SETLKW, &lock);
}

//**********************************************************

