# -*- perl -*-

use strict;
use Test;

BEGIN { plan tests => 13 }

use KNP::File;

my $file = 't/sample.knp';
my $dbfile = sprintf('%s.%d', $file, $$ );

my $x = new KNP::File( $file );
ok(defined $x);
ok($x->name eq $file);

my( $i, $y, $z );
for( $i = 0; $y = $x->each(); $i++ ){ $z = $y; }
ok( $i == 3 );

# ʸ ID �ǡ����١�����������ʤ��Ƥ⸡���Ǥ��뤫�ƥ���
$y = $x->look($i);
ok(defined $y);
ok($y->spec() eq $z->spec());

$x = new KNP::File( file => $file, dbfile => $dbfile );
ok(defined $x);
ok($x->name eq $file);
ok($x->dbname eq $dbfile);

if( ! eval { require DB_File; } ){
    print STDERR "\nDB_File is missing.  Skip remaining tests.\n";
    print "ok\n" x 5;
} else {
    # ʸ ID �ǡ����١����κ����ƥ���
    ok($x->makedb());
    ok(-f $dbfile);

    # ʸ ID �ǡ����١��������Ѥ�����ʸ���������ФΥƥ���
    $y = $x->look($i);
    ok(defined $y);
    ok($y->spec() eq $z->spec());

    unlink $dbfile;

    for ( $i = 0; $y = $x->each(); $i++ ) {
	$z = $y;
    }
    ok( $i == 3 );
}
