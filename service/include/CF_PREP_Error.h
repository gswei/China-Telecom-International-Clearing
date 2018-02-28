/****************************************************************
filename: CF_PREP_Error.h
module: classify&analyse
created by: Wulf
create date: 2005-12-23
update list: 
version: 1.1.1
description:
    the header file of the classes for error
*****************************************************************/

#ifndef _CF_PREP_CERROR_H_
#define _CF_PREP_CERROR_H_ 1

#include <string>


const int PREDEAL_ERR_NOT_ENOUGH_MEMORY  = 5001;  //��̬�����ڴ�ʧ��
const int PREDEAL_ERR_IN_SELECT_DB       = 5002;  //��ѯ���ݳ���
const int PREDEAL_ERR_IN_CONNECT_DB      = 5003;  //�������ݿ����
const int ERR_SHM_ERROR       					 = 5004;  //�����ڴ���ʳ���
const int PREDEAL_ERR_SLFILE_NOT_EXIT    = 5005;  //����ļ�������
const int PREDEAL_ERR_PK_DUPLICATE       = 5006;  //���ݱ������ظ�
const int PREDEAL_ERR_UNKNOWN_CATCH      = 5007;  //��׽���޷�ʶ��Ĵ�����
const int PREDEAL_ERR_NEED_RESTART       = 5008;  //���ش��󣬽�������������
const int  ERR_LACK_PARAM      			=		     5009;  //���������������ȷ
const int ERR_SEM_ERROR                  = 5010;  //�ź������ʳ���
const int ERR_NEW_STRUCT                 = 5011;  //�������ṹ��ռ����
const int PREDEAL_ERR_SERVER_DIF         = 5012;  //zhjs.env�е�SERVER��PIPE�еĲ�һ��
const int ERR_GET_FILE                   = 5013;  //��ȡ�����е��ļ�����

#endif
