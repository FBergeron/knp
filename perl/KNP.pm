# -*- perl -*-
#
# KNP �� perl ����ƤӽФ��饤�֥��
#
#	Masatoshi Tsuchiya (tsuchiya@pine.kuee.kyoto-u.ac.jp)
#	Sadao Kurohasi (kuro@i.kyoto-u.ac.jp)

# ���Υ⥸�塼�����Ѥ�����ˡ�ϡ�
#
#     perldoc KNP
#
# �Ȥ������ޥ�ɤǻ��Ȥ��뤳�Ȥ��Ǥ��ޤ���

# KNP ���֥������Ȥϡ��ʲ�����������Ƥ���褦�����Ǥ���ĥϥå���Ǥ���
# �ޤ��������˵��Ҥ����ʳ��ˡ�ư�����ɬ�פʾ�����ݻ��������Ǥ��ޤޤ�
# �ޤ����ϥå�������Ƥ����Ѱդ˽񤭴�����ȸ�ư��ޤ��Τǡ��ʤ�٤�
# �᥽�åɷ�ͳ�Ǿ������Ф��褦�ˤ��Ƥ���������

#     $this->{ALL}          : ���Ϸ�̤��Τޤ�
#
#     $this->{COMMENT}      : ���Ϸ�̤ΰ����(#�ǤϤ��ޤ륳���ȹ�)
#
#     $this->{MRPH_NUM}     : �����ǿ�
#
#     $this->{MRPH}         : ��������
#                  [i]      : i���ܤη�����
#                     {midasi}      : i���ܤη����Ǥθ��Ф�
#                     {yomi}        : i���ܤη����Ǥ��ɤ�
#                     {genkei}      : i���ܤη����Ǥθ���
#                     {hinsi}       : i���ܤη����Ǥ��ʻ�
#                     {hinsi_id}    : i���ܤη����Ǥ��ʻ��ֹ�
#                     {bunrui}      : i���ܤη����Ǥκ�ʬ��
#                     {bunrui_id}   : i���ܤη����Ǥκ�ʬ���ֹ�
#                     {katuyou1}    : i���ܤη����Ǥγ��ѷ�
#                     {katuyou1_id} : i���ܤη����Ǥγ��ѷ��ֹ�
#                     {katuyou2}    : i���ܤη����Ǥγ��ѷ�
#                     {katuyou2_id} : i���ܤη����Ǥγ��ѷ��ֹ�
#                     {imis}        : i���ܤη����Ǥΰ�̣
#                     {fstring}     : i���ܤη����Ǥ����Ƥ�feature
#                     {feature}[j]  : i���ܤη����Ǥ�j���ܤ�feature
#
#     $this->{BNST_NUM}     : ʸ���
#
#     $this->{BNST}         : ʸ����
#                  [i]      : i���ܤ�ʸ��
#                     {start}       : i���ܤ�ʸ��ΤϤ���η������ֹ�
#                     {end}         : i���ܤ�ʸ��Τ������η������ֹ�
#                     {parent}      : i���ܤ�ʸ��η�����
#                     {dpndtype}    : i���ܤ�ʸ��η���Υ�����(D,P,I,A)
#                     {child}       : i���ܤ�ʸ��˷��äƤ���ʸ��ꥹ��
#                     {fstring}     : i���ܤ�ʸ������Ƥ�feature
#                     {feature}[j]  : i���ܤ�ʸ���j���ܤ�feature


package KNP;
require 5.000;
use Juman;
use strict;
use vars qw( $COMMAND $KNP_OPTION $JUMAN_OPTION $JUMAN_SERVER $JUMAN $VERBOSE $HOST $VERSION $MRPH_TYPE $BNST_TYPE );


