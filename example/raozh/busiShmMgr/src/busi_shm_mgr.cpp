/******************************************************************
	Copyright (c) 2012, GUANGDONG YOSON TECHNOLOGY CO., LTD
	All rights reserved.

	Created:		2012/9/24
	Filename: 		busi_shm_mgr.cpp
	Description:	ҵ�����ڴ������ʵ�֡�
	                �ṩ��Ʒ��psshm�������ҵ�����ݽӿڣ�
					�ṩҵ�����ڴ�Ĺ���ӿڣ����������١��������ݡ�������ݵȣ���

	History:
	<table>
		Revision	Author			Date		Description
		--------	------			----		-----------
		v1.0		raozh		 2012/9/24	    ��ʼ����
	</table>
*******************************************************************/

#include "busi_shm_mgr.h"
#include "es/util/Path.h"

using namespace tpss;

//ҵ�����ڴ���ʼ���(˽�н��̿ռ��ڲ�ʹ��)
const int PI_BUSISHM_FIRST = 1;

bool CBusiShmMgr::bInstanced = false;

CBusiShmMgr::~CBusiShmMgr()
{
    for( T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.begin(); it != map_table_obj_.end(); it++ ) {
        delete it->second;
    }
}


CBusiShmMgr* CBusiShmMgr::Instance()
{
    CBusiShmMgr* pShm = es::Singleton<CBusiShmMgr>::instance();

    if( pShm != NULL && !bInstanced ) {
        if( !InitActiveApp() ) {
            return NULL;
        }

        if( !pShm->Init() ) {
            return NULL;
        }
    }

    return pShm;
}

