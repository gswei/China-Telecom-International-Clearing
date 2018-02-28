#
# Makefile
#
# $Id: Makefile 1736 2013-03-07 12:00:20Z hed $
#
# Makefile for writefile  project
#


include $(POCO_BASE)/build/rules/global
include $(POCO_BASE)/build/rules/oracle


INCDIR=$(SETTLEDIR)/src/shell/include
JSINCLUDEDIR=$(SETTLEDIR)/src/app/share
POCO_ADD_INCLUDE+=$(INCDIR1) $(INCDIR) $(INCDIR)/public $(INCDIR)/public/Process $(INCDIR)/public/table \
	$(INCDIR)/public/stl_hash $(INCDIR)/public/rapidxml $(INCDIR)/public/InfoData $(INCDIR)/business/psomc \
	$(JSINCLUDEDIR)/newcommonlib/include  \
	
objects = writeFile


target         = jswritefile 
target_version = 1.0
target_libs    = newcommon 
target_libs_shell = public process  


include $(POCO_BASE)/build/rules/exec
