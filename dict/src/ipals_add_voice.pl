#! /usr/local/bin/jperl -- -*-Perl-*-

# def/sa-you.def ����

@name = ("�ɣ�", "�ɤ�", "ɽ��", "��̣", "����", 
	 "�ʣ�", "�ʣ�", "�ʣ�", "�ʣ�", "�ʣ�", 
	 "�գ�", "�գ�", "�գ�", "�գ�", "�գ�",
	 "�㣱", "�㣲", "�㣳", "�㣴", "�㣵",
	 "�֣�", "�֣�", "�֣�", "�֣�", "�֣�", "�֣�", "�֣�");

$IPALV_FIELD_NUM = 27;

while( 1 ) {
 
    $WO = 0;
    for ($i = 0; $i < $IPALV_FIELD_NUM; $i++) {

	if (!($_ = <STDIN>)) {exit;}

	chop;
	($field_name, $data[$i]) = split;
	if ($field_name =~ /^��/ && $data[$i] =~ /��/) {
	    $WO = 1;
	}

	if ($i == 20) {
	    if ($WO) {
		print "$name[$i] �˻���\n";
	    } else {
		print "$name[$i] ����򡤥˻���\n";
	    }
	} elsif ($i == 21) {
	    if ($WO) {
		print "$name[$i] ľ�����ּ�\n";
	    } else {
		print "$name[$i] �ּ�\n";
	    }
	} elsif ($i == 22) {
	    if ($WO) {
		print "$name[$i] ��\n";
	    } else {
		print "$name[$i] nil\n";
	    }
	} elsif ($i == 23) {
	    if ($WO) {
		print "$name[$i] �ˡ��˥�å�\n";
	    } else {
		print "$name[$i] nil\n";
	    }
	} else {
	    print "$name[$i] $data[$i]\n";
	}
    }
}




	 
while(<STDIN>) {

    chop;
    ($type, $data) = split(/\t/);

    
    if ($type eq "�ƥ�����ˡ��" &&
	substr($data, 40, 4) ne "��  ") {

	if (!$map{substr($data, 8, 12)}) {
	    printf STDERR "$_\n"; 
	}

	($yomi, $hyouki, $imi) = split(/ /, $map{substr($data, 8, 12)});
	(@item) = split(/ /, substr($data, 54));
	$num = 0;

	# ID
	print "$name[$num++] $code2\n";
	# �ɤ�
	print "$name[$num++] $yomi\n";
	# ɽ��
	print "$name[$num++] $hyouki\n";
	# ��̣
	print "$name[$num++] $imi\n";
	# �Ҹ��� (����̾��Ϥʤ�)
	print "$name[$num++] nil\n";
	# �ʥե졼��
	for ($i = 0; $i < 3; $i++) {     # "��NP"�򥹥��å�
	    for ($j = 0; $j < 4; $j++) { # ����̾��Ϻ��磴����
		$item[6+$i+$j*4] =~ s/^[^=]+=\"|\"$//g;
		if (!$item[6+$i+$j*4]) {
		    $item[6+$i+$j*4] = "nil";
		}
		printf "$name[$num++] $item[6+$i+$j*4]\n";
	    }
	    printf "$name[$num++] nil\n";
	}
	# �� (����̾��Ϥʤ�)
	for ($i = 0; $i < 7; $i++) {
	    printf "$name[$num++] nil\n";
	}
    }
}

close(TMP);

system("rm tmp.tmp");
