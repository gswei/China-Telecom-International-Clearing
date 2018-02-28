/*******************************************************************************************
	Copyright 2004 Poson Co.,Ltd. Inc. All Rights Reversed   
********************************************************************************************

**Class			: CRecord_Change
**Description		: 综合结算--话单格式转换类
**Author		: sunhua
**StartTime		: 2005/09/08
**Last Change Time 	: 2005/09/08

*******************************************************************************************/

#ifndef _CRECORD_CHANGE_H_
#define _CRECORD_CHANGE_H_

#include <vector>

#include "CF_CError.h"
#include "CFmt_Change.h"
#include "config.h"
#include "COracleDB.h"

class CInput2Output
{
protected:
	int In_Idx;
	int Out_Idx;
public:
	CInput2Output(int in,int out);
	~CInput2Output();
	
	//获取In_Idx
	int get_In_Idx() { return In_Idx; }
	//获取Out_Idx
	int get_Out_Idx() { return Out_Idx; }
};
typedef vector<CInput2Output> VIn2Out;

class CRecord_Change
{
protected:
	/*输出文件类型ID*/
	char OutFileTypeID[6];
	/*输入文件类型ID*/
	char InFileTypeID[6];
	/*输入输出格式转换对应关系*/
	VIn2Out	FileType_Chg;
public:
	int Init( char *in, char *out );
	int Rec_Change( CFmt_Change &inR, CFmt_Change &outR );
};

#endif
