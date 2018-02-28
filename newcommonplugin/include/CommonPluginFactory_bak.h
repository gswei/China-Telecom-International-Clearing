/****************************************************************
 ** filename: CommonPluginFactory.h
 ** module:
 ** created by: jiangjz
 ** email: jiangjuzhi@gmail.com
 ** create date: 2010-5-27
 ** version: 1.0.0
 ** company: Guangdong Eshore Tech Co.,Ltd.
 ** description:
 **
 *****************************************************************/

#ifndef COMMONPLUGINFACTORY_H_
#define COMMONPLUGINFACTORY_H_

#include <string>
#include <vector>
#include <iostream>

#include "CF_CPlugin.h"
#include "CF_Config.h"
#include "CF_CPluginPacket.h"
#include "CommonPlugin.h"
#include "Comquery.h"
#include "CallingRegular.h"
#include "CalledRegular.h"
#include "AnaCallDirect.h"
#include "CallProperty.h"
#include "CallModify.h"
#include "CallFee.h"
#include "RateCycle.h"
#include "JLDXAnaAttachId.h"
#include "JLDXAnaCpId.h"
#include "JLDXAnaDownChargeFlag.h"
#include "JLDXAnaUpRate.h"
#include "JLDXAnaUserProperty.h"
#include "CtjfAnaRoute.h"
#include "CtjfAnaDirection.h"
#include "SWLHAnaCalledDirect.h"
#include "FeeAnalyze.h"
#include "IsBalanceValid.h"
#include "ISMPSPCheck.h"
#include "ISMPAttachId.h"
#include "200SPAnaCalledNo.h"
#include "TfAnaCode.h"
#include "PickList.h"
#include "SjywAnaVPDN.h"

class BaseCommonPlugin;

namespace zhjs
{

class CommonPluginFactory: public ::pluginengine::kernel::Factory
{
public:
	static CommonPluginFactory* getInstance();
	static void dispose(BaseCommonPlugin* plugin);
private:
	CommonPluginFactory();
	void removePluginInstance(const std::string& pluginName);
public:
	::pluginengine::kernel::Plugin* create(const std::string& pluginName);
	std::vector<std::string> available();
	const std::string& name();
	~CommonPluginFactory();
private:
	std::vector<std::string> availablePlugins;
	std::string factoryName;
	static ::zhjs::CommonPluginFactory* factoryInstance;
	std::map<std::string, ::pluginengine::kernel::Plugin*> createdPluginMap;
};
}

#endif /* COMMONPLUGINFACTORY_H_ */
