# $Id$
package KNP::Result;
require 5.004_04; # For base pragma.
use KNP::Bunsetsu;
use KNP::Morpheme;
use KNP::Tag;
use KNP::SynNodes;
use KNP::SynNode;
use strict;
use base qw/ KNP::BList /;
use vars qw/ %DEFAULT /;

=head1 NAME

KNP::Result - ��ʸ���Ϸ�̥��֥�������

=head1 SYNOPSIS

  $result = new KNP::Result( "* -1D <BGH:����>\n...\nEOS\n" );

=head1 DESCRIPTION

��ʸ���Ϸ�̤��ݻ����륪�֥������ȡ�

=head1 CONSTRUCTOR

=over 4

=item new ( RESULT )

KNP �ν���ʸ���󡤤ޤ��ϡ�����ʸ�����Ԥ�ñ�̤Ȥ��Ƴ�Ǽ���줿�ꥹ�Ȥ�
�Ф����ե���� RESULT ������Ȥ��ƸƤӽФ��ȡ����ι�ʸ���Ϸ�̤�ɽ
�����֥������Ȥ��������롥

=item new ( OPTIONS )

�ʲ��γ�ĥ���ץ�������ꤷ�ƥ��󥹥ȥ饯����ƤӽФ���

=over 4

=item result => RESULT

KNP �ν���ʸ���󡤤ޤ��ϡ�����ʸ�����Ԥ�ñ�̤Ȥ��Ƴ�Ǽ���줿�ꥹ�Ȥ�
�Ф����ե���󥹤���ꤹ�롥

=item pattern => STRING

��ʸ���Ϸ�̤�ü���뤿��Υѥ��������ꤹ�롥

=item bclass => NAME

ʸ�ᥪ�֥������Ȥ���ꤹ�롥̵����ξ��ϡ�C<KNP::Bunsetsu> ���Ѥ��롥

=item mclass => NAME

�����ǥ��֥������Ȥ���ꤹ�롥̵����ξ��ϡ�C<KNP::Morpheme> ���Ѥ��롥

=item tclass => NAME

�������֥������Ȥ���ꤹ�롥̵����ξ��ϡ�C<KNP::Tag> ���Ѥ��롥

=back

=cut
%DEFAULT = ( pattern => '^EOS$',
	     bclass  => 'KNP::Bunsetsu',
	     mclass  => 'KNP::Morpheme',
	     tclass  => 'KNP::Tag' );

sub new {
    my $class = shift;

    my( %opt ) = %DEFAULT;
    if( @_ == 1 ){
	$opt{result} = shift;
    } else {
	while( @_ ){
	    my $key = shift;
	    my $val = shift;
	    $key =~ s/^-+//;
	    $opt{lc($key)} = $val;
	}
    }
    my $result  = $opt{result};
    my $pattern = $opt{pattern};
    my $bclass  = $opt{bclass};
    my $mclass  = $opt{mclass};
    my $tclass  = $opt{tclass};
    return undef unless( $result and $pattern and $bclass and $mclass and $tclass );

    # ʸ����ľ�ܻ��ꤵ�줿���
    $result = [ map( "$_\n", split( /\n/, $result ) ) ] unless ref $result;

    my $this = { all => join( '', @$result ) };
    bless $this, $class;
    return $this unless $pattern;

    # ��ʸ���Ϸ�̤���Ƭ�˴ޤޤ�Ƥ��륳���Ȥȥ��顼����ʬ�������
    my( $str, $comment, $error );
    while( defined( $str = shift @$result ) ){
	if( $str =~ /^#/ ){
	    $comment .= $str;
	} elsif( $str =~ m!^;;! ){
	    $error .= $str;
	} else {
	    unshift( @$result, $str );
	    last;
	}
    }

    while( defined( $str = shift @$result ) ){
	if( $str =~ m!$pattern! and @$result == 0 ){
	    $this->{_eos} = $str;
	    last;
	} elsif( $str =~ m!^;;! ){
	    $error .= $str;
	} elsif( $str =~ m!^\*! ){
	    $this->push_bnst( $bclass->new( $str, scalar($this->bnst) ) );
	} elsif( $str =~ m!^\+! ){
	    $this->push_tag( $tclass->new( $str, scalar($this->tag) ) );
	} elsif( $str =~ m/^!!/ ){
	    my $synnodes = KNP::SynNodes->new($str);
	    push @{ ( $this->tag ) [-1]->{synnodes} }, $synnodes;
	} elsif( $str =~ m/^!/ ){
	    my $synnode = KNP::SynNode->new($str);
	    ( $this->tag ) [-1]->{synnodes}[-1]->push_synnode( $synnode );
	} else {
	    $this->push_mrph( $mclass->new( $str, scalar($this->mrph) ) );
	    my $fstring = ( $this->mrph )[-1]->fstring;
	    while ( $fstring =~ /<(ALT-[^>]+)>/g ){ # ALT
		( $this->mrph )[-1]->push_doukei( $mclass->new( $1, scalar($this->mrph) ) );
	    }
	}
    }

    # ��������������Ф�
    my( @bnst ) = $this->bnst;
    for my $bnst ( @bnst ){
	$bnst->make_reference( \@bnst );
    }
    my( @tag ) = $this->tag;
    for my $tag ( @tag ){
	$tag->make_reference( \@tag );
    }

    # �񤭹��ߤ�ػߤ���
    $this->set_readonly();

    $this->{comment} = $comment;
    $this->{error}   = $error;
    $this;
}

