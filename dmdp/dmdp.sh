#!/bin/sh

#	dmdp -- DMD printer program

#	last edit:	89/03/22	D A Gwyn

#	SCCS ID:	@(#)dmdp.sh	1.3

DMD=${DMD:=DmD}
export DMD
exec $DMD/bin/dmdld $DMD/lib/dmdp.m $*
