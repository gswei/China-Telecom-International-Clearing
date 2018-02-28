CXX_COMPILER=aCC
COMPILE_THREAD_FLAG=-mt -lpthread

#choose target platform: _WINDOWS | _SUNOS | _HPOS | _UNIXWARE | _LINUXOS
#choose target compiler: _MS_VC | _SUN_CC | _HP_ACC | _UW_CC

#PLATFORM_COMPILE_FLAG=-D_HPOS -D_HP_ACC +DA2.0W -AA -mt -lpthread# -D_RWSTD_MULTI_THREAD
PLATFORM_COMPILE_FLAG=-D_HPOS -D_HP_ACC -D__ia64=1 +DD64 -AA -mt -lpthread -w +u1
#PLATFORM_LINK_FLAG= +DA2.0W -AA -g 
PLATFORM_LINK_FLAG= +DD64 -AA -g -w

ARCHIVE = ar
ARCHIVE_FLAG = -r
ARCHIVE_O_SYM = 
