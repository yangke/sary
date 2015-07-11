$offset = 0;
while (<>) {
    print pack 'N', $offset;
    $offset += length;
}
