#
# Makefile
#
# $Id: collect.mk 1736 2012-10-26 01:51:20Z zhenghb $
#
# Makefile for db  project
#


include $(POCO_BASE)/build/rules/global
include $(POCO_BASE)/build/rules/oracle

INCDIR=$(SETTLEDIR)/src/shell/include
POCO_ADD_INCLUDE+=$(INCDIR) $(INCDIR)/public $(INCDIR)/public/Process $(INCDIR)/public/table \
                $(INCDIR)/public/stl_hash $(INCDIR)/public/rapidxml $(INCDIR)/business/psomc \
                $(INCDIR)/public/InfoData

objects = collect 

target         = pscollect
target_version = 1.0
target_libs    =PocoFoundation 
target_libs_shell = business public infodata process

include $(POCO_BASE)/build/rules/exec
