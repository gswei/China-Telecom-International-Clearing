#include <stdlib.h>
#include "acct_item_aggr.h"
#include "acct_item_owe.h"
#include "acct_item_type.h"
#include "address_party_rel.h"
#include "calc_result_detail.h"
#include "channel.h"
#include "channel_equip_rel.h"
#include "channel_member.h"
#include "grid.h"
#include "partner.h"
#include "partner_offer_instance.h"
#include "payment.h"
#include "product_offer.h"
#include "ratable_resource_accumulator.h"
#include "serv_channel_rel.h"
#include "serv_equip.h"
#include "serv_grid_rel.h"
#include "serv_location.h"
#include "serv_terminal.h"
#include "settle_item_detail.h"
#include "sim_card_active.h"
#include "sim_card_party_rel.h"
#include "stamp_info.h"
#include "terminal_regist.h"
#include "tif_fee_bill.h"
#include "WindleSharedMemory.h"
#include "CumulantSharedMemory.h"
#include "info.h"
#include "offer.h"


using namespace tpss;

//只支持一个参数
//这个参数可以有以下类型
//-s AcctItemAggr  显示(show)
//-c AcctItemAggr  创建(create)
//-l AcctItemAggr  加载(load)
//-e AcctItemAggr  清空(erase)
//-r AcctItemAggr  删除(rm)
//-h 帮助(help)
//-c All           批量创建
//-l All           批量加载数据
//-d All           批量删除共享内存里面的数据

const char* module_names[] = {SHMNAME_ACCT_ITEM_AGGR, SHMNAME_ACCT_ITEM_OWE,
                              SHMNAME_ACCT_ITEM_TYPE, SHMNAME_ADDRESS_PARTY_REL, SHMNAME_CALC_RESULT_DETAIL, SHMNAME_CHANNEL, SHMNAME_CHANNEL_EQUIP_REL, SHMNAME_CHANNEL_MEMBER, SHMNAME_GRID, SHMNAME_PARTNER, SHMNAME_PARTNER_OFFER_INSTANCE, SHMNAME_PAYMENT, SHMNAME_PRODUCT_OFFER, SHMNAME_RATABLE_RESOURCE_ACCUMULATOR, SHMNAME_SERV_CHANNEL_REL, SHMNAME_SERV_EQUIP,SHMNAME_SERV_GRID_REL, SHMNAME_SERV_LOCATION, SHMNAME_SERV_TERMINAL, SHMNAME_SETTLE_ITEM_DETAIL,
                              SHMNAME_SIM_CARD_ACTIVE, SHMNAME_SIM_CARD_PARTY_REL,
                              SHMNAME_STAMP_INFO, SHMNAME_TERMINAL_REGIST,SHMNAME_TIF_FEE_BILL, SHMNAME_WINDLE_SHM_RECORD, SHMNAME_CUMULANT_SHM_RECORD, "info", "offer"};


const char* index_names[] = {INDEX_OFFERINSTID_ACCT_ITEM_AGGR, INDEX_OFFERID_ACCT_ITEM_OWE,INDEX_CHANNELID_ADDRESS_PARTY_REL, INDEX_PARTYID_CHANNEL, INDEX_JUNCTION_CHANNEL_EQUIP_REL,INDEX_SUBLINE_CHANNEL_EQUIP_REL, INDEX_ACCNBR_SIM_CARD_ACTIVE,INDEX_ACCNBR_SIM_CARD_PARTY_REL, INDEX_CHANNELID_SERV_CHANNEL_REL,     				INDEX_SERVID_SERV_GRID_REL, INDEX_SERVID_SETTLE_ITEM_DETAIL};

const char* index_module_names[] = {SHMNAME_ACCT_ITEM_AGGR, SHMNAME_ACCT_ITEM_OWE, SHMNAME_ADDRESS_PARTY_REL, SHMNAME_CHANNEL, SHMNAME_CHANNEL_EQUIP_REL, SHMNAME_CHANNEL_EQUIP_REL, SHMNAME_SIM_CARD_ACTIVE, SHMNAME_SIM_CARD_PARTY_REL, SHMNAME_SERV_CHANNEL_REL,SHMNAME_SERV_GRID_REL, SHMNAME_SETTLE_ITEM_DETAIL};

