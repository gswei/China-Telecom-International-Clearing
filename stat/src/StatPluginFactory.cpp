/****************************************************************
 ** filename: StatPluginFactory
 ** module:
 ** created by: jiangjz
 ** email: jiangjuzhi@gmail.com
 ** create date: 2010-6-24
 ** version: 1.0.0
 ** company: Guangdong Eshore Tech Co.,Ltd.
 ** description:
 **
 *****************************************************************/

#include "StatPluginFactory.h"

using namespace std;
using namespace pluginengine;
using namespace pluginengine::kernel;
using namespace zhjs;
//
::pluginengine::kernel::Factory* StatPluginFactory::factoryInstance = NULL;

Factory* StatPluginFactory::getInstance() {
	if (!factoryInstance)
		factoryInstance = new StatPluginFactory();
	return factoryInstance;
}

StatPluginFactory::StatPluginFactory() {
	this->factoryName = "StatPluginFactory";
	this->availablePlugins.push_back(std::string("Stat"));
}

StatPluginFactory::~StatPluginFactory() {
	StatPluginFactory::factoryInstance = NULL;
}

const std::string& StatPluginFactory::name() {
	return this->factoryName;
}

Plugin* StatPluginFactory::create(const std::string& name) {
	if (name == "Stat") {
		try {
			Plugin* p = new CStatPlugin();
			return p;
		} catch (CException& ex) {
			ex.PushStack(5566, (std::string("creating plug-in of name: ")
					+ name).c_str(), __FILE__, __LINE__);
			throw ex;
		} catch (const std::exception &e) {
			throw CException(5566, (std::string("creating plug-in of name: ")
					+ name + ", " + e.what()).c_str(), __FILE__, __LINE__);
		} catch (...) {
			throw CException(5566, (std::string("creating plug-in of name: ")
					+ name + ", unknown error").c_str(), __FILE__, __LINE__);
		}
	}
	return NULL;
}

std::vector<std::string> StatPluginFactory::available() {
	return this->availablePlugins;
}

extern "C" std::string _PLUGIN_LOAD_FUNCTION_GET_VERSION_() {
	return std::string("1.0.0");
}

extern "C" std::string _PLUGIN_LOAD_FUNCTION_GET_NAME_() {
	return std::string("StatPlugin");
}

extern "C" bool _PLUGIN_LOAD_FUNCTION_DO_REGISTER_(Engine* engine) {
	::pluginengine::kernel::Factory* factory = StatPluginFactory::getInstance();
	engine->registerFactory("ALL", "ALL", std::string("Stat"), factory);
	return true;
}
