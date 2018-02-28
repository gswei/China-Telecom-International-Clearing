/*
 * =====================================================================================
 *
 *       Filename:  CommonMemManager.h
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

#ifndef __COMMON_MEM_MANAGER_H__
#define __COMMON_MEM_MANAGER_H__
#include "CommonMemTableDefine.h"
#include <vector>
#include "CommonMemRecord.h"
#include "CommonMemTable.h"
#include "S_CommonTable.h"
#include "C_NRWLock.h"
#include "CF_Common.h"
#include "CF_CReadIni.h"
#include "psutil.h"
#include "es/util/StringUtil.h"

#define DATABASE CDatabase

using std::vector;
	

	class CommonMemManager {
	public:
		CommonMemManager();
		void init(char * env_path );
		void createCommonTable();
		void AttachCommonTable();
		void DetachCommonTable();
		void ReadCommonInfo(int readLine);
		void loadAllTable();
		void freeAllTable();
		void destoryAllTable();
		void deleteCommonTable();
		void searchUpdate();
		void detach();
		void AttachTable(string mem_name1);	
		void queryTableRecord(string mem_name,const char *queryCondition,int id);
		~CommonMemManager();
	protected:
		int InitCommonMem();
	private:
		S_MemManager *commonTable;
		S_CommonTableInfo tableLine;
		int m_ishmidcommon;
		key_t m_icommonkey;
		vector<CommonMemTableDefine* > vtable;
		string serv_id;
		string shm_path;
		string mem_path;
		string sem_path;
		//long iSleepTime;
		C_NRWLock m_RWLock;
		DBConnection conn;//数据库连接
		void killSingleMem(int ismidkey);
	};



#endif
