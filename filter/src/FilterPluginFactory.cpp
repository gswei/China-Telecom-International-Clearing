/****************************************************************
 ** filename: FilterPluginFactory.cpp
 ** module:
 ** created by: jiangjz
 ** email: jiangjuzhi@gmail.com
 ** create date: 2010-5-27
 ** version: 1.0.0
 ** company: Guangdong Eshore Tech Co.,Ltd.
 ** description:
 **
 *****************************************************************/

#include "FilterPluginFactory.h"

using namespace std;
using namespace pluginengine;
using namespace pluginengine::kernel;
using namespace zhjs;

//
::pluginengine::kernel::Factory* FilterPluginFactory::factoryInstance = NULL;

Factory* FilterPluginFactory::getInstance() {
	if (!factoryInstance)
		factoryInstance = new FilterPluginFactory();
	return factoryInstance;
}

FilterPluginFactory::FilterPluginFactory() {
	this->factoryName = "FilterPluginFactory";
	this->availablePlugins.push_back(std::string("Filter"));
}

FilterPluginFactory::~FilterPluginFactory() {
	FilterPluginFactory::factoryInstance = NULL;
}

const std::string& FilterPluginFactory::name() {
	return this->factoryName;
}

Plugin* FilterPluginFactory::create(const std::string& name) {
	if (name == "Filter") {
		try {
			return C_CFilterPlugin::getInstance();
		} catch (CException& ex) {
			ex.PushStack(5566,
					(std::string("creating plug-in of name:") + name).c_str(),
					__FILE__, __LINE__);
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

std::vector<std::string> FilterPluginFactory::available() {
	return this->availablePlugins;
}

extern "C" std::string _PLUGIN_LOAD_FUNCTION_GET_VERSION_() {
	return std::string("1.0.0");
}

extern "C" std::string _PLUGIN_LOAD_FUNCTION_GET_NAME_() {
	return std::string("FilterPlugin");
}

extern "C" bool _PLUGIN_LOAD_FUNCTION_DO_REGISTER_(Engine* engine) {
	::pluginengine::kernel::Factory* factory =
			FilterPluginFactory::getInstance();
	engine->registerFactory("ALL", "ALL", std::string("Filter"), factory);
	return true;
}