bool CBusiShmMgr::Init()
{
    //

    ns_shm_data::IShmActiveObject* p_shm_obj( 0 );
    p_shm_obj = new ns_shm_data::T_SHMSET_NO_MUTEX_EXT<AcctItemAggrItem, PI_BUSISHM_FIRST>( "AcctItemAggr", 0 );

    if( p_shm_obj->Attach( false ) ) {
        map_table_obj_.insert( std::make_pair( std::string( "AcctItemAggr" ), p_shm_obj ) );
        return true;
    } else {
        return false;
    }

    //if (!InitSingleShm<AcctItemAggrItem, PI_BUSISHM_FIRST+1>("AcctItemAggr")) return false;
    /****
    //
    if (!InitSingleShm<IndexOfferInstIdOfAcctItemAggr, PI_BUSISHM_FIRST+2>("",2)) return false;
    //
    if (!InitSingleShm<AcctItemOweItem, PI_BUSISHM_FIRST+3>("")) return false;
    //
    if (!InitSingleShm<IndexOfferIdOfAcctItemOwe, PI_BUSISHM_FIRST+4>("",2)) return false;
    //
    if (!InitSingleShm<AcctItemTypeItem, PI_BUSISHM_FIRST+5>("")) return false;
    //
    if (!InitSingleShm<AddressPartyRelItem, PI_BUSISHM_FIRST+6>("")) return false;
    //
    if (!InitSingleShm<IndexChannelIdOfAddressPartyRel, PI_BUSISHM_FIRST+7>("",2)) return false;
    //
    if (!InitSingleShm<ChannelMemberItem, PI_BUSISHM_FIRST+8>("")) return false;
    //
    if (!InitSingleShm<ChannelEquipRelItem, PI_BUSISHM_FIRST+9>("")) return false;
    //
    if (!InitSingleShm<IndexSubLineBoxCodeOfChannelEquipRel, PI_BUSISHM_FIRST+10>("",2)) return false;
    //
    if (!InitSingleShm<IndexJunctionBoxCodeOfChannelEquipRel, PI_BUSISHM_FIRST+11>("",2)) return false;
    //
    if (!InitSingleShm<ChannelItem, PI_BUSISHM_FIRST+12>("")) return false;
    //
    if (!InitSingleShm<IndexPartyIdOfChannel, PI_BUSISHM_FIRST+13>("",2)) return false;
    //
    if (!InitSingleShm<CalcResultDetailItem, PI_BUSISHM_FIRST+14>("")) return false;
    //
    if (!InitSingleShm<GridItem, PI_BUSISHM_FIRST+15>("")) return false;
    //
    if (!InitSingleShm<ProductOfferItem, PI_BUSISHM_FIRST+16>("")) return false;
    //
    if (!InitSingleShm<PaymentItem, PI_BUSISHM_FIRST+17>("")) return false;
    //
    if (!InitSingleShm<PartnerOfferInstanceItem, PI_BUSISHM_FIRST+18>("")) return false;
    //
    if (!InitSingleShm<PartnerItem, PI_BUSISHM_FIRST+19>("")) return false;
    //
    if (!InitSingleShm<RatableResourceAccumulatorItem, PI_BUSISHM_FIRST+20>("")) return false;
    //
    if (!InitSingleShm<ServTerminalItem, PI_BUSISHM_FIRST+21>("")) return false;
    //
    if (!InitSingleShm<SimCardActiveItem, PI_BUSISHM_FIRST+22>("")) return false;
    //
    if (!InitSingleShm<IndexAccNbrOfSimCardActive, PI_BUSISHM_FIRST+23>("",2)) return false;
    //
    if (!InitSingleShm<StampInfoItem, PI_BUSISHM_FIRST+24>("")) return false;
    //
    if (!InitSingleShm<SimCardPartyRelItem, PI_BUSISHM_FIRST+25>("")) return false;
    //
    if (!InitSingleShm<IndexAccNbrOfSimCardPartyRel, PI_BUSISHM_FIRST+26>("",2)) return false;
    //
    if (!InitSingleShm<SettleItemDetailItem, PI_BUSISHM_FIRST+27>("")) return false;
    //
    if (!InitSingleShm<ServLocationItem, PI_BUSISHM_FIRST+28>("")) return false;
    //
    if (!InitSingleShm<ServGridRelItem, PI_BUSISHM_FIRST+29>("")) return false;
    //
    if (!InitSingleShm<IndexServIdOfServGridRel, PI_BUSISHM_FIRST+30>("",2)) return false;
    //
    if (!InitSingleShm<ServEquipItem, PI_BUSISHM_FIRST+31>("")) return false;
    //
    if (!InitSingleShm<ServChannelRelItem, PI_BUSISHM_FIRST+32>("")) return false;
    //
    if (!InitSingleShm<IndexChannelIdOfServChannelRel, PI_BUSISHM_FIRST+33>("",2)) return false;
    //
    if (!InitSingleShm<TerminalRegistItem, PI_BUSISHM_FIRST+34>("")) return false;
    ****/
}

//��ѯ����ҵ�����ڴ�ͳ����Ϣ
//��Ӧ��Ʒ������: psshm -stat
bool CBusiShmMgr::QueryStatInfo( std::vector<ShmStatInfo>& vec_stat_info )
{
    //
    bool ret = true;

    //�����
    for( T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.begin(); it != map_table_obj_.end(); it++ ) {
        ret = ret && it->second->QueryStatInfo( vec_stat_info );
    }

    //��������
    for( T_SHM_OBJ_MAP::const_iterator it = map_index_obj_.begin(); it != map_index_obj_.end(); it++ ) {
        ret = ret && it->second->QueryStatInfo( vec_stat_info );
    }

    return ret;
}

//��ѯ����ҵ�����ڴ����Ϣ
//��Ӧ��Ʒ������淶��Ҫ�󣬿�������չ
bool CBusiShmMgr::QueryTableInfo( std::vector<ShmTableInfo>& vec_table_info )
{
    //
    bool ret = true;

    for( T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.begin(); it != map_table_obj_.end(); it++ ) {
        ret = ret && it->second->QueryTableInfo( vec_table_info );
    }

    return ret;
}

