#! /usr/local/bin/jperl -- -*-Perl-*-

######################################################################
# KNP��ʸ��featture��Ϳ�롼���translator	(99/09/10 kuro)
######################################################################

# ���������Τ˽񤤤Ƥ��������褤�����Ȥꤢ�����Ȥ����Τ���

$pos_cond{"����̾��"} = " [̾�� ����̾��]";
$pos_repr{"����̾��"} = "����";
$pos_cond{"����Ū̾��"} = " [̾�� ����Ū̾��]";
$pos_repr{"����Ū̾��"} = "����";

$pos_cond{"������"} = " [������]";
$pos_repr{"������"} = "��";
$pos_cond{"̾����̾�������"} = " [������ ̾����̾�������]";
$pos_repr{"̾����̾�������"} = "��";
$pos_cond{"̾�����ü�������"} = " [������ ̾�����ü�������]";
$pos_repr{"̾�����ü�������"} = "����";

$pos_cond{"����"} = " [����]";
$pos_repr{"����"} = "����";
$pos_cond{"�ʽ���"} = " [���� �ʽ���]";
$pos_repr{"�ʽ���"} = "��";
$pos_cond{"������"} = " [���� ������]";
$pos_repr{"������"} = "����";

$pos_cond{"������ֻؼ���"} = " [�ؼ��� ������ֻؼ���]";
$pos_repr{"������ֻؼ���"} = "����";
$pos_cond{"Ϣ�λ�"} = " [Ϣ�λ�]";
$pos_repr{"Ϣ�λ�"} = "�ۤ��";
$pos_cond{"Ϣ�λ���ֻؼ���"} = " [�ؼ��� Ϣ�λ���ֻؼ���]";
$pos_repr{"Ϣ�λ���ֻؼ���"} = "����";
$pos_cond{"��³��"} = " [��³��]";
$pos_repr{"��³��"} = "������";
$pos_cond{"��ư��"} = " [��ư��]";
$pos_repr{"��ư��"} = "����";

$pos_cond{"����"} = " [�ü� ����]";
$pos_repr{"����"} = "��";

######################################################################
use Juman
$juman = new Juman("-e -B"); 

######################################################################
$num = 0;
while ( <STDIN> ) {
    
    chomp;
    $num ++;
    next if (/^[\s\t]*\;/ || length($_) == 0);

    if (/^([^\t]+)[\s\t]+([^\;\t]+)[\s\t]*(\;.+)$/) {
	$pattern = $1; $feature = $2; $comment = $3;
    } elsif (/^([^\t]+)[\s\t]+([^\;\t]+)[\s\t]*$/) {
	$pattern = $1; $feature = $2; $comment = "";
    } else {
	print STDERR  "line $num is invalid; $_\n";
	next;
    }
    print "; $pattern\n";

    $pattern =~ s/^\s+//;	# ǰ�Τ���
    $pattern =~ s/\s+$//;	# ǰ�Τ���

    $pattern =~ /^(\[[^\[\]]+\])?([^\[\]]+)(\[[^\[\]]+\])?$/;
    $tmp1 = $1; $self = $2; $tmp2 = $3;
    $tmp1 =~ s/^\[|\]$//g;
    @pres = split(/ /, $tmp1);
    $tmp2 =~ s/^\[|\]$//g;
    @poss = split(/ /, $tmp2);

    # print " pre  @pres\n self $self\n post @poss\n\n";

    # �ޤ���ɽ���Ĥ���
    undef @pres_str;
    undef @poss_str;
    foreach (@pres) {
	push (@pres_str, print_bnst_cond(0, $_));
    }
    $self_str = print_bnst_cond(0, $self);
    foreach (@poss) {
	push (@poss_str, print_bnst_cond(0, $_));
    }

    print "(\n";
    # ����ʸ����ξ��
    if (@pres) {
	print "( ?*";
	for ($i = 0; $i < @pres; $i++) {
	    if ($i == 0) {
		$l_context = "";
	    } else {
		$l_context = $pres_str[$i-1];
	    }
	    if ($i == (@pres - 1)) {
		$r_context = $self_str;
	    } else {
		$r_context = $pres_str[$i+1];;
	    }
	    print_bnst_cond(1, $pres[$i], $l_context, $r_context);
	}
	print " )\n";
    } else {
	print "( ?* )\n";
    }

    # ��ʬ�ξ��
    print "( ";

    if (@pres) {
	$l_context = $pres_str[@pres_str-1];
    } else {
	$l_context = "";
    }
    if (@poss) {
	$r_context = $poss_str[0];
    } else {
	$r_context = "";
    }
    print_bnst_cond(1, $self, $l_context, $r_context);

    print " )\n";

    # ���ʸ����ξ��
    if (@poss) {
	print "( ";
	for ($i = 0; $i < @poss; $i++) {
	    if ($i == 0) {
		$l_context = $self_str;
	    } else {
		$l_context = $poss_str[$i-1];
	    }
	    if ($i == (@poss - 1)) {
		$r_context = "";
	    } else {
		$r_context = $poss_str[$i+1];;
	    }
	    print_bnst_cond(1, $poss[$i], $l_context, $r_context);
	}
	print " ?* )\n";
    } else {
	print "( ?* )\n";
    }

    print "\t$feature\n";
    print ")\n";

    print "$comment\n" if $comment;
    print "\n";
}
$juman->close;

