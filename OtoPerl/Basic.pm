package OtoPerl::Basic;

my $PI2 = 3.14159265 * 2;
my $sample_rate = 48000;
my $freq_tuning = 440;

my $osc_sin_fact = $PI2 / $sample_rate;
sub osc_sin { # ($frame, $freq)
	sin( $osc_sin_fact * (shift) * (shift) );
}

my $osc_square_fact = 2 / $sample_rate;
sub osc_square { # ($frame, $freq)
	int((shift) * (shift) * $osc_square_fact) & 1 ? 1 : -1;
}

sub freq_bynote {
	int( $freq_tuning * (2**( ((shift)-69)/12 )) );
}

#scalednote([0,2,4,5,7,9,11], $note) for C
#scalednote([1,3,6,8,10], $note)
sub scalednote {
	my $s = shift;
	my $n = shift;
	$s->[$n % @$s] + 12 * int($n / @$s);
}

1;
