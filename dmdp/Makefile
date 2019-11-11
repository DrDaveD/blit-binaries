#	Makefile -- for dmdp

#	last edit:	89/03/22	D A Gwyn

#	SCCS ID:	@(#)Makefile	1.3

#	Host support:

PRODUCT = dmdp
MAKEFIL	= Makefile
SHFILES = ${PRODUCT}.sh
DMD	= /usr/local/630
BINDIR	= ${DMD}/bin
BINPERM	= 775
INS	= cp

#	DMD support and default rules:

DMDPROD	= ${PRODUCT}.m
DMDSRCS	= ${PRODUCT}.c
DMDOBJS	= ${PRODUCT}.mo
DMDDIR	= ${DMD}/lib
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

all:		${PRODUCT} ${DMDPROD}

${PRODUCT}:	${SHFILES}
	sed 's,DmD,$(DMD),g' $? > $@ && chmod ${BINPERM} $@

print:		${MAKEFIL} ${SHFILES} ${DMDSRCS}
	@pr -f ${MAKEFIL} ${SHFILES} ${DMDSRCS}

lint:		${DMDSRCS}
	lint ${DMDSRCS} > ${PRODUCT}.lint

flow:		${DMDSRCS}
	cflow ${DMDSRCS} > ${PRODUCT}.flow

xref:		${DMDSRCS}
	cxref -c -s -w132 ${DMDSRCS} > ${PRODUCT}.xref

test:		${PRODUCT}
	@echo ${PRODUCT} 'must be tested by hand'

compare:	all test
	cmp ${BINDIR}/${PRODUCT} ${PRODUCT}
	cmp ${DMDDIR}/${PRODUCT}.m ${PRODUCT}.m

install:	all test
	@if cmp -s ${BINDIR}/${PRODUCT} ${PRODUCT}; \
	 then	echo ${PRODUCT} unchanged; \
	 else	${INS} ${PRODUCT} ${BINDIR}; \
		echo ${PRODUCT} installed; \
	 fi
	-chmod ${BINPERM} ${BINDIR}/${PRODUCT}
	@if cmp -s ${DMDDIR}/${PRODUCT}.m ${PRODUCT}.m; \
	 then	echo ${PRODUCT}.m unchanged; \
	 else	${INS} ${PRODUCT}.m ${DMDDIR}; \
		echo ${PRODUCT}.m installed; \
	 fi
	-chmod ${DMDPERM} ${DMDDIR}/${PRODUCT}.m

clean:
#	if vax; then rm -f ${SHFILES} ${DMDSRCS}; fi
	-rm -f ${DMDOBJS} ${PRODUCT}.lint ${PRODUCT}.flow ${PRODUCT}.xref

clobber:	clean
	-rm -f ${PRODUCT} ${DMDPROD}
