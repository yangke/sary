#
# sample - Pick random N lines from a file.
#
use strict;
srand;

my $num = 10;
if (defined $ARGV[0] && $ARGV[0] =~ /^-(\d+)/) {
    $num = $1;
    shift @ARGV;
}

my @selected = ();
for (my $lineno = 1; <>; $lineno++) {
    my $rand = int(rand($lineno));
    if ($rand < $num) {
	push @selected,  $_;
	if (@selected > $num) {
	    splice @selected, $rand, 1;
	}
    }
}

print @selected;


