#	Makefile -- for draw

#	last edit:	85/07/15	D A Gwyn

#	SCCS ID:	@(#)Makefile	1.1

PRODUCT = draw
MAKEFIL	= Makefile
SHFILES = ${PRODUCT}.sh
CFILES	= ${PRODUCT}.c
OBJS	= ${PRODUCT}.o
BINDIR	= /vld/bin
LIBDIR	= /vld/lib
BINPERM	= 775
INS	= cp

#	DMD support and default rules:

DMDPROD	= ${PRODUCT}.m ${PRODUCT}.j
DMDOBJS	= ${PRODUCT}.mo ${PRODUCT}.jo
DMDDIR	= ${LIBDIR}/dmd
DMDPERM	= 664
DMDAR	= m32ar
DMDCC	= dmdcc
DFLAGS	= -O -g
DLFLAGS = -g
DLIBES	=	# -lmj

.SUFFIXES:	.mo .ma .m .jo .ja .j

.c~.ma:
	$(GET) $(GFLAGS) -p $< > $*.c
	$(DMDCC) -c $(DFLAGS) $*.c
	$(DMDAR) rv $@ $*.o
	rm -f $*.[co]

.c.ma:
	$(DMDCC) -c $(DFLAGS) $<
	$(DMDAR) rv $@ $*.o
	rm -f $*.o

.c~.mo:
	$(GET) $(GFLAGS) -p $< > $*.c
	$(DMDCC) $(DFLAGS) -c $*.c && mv $*.o $*.mo
	rm -f $*.c

.c.mo:
	$(DMDCC) $(DFLAGS) -c $< && mv $*.o $*.mo

.mo.m:
	$(DMDCC) -o $@ $(DFLAGS) $(DLFLAGS) $< ${DLIBES}

.c~.m:
	$(GET) $(GFLAGS) -p $< > $*.c
	$(DMDCC) -o $* $(DFLAGS) $(DLFLAGS) $*.c ${DLIBES}
	rm -f $*.c

.c.m:
	$(DMDCC) -o $@ $(DFLAGS) $(DLFLAGS) $< ${DLIBES}

.c~.ja:
	$(GET) $(GFLAGS) -p $< > $*.c
	$(DMDCC) -J -c $(DFLAGS) $*.c
	$(DMDAR) rv $@ $*.o
	rm -f $*.[co]

.c.ja:
	$(DMDCC) -J -c $(DFLAGS) $<
	$(DMDAR) rv $@ $*.o
	rm -f $*.o

.c~.jo:
	$(GET) $(GFLAGS) -p $< > $*.c
	$(DMDCC) -J $(DFLAGS) -c $*.c && mv $*.o $*.jo
	rm -f $*.c

.c.jo:
	$(DMDCC) -J $(DFLAGS) -c $< && mv $*.o $*.jo

.jo.j:
	$(DMDCC) -o $@ -J $(DFLAGS) $(DLFLAGS) $< ${DLIBES}

.c~.j:
	$(GET) $(GFLAGS) -p $< > $*.c
	$(DMDCC) -o $* -J $(DFLAGS) $(DLFLAGS) $*.c ${DLIBES}
	-rm -f $*.c

.c.j:
	$(DMDCC) -o $@ -J $(DFLAGS) $(DLFLAGS) $< ${DLIBES}

#	Generic SCCS rule:

.DEFAULT:
	$(GET) $(GFLAGS) -p s.$@ > $@

all:		${PRODUCT} ${DMDPROD}

${PRODUCT}:	${PRODUCT}.sh
	cp $? $@ && chmod ${BINPERM} $@

print:		${MAKEFIL} ${SHFILES} ${CFILES}
	( pr ${MAKEFIL} ${SHFILES} ${CFILES} ) | lpr

lint:		${CFILES}
#	lint ${CFILES} ${LLIBES} > ${PRODUCT}.lint

flow:		${CFILES}
#	cflow ${CFILES} > ${PRODUCT}.flow

xref:		${CFILES}
#	cxref -c -s -w132 ${CFILES} > ${PRODUCT}.xref

test:		${PRODUCT}
	@echo 'Test draw manually by "32ld draw.[mj]"'

compare:	all test
	cmp ${BINDIR}/${PRODUCT} ${PRODUCT}
	cmp ${DMDDIR}/${PRODUCT}.m ${PRODUCT}.m
	cmp ${DMDDIR}/${PRODUCT}.j ${PRODUCT}.j

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
		chmod ${DMDPERM} ${DMDDIR}/${PRODUCT}.m; \
		echo ${PRODUCT}.m installed; \
	 fi
	-chmod ${DMDPERM} ${DMDDIR}/${PRODUCT}.m
	@if cmp -s ${DMDDIR}/${PRODUCT}.j ${PRODUCT}.j; \
	 then	echo ${PRODUCT}.j unchanged; \
	 else	${INS} ${PRODUCT}.j ${DMDDIR}; \
		chmod ${DMDPERM} ${DMDDIR}/${PRODUCT}.j; \
		echo ${PRODUCT}.j installed; \
	 fi
	-chmod ${DMDPERM} ${DMDDIR}/${PRODUCT}.j

clean:
	if vax; then rm -f ${SHFILES} ${CFILES}; fi
	-rm -f ${OBJS} ${PRODUCT}.lint ${PRODUCT}.flow \
		${PRODUCT}.xref ${PRODUCT}.out ${PRODUCT}.err
	-rm -f ${DMDOBJS}

clobber:	clean
	-rm -f ${PRODUCT}
	-rm -f ${DMDPROD}
