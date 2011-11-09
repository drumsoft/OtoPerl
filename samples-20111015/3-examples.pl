use strict;
use warnings;

my $osc1   = OtoPerl::Basic::OSC->new(
	            OtoPerl::Basic::freq_bynote(63)
	            , undef, $frame );

my @scale = (0,4,7,11,14,17,21,24);
my $length = int($sample_rate / 2);
my $freq;

my $filter = OtoPerl::Basic::Filter->new();

sub perl_render {
	my $size = shift;
	my $channels = shift;
	my (@w, $i, $v);

	for ($i = 0; $i < $size; $i++) {
		# Sine Wave
#		$v = sin( 3.1415 * 2 * $frame * 440 / $sample_rate );

		# Square Wave
#		$v = $frame * 440 / $sample_rate;
#		$v = $v - int($v) > 0.5 ? 1 : -1;

		# Amplitude Modulation (AM)
		my $lfo = (sin( 3.1415 * 2 * $frame * 0.5 / $sample_rate ) + 1) / 2;
#		$v *= $lfo;

		# Pulse With Modulation (PWM)
#		$v = $frame * 440 / $sample_rate;
#		$v = $v - int($v) > $lfo ? 1 : -1;

		# Scale
		if ( ! defined($freq) || $frame % $length == 0 ) {
			my $note = $scale[ int($frame / $length) % @scale ];
			$freq = int( 440 * (2**( ($note + 48 - 69) / 12 )) );
		}
		$v = $frame * $freq / $sample_rate;
		$v = $v - int($v) > 0.5 ? 1 : -1;

		# Envelope
#		my $env = $frame % $length;
#		$env = ($length - $env) / $length;
#		$v *= $env;

		# Oscillator Module
#		$osc1->freq($freq);
#		$v = $osc1->next;

		# Frequency Modulation (FM)
#		my $fm = $env * 0.95 * 
#			sin( 3.1415 * 2 * $frame * 660 / $sample_rate ) + 1;
#		$osc1->freq($freq * $fm);
#		$v = $osc1->next;

		# Cut Off Filter
#		$v *= 0.5;
#		my $cutoff = 300 + 200 * sin( 3.1415 * 2 * $frame * 0.5 / $sample_rate );
#		$filter->set($cutoff, 10);
#		$v = $filter->val($v);

		$w[$i + $size*0] = $v;
		$w[$i + $size*1] = $v;
		$frame++;
	}
	return @w;
}
1;