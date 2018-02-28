#
# Makefile
#
# $Id: Makefile 1736 2012-10-26 01:51:20Z zhenghb $
#
# Makefile for Poco SAXParser
#

include $(POCO_BASE)/build/rules/global
include $(POCO_BASE)/build/rules/oracle
POCO_ADD_INCLUDE +=$(POCO_BASE)/../share/shmbase/include

objects = ShmAdmin

target         = $(PSBASEADMIN_EXE)
target_version = 1 
target_libs    = PocoXML PocoFoundation dbutil psutil esutil active shmbase

include $(POCO_BASE)/build/rules/exec
