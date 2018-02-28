
/********************************************************************************************
**Class			: LibLoader
**Description		: ���������(.so�ļ�)�İ�����,�򿪡��رա�ȡ����		  
**Author		: 
**StartTime		: 
**Last Change Time 	: 

*******************************************************************************************/

#ifndef _PLUGINLOADER_H_
#define _PLUGINLOADER_H_

#include "Plugin.h"
//add by wulf at 20060327ͳһ�����Ϣ
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
**Description		: �ṩ����ҵ�����Ĵ���,client������ж����ѯ������ڴ����Ķ����򷵻�
			  ���������ʾ��̬����������Ӧ��ҵ���߼�,��LibLoader�������õĵ�ַ�ռ为�����Ĵ���	  
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
	
	//��ʼ���ö�����Ҫ������ļ���,�Լ����ж������ڴ���������
	bool Initialize(const char *LibFileName, const char *SymbolName);
	
	//�رչ��������
	inline bool CloseSLFile(){    
	    // unload the triangle library
       return m_Loader.UnLoad();
        }
	
	//clientͨ��ģ�������±������Ҫ��BusinessRule,���ܽ���left-value operate
	BasePlugin * operator[](const char* ClassName);
	
	//ȡ�Ŀ�Ĵ�����Ϣ
	inline const char *GetError() { return m_Loader.GetError(); }
};

#endif