=back

=head1 METHODS

��ʸ���Ϸ�̤ϡ��оݤȤʤ�ʸ��ʸ��ñ�̤�ʬ�򤷤��ꥹ�Ȥȸ��뤳�Ȥ��Ǥ�
�롥���Τ��ᡤ�ܥ��饹�� C<KNP::BList> ���饹��Ѿ�����褦�˼�������
�Ƥ��ꡤ�ʲ��Υ᥽�åɤ����Ѳ�ǽ�Ǥ��롥

=over 4

=item bnst

ʸ�������Ф���

=item tag

���������Ф���

=item mrph

�����������Ф���

=back

�����Υ᥽�åɤξܺ٤ˤĤ��Ƥϡ�L<KNP::BList> �򻲾ȤΤ��ȡ�

�ä��ơ��ʲ��Υ᥽�åɤ��������Ƥ��롥

=over 4

=item all

��ʸ���Ϸ�̤���ʸ������֤���

=cut
sub all {
    my( $this ) = @_;
    $this->{all} || undef;
}

=item all_dynamic

��ʸ���Ϸ�̤���ʸ�����ưŪ�˺�ä��֤���
�ؿ�push_feature�ʤɤ�Ȥä����������񤭴��������ˡ�all�Ǥ��ѹ���ȿ�Ǥ���ʤ����ᡤ���δؿ���Ȥ���

=cut
sub all_dynamic {
    my( $this, $option ) = @_;

    my $ret;
    $ret .= $this->{comment};
    # ʸ�����֤�
    foreach my $bnst ($this->bnst) {
	$ret .= '* ';
	$ret .= defined $bnst->parent ? $bnst->parent->id : -1;
	$ret .=  $bnst->dpndtype . ' ' . $bnst->fstring . "\n";

	# ���ܶ����֤�
	foreach my $tag ($bnst->tag) {
	    $ret .=  '+ ';
	    $ret .= defined $tag->parent ? $tag->parent->id : -1;
	    $ret .= $tag->dpndtype . ' ' . $tag->fstring . "\n";

	    # �����Ǥ���֤�
	    for my $mrph ($tag->mrph) {
		$ret .= $mrph->midasi . ' ' . $mrph->yomi . ' ' . $mrph->genkei . ' ' . $mrph->hinsi . ' ' . $mrph->hinsi_id . ' ' . $mrph->bunrui . ' ' . $mrph->bunrui_id. ' ' . $mrph->katuyou1 . ' ' . $mrph->katuyou1_id . ' ' . $mrph->katuyou2 . ' ' . $mrph->katuyou2_id . ' ' . $mrph->imis . ' ' . $mrph->fstring . "\n";
	    }

	    # SynGraph
	    for my $synnodes ($tag->synnodes) {
		$ret .= '!! ';
		$ret .= $synnodes->tagid . ' ' . $synnodes->parent . $synnodes->dpndtype . ' <���Ф�:' . $synnodes->midasi . '>' . $synnodes->feature . "\n";

		for my $synnode ($synnodes->synnode) {
		    $ret .= '! ';
		    $ret .= $synnode->tagid . ' <SYNID:' . $synnode->synid . '><������:' . $synnode->score . '>' . $synnode->feature . "\n";
		}
	    }
	}
    }
    $ret .= "EOS\n";

    return $ret;
}

