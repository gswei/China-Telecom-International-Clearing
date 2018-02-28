/****************************************************************
filename: CF_InfoPoint.h
module: CF - Common Function
created by: zouguodong
create date: 2005-01-14
version: 1.0.0
description:
    the header file for infopoint
*****************************************************************/

#ifndef _CF_INFOPOINT_H_
#define _CF_INFOPOINT_H_ 1

//采集
#define INFO_COLLECTOR_COMM					"1-3-3-1-1"
#define INFO_COLLECTOR_FRONTDISK		"1-3-3-1-2"
#define INFO_COLLECTOR_CENTERDISK		"1-3-3-1-3"
#define INFO_COLLECTOR_FILENUM			"1-3-3-1-4"
//格式化+预处理
#define INFO_PREDEAL_ERROR					"1-3-3-2-1"
#define INFO_PREDEAL_IMBALANCE			"1-3-3-2-2"
#define INFO_PREDEAL_LACKINFO				"1-3-3-2-3"
//结算批价
#define INFO_CALCFEE_IMBALANCE			"1-3-3-3-1"
#define INFO_CALCFEE_LACKINFO				"1-3-3-3-4"
#define INFO_CALCFEE_ERROR					"1-3-3-3-6"
#define INFO_FILTER_ERROR						"1-3-3-3-7"
//结算处理
#define INFO_SMSSTAT_FILENUM				"1-3-3-4-1"
#define INFO_CRQBILL_ERROR					"1-3-3-4-3"
#define INFO_CRQBILL_IMBALANCE			"1-3-3-4-5"
#define INFO_STAT_CALLFLOW					"1-3-3-4-6"
//数据交换
#define INFO_DISPATCH_COMM					"1-3-3-5-2"
#define INFO_DISPATCH_MISDATA				"1-3-3-5-4"
//异常管理
#define INFO_ABNORMALITY_ERROR			"1-3-3-8-1"
#define INFO_ABNORMALITY_AFFIRM			"1-3-3-8-2"
#define INFO_ABNORMALITY_OVERLIMIT	"1-3-3-8-3"
#define INFO_ABNORMALITY_ULTRA			"1-3-3-8-4"
//出账处理
#define INFO_BALANCE_REDO						"1-3-3-10-1"
#define INFO_BALANCE_IMBALANCE			"1-3-3-10-3"
#define INFO_BALANCE_FINISH					"1-3-3-10-4"
#define INFO_BALANCE_CORRECTLY			"1-3-3-10-5"
//自定义
#define INFO_COLLECTOR_ERROR				"1-3-3-60-1"
#define INFO_FILETRANS_ERROR				"1-3-3-60-2"
#define INFO_FILEREC_ERROR					"1-3-3-60-3"
#define INFO_SMSSTAT_ERROR					"1-3-3-60-4"
#define INFO_DISPATCH_ERROR					"1-3-3-60-5"
#define INFO_COMMEM_ERROR						"1-3-3-60-6"
#define INFO_SMSBILL_ERROR					"1-3-3-60-7"
#define INFO_FILTERFM_ERROR					"1-3-3-60-8"
#define INFO_LIMIT_ERROR						"1-3-3-60-9"
#define INFO_ULTRA_ERROR						"1-3-3-60-10"
#define INFO_PICK_ERROR		  				"1-3-3-60-11"
#define INFO_PRELIST_ERROR					"1-3-3-60-12"
#define INFO_REPNOANA_ERROR					"1-3-3-60-13"

//信息点值
#define INFO_VALUE_TRUE							"(2,true)"
#define INFO_VALUE_FALSE						"(2,false)"


#endif
