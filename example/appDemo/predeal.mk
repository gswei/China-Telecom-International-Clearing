#
# Makefile
#
# $Id: predeal.mk 2015 2012-11-03 08:48:53Z zhenghb $
#
# Makefile for db  project
#


include $(POCO_BASE)/build/rules/global
include $(POCO_BASE)/build/rules/oracle

INCDIR=$(SETTLEDIR)/src/shell/include
POCO_ADD_INCLUDE+=$(INCDIR) $(INCDIR)/public $(INCDIR)/public/Process $(INCDIR)/public/table \
                $(INCDIR)/public/stl_hash $(INCDIR)/public/rapidxml $(INCDIR)/public/InfoData $(INCDIR)/business/psomc

objects = predeal 

target         = predealDemo
target_version = 1.0
target_libs    = dbutil PocoFoundation esutil psutil stringparser
target_libs_shell = business public infodata process

include $(POCO_BASE)/build/rules/exec
