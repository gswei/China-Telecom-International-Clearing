#
# Makefile
#
# $Id: Makefile 1736 2013-06-01 12:00:20Z hed $
#
# Makefile for format  project
#


include $(POCO_BASE)/build/rules/global
include $(POCO_BASE)/build/rules/oracle


INCDIR=$(SETTLEDIR)/src/shell/include
JSINCLUDEDIR=$(SETTLEDIR)/src/app/share
POCO_ADD_INCLUDE+=$(INCDIR1) $(INCDIR) $(INCDIR)/public $(INCDIR)/public/Process $(INCDIR)/public/table \
	$(INCDIR)/public/stl_hash $(INCDIR)/public/rapidxml $(INCDIR)/public/InfoData $(INCDIR)/business/psomc \
	$(SETTLEDIR)/src/shell/include/business/RTInfo $(JSINCLUDEDIR)/newcommonlib/include  $(DR_HOME)/include

#POCO_ADD_LIBRARY +=$(SETTLEDIR)/DRPT/lib64

objects = main FormatPlugin bill_statistic bill_statistic_filenametime bill_statistic_route CF_Comp_Exp formatcheck  voiceformat


target         = jsformat   
target_version = 1.0
target_libs    = newcommon  
target_libs_shell = public process infodata

#USERLIBS += -ldrapij

include $(POCO_BASE)/build/rules/exec
