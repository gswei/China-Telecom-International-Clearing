/*********************************************************************
*	 Copyright (c)	2010. �㶫��Ѹ�Ƽ����޹�˾
*      All Rights Reserved.
*
*	�ļ�����:	CTimeCheck.h
*	������:	 ����Ƿ񵽴���Ϣ������ʱ��
*	��ǰ�汾:	3.0.0
*	����:	    ���
*	�������:	2010-07-12
*
*********************************************************************/
#include <fstream>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <string>
#include <map>
#include "Log.h"
#include "wrlog.h"
#include "COracleDB.h"
#include "CF_MemFileIO.h"
#include "Common.h"

#ifndef _CTIMECHECK_
#define _CTIMECHECK_ 1



class CTimeCheck
{
  public:
    void init(char *inStr,int iDy);
    bool checkTime();   //����Ƿ񵽴�ʱ��㣬�������� 
    void getFromTo(char *from,char *to);
    void GetLastMonth(string& mon);
  private:
    string strTime; //ԭʼ��,��infopoint_defne���startime   
    string strCmpTime;
    //Mÿ��һ�Σ�Dÿ��һ�Σ�Eÿ�����ٷ���һ�Σ�֧��15��30��60��������ģʽ
    int timeType;  //0 ��,1 ��,3 ���ʱ��
    string strDay;   //�ֽ�õ���
    string strHHMMSS;//�ֽ�õ�ʱ����    
    string strSS;
    char qcType;

    int iDelay; 
    string m_strLastRestTime;
    string strFrom;
    string strTo;
    
    string strPreFrom;
    string strPreTo;    

    void resetTime(int);		//����ʱ��
    bool bProvided;    //�Ƿ��Ѿ��ṩ
};
#endif