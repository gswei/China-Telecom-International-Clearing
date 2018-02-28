/*************************************************
记载list_output_field表信息
*****************************************************/

#ifndef _DATAINPUTMAP_H_
#define _DATAINPUTMAP_H_ 1


#include "CF_Common.h"
#include "CF_CFmtChange.h"
#include "CF_Cerrcode.h"
#include <string>
#include <map>
#include "psutil.h"
//using namespace std;

#define ERROR_OUTPUTFIELD_DEFINED 10010
#define ERROR_OUTPUTFIELDTYPE_DEFINED        10016
#ifndef ERR_SELECT
#define          ERR_SELECT                   10003  /*查询数据出错*/
#endif

//LIST_OUTPUT_FIELD中FIELD字段可以包含话单字段的个数
#define FIELD_COUNT 20

#define  ERR_NEW_MEMORY   10001  /*动态分配内存失败*/

struct IN_AND_OUT {
	int field_index[FIELD_COUNT];
	char field_type;
	IN_AND_OUT() 
	{
		for(int i=0;i<FIELD_COUNT;i++)
			{
			field_index[i]=0;
			}
		field_type='D';
	}	
};

class CDataInOutMap {
public:
	IN_AND_OUT		*m_pList; //LIST_OUTPUT_FIELD中帐单可以包含所有费用项
	int			     m_iDataCount;	
	int			     m_iListLen;
	//CDatabase		*m_db;
	char 			inputFiletypeId[10];
	CDataInOutMap();
	~CDataInOutMap();
	void Init(char *szInputFiletypeId);
	int getOutputFieldCount();
	void printMe();
	//void mapping( const CInputInfo *inputList, CListInfo *outputList );

private:	
	int addData( char* data, char fieldType );
	
	void clearList();
	
	int getAllData();
	
	int getDataCount();
};

#endif
