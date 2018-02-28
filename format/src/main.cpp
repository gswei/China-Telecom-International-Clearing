
//2013-07-08 增加容灾的处理模式
//2013-07-16 当话单块有文件头时，需要将行号+1，输入文件备份时在目录中应该建立子目录YYYYMM/DD
//2013-07-19 文件序列号从数据库sequece中取得，保证唯一，日志路径级别等从核心参数中取，调用内存日志接口
			 //格式校验错误单独定义错误码，描述中写详细错误信息
//2013-07-26 头尾校验格式增加字段SPEC_FLAG，用来标志账务日期，需要将该日期写入每条记录配置到字段98,在发消息时设置
//2013-07-30  调度表增加file_id字段，给其他模块使用
//2013-08-01  C_FILE_RECEIVE_ENV表 增加字段截取时间,写入每条记录配置到字段99,source id对文件名上的时间做截取，放到sch表里，并且需要带到话单中。。若截取不到，则取系统时间。时间形式为YYYYMMDD
//2013-08-24新增容灾平台 sql更新写文件,调用父类接口实现
//2013-10-15 校验表新增错误码字段,自动程序自动读取该字段,
//			 但是预处理校验失败，生成异常说明文件上传的文件，如果上游重新下发 不能判重
//2013-10-17 备系统file_id由主系统传过去
//2013-10-24 修改账期取值方式,从核对结果详细表获取
//2013-10-27 将头尾记录字段值格式化到记录里面,弄成配置信息 input2ouput 头10开头,为90开头
//2013-11-27 修改写sql的方式,先直接写子类vecotor,当数据库只读才写父类vector写文件,类似程序jsload,jsFileInAudit
//2013-12-10 直接写insert,去除update sql语句
//2013-12-16 将容灾的函数调用接口封装到类中,便于统一处理
//2014-03-26 dealfile处理文件失败,记录错误状态
//2014-04-02 文件名重复也要登记调度表,解决日汇总不平衡问题
//2014-07-01 C_CYLCE_ADJ_DEFINE 增加封帐完成后该账期文件不被当错误文件(国内长途业务格式1)

#include "FormatPlugin.h"

//CDatabase DBConn;
//CLog theLog;
CLog theJSLog;
CReadIni theCfgFile;

int main(int argc,char** argv)
{

	/* 从环境变量文件中读取参数 */ 
	//CReadIni IniFile;
	//char* m_szEnvFile = "/mboss/home/zhjs/etc/zhjs/zhjs.ini" ;	
	//if(!IniFile.init(m_szEnvFile))
	//{
	//	cout<<"打开INI文件出错： "<<m_szEnvFile<<endl;
	//	return false;
	//}

	cout<<"********************************************** "<<endl;
	cout<<"*    China Telecom. Telephone Network          "<<endl;
	cout<<"*    InterNational Account Settle System       "<<endl;
	cout<<"*                                              "<<endl;
	cout<<"*           jsformat                           "<<endl;
	cout<<"*           sys.GJZW.Version 1.0	              "<<endl;
	cout<<"*    created time :     2013-06-01 by  hed 	  "<<endl;
	cout<<"*    last update time:  2014-07-01 by  hed	  "<<endl;
	cout<<"********************************************** "<<endl;

	FormatPlugin format ;
	if(!format.init(argc,argv)) return -1;
	//format.init("SERV1","CLYW1",1);
	//while(1)
	//{
		//theJSLog.reSetLog();
		format.run();
		//sleep(5);
	//}

    return 0;

}
