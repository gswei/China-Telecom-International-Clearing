# Default target
none :
	echo 'Error make command! Please input your target name:'
	echo 'make target'
        
exe : $(TARGET_OBJS)
	$(DEFAULT_BUILD) -L$(COMMONINCLUDEHOME) $(PLATFORM_LINK_FLAG) $(USER_LINK_FLAG) -o $(TARGET_NAME) $(TARGET_OBJS) $(COMMONLIB)
#       cp $(TARGET_NAME)  /home/myjstest/operator/bin/
        
lib : $(TARGET_OBJS)
	$(ARCHIVE)  $(ARCHIVE_FLAG) $(ARCHIVE_O_SYM) $(TARGET_NAME) $(TARGET_OBJS)

sl  : $(TARGET_OBJS)
	$(DEFAULT_BUILD) $(PLATFORM_LINK_FLAG) -b -o $(TARGET_NAME)  $(TARGET_OBJS) 

clear:
	rm *.o
lex.yy.o : $(LEX_TARGET_NAME).lex	
	lex $(LEX_TARGET_NAME).lex	
	$(LEX_COMPILER) $(PLATFORM_COMPILE_FLAG) $(USER_COMPILE_FLAG) -c lex.yy.c
y.tab.o : $(LEX_TARGET_NAME).y 
	yacc -d $(LEX_TARGET_NAME).y	
	$(YACC_COMPILER) $(PLATFORM_COMPILE_FLAG) $(USER_COMPILE_FLAG) -c y.tab.c

# Oracle precompile
include $(ORACLE_HOME)/precomp/lib/env_precomp.mk
#PROCPLSFLAGS= sqlcheck=full userid=$(USERID)
PROCINCLUDE=$(CPLUS_SYS_INCLUDE) include=. include=$(PRECOMPHOME)public include=$(RDBMSHOME)public include=$(RDBMSHOME)demo include=$(PLSQLHOME)public include=$(NETWORKHOME)public
PROCPPFLAGS= mode=ansi close_on_commit=NO code=cpp  cpp_suffix=cpp $(PROCINCLUDE)
#USERID=labs/labs
#OTTFLAGS=$(PCCFLAGS)
#CLIBS= $(TTLIBS_QA) $(LDLIBS)

# Default building setting
CXX_COMPILER=CC
C_COMPILER=cc
ARCHIVE=CC
ARCHIVE_FLAG=
ARCHIVE_O_SYM=-xar -o
COMPILE_DEBUG_FLAG= -g 
LINK_DEBUG_FLAG=$(COMPILE_DEBUG_FLAG)
COMPILE_THREAD_FLAG=-Kthread
LINK_THREAD_FLAG=$(COMPILE_THREAD_FLAG)
COMPILE_IOSTREAM_FLAG=
LINK_IOSTREAM_FLAG=$(COMPILE_IOSTREAM_FLAG)
COMPILE_TEMPLATE_FLAG=
LINK_TEMPLATE_FLAG=$(COMPILE_TEMPLATE_FLAG)
LEX=lex
LEX_FLAG=
YACC=yacc
YACC_FLAG=-d
LEX_COMPILER=$(C_COMPILER)
YACC_COMPILER=$(LEX_COMPILER)
STL_HOME=. 

PLATFORM_COMPILE_FLAG=
PLATFORM_LINK_FLAG=

USER_COMPILE_FLAG=
USER_LINK_FLAG=

#FAMILY
FAMILY=-D_VOC_POSON_
#FAMILY=-D_SHCJ_

#Version
VERSION=-D_VERSION_TY_

#-D_VERSION_TY_
#-D_VERSION_GZ_
#-D_VERSION_SZ_

#LIB_TYPE
LIB_TYPE=-D_LIB_64_

#-D_LIB_64_
#-D_LIB_32_

# Library settting
COMMONINCLUDEHOME=/mboss/home/zhjs/src/zhjs/share/oldlib/
COMMONLIBHOME=$(COMMONINCLUDEHOME)
COMMONLIB=/mboss/home/zhjs/src/zhjs/share/oldlib/common.a

# Specify platform config

# Compile specify
PROCPPINCLUDE=-I$(PRECOMPHOME)public -I$(RDBMSHOME)public -I$(RDBMSHOME)demo -I$(PLSQLHOME)public -I$(NETWORKHOME)public
INCLUDE=-I$(COMMONINCLUDEHOME) -I. $(PROCPPINCLUDE) -I$(STL_HOME)

#COMPILE_NORMAL=$(CXX_COMPILER) -g -c -xarch=v8 $(PLATFORM_COMPILE_FLAG) $(USER_COMPILE_FLAG) $(INCLUDE)  $(COMPILE_IOSTREAM_FLAG) $(COMPILE_TEMPLATE_FLAG) $(VERSION) $(MONTH_FLAG) $(LIB_TYPE)
COMPILE_NORMAL=$(CXX_COMPILER) $(COMPILE_DEBUG_FLAG) -c $(FAMILY) $(PLATFORM_COMPILE_FLAG) $(USER_COMPILE_FLAG) $(INCLUDE) $(COMPILE_IOSTREAM_FLAG) $(COMPILE_TEMPLATE_FLAG) $(VERSION) $(FAMILY) $(LIB_TYPE)
COMPILE_THREAD=$(COMPILE_NORMAL) $(COMPILE_THREAD_FLAG)

DEFAULT_COMPILE=$(COMPILE_NORMAL)
# Link specify
#BUILD_EXE=$(CXX_COMPILER) -xarch=v8  -L$(LIBHOME) $(CPPLDLIBS) $(LINK_IOSTREAM_FLAG) $(LINK_TEMPLATE_FLAG) -locci
BUILD_EXE=$(CXX_COMPILER) -L$(LIBHOME) $(CPPLDLIBS) $(LINK_IOSTREAM_FLAG) $(LINK_TEMPLATE_FLAG) -locci -lrt
BUILD_THREAD_EXE=$(BUILD_EXE) $(LINK_THREAD_FLAG)
DEFAULT_BUILD=$(BUILD_EXE)

# Achive specify
BUILD_LIB=$(ARCHIVE)

include /JSSH/src/app/project/commem/hp.mk

.SUFFIXES:
.SUFFIXES: .pc .c .o .typ .h .cpp

.pc.cpp:
	$(PROC) $(PROCPPFLAGS) iname=$*

.cpp.o:
	$(DEFAULT_COMPILE)  $*.cpp

.pc.o:
	$(PROC) $(PROCPPFLAGS) iname=$*
	$(DEFAULT_COMPILE)  $*.cpp
