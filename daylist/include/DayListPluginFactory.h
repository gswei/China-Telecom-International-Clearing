/****************************************************************
 ** filename: DayListPluginFactory.h
 ** module:
 ** created by: jiangjz
 ** email: jiangjuzhi@gmail.com
 ** create date: 2010-6-24
 ** version: 1.0.0
 ** company: Guangdong Eshore Tech Co.,Ltd.
 ** description:
 **
 *****************************************************************/

#ifndef DAYLISTPLUGINFACTORY_H_
#define DAYLISTPLUGINFACTORY_H_

#include <string>
#include <vector>
#include <iostream>

#include "CF_CPluginEngine.h"
#include "CF_CPlugin.h"
#include "CF_Config.h"
#include "CF_CPluginPacket.h"
#include "CF_CException.h"
#include "CDaylistPlugin.h"
#include "psutil.h"

namespace zhjs {

class DayListPluginFactory: public ::pluginengine::kernel::Factory {
public:
	static ::pluginengine::kernel::Factory* getInstance();
private:
	DayListPluginFactory();
	static ::pluginengine::kernel::Factory* factoryInstance;
public:
	::pluginengine::kernel::Plugin* create(const std::string& pluginName);
	std::vector<std::string> available();
	const std::string& name();
	~DayListPluginFactory();
public:
	std::vector<std::string> availablePlugins;
	std::string factoryName;
};

}

#endif /* DAYLISTPLUGINFACTORY_H_ */
