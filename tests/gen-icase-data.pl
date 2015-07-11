#
#  % echo foo | perl gen-icase-data.pl      
#  foo
#  foO
#  fOo
#  fOO
#  Foo
#  FoO
#  FOo
#  FOO

while (<>) {
    chomp;
    generate("", $_);
}

sub generate {
    my ($prefix, $str) = @_;

    if ($str eq "") {
	print $prefix, "\n";
    } else {
	while ($str =~ /\G(.)(.*)/g) {
	    my $char = $1;
	    my $rest = $2;
	    generate($prefix . $char, $rest);
	    $char =~ tr /a-z/A-Z/;
	    generate($prefix . $char, $rest);
	}
    }
}
