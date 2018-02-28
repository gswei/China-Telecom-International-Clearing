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

#define  ERR_NEW_MEMORY   10001  /*��̬�����ڴ�ʧ��*/
#define  ERROR_GROUPSEQ_DEFINED          10015//�ʵ������кŶ������

class CNumList 
{
public:
	int		iNumList[100];//һ������Ű����ķ�����LIST_OUTPUT_FIELD�ж�Ӧ�ı��
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
	char		fee_index[15];		//������ı��,�������
	char		group_index[15];	//��Ӧ�����
	char		len_type[150];		//���������Щ����
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
	vector<FEE_AND_GROUP> m_pList;//��¼��list_ruleno_define���м�¼����Ϣ
	int			m_iDataCount;	//list_ruleno_define�������ݵ�ʵ������
	int			m_iListLen;		//list_ruleno_define�й���Ÿ�����Ӧ����m_iDataCount���
	int			m_iGroupCount;	//�ʵ�����
	vector<int>  m_groupLenth;//��¼ÿ����ĳ���
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
