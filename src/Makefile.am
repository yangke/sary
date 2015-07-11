# Process this file with automake to produce Makefile.in
#
# $Id: Makefile.am,v 1.1.1.1 2004/06/11 18:57:27 satoru-t Exp $
#

AUTOMAKE_OPTIONS = 1.4 no-dependencies

bin_PROGRAMS =	sary mksary

INCLUDES = @GLIB_CFLAGS@ -DG_LOG_DOMAIN=\"Sary\" -I$(top_srcdir)
LDADD    = $(top_builddir)/sary/libsary.la @GLIB_LIBS@

sary_SOURCES =		sary.c getopt.h getopt.c getopt1.c

mksary_SOURCES =	mksary.c getopt.h getopt.c getopt1.c

noinst_PROGRAMS =	isearch-test cache-test cat-test cat-test2\
			search-benchmark repeated-test multi-test

cache_test_SOURCES =		cache-test.c

cat_test_SOURCES =		cat-test.c

cat_test2_SOURCES =		cat-test2.c

isearch_test_SOURCES =		isearch-test.c

repeated_test_SOURCES =		repeated-test.c

search_benchmark_SOURCES =	search-benchmark.c \
				getopt.h getopt.c getopt1.c

multi_test_SOURCES =		multi-test.c


# Memory leak checking. It requires mpatrol 
# <http://www.cbmamiga.demon.co.uk/mpatrol/>
# LDFLAGS is set for Linux.
mleak-check:
	@echo -n "recompiling for memory leak checking: "
	@cd .. && make clean  >/dev/null 2>&1 && \
	make LDFLAGS='-lmpatrol -lbfd -liberty' >/dev/null 2>&1
	@echo done

	@echo foo > foo

	@echo -n "checking mksary: "

	@MPATROL_OPTIONS="LOGALL PROF USEDEBUG" ./mksary -b0 foo >/dev/null

	@# Ignore memory leaks caused by pthread_create.
	@perl -n00e 'print unless /pthread_create/' \
		mpatrol.log >mpatrol2.log	
	@mleak mpatrol2.log | grep -q 'unfreed allocations: 0 ' && echo ok

	@echo -n "checking sary: "

	@MPATROL_OPTIONS="LOGALL PROF USEDEBUG" ./sary foo foo > /dev/null

	@# Ignore memory leaks caused by setlocale.
	@perl -n00e 'print unless /setlocale/' \
		mpatrol.log >mpatrol2.log	
	@mleak mpatrol2.log | grep -q 'unfreed allocations: 0 ' && echo ok

	@rm -f foo foo.ary mpatrol.log mpatrol2.log

	@echo -n "recompiling for normal use: "
	@cd .. && make clean  >/dev/null 2>&1 && make >/dev/null 2>&1
	@echo done
