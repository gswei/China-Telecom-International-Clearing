include ../mkfile/config2.0.mk 
all=infopoint#billing object objs

objs1 =Mainflow.o Infodeal.o CTimeCheck.o Infocom.o

all:$(all)

infopoint:$(objs1)
	$(MAKE) exe TARGET_NAME=$@ TARGET_OBJS="Mainflow.o Infodeal.o CTimeCheck.o Infocom.o"

clear:
	rm -f *.o

# Overwrite complie flag
DEFAULT_COMPILE=$(COMPILE_THREAD)
DEFAULT_BUILD=$(BUILD_THREAD_EXE)


