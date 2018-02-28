/****************************************************************
 ** filename: DayListPluginFactory
 ** module:
 ** created by: jiangjz
 ** email: jiangjuzhi@gmail.com
 ** create date: 2010-6-24
 ** version: 1.0.0
 ** company: Guangdong Eshore Tech Co.,Ltd.
 ** description:
 **
 *****************************************************************/

#include "DayListPluginFactory.h"

using namespace std;
using namespace pluginengine;
using namespace pluginengine::kernel;
using namespace zhjs;

//
::pluginengine::kernel::Factory* DayListPluginFactory::factoryInstance = NULL;

Factory* DayListPluginFactory::getInstance() {
	if (!factoryInstance)
		factoryInstance = new DayListPluginFactory();
	return factoryInstance;
}

DayListPluginFactory::DayListPluginFactory() {
	this->factoryName = "DayListPluginFactory";
	this->availablePlugins.push_back(std::string("DayList"));
}

DayListPluginFactory::~DayListPluginFactory() {
	DayListPluginFactory::factoryInstance = NULL;
}

const std::string& DayListPluginFactory::name() {
	return this->factoryName;
}

Plugin* DayListPluginFactory::create(const std::string& name) {
	if (name == "DayList") {
		try {
			Plugin* p = new CDaylistPlugin();
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

std::vector<std::string> DayListPluginFactory::available() {
	return this->availablePlugins;
}

extern "C" std::string _PLUGIN_LOAD_FUNCTION_GET_VERSION_() {
	return std::string("1.0.0");
}

extern "C" std::string _PLUGIN_LOAD_FUNCTION_GET_NAME_() {
	return std::string("DayListPlugin");
}

extern "C" bool _PLUGIN_LOAD_FUNCTION_DO_REGISTER_(Engine* engine) {
	::pluginengine::kernel::Factory* factory =
			DayListPluginFactory::getInstance();
	engine->registerFactory("ALL", "ALL", std::string("DayList"), factory);
	return true;
}