# �ץ�������������Ѥ��������ѿ�
$COMMAND      = "/share/tool/knp/system/knp";
$HOST         = "grape";		# KNP �����С��Υۥ���̾ ( �����С������Ѥ��ʤ����϶��ˤ��Ƥ��� )
$KNP_OPTION   = "-case2 -tab";		# KNP ���Ϥ���륪�ץ����
$JUMAN_OPTION = "-e";			# Juman ���Ϥ���륪�ץ����
$JUMAN_SERVER = "grape:1";		# Juman �����С�
$VERBOSE      = 0;			# ���顼�ʤɤ�ȯ���������˷ٹ𤵤��뤿��ˤ� 1 �����ꤹ��
$JUMAN        = 0;
$VERSION      = sprintf("%d.%02d", q$Revision$ =~ /(\d+)\.(\d+)/);

$MRPH_TYPE    = '^(?:midasi|yomi|genkei|hinsi|hinsi_id|bunrui|bunrui_id|katuyou1|katuyou1_id|katuyou2|katuyou2_id|imis|fstring|feature)$';
$BNST_TYPE    = '^(?:start|end|parent|dpndtype|child|fstring|feature)$';



sub Version { $VERSION; }


sub BEGIN {
    unless( $HOST ){
	# KNP ��ҥץ����Ȥ��Ƽ¹Ԥ��Ƥ����硢ɸ����ϤΥХåե���
	# �󥰤ˤ�äƽ��Ϥ���Ťˤʤ�ʤ��褦�ˤ��뤿��Τ��ޤ��ʤ�
	require FileHandle or die;
	STDOUT->autoflush(1);
    }
}


sub DESTORY {
    my( $this ) = @_;
    &kill_knp( $this ) if $this->{KNP};
}


sub new {
    my( $this, $option );
    if( @_ == 2 ){
	# �����ˤ�äƻ��ꤵ�줿���ץ��������Ѥ��� KNP ��¹Ԥ�����
	( $this, $option ) = @_;
    } else {
	# �ǥե���ȤΥ��ץ��������Ѥ��� KNP ��¹Ԥ�����
	$this   = shift;
	$option = $KNP_OPTION;
    }

    unless( $JUMAN ){
	$JUMAN = new Juman( $JUMAN_OPTION, $JUMAN_SERVER ) || die "KNP.pm: Can't make JUMAN object\n";
    }

    if( $HOST ){
	require IO::Socket::INET or die "KNP.pm: Can't load module: IO::Socket::INET\n";
    } else {
	require Fork or die "KNP.pm: Can't load module: Fork\n";
    }

    $this = { ALL      => "",
	      COMMENT  => "",
	      ERROR    => "",
	      MRPH_NUM => 0,
	      MRPH     => [],
	      BNST_NUM => 0,
	      BNST     => [],
	      OPTION   => $option,
	      PREVIOUS => [] };

    # -i ���ץ������б�
    if( $option =~ /\-i +(\S)+/ ){
	my $pat = $1;
	$this->{PATTERN} = "\Q$pat\E";
    }

    bless $this;
    $this;
}


