#
# Makefile
#
# $Id: Makefile 1736 2012-03-01 vivi $
#
# Makefile for zhjs librateplugind.sl
#


include $(POCO_BASE)/build/rules/global
include $(POCO_BASE)/build/rules/oracle
INCLUDE+=-I$(SETTLEDIR)/src/shell/include -I$(SETTLEDIR)/src/zhjs/share/new_commonlib/include

POCO_FLAGS+=-DUTIL_DB_ONLY

objects = DataInOutMap ExpressCtl InputInfo RuleNo CDaylistPlugin \
	DayListPluginFactory FeeGroupMapList ListSpecialCtl TreeTool

target         = daylistplugin
target_version = 1.0.0 
target_libs    = newcommon psutil 
target_libs_shell = 

include $(POCO_BASE)/build/rules/plug