//�鿴���¼
//��Ӧ��Ʒ������: psshm -show table_name
//����ֵ��1 �����м�¼ 0 ���� -1 ����
int CBusiShmMgr::ShowTable( const std::string& table_name,
                            std::vector<std::vector<std::pair<std::string, std::string> > >& vec_record )
{
    int ret = -1;

    T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.find( table_name );

    if( it != map_table_obj_.end() ) {
        if( table_name != table_name_ || conditon_ != "" ) {
            page_index_ = 1;
            table_name_ = table_name;
            conditon_ = "";
        }

        ret = it->second->ShowData( page_index_, SHOW_PAGE_SIZE, vec_record );
        ++page_index_;
    } else {
        err_code_ = SHMTABLE_NOT_EXISTS;
        err_msg_ = "����" + table_name + "������";
    }

    return ret;
}

//��Ӧ��Ʒ������: psshm -show table_name [indexcolumn=value][,indexcolumn=value]
//����ֵ��1 �����м�¼ 0 ���� -1 ����
//indexcolumnҪ֧�������У�������ʹ�ñ���ʵ��
int CBusiShmMgr::ShowTable( const std::string& table_name, const std::string& condition,
                            std::vector<std::vector<std::pair<std::string, std::string> > >& vec_record )
{
    int ret = -1;

    T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.find( table_name );

    if( it != map_table_obj_.end() ) {
        if( table_name != table_name_ || condition != conditon_ ) {
            page_index_ = 1;
            table_name_ = table_name;
            conditon_ = condition;
        }

        ret = it->second->ShowData( condition, page_index_, SHOW_PAGE_SIZE, vec_record, err_msg_ );
        ++page_index_;

        if( ret = -1 ) {
            err_code_ = CONDITION_IS_INVALID;
        }
    } else {
        err_code_ = SHMTABLE_NOT_EXISTS;
        err_msg_ = "����" + table_name + "������";
    }

    return ret;
}

//��ҳ��λ.��λ����һҳ
bool CBusiShmMgr::ShowReset()
{
    page_index_ = 1;
}


//ͳ�Ƽ�¼��
//��Ӧ��Ʒ������: psshm -count table_name
//����ֵ��>=0 ��¼��  -1 ����
int CBusiShmMgr::QueryRecordCount( const std::string& table_name )
{
    int ret = -1;

    T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.find( table_name );

    if( it != map_table_obj_.end() ) {
        ret = it->second->QueryRecordCount();
    } else {
        err_code_ = SHMTABLE_NOT_EXISTS;
        err_msg_ = "����" + table_name + "������";
    }

    return ret;
}

//��Ӧ��Ʒ������: psshm -count table_name [indexcolumn=value]
//����ֵ��>=0 ��¼��  -1 ����
int CBusiShmMgr::QueryRecordCount( const std::string& table_name, const std::string& condition )
{
    int ret = -1;

    T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.find( table_name );

    if( it != map_table_obj_.end() ) {
        ret = it->second->QueryRecordCount( condition, err_msg_ );

        if( ret == -1 ) {
            err_code_ = CONDITION_IS_INVALID;
        }
    } else {
        err_code_ = SHMTABLE_NOT_EXISTS;
        err_msg_ = "����" + table_name + "������";
    }

    return ret;
}

//��ѯ��״̬
//��Ӧ��Ʒ������: psshm -lockstat [table_name]
//����ֵ��0 ����  1����
int CBusiShmMgr::QueryLockStatus( const std::string& table_name )
{
    int ret = -1;

    T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.find( table_name );

    if( it != map_table_obj_.end() ) {
        ret = it->second->QueryLockStatus();
    } else {
        err_code_ = SHMTABLE_NOT_EXISTS;
        err_msg_ = "����" + table_name + "������";
    }

    return ret;
}

