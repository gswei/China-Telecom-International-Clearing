/*********************************************************************
*	 Copyright (c)	2010. 广东亿迅科技有限公司
*      All Rights Reserved.
*
*	文件名称:	CTimeCheck.h
*	简单描述:	 检测是否到达信息点生成时间
*	当前版本:	3.0.0
*	作者:	    李春明
*	完成日期:	2010-07-12
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
    bool checkTime();   //检查是否到达时间点，若到达则 
    void getFromTo(char *from,char *to);
    void GetLastMonth(string& mon);
  private:
    string strTime; //原始串,邋infopoint_defne表的startime   
    string strCmpTime;
    //M每月一次，D每日一次，E每隔多少分钟一次，支持15，30，60分钟三种模式
    int timeType;  //0 月,1 日,3 间隔时间
    string strDay;   //分解得到日
    string strHHMMSS;//分解得到时分秒    
    string strSS;
    char qcType;

    int iDelay; 
    string m_strLastRestTime;
    string strFrom;
    string strTo;
    
    string strPreFrom;
    string strPreTo;    

    void resetTime(int);		//重设时间
    bool bProvided;    //是否已经提供
};
#endif