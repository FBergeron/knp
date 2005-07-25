# -*- perl -*-

use strict;
use Test;

BEGIN { plan tests => 16 }

use KNP::Tag;

my $t = KNP::Tag->new( <<'__tag__', 1 );
+ 2D <SM:�ٻ�:2110********><ʸ����><��̾><�θ�><��:����><��̾��><����:0-0><RID:1352><������-����><���ϳ�-��><���ϳ�-��><���ϳ�-��><����Ϣ��-��><����Ϣ��-���δط�><����Ϣ��-����>
__tag__

ok( defined $t );
ok( $t->id == 1 );
ok( $t->dpndtype eq "D" );
ok( $t->mrph == 0 );

ok( $t->parent_id == 2 );
ok( ! $t->parent_id( undef ) );

ok( $t->fstring );
ok( $t->feature == 15 );
ok( ! $t->fstring( "" ) );
ok( ! $t->fstring );
ok( $t->feature == 0 );

my $m = KNP::Morpheme->new( <<'__koubun_mrph__' );
��ʸ �����֤� ��ʸ ̾�� 6 ����̾�� 1 * 0 * 0 NIL <ʸƬ><����><���ʴ���><��Ω><̾��������>
__koubun_mrph__
ok( defined $m );
$t->push_mrph( $m );
ok( scalar($t->mrph) == 1 );

$m = KNP::Morpheme->new( <<'__kaiseki_mrph__' );
���� �������� ���� ̾�� 6 ����̾�� 2 * 0 * 0 NIL <ʸ��><ɽ��ʸ��><����><���ʴ���><����><��Ω><��ʣ��><̾��������>
__kaiseki_mrph__
ok( defined $m );
$t->push_mrph( $m );
ok( scalar($t->mrph) == 2 );

ok( join('',map($_->midasi,$t->mrph)) eq '��ʸ����' );
