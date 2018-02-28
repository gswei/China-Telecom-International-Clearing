/******************************************************************
	Copyright (c) 2012, GUANGDONG YOSON TECHNOLOGY CO., LTD
	All rights reserved.

	Created:		2012/9/22
	Filename: 		busi_shm_struct.h
	Description:	汇总所有业务共享内存的业务结构体。仅供busi_shm_mgr.h使用。
					!!!注意：若各业务内存结构体变更，需保持一致!!!

	History:
	<table>
		Revision	Author			Date		Description
		--------	------			----		-----------
		v1.0		raozh		 2012/9/22	    完成所有编码
	</table>
*******************************************************************/

#ifndef BUSI_SHM_STRUCT_H_
#define BUSI_SHM_STRUCT_H_

#include "shm_Set.h"

namespace tpss
{

typedef ns_shm_data::T_SHMSET_NO_MUTEX < long, -1 >::iterator VirtualSetIter;	//虚拟Set迭代器（起占位用）

//以下所有结构体都是从各业务共享内存类抽取
//

//
struct AcctItemAggrItem {
    Int64 	acct_item_id;
    Int64 	serv_id;
    int	  	billing_cycle_id;
    int   	acct_item_type_id;
    Int64 	charge;
    Int64 	acct_id;
    Int64	offer_inst_id;
    int		item_source_id;
    int		offer_id;
    Int64	create_date;

    AcctItemAggrItem() {
        Clear();
    }
    bool operator < ( const AcctItemAggrItem& rhs )const {
        return serv_id < rhs.serv_id ||
               ( serv_id == rhs.serv_id && acct_item_id < rhs.acct_item_id );
    }
    static bool comp_serv_id( const AcctItemAggrItem& a, const AcctItemAggrItem& b ) {
        return a.serv_id < b.serv_id;
    }

    std::string& toString( std::string& str )const {
        return str;
    }

    void Clear() {
        acct_item_id = 0;
        serv_id = 0;
        billing_cycle_id = 0;
        acct_item_type_id = 0;
        charge = 0;
        acct_id = 0;
        offer_inst_id = 0;
        item_source_id = 0;
        offer_id = 0;
        create_date = 0;
    }

    bool FitCondition( const std::string& condition, std::string& err_msg ) {
        bool ret = true;
        StringTokenizer token( condition, "," );

        for( int i = 0; i < token.size(); i++ ) {
            StringTokenizer token2( token[i], "=" );
            std::string column_name = token2[0];
            std::string value = token2[1];

            //
            if( column_name == "acct_item_id" ) {
                ret = ret && ( acct_item_id == StringUtil::toInt64( value ) );
            } else if( column_name == "serv_id" ) {
                ret = ret && ( serv_id == StringUtil::toInt64( value ) );
            } else if( column_name == "billing_cycle_id" ) {
                ret = ret && ( billing_cycle_id == StringUtil::toInt( value ) );
            } else if( column_name == "acct_item_type_id" ) {
                ret = ret && ( acct_item_type_id == StringUtil::toInt( value ) );
            } else if( column_name == "charge" ) {
                ret = ret && ( charge == StringUtil::toInt64( value ) );
            } else if( column_name == "acct_id" ) {
                ret = ret && ( acct_id == StringUtil::toInt64( value ) );
            } else if( column_name == "offer_inst_id" ) {
                ret = ret && ( offer_inst_id == StringUtil::toInt64( value ) );
            } else if( column_name == "item_source_id" ) {
                ret = ret && ( item_source_id == StringUtil::toInt( value ) );
            } else if( column_name == "offer_id" ) {
                ret = ret && ( offer_id == StringUtil::toInt( value ) );
            } else if( column_name == "create_date" ) {
                ret = ret && ( create_date == StringUtil::toInt64( value ) );
            } else {
                ret = false;
                err_msg = "查询条件中存在非法列名";
            }

            if( !ret ) {
                break;
            }
        }

        return ret;
    }

    void toVector( std::vector<std::pair<std::string, std::string> >& vec ) const {
        vec.push_back( std::make_pair( std::string( "acct_item_id" ), StringUtil::toString( acct_item_id ) ) );
        vec.push_back( std::make_pair( std::string( "serv_id" ), StringUtil::toString( serv_id ) ) );
        vec.push_back( std::make_pair( std::string( "billing_cycle_id" ), StringUtil::toString( billing_cycle_id ) ) );
        vec.push_back( std::make_pair( std::string( "acct_item_type_id" ), StringUtil::toString( acct_item_type_id ) ) );
        vec.push_back( std::make_pair( std::string( "charge" ), StringUtil::toString( charge ) ) );
        vec.push_back( std::make_pair( std::string( "acct_id" ), StringUtil::toString( acct_id ) ) );
        vec.push_back( std::make_pair( std::string( "offer_inst_id" ), StringUtil::toString( offer_inst_id ) ) );
        vec.push_back( std::make_pair( std::string( "item_source_id" ), StringUtil::toString( item_source_id ) ) );
        vec.push_back( std::make_pair( std::string( "offer_id" ), StringUtil::toString( offer_id ) ) );
        vec.push_back( std::make_pair( std::string( "create_date" ), StringUtil::toString( create_date ) ) );
    }

    //import相关
    static std::string GetQuerySql( const std::string& table_name ) {
        return "select acct_item_id, serv_id, billing_cycle_id, acct_item_type_id, charge, acct_id,"
               " offer_inst_id, item_source_id, offer_id, create_date"
               " from " + table_name;
    }

    bool ImportFrom( const std::string& str, std::string& err_msg, const std::string& delimiter = "|" ) {
        StringTokenizer token( str, delimiter );

        if( token.size() != 10 ) {
            err_msg = "文件中存在列数不规范的行记录";
            return false;
        } else {
            acct_item_id = StringUtil::toInt64( token[0] );
            serv_id = StringUtil::toInt64( token[1] );
            billing_cycle_id = StringUtil::toInt( token[2] );
            acct_item_type_id = StringUtil::toInt( token[3] );
            charge = StringUtil::toInt64( token[4] );
            acct_id = StringUtil::toInt64( token[5] );
            offer_inst_id = StringUtil::toInt64( token[6] );
            item_source_id = StringUtil::toInt( token[7] );
            offer_id = StringUtil::toInt( token[8] );
            create_date = StringUtil::toInt64( token[9] );
            return true;
        }
    }
    bool ImportFrom( Statement& stmt ) {
        return ( stmt >> acct_item_id
                 >> serv_id
                 >> billing_cycle_id
                 >> acct_item_type_id
                 >> charge
                 >> acct_id
                 >> offer_inst_id
                 >> item_source_id
                 >> offer_id
                 >> create_date );
    }
    //export相关
    static std::string GetCreateTableSql( const std::string& table_name ) {
        return "create table " + table_name + "("
               " ACCT_ITEM_ID      NUMBER(12) not null,"
               " SERV_ID           NUMBER(12),"
               " BILLING_CYCLE_ID  NUMBER(9),"
               " ACCT_ITEM_TYPE_ID NUMBER(9),"
               " CHARGE            NUMBER(12),"
               " ACCT_ID           NUMBER(12),"
               " OFFER_INST_ID     NUMBER(12),"
               " ITEM_SOURCE_ID    NUMBER(9),"
               " OFFER_ID          NUMBER(9),"
               " CREATE_DATE       NUMBER(14)"
               ")";
    }
    static std::string GetInsertSql( const std::string& table_name ) {
        return "insert into " + table_name + "("
               "acct_item_id,serv_id,billing_cycle_id,acct_item_type_id,charge,acct_id,"
               "offer_inst_id,item_source_id,offer_id,create_date)"
               " values(:1, :2, :3, :4, :5, :6, :7, :8, :9, :10)";
    }