const char* index_field_names[] = {"offer_inst_id", "offer_id", "channel_id", "party_id", "junction_box_code", "sub_line_box_code", "acc_nbr", "acc_nbr", "channel_id", "serv_id", "serv_id"};                            
                              
const char* action_names[] = {"-c", "-r", "-s", "-e", "-l", "-wl", "-rl", "-ul", "-si", "-ri", "-id", "-ed", "-if", "-ef", "-m", "-i", "-h"};

const char* description[] = {"-c [module_name]: create shared memory", 
							 "-r [module_name]: remove the shared meomory",
							 "-s [module_name]: show  the datas from shared memory",
							 "-e [module_name]: erase the datas from shared memory",
							 "-l [module_name]: load the datas from database",
							 "-wl [module_name]: lock shared memory's write locked mode",
							 "-rl [module_name]: lock shared memory's read locked mode",
							 "-ul [module_name]: unlock shared memory's read and write locked mode",
                             "-si [module_name]: show the module's ids of shared memory",
							 "-id [database] [module_name]: import the datas from database to shared memory",
							 "-ed [database] [module_name]: export the datas from shared memory to database",
							 "-if [filename] [module_name]: import the datas from file to shared memory", 
							 "-ef [filename] [module_name]: export the datas from shared memory to file",
                             "-ri [index_name]: remove the index of shared memory",
                             "-m: show all the modules",
                             "-i: show the indexs of all the modules",
							 "[-c,-r,-s,-e,-l,-wl,-rl,-ul] [All]: do operations to all the shared memory", 
                             "-d [-c, -r, -s, -e, -l, -wl .. etc] [^,additon]  [module_name]: -d, go into debug mode",
							 "-h: about the command"};


union ADDITION {
    char* ptr;
    int   id;
};
							 
template <typename T>
bool Create( const char* shm_name ) {
    bool ret = true;
    if (T::Instance(true)) {
        ret = true;
        //printf( "create shared memory for %s module successd!\n", shm_name );
    } else {
        ret = false;
        printf( "create shared memory for %s module failed\n", shm_name );
    }
    return ret;
}

int GetSemId(const char* shm_name) {
	int sem_id = -1;
	char t_sql[101];
	snprintf(t_sql, 100, "select semid from billing_sem where name = '%s'", shm_name);
	try {
		theDS.setSQLString(t_sql);
		if (theDS.execute()){
			theDS >> sem_id;
		}
	} catch (SQLException ex) {
			writelog(ex);
			sem_id = -1;
    }
	return sem_id;
}

template <typename T>
bool Destroy( const char* shm_name ) {
    bool ret = true;
	
    IShmResolver* ptr = T::Instance( false );

    if( NULL == ptr ) {
        printf( "instance %s module failed!\n", shm_name );
        return false;
    }

    if (!ptr->DestroyShm()) {
        printf("remove %s's shared memory and semaphore failed!\n", shm_name);
        return false;
    } else {
        ret = true;
        // printf("remove %s's shared memory and semaphore success!\n", shm_name);
    }

    return ret;
}

template <typename T>
bool DestroyIndex( const char* shm_name, const char* index_name) {
    bool ret = true;
    IShmResolver* ptr = T::Instance(false);
    if (NULL == ptr) {
        printf( "instance %s module failed!\n", shm_name );
        return false;
    }
    if (!ptr->DestroyIndex(index_name)) {
        printf("remove %s index failed!\n", index_name);
        return false;
    } else {
        ret = true;
    }
    return ret;
}

template <typename T>
bool Show( const char* shm_name ) {
    IShmResolver* ptr = T::Instance( false );
    bool ret = true;
    if( NULL == ptr ) {
        printf( "instance %s module failed!\n", shm_name );
        ret = false;
    } else {
        ptr->Report();
    }
    return ret;
}

template <typename T>
bool Erase( const char* shm_name ) {
    IShmResolver* ptr = T::Instance( false );
    bool ret = true;
    if( NULL == ptr ) {
        printf( "instance %s module failed!\n", shm_name );
        ret = false;
    } else {
        ret = true;
        ptr->Clear();
        // printf( "Clear the datas of %s module success!\n", shm_name );
    }
    return ret;
}

