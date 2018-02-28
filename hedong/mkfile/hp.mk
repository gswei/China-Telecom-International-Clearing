CXX_COMPILER=aCC
COMPILE_THREAD_FLAG=-lpthread

#choose target platform: _WINDOWS | _SUNOS | _HPOS | _UNIXWARE
#choose target compiler: _MS_VC | _SUN_CC | _HP_ACC | _UW_CC

PLATFORM_COMPILE_FLAG=-D_HPOS -D_HP_ACC -fast +DA2.0W -g
PLATFORM_LINK_FLAG=+DA2.0W -g
