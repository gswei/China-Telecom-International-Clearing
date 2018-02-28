#include "PluginLoader.h"

/////////////////////////////////////////////封装操作共享库的方法/////////////////////////////////////////////////
bool LibLoader::Load(const char *FileName)
{
	bool bRet;

	if (m_FileHandle != NULL)
		return false;
		
	m_FileHandle = dlopen(FileName, RTLD_NOW);
	bRet = (m_FileHandle != NULL);
	
	if (bRet)
		this->m_FileName = FileName;
	
	return bRet;
}

bool LibLoader::UnLoad()
{
	bool bRet=true;
	if ( m_FileHandle )
	{
		bRet =  (dlclose(m_FileHandle) == 0);
		if (bRet)
			m_FileHandle = 0;
	}
	return  bRet;
}

bool LibLoader::ResolveSymbol(const char *SymbolName, void **AddressRef)
{
	if ( (m_FileHandle == NULL) || 
		(SymbolName == NULL) || (*SymbolName == 0))
		return false;

	*AddressRef = dlsym(m_FileHandle, SymbolName);
	
	return (*AddressRef != NULL);
}

const char * LibLoader::GetError()
{
	const char *pErrorInfo = 0;
	
	pErrorInfo = dlerror();
	
	return pErrorInfo;
}

/////////////////////////////////////////业务创建代理////////////////////////////////////////

bool PluginProxy::Initialize(const char *LibFileName, const char *SymbolName)
{
	if (!m_Loader.Load(LibFileName))
	{
	        printf("1%s\n",GetError());
		return false;
	}
		
	if (!m_Loader.ResolveSymbol(SymbolName, &m_SymbolHandle)) 
        {        		
		printf("2%s\n",GetError());
		return false;
	}	

	return true;
}

BasePlugin * PluginProxy::LoadClass(const string &ClassName)
{
	if (m_SymbolHandle == NULL){
	        cout <<"m_SymbolHandle is null!"<<endl;
		return NULL;
	        }
	return ( (BasePlugin * (*)(const string &)) m_SymbolHandle)(ClassName);
}


PluginProxy::~PluginProxy()
{
	for (PluginTable::iterator it = m_PluginArray.begin(); it != m_PluginArray.end(); ++it)
	{
		//delete it->second;
	}
	
}


BasePlugin * PluginProxy::operator [](const char* ClassName)
{
	BasePlugin *Rule = 0;
	
	//先查找关联数组
	PluginTable::iterator it = m_PluginArray.find(ClassName);
	if (it != m_PluginArray.end())
		return it->second;
	
	//如不存在则尝试从dll中创建
	//modify by wulf at 20060327统一输出信息
	expTrace("Y", __FILE__, __LINE__, "begin loading the LoadClass():%s;", ClassName);
	//cout <<"begin loading the LoadClass();"<<ClassName<<endl;
	Rule = LoadClass(ClassName);
	if (Rule != NULL)
		m_PluginArray[ClassName] = Rule;
	
	return Rule;
}
