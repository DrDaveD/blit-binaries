#	Makefile -- for "crabs"

#	last edit:	89/02/25	D A Gwyn

#	SCCS ID:	@(#)Makefile	1.3

DMDDEMO	= $(DMD)/lib/demolib# $(DMD)/demo/layers for 5620
MCC	= $(DMD)/bin/dmdcc
MCFLAGS	= -O -g

all:		crabs

crabs:		crabs.c
	$(MCC) $(MCFLAGS) -o crabs crabs.c

install:	all
	cp crabs $(DMDDEMO)/crabs

clean:

clobber:	clean
	rm -f crabs
