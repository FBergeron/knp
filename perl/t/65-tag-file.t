# -*- perl -*-

use strict;
use Test;

BEGIN { plan tests => 17 }

use KNP::File;

my $file = 't/tag-sample.knp';
my $dbfile = sprintf('%s.%d', $file, $$ );

my $x = new KNP::File( $file );
ok(defined $x);
ok($x->name eq $file);

my( $i, $y, $z );
for( $i = 0; $y = $x->each(); $i++ ){ $z = $y; }
ok( $i == 9 );

$z = $x->each();
ok( join( "", map( $_->midasi, $z->mrph ) ) eq "¼���ٻԼ����ǯƬ�ˤ�������괱š����յ��Բ����Ȭ���񸫤����Ҳ��ޤο�̱��Ϣ���°�İ���Υ������ˤĤ��ơ������˱ƶ���ڤܤ����ȤˤϤʤ�ʤ���Υ�޼Ԥ����Ƥ⡢�����ϰϤˤȤɤޤ�Ȼפ��פȽҤ١�����Υ�ޤˤϻ��ʤ��Ȥθ��̤��򼨤�����" );
ok( $z->bnst == 27 );
ok( $z->tag == 38 );

$y = ( $z->tag )[0];
ok( ref $y eq 'KNP::Tag' );
ok( ref $y->parent eq 'KNP::Tag' );
ok( $y->parent->id == 1 );

$y = ( $z->tag )[1];
ok( ref $y eq 'KNP::Tag' );
ok( ref $y->parent eq 'KNP::Tag' );
ok( $y->parent->id == 37 );

$y = ( $z->tag )[36];
ok( ref $y eq 'KNP::Tag' );
ok( ref $y->parent eq 'KNP::Tag' );
ok( $y->parent->id == 37 );

$y = ( $z->tag )[-1];
ok( ref $y eq 'KNP::Tag' );
ok( ! $y->parent );
