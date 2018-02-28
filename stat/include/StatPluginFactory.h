/****************************************************************
 ** filename: StatPluginFactory.h
 ** module:
 ** created by: jiangjz
 ** email: jiangjuzhi@gmail.com
 ** create date: 2010-6-24
 ** version: 1.0.0
 ** company: Guangdong Eshore Tech Co.,Ltd.
 ** description:
 **
 *****************************************************************/

#ifndef STATPLUGINFACTORY_H_
#define STATPLUGINFACTORY_H_

#include <string>
#include <vector>
#include <iostream>

#include "CF_CPluginEngine.h"
#include "CF_CPlugin.h"
#include "CF_Config.h"
#include "CF_CPluginPacket.h"
#include "CStatPlugin.h"
//#include "CF_CStat.h"
#include "CF_CException.h"

namespace zhjs {

class StatPluginFactory: public ::pluginengine::kernel::Factory {
public:
	static ::pluginengine::kernel::Factory* getInstance();
private:
	StatPluginFactory();
	static ::pluginengine::kernel::Factory* factoryInstance;
public:
	::pluginengine::kernel::Plugin* create(const std::string& pluginName);
	std::vector<std::string> available();
	const std::string& name();
	~StatPluginFactory();
public:
	std::vector<std::string> availablePlugins;
	std::string factoryName;
};

}

#endif /* STATPLUGINFACTORY_H_ */