template <typename T>
bool Load( const char* shm_name ) {
    IShmResolver* ptr = T::Instance( false );
    bool ret = true;
    if( NULL == ptr ) {
        printf( "instance %s module failed!\n", shm_name );
        ret = false;
    } else {
		try {
            ret = ptr->LoadDataFromDB();
		} catch (exception& ex) {
            ret = false;
			printf("%s\n", ex.what());
		}
        if (false == ret) {
            printf( "Load %s module failed!\n", shm_name );
        }
    }
    return ret;
}

template <typename T>
bool WLock(const char* shm_name) {
	bool ret = true;
	IShmResolver* ptr = T::Instance( false );
    if( NULL == ptr ) {
		ret = false;
        printf( "instance %s module failed!\n", shm_name );
    } else {
		ret = ptr->WLock();
    }
	if (ret) {
		printf("write locking.......\npress any key for unlock");
		getchar();
	} else {
		printf("lock %s's write failed\n", shm_name);
	}
	return ret;
}

template <typename T>
bool RLock(const char* shm_name) {
	bool ret = true;
	IShmResolver* ptr = T::Instance( false );
    if( NULL == ptr ) {
		ret = false;
        printf( "instance %s module failed!\n", shm_name );
    } else {
		ret = ptr->RLock();
    }
	if (ret) {
		printf("read locking.......\npress any key for unlock");
		getchar();
	} else {
		printf("lock %s's read failed\n", shm_name);
	}
	return ret;
}

template <typename T>
bool ShowShmInfo(const char* shm_name) {
    IShmResolver* ptr = T::Instance(false);
    bool ret = true;
    if (NULL == ptr) {
        printf("instance %s module failed!\n", shm_name);
        ret = false;
    } else {
        std::vector<int> shm_id;
        ptr->GetShmIds(shm_id);
        printf("%s's shared memory id, total %d:\n", shm_name, shm_id.size());
        for (int i = 0; i < shm_id.size(); i ++) {
            printf("%-10d ", shm_id[i]);
            if (4 == i % 5) {
                putchar('\n');
            }
        }
        if (0 != shm_id.size() % 5) {
            putchar('\n');    
        }
    }
    return ret;
}

template <typename T>
bool LockReset(const char* shm_name) {
	bool ret = true;
	IShmResolver* ptr = T::Instance( false );
    if( NULL == ptr ) {
		ret = false;
        printf( "instance %s module failed!\n", shm_name );
    } else {
		if (ptr->LockReset() < 0) {
			ret = false;
			printf("reset %s's lock failed\n", shm_name);
		}
    }
	return ret;
}

template <typename T>
bool ImportFromDB(const char* shm_name, const char* table_name) {
	bool ret = true;
	IShmResolver* ptr = T::Instance( false );
    if( NULL == ptr ) {
		ret = false;
        printf( "instance %s module failed!\n", shm_name );
    } else {
		if (ptr->ImportFromDB(theDS, table_name) < 0) {
			printf("import %s module from %s table failed\n", shm_name, table_name);
			ret = false;
		}
    }
	return ret;
}

template <typename T>
bool ExportToDB(const char* shm_name, const char* table_name) {
	bool ret = true;
	IShmResolver* ptr = T::Instance( false );
    if( NULL == ptr ) {
		ret = false;
        printf( "instance %s module failed!\n", shm_name );
    } else {
		if (ptr->ExportToDB(theDS, table_name) < 0) {
			printf("export %s module to %s table failed\n", shm_name, table_name);
			ret = false;
		}
    }
	return ret;
}

template <typename T>
bool ImportFromFile(const char* shm_name, const char* filename) {
	bool ret = true;
	IShmResolver* ptr = T::Instance( false );
    if( NULL == ptr ) {
		ret = false;
        printf( "instance %s module failed!\n", shm_name );
    } else {
		if (ptr->ImportFromFile(filename) < 0) {
			ret = false;
			printf("import %s module from %s file failed\n", shm_name, filename);
		}
    }
	return ret;
}