######################################################################
sub print_bnst_cond
{
    my ($flag, $input, $l_context, $r_context) = @_;
    my ($ast_flag, $input1, $input2);

    if ($input =~ /^\((.+)\)$/) {
	$input = $1;
	$ast_flag = 1;
    } else {
	$ast_flag = 0;
    }

    if ($input =~ /^\<([^\<\>]+)\>$/) {
	$input = $1;
	if ($flag) {
	    printf "< (?*) %s >", feature2str($input);
	} else {
	    return "��";
	}
    } elsif ($input =~ /^(.+)\<([^\<\>]+)\>$/) {
	$input1 = $1; $input2 = $2;
	if ($flag) {
	    print "< (";
	    print_normal_cond(1, $input1, $l_context, $r_context);
	    printf ") %s >", feature2str($input2);
	} else {
	    return print_normal_cond(0, $input1);
	}
    } else {
	if ($flag) {
	    print "< (";
	    print_normal_cond(1, $input, $l_context, $r_context);
	    print ") >";
	} else {
	    return print_normal_cond(0, $input);
	}
    }
    print "*" if $ast_flag;
    print " ";
}

######################################################################
sub feature2str
{
    my ($input) = @_;

    $data = "(";
    foreach (split(/\|\|/, $input)) {
	s/\&\&/ /g;
	$data .= "($_)";
    }
    $data .= ")";
    return $data;
}