//������״̬
//��Ӧ��Ʒ������: psshm -lockreset [table_name] [,table_name]
//����ֵ��0 ����  1����
int CBusiShmMgr::LockReset( const std::string& table_name )
{
    StringTokenizer token( table_name, "," );

    //�Ⱥ˲�table_name�Ƿ񶼺Ϸ�
    for( int i = 0; i < token.size(); i++ ) {
        T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.find( token[i] );

        if( it == map_table_obj_.end() ) {
            err_code_ = SHMTABLE_NOT_EXISTS;
            err_msg_ = "����" + token[i] + "������";
            return -1;
        }
    }

    //Reset
    for( int i = 0; i < token.size(); i++ ) {
        T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.find( token[i] );

        if( it != map_table_obj_.end() ) {
            it->second->LockReset();
        }
    }

    return 0;
}

//����
//��Ӧ��Ʒ������: psimport �Cm disct �Ct disct_data
//����ֵ��>=0 �ɹ�����ļ�¼�� -1 ����
int CBusiShmMgr::ImportFromDB( const std::string& shm_name, const std::string& table_name )
{
    int ret = -1;

    T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.find( shm_name );

    if( it != map_table_obj_.end() ) {
        ret = it->second->ImportFromDB( table_name, err_code_, err_msg_ );
    } else {
        err_code_ = SHMTABLE_NOT_EXISTS;
        err_msg_ = "�����ڴ�" + shm_name + "������";
    }

    return ret;
}

//��Ӧ��Ʒ�����psimport �Cm disct �Cf file_name
//����ֵ��>=0 �ɹ�����ļ�¼�� -1 ����
int CBusiShmMgr::ImportFromFile( const std::string& shm_name, const std::string& file_name )
{
    int ret = -1;

    std::string file_full_name = file_name;

    if( !es::Path::isAbsolute( file_name ) ) {
        std::string default_path = getenv( "SETTLEDIR" );
        default_path = default_path + "/tmp/shm_bak";
        file_full_name = default_path + "/" + file_name + ".txt";
    }

    T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.find( shm_name );

    if( it != map_table_obj_.end() ) {
        ret = it->second->ImportFromFile( file_full_name, err_code_, err_msg_ );
    } else {
        err_code_ = SHMTABLE_NOT_EXISTS;
        err_msg_ = "�����ڴ�" + shm_name + "������";
    }

    return ret;
}

//����
//��Ӧ��Ʒ������: psexport �Cm disct �Ct disct_data
//����ֵ��>=0 �ɹ������ļ�¼�� -1 ����
int CBusiShmMgr::ExportToDB( const std::string& shm_name, const std::string& table_name )
{
    int ret = -1;

    T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.find( shm_name );

    if( it != map_table_obj_.end() ) {
        ret = it->second->ExportToDB( table_name, err_code_, err_msg_ );
    } else {
        err_code_ = SHMTABLE_NOT_EXISTS;
        err_msg_ = "�����ڴ�" + shm_name + "������";
    }

    return ret;
}

//��Ӧ��Ʒ�����psexport �Cm disct �Cf file_name
//����ֵ��>=0 �ɹ������ļ�¼�� -1 ����
int CBusiShmMgr::ExportToFile( const std::string& shm_name, const std::string& file_name )
{
    int ret = -1;

    std::string file_full_name = file_name;

    if( !es::Path::isAbsolute( file_name ) ) {
        std::string default_path = getenv( "SETTLEDIR" );
        default_path = default_path + "/tmp/shm_bak";
        file_full_name = default_path + "/" + file_name + ".txt";
    }

    std::string file_path = es::Path::extractPath( es::Path( file_full_name ) );

    //�ж�Ŀ¼�Ƿ���ڣ��������򴴽�
    if( access( file_path.c_str(), F_OK ) < 0 ) {
        mkdir( file_path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO );
    }

    T_SHM_OBJ_MAP::const_iterator it = map_table_obj_.find( shm_name );

    if( it != map_table_obj_.end() ) {
        ret = it->second->ExportToFile( file_full_name, err_code_, err_msg_ );
    } else {
        err_code_ = SHMTABLE_NOT_EXISTS;
        err_msg_ = "�����ڴ�" + shm_name + "������";
    }

    return ret;
}

