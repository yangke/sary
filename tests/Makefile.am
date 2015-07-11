INCLUDES = @GLIB_CFLAGS@ -DG_LOG_DOMAIN=\"Sary\" -I$(top_srcdir)
LDADD    = @GLIB_LIBS@

TESTS =	sary-1 sary-2 sary-3 sary-4 sary-5 sary-6 sary-7 sary-8 sary-9\
	mksary-1 mksary-2 mksary-3 mksary-4 mksary-5 \
	array-1 cache-1 cat-1 cat-2 isearch-1 iso-8859-1 null-1 multi-1

TEST_CASES = 	eucjp.txt iso-8859-1.txt null.txt tagged.txt words.txt\
		repeated.txt

TEST_TOOLS = 	all-substrs.pl byte-indexer.pl gen-icase-data.pl \
		line-indexer.pl sample.pl word-indexer.pl

EXTRA_DIST = 	$(TESTS) $(TEST_CASES) $(TEST_TOOLS)

clean-local:
	rm -rf tmp.*

benchmark: benchmark-search benchmark-mksary

benchmark-search:
	@cp $(top_srcdir)/COPYING tmp.COPYING
	@$(top_srcdir)/src/mksary -q tmp.COPYING
	@for pat in GNU straightforwardly lesser e; do \
		echo PATTERN: $$pat; \
		$(top_srcdir)/src/search-benchmark -n 100000 \
							$$pat tmp.COPYING; \
		echo; \
	done

benchmark-mksary:
	@echo
	@rm -f tmp.garbage
	@target=$(PACKAGE)`date +"-%Y-%m-%d"` && \
	cd $(top_srcdir) && $(MAKE) dist distdir=$$target >/dev/null&& \
	gunzip $$target.tar.gz && \
	mv $$target.tar tests/tmp.garbage
	time $(top_srcdir)/src/mksary -q tmp.garbage
	@echo
	time $(top_srcdir)/src/mksary -q tmp.garbage
	@echo
