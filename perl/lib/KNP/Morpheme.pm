# $Id$
package KNP::Morpheme;
require 5.000;
use Exporter;
use Juman::Morpheme;
use KNP::Fstring;
use strict;
use vars qw/ @ISA @EXPORT_OK @ATTRS /;
@ISA = qw/ KNP::Fstring Juman::Morpheme Exporter /;
@EXPORT_OK = qw/ @ATTRS /;

=head1 NAME

KNP::Morpheme - �����ǥ��֥������� in KNP

=head1 SYNOPSIS

  $m = new KNP::Morpheme( "���� �������� ���� ̾�� 6 ����̾�� 2 * 0 * 0 NIL <ʸƬ>", 1 );

=head1 DESCRIPTION

�����ǤγƼ������ݻ����륪�֥������ȡ�

=head1 CONSTRUCTOR

=over 4

=item new ( SPEC, ID )

��1���� C<SPEC> �� KNP �ν��Ϥ��������ƸƤӽФ��ȡ����ιԤ����Ƥ����
����������������ǥ��֥������Ȥ��������롥

=cut

@ATTRS = ( 'fstring' );

sub new {
    my( $class, $spec, $id ) = @_;
    my $this = { id => $id };

    my @value;
    my( @keys ) = @Juman::Morpheme::ATTRS;
    push( @keys, @ATTRS );
    $spec =~ s/\s*$//;
    if( $spec =~ s/^\\ \\ \\ �ü� 1 ���� 6 // ){
	@value = ( '\ ', '\ ', '\ ', '�ü�', '1', '����', '6' );
	push( @value, split( / /, $spec, scalar(@keys) - 7 ) );
    } else {
	@value = split( / /, $spec, scalar(@keys) );
    }
    while( @keys and @value ){
	my $key = shift @keys;
	$this->{$key} = shift @value;
    }

    &KNP::Fstring::fstring( $this, $this->{fstring} );
    bless $this, $class;
}

=back

=head1 METHODS

L<Juman::Morpheme> �γƥ᥽�åɤ˲ä��ơ�KNP �ˤ�äƳ�����Ƥ�줿��
ħʸ����򻲾Ȥ��뤿��Υ᥽�åɤ����Ѳ�ǽ�Ǥ��롥

=over 4

=item fstring

��ħʸ������֤���

=item feature

��ħ�Υꥹ�Ȥ��֤���

=item push_feature

��ħ���ɲä��롥

=back

�����Υ᥽�åɤξܺ٤ˤĤ��Ƥϡ�L<KNP::Fstring> �򻲾ȤΤ��ȡ����ˡ�
�ʲ��Υ᥽�åɤ����Ѳ�ǽ�Ǥ��롥

=over 4

=item spec

�����Ǥ����Ƥν�����ؼ�����ʸ������������롥KNP �ν��Ϥ�1�Ԥ�������
�롥

=cut

sub spec {
    my( $this ) = @_;
    sprintf( "%s\n", join( ' ', map( $this->{$_}, ( @Juman::Morpheme::ATTRS, @ATTRS ) ) ) );
}

=back

=head1 SEE ALSO

=over 4

=item *

L<KNP::Fstring>

=item *

L<Juman::Morpheme>

=back

=head1 AUTHOR

=over 4

=item
�ڲ� ��̭ <tsuchiya@pine.kuee.kyoto-u.ac.jp>

=cut

1;
__END__
# Local Variables:
# mode: perl
# coding: euc-japan
# use-kuten-for-period: nil
# use-touten-for-comma: nil
# End:
