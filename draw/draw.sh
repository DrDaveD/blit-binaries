#!/usr/5bin/sh

#	draw -- download DMD "draw" program

#	last edit:	85/07/14	D A Gwyn

#	SCCS ID:	@(#)draw.sh	1.1

if [ "$DMD" = '' ]
then	DMD=/usr/local/DMD export DMD
fi

DMDLIB=/vld/lib/dmd

PATH="$DMD"/bin:"$PATH"

if ismpx -
then	exec 32ld $DMDLIB/draw.m
else	exec 32ld $DMDLIB/draw.j
fi
