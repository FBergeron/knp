# $Id$
package KNP::SynGraph;
require 5.004_04; # For base pragma.
use strict;
use KNP::SynNode;
use Encode;

=head1 NAME

KNP::SynGraph - SynGraph in KNP

=head1 DESCRIPTION

SynGraph�γƼ������ݻ����륪�֥������ȡ�

=cut
sub new {
    my( $class, $str ) = @_;
    my $this = {};

    my $midasi_pat = '���Ф�';
    if( utf8::is_utf8( $str ) ){
	$midasi_pat = decode('euc-jp', $midasi_pat);
    }

    # !! 0 1D <���Ф�:��ݲ���><�ʲ��Ϸ��:����>
    my ($tagid, $dpnd, $string) = (split(' ', $str))[1,2,3];
    my @tagids = split(',', $tagid);

    my ($parent, $dpndtype, @parentids);
    if ($dpnd =~ /^([\-\,\/\d]+)([DPIA])$/) {
	$parent = $1;
	$dpndtype = $2;

	# �����褬ʣ�������礬����
	for (split('/', $parent)) {
	    # �б�������ܶ�ID��ʣ�������礬����
	    push @parentids, [split(',', $_)];
	}
    }
    else {
	die "KNP::SynGraph: Illegal dpnd = $dpnd\n";
    }

    my ($midasi, @features);
    while ($string =~ /(<.+?>)/g) {
	my $s = $1;
	# ���Ф�
	if ($s =~ /<$midasi_pat:(.+?)>/) {
	    $midasi = $1;
	}
	# ����¾��feature
	else {
	    push @features, $s;
	}
    }

    $this->{tagid} = $tagid;
    $this->{tagids} = \@tagids;
    $this->{parent} = $parent;
    $this->{parentids} = \@parentids;
    $this->{dpndtype} = $dpndtype;
    $this->{midasi} = $midasi;
    $this->{feature} = join('', @features);

    bless $this, $class;
}

=head1 METHODS

�ʲ��Υ᥽�åɤ����Ѳ�ǽ�Ǥ��롣

=over 4

=item push_synnode

Syn�Ρ��ɤ��ɲä���

=cut
sub push_synnode {
    my ($this, $str) = @_;

    my $synnode = KNP::SynNode->new($str);

    push @{$this->{synnodes}}, $synnode;
}

=item synnode

���Ƥ�Syn�Ρ��ɤ��֤�

=cut
sub synnode {
    my ($this) = @_;

    if( defined $this->{synnodes} ){
	@{$this->{synnodes}};
    } else {
	wantarray ? () : 0;
    }
}

=item tagid

�б�������ܶ�ID���֤���

=cut
sub tagid {
    my ($this) = @_;
    $this->{tagid};
}

=item tagids

�б�������ܶ�ID(����)���֤���

=cut
sub tagids {
    my ($this) = @_;
    @{$this->{tagids}};
}

=item parent

������δ��ܶ�ID���֤���

=cut
sub parent {
    my ($this) = @_;
    $this->{parent};
}

=item parent

������δ��ܶ�ID(���������)���֤���

=cut
sub parentids {
    my ($this) = @_;
    @{$this->{parentids}};
}

=item dpndtype

��¸�ط��μ���(D,P,I,A)���֤���

=cut
sub dpndtype {
    my ($this) = @_;
    $this->{dpndtype};
}

=item midasi

���Ф����֤���

=cut
sub midasi {
    my ($this) = @_;
    $this->{midasi};
}

=item feature

ʸˡ�������֤���

=cut
sub feature {
    my ($this) = @_;
    $this->{feature};
}

=back

=head1 AUTHOR

=over 4

=item
���� �ν� <shibata@nlp.kuee.kyoto-u.ac.jp>

=cut

1;
__END__
# Local Variables:
# mode: perl
# coding: euc-japan
# use-kuten-for-period: nil
# use-touten-for-comma: nil
# End:

