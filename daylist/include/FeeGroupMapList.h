#ifndef _FEEGROUPMAPLIST_H_
#define _FEEGROUPMAPLIST_H_ 1

#include "CF_Common.h"
#include "CF_Cerrcode.h"
#include "CF_CFmtChange.h"

#include <string>
#include <map>
//using namespace std;

#include "DataInOutMap.h"
#include "InputInfo.h"
#include "psutil.h"

#define  ERR_NEW_MEMORY   10001  /*动态分配内存失败*/
#define  ERROR_GROUPSEQ_DEFINED          10015//帐单组序列号定义错误

class CNumList 
{
public:
	int		iNumList[100];//一个规则号包含的费用在LIST_OUTPUT_FIELD中对应的编号
	int		iNumCount;
	
	CNumList();
	~CNumList();
	
	void addNum( int iNewNum );
	
	int isNumExisted( int iNum );
	
	void clear();
	
	void getAllNum( const char *pChar );
	
	void PrintMe();
};

struct FEE_AND_GROUP {
	char		fee_index[15];		//费用项的编号,即规则号
	char		group_index[15];	//对应组序号
	char		len_type[150];		//该组包含那些费用
	CNumList	num_list;
	
	FEE_AND_GROUP() {
		memset( fee_index, 0, 15 );
		memset( group_index, 0, 15 );
		memset( len_type, 0, 100 );
	}	
};

class CFeeGroupMapList 
{
public:
	vector<FEE_AND_GROUP> m_pList;//记录了list_ruleno_define所有记录的信息
	int			m_iDataCount;	//list_ruleno_define表中数据的实际条数
	int			m_iListLen;		//list_ruleno_define中规则号个数，应该与m_iDataCount相等
	int			m_iGroupCount;	//帐单组数
	vector<int>  m_groupLenth;//记录每个组的长度
	//CDatabase	*m_db;	
	int total_count_flag;
	int attr_group_seq;
	CFeeGroupMapList();
	void Init(char *list_id);
	~CFeeGroupMapList();
	
	int getGroupIndex( const char *feeIndex );
	
	int getGroupCount(char *list_id);
	
						
	char *getFeeItem( int groupIndex );

public:	
	int addData( FEE_AND_GROUP data );
	
	void clearList();	
	
	int getAllData(char *list_id);
	
	int getListLength(char *list_id);
	
	int getLenType( char *pLenType, const char *feeIndex );
	
	int getLenType( char *pLenType, int groupIndex );
	
	int isFieldEnabled( int iGroupIndex, int iFieldIndex );
	
	void printAllList();

	int getDataCount();
	
};

#endif
