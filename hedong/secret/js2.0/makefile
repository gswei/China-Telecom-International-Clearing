include ../../mkfile/config2.0.mk
all = sec2.0

#billing object objs

objs1 = main.o  

all:$(all)

sec2.0:$(objs1)
	$(MAKE) exe TARGET_NAME=$@ TARGET_OBJS="main.o"

clear:
	rm -f *.o

# Overwrite complie flag
DEFAULT_COMPILE=$(COMPILE_THREAD)
DEFAULT_BUILD=$(BUILD_THREAD_EXE)



