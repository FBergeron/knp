package KNP::File;
require 5.000;
use Exporter;
use FileHandle;
use POSIX qw/ SEEK_SET O_RDONLY O_CREAT /;
use KNP::Result;
use strict;
use vars qw/ @ISA $PATTERN /;
@ISA = qw/ Exporter /;

=head1 NAME

KNP::File - ��ʸ���Ϸ�̤γ�Ǽ����Ƥ���ե����������⥸�塼��

=head1 SYNOPSIS

 $knp = new KNP::File( $file ) or die;
 while( $result = $knp->each() ){
     print $result->spec;
 }

=head1 CONSTRUCTOR

=over 4

=item new ( FILE )

��ʸ���Ϸ�̤γ�Ǽ����Ƥ���ե��������ꤷ�ơ����֥������Ȥ�������
�롥

=item new ( OPTIONS )

��ĥ���ץ�������ꤷ�ƥ��֥������Ȥ��������롥�㤨�С���ʸ���Ϸ��
�ե�����˴ޤޤ�Ƥ�����Ϸ�̤�ʸ ID �Υǡ����١����ե��������ꤹ��
ɬ�פ�������ˤϡ��ʲ��Τ褦�˻��ꤹ�뤳�Ȥ��Ǥ��롥

  Example:

    $knp = new KNP::File( file => 'path_to_file',
                          dbfile => 'path_to_dbfile' );

=cut
sub new {
    my $class = shift;
    my %opt;
    if( @_ == 1 ){
	$opt{file} = shift;
    } else {
	while( @_ ){
	    my $key = shift;
	    my $val = shift;
	    $key =~ s/^-+//;
	    $opt{lc($key)} = $val;
	}
    }

    my $name   = $opt{file};
    my $dbname = $opt{dbfile} || $name.'.db';
    my $bclass = $opt{bclass} || $KNP::Result::DEFAULT{bclass};
    my $mclass = $opt{mclass} || $KNP::Result::DEFAULT{mclass};
    my $tclass = $opt{tclass} || $KNP::Result::DEFAULT{tclass};

    if( my $fh = new FileHandle( $name, "r" ) ){
	my $new = { name   => $name,
		    dbname => $dbname,
		    bclass => $bclass,
		    mclass => $mclass,
		    tclass => $tclass,
		    _file_handle => $fh };
	bless $new, $class;
    } else {
	undef;
    }
}

=back

=head1 METHODS

=over 4

=item name

���Ȥ��Ƥ���ե�����Υե�����̾���֤���

=cut
sub name {
    my( $this ) = @_;
    $this->{name};
}

=item each

��Ǽ����Ƥ��빽ʸ���Ϸ�̤�ʸ��ñ�̤Ȥ��ƽ���֤���

=cut
$PATTERN ='^EOS$';

sub each {
    my( $this ) = @_;
    my $fh = $this->{_file_handle};
    $this->setpos( 0 )
	unless $this->{_each_pos} and ( $this->getpos == $this->{_each_pos} );
    my @buf;
    while( <$fh> ){
	push( @buf, $_ );
	if( m!$PATTERN! ){
	    $this->{_each_pos} = $this->getpos;
	    return &_result( $this, \@buf );
	}
    }
    $this->{_each_pos} = 0;
    undef;
}

sub _result {
    my( $this, $spec ) = @_;
    KNP::Result->new( result  => $spec,
		      pattern => $PATTERN,
		      bclass  => $this->{bclass},
		      mclass  => $this->{mclass},
		      tclass  => $this->{tclass} );
}

=item look

ʸ ID ����ꤷ�ơ���ʸ���Ϸ�̤���Ф���

=cut
sub look {
    my( $this, $sid ) = @_;
    unless( $this->{_db} ){
	my %db;
	if( -f $this->dbname ){
	    require DB_File;
	    tie( %db, 'DB_File', $this->dbname, O_RDONLY ) or return undef;
	} else {
	    &_make_hash( $this, \%db );
	}
	$this->{_db} = \%db;
    }
    if( my $spec = $this->{_db}->{$sid} ){
	my( $pos, $len ) = split( /,/, $spec );
	$this->setpos( $pos );
	read( $this->{_file_handle}, $spec, $len );
	&_result( $this, $spec );
    } else {
	undef;
    }
}

=item makedb

�ե�����˴ޤޤ�Ƥ��빽ʸ���Ϸ�̤�ʸ ID �Υǡ����١�����������롥

=cut
sub makedb {
    my( $this ) = @_;

    my %db;
    require DB_File;
    tie( %db, 'DB_File', $this->dbname, O_CREAT ) or return 0;
    &_make_hash( $this, \%db ) or return 0;
    untie %db;
    1;
}

# ʸ ID ��Ϣ�������������������ؿ�
sub _make_hash {
    my( $this, $hash ) = @_;

    %$hash = ();			# Ϣ�����������
    $this->setpos( 0 ) or return 0;
    my $pos = 0;
    my( $end, $result );
    while( $result = $this->each() ){
	$end = $this->getpos;
	$result->id and $hash->{ $result->id } = sprintf( "%d,%d", $pos, ( $end - $pos ) );
	$pos = $end;
    }
    1;
}

=item dbname

ʸ ID �ǡ����١����Υե�����̾���֤���

=cut
sub dbname {
    my( $this ) = @_;
    $this->{dbname};
}

=item getpos

�����Ƥ���ե�����θ��ߤΥե�����ݥ��󥿤ΰ��֤��֤���

=cut
sub getpos {
    my( $this ) = @_;
    my $fh = $this->{_file_handle};
    $fh->tell;
}

=item setpos ( POS )

�����Ƥ���ե�����Υե�����ݥ��󥿤ΰ��֤� C<POS> �˰�ư���롥������
�ˤ� 1 �򡤼��Ի��ˤ� 0 ���֤���

=cut
sub setpos {
    my( $this, $pos ) = @_;
    my $fh = $this->{_file_handle};
    $fh->seek( $pos, SEEK_SET );
}

=back

=head1 SEE ALSO

=over 4

=item *

L<KNP::Result>

=back

=head1 AUTHOR

=over 4

=item
�ڲ� ��̭ <tsuchiya@pine.kuee.kyoto-u.ac.jp>

=cut

1;
