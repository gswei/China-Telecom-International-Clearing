#
# Makefile
#
# $Id: Makefile 1736 2013-08-02 12:00:20Z hed $
#
# Makefile for sum  project
#


include $(POCO_BASE)/build/rules/global
include $(POCO_BASE)/build/rules/oracle


INCDIR=$(SETTLEDIR)/src/shell/include
JSINCLUDEDIR=$(SETTLEDIR)/src/app/share
POCO_ADD_INCLUDE+=$(INCDIR) $(INCDIR1) $(INCDIR)/public $(INCDIR)/public/Process $(INCDIR)/public/table \
	$(INCDIR)/public/stl_hash $(INCDIR)/public/rapidxml $(INCDIR)/public/InfoData $(INCDIR)/business/psomc \
	$(JSINCLUDEDIR)/newcommonlib/include $(SETTLEDIR)/src/shell/include/business/RTInfo $(DR_HOME)/include

#POCO_ADD_LIBRARY +=$(SETTLEDIR)/DRPT/lib64

objects = Common CMsum CDsum CShare  main   

target         = jsSummary 
target_version = 1.0
target_libs    = newcommon 
target_libs_shell = public process infodata 

#USERLIBS += -ldrapij

include $(POCO_BASE)/build/rules/exec
