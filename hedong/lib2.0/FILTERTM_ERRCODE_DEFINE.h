/******************************************************************************
*FILTERTM_ERRCODE_DEFINE.h
*created by tanj
******************************************************************************/
#ifndef _FILTERTM_ERRCODE_DEFINE_H_
#define _FILTERTM_ERRCODE_DEFINE_H_



const int  FILTERTM_NOT_ENOUGH_MEMORY         = 7010;  //��̬�����ڴ�ʧ��
const int  FILTERTM_LOGIC_ERR_IN_PROGRAM      = 7011;
const int  FILTERTM_ERR_IN_CREATE_DIR         = 7012;
const int  FILTERTM_ERR_IN_CREATE_INDEX_FILE  = 7013;
const int  FILTERTM_ERR_IN_OPEN_INDEX_FILE    = 7014;
const int  FILTERTM_ERR_IN_READ_ENV_VAR       = 7015;  //��ȡ������������
const int  FILTERTM_ERR_IN_CONNECT_DB         = 7020;  //�������ݿ����
const int  FILTERTM_NO_MATCH_RECORD_IN_DB     = 7022;
const int  FILTERTM_DATA_NOT_CONSISTENT       = 7023;
const int  FILTERTM_ERR_IN_WRITE_FILE         = 7024;
const int  FILTERTM_ERR_PK_DUPLICATE          = 7025;  //���ݱ������ظ�
const int  FILTERTM_ERR_CYCLE_ID              = 7026;    //�޷��鵽�Ʒ����ڣ����ڰ���ȥ��
const int  FILTERTM_ERR_UNKNOWN_CATCH         = 7027;  //��׽���޷�ʶ��Ĵ�����
const int  FILTERTM_ERR_IN_FILTER_CONDITION   = 7028;   //added by tanj 20060116 ȥ����������ȷ
const int  FILTERTM_ERR_IN_TIME_FIELD_FORMAT  = 7029;  //ADDED BY TANJ 20060209 ȥ��ʱ��������ʽ����ȷ

//const int  FILTERTM_ERR_NEED_RESTART          = 324;











#endif
