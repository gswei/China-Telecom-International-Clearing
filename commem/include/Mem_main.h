/*
 * =====================================================================================
 *
 *       Filename:  Mem_main.cpp
 *
 *    Description:  �����ݿ��е����ݱ���빲���ڴ����������ֺ����ݿ��ͬ����ʹ������������������ݷ���
 *
 *        Version:  1.0
 *        Created:  2010��05��09�� 08ʱ59��13��
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *        Company:
 *
 *    update list: 
 *    2010-05-09 :  ��ʼ�汾

 * =====================================================================================
 */

#ifndef _COMMON_MAIN_H_
#define _COMMON_MAIN_H_

#include "CommonMemManager.h"
//#include "CF_COracleDB.h"
#include "CF_CReadIni.h"
#include <stdlib.h> 
#include "es/util/StringUtil.h"


//��� commem -c �����Ƿ����
int ProcIsExist(int argc, char *argv[]);
//int commenKeyValue; //   �����ڴ�ֵ���Ӻ��Ĳ�����ȡ
//ɱ������
void  KillProc(int iProc);

#endif
