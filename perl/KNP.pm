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
use vars qw( $COMMAND $KNP_OPTION $JUMAN_OPTION $JUMAN $VERBOSE );
no strict 'refs';


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
my $FH        = "KNP00000";
my $TIMEOUT   = 300;



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

    $this = { ALL      => "",
	      COMMENT  => "",
	      ERROR    => "",
	      MRPH_NUM => 0,
	      MRPH     => [],
	      BNST_NUM => 0,
	      BNST     => [],
	      PID      => 0,
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
    &kill_knp( $this ) if( $this->{PID} );
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
    unless( $this->{PID} ){
	# knp �� fork ���롣
	my( $pid, $write, $read ) = &fork( "$COMMAND $this->{OPTION}" );
	$this->{PID}   = $pid;
	$this->{WRITE} = $write;
	$this->{READ}  = $read;
    }
    $this->{WRITE}->print( $JUMAN->parse( $input ) );
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
    while( $_ = &read( $this->{READ}, $TIMEOUT ) ){
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
    $this->{WRITE}->close;
    $this->{READ}->close;
    sleep 1;
    kill 15, $this->{PID};
    sleep 1;
    kill 9, $this->{PID};
    $this->{PID} = 0;
    1;
}


# ���ꤵ�줿���ޥ�ɤ� fork ���ơ�
#     (1) ���Υ��ޥ�ɤ� PID
#     (2) ���Υ��ޥ�ɤ�ɸ�����ϤΥե�����ϥ�ɥ�
#     (3) ���Υ��ޥ�ɤ�ɸ����Ϥ����ɸ�२�顼���ϤΥե�����ϥ�ɥ�
# �Ȥ���3�Ĥ����Ǥ���ʤ�������֤��ؿ�
sub fork {
    my $command = shift;

    my $parent_read  = ++$FH;
    my $child_write  = ++$FH;
    pipe $parent_read, $child_write;

    my $parent_write = ++$FH;
    my $child_read   = ++$FH;
    pipe $child_read, $parent_write;

  FORK: {
	if( my $pid = fork ){
	    # �ƥץ���¦�ν���
	    close $child_read;
	    close $child_write;
	    $parent_write->autoflush(1);
	    return ( $pid, $parent_write, $parent_read );
	} elsif( defined $pid ){
	    # �ҥץ���¦�ν���
	    close $parent_write;
	    open \*STDIN, "<&$child_read";
	    close $child_read;
	    close $parent_read;
	    open \*STDOUT, ">&$child_write";
	    open \*STDERR, ">&$child_write";
	    close $child_write;
	    exec "$command";
	    exit 0;
	} elsif( $! =~ /No more process/ ){
	    sleep 5;
	    redo FORK;
	} else {
	    die "Can't fork: $!\n";
	}
    }
}


# ���ꤵ�줿�ե�����ϥ�ɥ뤫�顢�����ॢ���ȤĤ����ɤ߹��ߤ�Ԥ��ؿ�
sub read {
    my( $fh, $timeout ) = @_;
    my $buf;
    $SIG{ALRM} = sub { die "SIGALRM is received\n"; };
    eval {
	alarm $timeout;
	$buf = <$fh>;
	alarm 0;
    };
    if( $@ =~ /SIGALRM is received/ ){
	return undef;
    }
    $buf;
}


1;
