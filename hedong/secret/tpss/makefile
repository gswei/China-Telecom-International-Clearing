include ../../mkfile/config.mk
all = cipher

#billing object objs

objs1 = main.o cipher.o

all:$(all)

cipher:$(objs1)
	$(MAKE) exe TARGET_NAME=$@ TARGET_OBJS="main.o cipher.o"

clear:
	rm -f *.o

# Overwrite complie flag
DEFAULT_COMPILE=$(COMPILE_THREAD)
DEFAULT_BUILD=$(BUILD_THREAD_EXE)



