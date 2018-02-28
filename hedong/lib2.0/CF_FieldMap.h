#ifndef _CF_FieldMap_H_
#define _CF_FieldMap_H_ 1

struct FIELD_MAP
{
	int input_index;
	int output_index;
};

struct FILE_CONT
{
	char file_id[6];
	FILE_CONT *pre;
	FILE_CONT **next;
};

struct FILE_LIST
{
	char file_id[6];
};

class CF_FieldMap
{
	private:
		int field_count;
		FIELD_MAP *field_map;
		FILE_LIST *file_list;
		int filelist_num;
		char input_id[6];
		char output_id[6];
		FILE_CONT *first_node;
		int filetype_num;
		int same_one;
		int build_filenode(FILE_CONT *father_node);
		int clear_filenode(FILE_CONT *father_node);
    int search_filenode(FILE_CONT *father_node,char *file_id);		
	public:
		CF_FieldMap();
		~CF_FieldMap();
		int Init(char *input_id,char *output_id);
		int Get_Index(int input_index);
};

#endif