=item comment

��ʸ���Ϸ����Υ����Ȥ��֤���

=cut
sub comment {
    my( $this ) = @_;
    $this->{comment} || undef;
}

=item error

��ʸ���Ϸ����Υ��顼��å��������֤���

=cut
sub error {
    my( $this ) = @_;
    $this->{error} || undef;
}

=item id

��ʸ���Ϸ��ID�����롥

=cut
sub id {
    my $this = shift;
    if( @_ ){
	$this->set_id( @_ );
    } else {
	unless( defined $this->{_id} ){
	    $this->{_id} = $this->{comment} =~ m/# S-ID:(\S+)/ ? $1 : -1;
	}
	$this->{_id};
    }
}

=item set_id ( ID )

��ʸ���Ϸ��ID�����ꤹ�롥

=cut
sub set_id {
    my( $this, $id ) = @_;
    if( defined $this->{comment} ){
	( $this->{comment} =~ s/# S-ID:\S+/# S-ID:$id/ )
	    or ( $this->{comment} = "S-ID:$id\n" . $this->{comment} );
    } else {
	$this->{comment} = "S-ID:$id\n";
    }
    $this->{_id} = $id;
}

=item spec

��ʸ���Ϸ�̤�ɽ������ʸ������������롥
SynGraph��ʬ�Ͻ�����롥

=cut
sub spec {
    my( $this ) = @_;
    sprintf( "%s%s%s",
	     $this->{comment},
	     $this->KNP::BList::spec(),
	     $this->{_eos} );
}

=item make_ss

ɸ�๽¤(Standard Structure)���֤���

=cut
sub make_ss {
    my ( $this ) = @_;

    my %ss;

    $ss{sentence}{id} = $this->id;
    $ss{sentence}{comment} = $this->comment;
    chomp $ss{sentence}{comment};

    $ss{sentence}{phrase} = [];

    my $phrase = $ss{sentence}{phrase};

    # ���ܶ����֤�
    foreach my $tag ( $this->tag ) {
	# �Ҷ�
	my @child_ids;
	if (defined $tag->child) {
	    foreach my $ctag ($tag->child) {
		push @child_ids, $ctag->id;
	    }
	}

	push @{$phrase}, { id => $tag->id,
			   fstring => $tag->fstring,
			   dpndtype => $tag->dpndtype,
			   parent => defined $tag->parent ? $tag->parent->id : -1,
			   child => join('/', @child_ids)
		       };

	push @{$phrase->[-1]{node}}, { type => 'base' };

	# �����Ǥ���֤�
	foreach my $mrph ( $tag->mrph ) {
	    my $repname = $mrph->repname ? $mrph->repname : $mrph->genkei . '/' .  $mrph->yomi;

	    push @{$phrase->[-1]{node}[0]{word}}, { fstring => $mrph->fstring,
						    content => $mrph->midasi, # <word ...>(����������)</word> 
						    katuyou1 => $mrph->katuyou1 eq '*' ? '' : $mrph->katuyou1,
						    katuyou2 => $mrph->katuyou2 eq '*' ? '' : $mrph->katuyou2,
						    repname => $repname,
						    imis => $mrph->imis eq 'NIL' ? '' : $mrph->imis,
						    yomi => $mrph->yomi,
						    hinsi => $mrph->hinsi,
						    bunrui => $mrph->bunrui eq '*' ? '' : $mrph->bunrui
						    };
	}
    }

    return \%ss;
}

=item all_xml

XML���֤���

=cut
sub all_xml {
    my ( $this ) = @_;

    require XML::Simple;

    my $xs = new XML::Simple;

    my $ss = $this->make_ss;

    my $xml = $xs->XMLout($ss, KeepRoot => 1);

    return $xml;
}

=back

=head1 SEE ALSO

=over 4

=item *

L<KNP::BList>

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
