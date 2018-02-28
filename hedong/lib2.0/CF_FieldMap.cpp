#include "CF_FieldMap.h"
#include <stdio.h>
#include <string.h>
#include <iostream.h>
#include "COracleDB.h"
#include "CF_CError.h"

CF_FieldMap::CF_FieldMap()
{
	field_map = NULL;
}

CF_FieldMap::~CF_FieldMap()
{
	if (field_map != NULL)
	{
		delete[] field_map;
		field_map = NULL;
	}
}

int CF_FieldMap::Init(char *input_id,char *output_id)
{
	CBindSQL SqlStr( DBConn );
	
	field_count = 0;
	if (field_map != NULL) {
		delete[] field_map;
		field_map = NULL;
	}
	
	if (strcmp(input_id,output_id)==0) {
		same_one = 1;
		return 0;
	}
	else
		same_one = 0;

	SqlStr.Open("select count(*) from (select distinct input_id from input2output\
	  union select distinct output_id from input2output)", SELECT_QUERY);			
	SqlStr>>filetype_num;
	SqlStr.Close();
	
	first_node = new FILE_CONT;
	strcpy(first_node->file_id,input_id);
	first_node->pre = NULL;
	first_node->next = new FILE_CONT*[filetype_num-1];
	build_filenode(first_node);
	
	file_list = new FILE_LIST[filetype_num];
	filelist_num = 0;
	if (search_filenode(first_node,output_id)==-1) {
  	throw CF_CError('D','H',-1,0,(char *)"can't find matching file type.",__FILE__,__LINE__);
    return -1;
  }
  
  clear_filenode(first_node);
/*
	else {
		printf("%d\n",filelist_num);
		for (int i=filelist_num-1;i>=0;i--)
			printf("%s\n",file_list[i].file_id);
	}
*/
	
	int iFirst=0;
	int field_num_a=0;
	FIELD_MAP *tmp_map_a;
	for (int i=filelist_num-1;i>=1;i--)
	{
		char in_id[6],out_id[6];
		int field_num_b,field_num_c;
		FIELD_MAP *tmp_map_b,*tmp_map_c;
		
		strcpy(in_id,file_list[i].file_id);
		strcpy(out_id,file_list[i-1].file_id);
		SqlStr.Open("select count(*) from input2output where input_id=:input_id and output_id=:output_id", SELECT_QUERY);			
		SqlStr<<in_id<<out_id;
		SqlStr>>field_num_c;
		SqlStr.Close();
		
		tmp_map_c = new FIELD_MAP[field_num_c];
		
		SqlStr.Open("select input_index,output_index from input2output where input_id=:input_id and output_id=:output_id order by input_index", SELECT_QUERY);			
		SqlStr<<in_id<<out_id;
		for (int i=0;i<=field_num_c-1;i++)
		{
			SqlStr>>tmp_map_c[i].input_index>>tmp_map_c[i].output_index;
		}
		SqlStr.Close();				
		
		if (iFirst == 0) {
			tmp_map_a = tmp_map_c;
			field_num_a = field_num_c;
			iFirst = 1;
			continue;
		}
		else {
			tmp_map_b = tmp_map_a;
			field_num_b = field_num_a;		
			field_num_a=0;
			tmp_map_a = new FIELD_MAP[field_num_b];
			
			for (int i=0;i<field_num_b;i++)
				for (int j=0;j<field_num_c;j++)
				{
					if (tmp_map_b[i].output_index == tmp_map_c[j].input_index) {
						tmp_map_a[field_num_a].input_index = tmp_map_b[i].input_index;
						tmp_map_a[field_num_a].output_index = tmp_map_c[j].output_index;
						field_num_a++;
						break;
					}
				}
		}
		
		delete[] tmp_map_b;
		delete[] tmp_map_c;
	}
	
	field_map = new FIELD_MAP[field_num_a];
	field_count = field_num_a;
	//printf("%d\n",field_count);
	for (int i=0;i<field_count;i++) {
		field_map[i].input_index = tmp_map_a[i].input_index;
		field_map[i].output_index = tmp_map_a[i].output_index;
		//printf("%d	%d\n",field_map[i].input_index,field_map[i].output_index);
	}
	delete[] tmp_map_a;
	delete[] file_list;
	return 0;
}

