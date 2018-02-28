

#ifndef _CF_COMP_EXP_H_
#define _CF_COMP_EXP_H_ 1


#include "voiceformat.h"
#include "CF_CInterpreter.h"
#include "CF_Common.h"




class Comp_Exp
{
public:
  int AddVariable(CFmt_Change &);
  int Comp_Expression(char *);
  int Set_FileFmt(char *);
  char *Get_FileFmt();
private:
  C_Compile m_Compile;
  char m_FileFmt[6];

};

#endif