    static void ExportHeader( std::string& header, const std::string& delimiter = "|" ) {
        header = "acct_item_id"
                 + delimiter + "serv_id"
                 + delimiter + "billing_cycle_id"
                 + delimiter + "acct_item_type_id"
                 + delimiter + "charge|acct_id"
                 + delimiter + "offer_inst_id"
                 + delimiter + "item_source_id"
                 + delimiter + "offer_id"
                 + delimiter + "create_date";
    }

    void ExportTo( std::string& str, const std::string& delimiter = "|" ) {
        str = StringUtil::toString( acct_item_id )
              + delimiter + StringUtil::toString( serv_id )
              + delimiter + StringUtil::toString( billing_cycle_id )
              + delimiter + StringUtil::toString( acct_item_type_id )
              + delimiter + StringUtil::toString( charge )
              + delimiter + StringUtil::toString( acct_id )
              + delimiter + StringUtil::toString( offer_inst_id )
              + delimiter + StringUtil::toString( item_source_id )
              + delimiter + StringUtil::toString( offer_id )
              + delimiter + StringUtil::toString( create_date );
    }

    void ExportTo( Statement& stmt ) {
        stmt << acct_item_id
             << serv_id
             << billing_cycle_id
             << acct_item_type_id
             << charge << acct_id
             << offer_inst_id
             << item_source_id
             << offer_id
             << create_date;
    };
};

//
struct IndexOfferInstIdOfAcctItemAggr {
    int offer_inst_id;
    VirtualSetIter iter;
};
struct AcctItemOweItem {
public:
    Int64 acct_item_id;
    Int64 serv_id;
    Int64 acct_id;

    int   item_source_id;
    int   billing_cycle_id;
    int   acct_item_type_id;
    Int64 amount;
    Int64 bill_id;
    int   payment_method;
    int   offer_id;
    int   fee_cycle_id;
    char  state[3];
    bool operator < ( const AcctItemOweItem& rhs )const {
        return serv_id < rhs.serv_id ||
               ( serv_id == rhs.serv_id && offer_id < rhs.offer_id );
    }
    std::string& toString( std::string& str )const {
        return str = string( state );
    }
    static bool comp_serv_id( const AcctItemOweItem& a, const AcctItemOweItem& b ) {
        return a.serv_id < b.serv_id;
    }
};

struct IndexOfferIdOfAcctItemOwe {
    int offer_id;
    VirtualSetIter iter;
};

struct AcctItemTypeItem {
    bool operator < ( const AcctItemTypeItem& rhs )const {
        return acct_item_type_id < rhs.acct_item_type_id;
    }
    string& toString( string& str )const {
        return str;
    }
    static bool comp_acct_item_type_id( const AcctItemTypeItem& a, const AcctItemTypeItem& b ) {
        return a.acct_item_type_id < b.acct_item_type_id;
    }
    int		acct_item_type_id;
    int		acct_item_class_id;
    char	name[50];
    char	charge_mark[3];
    char	total_mark[3];
    char	acct_item_type_code[15];
};

struct AddressPartyRelItem {
    Int64 addr_party_rel_id;
    Int64 address_id;
    int	  party_id;
    int   channel_id;
    Int64 eff_date;
    Int64 exp_date;

