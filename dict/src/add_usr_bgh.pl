#!/usr/bin/env perl

# $Id$

use strict;
use encoding 'euc-jp';
binmode STDERR, ':encoding(euc-jp)';
binmode DB::OUT, ':encoding(euc-jp)';
use KNP;

# ʬ�����ɽ�˥���ȥ���ɲä��륹����ץ�

# usage:
# ./add_usr_bgh.pl bgh.dat < usr_bgh.dat
 
# usr_bgh.dat����
# �����ѥ�=���夦��

my $knp = new KNP;

my %word2code;

open F, "<:encoding(euc-jp)", $ARGV[0] or die;

# bgh.orig���ɤ߹���
while (<F>) {
    print;

    chomp;

    my ($word, $code) = split;

    $word2code{$word}{$code} = 1;
}
close F;

# usr_bgh.dat���ɤ߹���
while (<STDIN>) {
    chomp;

    next if /\#/ || $_ eq "";

    my ($newword, $word);

    if (/^(\S+)=(\S+)$/) {
	($newword, $word) = ($1, $2);
    }
    else {
	print STDERR "Format Error: $_\n";
	next;
    }

    my ($new_repname, $repname);
    $new_repname = $newword =~ /\// ? $newword : &get_repname($newword);
    $repname = $word =~ /\// ? $word : &get_repname($word);

    # ���Ǥ˥���ȥ꤬����
    if (defined $word2code{$new_repname}) {
	my $outputted_flag = 0;
	foreach my $code (keys %{$word2code{$repname}}) {
	    unless (defined $word2code{$new_repname}{$code}) {
		print "$new_repname $code\n";
		$outputted_flag = 1;
	    }		
	}

	print STDERR "!!$newword is already registered\n" unless $outputted_flag;
    }
    
    elsif (defined $word2code{$repname}) {
	foreach my $code (keys %{$word2code{$repname}}) {
	    print "$new_repname $code\n";
	}
	
	# �ɲä�����Τ���Ͽ���Ƥ���
	$word2code{$new_repname} = $word2code{$repname};
    }
    else {
	print STDERR "!!Not Found: $word($repname)\n";
    }
}

sub get_repname {
    my ($word) = @_;

    my $result = $knp->parse($word);

    if ($result && $result->bnst == 1) {
	# ۣ������������(?��Ϣ�뤵��Ƥ�����)�����ֺǽ������Ȥ�
	return (split(/\?/, ($result->bnst)[0]->repname))[0];
    }
    else {
	print STDERR "Error!! Can't get the repname of $word\n";
	return;
    }
}
