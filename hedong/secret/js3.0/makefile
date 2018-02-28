include ../../mkfile/config.mk
all = sec3.0

#billing object objs

objs1 = main.o 

all:$(all)

sec3.0:$(objs1)
	$(MAKE) exe TARGET_NAME=$@ TARGET_OBJS="main.o"

clear:
	rm -f *.o

# Overwrite complie flag
DEFAULT_COMPILE=$(COMPILE_THREAD)
DEFAULT_BUILD=$(BUILD_THREAD_EXE)
