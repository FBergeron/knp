#! /usr/local/bin/jperl -- -*-Perl-*-

# Conversion from "ipal?.dat" to "scase.dat"

# �� ��-����ʸ�򰷤�����$frame��11�������ѹ�

$kaku_num{"��"} = 1;
$kaku_num{"��"} = 2;
$kaku_num{"��"} = 3;
$kaku_num{"��"} = 4;
$kaku_num{"����"} = 5;
$kaku_num{"��"} = 6;
$kaku_num{"���"} = 7;
$kaku_num{"��"} = 8;
$kaku_num{"�ޥ�"} = 9;
$kaku_num{"��"} = 10;
$kaku_num{"����"} = 11;

$IPALV_FIELD_NUM = 27;

while (1) {

    $frame = "00000000000";
    @frame = split(//, $frame);
    for ($i = 0; $i < $IPALV_FIELD_NUM; $i++) {

	if (!($_ = <STDIN>)) {exit;}

	chop;
	($field_name, $data[$i]) = split;
	if ($field_name =~ /^��/ && $data[$i] !~ /^nil$/) {
	    $data[$i] =~ s/��//g;
	    foreach $item (split(/��/, $data[$i])) {
		# ��-����ʸ�ΰ���
		if ($item eq "��" && $frame[$kaku_num{"��"}-1] == '1') {
		    $frame[$kaku_num{"����"}-1] = '1';
		}

		if ($kaku_num{$item}) {
		    $frame[$kaku_num{$item}-1] = '1';
		}
	    }
	}
    }
    $frame = join('', @frame);

    printf "$data[1] $frame\n";			# �ɤߤ��Ф������
    foreach $item (split(/��/, $data[2])) {
	printf "$item $frame\n";		# ɽ�����Ф������
    }
}