    bool operator < ( const AddressPartyRelItem& rhs )const {
        return address_id < rhs.address_id ||
               ( address_id == rhs.address_id && addr_party_rel_id < rhs.addr_party_rel_id );
    }
    std::string& toString( std::string& str )const {
        return str;
    }
    static bool comp_address_id( const AddressPartyRelItem& a, const AddressPartyRelItem& b ) {
        return a.address_id < b.address_id;
    }
};

struct IndexChannelIdOfAddressPartyRel {
    int channel_id;
    VirtualSetIter iter;
};

struct ChannelMemberItem {
    int channel_id;
    Int64 channel_member_id;
    char channel_member_name[32];
    char member_type_cd[3];
    Int64 eff_date;
    Int64 exp_date;
    bool operator < ( const ChannelMemberItem& rhs )const {
        return channel_member_id < rhs.channel_member_id;
    }
    std::string& toString( std::string& str )const {
        return str;
    }
};

struct ChannelEquipRelItem {
    Int64 	equip_channel_rel_id;
    char 	optical_cable_code[50];
    char	trunk_line_code[50];
    char   	sub_line_box_code[50];
    char 	junction_box_code[50];
    int		channel_id;
    int		party_id;
    bool operator < ( const ChannelEquipRelItem& rhs )const {
        return channel_id < rhs.channel_id ||
               ( channel_id < rhs.channel_id && equip_channel_rel_id < rhs.equip_channel_rel_id );
    }
    std::string& toString( std::string& str )const {
        return str;
    }
    static bool comp_channel_id( const ChannelEquipRelItem& a, const ChannelEquipRelItem& b ) {
        return a.channel_id < b.channel_id;
    }
};

struct IndexSubLineBoxCodeOfChannelEquipRel {
    char sub_line_box_code[50];
    VirtualSetIter iter;
};

struct IndexJunctionBoxCodeOfChannelEquipRel {
    char junction_box_code[50];
    VirtualSetIter iter;
};

struct ChannelItem {
    int 	channel_id;
    char 	channel_name[50];
    char	channel_level_cd[3];
    char   	channel_type_cd[6];
    char	channel_subtype_cd[10];
    int 	status;
    int		parent_chn_id;
    char	channel_nbr[30];
    int		party_id;
    int		lant_id;
    Int64 eff_date;
    Int64 exp_date;
    bool operator < ( const ChannelItem& rhs )const {
        return channel_id < rhs.channel_id;
    }
    std::string& toString( std::string& str )const {
        return str;
    }
    static bool comp_channel_id( const ChannelItem& a, const ChannelItem& b ) {
        return a.channel_id < b.channel_id;
    }
};

struct IndexPartyIdOfChannel {
    int party_id;
    VirtualSetIter iter;
};

struct CalcResultDetailItem {
    Int64 	service_control_id;
    Int64 	party_offer_inst_id;
    Int64 	sales_id;
    Int64	staff_id;
    Int64 	serv_id;
    char  	acc_nbr[32];
    int   	product_id;
    short int 	cost_flag;
    short int 	adjust_flag;
    short int 	settle_obj_type;
    Int64     	settle_obj_id;
    int       	acct_item_type_id;
    Int64	charge;
    int		channel_id;
    int		party_id;
    int		billing_cycle_id;
    int		offer_id;
    Int64	created_date;
    short int 	state;
    Int64	state_date;
    int		strategy_id;
    int		lant_id;
    char	element_str[2000];
    short int 	is_settle_item; //是否为结算清单1是0否
    std::string& toString( std::string& str ) const {
        return str;
    }
    bool operator < ( const CalcResultDetailItem& item ) const {
        return service_control_id < item.service_control_id;
    }
};

struct GridItem {
    Int64 grid_id;
    char  grid_code[30];
    char  grid_name[250];
    char  grid_type[3];
    char  grid_range[2000];
    char  status_cd[3];
    GridItem() : grid_id( 0 ), grid_code( "0" ), grid_name( "0" ), grid_type( "0" ),
        grid_range( "0" ), status_cd( "0" ) {
    }
    void Clear() {
        grid_id = 0;
        grid_code[0] = '\0';
        grid_name[0] = '\0';
        grid_type[0] = '\0';
        grid_range[0] = '\0';
        status_cd[0] = '\0';
    }
    bool operator < ( const GridItem& rhs )const {
        return grid_id < rhs.grid_id;
    }
    string& toString( string& str )const {
        return str;
    }
    static bool comp_grid_id( const GridItem& a, const GridItem& b ) {
        return a.grid_id < b.grid_id;
    }
};

struct ProductOfferItem {
    int offer_id;
    int band_id;
    int pricing_plan_id;
    int integral_pricing_plan_id;
    char offer_name[50];
    char offer_comments[250];
    char can_be_buy_alone;
    char offer_code[15];
    short int priority;
    char state[3];
    int  offer_type;
    Int64 base_fee;
    Int64 floor_fee;
    char  merge_flag;
    Int64 eff_date;
    Int64 exp_date;
    bool operator < ( const ProductOfferItem& rhs )const {
        if( offer_id < rhs.offer_id ) {
            return true;
        }

        return false;
    }
    std::string& toString( std::string& str )const {
        return str;
    }
    static bool comp_offer_id( const ProductOfferItem& a, const ProductOfferItem& b ) {
        return a.offer_id < b.offer_id;
    }
};

struct PaymentItem {
    Int64 	payment_id;
    Int64 	acct_id;
    int   	payment_method;
    int   	staff_id;
    int   	payed_method;
    char  	operation_type[3];
    char  	amount[3];
    Int64  	payment_date;
    char  	state[3];
    Int64  	serv_id;
    char  	acc_nbr[32];
    int   	pay_cycle_id;

