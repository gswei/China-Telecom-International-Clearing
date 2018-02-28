/****************************************************************
  Project	
  Copyright (c)	2004-2006. All Rights Reserved.
		广州普信科技有限公司 
  SUBSYSTEM:	插件源文件  
  FILE:		Plugin.cpp
  AUTHOR:	wulei
  Create Time:  2004-08-25
==================================================================
  Description:  

  UpdateRecord:

==================================================================

 *****************************************************************/

#include "Plugin.h"

char* BasePlugin::getVersionNo() {
	return m_versionNo;
};

/*
void BasePlugin::setVersionNo(const char* versionNo){
	strcpy(m_versionNo , versionNo);
};*/

char* BasePlugin::getName() {
	return m_name;
};

/*
void BasePlugin::setName(const char* name){
	strcpy(m_name , name);
};*/

/*
在插件中增加一个参数
输入参数：1、该参数的序列号
         2、参数的名称
         3、参数的类型
         4、参数值
输出值：  无，将参数直接放入成员变量m_params中
返回值：  成功标志0
*/
int BasePlugin::addParam(const int idx ,const char name[10] ,const char type,const char val[120]){
	SParam sp;
	sp.idx = idx;
	strcpy(sp.name,name);
	//sp.name = string(name);
	sp.type = type;
	strcpy(sp.value,val);
	//sp.value = strig(val);
	m_params.push_back(sp);
	return 0;
};
/*
返回插件中的所有参数
输入参数：1、该参数的序列号
         2、参数的名称
         3、参数的类型
         4、参数值
输出值：  无，将参数直接放入成员变量m_params中
返回值：  成功标志0
*/
VParam* BasePlugin::getVParam(){
        return &(m_params);
        
}
/*
返回插件中的第idx个参数
输入参数：该参数的序列号
输出值：  无
返回值：  如果查到返回包含该参数的SParam结构体的指针，否则返回NULL
*/
SParam* BasePlugin::getParam(const int idx){
	VParam::iterator begin = m_params.begin();
	VParam::iterator end = m_params.end();
	for (;begin!=end;begin++){
		if (idx == begin->idx){
			return (SParam*)&(*begin);
		};
	};
	return NULL;
};


//add by weixy 20080604
/*int BasePlugin::execute(PacketParser& ps,ResParser& retValue, void * point  )
{
	return 0;
};*/