/****************************************************************
  Project	
  Copyright (c)	2004-2006. All Rights Reserved.
		�������ſƼ����޹�˾ 
  SUBSYSTEM:	���Դ�ļ�  
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
�ڲ��������һ������
���������1���ò��������к�
         2������������
         3������������
         4������ֵ
���ֵ��  �ޣ�������ֱ�ӷ����Ա����m_params��
����ֵ��  �ɹ���־0
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
���ز���е����в���
���������1���ò��������к�
         2������������
         3������������
         4������ֵ
���ֵ��  �ޣ�������ֱ�ӷ����Ա����m_params��
����ֵ��  �ɹ���־0
*/
VParam* BasePlugin::getVParam(){
        return &(m_params);
        
}
/*
���ز���еĵ�idx������
����������ò��������к�
���ֵ��  ��
����ֵ��  ����鵽���ذ����ò�����SParam�ṹ���ָ�룬���򷵻�NULL
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