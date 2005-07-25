# -*- perl -*-

use strict;
use Test;

BEGIN { plan tests => 17 }

unless( eval { require KULM::KNP::Result; } ){
    print STDERR "KULM::KNP::Result is missing.  Skip all tests.\n";
    for( 1 .. 17 ){
	print "ok $_\n";
    }
    exit 0;
}
use KNP::Result;

my $str = "��ʸ���Ϥμ���򼨤���";
my $result = <<'__result__';
# S-ID:123
* 1D <BGH:����><SM:����:711006601***71100650****><ʸƬ><����><����><�θ�><��:�γ�><����:0-4><RID:992><���>
��ʸ �����֤� ��ʸ ̾�� 6 ����̾�� 1 * 0 * 0 NIL <ʸƬ><����><���ʴ���><��Ω><̾��������>
���� �������� ���� ̾�� 6 ����̾�� 2 * 0 * 0 NIL <����><���ʴ���><����><��Ω><��ʣ��><̾��������>
�� �� �� ���� 9 ��³���� 3 * 0 * 0 NIL <���ʴ���><�Ҥ餬��><��°>
* 2D <BGH:����><SM:����:31212*******312232******311006b0****><��><����><�θ�><��:���><����:0-0><RID:1031><���>
���� ���Ĥ줤 ���� ̾�� 6 ����̾�� 1 * 0 * 0 NIL <����><���ʴ���><��Ω><̾��������>
�� �� �� ���� 9 �ʽ��� 1 * 0 * 0 NIL <���ʴ���><�Ҥ餬��><��°>
* -1D <BGH:����><ʸ��><����><�Ѹ�:ư><��٥�:C><����:5-5><ID:��ʸ����><RID:110><�����:30>
���� ���᤹ ���� ư�� 2 * 0 �Ҳ�ư�쥵�� 5 ���ܷ� 2 NIL <ɽ��ʸ��><���ʴ���><��Ω><���Ѹ�>
�� �� �� �ü� 1 ���� 1 * 0 * 0 NIL <ʸ��><��°>
EOS
__result__

my $x = KNP::Result->new( $result );
my $y = KULM::KNP::Result->new( input => $str );
$y->setup( [ map( "$_\n", split(/\n/, $result) ) ],
	   { PATTERN => '^EOS$',
	     OPTION => { bclass => "KULM::KNP::B",
			 mclass => "KULM::KNP::M",
			 option => "-tab", } } );

ok( defined $x );
ok( defined $y );
ok( $x->mrph_list == $y->mrph_list );
ok( $x->bnst_list == $y->bnst_list );
ok( &midasi($x) eq &midasi($y) );

sub midasi {
    my( $ml ) = @_;
    join( "", map( $_->midasi, $ml->mrph_list ) );
}

my $p = ( $x->bnst_list )[1];
my $q = ( $y->bnst_list )[1];
ok( $p->string() eq $q->string() );
ok( $p->string(undef, "all") eq $q->string(undef, "all") );
ok( $p->get('ID') == $q->get('ID') );
ok( $p->get('P') and $q->get('P') and $p->get('P')->string(undef, "all") eq $q->get('P')->string(undef, "all") );
ok( $p->get('D') eq $q->get('D') );
ok( &attr_midasi($p, 'C') eq &attr_midasi($q, 'C') );
ok( &attr_midasi($p, 'ML') eq &attr_midasi($q, 'ML') );
ok( $p->get('FS') eq $q->get('FS') );
ok( join("\n", @{$p->get('F')}) eq join("\n", @{$q->get('F')}) );
ok( $p->get( [ F => 1 ] ) eq $q->get( [ F => 1 ] ) );
ok( $p->get('string') eq $q->get('string') );
ok( $p->get('p_id') == $q->get('p_id') );

sub attr_midasi {
    my( $b, $attr ) = @_;
    join( "\n", map( $_->string(undef, "all"), @{$b->get($attr)} ) );
}