    bool operator < ( const PaymentItem& rhs )const {
        return serv_id < rhs.serv_id ||
               ( serv_id == rhs.serv_id && payment_id < rhs.payment_id );
    }
    string& toString( string& str )const {
        return str;
    }
    static bool comp_serv_id( const PaymentItem& a, const PaymentItem& b ) {
        return a.serv_id < b.serv_id;
    }
};

struct PartnerOfferInstanceItem {
    Int64 	party_offer_inst_id;
    int   	obj_type;
    int   	obj_id;
    int   	offer_id;
    Int64 	eff_date;
    Int64 	exp_date;

    bool operator < ( const PartnerOfferInstanceItem& rhs )const {
        return obj_id < rhs.obj_id ||
               ( obj_id == rhs.obj_id && obj_type < rhs.obj_type ) ||
               ( obj_id == rhs.obj_id && obj_type == rhs.obj_type && party_offer_inst_id < rhs.party_offer_inst_id );
    }

    std::string& toString( std::string& str )const {
        return str;
    }

    static bool comp_obj_type_id( const PartnerOfferInstanceItem& a, const PartnerOfferInstanceItem& b ) {
        return a.obj_id < b.obj_id ||
               ( a.obj_id == b.obj_id && a.obj_type < b.obj_type );
    }

};

struct PartnerItem {
    int party_id;
    char pard_code[15];
    char pard_type[10];
    char pard_name[50];
    char pard_desc[250];
    int  lant_id;
    char contact_person[20];
    char contact_phone[40];
    char shopcard_num[30];
    char corporation_name[32];
    char corporation_id_no[30];
    char corporation_phone[30];
    char address_info[250];
    short int state;
    Int64 eff_date;
    Int64 exp_date;

    bool operator < ( const PartnerItem& rhs )const {
        return party_id < rhs.party_id;
    }
    string& toString( string& str )const {
        return str;
    }
    static bool comp_party_id( const PartnerItem& a, const PartnerItem& b ) {
        return a.party_id < b.party_id;
    }
};

struct RatableResourceAccumulatorItem {
    int 	ratable_resource_id;
    char 	owner_type[3];
    Int64	owner_id;
    int		ratable_cycle_id;
    int		balance;
    bool operator < ( const RatableResourceAccumulatorItem& rhs )const {
        return ratable_resource_id < rhs.ratable_resource_id ||
               ( ratable_resource_id == rhs.ratable_resource_id && owner_id < rhs.owner_id ) ||
               ( ratable_resource_id == rhs.ratable_resource_id && owner_id == rhs.owner_id && strcmp( owner_type,  rhs.owner_type ) == -1 );
    }
    std::string& toString( std::string& str )const {
        return str;
    }

