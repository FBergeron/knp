# $Id$
package KNP::SynNode;
require 5.004_04; # For base pragma.
use strict;
use Encode;

=head1 NAME

KNP::SynNode - SynNode in KNP

=head1 DESCRIPTION

SynNode�γƼ������ݻ����륪�֥������ȡ�

=cut
sub new {
    my( $class, $str ) = @_;
    my $this = {};

    my $score_pat = '������';
    if( utf8::is_utf8( $str ) ){
	$score_pat = decode('euc-jp', $score_pat);
    }

    # ! 1 <SYNID:�ᤤ/������><������:1>
    # ! 1 <SYNID:s199:�Ƥ���/��������><������:0.99>
    # ! 1 <SYNID:s1201:���/���礶��><������:0.693><��̸�><���̸��:323>
    my ($tagid, $string) = (split(' ', $str))[1,2];
    my @tagids = split(',', $tagid);

    my ($synid, $score, @features);
    while ($string =~ /(<.+?>)/g) {
	my $s = $1;
	# SYNID
	if ($s =~ /<SYNID:(.+?)>/) {
	    $synid = $1;
	}
	elsif ($s =~ /<$score_pat:(.+?)>/) {
	    $score = $1;
	}
	# ����¾��feature
	else {
	    push @features, $s;
	}
    }

    $this->{synid} = $synid;
    $this->{tagid} = $tagid;
    $this->{tagids} = \@tagids;
    $this->{score} = $score;
    $this->{feature} = join('', @features);

    bless $this, $class;
}

=head1 METHODS

�ʲ��Υ᥽�åɤ����Ѳ�ǽ�Ǥ��롣

=over 4

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

=item synid

SynID���֤���

=cut
sub synid {
    my ($this) = @_;
    $this->{synid};
}

=item synid

���������֤���

=cut
sub score {
    my ($this) = @_;
    $this->{score};
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
