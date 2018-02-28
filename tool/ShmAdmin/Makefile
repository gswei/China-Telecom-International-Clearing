#
#Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
# All rights reserved.
#
# Created:     2012-9-12 IONg10:05:45
# Module:      ShmAdmin
# Author:      Zhaojing Qu
# Revision:    $Id: Makefile 2158 2012-11-09 07:50:14Z zhenghb $
# Description: Makefile for ShmAdmin
#

include $(POCO_BASE)/build/rules/global
include $(POCO_BASE)/build/rules/oracle
POCO_ADD_INCLUDE += $(SETTLEDIR)/src/app/share/shmbase/include

#BINPATH=./bin

objects = main

target         = $(PSBSHM_EXE)
target_version = 2
target_libs    = psutil active dbutil

include $(POCO_BASE)/build/rules/exec