    static bool comp_ratable_source_owner_id( const RatableResourceAccumulatorItem& a, const RatableResourceAccumulatorItem& b ) {
        return a.ratable_resource_id < b.ratable_resource_id ||
               ( a.ratable_resource_id == b.ratable_resource_id && a.owner_id < b.owner_id ) ;
    }
};

struct ServTerminalItem {
    Int64 serv_id;
    char  terminal_mode[60];
    char  brand[60];
    short int intelligence;
    char phone_strcode[30];
    Int64 create_date;
    bool operator < ( const ServTerminalItem& rhs )const {
        return serv_id < rhs.serv_id;
    }
    string& toString( string& str )const {
        printf( "serv_id = %d terminal_mode = %d\n", serv_id, terminal_mode );
        return str;
    }
    static bool comp_serv_id( const ServTerminalItem& a, const ServTerminalItem& b ) {
        return a.serv_id < b.serv_id;
    }
};

struct SimCardActiveItem {
    Int64 	active_id;
    Int64 	serv_id;
    char   	acc_nbr[32];
    Int64 	active_date;
    int		latn_id;
    short int state;
    Int64	state_date;
    bool operator < ( const SimCardActiveItem& rhs )const {
        return serv_id < rhs.serv_id ||
               ( serv_id == rhs.serv_id && active_id < rhs.active_id );
    }
    std::string& toString( std::string& str )const {
        return str;
    }

    static bool comp_serv_id( const SimCardActiveItem& a, const SimCardActiveItem& b ) {
        return a.serv_id < b.serv_id;
    }
};

struct IndexAccNbrOfSimCardActive {
    char acc_nbr[32];
    VirtualSetIter iter;
};

struct StampInfoItem {
    bool operator < ( const StampInfoItem& rhs )const {
        return strcmp( stamp_code, rhs.stamp_code ) == -1;
    }
    string& toString( string& str )const {
        return str;
    }
    static bool comp_stamp_code( const StampInfoItem& a, const StampInfoItem& b ) {
        return strcmp( a.stamp_code, b.stamp_code ) == -1;
    }
    char  	stamp_code[50];
    int   	obj_type;
    Int64 	obj_id;
    int   	stamp_type;
    Int64  	amount;
    int    	latn_id;
    Int64   	create_date;
    short int 	settler_mode;
};

struct SimCardPartyRelItem {
    char 	card_seq[30];
    char 	acc_nbr[30];
    int  	card_type;
    char 	card_code[20];
    int  	card_value;
    int     owner_channel_id;
    int  	owner_acc_nbr;
    Int64 	add_date;
    int  	sales_acc_nbr;
    int     sales_channel_id;
    Int64 	sales_time;
    int		latn_id;
    int		party_id;
    Int64	send_date;
    bool operator < ( const SimCardPartyRelItem& rhs )const {
        return strcmp( card_seq, rhs.card_seq ) < 0;
    }
    std::string& toString( std::string& str )const {
        return str;
    }
    static bool comp_card_seq( const SimCardPartyRelItem& a, const SimCardPartyRelItem& b ) {
        return strcmp( a.card_seq, b.card_seq ) == -1;
    }
};

struct IndexAccNbrOfSimCardPartyRel {
    char acc_nbr[30];
    VirtualSetIter iter;
};

struct SettleItemDetailItem {
    Int64       settle_detail_id;
    short int 	settle_obj_type;
    Int64	    settle_obj_id;
    Int64      	charge;
    int	       	billing_cycle_id;
    int        	strategy_id;
    int        	event_type_id;
    bool operator < ( const SettleItemDetailItem& rhs )const {
        return settle_obj_id < rhs.settle_obj_id ||
               ( settle_obj_id == rhs.settle_obj_id && settle_obj_type < rhs.settle_obj_type ) ||
               ( settle_obj_id == rhs.settle_obj_id && settle_obj_type == rhs.settle_obj_type && settle_detail_id < rhs.settle_detail_id );

    }
    string& toString( string& str )const {
        return str;
    }
    static bool comp_settle_obj_type_id( const SettleItemDetailItem& a, const SettleItemDetailItem& b ) {
        return a.settle_obj_id < b.settle_obj_id ||
               ( a.settle_obj_id == b.settle_obj_id && a.settle_obj_type < b.settle_obj_type );
    }
};

struct ServLocationItem {
    Int64 serv_id;
    Int64 agreement_id;
    Int64 address_id;
    int   bureau_id;
    int   exchange_id;
    int   stat_region_id;

