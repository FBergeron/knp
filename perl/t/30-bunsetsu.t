# -*- perl -*-

use strict;
use Test;

BEGIN { plan tests => 19 }

use KNP::Bunsetsu;

my $b = KNP::Bunsetsu->new( <<'__bunsetsu__', 3 );
* -1D <BGH:����><SM:����:711006601***71100650****><ʸƬ><ʸ��><����><�θ�><�Ѹ�:ư><�θ���><��٥�:C><����:5-5><ID:��ʸ����><RID:110><�����:30><���>
__bunsetsu__

ok( defined $b );
ok( $b->id == 3 );
ok( $b->dpndtype eq "D" );
ok( $b->mrph == 0 );
ok( $b->tag == 0 );

my $pstring = "��ʸ�ڤ��ɲ�ʸ����";
ok( $b->pstring( $pstring ) eq $pstring );
ok( $b->pstring eq $pstring );
ok( ! $b->pstring( "" ) );
ok( ! $b->pstring );

ok( $b->fstring );
ok( $b->feature == 14 );
ok( ! $b->fstring( "" ) );
ok( ! $b->fstring );
ok( $b->feature == 0 );

my $m = KNP::Morpheme->new( <<'__koubun_mrph__' );
��ʸ �����֤� ��ʸ ̾�� 6 ����̾�� 1 * 0 * 0 NIL <ʸƬ><����><���ʴ���><��Ω><̾��������>
__koubun_mrph__
ok( defined $m );
$b->push_mrph( $m );
ok( scalar($b->mrph) == 1 );

$m = KNP::Morpheme->new( <<'__kaiseki_mrph__' );
���� �������� ���� ̾�� 6 ����̾�� 2 * 0 * 0 NIL <ʸ��><ɽ��ʸ��><����><���ʴ���><����><��Ω><��ʣ��><̾��������>
__kaiseki_mrph__
ok( defined $m );
$b->push_mrph( $m );
ok( scalar($b->mrph) == 2 );

ok( join('',map($_->midasi,$b->mrph)) eq '��ʸ����' );
