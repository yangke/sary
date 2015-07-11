#
# Generate all substrings. Number of sutrings = n * (n + 1) / 2 
#
#   % echo abcd | perl all-substrs.pl
#   abcd
#   abc
#   ab
#   a
#   bcd
#   bc
#   b
#   cd
#   c
#   d

require 5.004;

$text = join "", <>;
chomp $text;

for $suffix (gen_suffixes($text)) {
    for $prefix (gen_prefixes($suffix)) {
        print $prefix, "\n";
    }
}

sub gen_suffixes() {
    my ($text) = @_;
    my @suffixes = ();

    do {
	push @suffixes, $text;
    } while ($text =~ s/^.(.)/$1/);

    return @suffixes;
}

sub gen_prefixes() {
    my ($text) = @_;
    my @prefixes = ();

    do {
	push @prefixes, $text;
    } while ($text =~ s/(.).$/$1/);

    return @prefixes;
}


