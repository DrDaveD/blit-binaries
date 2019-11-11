#	Makefile -- for eyes

#	last edit:	89/09/09	D A Gwyn

#	SCCS ID:	@(#)Makefile	1.1

PRODUCT = eyes

#	Host support:

MAKEFIL	= Makefile
INS	= cp

#	DMD support and default rules:

DMD	= /usr/opt/630
DMDPROD	= ${PRODUCT}.m
DMDSRCS	= ${PRODUCT}.c
DMDOBJS	= ${PRODUCT}.mo
DMDDIR	= ${DMD}/lib/demolib
DMDPERM	= 664
DMDCC	= dmdcc
DFLAGS	= -O # -g
DLFLAGS =

.SUFFIXES:	.mo .m

.c~.mo:
	$(GET) $(GFLAGS) -p $< > $*.c
	$(DMDCC) $(DFLAGS) -c $*.c && mv $*.o $*.mo
	rm -f $*.c

.c.mo:
	$(DMDCC) $(DFLAGS) -c $< && mv $*.o $*.mo

.mo.m:
	$(DMDCC) -o $@ $(DFLAGS) $(DLFLAGS) $<

.c~.m:
	$(GET) $(GFLAGS) -p $< > $*.c
	$(DMDCC) -o $* $(DFLAGS) $(DLFLAGS) $*.c
	rm -f $*.c

.c.m:
	$(DMDCC) -o $@ $(DFLAGS) $(DLFLAGS) $<

#	Generic SCCS rule:

.DEFAULT:
	$(GET) $(GFLAGS) -p s.$@ > $@

all:		${DMDPROD}

print:		${MAKEFIL} ${DMDSRCS}
	@pr -f ${MAKEFIL} ${DMDSRCS}

lint:		${DMDSRCS}
	lint ${DMDSRCS} > ${PRODUCT}.lint

flow:		${DMDSRCS}
	cflow ${DMDSRCS} > ${PRODUCT}.flow

xref:		${DMDSRCS}
	cxref -c -s -w132 ${DMDSRCS} > ${PRODUCT}.xref

test:		${DMDPROD}
	@echo '${PRODUCT} must be tested by hand;'
	@echo 'Type "dmdld ${DMDPROD}" in a small layer.'

compare:	all test
	cmp -s ${DMDDIR}/${PRODUCT} ${DMDPROD}

install:	all test
	@if cmp -s ${DMDDIR}/${PRODUCT} ${DMDPROD}; \
	 then	echo ${PRODUCT} unchanged; \
	 else	${INS} ${DMDPROD} ${DMDDIR}/${PRODUCT}; \
		chmod ${DMDPERM} ${DMDDIR}/${PRODUCT} 2> /dev/null || :; \
		echo ${PRODUCT} installed; \
	 fi

clean:
	for i in ${DMDSRCS}; \
	do	if [ -f s.$$i ]; \
		then	if [ -f p.$$i ]; \
			then	echo "*** File \"$$i\" is being edited."; \
			else	rm -f $$i; \
			fi; \
		fi; \
	done
	-rm -f ${DMDOBJS} ${PRODUCT}.lint ${PRODUCT}.flow ${PRODUCT}.xref

clobber:	clean
	-rm -f ${DMDPROD}