######################################################################
sub print_normal_cond
{
    my ($flag, $input, $l_context, $r_context) = @_;
    my (@data, @feature, @mrph);
    my ($i, $j, $k, $l, $error_flag);
    my $from_any = 1;

    if ($input =~ /^\^/) {
	$input =~ s/^\^//;
	$from_any = 0;
    }

    # ���Ǥ��Ȥ�ʬ��
    
    $input =~ s/\(/ \(/g;
    $input =~ s/\)/\) /g;
    $input =~ s/\{/ \{/g;
    $input =~ s/\}/\} /g;
    # $input =~ s/\<\</ \<\</g;
    $input =~ s/\>\>/\>\> /g;
    $input =~ s/\*/\* /g;
    $input =~ s/\?\*/ \?\*/g;
    $input =~ s/^ +| +$//g;

    # ɸ��ǡ�����$data[0][0]��

    $i = 0;
    foreach $item (split(/ +/, $input)) {
	if ($item =~ /^\((.+)\)$/) {
	    $item =  $1;
	    $feature[$i]{ast} = 1;
	}
	elsif ($item =~ /^\{(.+)\}$/) {
	    $item =  $1;
	}

	if ($item !~ /^\<\</ && $item =~ /\|/) {

	    # OR�ξ��϶����

	    $feature[$i]{or} = 1;
	    @{$part[$i]} = split(/\|/, $item);

	} else {

	    # ¾�ϼ�λ�����θ

	    @{$part[$i]} = ($item);

	    if (0 && $part[$i][0] =~ /\<\<(.+)\>\>$/) {
		$feature[$i]{result} = 
		    " [* * * * * " . feature2str($1) . "]";
		$part[$i][0] = "��";
	    }

	    if ($part[$i][0] =~ /\<\<(.+)\>\>$/) {
		$feature[$i]{lastfeature} = feature2str($1);
		$part[$i][0] =~ s/\<\<.+\>\>$//;
	    }
	    elsif ($part[$i][0] eq "?*") {
		$feature[$i]{result} = " ?*";
		$part[$i][0] = "��";
	    }
	    elsif ($part[$i][0] =~ /\*$/) {
		$feature[$i]{lastast} = 1;
		$part[$i][0] =~ s/\*$//;
	    }
	    elsif ($pos_cond{$part[$i][0]}) {
		$feature[$i]{result} = $pos_cond{$part[$i][0]};
		$part[$i][0] = $pos_repr{$part[$i][0]};
	    }
	}

	$data[0][0]{phrase} .= $part[$i][0];
	$data[0][0]{length}[$i] = length($part[$i][0]);
	$i++;
    }
    $part_num = $i;

    if ($flag == 0) {
	return $data[0][0]{phrase};
    }

    # i���ܤ�part��j���ܤθ�ˤ�����Τ�$data[i][j]��

    for ($i = 0; $i < $part_num; $i++) {
	next unless ($feature[$i]{or});
	for ($j = 1; $j < @{$part[$i]}; $j++) {
	    for ($k = 0; $k < $part_num; $k++) {
		if ($k != $i) {
		    $data[$i][$j]{phrase} .= $part[$k][0];
		    $data[$i][$j]{length}[$k] = length($part[$k][0]);
		} else {
		    $data[$i][$j]{phrase} .= $part[$k][$j];
		    $data[$i][$j]{length}[$k] = length($part[$k][$j]);
		}
	    }
	}
    }

    # ���줾��JUMAN�ǽ���

    for ($i = 0; $i < $part_num; $i++) {
	for ($j = 0; $j < @{$part[$i]}; $j++) {
	    next unless ($data[$i][$j]);
	    # print ">> ($l_context)$data[$i][$j]{phrase}($r_context)(@{$data[$i][$j]{length}})\n";

	    if ($r_context) {
		@juman_result = $juman->parse($l_context . $data[$i][$j]{phrase} . $r_context); 
	    } elsif (!$r_context && $data[$i][$j]{phrase} =~ /(��|��)$/) {
		@juman_result = $juman->parse($l_context . $data[$i][$j]{phrase} . "��"); 
	    } else {
		@juman_result = $juman->parse($l_context . $data[$i][$j]{phrase} . "����"); 
	    }
	    $k = 0;
	    $length = 0;
	    $begin_check = 0;
	    foreach $item (@juman_result) { 
		next if ($item =~ /^EOS/);
		next if ($item =~ /^\@/);
		@tmp_mrph = split(/ /, $item);

		if ($length < length($l_context)) {
		    $length += length($tmp_mrph[0]);
		    next;
		} elsif ($length < (length($l_context) + length($data[$i][$j]{phrase}))) {
		    if ($begin_check == 0) {
			if ($length != length($l_context)) {
			    print "CONTEXT ERROR @tmp_mrph\n";
			    return;
			} else {
			    $begin_check = 1;
			}
		    }

		    $length += length($tmp_mrph[0]);
		    ($mrph[$i][$j][$k]{word}, 
		     $mrph[$i][$j][$k]{yomi}, 
		     $mrph[$i][$j][$k]{base}, 
		     $mrph[$i][$j][$k]{pos}, 
		     $mrph[$i][$j][$k]{d1}, 
		     $mrph[$i][$j][$k]{pos2}, 
		     $mrph[$i][$j][$k]{d2}, 
		     $mrph[$i][$j][$k]{conj}, 
		     $mrph[$i][$j][$k]{d3}, 
		     $mrph[$i][$j][$k]{conj2}, 
		     $mrph[$i][$j][$k]{d4}) = @tmp_mrph;
		    $mrph[$i][$j][$k]{length} = length($mrph[$i][$j][$k]{word});

		    # �����Ǻ��Τϥ����å��Τ������
		    $mrph[$i][$j][$k]{result} = 		    
			" [$mrph[$i][$j][$k]{pos} $mrph[$i][$j][$k]{pos2} $mrph[$i][$j][$k]{conj} $mrph[$i][$j][$k]{conj2} $mrph[$i][$j][$k]{base}]";
		    $k++;
		} else {
		    if ($length != (length($l_context) + length($data[$i][$j]{phrase}))) {
			print "CONTEXT ERROR @tmp_mrph\n";
			return;
		    }
		}
	    }

	    # part �� mrph ���б��Ĥ� (���ˤ�ä�OR��ʸ�������㤦�Τǳ�$data[$i][$j]��ɬ��)
	    $part_length = 0;
	    $mrph_length = 0;
	    $k = 0;
	    for ($l = 0; $l < $part_num; $l++) {
		$part_length += $data[$i][$j]{length}[$l];
		$data[$i][$j]{start}[$l] = $k;
		for (; $mrph_length < $part_length; $k++) {
		    $mrph_length += $mrph[$i][$j][$k]{length};
		}
		$data[$i][$j]{end}[$l] = $k - 1;
		# print "($data[$i][$j]{start}[$l] $data[$i][$j]{end}[$l])";
	    }
	}
    }

    # OR����ʬ�Υޡ���

    $start_pos = 0;
    $error_flag = 0;
    for ($i = 0; $i < $part_num; $i++) {
	if ($feature[$i]{or}) {
	    for ($j = 1; $j < @{$part[$i]}; $j++) {

		# OR������η����ǿ��ΰ��ס� OR���������
		if ($data[0][0]{start}[$i] != $data[$i][$j]{start}[$i] ||
		    $data[0][0]{end}[$i] != $data[$i][$j]{end}[$i] ||
		    $data[0][0]{start}[$i] != $data[0][0]{end}[$i] ||
		    $data[$i][$j]{start}[$i] != $data[$i][$j]{end}[$i] ||
		    @{$mrph[0][0]} != @{$mrph[$i][$j]}) {
		    $error_flag = 1;
		}
		# OR�����η�������ΰ���
		for ($k = 0; $k < $data[0][0]{start}[$i]; $k++) {
		    if ($mrph[0][0][$k]{result} ne $mrph[$i][$j][$k]{result}) {
			$error_flag = 1;
			break;
		    }
		}
		# OR�θ�η�������ΰ���
		for ($k = $data[0][0]{end}[$i]+1; $k < @{$mrph[0][0]}; $k++) {
		    if ($mrph[0][0][$k]{result} ne $mrph[$i][$j][$k]{result}) {
			$error_flag = 1;
			break;
		    }
		}
		
		# OR��Ʊ���ʻ줫
		if ($mrph[0][0][$data[0][0]{start}[$i]]{pos} ne $mrph[$i][$j][$data[$i][$j]{start}[$i]]{pos}) {
		    $error_flag = 1;
		    if ($mrph[0][0][$data[0][0]{start}[$i]]{conj} eq "*" && 
			$mrph[$i][$j][$data[$i][$j]{start}[$i]]{conj} ne "*") {
			# ���Ѹ�Ȥ����Ǥʤ���Τδ֤ϼ㴳�ۼ�
			$mrph[$i][$j][$data[$i][$j]{start}[$i]]{base} = $mrph[$i][$j][$data[$i][$j]{start}[$i]]{word}
		    }
		}

		$WORD = $mrph[0][0][$data[0][0]{start}[$i]]{base} if ($j == 1);
		$WORD .= " $mrph[$i][$j][$data[$i][$j]{start}[$i]]{base}";
	    }
	    # print "$WORD\n";
	    $mrph[0][0][$data[0][0]{start}[$i]]{base} = "($WORD)";
	}
	$start_pos += $data[0][0]{length}[$i];
    }

    # OR�ξ����������ʤ���Х��顼����
    if ($error_flag) {
	for ($i = 0; $i < $part_num; $i++) {
	    for ($j = 0; $j < @{$part[$i]}; $j++) {
		next unless ($data[$i][$j]);
		print STDERR "ERROR($i,$j) ";
		for ($k = 0; $k < @{$mrph[$i][$j]}; $k++) {
		    print STDERR "$mrph[$i][$j][$k]{result}";
		}
		print STDERR "\n";
	    }
	}
	print STDERR "\n";
    }

    # ����

    print " ?*" if ($from_any);

    for ($i = 0; $i < $part_num; $i++) {
	if ($feature[$i]{result}) {
	    print $feature[$i]{result};
	} else {
	    for ($k = $data[0][0]{start}[$i]; $k <= $data[0][0]{end}[$i]; $k++) {
		if (($mrph[0][0][$k]{base} eq "��" || $mrph[0][0][$k]{base} eq "�ɤ�") && 
		    $k == $feature[$i]{end} &&
		    $feature[$i]{lastast}) {
		    $mrph[0][0][$k]{result} = " [* * * * * ((���Ѹ�))]";
		}
		elsif ($mrph[0][0][$k]{base} eq "��" || $mrph[0][0][$k]{base} eq "�ɤ�") {
		    $mrph[0][0][$k]{result} = " [* * (�첻ư�� �Ҳ�ư�쥫�� �Ҳ�ư�쥫��¥���ط� �Ҳ�ư�쥬�� �Ҳ�ư�쥵�� �Ҳ�ư�쥿�� �Ҳ�ư��ʹ� �Ҳ�ư��й� �Ҳ�ư��޹� �Ҳ�ư���� �Ҳ�ư���ԥ��� �Ҳ�ư���� �Ҳ�ư����ʸ�첻�ط� ����ư�� ����ư���� ����ư�� ����ư�� ư�����������ޤ��� ư�������������뷿 ư�������������뷿 ̵���ѷ�) $mrph[0][0][$k]{conj2} *]";
		}
		elsif ($mrph[0][0][$k]{base} eq "������") {
		    $mrph[0][0][$k]{result} = " [* * (�����ƻ쥢������ �����ƻ쥤�� �����ƻ쥤���ü� ��ư��̷� ��ư�줯��) $mrph[0][0][$k]{conj2} *]";
		}
		elsif ($mrph[0][0][$k]{base} eq "�Ť���" || $mrph[0][0][$k]{base} eq "���̤�") {
		    $mrph[0][0][$k]{result} = " [* * (�ʷ��ƻ� �ʷ��ƻ��ü� �ʥη��ƻ� Ƚ��� ��ư��̷� ��ư������� ��ư�줽������) $mrph[0][0][$k]{conj2} *]";
		}
		elsif ($mrph[0][0][$k]{base} eq "Ʋ������") {
		    $mrph[0][0][$k]{result} = " [* * (������ƻ�) $mrph[0][0][$k]{conj2} *]";
		}
		elsif ($mrph[0][0][$k]{base} eq "����" &&
		       $k == $feature[$i]{end} &&
		       $feature[$i]{lastast}) {
		    $mrph[0][0][$k]{result} = " [* * * * * ((^���Ѹ�))]";
		}
		elsif ($mrph[0][0][$k]{base} eq "����") {
		    $mrph[0][0][$k]{result} = " [* * * * * ((̾��������))]";
		}
		elsif ($mrph[0][0][$k]{base} eq "������") {
		    $mrph[0][0][$k]{result} = " [���� * * * *]";
		}
		elsif ($mrph[0][0][$k]{base} eq "����") {
		    $mrph[0][0][$k]{result} = " [̾�� ����̾�� * * *]";
		}
		elsif ($mrph[0][0][$k]{base} eq "��") {
		    $mrph[0][0][$k]{result} = " [�ü� ���� * * *]";
		}
		elsif ($mrph[0][0][$k]{base} eq "��") {
		    $mrph[0][0][$k]{result} = " [�ü� ���� * * *]";
		}
		elsif ($mrph[0][0][$k]{base} eq "��") {
		    $mrph[0][0][$k]{result} = " [�ü� ��̻� * * *]";
		}
		elsif ($mrph[0][0][$k]{base} eq "��") {
		    $mrph[0][0][$k]{result} = " [�ü� ��̽� * * *]";
		}
		elsif ($mrph[0][0][$k]{base} eq "��") {
		    $mrph[0][0][$k]{result} = "";
		}
		else {
		    $mrph[0][0][$k]{pos2} = "*" if ($mrph[0][0][$k]{pos} eq "̾��");
		    $mrph[0][0][$k]{pos2} = "*" if ($mrph[0][0][$k]{pos} eq "����");
		    $mrph[0][0][$k]{pos2} = "*" if ($mrph[0][0][$k]{pos} eq "����");
		    $mrph[0][0][$k]{conj2} = "*" if ($feature[$i]{lastast});
		    $mrph[0][0][$k]{result} =
			" [$mrph[0][0][$k]{pos} $mrph[0][0][$k]{pos2} $mrph[0][0][$k]{conj} $mrph[0][0][$k]{conj2} $mrph[0][0][$k]{base}]";
		}

		# �����Ǥ�feature�����ꤵ��Ƥ�����
		if ($feature[$i]{lastfeature}) {
		    if ($mrph[0][0][$k]{result}) {
			$mrph[0][0][$k]{result} =~ s/\]$/ $feature[$i]{lastfeature}]/;
		    }
		    else {
			$mrph[0][0][$k]{result} = " [* * * * * $feature[$i]{lastfeature}]";
		    }
		}

		print $mrph[0][0][$k]{result};
		print "*" if ($feature[$i]{ask});
	    }
	}

	if ($feature[$i]{ast}) {
	    print "*";
	}
    }
}

######################################################################
sub print_juman
{
    my ($input) = @_;
    my ($data, $item, @juman_result);
    $data = "";

    @juman_result = $juman->parse($input);

    foreach $item (@juman_result) {
        next if ($item =~ /^EOS/);
        next if ($item =~ /^\@/);
        ($word, $yomi, $base, $pos, $d1, $pos2, $d2, $conj, $d3, $conj2, $d4)
            = split(/ /, $item);
        $data .= "  [$pos $pos2 $conj $conj2 ($base)] \n";
    }
    print $data;
}
######################################################################
