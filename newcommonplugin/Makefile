#
# Makefile
#
# $Id: Makefile 1736 2012-03-01 vivi $
#
# Makefile for zhjs libcommonplugind.sl
#


include $(POCO_BASE)/build/rules/global
include $(POCO_BASE)/build/rules/oracle
INCLUDE+=-I$(SETTLEDIR)/src/shell/include -I$(SETTLEDIR)/src/app/share/newcommonlib/include

POCO_FLAGS+=-DUTIL_DB_ONLY

objects = 200SPAnaCalledNo  TfAnaCode  ISMPAttachId   ISMPSPCheck   IsBalanceValid   FeeAnalyze  \
         SWLHAnaCalledDirect   CtjfAnaRoute   CtjfAnaDirection   JLDXAnaUpRate   JLDXAnaDownChargeFlag  \
        JLDXAnaAttachId   JLDXAnaCpId   JLDXAnaUserProperty   RateCycle   CallFee   CallModify   \
        CallProperty   CallNumber   CallingRegular   CalledRegular   AnaCallDirect   ComFunction  \
        CommonPlugin   CommonPluginFactory   Comquery   SjywAnaVPDN   PickList  CExpCal

target         = newcommonplugin
target_version = 1.0.0 
target_libs    = newcommon psutil
target_libs_shell = 

include $(POCO_BASE)/build/rules/plug


