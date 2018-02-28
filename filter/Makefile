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

objects = C_CFTime C_CFilterPlugin C_CMemBlockListHead FilterCommonFunction \
	C_CFileLock C_CMemBlock C_CMemFilter FilterPluginFactory

target         = filterplugin
target_version = 1.0.0 
target_libs    = newcommon psutil
target_libs_shell = 

include $(POCO_BASE)/build/rules/plug