int CF_FieldMap::Get_Index(int input_index)
{
  int pstart=0;
  int pend = field_count-1;
  int pmiddle = (pend+pstart)/2;
  int plast = 0;
  int ifind = 0;
  int islarger = 0;
  int isequal = 0;

  if (same_one == 1)
  	return input_index;
  
  FIELD_MAP *cur_field;
  cur_field = field_map;

  while (pstart <= pend) {
    if ((isequal==0)&&(pstart == pend))
      isequal = 1;
      else if (isequal==1)
        break;
    pmiddle = (pend+pstart)/2;
    if (plast<pmiddle) {
      cur_field=cur_field+(pmiddle-plast);
    }
      else if (plast>pmiddle) {
        //for (int i=plast;i>pmiddle;i--)
        cur_field=cur_field-(plast-pmiddle);
      }

    if (cur_field->input_index > input_index)
     	islarger = -1;
    	else if (cur_field->input_index < input_index)
    		islarger = 1;
  		else
  			islarger = 0;
    if (islarger>0) {
      //forward
      pstart = pmiddle+1;
      plast  = pmiddle;
    }
      else if (islarger<0) {
        //backward
        pend = pmiddle-1;
        plast  = pmiddle;
      }
        else {
          ifind = 1;
          plast = pmiddle;
          break;
        } //end else
        if (ifind == 1)
          break;
  }  //end while

  if (ifind==1) {
    return cur_field->output_index;
  }
    else {
  		//throw CF_CError('D','H',-1,0,(char *)"can't find matching field.",__FILE__,__LINE__);
      return -1;
    }
}

int CF_FieldMap::build_filenode(FILE_CONT *father_node)
{
	char father_id[6],child_id[6];
	CBindSQL SqlStr( DBConn );
	FILE_CONT *child_node,*cur_pre;
	
	strcpy(father_id,father_node->file_id);
	int iCur=0; 
	
	SqlStr.Open("select distinct output_id from input2output where input_id=:input_id and input_id<>output_id", SELECT_QUERY);
	SqlStr<<father_id;
	while (SqlStr>>child_id )
	{
		int iDur=0;
		cur_pre = father_node;
		while (cur_pre->pre != NULL)
		{
			cur_pre = cur_pre->pre;
			if (strcmp(child_id,cur_pre->file_id)==0)
			{
				iDur = 1;
				break;
			}
		}
		if (iDur == 1)
		  continue;
		  
		child_node = new FILE_CONT;
		strcpy(child_node->file_id,child_id);
		child_node->pre = father_node;
		child_node->next = new FILE_CONT*[filetype_num-1];
		build_filenode(child_node);
		father_node->next[iCur] = child_node;
		iCur++;
	}
	for (int i=iCur;i<=filetype_num-2;i++)
		father_node->next[i] = NULL;
	SqlStr.Close();
/*
	printf("------------------------\n");
	printf("%x\n",father_node);
	printf("%s\n",father_node->file_id);
	printf("%x\n",father_node->pre);
	for (int i=0;i<=filetype_num-2;i++)
		printf("%x	",father_node->next[i]);
	printf("\n");		
	printf("------------------------\n");
*/
	return 0;
}

int CF_FieldMap::clear_filenode(FILE_CONT *father_node)
{
	int iCur=0;
	
	while (father_node->next[iCur] != NULL)
	{
		clear_filenode(father_node->next[iCur]);
		iCur++;
		if (iCur == filetype_num-1)
			break;
	}
	delete[] *(father_node->next);
	delete[] father_node;
	return 0;	
}

int CF_FieldMap::search_filenode(FILE_CONT *father_node,char *file_id)
{
	int iCur=0;
	
	//printf("%s\n",father_node->file_id);
  if (strcmp(father_node->file_id,file_id)==0)
  {
  	//std::cout<<father_node->file_id<<endl;
  	strcpy(file_list[filelist_num].file_id,father_node->file_id);
  	filelist_num++;
  	while (father_node->pre != NULL)
  	{
  		father_node = father_node->pre;
  	  //std::cout<<father_node->file_id<<endl;  		
	   	strcpy(file_list[filelist_num].file_id,father_node->file_id);
  		filelist_num++;
  	}
  	return 0;
  }
	while (father_node->next[iCur] != NULL)
	{
		if (search_filenode(father_node->next[iCur],file_id)==0)
		  return 0;
		iCur++;
		if (iCur == filetype_num-1)
			break;
	}
	return -1;
}