/****************************************************************
 ** filename: CommonPluginFactory.cpp
 ** module:
 ** created by: jiangjz
 ** email: jiangjuzhi@gmail.com
 ** create date: 2010-5-27
 ** version: 1.0.0
 ** company: Guangdong Eshore Tech Co.,Ltd.
 ** description:
 **
 *****************************************************************/

#include "CommonPluginFactory.h"

using namespace std;
using namespace pluginengine;
using namespace pluginengine::kernel;
using namespace zhjs;

//
::zhjs::CommonPluginFactory* CommonPluginFactory::factoryInstance = NULL;

CommonPluginFactory* CommonPluginFactory::getInstance() {
	if (!factoryInstance)
		factoryInstance = new CommonPluginFactory();
	return factoryInstance;
}

void CommonPluginFactory::dispose(BaseCommonPlugin* plugin) {
	if (!plugin)
		return;
	const std::string& name = plugin->getPluginName();
	CommonPluginFactory * f = CommonPluginFactory::getInstance();
	if (!f)
		throw jsexcp::CException(223344, "get factory instance failed", __FILE__,
				__LINE__);
	f->removePluginInstance(name);
}

CommonPluginFactory::CommonPluginFactory() {
	this->factoryName = "CommonPluginFactory";
	this->availablePlugins.push_back("SubString");
	this->availablePlugins.push_back("Output");
	this->availablePlugins.push_back("Length");
	this->availablePlugins.push_back("Case");
	this->availablePlugins.push_back("Connect");
	this->availablePlugins.push_back("Cut");
	this->availablePlugins.push_back("Fill");
	this->availablePlugins.push_back("Change");
	this->availablePlugins.push_back("Sum");
	this->availablePlugins.push_back("HhMmSsTt2S");
	this->availablePlugins.push_back("Minus");
	this->availablePlugins.push_back("Multiple");
	this->availablePlugins.push_back("Divide");
	this->availablePlugins.push_back("TimeOperator");
	this->availablePlugins.push_back("IsDigit");
	this->availablePlugins.push_back("StringReplace");
	this->availablePlugins.push_back("TimeCalculate");
	this->availablePlugins.push_back("LackInfo");
	this->availablePlugins.push_back("Abnormal");
	this->availablePlugins.push_back("Classify");
	this->availablePlugins.push_back("Comquery");
	this->availablePlugins.push_back("StdChkCallingNbr");
	this->availablePlugins.push_back("StdChkCalledNbr");
	this->availablePlugins.push_back("AnaCallDirect");
	this->availablePlugins.push_back("AnaNbrProperty");
	this->availablePlugins.push_back("ModifyCallNumber");
	this->availablePlugins.push_back("RateCycle");
	this->availablePlugins.push_back("JLDXAnaAttachId");
	this->availablePlugins.push_back("JLDXAnaCpId");
	this->availablePlugins.push_back("JLDXAnaDownChargeFlag");
	this->availablePlugins.push_back("JLDXAnaUpRate");
	this->availablePlugins.push_back("JLDXAnaUserProperty");
	this->availablePlugins.push_back("CTJFAnaRoute");
	this->availablePlugins.push_back("CTJFAnaDirection");
	this->availablePlugins.push_back("SWLHAnaCalledDirect");
	this->availablePlugins.push_back("IsFreeNbr");
	this->availablePlugins.push_back("IsBalanceValid");
	this->availablePlugins.push_back("PreFeeAnalyze");
	this->availablePlugins.push_back("ISMPSPCheck");
	this->availablePlugins.push_back("ISMPAnaAttachId");
	this->availablePlugins.push_back("AnaCalledNo");
	this->availablePlugins.push_back("SpAnaCode");
	this->availablePlugins.push_back("TimeFormat");
	this->availablePlugins.push_back("PickList");
        this->availablePlugins.push_back("SjywAnaVPDN");
        this->availablePlugins.push_back("IsXDigit");
        this->availablePlugins.push_back("MinusTimes");
	this->availablePlugins.push_back("ExpCal");
}

CommonPluginFactory::~CommonPluginFactory() {
	CommonPluginFactory::factoryInstance = NULL;
}

const std::string& CommonPluginFactory::name() {
	return this->factoryName;
}

