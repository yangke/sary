#include <assert.h>
#include <errno.h>
#include <sary.h>

static gint 
search (SarySearcher *searcher, gchar* pattern, gchar *start, gchar *end)
{
    gint freq = 0;

    if (sary_searcher_search(searcher, pattern)) {
        gchar *line;
        sary_searcher_sort_occurrences(searcher);
        while((line = sary_searcher_get_next_tagged_region(searcher,
                                                           start, end))) {
	    freq++;
            g_free(line);
        }
    }
    return freq;
}

int
main (int argc, char **argv)
{
    SarySearcher *searcher;
    gchar *file  = argv[1];

    searcher = sary_searcher_new(file);
    if(searcher == NULL){
        g_print("error: %s(.ary): %s\n", file, g_strerror(errno));
        return 1;
    }

    {
	/*
	 * They differ Sary 1.0.3 or older.
	 */
	gint x, y;
	x = search(searcher, "mmm",  "<p", "</p>");
	y = search(searcher, "mmm",  "<p", "</p");
	assert(x == y);
    }

    sary_searcher_destroy(searcher);
    return 0;
}
