#
# Makefile
#
# The global Makefile for projects
# $Id: Makefile 2562 2012-11-23 08:51:30Z zhenghb $
# 
#

.PHONY: all binexecs  clean distclean install
#
all shared_debug shared_release static_debug static_release : binexecs

#
SUBDIRS= collect predeal feecalc bill pay expintf tool testTool petriTool

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
