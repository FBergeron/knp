#! /usr/local/bin/jperl -- -*-Perl-*-

# Conversion from "JWD.DIC" (version 1.0) to "scase.dat"

# �� ��-����ʸ�򰷤�����$frame��11�������ѹ�

# ���쥳���ɤξ��� 
#      �쥳�����ֹ� \t ñ�츫�Ф� \t ���Ѳ���-Ϣ��°���� \t 
#       ����ɽ�� \t ȯ�� \t �ʻ� \t ��ʸ�� \t ���Ѿ��� \t ɽ�سʾ��� \t
#       ����� \t ��ǽ����� \t ��ǰ���̻� \t �Ѹ쳵ǰ���Ф� \t
#       ���ܸ쳵ǰ���Ф� \t �Ѹ쳵ǰ���� \t ���ܸ쳵ǰ���� \t
#       ��ˡ \t ���� \t �������� \n

<STDIN>;			# skip header ("Copyright ... ")

while(<STDIN>) {

    (@record) = split(/\t/);

    if ($record[8] !~ /^""$/) {

	if ($record[5] =~ /JAP|JMP|JPR/) {
	    next;
	} elsif ($record[5] !~ /JVE|JAJ|JAM/) {
	    # printf STDERR "$record[1] $record[5] $record[8]\n";
	    next;
	} 

	$record[1] =~ /^(.*)\[(.*)\]$/;
	$hyouki = $1;
	$yomi = $2;
	$yomi =~ s/��//g;
	$yomi =~ tr/��-��/��-��/;

	$frame = "00000000000";
	@frame = split(//, $frame);
	if ($record[8] =~ /JK01/) {$frame[0] = '1';}
	if ($record[8] =~ /JK02/) {$frame[1] = '1';}
	if ($record[8] =~ /JK03/) {$frame[2] = '1';}
	if ($record[8] =~ /JK04/) {$frame[3] = '1';}
	if ($record[8] =~ /JK05/) {$frame[4] = '1';}
	if ($record[8] =~ /JK06/) {$frame[5] = '1';}
	if ($record[8] =~ /JK07/) {$frame[6] = '1';}
	if ($record[8] =~ /JK08/) {$frame[7] = '1';}
	if ($record[8] =~ /JK09/) {$frame[8] = '1';}
	if ($record[8] =~ /JK10/) {$frame[9] = '1';}
	$frame = join('', @frame);

	$yomi =~ s/����$//;	# ����̾��������Ρ֤���פ���
	$hyouki =~ s/����$//;

	print "$yomi $frame\n";
	print "$hyouki $frame\n";
    }
}

