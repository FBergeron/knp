# $Id$
package KNP::MList;
require 5.003_07; # For UNIVERSAL->isa().
use strict;
use base qw/ Juman::MList /;
use Encode;

=head1 NAME

KNP::MList - �������󥪥֥�������

=head1 SYNOPSIS

  $result = new KNP::MList();

=head1 DESCRIPTION

����������ݻ����륪�֥������ȡ�

=head1 CONSTRUCTOR

=over 4

=item new ( [MRPHS] )

���ꤵ�줿����������ݻ����륪�֥������Ȥ��������롥��ά���줿���ϡ�
��������������ͤȤ����Ѥ��롥

=head1 METHODS

=over 4

=item mrph ( NUM )

�� I<NUM> ���ܤη����Ǥ��֤���

=item mrph

���Ƥη����ǤΥꥹ�Ȥ��֤���

=begin comment

C<mrph> �᥽�åɤμ��Τ� C<Juman::KULM::MList> ���������Ƥ��롥

=end comment

=item mrph_list

���Ƥη����ǤΥꥹ�Ȥ��֤���

=item push_mrph ( @MRPH )

���ꤵ�줿���������ʸ�����ɲä��롥

=item set_readonly

����������Ф���񤭹��ߤ��Ե��Ĥ����ꤹ�롥

=item spec

�����������ʸ������֤���KNP �ˤ����Ϥ�Ʊ�������η�̤������롥

=cut
sub spec {
    my( $this ) = @_;
    my $str;
    for my $mrph ( $this->mrph_list() ){
	$str .= $mrph->spec();

	# KNP::Morpheme �� fstring ��Ʊ���������ޤ�Ƥ���Τ�
	# ���̤ʽ����ϹԤʤ�ʤ�
    }
    $str;
}

=item repname

�����������ɽɽ�����֤���

=cut
sub repname {
    my ( $this ) = @_;
    my $pat = '��������ɽɽ��';
    if( utf8::is_utf8( $this->fstring ) ){
	$pat = decode('euc-jp', $pat);
    }

    if ( defined $this->{fstring} ){
	if ($this->{fstring} =~ /<$pat:([^\>]+)>/){
	    return $1;
	}
    }
    return undef;
}

=back

=head1 SEE ALSO

=over 4

=item *

L<KNP::Result>

=item *

L<KNP::Morpheme>

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
