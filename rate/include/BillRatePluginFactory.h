/****************************************************************
 ** filename: BillRatePluginFactory.h
 ** module:
 ** created by: jiangjz
 ** email: jiangjuzhi@gmail.com
 ** create date: 2010-5-27
 ** version: 1.0.0
 ** company: Guangdong Eshore Tech Co.,Ltd.
 ** description:
 **
 *****************************************************************/

#ifndef BILLRATEPLUGINFACTORY_H_
#define BILLRATEPLUGINFACTORY_H_

#include <string>
#include <vector>
#include <iostream>

#include "CF_CPluginEngine.h"
#include "CF_CPlugin.h"
#include "CF_Config.h"
#include "CF_CPluginPacket.h"
#include "C_BillRate.h"
#include "CF_CException.h"

namespace zhjs {

class BillRatePluginFactory: public ::pluginengine::kernel::Factory {
public:
	static ::pluginengine::kernel::Factory* getInstance();
private:
	BillRatePluginFactory();
	static ::pluginengine::kernel::Factory* factoryInstance;
public:
	::pluginengine::kernel::Plugin* create(const std::string& pluginName);
	std::vector<std::string> available();
	const std::string& name();
	~BillRatePluginFactory();
public:
	std::vector<std::string> availablePlugins;
	std::string factoryName;
};
}

#endif /* BILLRATEPLUGINFACTORY_H_ */
