#! /usr/local/bin/jperl -- -*-Perl-*-

# dousi1/files.doc ����

$id_head = "ư";

@name = ("�ɣ�", "�ɤ�", "ɽ��", "��̣", "����", 
	 "�ʣ�", "�ʣ�", "�ʣ�", "�ʣ�", "�ʣ�", 
	 "�գ�", "�գ�", "�գ�", "�գ�", "�գ�",
	 "�㣱", "�㣲", "�㣳", "�㣴", "�㣵",
	 "�֣�", "�֣�", "�֣�", "�֣�", "�֣�", "�֣�", "�֣�");

while(<STDIN>) {

    chop;
    ~ s/\"\"/nil/g;
    ~ s/\"//g;
    (@item) = split(/,/);

    $item[0] =~ s/��//g;
    $item[14] =~ s/��[^��]+��//g;

    $num = 0;

    # ID
    print "$name[$num++] $id_head$item[0]$item[1]$item[2]\n";
    # �ɤ�
    print "$name[$num++] $item[0]\n";
    # ɽ��
    print "$name[$num++] $item[14]\n";
    # ��̣
    print "$name[$num++] $item[5]\n";
    # �Ҹ���
    print "$name[$num++] $item[31]\n";
    # �ʥե졼��
    for ($i = 0; $i < 3; $i++) {
	for ($j = 0; $j < 5; $j++) {
	    printf "$name[$num++] $item[32+$i+$j*3]\n";
	}
    }
    # ��
    for ($i = 0; $i < 7; $i++) {
	printf "$name[$num++] $item[47+$i]\n";
    }
}