template <typename T>
bool ExportToFile(const char* shm_name, const char* filename) {
	bool ret = true;
	IShmResolver* ptr = T::Instance( false );
    if( NULL == ptr ) {
		ret = false;
        printf( "instance %s module failed!\n", shm_name );
    } else {
		if (ptr->ExportToFile(filename) < 0) {
			printf("export %s module to %s file failed\n", shm_name, filename);
			ret = false;
		}
    }
	return ret;
}

template <typename T>
bool Action( const char* shm_name, int action, const ADDITION& addition) {
    bool ret = true;
    switch (action) {
    case 0:
        ret = Create<T>( shm_name );
        break;
    case 1:
        ret = Destroy<T>( shm_name );
        break;
    case 2:
        ret = Show<T>( shm_name );
        break;
    case 3:
        ret = Erase<T>( shm_name );
        break;
    case 4:
        ret = Load<T> (shm_name);
        break;
	case 5:
		ret = WLock<T>(shm_name);
		break;
	case 6:
		ret = RLock<T>(shm_name);
		break;
	case 7:
		ret = LockReset<T>(shm_name);
		break;
    case 8:
        ret = ShowShmInfo<T>(shm_name);
        break;
    case 9:
        ret = DestroyIndex<T>(shm_name, index_names[addition.id]);
        break;
	case 10:
		ret = ImportFromDB<T>(shm_name, addition.ptr);
		break;
	case 11:
		ret = ExportToDB<T>(shm_name, addition.ptr);
		break;
	case 12:
		ret = ImportFromFile<T>(shm_name, addition.ptr);
		break;
	case 13:
		ret = ExportToFile<T>(shm_name, addition.ptr);
		break;
    case 14:
        ret = DestroyIndex<T>(shm_name, index_names[addition.id]);
        break;
    default:
        printf("invalid action\n");
        ret = false;
        break;
    }
    return ret;
}

bool BeginAction( int module_id, int action_id, const ADDITION& addition) {
    bool ret = true;
    switch( module_id ) {
        case 0:
            ret = Action<CShmAcctItemAggrMgr>( module_names[module_id], action_id, addition );
            break;

        case 1:
            ret = Action<CShmAcctItemOweMgr>( module_names[module_id], action_id, addition );
            break;

        case 2:
            ret = Action<CShmAcctItemTypeMgr>( module_names[module_id], action_id, addition );
            break;

        case 3:
            ret = Action<CShmAddressPartyRelMgr>( module_names[module_id], action_id, addition );
            break;

        case 4:
            ret = Action<CShmCalcResultDetailMgr>( module_names[module_id], action_id, addition );
            break;

        case 5:
            ret = Action<CShmChannelMgr>( module_names[module_id], action_id, addition );
            break;

        case 6:
            ret = Action<CShmChannelEquipRelMgr>( module_names[module_id], action_id, addition );
            break;

        case 7:
            ret = Action<CShmChannelMemberMgr>( module_names[module_id], action_id, addition );
            break;

        case 8:
            ret = Action<CShmGridMgr>( module_names[module_id], action_id, addition );
            break;

        case 9:
            ret = Action<CShmPartnerMgr>( module_names[module_id], action_id, addition );
            break;

        case 10:
            ret = Action<CShmPartnerOfferInstanceMgr>( module_names[module_id], action_id, addition );
            break;

        case 11:
            ret = Action<CShmPaymentMgr>( module_names[module_id], action_id, addition );
            break;

        case 12:
            ret = Action<CShmProductOfferMgr>( module_names[module_id], action_id, addition );
            break;

        case 13:
            ret = Action<CShmRatableResourceAccumulatorMgr>( module_names[module_id], action_id, addition );
            break;

        case 14:
            ret = Action<CShmServChannelRelMgr>( module_names[module_id], action_id, addition );
            break;

        case 15:
            ret = Action<CShmServEquipMgr>( module_names[module_id], action_id, addition );
            break;

        case 16:
            ret = Action<CShmServGridRelMgr>( module_names[module_id], action_id, addition );
            break;

        case 17:
            ret = Action<CShmServLocationMgr>( module_names[module_id], action_id, addition );
            break;

        case 18:
            ret = Action<CShmServTerminalMgr>( module_names[module_id], action_id, addition );
            break;

        case 19:
            ret = Action<CShmSettleItemDetailMgr>( module_names[module_id], action_id, addition );
            break;

        case 20:
            ret = Action<CShmSimCardActiveMgr>( module_names[module_id], action_id, addition );
            break;

        case 21:
            ret = Action<CShmSimCardPartyRelMgr>( module_names[module_id], action_id, addition );
            break;

        case 22:
            ret = Action<CShmStampInfoMgr>( module_names[module_id], action_id, addition );
            break;

        case 23:
            ret = Action<CShmTerminalRegistMgr>( module_names[module_id], action_id, addition );
            break;

        case 24:
            ret = Action<CShmTifFeeBillMgr>( module_names[module_id], action_id, addition );
            break;
            
        case 25:
            ret = Action<WindleSharedMemory>( module_names[module_id], action_id, addition );
            break;
            
        case 26:
            ret = Action<CumulantSharedMemory> (module_names[module_id], action_id, addition);
            break;
            
        case 27:
            ret = Action<tpss::CShmInfoMgr>(module_names[module_id], action_id, addition);
            break;
            
        case 28:
            ret = Action<CShmOfferMgr>(module_names[module_id], action_id, addition);
            break;
            
        default:
            ret = false;
            printf( "%d module not existed \n", module_id );
            break;
    }
    return ret;
}

