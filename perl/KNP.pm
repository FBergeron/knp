#
# KNP��perl����ƤӽФ��饤�֥�� (Ver 0.1)
#
#		Masatoshi Tsuchiya (tsuchiya@pine.kuee.kyoto-u.ac.jp)
#		Sadao Kurohasi (kuro@i.kyoto-u.ac.jp)
#

# $this->{ALL}		: ���Ϸ�̤��Τޤ�
#
# $this->{COMMENT}	: ���Ϸ�̤ΰ����(#�ǤϤ��ޤ륳���ȹ�)
#
# $this->{MRPH_NUM}	: �����ǿ�
#
# $this->{MRPH}		: ��������
#              [i]	: i���ܤη�����
#                 {midasi}	: i���ܤη����Ǥθ��Ф�
#                 {yomi}	: i���ܤη����Ǥ��ɤ�
#                 {genkei}	: i���ܤη����Ǥθ���
#                 {hinsi}	: i���ܤη����Ǥ��ʻ�
#                 {hinsi_id}	: i���ܤη����Ǥ��ʻ��ֹ�
#                 {bunrui}	: i���ܤη����Ǥκ�ʬ��
#                 {bunrui_id}	: i���ܤη����Ǥκ�ʬ���ֹ�
#                 {katuyou1}	: i���ܤη����Ǥγ��ѷ�
#                 {katuyou1_id}	: i���ܤη����Ǥγ��ѷ��ֹ�
#                 {katuyou2}	: i���ܤη����Ǥγ��ѷ�
#                 {katuyou2_id}	: i���ܤη����Ǥγ��ѷ��ֹ�
#                 {imis}	: i���ܤη����Ǥΰ�̣
#                 {feature}[j]	: i���ܤη����Ǥ�j���ܤ�feature
#
# $this->{BNST_NUM}	: ʸ���
#
# $this->{BNST}		: ʸ����
#              [i]	: i���ܤ�ʸ��
#                 {start}	: i���ܤ�ʸ��ΤϤ���η������ֹ�
#                 {end}		: i���ܤ�ʸ��Τ������η������ֹ�
#                 {parent}	: i���ܤ�ʸ��η�����
#                 {dpndtype}	: i���ܤ�ʸ��η���Υ�����(D,P,I,A)
#                 {child}	: i���ܤ�ʸ��˷��äƤ���ʸ��ꥹ��
#                 {feature}[j]	: i���ܤ�ʸ���j���ܤ�feature
#
# ������
#
# use KNP;
# $knp = new KNP();
# while ( <STDIN> ) {
#    chomp;
#    $knp->parse($_);
# }


package KNP;
require 5.000;
use FileHandle;
use Juman;
use strict;
use vars qw( $COMMAND $KNP_OPTION $JUMAN_OPTION $JUMAN $VERBOSE $HOST );


# �ץ�������������Ѥ��������ѿ�
$COMMAND = "";				# KNP �Υѥ�̾
if( $ENV{OS_TYPE} eq "Solaris" ){
    $COMMAND = "/share/tool/knp/system/knp";
} else {
    die "Only Solaris is supported now !";
}
$KNP_OPTION   = "-case2 -tab";		# KNP ���Ϥ���륪�ץ����
$JUMAN_OPTION = "-e";			# Juman ���Ϥ���륪�ץ����
$VERBOSE      = 0;			# ���顼�ʤɤ�ȯ���������˷ٹ𤵤��뤿��ˤ� 1 �����ꤹ��
$JUMAN        = 0;
$HOST         = "grape";



sub BEGIN {
    # ���Ϥ���Ťˤʤ�ʤ��褦�ˤ��뤿��Τ��ޤ��ʤ�
    STDOUT->autoflush(1);
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
	$JUMAN = new Juman( $JUMAN_OPTION, "grape:1" ) or die;
    }

    if( $HOST ){
	require IO::Socket::INET or die;
    } else {
	require Fork or die;
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


# �ƤӽФ��� KNP ��λ����᥽�å�
sub DESTORY {
    my( $this ) = @_;
    &kill_knp( $this ) if( $this->{KNP}->alive );
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
		    $f_string =~ s/^\<//;
		    $f_string =~ s/\>$//g;
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
	$this->{KNP}->close;
    } else {
	$this->{KNP}->kill;
    }
    delete $this->{KNP};
    1;
}


1;
