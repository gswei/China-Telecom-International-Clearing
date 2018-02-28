/****************************************************************
filename: CF_CEnv.h
module: CF - Common Function
created by: wang hui
create date: 2000-10-13
update list: 
version: 1.1.1
description:
    the header file of the classes for CF_CEnv
*****************************************************************/

#ifndef _CF_CEnv_H_
#define _CF_CEnv_H_ 1
#define TIME_LEN		14

class CF_CEnv {
  public :
  	CF_CEnv(char *exchange_code, int module_id, char *env_name);
  	CF_CEnv(int module_id, char *env_name);
  	CF_CEnv(char *fileName, char *env_name);
 	
  	int loadEnvVar();

  	int loadEnvVarDefault();
	
	int readEnvVarFromFile();
	  	
  	void getenv(long &env_value);

	void getenv(char *env_value);
	
	void CF_get_current_time(char * curtime);
	
	int DeleteSpace( char *ss );

  private:
  	char localcode[8];
  	int moduleID;
  	char envName[32];
  	char envValue[80];
  	char file[80];
};

#endif

