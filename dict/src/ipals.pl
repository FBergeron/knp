#! /usr/local/bin/jperl -- -*-Perl-*-

# def/sa-you.def ����

$id_head = "̾";

@name = ("�ɣ�", "�ɤ�", "ɽ��", "��̣", "����", 
	 "�ʣ�", "�ʣ�", "�ʣ�", "�ʣ�", "�ʣ�", 
	 "�գ�", "�գ�", "�գ�", "�գ�", "�գ�",
	 "�㣱", "�㣲", "�㣳", "�㣴", "�㣵",
	 "�֣�", "�֣�", "�֣�", "�֣�", "�֣�", "�֣�", "�֣�");

open(TMP, "> tmp.tmp");

while(<STDIN>) {

    print TMP;

    chop;
    ($type, $data) = split(/\t/);

    if ($type eq "����ʬ��") {
	$code2 = substr($data, 8, 12); 
	$yomi = substr($data, 20, 20); 
	$yomi =~ s/ //g;
	(@item) = split(/ /, substr($data, 54));
	$item[2] =~ s/^[^=]+=\"|\"$//g;
	$item[2] =~ s/��[^��]+��//g;
	$item[2] =~ s/��|��//g;
	$hyouki = $item[2];
	$item[3] =~ s/^[^=]+=\"|\"$//g;
	$imi = $item[3];
	$map{$code2} = "$yomi $hyouki $imi"; 
	# print "$map{$code2}\n";
    }
} 

close(TMP);

open(TMP, "tmp.tmp");

while(<TMP>) {

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
	print "$name[$num++] $id_head$code2\n";
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
