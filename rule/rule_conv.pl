#!/usr/bin/env jperl

# KNP�롼��ե�����γ��([, ], <, >)���Ѵ�

for ($i = 0; $i < @ARGV; $i++) {

    open(INPUT, $ARGV[$i]);
    $ARGV[$i] =~ s/rule$/data/;

    print "make $ARGV[$i] ... \n";

    chmod 0640, $ARGV[$i];
    open(OUTPUT, "> $ARGV[$i]") || die;

    while (<INPUT>) {

	s/\[/\(/g;
	s/\]/\)/g;
	s/\</\(/g;
	s/\>/\)/g;

	print OUTPUT;
    }

    close INPUT;
    close OUTPUT;

    chmod 0444, $ARGV[$i];
}
