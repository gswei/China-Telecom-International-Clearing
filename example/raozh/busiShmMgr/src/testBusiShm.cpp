#include "busi_shm_mgr.h"
#include <iostream>

using namespace tpss;

int main( int argc, char** argv )
{
    CBusiShmMgr* pShmMgr = CBusiShmMgr::Instance();


    //QueryTableInfo
    printf( "/****************QueryTableInfo*******************/\n" );
    std::vector<ShmTableInfo> vecTable;
    pShmMgr->QueryTableInfo( vecTable );

    for( std::vector<ShmTableInfo>::const_iterator it = vecTable.begin();
            it != vecTable.end(); it++ ) {
        printf( "table_name=%s, record_count=%d, size=%d, index_column=%s \n",
                it->table_name, it->record_count, it->size, it->index_column );
    }

    //QueryStatInfo
    printf( "/****************QueryStatInfo*******************/\n" );
    std::vector<ShmStatInfo> vecStat;
    pShmMgr->QueryStatInfo( vecStat );

    for( std::vector<ShmStatInfo>::const_iterator it = vecStat.begin();
            it != vecStat.end(); it++ ) {
        printf( "shm_id=%d, table_name=%s, create_time=%d, lock_status=%d, size=%d, used_size=%d, \
			   free_percent=%d, unused_percent=%d \n",
                it->shm_id, it->shm_name, it->create_time, it->lock_status, it->size,
                it->used_size, it->free_percent, it->unused_percent );
    }

    //ShowTable
    //printf("/****************ShowTable*******************/\n");
    /*
    std::vector<std::vector<std::pair<std::string, std::string> > > vec_record;
    printf("第1页\n");
    pShmMgr->ShowTable("AcctItemAggr", vec_record);
    for (std::vector<std::vector<std::pair<std::string, std::string> > >::const_iterator it = vec_record.begin();
        it != vec_record.end(); it++)
    {
    	for (std::vector<std::pair<std::string, std::string> >::const_iterator it2 = it->begin();
    		it2 != it->end(); it2++)
    	{
    		printf("[%s] = %s \n", it2->first, it2->second);
    	}

    	printf("\n");
    }


    printf("第2页\n");
    vec_record.clear();
    pShmMgr->ShowTable("AcctItemAggr", vec_record);
    for (std::vector<std::vector<std::pair<std::string, std::string> > >::const_iterator it = vec_record.begin();
        it != vec_record.end(); it++)
    {
    	for (std::vector<std::pair<std::string, std::string> >::const_iterator it2 = it->begin();
    		it2 != it->end(); it2++)
    	{
    		printf("[%s] = %s \n", it2->first, it2->second);
    	}

    	printf("\n");
    }

    printf("第3页\n");
    vec_record.clear();
    pShmMgr->ShowReset();
    pShmMgr->ShowTable("AcctItemAggr", vec_record);
    for (std::vector<std::vector<std::pair<std::string, std::string> > >::const_iterator it = vec_record.begin();
        it != vec_record.end(); it++)
    {
    	for (std::vector<std::pair<std::string, std::string> >::const_iterator it2 = it->begin();
    		it2 != it->end(); it2++)
    	{
    		printf("[%s] = %s \n", it2->first, it2->second);
    	}

    	printf("\n");
    }
    */

    //ShowTable(带条件)
    //printf("/****************ShowTable*******************/\n");
    /*
    std::vector<std::vector<std::pair<std::string, std::string> > > vec_record;
    pShmMgr->ShowTable("AcctItemAggr", "serv_id=8977897899,acct_item_id=3", vec_record);
    for (std::vector<std::vector<std::pair<std::string, std::string> > >::const_iterator it = vec_record.begin();
        it != vec_record.end(); it++)
    {
    	for (std::vector<std::pair<std::string, std::string> >::const_iterator it2 = it->begin();
    		it2 != it->end(); it2++)
    	{
    		printf("[%s] = %s \n", it2->first, it2->second);
    	}

    	printf("\n");
    }
    */

    //QueryRecordCount
    printf( "/****************QueryRecordCount*******************/\n" );
    int record_count = pShmMgr->QueryRecordCount( "AcctItemAggr" );
    printf( "recordCount: %d \n", record_count );
    int record_count2 = pShmMgr->QueryRecordCount( "AcctItemAggr", "serv_id=8977897899" );
    printf( "recordCount2: %d \n", record_count2 );

    //QueryLockStatus
    printf( "/****************QueryLockStatus*******************/\n" );
    int lock_status = pShmMgr->QueryLockStatus( "AcctItemAggr" );
    printf( "lock_status: %d \n", lock_status );
    pShmMgr->LockReset( "AcctItemAggr" );
    lock_status = pShmMgr->QueryLockStatus( "AcctItemAggr" );
    printf( "lock_status: %d \n", lock_status );

    //ExportToFile
    //printf("/****************ExportToFile*******************/\n");

    std::string file_name = "test";
    int ret = pShmMgr->ExportToFile( "AcctItemAggr", file_name );

    if( ret == -1 ) {
        int err_code;
        std::string err_msg;
        pShmMgr->GetErrorMsg( err_code, err_msg );
        printf( "export error: err_code = %d, err_msg = %s \n", err_code, err_msg );
    } else {
        printf( "export %d record to file %s \n", record_count, file_name );
    }


    //ImportFromFile
    //printf("/****************ImportFromFile*******************/\n");
    /*
    std::string file_name = "test";
    int ret = pShmMgr->ImportFromFile("AcctItemAggr", file_name);
    if (ret == -1)
    {
    	int err_code;
    	std::string err_msg;
    	pShmMgr->GetErrorMsg(err_code, err_msg);
    	printf("import error: err_code = %d, err_msg = %s \n", err_code, err_msg);
    }
    else
    {
    	printf("import %d record from file %s \n", ret, file_name);
    }
    */

    //ExportToDB
    //printf("/****************ExportToDB*******************/\n");
    /*
    std::string table_name = "tmp_raozh_shmexp_test";
    int ret = pShmMgr->ExportToDB("AcctItemAggr", table_name);
    if (ret == -1)
    {
    	int err_code;
    	std::string err_msg;
    	pShmMgr->GetErrorMsg(err_code, err_msg);
    	printf("export error: err_code = %d, err_msg = %s \n", err_code, err_msg);
    }
    else
    {
    	printf("export %d record to table %s \n", ret, table_name);
    }
    */

    //ImportFromDB
    //printf("/****************ImportFromDB*******************/\n");
    /*
    std::string table_name = "tmp_raozh_shmexp_test";
    int ret = pShmMgr->ImportFromDB("AcctItemAggr", table_name);
    if (ret == -1)
    {
    	int err_code;
    	std::string err_msg;
    	pShmMgr->GetErrorMsg(err_code, err_msg);
    	printf("import error: err_code = %d, err_msg = %s \n", err_code, err_msg);
    }
    else
    {
    	printf("import %d record from table %s \n", ret, table_name);
    }
    */

    printf( "the end\n" );
}