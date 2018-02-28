/******************************************************************
	Copyright (c) 2012, GUANGDONG YOSON TECHNOLOGY CO., LTD
	All rights reserved.

	Created:		2012/9/19
	Filename: 		busi_shm_mgr.h
	Description:	ҵ�����ڴ�����ࡣ
	                �ṩ��Ʒ��psshm�������ҵ�����ݽӿڣ�
					�ṩҵ�����ڴ�Ĺ���ӿڣ����������١��������ݡ�������ݵȣ���
					�ر�˵�����˹�����ʵ�ֵ�import/export�ӿڽ�ʵ�����ݵĵ���/������δʵ������ͬ�����£�

	History:
	<table>
		Revision	Author			Date		Description
		--------	------			----		-----------
		v1.0		raozh		 2012/9/19	    �����ӿڵĶ���
	</table>
*******************************************************************/

#ifndef BUSI_SHM_MGR_H_
#define BUSI_SHM_MGR_H_

#include "busi_shm_struct.h"
#include "es/util/Singleton.h"
#include <string>
#include <vector>
#include <map>

namespace tpss
{

typedef ns_shm_data::ShmStatInfo ShmStatInfo;
typedef ns_shm_data::ShmTableInfo ShmTableInfo;

//�����붨��
const int SHMTABLE_NOT_EXISTS = -100001;         //��ѯ���ڴ������
const int CONDITION_IS_INVALID = -100002;		 //��ѯ�������Ϸ�
const int TABLE_NOT_EXISTS = -100003;            //���ݿ������

//ÿҳshow�ļ�¼����
const int SHOW_PAGE_SIZE = 2;

//ҵ�����ڴ������
class CBusiShmMgr
{
public:
    CBusiShmMgr() {};
    ~CBusiShmMgr();

    //0.ͨ�ýӿ�
    //��ʼ������

    static CBusiShmMgr* Instance();
    //��ȡ������Ϣ
    void GetErrorMsg( int& err_code, std::string& err_msg ) {
        err_code = err_code_;
        err_msg = err_msg_;
    };

public:
    //1.��Ʒ������ӿ�

    //��ѯ����ҵ�����ڴ�ͳ����Ϣ
    //��Ӧ��Ʒ������: psshm -stat
    bool QueryStatInfo( std::vector<ShmStatInfo>& vec_stat_info );

    //��ѯ����ҵ�����ڴ����Ϣ
    //��Ӧ��Ʒ������淶��Ҫ�󣬿�������չ
    bool QueryTableInfo( std::vector<ShmTableInfo>& vec_table_info );

    //�鿴���¼
    //��Ӧ��Ʒ������: psshm -show table_name
    //����ֵ��1 �����м�¼ 0 ���� -1 ����
    int ShowTable( const std::string& table_name,
                   std::vector<std::vector<std::pair<std::string, std::string> > >& vec_record );
    //��Ӧ��Ʒ������: psshm -show table_name [indexcolumn=value]
    //����ֵ��1 �����м�¼ 0 ���� -1 ����
    //indexcolumnҪ֧�������У�������ʹ�ñ���ʵ��
    int ShowTable( const std::string& table_name, const std::string& condition,
                   std::vector<std::vector<std::pair<std::string, std::string> > >& vec_record );
    //��ҳ��λ.��λ����һҳ
    bool ShowReset();


    //ͳ�Ƽ�¼��
    //��Ӧ��Ʒ������: psshm -count table_name
    //����ֵ��>=0 ��¼��  -1 ����
    int QueryRecordCount( const std::string& table_name );
    //��Ӧ��Ʒ������: psshm -count table_name [indexcolumn=value]
    //����ֵ��>=0 ��¼��  -1 ����
    int QueryRecordCount( const std::string& table_name, const std::string& condition );

    //��ѯ��״̬
    //��Ӧ��Ʒ������: psshm -lockstat [table_name]
    //����ֵ��0 ����  1����  -1����
    int QueryLockStatus( const std::string& table_name );
    //������״̬
    //��Ӧ��Ʒ������: psshm -lockreset [table_name] [,table_name]
    //����ֵ��0 ����  1����  -1����
    int LockReset( const std::string& table_name );

    //����
    //��Ӧ��Ʒ������: psimport �Cm disct �Ct disct_data
    //����ֵ��>=0 �ɹ�����ļ�¼�� -1 ����
    int ImportFromDB( const std::string& shm_name, const std::string& table_name );
    //��Ӧ��Ʒ�����psimport �Cm disct �Cf file_name
    //����ֵ��>=0 �ɹ�����ļ�¼�� -1 ����
    int ImportFromFile( const std::string& shm_name, const std::string& file_name );

    //����
    //��Ӧ��Ʒ������: psexport �Cm disct �Ct disct_data
    //����ֵ��>=0 �ɹ������ļ�¼�� -1 ����
    int ExportToDB( const std::string& shm_name, const std::string& table_name );
    //��Ӧ��Ʒ�����psexport �Cm disct �Cf file_name
    //����ֵ��>=0 �ɹ������ļ�¼�� -1 ����
    int ExportToFile( const std::string& shm_name, const std::string& file_name );

    //�߶�
    //��Ӧ��Ʒ�����psshm -show �Chighquota
    //����ֵ��>0:�����м�¼  0:����  -1:����
    int ShowHighQuota( std::vector<std::vector<std::pair<std::string, std::string> > >& vec_record ) {};
    //�߶��ҳ��λ.��λ����һҳ
    bool ShowReset_HighQuota() {};

    //LRU
    //��Ӧ��Ʒ�����psshm -show �Clru
    //����ֵ��>0:�����м�¼  0:����  -1:����
    int ShowLRU( std::vector<std::pair<std::string, std::string> > & vec_record ) {};
public:
    //2.����ӿڣ����������١��������ݡ�������ݵȣ�

public:
    //typedef��ҵ�����ڴ�
    //T_SHMSET_NO_MUTEX

private:
    typedef std::map<std::string, ns_shm_data::IShmActiveObject*> T_SHM_OBJ_MAP;

    bool Init();

    static bool bInstanced;                 //�Ƿ���ʵ������
    int err_code_;                          //��ǰ������
    std::string err_msg_;                   //��ǰ������Ϣ
    T_SHM_OBJ_MAP map_table_obj_;           //ҵ�����ڴ����󼯺�(map)
    T_SHM_OBJ_MAP map_index_obj_;           //ҵ�����ڴ��������󼯺�(map)
    std::string table_name_;                //��ǰshow��table_name
    std::string conditon_;                  //��ǰshow��condition
    int page_index_;                        //��ǰshow��page_index
};

}

#endif  /*BUSI_SHM_MGR_H_*/
