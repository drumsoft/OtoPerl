use strict;
use warnings;

my $PI2 = 3.14159265 * 2;

my $osc_sin_fact = $PI2 / $sample_rate;
sub osc_sin { # ($frame, $freq)
	sin( $osc_sin_fact * (shift) * (shift) );
}

my $osc_square_fact = 2 / $sample_rate;
sub osc_square { # ($frame, $freq)
	int((shift) * (shift) * $osc_square_fact) & 1 ? 1 : -1;
}

my $freq_tuning = 440;
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

my $trigger = 0;
my $triggerl = 0;
my $f3 = 0;
my $f6 = 0;
my $nn;
my $nnl;

my $osc1   = OtoPerl::Basic::OSC->new(
	            OtoPerl::Basic::freq_bynote(63) );
my $width_base = int($sample_rate * 2 / 3);
my $width = 1 + $width_base;

sub perl_render {
	my $size = shift;
	my $channels = shift;
	my (@w, $i, $v, @v, $k, $f, $amp);
	my $m = 4;
	my $f1 = freq_bynote(70);
	my $f2 = freq_bynote(63);
	for ($i = 0; $i < $size; $i++) {
		# downstair note > sine
		if (0 == $frame % $width) {
			$f6 = OtoPerl::Basic::freq_bynote(OtoPerl::Basic::scalednote([0,4,7,9], int(rand(15)+0)));
		}
		$osc1->freq( $f6 );
		my $v6 = (($frame % $width) / $width)
			 * $osc1->val( $frame + int(
			 500*OtoPerl::Basic::osc_sin( $frame, 0.07 )
			 *OtoPerl::Basic::osc_sin( $frame, 400 )) );

		$width = 1 + $width_base + int($width_base*OtoPerl::Basic::osc_sin( $frame, 0.05 ));

		$v = 
		OtoPerl::Basic::osc_sin( $frame, 0.007 ) * 
		OtoPerl::Basic::osc_sin( $frame, 80 );

		$w[$i + $size*0] = $v6+$v;
		$w[$i + $size*1] = $v6+$v;
		$frame++;
	}
	return @w;
}
1;