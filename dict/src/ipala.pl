#! /usr/local/bin/jperl -- -*-Perl-*-

# keiyou/files.doc ����

$id_head = "��";

@name = ("�ɣ�", "�ɤ�", "ɽ��", "��̣", "����", 
	 "�ʣ�", "�ʣ�", "�ʣ�", "�ʣ�", "�ʣ�", 
	 "�գ�", "�գ�", "�գ�", "�գ�", "�գ�",
	 "�㣱", "�㣲", "�㣳", "�㣴", "�㣵",
	 "�֣�", "�֣�", "�֣�", "�֣�", "�֣�", "�֣�", "�֣�");

open(TABLE, $ARGV[0]) || die;

while(<TABLE>) {
    ~ s/\"//g;
    (@item) = split(/,/);
    $item[6] =~ s/��[^��]+��//g;
    $item[6] =~ s/��|��//g;
    $hyouki{substr($item[0], 0, 10)} = $item[6];
    $meaning{substr($item[0], 0, 10)} = $item[24];
    # print substr($item[0], 0, 10);
}

close(TABLE);

open(DATA, $ARGV[1]) || die;

while(<DATA>) {

    chop;
    ~ s/\"\"/nil/g;
    ~ s/\"//g;
    (@item) = split(/,/);

    $num = 0;

    # ID
    print "$name[$num++] $id_head$item[0]\n";
    # �ɤ�
    print "$name[$num++] $item[1]\n";
    # ɽ��
    print "$name[$num++] $hyouki{substr($item[0], 0, 10)}\n";
    # ��̣
    print "$name[$num++] $meaning{substr($item[0],0,10)}\n";
    # �Ҹ���
    print "$name[$num++] $item[7]\n";
    # �ʥե졼��
    for ($i = 1; $i < 4; $i++) {     # "�ΣФ�ź��"�򥹥��å�
	for ($j = 0; $j < 3; $j++) { # ���ƻ�Ϻ��磳����
	    printf "$name[$num++] $item[8+$i+$j*4]\n";
	}
	printf "$name[$num++] nil\n";
	printf "$name[$num++] nil\n";
    }
    # �� (���ƻ�Ϥʤ�)
    for ($i = 0; $i < 7; $i++) {
	printf "$name[$num++] nil\n";
    }
}
