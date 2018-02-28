/*filename:ReadIni.cpp
descreption:define the interface to read ini file
create time:2010-04-13*/
#include "CF_CReadIni.h"

CReadIni theCfgFile;

CReadIni::CReadIni()
	{memset(IniFilename,0,sizeof(IniFilename));
	}
char * CReadIni::getVersion()
{
	return("3.0.0");
}

bool CReadIni::init(const char *Filename)
	{//char tmpFilename[LENTH];
  char LineBuff[LENTH];
	strcpy(IniFilename,Filename);
	if(access(Filename,F_OK|R_OK)!=0)
		return false;
	if(v_record.size()!=0)
			v_record.clear();
	ifstream ifs;
	ifs.open(Filename);
	if(!ifs)	
	return false;
	while(!ifs.eof())
	  {memset(LineBuff,0,sizeof(LineBuff));
	  ifs.getline(LineBuff,sizeof(LineBuff)-1);
	  trim(LineBuff);
	  if(LineBuff[0]=='\0' || LineBuff[0]=='#' || LineBuff[0]==';')
		continue;
		v_record.push_back(LineBuff);
		}
	ifs.close();
	return true;
	}

bool CReadIni::GetValue(const char *Section,const char *Variable,char *Value,char Reread)
	{
    if(Reread=='Y')
       if(!init(IniFilename))
	   	return false;
	if(IniFilename==NULL || v_record.size()==0)
		return false;
	//char tmpFilename[LENTH];
	char tmpSection[LENTH];
	char tmpVariable[LENTH];
	char UpperVariable[LENTH];//update 20100810,存放大写的variable
	char *tmpValue=NULL;
	char tmpRecord[LENTH];
	char chVariable[LENTH];//存放value值是变量的value;
	
	/*strcpy(tmpFilename,Filename);
	if(strcmp(tmpFilename,LastFilename)!=0)
		{if(v_record.size()!=0)
			v_record.clear();
			if(!init(tmpFilename))
				return false;
			}*/
	//init();
  sprintf(tmpSection,"[%s]",Section);
  toUpper(tmpSection);//update 20100810
  strcpy(UpperVariable,Variable);
  toUpper(UpperVariable);
  
  //先找到section
  vector<string>::iterator it;
  for(it=v_record.begin();it!=v_record.end();it++){
  	string UpperSection=*it;//update 20100810
    toUpper(UpperSection);
  	if(strcmp(tmpSection,(UpperSection).c_str())==0)
  		break;
  	}
  if(it==v_record.end())
  	return false;
  
  //再找Variable
  it++;
 // strcpy(tmpVariable,Variable);
  while(it!=v_record.end()){
  	memset(tmpRecord,0,sizeof(tmpRecord));
  	strcpy(tmpRecord,(*it).c_str());
	//toUpper(tmpRecord);//update 20100810,转换成大写
  	if(tmpRecord[0]=='[') //没有在指定的section在找到相应的变量Variable
  		return false;
  	if((tmpValue=strchr(tmpRecord,'='))==NULL)
        {it++;
	    continue;}
  	strncpy(tmpVariable,tmpRecord,tmpValue-tmpRecord);
  	tmpVariable[tmpValue-tmpRecord]='\0';
	trim(tmpVariable);
	toUpper(tmpVariable);
  	
  	if(strcmp(tmpVariable,UpperVariable)==0)//找到相应变量
  		{tmpValue++;
  		break;
  		}
  	it++;
  	}
  if(it==v_record.end())
  	return false;
  memset(Value,0,sizeof(Value));
  strcpy(Value,tmpValue);//对Value赋值
  trim(Value);
  
  int StrLength=strlen(Value);
  
  //对换行的value值进行处理
  it++;
  char tmpStrCat[LENTH];
  if(Value[StrLength-1]=='\\'){
  	Value[StrLength-1]='\0';
  	while(it!=v_record.end()){
  		strcpy(tmpStrCat,(*it).c_str());
  		StrLength=strlen(tmpStrCat);
  		if(tmpStrCat[StrLength-1]=='\\')
  			{tmpStrCat[StrLength-1]='\0';
  			 strcat(Value,tmpStrCat);
  			 it++;
  			}
  		else
  			{strcat(Value,tmpStrCat);
  				break;
				trim(Value);
  			}
  		}
  	}
 
  //Value值是一个变量
  char *posValue=NULL;
  if(Value[0]=='$')
  	{posValue=Value;
  	posValue++;
  	if((tmpValue=strchr(Value,'.'))!=NULL)
  		{strncpy(tmpSection,posValue,tmpValue-posValue);
  		tmpSection[tmpValue-posValue]='\0';
  		tmpValue++;
		strcpy(chVariable,tmpValue);
  		return GetValue(tmpSection,chVariable,Value);
  		}
  	else
  		{strcpy(chVariable,posValue);
  		return GetValue(Section,chVariable,Value);
  		}
  	}
  return true;
  
  }

bool CReadIni::GetValue(const char * Section, const char * Variable, int &Value, char Reread)
{char chValue[500];
if(!GetValue(Section,Variable,chValue,Reread))
	return false;
  if((Value=atoi(chValue))==0 && strcmp(chValue,"0")!=0)
		return false;
  return true;
}