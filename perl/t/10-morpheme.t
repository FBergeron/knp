# -*- perl -*-

use strict;
use Test;

BEGIN { plan tests => 21 }

use KNP::Morpheme;

my $spec = "��ʸ �����֤� ��ʸ ̾�� 6 ����̾�� 1 * 0 * 0 NIL <����><���ʴ���><��Ω><��ʣ��><̾��������>\n";
my $mrph = KNP::Morpheme->new( $spec );

ok(defined $mrph);
ok($mrph->midasi eq '��ʸ');
ok($mrph->yomi eq '�����֤�');
ok($mrph->genkei eq '��ʸ');
ok($mrph->hinsi eq '̾��');
ok($mrph->hinsi_id == 6);
ok($mrph->bunrui eq '����̾��');
ok($mrph->bunrui_id == 1);
ok($mrph->katuyou1 eq '*');
ok($mrph->katuyou1_id == 0);
ok($mrph->katuyou2 eq '*');
ok($mrph->katuyou2_id == 0);
ok($mrph->imis eq 'NIL');
ok($mrph->fstring eq '<����><���ʴ���><��Ω><��ʣ��><̾��������>');
ok($mrph->feature == 5);
ok($mrph->spec eq $spec);

ok($mrph->push_feature('TEST') == 6);
ok($mrph->fstring =~ /<TEST>/);

ok( ! $mrph->fstring( "" ) );
ok( ! $mrph->fstring );
ok( $mrph->feature == 0 );
