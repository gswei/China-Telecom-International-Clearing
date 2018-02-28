#
# Makefile
#
# $Id: Makefile 1736 2012-10-26 01:51:20Z zhenghb $
#
# Makefile for db  project
#


IGNORE_DEFAULT_INCLUDE = 1

include $(POCO_BASE)/build/rules/global
include $(POCO_BASE)/build/rules/oracle

POCO_ADD_INCLUDE += $(SETTLEDIR)/src/app/share/zbshm/include $(SETTLEDIR)/src/app/share/db/include \
                    $(SETTLEDIR)/src/app/share/es/include

objects = busi_shm_mgr testBusiShm

target         = testBusiShm
target_version = 1.0
target_libs    = zbshm esutil


include $(POCO_BASE)/build/rules/exec
