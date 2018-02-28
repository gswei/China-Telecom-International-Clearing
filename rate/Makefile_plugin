#
# Makefile
#
# $Id: Makefile 1736 2012-03-01 vivi $
#
# Makefile for zhjs librateplugind.sl
#


include $(POCO_BASE)/build/rules/global
include $(POCO_BASE)/build/rules/oracle
#INCLUDE+=-I$(SETTLEDIR)/src/shell/include -I$(SETTLEDIR)/src/zhjs/share/new_commonlib/include

INCDIR=$(SETTLEDIR)/src/shell/include
JSINCLUDEDIR=$(SETTLEDIR)/src/app/share
POCO_ADD_INCLUDE+=$(INCDIR1) $(INCDIR) $(INCDIR)/public $(INCDIR)/public/Process $(INCDIR)/public/table \
	$(INCDIR)/public/stl_hash $(INCDIR)/public/rapidxml $(INCDIR)/public/InfoData $(INCDIR)/business/psomc \
	$(JSINCLUDEDIR)/newcommonlib/include $(JSINCLUDEDIR)/newcommonmemory/include $(JSINCLUDEDIR)/newpluginengine/include \
	$(SETTLEDIR)/src/shell/include/business/RTInfo/ 

POCO_FLAGS+=-DUTIL_DB_ONLY

objects = CF_CMemManager C_BillRate C_CFileLock ChargeType \
	DealRecord IndexTree RateGroup RuleBlur RuleExact RuleStruct settlerule Formula BillRatePluginFactory

target         = rateplugin 
target_version = 1.0.0 
target_libs    = newcommon psutil
target_libs_shell = public process infodata
 
include $(POCO_BASE)/build/rules/plug