# ��ʸ���Ϥ�Ԥ��᥽�å�
sub parse {
    my( $this, $input ) = @_;

    $this->{ALL}      = "";
    $this->{COMMENT}  = "";
    $this->{ERROR}    = "";
    $this->{MRPH_NUM} = 0;
    $this->{MRPH}     = [];
    $this->{BNST_NUM} = 0;
    $this->{BNST}     = [];

    my $counter = 0;
  PARSE:
    # knp �� fork ���롣
    if( $HOST ){
	unless( $this->{KNP} ){
	    my $sock = new IO::Socket::INET( PeerAddr => $HOST, PeerPort => 31000, Proto => 'tcp' )
		|| die "KNP.pm: Can't connect server: host=$HOST\n";
	    $sock->timeout( 60 );
	    my $tmp = $sock->getline;
	    ( $tmp =~ /^200/ ) || die "KNP.pm: Illegal message: host=$HOST, msg=$tmp\n";
	    $sock->print( sprintf( "RUN %s\n", $this->{OPTION} ) );
	    $tmp = $sock->getline;
	    ( $tmp =~ /^200/ ) || die "KNP.pm: Configuration error: host=$HOST, msg=$tmp\n";
	    $this->{KNP} = $sock;
	}
    } else {
	unless( $this->{KNP} && $this->{KNP}->alive ){
	    $this->{KNP} = new Fork( $COMMAND, $this->{OPTION} ) || die "KNP.pm: Can't fork: command=$COMMAND\n";
	    $this->{KNP}->timeout( 60 );
	}
    }
    my( @juman ) = $JUMAN->parse( $input );
    $this->{KNP}->print( @juman );
    $counter++;

    # Parse ERROR �ʤɤ�ȯ���������˸�����Ĵ�٤뤿�ᡢ��ʸ���Ϥ���ʸ
    # ���������¸���Ƥ�����
    unshift( @{$this->{PREVIOUS}}, $input );
    splice( @{$this->{PREVIOUS}}, 10 );

    # ��ʸ���Ϸ�̤��ɤ߽Ф���
    if( $input =~ /^\#/ ){
	# "#" �ǻϤޤ�����ʸ�ξ��ϲ��Ϸ�̤��ɤ߹��ޤ��ˡ�ñ��1���֤���
	$this->{ALL} = $input;
	return 1;
    }
    my $buf = "";
    while( $_ = $this->{KNP}->getline ){
	$buf .= $_;
	last if( $buf =~ /\nEOS$/ || $this->{PATTERN} && /^$this->{PATTERN}/ );
    }
    $this->{ALL} = $buf;

    # ��ʸ���Ϸ�̤κǸ�� EOS �ΤߤιԤ�̵�����ϡ��ɤ߽Ф���˥���
    # �ॢ���Ȥ�ȯ�����Ƥ��롣
    unless( $buf =~ /\nEOS$/ || $this->{PATTERN} && $buf =~ /\n$this->{PATTERN}/ ){
 	if(( $counter==1 )&&( $VERBOSE )){
 	    print STDERR ";; TIMEOUT is occured.\n";
 	    for( my $i=$[; $this->{PREVIOUS}[$i]; $i++ ){
 		print STDERR sprintf( ";; TIMEOUT:%02d:%s\n", $i, $this->{PREVIOUS}[$i] );
 	    }
 	}
	&kill_knp( $this );
	goto PARSE if( $counter <= 1 );
	return 0;
    }

    # "Cannot detect consistent CS scopes." �Ȥ������顼�ξ��ϡ�KNP 
    # �ΥХ��Ǥ����ǽ��������Τǡ���ö KNP ��Ƶ�ư���롣
    if( $buf =~ /;; Cannot detect consistent CS scopes.\n/ ){
 	if(( $counter==1 )&&( $VERBOSE )){
 	    print STDERR ";; Cannot detect consistent CS scopes.\n";
 	    for( my $i=$[; $this->{PREVIOUS}[$i]; $i++ ){
 		print STDERR sprintf( ";; CS:%02d:%s\n", $i, $this->{PREVIOUS}[$i] );
 	    }
 	}
 	&kill_knp( $this );
 	goto PARSE if( $counter <= 1 );
    }

    # -tab ���ץ���󤬤ʤ����ϡ����Ϸ�̤�������ʤ���
    return 1 if $this->{OPTION} !~ /\-tab/;

    # ��ʸ���Ϸ�̤�������롣
    my( $mrph_num, $bnst_num, $f_string );
    ( $this->{COMMENT} ) = ( $buf =~ s/^(\#[^\n]*?)\n// );     # �ǽ�Υ�����( # S-ID:1 )�������
    for( split( /\n/,$buf ) ){
	chomp;

	if ( /^EOS$/ || $this->{PATTERN} && /^$this->{PATTERN}/) {
	    $this->{BNST}[$bnst_num - 1]{end} = $mrph_num - 1 if $bnst_num > $[;
	    last;
	}
	elsif (/^;;/) {
	    $this->{ERROR} .= $_;
	}
	elsif (/^\*/) {
	    if ($bnst_num != 0) {
		$this->{BNST}[$bnst_num - 1]{end} = $mrph_num - 1;
	    }
	    $this->{BNST}[$bnst_num]{start} = $mrph_num;
	    if( s/^\* ([\-0-9]+)([DPIA])// ){
		$this->{BNST}[$bnst_num]{parent} = $1;
		$this->{BNST}[$bnst_num]{dpndtype} = $2;
		if( ( $f_string ) = /^ (.+)$/ ){
		    # ʸ���feature
		    $this->{BNST}[$bnst_num]{fstring} = $f_string;
		    $f_string =~ s/^\<|\>$//g;
		    @{$this->{BNST}[$bnst_num]{feature}} = split(/\>\</, $f_string);
		} else {
		    @{$this->{BNST}[$bnst_num]{feature}} = ();
		}
	    } else {
		$this->{ALL}    = ";; KNP.pm : Illegal output of knp : output=$_\n" . $this->{ALL};
		$this->{ERROR} .= ";; KNP.pm : Illegal output of knp : output=$_\n"
	    }
	    $bnst_num++;
	}
	else {
	    # @{$this->{MRPH}[$mrph_num]} = split;
	    ( $this->{MRPH}[$mrph_num]{midasi},
	      $this->{MRPH}[$mrph_num]{yomi},
	      $this->{MRPH}[$mrph_num]{genkei},
	      $this->{MRPH}[$mrph_num]{hinsi},
	      $this->{MRPH}[$mrph_num]{hinsi_id},
	      $this->{MRPH}[$mrph_num]{bunrui},
	      $this->{MRPH}[$mrph_num]{bunrui_id},
	      $this->{MRPH}[$mrph_num]{katuyou1},
	      $this->{MRPH}[$mrph_num]{katuyou1_id},
	      $this->{MRPH}[$mrph_num]{katuyou2},
	      $this->{MRPH}[$mrph_num]{katuyou2_id},
	      $this->{MRPH}[$mrph_num]{imis},
	      $f_string ) = split;
	    $this->{MRPH}[$mrph_num]{fstring} = $f_string;
	    $f_string =~ s/^\<|\>$//g;
	    @{$this->{MRPH}[$mrph_num]{feature}} = split(/\>\</, $f_string);
	    $mrph_num++;
	}
    }

    $this->{MRPH_NUM} = $mrph_num;
    $this->{BNST_NUM} = $bnst_num;
    for my $i ( 0 .. ($this->{BNST_NUM}-2) ) {
	push(@{$this->{BNST}[$this->{BNST}[$i]{parent}]{child}}, $i);
    }

    # ��ʸ���Ϸ�̤ˡ�;; �ǻϤޤ륨�顼��å��������ޤޤ�Ƥ������
    return 0 if $this->{ERROR};

    return 1;
}


# parse �ؿ�����ƤӽФ���� KNP ��λ�����뤿��Υ��֥롼����
sub kill_knp {
    my( $this ) = @_;
    $this->{PREVIOUS} = [];
    if( $HOST ){
	$this->{KNP}->print( pack("c",0x0b)."\n" );
	$this->{KNP}->close;
    } else {
	$this->{KNP}->alive && $this->{KNP}->kill;
    }
    delete $this->{KNP};
    1;
}


sub all {
    my( $this ) = @_;
    $this->{ALL};
}

sub comment {
    my( $this ) = @_;
    $this->{COMMENT};
}

sub mrph_num {
    my( $this ) = @_;
    $this->{MRPH_NUM};
}

sub mrph {
    my $this = shift;
    unless( @_  ){
	$this->{MRPH};
    } else {
	my $i = shift;
	( $i =~ /[^0-9]/ )
	    and warn( "KNP.pm (mrph): Integer is required: arg=$i\n" ), return undef;
	( $i >= ( $this->{MRPH_NUM} + $[ ) )
	    and warn( "KNP.pm (mrph): Argument overflow: arg=$i\n" ), return undef;
	unless( @_ ){
	    $this->{MRPH}[$i];
	} else {
	    my $x = shift;
	    unless( @_ ){
		( $x =~ /$MRPH_TYPE/o )
		    or warn( "KNP.pm (mrph): Unknown type is specified: arg=$i, type=$x\n" ), return undef;
		$this->{MRPH}[$i]{$x};
	    } else {
		my $j = shift;
		( $j =~ /[^0-9]/ )
		    and warn "KNP.pm (mrph): Integer is required: arg=$i, type=$x, suffix=$j\n", return undef;
		( $x eq 'feature' )
		    or warn "KNP.pm (mrph): Illegal type is specified: arg=$i, type=$x, suffix=$j\n", return undef;
		$this->{MRPH}[$i]{$x}[$j];
	    }
	}
    }
}
    
sub bnst_num {
    my( $this ) = @_;
    $this->{BNST_NUM};
}

sub bnst {
    my $this = shift;
    unless( @_  ){
	$this->{BNST};
    } else {
	my $i = shift;
	( $i =~ /[^0-9]/ )
	    and warn( "KNP.pm (bnst): Integer is required: arg=$i\n" ), return undef;
	( $i >= ( $this->{BNST_NUM} + $[ ) )
	    and warn( "KNP.pm (bnst): Argument overflow: arg=$i\n" ), return undef;
	unless( @_ ){
	    $this->{BNST}[$i];
	} else {
	    my $x = shift;
	    unless( @_ ){
		( $x =~ /$BNST_TYPE/o )
		    or warn( "KNP.pm (bnst): Unknown type is specified: arg=$i, type=$x\n" ), return undef;
		$this->{BNST}[$i]{$x};
	    } else {
		my $j = shift;
		( $j =~ /[^0-9]/ )
		    and warn "KNP.pm (bnst): Integer is required: arg=$i, type=$x, suffix=$j\n", return undef;
		( $x eq 'feature' )
		    or warn "KNP.pm (bnst): Illegal type is specified: arg=$i, type=$x, suffix=$j\n", return undef;
		$this->{BNST}[$i]{$x}[$j];
	    }
	}
    }
}

1;


__END__

=head1 NAME

KNP - ��ʸ���Ϥ�Ԥ��⥸�塼��

=head1 SYNOPSIS

 use KNP;
 $knp = new KNP;
 $knp->parse( "����ʸ��ʸ���Ϥ��Ƥ���������" );
 print $knp->all;

=head1 DESCRIPTION

C<KNP> �ϡ���ʸ���ϴ� knp �� Perl �������Ѥ��뤿��Υ⥸�塼��Ǥ���

=head1 CONSTRUCTOR

=over 4

=item new ( [OPTION] )

C<KNP> ���֥������Ȥ��������ޤ��������˻��ꤵ�줿ʸ����� knp ��¹Ԥ�
����Υ��ץ����Ȥ������Ѥ��ޤ���

Examples:

   $knp = new KNP;
   $knp = new KNP( "" );
   $knp = new KNP( "-case2 -tab -helpsys" );

��������ά���줿���ϡ�"-case2 -tab" �򥪥ץ����Ȥ��� knp ��¹Ԥ�
�ޤ���

=back

=head1 METHODS

=over 4

=item parse( STRING )

STRING �ι�ʸ���Ϥ�Ԥ��ޤ���

=item all()

knp �����Ϥ�����ʸ���Ϸ�̤��Τޤޤ�ʸ������֤��᥽�åɤǤ���

=item comment()

knp �����Ϥ�����ʸ���Ϸ�̤�1���ܤ˴ޤޤ�륳���Ȥ��֤��᥽�åɤǤ���

=item mrph_num()

�����ǿ����֤��᥽�åɤǤ���

=item mrph( [ARG,TYPE,SUFFIX] )

��ʸ���Ϸ�̤η����Ǿ���˥����������뤿��Υ᥽�åɤǤ���

Examples:

   $knp->mrph;
   # ��������ά���줿���ϡ������Ǿ���Υꥹ�Ȥ���
   # �����ե���󥹤��֤���

   $knp->mrph( 1 );
   # ARG �ˤ�äơ������ܤη����Ǥξ�����֤������
   # �ꤹ�롣���ξ��ϡ�1���ܤη����Ǿ���Υϥå���
   # ���Ф����ե���󥹤��֤���

   $knp->mrph( 2, 'fstring' );
   # TYPE �ˤ�ä�ɬ�פʷ����Ǿ������ꤹ�롣���ξ�
   # �硢2���ܤη����Ǥ����Ƥ� feature ��ʸ�������
   # ����

   $knp->mrph( 3, 'feature', 4 );
   # 3���ܤη����Ǥ�4���ܤ� feature ���֤���

TYPE �Ȥ��ƻ��ꤹ�뤳�Ȥ��Ǥ���ʸ����ϼ����̤�Ǥ���

   midasi
   yomi
   genkei
   hinsi
   hinsi_id
   bunrui
   bunrui_id
   katuyou1
   katuyou1_id
   katuyou2
   katuyou2_id
   imis
   fstring
   feature

��3���� SUFFIX ���뤳�Ȥ��Ǥ���Τ� TYPE �Ȥ��� feature ����ꤷ����
��˸¤��ޤ���

=item bnst_num()

ʸ������֤��᥽�åɤǤ���

=item bnst( [ARG,TYPE,SUFFIX] )

��ʸ���Ϸ�̤�ʸ��˴ؤ���������Ф��᥽�åɤǤ���

Examples:

   $knp->bnst;
   # ��������ά���줿���ϡ�ʸ�����Υꥹ�Ȥ��Ф�
   # ���ե���󥹤��֤���

   $knp->bnst( 1 );
   # ARG �ˤ�äơ������ܤ�ʸ��ξ�����֤��������
   # ���롣���ξ��ϡ�1���ܤ�ʸ�����Υϥå������
   # �����ե���󥹤��֤���

   $knp->bnst( 2, 'fstring' );
   # TYPE �ˤ�ä�ɬ�פ�ʸ��������ꤹ�롣���ξ�硢
   # 2���ܤ�ʸ������Ƥ� feature ��ʸ������֤���

   $knp->bnst( 3, 'feature', 4 );
   # 3���ܤ�ʸ���4���ܤ� feature ���֤���

TYPE �Ȥ��ƻ��ꤹ�뤳�Ȥ��Ǥ���ʸ����ϼ����̤�Ǥ���

   start
   end
   parent
   dpndtype
   child
   fstring
   feature

��3���� SUFFIX ���뤳�Ȥ��Ǥ���Τ� TYPE �Ȥ��� feature ����ꤷ����
��˸¤��ޤ���

=back

=head1 NOTE

C<KNP> ���֥������Ȥ�ľ�ܻ��Ȥ��뤳�Ȥˤ�äƷ����Ǿ����ʸ��������
�뤳�Ȥ�Ǥ��ޤ���������ˡ�ˤĤ��Ƥϡ�source �򻲾Ȥ��Ƥ�������������
������ư��ʤɤ��򤱤뤿�ᡢ���������᥽�åɷ�ͳ�Ǿ������Ф��Ƥ�
��������

=head1 AUTHORS

=over 4

=item
�ڲ� ��̭ <tsuchiya@pine.kuee.kyoto-u.ac.jp>

=item
���� ���� <kuro@pine.kuee.kyoto-u.ac.jp>

=cut
