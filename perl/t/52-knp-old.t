# -*- perl -*-

use strict;
use Test;

BEGIN { plan tests => 15 }

use KNP;

my $knp = new KNP;
$knp->parse( "�֤��֤��餤����" );

ok( $knp->result );
ok( $knp->bnst_num == 3 );
ok( ref $knp->bnst eq 'ARRAY' );
ok( @{$knp->bnst} == $knp->bnst_num );
ok( $knp->bnst( 1, 'start' ) == 1 );
ok( $knp->bnst( 1, 'end' ) == 2 );
ok( $knp->bnst( 1, 'parent_id' ) == 2 );
ok( $knp->bnst( 1, 'child_id' ) == 1 );
ok( @{$knp->{BNST}[1]{mrph}} == 2 );

ok( $knp->mrph_num == 5 );
ok( ref $knp->mrph eq 'ARRAY' );
ok( @{$knp->mrph} == $knp->mrph_num );
ok( $knp->mrph( 0, 'midasi' ) eq "�֤�" );
ok( $knp->{MRPH}[0]{midasi} eq "�֤�" );
ok( $knp->mrph( 0, 'feature', 0 ) eq "ʸƬ" );