void ShowHelp() {
	int len = 0;
    printf( "Please input command as below: \n" );
	len = sizeof( description ) / sizeof( char* );
	for(int i = 0 ; i< len; i ++) {
		printf("%s\n", description[i]);
	}
}

void ShowModule() {
    int nLine = 4;
    int len = sizeof( module_names ) / sizeof( char* );
    printf("the modules we surport are below:\n");
    for(int i = 0; i < len; i ++) {
        printf("%-30s ", module_names[i]);
        if (nLine - 1 == i % nLine) {
            putchar('\n');
        }
    }
    if (len % nLine) {
        putchar('\n');
    }
}

void ShowIndex() {
    printf("%45s THE INDEX INFOS\n", "");
    printf("%-50s %-30s %-30s\n", "INDEX", "MODULE", "FIELD");
    long len = sizeof(index_names) / sizeof(char*);
    for(int i = 0; i < len; i ++) {
        printf("%-50s %-30s %-30s\n", index_names[i], index_module_names[i], index_field_names[i]);
    }
}

int GetModuleId(const char* module_name) {
	int len = sizeof( module_names ) / sizeof( char* );
	int module_id = 0;
	for(module_id = 0; module_id < len; module_id ++ ) {
		if( 0 == strcmp( module_name, module_names[module_id] ) ) {
			break;
		}
	}
	if( module_id == len ) {
		module_id = -1;  
	}
	return module_id;
}

int GetIndexId(const char* index_name) {
    int len = sizeof(index_names) / sizeof(char*);
    int index_id = 0;
    for(index_id = 0; index_id < len; ++ index_id) {
        if (0 == strcmp(index_name, index_names[index_id])) {
            break;
        }
    }
    if (len == index_id) {
        index_id = - 1;
    }
    return index_id;
}

int GetActionId(const char* action_name) {
    int len = sizeof(action_names) / sizeof(char*);
    int i = 0;
    for (i = 0; i < len; i ++) {
        if (0 == strcmp(action_names[i], action_name)) {
            break;
        }
    }
    if (i == len) {
        i = -1;
    }
    return i;
}

/*
 * Name         : main
 *
 * Synopsis     : int main( int argc, char** argv ) *
 * Arguments    : int  argc : 命令、参数的个数
 *                char** argv : -c -rm -s -l -e -w -r -u -id -ed -if -ef
 *
 * Description  : psbshm 命令总集
 * 
 * Returns      : int 0-表示成功 -1-表示失败
 */