    bool operator < ( const ServLocationItem& rhs )const {
        return serv_id < rhs.serv_id;
    }
    std::string& toString( string& str )const {
        return str;
    }
    static bool comp_serv_id( const ServLocationItem& a, const ServLocationItem& b ) {
        return a.serv_id < b.serv_id;
    }
};

struct ServGridRelItem {
    Int64 	serv_grid_rel_id;
    Int64 	grid_id;
    Int64	serv_id;
    Int64   eff_date;
    Int64 	exp_date;
    ServGridRelItem() : serv_grid_rel_id( 0 ), grid_id( 0 ), serv_id( 0 ),
        eff_date( 0 ), exp_date( 0 ) {
    }
    void Clear() {
        memset( this, 0, sizeof( ServGridRelItem ) );
    }
    bool operator < ( const ServGridRelItem& rhs )const {
        return grid_id < rhs.grid_id || ( grid_id == rhs.grid_id && serv_grid_rel_id < rhs.serv_grid_rel_id );
    }
    static bool comp_grid_id( const ServGridRelItem& a, const ServGridRelItem& b ) {
        return a.grid_id < b.grid_id;
    }
    std::string& toString( std::string& str )const {
        printf( "grid_id = %lld, serv_id = %lld\n", grid_id, serv_id );
        return str;
    }
};

struct IndexServIdOfServGridRel {
    int serv_id;
    VirtualSetIter iter;
};

struct ServEquipItem {
    Int64 serv_id;
    char  trunk_line_code[50];
    char  sub_line_box_code[50];
    char  junction_box_code[50];
    bool operator < ( const ServEquipItem& rhs )const {
        return serv_id < rhs.serv_id;
    }
    std::string& toString( std::string& str )const {
        return str;
    }
    static bool comp_serv_id( const ServEquipItem& a, const ServEquipItem& b ) {
        return a. serv_id < b.serv_id;
    }
};

struct ServChannelRelItem {
    Int64 	serv_channel_rel_id;
    Int64 	serv_id;
    int   	channel_id;
    int		party_id;
    Int64 	create_date;

    bool operator < ( const ServChannelRelItem& rhs )const {
        return serv_id < rhs.serv_id ||
               ( serv_id == rhs.serv_id && serv_channel_rel_id < rhs.serv_channel_rel_id );

    }
    std::string& toString( std::string& str )const {
        return str;
    }
    static bool comp_serv_id( const ServChannelRelItem& a, const ServChannelRelItem& b ) {
        return a.serv_id < b.serv_id;
    };
};

struct IndexChannelIdOfServChannelRel {
    int channel_id;
    VirtualSetIter iter;
};

struct TerminalRegistItem {
    Int64        regist_id;             //注册标识
    Int64        serv_id;               //用户标识
    char         acc_nbr[32];           //用户号码
    char         terminal_mode[60];     //终端型号
    char         brand[60];             //终端品牌
    short        intelligence;          //智能机标志
    char         phone_strcode[30];     //手机串号
    Int64        regist_date;           //注册时间

    TerminalRegistItem() {
        Clear();
    };

    void Clear() {
        regist_id = 0;
        serv_id = 0;
        intelligence = 0;
        regist_date = 0;
        acc_nbr[0] = '\0';
        terminal_mode[0] = '\0';
        brand[0] = '\0';
        phone_strcode[0] = '\0';
    }

    bool operator < ( const TerminalRegistItem& rhs )const {
        return serv_id < rhs.serv_id
               || ( serv_id == rhs.serv_id && regist_id < rhs.regist_id );
    }

    static bool comp_serv_id( const TerminalRegistItem& a, const TerminalRegistItem& b ) {
        return a.serv_id < b.serv_id;
    };

    std::string& toString( std::string& str )const {
        return str;
    }
};

}

#endif //BUSI_SHM_STRUCT_H_