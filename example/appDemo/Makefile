all:collect predeal realcalc bill

bill:
	gmake -f bill.mk
collect:
	gmake -f collect.mk
predeal:
	gmake -f predeal.mk 
realcalc:
	gmake -f realcalc.mk

install:
	gmake -f bill.mk install
	gmake -f predeal.mk install
	gmake -f realcalc.mk install