int main( int argc, char** argv ) {
    int ret = 0;
    try {
		int len = 0;
		int module_id = 0, action_id = 0;
        char module_name[30];
        ADDITION addition;
        if (argc < 2) {
            ShowHelp();
            return 1;
        }
        
        // -h -m -is
        if (argc == 2) {
            if (strcmp(argv[1], "-h") == 0) {
                ShowHelp();
                return 0;
            } else if (strcmp(argv[1], "-m") == 0) {
                ShowModule();
                return 0;
            } else if (strcmp(argv[1], "-i") == 0) {
                ShowIndex();
                return 0;
            }
            else {
                ShowHelp();
                return 1;
            }
        }
        
        // -d 模式
        int act[2];
        bool is_debug = false;
        act[0] = GetActionId(argv[1]);
        act[1] = GetActionId(argv[2]);
        //判断是否是-d(调试)模式
        if (0 == strcmp("-d", argv[1]) && act[1] <= 13 && act[1] >= 0) {
            is_debug = true;   
        }
        initializeLog(argc, argv, is_debug);
        //确定action_id, module_id,以及addition
        if (is_debug) {
            
            if (act[1] >= 0 && act[1] <= 9 && 4 == argc) {
            //psbshm -d ["-c", "-d", "-s", "-e", "-l", "-w", "-r", "-u"] acct_item_aggr
                action_id = act[1];
                //-ri
                if (act[1] < 9 ) {
                    module_id = GetModuleId(argv[3]);
                    strcpy(module_name, argv[3]);
                    addition.ptr = argv[3];
                } else {
                    int index_id = GetIndexId(argv[3]);
                    if (-1 == index_id) {
                        addition.id = index_id;
                        module_id = -1;
                    } else {
                        module_id = GetModuleId(index_module_names[index_id]);
                        strcpy(module_name, index_module_names[index_id]);
                        addition.id = index_id;
                    }
                }
            } else if (act[1] > 9 && act[1] <= 13 && 5 == argc) {
            //psbshm -d ["id", "ed", "if", "ef"] acct_item_aggr_db_or_file acct_item_aggr
                action_id = act[1];
                module_id = GetModuleId(argv[4]);
                strcpy(module_name, argv[4]);
                addition.ptr = argv[3];
            } else {
                ShowHelp();
                return 1;
            }
        } else {
            if (act[0] <= 9 && act[0] >= 0 && 3 == argc) {
                //psbshm ["-c", "-d", "-s", "-e", "-l", "-w", "-r", "-u"] acct_item_aggr
                action_id = act[0];
                if (act[0] < 9) {
                    strcpy(module_name, argv[2]);
                    module_id = GetModuleId(argv[2]);
                    addition.ptr = argv[2];
                } else {
                    int index_id = GetIndexId(argv[2]);
                    if (-1 == index_id) {
                        addition.id = index_id;
                        module_id = -1;
                    } else {
                        module_id = GetModuleId(index_module_names[index_id]);
                        strcpy(module_name, index_module_names[index_id]);
                        addition.id = index_id;
                    }
                }
            } else if (act[0] > 9 && act[0] <= 13 && 4 == argc) {
                //psbshm ["id", "ed", "if", "ef"] acct_item_aggr_db_or_file acct_item_aggr
                action_id = act[0];
                strcpy(module_name, argv[3]);
                module_id = GetModuleId(argv[3]);
                addition.ptr = argv[2];
            } else {
                ShowHelp();
                return 1;
            }
        }
        
        len = sizeof(module_names) / sizeof(char*);
        if ( 0 == strcmp("All", module_name) && action_id >= 0 && action_id < 9) {
            for(int i = 0; i < len; i ++) {
                if (!BeginAction(i, action_id, addition)) {
                    ret = -1;
                }
            }
        } else if (module_id >= 0 && module_id < len && action_id >= 0 && action_id <= 13){
            if (!BeginAction(module_id, action_id, addition)) {
                ret = -1;
            }
        } else {
            ret = -1;
            if (9 == action_id) {
                ShowIndex();
            } else {
                ShowModule();
            }
        }
        if (0 == ret) {
            printf("All is well\n");
        }
		
    } catch (exception& ex){
        ret = -1;
        printf("%s\n", ex.what());
    }
    return ret;
}

