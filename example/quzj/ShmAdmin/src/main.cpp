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

using namespace tpss;

//只支持一个参数
//这个参数可以有以下类型
//-s AcctItemAggr  显示(show)
//-c AcctItemAggr  创建(create)
//-l AcctItemAggr  加载(load)
//-e AcctItemAggr  清空(empty)
//-d AcctItemAggr  删除(destroy)
//-h 帮助(help)
const char* shm_names[] = {"AcctItemAggr", "AcctItemOwe", "AcctItemType", "AddressPartyRel",
                           "CalcResultDetail", "Channel", "ChannelEquipRel", "ChannelMember",
                           "Grid", "Partner", "PartnerOfferInstance", "Payment",
                           "ProductOffer", "RatableResourceAccumulator", "ServChannelRel", "ServEquip",
                           "ServGridRel", "ServLocation", "ServTerminal", "SettleItemDetail",
                           "SimCardActive", "SimCardPartyRel", "StampInfo", "TerminalRegist", "TifFeeBill"
                          };

const char* action_names[] = {"-c", "-s", "-l", "-h"};


template <typename T>
void Create( const char* shm_name )
{
    T* ptr = T::Instance( true );

    if( NULL == ptr ) {
        printf( "create shared memory for %s module failed\n", shm_name );
    } else {
        printf( "create shared memory for %s module successd!\n", shm_name );
    }
}

template <typename T>
void Show( const char* shm_name )
{
    T* ptr = T::Instance( false );

    if( NULL == ptr ) {
        printf( "instance %s module failed!\n", shm_name );
    } else {
        ptr->Report();
    }
}

template <typename T>
void Load( const char* shm_name )
{
    T* ptr = T::Instance( false );

    if( NULL == ptr ) {
        printf( "instance %s module failed!\n", shm_name );
    } else {
        ptr->LoadDataFromDB();
    }
}

template <typename T>
void Action( const char* shm_name, int action )
{
    if( 0 == action ) {	// 创建
        Create<T>( shm_name );
    } else if( 1 == action ) { //显示
        Show<T>( shm_name );
    } else if( 2 == action ) { //加载数据
        Load<T>( shm_name );
    }
}

void BeginAction( int module_id, int action_id )
{
    switch( module_id ) {
        case 0:
            Action<CShmAcctItemAggrMgr>( shm_names[module_id], action_id );
            break;

        case 1:
            Action<CShmAcctItemOweMgr>( shm_names[module_id], action_id );
            break;

        case 2:
            Action<CShmAcctItemTypeMgr>( shm_names[module_id], action_id );
            break;

        case 3:
            Action<CShmAddressPartyRelMgr>( shm_names[module_id], action_id );
            break;

        case 4:
            if( 3 != action_id ) {
                Action<CShmCalcResultDetailMgr>( shm_names[module_id], action_id );
            } else {
                printf( "CalcResultDetail hasn't LoadDataFromDB operation\n" );
            }

            break;

        case 5:
            Action<CShmChannelMgr>( shm_names[module_id], action_id );
            break;

        case 6:
            Action<CShmChannelEquipRelMgr>( shm_names[module_id], action_id );
            break;

        case 7:
            Action<CShmChannelMemberMgr>( shm_names[module_id], action_id );
            break;

        case 8:
            Action<CShmGridMgr>( shm_names[module_id], action_id );
            break;

        case 9:
            Action<CShmPartnerMgr>( shm_names[module_id], action_id );
            break;

        case 10:
            Action<CShmPartnerOfferInstanceMgr>( shm_names[module_id], action_id );
            break;

        case 11:
            Action<CShmPaymentMgr>( shm_names[module_id], action_id );
            break;

        case 12:
            Action<CShmProductOfferMgr>( shm_names[module_id], action_id );
            break;

        case 13:
            Action<CShmRatableResourceAccumulatorMgr>( shm_names[module_id], action_id );
            break;

        case 14:
            Action<CShmServChannelRelMgr>( shm_names[module_id], action_id );
            break;

        case 15:
            Action<CShmServEquipMgr>( shm_names[module_id], action_id );
            break;

        case 16:
            Action<CShmServGridRelMgr>( shm_names[module_id], action_id );
            break;

        case 17:
            Action<CShmServLocationMgr>( shm_names[module_id], action_id );
            break;

        case 18:
            Action<CShmServTerminalMgr>( shm_names[module_id], action_id );
            break;

        case 19:
            Action<CShmSettleItemDetailMgr>( shm_names[module_id], action_id );
            break;

        case 20:
            Action<CShmSimCardActiveMgr>( shm_names[module_id], action_id );
            break;

        case 21:
            Action<CShmSimCardPartyRelMgr>( shm_names[module_id], action_id );
            break;

        case 22:
            Action<CShmStampInfoMgr>( shm_names[module_id], action_id );
            break;

        case 23:
            Action<CShmTerminalRegistMgr>( shm_names[module_id], action_id );
            break;

        case 24:
            Action<CShmTifFeeBillMgr>( shm_names[module_id], action_id );
            break;

        default:
            printf( "%d module not existed \n", module_id );
            break;
    }
}

void ShowHelp()
{
    printf( "Please input command as below: command -[c,d,s,h] [modulename]\n" );
    printf( "-c   create shared memory\n"
            "-s   show shared memory data\n"
            "-l   load data from db to shared memory\n"
            "-h   the program's help\n" );
    printf( "module names:\n" );
    int len = sizeof( shm_names ) / sizeof( char* );

    for( int i = 0; i < len; i ++ ) {
        printf( "%s\n", shm_names[i] );
    }

}

int main( int argc, char** argv )
{
    if( 2 == argc && strcmp( "-h", argv[1] ) ) {
        ShowHelp();
        return 0;
    } else if( argc != 3 ) {
        ShowHelp();
        return -1;
    } else {
        int len = sizeof( action_names ) / sizeof( char* );
        int action_id = 0;

        //action_id
        for( action_id = 0; action_id < len; action_id ++ ) {
            if( 0 == strcmp( argv[1], action_names[action_id] ) ) {
                break;
            }
        }

        if( action_id == len ) {
            printf( "%s action not existed\n", argv[1] );
            return -1;
        }

        len = sizeof( shm_names ) / sizeof( char* );
        int module_id = 0;

        for( module_id = 0; module_id < len; module_id ++ ) {
            if( 0 == strcmp( argv[2], shm_names[module_id] ) ) {
                break;
            }
        }

        if( module_id == len ) {
            printf( "%s module not existed\n", argv[2] );
            return -1;
        }

        BeginAction( module_id, action_id );
    }

    return 0;
}