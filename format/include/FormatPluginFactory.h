/****************************************************************
 ** filename: FormatPluginFactory.h
 ** module:
 ** created by: jiangjz
 ** email: jiangjuzhi@gmail.com
 ** create date: 2010-6-9
 ** version: 1.0.0
 ** company: Guangdong Eshore Tech Co.,Ltd.
 ** description:
 **
 *****************************************************************/

#ifndef FORMATPLUGINFACTORY_H_
#define FORMATPLUGINFACTORY_H_

#include <string>
#include <vector>
#include <iostream>

#include "CF_CPluginEngine.h"
#include "CF_CPlugin.h"
#include "CF_Config.h"
#include "CF_CPluginPacket.h"
#include "FormatPlugin.h"

namespace zhjs {

class FormatPluginFactory: public ::pluginengine::kernel::Factory {
public:
	static ::pluginengine::kernel::Factory* getInstance();
private:
	FormatPluginFactory();
	static ::pluginengine::kernel::Factory* factoryInstance;
public:
	::pluginengine::kernel::Plugin* create(const std::string& pluginName);
	std::vector<std::string> available();
	const std::string& name();
	~FormatPluginFactory();
public:
	::pluginengine::kernel::Plugin* singletonPlugin;
	std::vector<std::string> availablePlugins;
	std::string factoryName;
};
}

#endif /* FORMATPLUGINFACTORY_H_ */
