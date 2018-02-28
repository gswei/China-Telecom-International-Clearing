#
# Makefile
#
# The global Makefile for projects
# $Id: Makefile 1736 2012-10-26 01:51:20Z zhenghb $
# 

.PHONY: all binexecs  clean distclean install
#
all shared_debug shared_release static_debug static_release : binexecs

#
SUBDIRS= info offer shmadmin
#
.PHONY: binexecs $(SUBDIRS)
#
binexecs: $(SUBDIRS)

$(SUBDIRS):
	@echo "**building project $@..."
	$(MAKE) -C $@  $(MAKECMDGOALS)
#

install:
	@for comp in $(SUBDIRS) ; do \
		$(MAKE) -C $$comp install;\
	done

distclean:
	@for comp in $(SUBDIRS) ; do \
		echo "";\
		$(MAKE) -C $$comp distclean;\
	done
#
clean:
	@for comp in $(SUBDIRS) ; do \
		echo "";\
		$(MAKE) -C $$comp clean;\
	done
