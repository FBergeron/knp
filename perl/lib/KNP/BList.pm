# $Id$
package KNP::BList;
require 5.003_07; # For UNIVERSAL->isa().
use Exporter;
use KNP::Bunsetsu;
use KNP::DrawTree;
use KNP::TList;
use strict;
use vars qw/ @ISA /;
@ISA = qw/ KNP::DrawTree Exporter /;

=head1 NAME

KNP::BList - ʸ���󥪥֥�������

=head1 SYNOPSIS

  $result = new KNP::BList();

=head1 DESCRIPTION

ʸ������ݻ����륪�֥������ȡ�

=head1 CONSTRUCTOR

=over 4

=item new( @BNST )

���ꤵ�줿ʸ��Υꥹ�Ȥ��ݻ����륪�֥������Ȥ��������롥��������ά����
�����ϡ���ʸ������ݻ����륪�֥������Ȥ��������롥

=cut
sub new {
    my $new = bless( {}, shift );
    if( @_ ){
	$new->push_bnst( @_ );
    }
    $new;
}

=back

=head1 METHODS

=over 4

=item bnst

=item bnst_list

���Ƥ�ʸ��Υꥹ�Ȥ��֤���

=cut
sub bnst {
    my( $this ) = @_;
    if( defined $this->{bnst} ){
	@{$this->{bnst}};
    } else {
	wantarray ? () : 0;
    }
}

sub bnst_list {
    shift->bnst( @_ );
}

=item push_bnst( @BNST )

���ꤵ�줿ʸ�����ʸ�����ɲä��롥Ʊ���ˡ�ʸ����˴ޤޤ�Ƥ��륿����
����������ɲä��롥

=cut
sub push_bnst {
    my( $this, @bnst ) = @_;
    if( grep( ! $_->isa('KNP::Bunsetsu'), @bnst ) ){
	die "Illegal type of argument.";
    } elsif( $this->{BLIST_READONLY} ){
	die;
    } else {
	push( @{ $this->{bnst} ||= [] }, @bnst );
    }
}

=item tag

=item tag_list

���ƤΥ����Υꥹ�Ȥ��֤���

=cut
sub tag {
    map( $_->tag, shift->bnst );
}

sub tag_list {
    shift->tag( @_ );
}

=item push_tag( @TAG )

���ꤵ�줿������ʸ�����ɲä�������ʸ��Υ�����Ȥ��Ƥ�Ĺ�����֤����ɲ�
�оݤȤʤ�ʸ�᤬¸�ߤ��ʤ�(= ʸ���󤬶��Ǥ���)���ϡ��ɲäϹԤ��ʤ���

=cut
sub push_tag {
    my $this = shift;
    if( $this->bnst ){
	( $this->bnst )[-1]->push_tag( @_ );
    } else {
	0;
    }
}

=item mrph

=item mrph_list

���Ƥη����ǤΥꥹ�Ȥ��֤���

=cut
sub mrph {
    map( $_->mrph, shift->bnst );
}

sub mrph_list {
    shift->mrph( @_ );
}

=item push_mrph( @MRPH )

���ꤵ�줿�����Ǥ�ʸ�����ɲä�������ʸ��η�������Ȥ��Ƥ�Ĺ�����֤���
�ɲ��оݤȤʤ�ʸ�᤬¸�ߤ��ʤ�(= ʸ���󤬶��Ǥ���)���ϡ��ɲäϹԤ��
�ʤ���

=cut
sub push_mrph {
    my $this = shift;
    if( $this->bnst ){
	( $this->bnst )[-1]->push_mrph( @_ );
    } else {
	0;
    }
}

=item set_readonly

ʸ������Ф���񤭹��ߤ��Ե��Ĥ����ꤹ�롥

=cut
sub set_readonly {
    my( $this ) = @_;
    for my $bnst ( $this->bnst ){
	$bnst->set_readonly();
    }
    $this->{BLIST_READONLY} = 1;
}

=item spec

ʸ�����ʸ������Ѵ����롥

=cut
sub spec {
    my( $this ) = @_;
    join( '', map( $_->spec, $this->bnst ) );
}

=item draw_tree

=item draw_bnst_tree

ʸ����ΰ�¸�ط����ڹ�¤�Ȥ���ɽ�����ƽ��Ϥ��롥

=cut
sub draw_bnst_tree {
    shift->draw_tree( @_ );
}

=item draw_tag_tree

������ΰ�¸�ط����ڹ�¤�Ȥ���ɽ�����ƽ��Ϥ��롥

=cut
sub draw_tag_tree {
    KNP::TList->new( shift->tag )->draw_tree( @_ );
}

# draw_tree �᥽�åɤȤ��̿��ѤΥ᥽�åɡ�
sub draw_tree_leaves {
    shift->bnst( @_ );
}

=back

=head1 DESTRUCTOR

ʸ�ᥪ�֥������ȴ֤˴ľ��Υ�ե���󥹤����������ȡ��̾�� Garbage
Collection �ˤ�äƤϥ��꤬�������ʤ��ʤ롥����������򤱤뤿��ˡ�
����Ū�˥�ե���󥹤��˲����� destructor ��������Ƥ��롥

=cut
sub DESTROY {
    my( $this ) = @_;
    grep( ref $_ && $_->isa('KNP::Bunsetsu') && $_->DESTROY, $this->bnst );
}

=head1 SEE ALSO

=over 4

=item *

L<KNP::Bunsetsu>

=back

=head1 AUTHOR

=over 4

=item
�ڲ� ��̭ <tsuchiya@pine.kuee.kyoto-u.ac.jp>

=back

=cut

1;
__END__
# Local Variables:
# mode: perl
# coding: euc-japan
# use-kuten-for-period: nil
# use-touten-for-comma: nil
# End:
