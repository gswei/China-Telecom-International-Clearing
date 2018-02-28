#
# $Id: Makefile 1736 2012-10-26 01:51:20Z zhenghb $
#
# Makefile for db  project
#
include $(POCO_BASE)/build/rules/global
include $(POCO_BASE)/build/rules/oracle

INCDIR=$(SETTLEDIR)/src/shell/include
JSINCLUDEDIR=$(SETTLEDIR)/src/app/share
POCO_ADD_INCLUDE+=$(INCDIR1) $(INCDIR) $(INCDIR)/public $(INCDIR)/public/Process $(INCDIR)/public/table \
	$(INCDIR)/public/stl_hash $(INCDIR)/public/rapidxml $(INCDIR)/public/InfoData \
	$(JSINCLUDEDIR)/newcommonlib/include 

objects = ShellCrontab paramConfigParse

target         = psShellCrontab
target_version = 1.0
target_libs    = newcommon dbutil util psutil PocoFoundation PocoUtil PocoXML
#target_libs_shell = public process infodata

include $(POCO_BASE)/build/rules/exec
