# -*- perl -*-

package MyBunsetsu;
use Exporter;
use Juman::MList;
use KNP::Bunsetsu;
use strict;
use vars qw/ @ISA /;
@ISA = qw/ KNP::Bunsetsu Exporter /;

# ʸ���Ω�������°�����ʬ�䤹��᥽�å�
sub split {
    my( $this ) = @_;
    my @mrph = $this->mrph;
    my @buf;
    while ( @mrph and $mrph[0]->fstring !~ /<��°>/ ) {
	push( @buf, shift @mrph );
    }
    ( new Juman::MList( @buf ), new Juman::MList( @mrph ) );
}

package main;
use strict;
use Test;

BEGIN { plan tests => 3 }

use KNP;

my $knp = new KNP( bclass => 'MyBunsetsu' );
my $result = $knp->parse( "�֤��֤��餤����" );
ok( $result );
if( ($result->bnst)[1]->can('split') ){
    my( $jiritsu, $huzoku ) = ( $result->bnst )[1]->split();
    ok( join( '', map( $_->midasi, $jiritsu->mrph ) ) eq "��" );
    ok( join( '', map( $_->midasi, $huzoku->mrph ) ) eq "��" );
} else {
    ok(0);
    ok(0);
}
