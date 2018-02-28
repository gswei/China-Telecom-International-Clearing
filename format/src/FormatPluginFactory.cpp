/****************************************************************
 ** filename: FormatPluginFactory.cpp
 ** module:
 ** created by: jiangjz
 ** email: jiangjuzhi@gmail.com
 ** create date: 2010-6-9
 ** version: 1.0.0
 ** company: Guangdong Eshore Tech Co.,Ltd.
 ** description:
 **
 *****************************************************************/

#include "FormatPluginFactory.h"

using namespace std;
using namespace pluginengine;
using namespace pluginengine::kernel;
using namespace zhjs;

//
::pluginengine::kernel::Factory* FormatPluginFactory::factoryInstance = NULL;

Factory* FormatPluginFactory::getInstance() {
	if (!factoryInstance)
		factoryInstance = new FormatPluginFactory();
	return factoryInstance;
}

FormatPluginFactory::FormatPluginFactory() {
	this->singletonPlugin = NULL;
	this->factoryName = "FormatPluginFactory";
	this->availablePlugins.push_back(std::string("Format"));
}

FormatPluginFactory::~FormatPluginFactory() {
	FormatPluginFactory::factoryInstance = NULL;
}

const std::string& FormatPluginFactory::name() {
	return this->factoryName;
}

Plugin* FormatPluginFactory::create(const std::string& name) {
	if (name == "Format") {
		try {
			if (!this->singletonPlugin)
				this->singletonPlugin = new FormatPlugin();
			return this->singletonPlugin;
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
		return NULL;
	}
}

std::vector<std::string> FormatPluginFactory::available() {
	return this->availablePlugins;
}

extern "C" std::string _PLUGIN_LOAD_FUNCTION_GET_VERSION_() {
	return std::string("1.0.0");
}

extern "C" std::string _PLUGIN_LOAD_FUNCTION_GET_NAME_() {
	return std::string("FormatPlugin");
}

extern "C" bool _PLUGIN_LOAD_FUNCTION_DO_REGISTER_(Engine* engine) {
	::pluginengine::kernel::Factory* factory =
			FormatPluginFactory::getInstance();
	engine->registerFactory("ALL", "ALL", std::string("Format"), factory);
	return true;
}
