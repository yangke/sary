$offset = 0;
while (<>) {
    while (/(\S+)/g) {
	print pack 'N', $offset + pos() - length($1);
    }
    $offset += length;
}
