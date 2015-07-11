$offset = 0;
while (<>) {
    while (/(.)/gs) {
	print pack 'N', $offset;
	$offset++;
    }
}
