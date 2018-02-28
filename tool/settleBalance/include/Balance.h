#include<iostream>
#include<vector>
//#include<stdio.h>
#include "CF_Common.h"
#include "CF_CLogger.h"

#include "CfgParam.h"
#include<dirent.h>

//季度,一年共4个

class Balance
{
	private:
		char ratecycle[6+1];
		int  cyclePos[4];
		char erro_msg[2048];
		char sql[4096];

		DBConnection conn;

	public:
		Balance();
		~Balance();

		bool init(int argc,char** argv);
		
		void run();

};


