# -*- perl -*-

use strict;
use Test;

BEGIN { plan tests => 14 }

use KNP::TList;

my $x = new KNP::TList();
ok(defined $x);
ok($x->tag == 0);
ok($x->mrph == 0);

my $m = KNP::Morpheme->new( <<'__koubun_mrph__' );
��ʸ �����֤� ��ʸ ̾�� 6 ����̾�� 1 * 0 * 0 NIL <ʸƬ><����><���ʴ���><��Ω><̾��������>
__koubun_mrph__
ok( defined $m );
$x->push_mrph( $m );
ok( $x->tag == 0 );
ok( $x->mrph == 0 );

my $t = KNP::Tag->new( <<'__tag__', 1 );
+ 2D <SM:�ٻ�:2110********><ʸ����><��̾><�θ�><��:����><��̾��><����:0-0><RID:1352><������-����><���ϳ�-��><���ϳ�-��><���ϳ�-��><����Ϣ��-��><����Ϣ��-���δط�><����Ϣ��-����>
__tag__
ok( defined $t );
$x->push_tag( $t );
ok( $x->tag == 1 );
ok( $x->mrph == 0 );
ok( $t->mrph == 0 );

$m = KNP::Morpheme->new( <<'__kaiseki_mrph__' );
���� �������� ���� ̾�� 6 ����̾�� 2 * 0 * 0 NIL <ʸ��><ɽ��ʸ��><����><���ʴ���><����><��Ω><��ʣ��><̾��������>
__kaiseki_mrph__
ok( defined $m );
$x->push_mrph( $m );
ok( $x->tag == 1 );
ok( $x->mrph == 1 );
ok( $t->mrph == 1 );
