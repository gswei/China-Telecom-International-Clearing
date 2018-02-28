/*******************************************************************************************
*CF_CCdrTransformer.h
*created by tanj 2005.1.4
*******************************************************************************************/
#ifndef  _CF_CCDRTRANSFORMER_H_
#define  _CF_CCDRTRANSFORMER_H_


#include "config.h"
#include "COracleDB.h"
#include "CFmt_Change.h"
#include "CF_FieldMap.h"


class CF_CCdrTransformer
{
private:
  char Filetype_From[6];
  char Filetype_To[6];
  CF_FieldMap fieldMap;
public:
  CF_CCdrTransformer(char *filetype_from, char *filetype_to);
  ~CF_CCdrTransformer();
  void cdrTransform(CFmt_Change &fmtFrom, CFmt_Change &fmtTo);
  void cdrTransform(char *cdrFrom, CFmt_Change &fmtTo);
  void cdrTransform(char *cdrFrom, char *cdrTo);
  void cdrTransform(CFmt_Change &fmtFrom, char *cdrTo);
};


#endif
