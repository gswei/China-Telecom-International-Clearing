/****************************************************************
  Project	
  Copyright (c)	2004-2006. All Rights Reserved.
		广州普信科技有限公司 
  SUBSYSTEM:	插件框架模块数据解析公用头文件  
  FILE:		Packet.h
  AUTHOR:	wulei
  Create Time:  2004-08-25
==================================================================
  Description:  

  UpdateRecord: 
	
==================================================================

 *****************************************************************/

#ifndef _PACKET_H_
#define _PACKET_H_

#include <Const_Packet.h>
#include <stdio.h>
#include <iostream.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

#define RECORD_LENGTH  100

//typedef TPackType Packet;

class BufferParser {
	private:
	    char m_buffer[ITEMLENGTH];

	public:
		BufferParser();
		BufferParser(const char* buf); 

		~BufferParser();

		/* Set value of indicated field, then set the responsable bit in bitmap.
		 * Return 0 on success, -1 on failure
		 */
		int setFieldValue (const int field, const char buffer[], const int len);

		/* Get value of indicated field.
		 * Return length of field.
		 */
		int getFieldValue (const int field, char* buffer,const int len);

		/* Clear value of indicated field. */
		void  clearFieldValue (const int field, const int len);


		void printMe();
		
		void DelSpace( char *ss );

};


typedef struct SPacket{
	int  	item_num;
	char*	m_buffer[ATTRINUMBER];
	int	m_length[ATTRINUMBER];
}PPacket;

class PacketParser : private PPacket{

	public:
		PacketParser();
		//PacketParser(const BYTE bit_map[] , const BYTE string[]); 
		~PacketParser();

		/* Get a null pack with variables initialized.
		 * Return pointer to the pack on success, NULL on failure.
		 */
		void initPacket();

		/* Free the memory allocated by the buffers of the pack. */
		void freePacket();		

		/* Set value of indicated field, then set the responsable bit in bitmap.
		 * Return 0 on success, -1 on failure
		 */
		int setFieldValue (const int field, char* buffer, const int len);
		
		int setValue(const int field, char* buffer, const int len);
		
		/* Get value of indicated field.
		 * Return length of field.
		 */
		int getFieldValue (const int field, char buffer[]) const;

		/* Clear value of indicated field. */
		void  clearFieldValue (const int field);

		/*get the item_num value*/
		int getItem_num(){return item_num;};
		
		/*	print packet data
			for test only
		*/
		void printMe();

};

typedef struct RPacket{
	int  	resitem_num;
	char	m_buffer[RESATTRINUMBER][MAXLENGTH];
	int	m_length[RESATTRINUMBER];
}RParser;

class ResParser : private RParser{

	public:
		ResParser();
		
		~ResParser();

		/* Get a null pack with variables initialized.
		 * Return pointer to the pack on success, NULL on failure.
		 */
		void initPacket();

		/* Free the memory allocated by the buffers of the pack. */
		//void freePacket();		

		/* Set value of indicated field, then set the responsable bit in bitmap.
		 * Return 0 on success, -1 on failure
		 */
		int setFieldValue (const int field, const char buffer[], const int len);

		/* Get value of indicated field.
		 * Return length of field.
		 */
		char* getFieldValue (const int field);
		int getLength  (const int field);

		/* Clear value of indicated field. */
		void  clearFieldValue (const int field);

		/*get the item_num value*/
		int getItem_num(){return resitem_num;};
		
		/*	print packet data
			for test only
		*/
		void printMe();

};
#endif

