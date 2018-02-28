/****************************************************************
  Project	
  Copyright (c)	2004-2006. All Rights Reserved.
		广州普信科技有限公司 
  SUBSYSTEM:	插件框架模块数据解析公用文件  
  FILE:		Packet.cpp
  AUTHOR:	wulei   
  Create Time:  2004-08-25
==================================================================
  Description:  

  UpdateRecord: 
==================================================================

 *****************************************************************/

#include "Packet.h"

BufferParser::BufferParser(){
	memset(m_buffer, 0, sizeof(m_buffer));
};

BufferParser::BufferParser(const char* buf){
	memset(m_buffer, 0, sizeof(m_buffer));
	strcpy(m_buffer,buf);
	return;
}; 

BufferParser::~BufferParser(){

};

/* Set value of indicated field, then set the responsable bit in bitmap.
 * Return 0 on success, -1 on failure
 */
int BufferParser::setFieldValue(int field, const char buffer[], const int len){
         for (int i =0;i<len+1;i++)
	{
	        if ((field+i-1) > strlen(m_buffer)){
	                return -1;
	                }
	        m_buffer[field+i-1]=buffer[i];
	}
	
	return 0;
};

/* Get value of indicated field.
 * Return length of buffer.
 */
int BufferParser::getFieldValue (const int field, char* buffer,const int len){
	for (int i =0;i<len+1;i++)
	{
	        if ((field+i-1) > strlen(m_buffer)){
	                return -1;
	                }
	        buffer[i]=m_buffer[field+i-1];
	}
	DelSpace(buffer);
	return strlen(buffer);
};

/* Clear value of indicated field. */
void BufferParser::clearFieldValue (const int field, const int len){
	for (int i =0;i<len+1;i++)
	{
	        if ((field+i-1) > strlen(m_buffer)){
	                return ;
	                }
	        m_buffer[field+i-1]=0;
	}
};


void BufferParser::printMe(){
        cout <<m_buffer << endl;
	return;
};

void BufferParser::DelSpace( char *ss )
{
    int i,len,j;
    i = strlen(ss)-1;
    while (i && ss[i] == ' ') i--;
    ss[i+1] = 0;
    i = 0;
    len = strlen(ss);
    while((i<len)&&ss[i]==' ')i++;
    if(i!=0)
    {
        for(j = i;j<len;j++)
        {
           ss[j-i] = ss[j];
        }
    }
    ss[len-i] = 0;	
}


//*****************************
//Class PacketParser code begin

PacketParser::PacketParser(){
	initPacket();
};


PacketParser::~PacketParser(){
	//freePacket();
};


/* Get a null pack with variables initialized.
 * Return pointer to the pack on success, NULL on failure.
 */
void PacketParser::initPacket(){

        item_num = 0;
	for (int i=0; i<ATTRINUMBER; i++){
		m_length[i] = 0;
		m_buffer[i] = NULL;
	}
	return ;
};


/* Free the memory allocated by the buffers of the pack. */
void PacketParser::freePacket(){
	for (int i=0; i<ATTRINUMBER; i++)
		if (m_buffer[i]){
			free(m_buffer[i]);
			m_buffer[i] = NULL;
		};
};


//add by lwu 2005-04-20
/* Set value of indicated field, then set the responsable bit in bitmap.
 * Return 0 on success, -1 on failure
 */
int PacketParser::setValue(const int field, char* buffer, const int len){

	if (len<1)
	        return -1;
	        
	//if (m_buffer[field] == NULL)
	/* If the buffer of this field hasn't been alocated */
	//	if ((m_buffer[field] = (char *)malloc(len)) == NULL)
	//		return -1;
	
	//modify by wulf at 20060421 结尾加结束符	
	m_length[field] = len;
	memcpy(m_buffer[field], buffer, len);
	m_buffer[field][len]=0;
	//m_buffer[field] = buffer;
	//item_num = item_num+1;
	return 0;
};

/* Set value of indicated field, then set the responsable bit in bitmap.
 * Return 0 on success, -1 on failure
 */
int PacketParser::setFieldValue(const int field, char* buffer, const int len){

	if (len<1)
	        return -1;
	        
	//if (m_buffer[field] == NULL)
	/* If the buffer of this field hasn't been alocated */
	//	if ((m_buffer[field] = (char *)malloc(len)) == NULL)
	//		return -1;

	m_length[field] = len;

	//memcpy(m_buffer[field], buffer, m_length[field]);
	m_buffer[field] = buffer;
	item_num = item_num+1;
	return 0;
};

/* Get value of indicated field.
 * Return length of field.
 */
int PacketParser::getFieldValue (const int field, char buffer[])const{
	memcpy(buffer,m_buffer[field], m_length[field]);
	return m_length[field];
};

/* Clear value of indicated field. */
void PacketParser::clearFieldValue (const int field){
	m_buffer[field] = NULL;
	m_length[field] = 0;
	item_num = item_num-1;
};

void PacketParser::printMe(){
	cout<<"The total number is"<< item_num<<endl;
        for (int i=0; i<item_num; i++){
		if (m_buffer[i]){
			cout<<"field id is :"<<i<<", values is:"<<m_buffer[i]<<",length is :"<< m_length[i]<<endl; 
		};
	};
	return;
}

//*****************************
//Class ResParser code begin

ResParser::ResParser(){
	initPacket();
};


ResParser::~ResParser(){
	//freePacket();
};


/* Get a null pack with variables initialized.
 * Return pointer to the pack on success, NULL on failure.
 */
void ResParser::initPacket(){

        resitem_num = 0;
	for (int i=0; i<RESATTRINUMBER; i++){
		m_length[i] = 0;
		memset(m_buffer[i],0,MAXLENGTH);
	}
	return ;
};



/* Set value of indicated field, then set the responsable bit in bitmap.
 * Return 0 on success, -1 on failure
 */
int ResParser::setFieldValue(const int field, const char buffer[], const int len){

	if (len<1)
	        return -1;
	        
	m_length[field] = len;

	memcpy(m_buffer[field], buffer, m_length[field]);
	//add by lwu 2005-06-21 结尾加结束符
	m_buffer[field][m_length[field]]=0;
	resitem_num = resitem_num+1;
	return 0;
};

/* Get value of indicated field.
 * Return length of field.
 
int ResParser::getFieldValue (const int field, char buffer[])const{
	memcpy(buffer,m_buffer[field], m_length[field]);
	return m_length[field];
};*/
char* ResParser::getFieldValue (const int field){
        return m_buffer[field];
        };
int ResParser::getLength  (const int field){
        return m_length[field];
        };

/* Clear value of indicated field. */
void ResParser::clearFieldValue (const int field){
	memset(m_buffer[field],0,MAXLENGTH);
	m_length[field] = 0;
	resitem_num = resitem_num-1;
};

void ResParser::printMe(){
	cout<<"The total number is"<< resitem_num<<endl;


	for (int i=0; i<resitem_num; i++){
		if (m_buffer[i]){
			cout<<"field id is :"<<i<<", values is:"<<m_buffer[i]<<",length is :"<< m_length[i]<<endl; 
		};
	};
	return;
}

