# -*- perl -*-

use strict;
use Test;

BEGIN { plan tests => 20 }

use KNP::BList;

my $x = new KNP::BList();
ok(defined $x);
ok( $x->bnst == 0 );
ok( $x->tag == 0 );
ok( $x->mrph == 0 );

my $b = KNP::Bunsetsu->new( <<'__bunsetsu__', 3 );
* -1D <BGH:����><SM:����:711006601***71100650****><ʸƬ><ʸ��><����><�θ�><�Ѹ�:ư><�θ���><��٥�:C><����:5-5><ID:��ʸ����><RID:110><�����:30><���>
__bunsetsu__
ok( defined $b );
$x->push_bnst( $b );
ok( $x->bnst == 1 );
ok( $x->tag  == 0 );
ok( $x->mrph == 0 );

my $t = KNP::Tag->new( <<'__tag__', 0 );
+ 1D <���ߡ�1>
__tag__
ok( defined $t );
$x->push_tag( $t );
ok( $x->bnst == 1 );
ok( $x->tag  == 1 );
ok( $x->mrph == 0 );

my $m = KNP::Morpheme->new( <<'__koubun_mrph__' );
��ʸ �����֤� ��ʸ ̾�� 6 ����̾�� 1 * 0 * 0 NIL <ʸƬ><����><���ʴ���><��Ω><̾��������>
__koubun_mrph__
ok( defined $m );
$x->push_mrph( $m );
ok( $x->bnst == 1 );
ok( $x->tag  == 1 );
ok( $x->mrph == 1 );
ok( $t->mrph == 1 );

$m = KNP::Morpheme->new( <<'__kaiseki_mrph__' );
���� �������� ���� ̾�� 6 ����̾�� 2 * 0 * 0 NIL <ʸ��><ɽ��ʸ��><����><���ʴ���><����><��Ω><��ʣ��><̾��������>
__kaiseki_mrph__
ok( defined $m );
$x->push_mrph( $m );
ok( $b->mrph == 2 );
ok( $t->mrph == 2 );