Plugin* CommonPluginFactory::create(const std::string& name) {
	std::map<std::string, ::pluginengine::kernel::Plugin*>::iterator it =
			this->createdPluginMap.find(name);
	if (it != this->createdPluginMap.end())
		return it->second;
	::pluginengine::kernel::Plugin* newPlugin = NULL;
	try {
		if (name == "SubString") {
			newPlugin = new SubString();
		} else if (name == "Output") {
			newPlugin = new Output();
		} else if (name == "Length") {
			newPlugin = new CLength();
		} else if (name == "Case") {
			newPlugin = new CCase();
		} else if (name == "Connect") {
			newPlugin = new CConnect();
		} else if (name == "Cut") {
			newPlugin = new Cut();
		} else if (name == "Fill") {
			newPlugin = new Fill();
		} else if (name == "Change") {
			newPlugin = new Change();
		} else if (name == "Sum") {
			newPlugin = new Sum();
		} else if (name == "HhMmSsTt2S") {
			newPlugin = new C_HhMmSsTt2S();
		} else if (name == "Minus") {
			newPlugin = new C_Minus();
		} else if (name == "Multiple") {
			newPlugin = new C_Multiple();
		} else if (name == "Divide") {
			newPlugin = new C_Divide();
		} else if (name == "TimeOperator") {
			newPlugin = new C_TimeOperator();
		} else if (name == "IsDigit") {
			newPlugin = new C_IsDigit();
		} else if (name == "StringReplace") {
			newPlugin = new C_StringReplace();
		} else if (name == "TimeCalculate") {
			newPlugin = new C_TimeCalculate();
		} else if (name == "LackInfo") {
			newPlugin = new LackInfo();
		} else if (name == "Abnormal") {
			newPlugin = new Abnormal();
		} else if (name == "Classify") {
			newPlugin = new Classify();
		} else if (name == "Comquery") {
			newPlugin = new C_Comquery();
		} else if (name == "StdChkCallingNbr") {
			newPlugin = new C_CallingRegular();
		} else if (name == "StdChkCalledNbr") {
			newPlugin = new C_CalledRegular();
		} else if (name == "AnaCallDirect") {
			newPlugin = new C_AnaCallDirect();
		} else if (name == "AnaNbrProperty") {
			newPlugin = new C_CallProperty();
		} else if (name == "ModifyCallNumber") {
			newPlugin = new C_CallModify();
		} else if (name == "IsFreeNbr") {
			newPlugin = new C_CallFee();
		} else if (name == "RateCycle") {
			newPlugin = new C_RateCycle();
		} else if (name == "JLDXAnaAttachId") {
			newPlugin = new C_JLDXAnaAttachId();
		} else if (name == "JLDXAnaCpId") {
			newPlugin = new C_JLDXAnaCpId();
		} else if (name == "JLDXAnaDownChargeFlag") {
			newPlugin = new C_JLDXAnaDownChargeFlag();
		} else if (name == "JLDXAnaUpRate") {
			newPlugin = new C_JLDXAnaUpRate();
		} else if (name == "JLDXAnaUserProperty") {
			newPlugin = new C_JLDXAnaUserProperty();
		} else if (name == "CTJFAnaRoute") {
			newPlugin = new C_CtjfAnaRoute();
		} else if (name == "CTJFAnaDirection") {
			newPlugin = new C_CtjfAnaDirection();
		} else if (name == "SWLHAnaCalledDirect") {
			newPlugin = new C_SWLHAnaCalledDirect();
		} else if (name == "PreFeeAnalyze") {
			newPlugin = new C_FeeAnalyze();
		} else if (name == "IsBalanceValid") {
			newPlugin = new C_IsBalanceValid();
		} else if (name == "ISMPSPCheck") {
			newPlugin = new C_ISMPSPCheck();
		} else if (name == "ISMPAnaAttachId") {
			newPlugin = new C_ISMPAttachId();
                } else if (name == "AnaCalledNo") {
                        newPlugin = new C_200SPAnaCalledNo();
                } else if (name == "SpAnaCode") {
                        newPlugin = new C_TfAnaCode();
		} else if (name == "TimeFormat") {
			newPlugin = new TimeFormat();
		} else if (name == "PickList") {
			newPlugin = new C_PickList();
                } else if (name == "SjywAnaVPDN") {
                        newPlugin = new C_SjywAnaVPDN();
                } else if (name == "IsXDigit") {
                        newPlugin = new C_IsXDigit();
                } else if (name == "MinusTimes") {
                        newPlugin = new C_MinusTime();
		}  else if (name == "ExpCal") {

                        newPlugin = new CExpCal();

		}else {
			return NULL;
		}
		this->createdPluginMap.insert(std::map<std::string,
				::pluginengine::kernel::Plugin*>::value_type(name, newPlugin));
		return newPlugin;
	} catch (jsexcp::CException& ex) {
		ex.PushStack(5566,
				(std::string("creating plug-in of name: ") + name).c_str(),
				__FILE__, __LINE__);
		throw ex;
	} catch (const std::exception &e) {
		throw jsexcp::CException(5566, (std::string("creating plug-in of name: ")
				+ name + ", " + e.what()).c_str(), __FILE__, __LINE__);
	} catch (...) {
		throw jsexcp::CException(5566, (std::string("creating plug-in of name: ")
				+ name + ", unknown error").c_str(), __FILE__, __LINE__);
	}
}

void CommonPluginFactory::removePluginInstance(const std::string& name) {
	std::map<std::string, ::pluginengine::kernel::Plugin*>::iterator it =
			this->createdPluginMap.find(name);
	if (it != this->createdPluginMap.end())
		this->createdPluginMap.erase(it);
}

std::vector<std::string> CommonPluginFactory::available() {
	return this->availablePlugins;
}

extern "C" std::string _PLUGIN_LOAD_FUNCTION_GET_VERSION_() {
	return std::string("1.0.0");
}

extern "C" std::string _PLUGIN_LOAD_FUNCTION_GET_NAME_() {
	return std::string("CommonPlugin");
}

extern "C" bool _PLUGIN_LOAD_FUNCTION_DO_REGISTER_(Engine* engine) {
	::pluginengine::kernel::Factory* factory =
			CommonPluginFactory::getInstance();
	bool result = true;
	std::vector<std::string> availables = factory->available();
	for (size_t i = 0; i < availables.size(); i++) {
		result = engine->registerFactory("ALL", "ALL", availables[i], factory);
		if (!result) {
			// What to do?
		}
	}
	return true;
}
