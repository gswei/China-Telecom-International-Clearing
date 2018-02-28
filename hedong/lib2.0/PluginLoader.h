
/********************************************************************************************
**Class			: LibLoader
**Description		: 操作共享库(.so文件)的帮助类,打开、关闭、取符号		  
**Author		: 
**StartTime		: 
**Last Change Time 	: 

*******************************************************************************************/

#ifndef _PLUGINLOADER_H_
#define _PLUGINLOADER_H_

#include "Plugin.h"
//add by wulf at 20060327统一输出信息
#include "Common.h"

#include <dlfcn.h>
#ifdef _HPOS
#include <symlink.h>
#else
#include <link.h>
#endif
#include <map>

class LibLoader
{
private:
	void *m_FileHandle;
	const char* m_FileName;
private:
	LibLoader(const LibLoader &);
	LibLoader &operator= (const LibLoader &);

public:
	explicit LibLoader() :m_FileHandle(0) {}
	~LibLoader() { UnLoad(); }
	
	bool Load(const char *FileName);
	bool UnLoad();
	
	bool ResolveSymbol(const char *SymbolName, void **AddressRef);
	
	const char * GetError();
	const char* GetLibName() { return m_FileName; }
};



/********************************************************************************************
**Class			: BusinessProxy
**Description		: 提供创建业务对象的代理,client对其进行对象查询如果存在创建的对象则返回
			  否则进行显示动态链接载入相应的业务逻辑,由LibLoader对象引用的地址空间负责对象的创建	  
**Author		: 
**StartTime		: 
**Last Change Time 	: 

*******************************************************************************************/
class PluginProxy
{
private:
	LibLoader m_Loader;
	void * m_SymbolHandle;
	typedef map<string, BasePlugin* > PluginTable;
	PluginTable m_PluginArray;
private:
	PluginProxy(const PluginProxy &);
	PluginProxy &operator= (const PluginProxy &);

private:
	BasePlugin * LoadClass(const string &ClassName);

public:
	explicit PluginProxy() : m_SymbolHandle(0) {}
	~PluginProxy();
	
	//初始化该对象需要共享库文件名,以及库中对象的入口创建函数名
	bool Initialize(const char *LibFileName, const char *SymbolName);
	
	//关闭共享库链接
	inline bool CloseSLFile(){    
	    // unload the triangle library
       return m_Loader.UnLoad();
        }
	
	//client通过模块名作下标访问需要的BusinessRule,不能进行left-value operate
	BasePlugin * operator[](const char* ClassName);
	
	//取的库的错误信息
	inline const char *GetError() { return m_Loader.GetError(); }
};

#endif
