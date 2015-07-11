# Process this file with automake to produce Makefile.in
#
# $Id: Makefile.am,v 1.1.1.1 2004/06/11 18:57:27 satoru-t Exp $
#

AUTOMAKE_OPTIONS = 1.4 no-dependencies

SUBDIRS = . docs man sary src tests debian

include_HEADERS = sary.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = sary.pc

EXTRA_DIST  =	sary.pc.in sary.spec.in sary.spec

##Bug in automake: Can't use `if MAINTAINER_MODE'
@MAINTAINER_MODE_TRUE@include $(srcdir)/maintMakefile

