/****************************************************************
 ** filename: BillRatePluginFactory.cpp
 ** module:
 ** created by: jiangjz
 ** email: jiangjuzhi@gmail.com
 ** create date: 2010-5-27
 ** version: 1.0.0
 ** company: Guangdong Eshore Tech Co.,Ltd.
 ** description:
 **
 *****************************************************************/

#include "BillRatePluginFactory.h"

using namespace std;
using namespace pluginengine;
using namespace pluginengine::kernel;
using namespace zhjs;

//
::pluginengine::kernel::Factory* BillRatePluginFactory::factoryInstance = NULL;

Factory* BillRatePluginFactory::getInstance() {
	if (!factoryInstance)
		factoryInstance = new BillRatePluginFactory();
	return factoryInstance;
}

BillRatePluginFactory::BillRatePluginFactory() {
	this->factoryName = "BillRatePluginFactory";
	this->availablePlugins.push_back(std::string("BillRate"));
}

BillRatePluginFactory::~BillRatePluginFactory() {
	BillRatePluginFactory::factoryInstance = NULL;
}

const std::string& BillRatePluginFactory::name() {
	return this->factoryName;
}

Plugin* BillRatePluginFactory::create(const std::string& name) {
	if (name == "BillRate") {
		try {
			Plugin* p = new BillRate();
			return p;
		} catch (jsexcp::CException& ex) {
			ex.PushStack(5566, (std::string("creating plug-in of name: ")
					+ name).c_str(), __FILE__, __LINE__);
			throw ex;
		} catch (const std::exception &e) {
			throw jsexcp::CException(5566, (std::string("creating plug-in of name: ")
					+ name + ", " + e.what()).c_str(), __FILE__, __LINE__);
		} catch (...) {
			throw jsexcp::CException(5566, (std::string("creating plug-in of name: ")
					+ name + ", unknown error").c_str(), __FILE__, __LINE__);
		}
	}
	return NULL;
}

std::vector<std::string> BillRatePluginFactory::available() {
	return this->availablePlugins;
}

extern "C" std::string _PLUGIN_LOAD_FUNCTION_GET_VERSION_() {
	return std::string("1.0.0");
}

extern "C" std::string _PLUGIN_LOAD_FUNCTION_GET_NAME_() {
	return std::string("BillRatePlugin");
}

extern "C" bool _PLUGIN_LOAD_FUNCTION_DO_REGISTER_(Engine* engine) {
	::pluginengine::kernel::Factory* factory =
			BillRatePluginFactory::getInstance();
	engine->registerFactory("ALL", "ALL", std::string("BillRate"), factory);
	return true;
}
